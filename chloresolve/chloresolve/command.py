import os
from . import dispatch


async def not_found(args: dispatch.Arguments):
    await args.resolver.message(args.channel, args.nickname, "Command not found")


async def ping(args: dispatch.Arguments):
    await args.resolver.message(args.channel, args.nickname, "Pong")


async def help(args: dispatch.Arguments):
    match len(args.chanargs):
        case 1:
            available = ", ".join([*args.resolver.commands])
            await args.resolver.message(args.channel, args.nickname, f"Commands available are '{available}'")
        case 2:
            dispatch_cmd = args.resolver.commands.get(
                args.chanargs[1], not_found)
            await args.resolver.message(args.channel, args.nickname, f"{args.chanargs[1]} - {dispatch_cmd.help_str}")
        case _:
            await args.resolver.message(args.channel, args.nickname, "This call takes 0 or 1 arguments")


async def version(args: dispatch.Arguments):
    version = await args.resolver.version_string()
    await args.resolver.message(args.channel, args.nickname, f"Chlorobot - Core v{version} - Resolver v{__version__}")


async def join(args: dispatch.Arguments):
    if args.cloak.lower() == os.environ["CHLOROBOT_OWNER"]:
        await args.resolver.send(None, "JOIN", [args.chanargs[1]], None)
    else:
        await args.resolver.message(args.channel, args.nickname, f"Not authorized")


async def part(args: dispatch.Arguments):
    if args.cloak.lower() == os.environ["CHLOROBOT_OWNER"]:
        await args.resolver.send(None, "PART", [args.chanargs[1]], None)
    else:
        await args.resolver.message(args.channel, args.nickname, f"Not authorized")
