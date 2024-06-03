import logging.handlers
import pathlib
import chloresolve.command
import chloresolve.dispatch
import chlorobot_rpc_pb2_grpc
import chlorobot_rpc_pb2
import grpc
from typing import Optional
import logging
import asyncio
import os
import chloresolve
from chloresolve import dispatch, command

CHLOROBOT_ENCODER_VERSION = 1


class Chloresolver:
    def __init__(self, stub, token: str, trigger: bytes, commands: dict[str, chloresolve.dispatch.Command]) -> None:
        self.stub = stub
        self.authentication = chlorobot_rpc_pb2.ChlorobotAuthentication(
            token=token, version=CHLOROBOT_ENCODER_VERSION)
        self.logger = logging.getLogger(__class__.__name__)
        self.trigger = trigger
        self.commands = commands

    async def version_string(self) -> str:
        version_got: chlorobot_rpc_pb2.ChlorobotAcknowledgement = await self.stub.Send(
            chlorobot_rpc_pb2.ChlorobotRequest(
                auth=self.authentication,
                command_type=chlorobot_rpc_pb2.ChlorobotCommandEnum.SEND_VERSION
            )
        )
        return version_got.version.pretty

    async def listen(self) -> None:
        self.listener = self.stub.Listen(self.authentication)
        async for message in self.listener:
            message: chlorobot_rpc_pb2.ChlorobotPacket
            command = None

            # Log the message
            if message.non_numeric:
                self.logger.debug(f"[RECV] p:{message.prefix} | cs:{message.non_numeric} | a:{
                    message.parameters} | t:{message.trailing_parameter}")
                command = message.non_numeric
            elif message.numeric:
                self.logger.debug(f"[RECV] p:{message.prefix} | c#:{message.numeric} | a:{
                    message.parameters} | t:{message.trailing_parameter}")
                command = message.numeric

            match command:
                case b"PRIVMSG":
                    # gRPC endpoint does bytes
                    [b_nickname, b_ident_cloak] = message.prefix.split(b'!', 1)
                    [b_ident, b_cloak] = b_ident_cloak.split(b'@', 1)
                    b_channel = message.parameters[0]

                    # Decode them all
                    nickname: str = b_nickname.decode('utf-8')
                    ident: str = b_ident.decode('utf-8')
                    cloak: str = b_cloak.decode('utf-8')
                    channel: str = b_channel.decode('utf-8')

                    self.logger.info(f"[{channel}] <{nickname}> {message}")
                    if message.trailing_parameter.startswith(self.trigger):
                        s_message: str = message.trailing_parameter.decode('utf-8', errors='ignore')
                        chanargs = s_message.trailing_parameter.removeprefix(
                            self.trigger).split(' ')
                        self.logger.debug(f"[RCMD] n:{nickname} i:{ident} c:{
                                         cloak} | h:{channel} | a:{chanargs}")

                        await self.dispatch(chloresolve.dispatch.Arguments(self, channel, nickname, ident, cloak, chanargs))

    async def dispatch(self, dispatch_info: chloresolve.dispatch.Arguments) -> None:
        dispatch = dispatch_info.chanargs[0].lower()
        dispatch_cmd = self.commands.get(
            dispatch, command.COMMAND_NOT_FOUND)
        await dispatch_cmd(dispatch_info)

    async def message(self, channel: str, nickname: str, message: str) -> None:
        myself: str = os.environ["CHLOROBOT_NICKNAME"]
        if channel.startswith('#') or channel.startswith('&'):
            # channel
            self.logger.info(f"[{channel}] <{myself}> {message}")
            try:
                await self.send(None, b"PRIVMSG", [channel.encode()], f"{nickname}: {message}".encode())
            except UnicodeEncodeError:
                self.logger.warning("Could not encode the last message into bytes!")
        else:
            # user
            self.logger.info(f"[{nickname}] <{myself}> {message}")
            try:
                await self.send(None, b"NOTICE", [nickname.encode()], message.encode())
            except UnicodeEncodeError:
                self.logger.warning("Could not encode the last message into bytes!")

    async def send(self, prefix: Optional[bytes], command: (int | bytes), parameters: list[bytes], trailing_parameter: Optional[bytes]) -> None:
        message = None

        # Log the message
        if isinstance(command, bytes):
            message = chlorobot_rpc_pb2.ChlorobotPacket(
                prefix=prefix,
                non_numeric=command,
                parameters=parameters,
                trailing_parameter=trailing_parameter)
            self.logger.debug(f"[SEND] p:{message.prefix} | cs:{message.non_numeric} | a:{
                             message.parameters} | t:{message.trailing_parameter}")
        elif isinstance(command, int):
            message = chlorobot_rpc_pb2.ChlorobotPacket(
                prefix=prefix,
                numeric=command,
                parameters=parameters,
                trailing_parameter=trailing_parameter)
            self.logger.debug(f"[SEND] p:{message.prefix} | c#:{message.numeric} | a:{
                             message.parameters} | t:{message.trailing_parameter}")

        # Send it after
        request = chlorobot_rpc_pb2.ChlorobotRequest(
            auth=self.authentication, packet=message)
        await self.stub.Send(request)

    async def cancel(self) -> None:
        logging.info("Disconnecting from gRPC socket")
        self.listener.cancel()


async def main() -> None:
    logging.info(f"Chloresolver {chloresolve.__version__}")
    async with grpc.aio.insecure_channel(f"{os.environ["CHLOROBOT_RPC_SERVER"]}:50051") as channel:
        logging.info("Trying to connect to gRPC socket")
        stub = chlorobot_rpc_pb2_grpc.ChlorobotRPCStub(channel)
        resolver = Chloresolver(stub, os.environ["CHLOROBOT_RPC_TOKEN"], b'c|', {
            "ping": chloresolve.dispatch.Command(chloresolve.command.ping, "acknowledges if the bot resolver is online"),
            "help": chloresolve.dispatch.Command(chloresolve.command.help, "lists commands or gives a detailed description of one"),
            "join": chloresolve.dispatch.Command(chloresolve.command.join, "joins a channel"),
            "part": chloresolve.dispatch.Command(chloresolve.command.part, "parts a channel"),
            "version": chloresolve.dispatch.Command(chloresolve.command.version, "gets the bot's version information"),
            "chain": chloresolve.dispatch.Command(chloresolve.command.chain, "creates a sentence using a Markov chain"),
            "wiki": chloresolve.dispatch.Command(chloresolve.command.wiki, "gets a Wikipedia article's short description")
        })

        ping = chlorobot_rpc_pb2.ChlorobotRequest(
            auth=resolver.authentication, command_type=chlorobot_rpc_pb2.ChlorobotCommandEnum.SEND_NOTHING)
        result = await stub.Send(ping, timeout=15)

        logging.info("Connected to gRPC socket")
        await resolver.listen()


if __name__ == "__main__":
    logging.basicConfig(
        level=logging.INFO, format='[%(asctime)s] [%(name)s - %(levelname)s] %(message)s')
    asyncio.run(main())
