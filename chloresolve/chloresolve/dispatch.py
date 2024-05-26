class Arguments:
    def __init__(self, resolver, channel: str, nickname: str, ident: str, cloak: str, chanargs: list[str]):
        self.channel = channel
        self.nickname = nickname
        self.ident = ident
        self.cloak = cloak
        self.chanargs = chanargs
        self.resolver = resolver


class Command:
    def __init__(self, function, help_str: str = "no help available") -> None:
        self.function = function
        self.help_str = help_str

    async def __call__(self, args: Arguments) -> None:
        await self.function(args)