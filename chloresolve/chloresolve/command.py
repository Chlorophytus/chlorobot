import os
from . import dispatch, markov, __version__


async def not_found(args: dispatch.Arguments):
    await args.resolver.message(args.channel, args.nickname, "Command not found")


COMMAND_NOT_FOUND: dispatch.Command = dispatch.Command(
    not_found, "command not found")

global CHLOROBOT_MARKOV_CHAINER


async def ping(args: dispatch.Arguments):
    await args.resolver.message(args.channel, args.nickname, "Pong")


async def help(args: dispatch.Arguments):
    match len(args.chanargs):
        case 1:
            available = ", ".join([*args.resolver.commands])
            await args.resolver.message(args.channel, args.nickname, f"Commands available are '{available}'")
        case 2:
            dispatch_cmd = args.resolver.commands.get(
                args.chanargs[1], COMMAND_NOT_FOUND)
            await args.resolver.message(args.channel, args.nickname, f"{args.chanargs[1]} - {dispatch_cmd.help_str}")
        case _:
            await args.resolver.message(args.channel, args.nickname, "This command takes 0 or 1 arguments")


async def version(args: dispatch.Arguments):
    version = await args.resolver.version_string()
    await args.resolver.message(args.channel, args.nickname, f"Using core v{version} and resolver v{__version__}")


async def join(args: dispatch.Arguments):
    if args.cloak.lower() == os.environ["CHLOROBOT_OWNER"]:
        await args.resolver.send(None, "JOIN", [args.chanargs[1]], None)
    else:
        await args.resolver.message(args.channel, args.nickname, "Not authorized")


async def part(args: dispatch.Arguments):
    if args.cloak.lower() == os.environ["CHLOROBOT_OWNER"]:
        await args.resolver.send(None, "PART", [args.chanargs[1]], None)
    else:
        await args.resolver.message(args.channel, args.nickname, "Not authorized")


async def chain(args: dispatch.Arguments):
    if args.cloak.lower() == os.environ["CHLOROBOT_OWNER"]:
        chanarg_len = len(args.chanargs)
        if chanarg_len > 1:
            chanarg_determine = args.chanargs[1].lower()
            match chanarg_determine:
                case "clear":
                    CHLOROBOT_MARKOV_CHAINER = markov.Chainer()
                    await args.resolver.message(args.channel, args.nickname, "Cleared the Markov chainer")
                case "parse":
                    if chanarg_len > 2:
                        path = " ".join(args.chanargs[2:])
                        try:
                            if CHLOROBOT_MARKOV_CHAINER is None:
                                CHLOROBOT_MARKOV_CHAINER = markov.Chainer()
                            
                            with open(path, "r") as f:
                                CHLOROBOT_MARKOV_CHAINER.parse(f)
                                await args.resolver.message(args.channel, args.nickname, f"Parsed '{path}'")
                        except FileNotFoundError:
                            await args.resolver.message(args.channel, args.nickname, "File does not exist")
                    else:
                        await args.resolver.message(args.channel, args.nickname, "Parser takes 2 or more arguments")
                case "run":
                    if chanarg_len == 4:
                        try:
                            min: int = int(args.chanarg[2])
                            max: int = int(args.chanarg[3])

                            await args.resolver.message(args.channel, args.nickname, CHLOROBOT_MARKOV_CHAINER.run(min, max))
                        except ValueError:
                            await args.resolver.message(args.channel, args.nickname, "Minimum and maximum sentence word lengths must be integers")
                    else:
                        await args.resolver.message(args.channel, args.nickname, "Minimum and maximum sentence word lengths must be specified")
                case _:
                    await args.resolver.message(args.channel, args.nickname, "Use subcommand 'clear', 'parse', or 'run'")
        else:
            await args.resolver.message(args.channel, args.nickname, "Use subcommand 'clear', 'parse', or 'run'")
    else:
        await args.resolver.message(args.channel, args.nickname, "Not authorized")
