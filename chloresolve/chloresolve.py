import chlorobot_rpc_pb2_grpc
import chlorobot_rpc_pb2
import grpc
from typing import Optional
import logging
import asyncio
import os


class Chloresolver:
    pass


class ChloresolverDispatchArgs:
    def __init__(self, resolver: Chloresolver, channel: str, nickname: str, ident: str, cloak: str, chanargs: list[str]):
        self.channel = channel
        self.nickname = nickname
        self.ident = ident
        self.cloak = cloak
        self.chanargs = chanargs
        self.resolver = resolver


class ChloresolverCommand:
    def __init__(self, function, help_str: str = "no help available") -> None:
        self.function = function
        self.help_str = help_str

    async def __call__(self, args: ChloresolverDispatchArgs) -> None:
        await self.function(args)

    @staticmethod
    async def not_found(args: ChloresolverDispatchArgs):
        await args.resolver.message(args.channel, args.nickname, "Command not found")

    @staticmethod
    async def ping(args: ChloresolverDispatchArgs):
        await args.resolver.message(args.channel, args.nickname, "Pong")

    @staticmethod
    async def help(args: ChloresolverDispatchArgs):
        match len(args.chanargs):
            case 1:
                available = ", ".join([*args.resolver.commands])
                await args.resolver.message(args.channel, args.nickname, f"Commands available are '{available}'")
            case 2:
                dispatch_cmd = args.resolver.commands.get(
                    args.chanargs[1], CHLORESOLVE_COMMAND_NOT_FOUND)
                await args.resolver.message(args.channel, args.nickname, f"{args.chanargs[1]} - {dispatch_cmd.help_str}")
            case _:
                await args.resolver.message(args.channel, args.nickname, "This call takes 0 or 1 arguments")

    @staticmethod
    async def version(args: ChloresolverDispatchArgs):
        version = await args.resolver.version_string()
        await args.resolver.message(args.channel, args.nickname, f"Chlorobot - Core v{version}")

    @staticmethod
    async def join(args: ChloresolverDispatchArgs):
        if args.cloak.lower() == os.environ["CHLOROBOT_OWNER"]:
            await args.resolver.send(None, "JOIN", [args.chanargs[1]], None)
        else:
            await args.resolver.message(args.channel, args.nickname, f"Not authorized")

    @staticmethod
    async def part(args: ChloresolverDispatchArgs):
        if args.cloak.lower() == os.environ["CHLOROBOT_OWNER"]:
            await args.resolver.send(None, "PART", [args.chanargs[1]], None)
        else:
            await args.resolver.message(args.channel, args.nickname, f"Not authorized")


CHLORESOLVE_COMMAND_NOT_FOUND = ChloresolverCommand(
    ChloresolverCommand.not_found, "invalid command")


class Chloresolver:
    def __init__(self, stub, token: str, trigger: str, commands: dict[str, ChloresolverCommand]) -> None:
        self.stub = stub
        self.authentication = chlorobot_rpc_pb2.ChlorobotAuthentication(
            token=token)
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
                self.logger.info(f"[RECV] p:{message.prefix} | cs:{message.non_numeric} | a:{
                    message.parameters} | t:{message.trailing_parameter}")
                command = message.non_numeric
            elif message.numeric:
                self.logger.info(f"[RECV] p:{message.prefix} | c#:{message.numeric} | a:{
                    message.parameters} | t:{message.trailing_parameter}")
                command = message.numeric

            match command:
                case "PRIVMSG":
                    if message.trailing_parameter.startswith(self.trigger):
                        [nickname, ident_cloak] = message.prefix.split("!", 1)
                        [ident, cloak] = ident_cloak.split("@", 1)
                        channel = message.parameters[0]
                        chanargs = message.trailing_parameter.removeprefix(
                            self.trigger).split(" ")
                        self.logger.info(f"[RCMD] n:{nickname} i:{ident} c:{
                                         cloak} | h:{channel} | a:{chanargs}")

                        await self.dispatch(ChloresolverDispatchArgs(self, channel, nickname, ident, cloak, chanargs))

    async def dispatch(self, dispatch_info: ChloresolverDispatchArgs) -> None:
        dispatch = dispatch_info.chanargs[0].lower()
        dispatch_cmd = self.commands.get(
            dispatch, CHLORESOLVE_COMMAND_NOT_FOUND)
        await dispatch_cmd(dispatch_info)

    async def message(self, channel: str, nickname: str, message: str) -> None:
        if channel.startswith("#") or channel.startswith("&"):
            # channel
            await self.send(None, "PRIVMSG", [channel], f"{nickname}: {message}")
        else:
            # user
            await self.send(None, "NOTICE", [nickname], message)

    async def send(self, prefix: Optional[str], command: (int | str), parameters: list[str], trailing_parameter: Optional[str]) -> None:
        message = None

        # Log the message
        if isinstance(command, str):
            message = chlorobot_rpc_pb2.ChlorobotPacket(
                prefix=prefix,
                non_numeric=command,
                parameters=parameters,
                trailing_parameter=trailing_parameter)
            self.logger.info(f"[SEND] p:{message.prefix} | cs:{message.non_numeric} | a:{
                             message.parameters} | t:{message.trailing_parameter}")
        elif isinstance(command, int):
            message = chlorobot_rpc_pb2.ChlorobotPacket(
                prefix=prefix,
                numeric=command,
                parameters=parameters,
                trailing_parameter=trailing_parameter)
            self.logger.info(f"[SEND] p:{message.prefix} | c#:{message.numeric} | a:{
                             message.parameters} | t:{message.trailing_parameter}")

        # Send it after
        request = chlorobot_rpc_pb2.ChlorobotRequest(
            auth=self.authentication, packet=message)
        await self.stub.Send(request)

    async def cancel(self) -> None:
        logging.info("Disconnecting from gRPC socket")
        self.listener.cancel()


async def main() -> None:
    async with grpc.aio.insecure_channel(f"{os.environ["CHLOROBOT_RPC_SERVER"]}:50051") as channel:
        stub = chlorobot_rpc_pb2_grpc.ChlorobotRPCStub(channel)
        resolver = Chloresolver(stub, os.environ["CHLOROBOT_RPC_TOKEN"], "c|", {
            "ping": ChloresolverCommand(ChloresolverCommand.ping, "acknowledges if the bot resolver is online"),
            "help": ChloresolverCommand(ChloresolverCommand.help, "lists commands or gives a detailed description of one"),
            "join": ChloresolverCommand(ChloresolverCommand.join, "joins a channel"),
            "part": ChloresolverCommand(ChloresolverCommand.part, "parts a channel"),
            "version": ChloresolverCommand(ChloresolverCommand.version, "gets the core's version")
        })
        logging.info("Connected to gRPC socket")
        await resolver.listen()


if __name__ == "__main__":
    logging.basicConfig(
        level=logging.INFO, format='[%(asctime)s] [%(name)s - %(levelname)s] %(message)s')
    asyncio.run(main())
