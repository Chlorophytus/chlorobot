import logging.handlers
import chlorobot_rpc_pb2_grpc
import chlorobot_rpc_pb2
import grpc
from typing import Optional
import logging
import asyncio
import os
import chloresolve
from chloresolve import command, dispatch, strip


class Chloresolver:
    def __init__(
        self,
        stub,
        token: str,
        trigger: str,
        commands: dict[str, chloresolve.dispatch.Command],
    ) -> None:
        self.stub = stub
        self.authentication = chlorobot_rpc_pb2.ChlorobotAuthentication(
            token=token, version=1
        )
        self.logger = logging.getLogger(__class__.__name__)
        self.trigger = trigger
        self.commands = commands

    async def version_string(self) -> str:
        version_got: chlorobot_rpc_pb2.ChlorobotAcknowledgement = await self.stub.Send(
            chlorobot_rpc_pb2.ChlorobotRequest(
                auth=self.authentication,
                command_type=chlorobot_rpc_pb2.ChlorobotCommandEnum.SEND_VERSION,
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
                self.logger.debug(
                    f"[RECV] p:{message.prefix} | cs:{message.non_numeric} | a:{
                    message.parameters} | t:{message.trailing_parameter}"
                )
                command = message.non_numeric
            elif message.numeric:
                self.logger.debug(
                    f"[RECV] p:{message.prefix} | c#:{message.numeric} | a:{
                    message.parameters} | t:{message.trailing_parameter}"
                )
                command = message.numeric

            match command:
                case b"PRIVMSG":
                    # gRPC endpoint does bytes
                    [b_nickname, b_ident_cloak] = message.prefix.split(b"!", 1)
                    [b_ident, b_cloak] = b_ident_cloak.split(b"@", 1)
                    b_channel = message.parameters[0]

                    # Decode them all
                    nickname: str = b_nickname.decode("utf-8")
                    ident: str = b_ident.decode("utf-8")
                    cloak: str = b_cloak.decode("utf-8")
                    channel: str = b_channel.decode("utf-8")

                    message_stripped: bytes = chloresolve.strip.strip_irc_attributes(
                        message.trailing_parameter
                    )

                    if chloresolve.strip.is_action(message_stripped):
                        message_stripped = chloresolve.strip.strip_action(
                            message_stripped
                        )
                        s_message: str = message_stripped.decode(
                            "utf-8", errors="ignore"
                        )
                        self.logger.info(f"[{channel}] * {nickname}{s_message}")
                    else:
                        s_message: str = message_stripped.decode(
                            "utf-8", errors="ignore"
                        )
                        self.logger.info(f"[{channel}] <{nickname}> {s_message}")
                        if s_message.startswith(self.trigger):
                            chanargs = s_message.removeprefix(self.trigger).split(" ")
                            self.logger.debug(
                                f"[RCMD] n:{nickname} i:{ident} c:{
                                             cloak} | h:{channel} | a:{chanargs}"
                            )
                            await self.dispatch(
                                chloresolve.dispatch.Arguments(
                                    self, channel, nickname, ident, cloak, chanargs
                                )
                            )
                case b"MODE":
                    # gRPC endpoint does bytes
                    [b_nickname, b_ident_cloak] = message.prefix.split(b"!", 1)
                    [b_ident, b_cloak] = b_ident_cloak.split(b"@", 1)

                    b_channel = message.parameters[0]

                    if b_channel.startswith(b"#") or b_channel.startswith(b"&"):
                        b_modes = b" ".join(message.parameters[1:])

                        # Decode them all
                        nickname: str = b_nickname.decode("utf-8")
                        channel: str = b_channel.decode("utf-8")
                        modes: str = b_modes.decode("utf-8")

                        self.logger.info(f"[{channel}] {nickname} sets modes '{modes}'")
                    else:
                        b_modes = b" ".join(message.parameters)

                        nickname: str = b_nickname.decode("utf-8")
                        modes: str = b_modes.decode("utf-8")

                        self.logger.info(f"{nickname} sets modes '{modes}'")
                case b"JOIN":
                    # gRPC endpoint does bytes
                    [b_nickname, b_ident_cloak] = message.prefix.split(b"!", 1)
                    [b_ident, b_cloak] = b_ident_cloak.split(b"@", 1)
                    b_channel = message.parameters[0]

                    # Decode them all
                    nickname: str = b_nickname.decode("utf-8")
                    channel: str = b_channel.decode("utf-8")

                    self.logger.info(f"[{channel}] {nickname} joins")
                case b"PART":
                    # gRPC endpoint does bytes
                    [b_nickname, b_ident_cloak] = message.prefix.split(b"!", 1)
                    [b_ident, b_cloak] = b_ident_cloak.split(b"@", 1)
                    b_channel = message.parameters[0]

                    # Decode them all
                    nickname: str = b_nickname.decode("utf-8")
                    channel: str = b_channel.decode("utf-8")
                    reason: str = message.trailing_parameter.decode(
                        "utf-8", errors="ignore"
                    )

                    self.logger.info(f"[{channel}] {nickname} parts ({reason})")
                case b"QUIT":
                    # gRPC endpoint does bytes
                    [b_nickname, b_ident_cloak] = message.prefix.split(b"!", 1)
                    [b_ident, b_cloak] = b_ident_cloak.split(b"@", 1)

                    # Decode them all
                    nickname: str = b_nickname.decode("utf-8")
                    reason: str = message.trailing_parameter.decode(
                        "utf-8", errors="ignore"
                    )

                    self.logger.info(f"{nickname} quits ({reason})")
                case b"KICK":
                    # gRPC endpoint does bytes
                    [b_nickname, b_ident_cloak] = message.prefix.split(b"!", 1)
                    [b_ident, b_cloak] = b_ident_cloak.split(b"@", 1)
                    b_channel = message.parameters[0]
                    b_victim = message.parameters[1]

                    # Decode them all
                    nickname: str = b_nickname.decode("utf-8")
                    channel: str = b_channel.decode("utf-8")
                    reason: str = message.trailing_parameter.decode(
                        "utf-8", errors="ignore"
                    )
                    victim: str = b_victim.decode("utf-8")

                    self.logger.info(
                        f"[{channel}] {nickname} kicks {victim} ({reason})"
                    )
                case b"TOPIC":
                    # gRPC endpoint does bytes
                    [b_nickname, b_ident_cloak] = message.prefix.split(b"!", 1)
                    [b_ident, b_cloak] = b_ident_cloak.split(b"@", 1)
                    b_channel = message.parameters[0]

                    # Decode them all
                    nickname: str = b_nickname.decode("utf-8")
                    channel: str = b_channel.decode("utf-8")
                    topic: str = message.trailing_parameter.decode(
                        "utf-8", errors="ignore"
                    )

                    self.logger.info(f"[{channel}] {nickname} sets topic '{topic}'")

    async def dispatch(self, dispatch_info: chloresolve.dispatch.Arguments) -> None:
        dispatch = dispatch_info.chanargs[0].lower()
        dispatch_cmd = self.commands.get(dispatch, command.COMMAND_NOT_FOUND)
        await dispatch_cmd(dispatch_info)

    async def message(self, channel: str, nickname: str, message: str) -> None:
        myself: str = os.environ["CHLOROBOT_NICKNAME"]
        if channel.startswith("#") or channel.startswith("&"):
            # channel
            self.logger.info(f"[{channel}] <{myself}> {message}")
            try:
                await self.send(
                    None,
                    b"PRIVMSG",
                    [channel.encode()],
                    f"{nickname}: {message}".encode(),
                )
            except UnicodeEncodeError:
                self.logger.warning("Could not encode the last message into bytes!")
        else:
            # user
            self.logger.info(f"[{nickname}] <{myself}> {message}")
            try:
                await self.send(None, b"NOTICE", [nickname.encode()], message.encode())
            except UnicodeEncodeError:
                self.logger.warning("Could not encode the last message into bytes!")

    async def send(
        self,
        prefix: Optional[bytes],
        command: int | bytes,
        parameters: list[bytes],
        trailing_parameter: Optional[bytes],
    ) -> None:
        message = None

        # Log the message
        if isinstance(command, bytes):
            message = chlorobot_rpc_pb2.ChlorobotPacket(
                prefix=prefix,
                non_numeric=command,
                parameters=parameters,
                trailing_parameter=trailing_parameter,
            )
            self.logger.debug(
                f"[SEND] p:{message.prefix} | cs:{message.non_numeric} | a:{
                             message.parameters} | t:{message.trailing_parameter}"
            )
        elif isinstance(command, int):
            message = chlorobot_rpc_pb2.ChlorobotPacket(
                prefix=prefix,
                numeric=command,
                parameters=parameters,
                trailing_parameter=trailing_parameter,
            )
            self.logger.debug(
                f"[SEND] p:{message.prefix} | c#:{message.numeric} | a:{
                             message.parameters} | t:{message.trailing_parameter}"
            )

        # Send it after
        request = chlorobot_rpc_pb2.ChlorobotRequest(
            auth=self.authentication, packet=message
        )
        await self.stub.Send(request)

    async def cancel(self) -> None:
        logging.info("Disconnecting from gRPC socket")
        self.listener.cancel()


async def main() -> None:
    logging.info(f"Chloresolver {chloresolve.__version__}")
    async with grpc.aio.insecure_channel(
        f"{os.environ["CHLOROBOT_RPC_SERVER"]}:50051"
    ) as channel:
        logging.info("Trying to connect to gRPC socket")
        stub = chlorobot_rpc_pb2_grpc.ChlorobotRPCStub(channel)
        resolver = Chloresolver(
            stub,
            os.environ["CHLOROBOT_RPC_TOKEN"],
            "c|",
            {
                "ping": chloresolve.dispatch.Command(
                    chloresolve.command.ping,
                    "acknowledges if the bot resolver is online",
                ),
                "help": chloresolve.dispatch.Command(
                    chloresolve.command.help,
                    "lists commands or gives a detailed description of one",
                ),
                "join": chloresolve.dispatch.Command(
                    chloresolve.command.join, "joins a channel"
                ),
                "part": chloresolve.dispatch.Command(
                    chloresolve.command.part, "parts a channel"
                ),
                "version": chloresolve.dispatch.Command(
                    chloresolve.command.version, "gets the bot's version information"
                ),
                "chain": chloresolve.dispatch.Command(
                    chloresolve.command.chain, "creates a sentence using a Markov chain"
                ),
                "wiki": chloresolve.dispatch.Command(
                    chloresolve.command.wiki,
                    "gets a Wikipedia article's short description",
                ),
            },
        )

        ping = chlorobot_rpc_pb2.ChlorobotRequest(
            auth=resolver.authentication,
            command_type=chlorobot_rpc_pb2.ChlorobotCommandEnum.SEND_NOTHING,
        )
        result = await stub.Send(ping, timeout=20)

        logging.info("Connected to gRPC socket")
        await resolver.listen()


if __name__ == "__main__":
    logging.basicConfig(
        level=logging.INFO,
        format="[%(asctime)s] [%(name)s - %(levelname)s] %(message)s",
    )
    asyncio.run(main())
