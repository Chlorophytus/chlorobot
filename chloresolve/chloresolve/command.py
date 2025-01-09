import os
import random
from . import dispatch, markov, mediawiki, __version__, tables, calc


async def not_found(args: dispatch.Arguments):
    await args.resolver.message(args.channel, args.nickname, "Command not found")


COMMAND_NOT_FOUND: dispatch.Command = dispatch.Command(not_found, "command not found")


async def ping(args: dispatch.Arguments):
    await args.resolver.message(args.channel, args.nickname, "Pong")


async def help(args: dispatch.Arguments):
    match len(args.chanargs):
        case 1:
            available = ", ".join([*args.resolver.commands])
            await args.resolver.message(
                args.channel, args.nickname, f"Commands available are '{available}'"
            )
        case 2:
            dispatch_cmd = args.resolver.commands.get(
                args.chanargs[1], COMMAND_NOT_FOUND
            )
            await args.resolver.message(
                args.channel,
                args.nickname,
                f"{args.chanargs[1]} - {dispatch_cmd.help_str}",
            )
        case _:
            await args.resolver.message(
                args.channel, args.nickname, "This command takes 0 or 1 arguments"
            )


async def perms(args: dispatch.Arguments):
    match len(args.chanargs):
        case 1:
            with tables.PermissionsTable(args.cloak) as t:
                await args.resolver.message(
                    args.channel,
                    args.nickname,
                    f"You have permissions '{', '.join(t.perms)}'",
                )
        case 2:
            can_use = False
            with tables.PermissionsTable(args.cloak) as t:
                can_use = t.perm_get("perms_get")

            if can_use:
                with tables.PermissionsTable(args.chanargs[1]) as t:
                    await args.resolver.message(
                        args.channel,
                        args.nickname,
                        f"User with cloak '{args.chanargs[1]}' has permissions '{', '.join(t.perms)}'",
                    )
        case _:
            can_modify = False
            with tables.PermissionsTable(args.cloak) as t:
                can_modify = t.perm_get("perms_mut")

            if can_modify:
                with tables.PermissionsTable(args.chanargs[1]) as t:
                    for pflag in args.chanargs[2:]:
                        match pflag[0]:
                            case "+":
                                t.perm_set(pflag[1:], True)
                            case "-":
                                t.perm_set(pflag[1:], False)

                    await args.resolver.message(
                        args.channel,
                        args.nickname,
                        f"User with cloak '{args.chanargs[1]}' now has permissions '{', '.join(t.perms)}'",
                    )


async def version(args: dispatch.Arguments):
    version = await args.resolver.version_string()
    await args.resolver.message(
        args.channel,
        args.nickname,
        f"Using core v{version} and resolver v{__version__}",
    )


async def join(args: dispatch.Arguments):
    can_use = False
    with tables.PermissionsTable(args.cloak) as t:
        can_use = t.perm_get("channels_mut")

    if can_use:
        await args.resolver.send(None, b"JOIN", [args.chanargs[1].encode()], None)


async def part(args: dispatch.Arguments):
    can_use = False
    with tables.PermissionsTable(args.cloak) as t:
        can_use = t.perm_get("channels_mut")

    if can_use:
        await args.resolver.send(None, b"PART", [args.chanargs[1].encode()], None)


async def exit(args: dispatch.Arguments):
    can_use = False
    with tables.PermissionsTable(args.cloak) as t:
        can_use = t.perm_get("exit")

    if can_use:
        await args.resolver.quit()


async def chain(args: dispatch.Arguments):
    can_use = False
    with tables.PermissionsTable(args.cloak) as t:
        can_use = t.perm_get("markov")

    global CHAINER_OBJECT
    if can_use:
        chanarg_len = len(args.chanargs)
        if chanarg_len > 1:
            chanarg_determine = args.chanargs[1].lower()
            match chanarg_determine:
                case "clear":
                    CHAINER_OBJECT = markov.Chainer()
                    await args.resolver.message(
                        args.channel, args.nickname, "Cleared the Markov chainer"
                    )
                case "parse":
                    if chanarg_len > 2:
                        path = " ".join(args.chanargs[2:])
                        try:
                            with open(path, "r") as f:
                                CHAINER_OBJECT.parse(f)
                                await args.resolver.message(
                                    args.channel, args.nickname, f"Parsed '{path}'"
                                )
                        except FileNotFoundError:
                            await args.resolver.message(
                                args.channel, args.nickname, "File does not exist"
                            )
                    else:
                        await args.resolver.message(
                            args.channel,
                            args.nickname,
                            "Parser takes 2 or more arguments",
                        )
                case "run":
                    if chanarg_len == 4:
                        try:
                            min: int = int(args.chanargs[2])
                            max: int = int(args.chanargs[3])

                            await args.resolver.message(
                                args.channel,
                                args.nickname,
                                CHAINER_OBJECT.run(min, max),
                            )
                        except ValueError:
                            await args.resolver.message(
                                args.channel,
                                args.nickname,
                                "Minimum and maximum sentence word lengths must be integers",
                            )
                    else:
                        await args.resolver.message(
                            args.channel,
                            args.nickname,
                            "Minimum and maximum sentence word lengths must be specified",
                        )
                case _:
                    await args.resolver.message(
                        args.channel,
                        args.nickname,
                        "Use subcommand 'clear', 'parse', or 'run'",
                    )
        else:
            await args.resolver.message(
                args.channel, args.nickname, "Use subcommand 'clear', 'parse', or 'run'"
            )


async def wiki(args: dispatch.Arguments):
    can_use = False
    with tables.PermissionsTable(args.cloak) as t:
        can_use = t.perm_get("wiki")


    if can_use:
        if len(args.chanargs) > 1:
            article: str = " ".join(args.chanargs[1:])
            wiki_lookup = mediawiki.PageLookup("https://en.wikipedia.org/w/api.php")
            try:
                description: str = await wiki_lookup.query(article)
                await args.resolver.message(args.channel, args.nickname, description)
            except:
                await args.resolver.message(
                    args.channel, args.nickname, f"Article synopsis lookup failed"
                )
        else:
            await args.resolver.message(
                args.channel, args.nickname, "Please give an article that can be looked up"
            )


async def botsnack(args: dispatch.Arguments):
    # Random greens
    if args.channel != args.nickname:
        pick = random.choice([
            "spirulina",
            "lettuce",
            "cilantro",
            "celery",
            "broccoli"
        ])

        await args.resolver.action(args.channel, f"eats some {pick}")


async def calculate(args: dispatch.Arguments):
    calculation = calc.Calculator(args.chanargs[1:])
    calculation.run()
    result = calculation.get_result()
    if result is None:
        await args.resolver.message(args.channel, args.nickname, calculation.get_error())
    else:
        await args.resolver.message(args.channel, args.nickname, f"= {result}")
