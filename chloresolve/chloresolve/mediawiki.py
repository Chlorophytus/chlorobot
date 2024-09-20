import asyncio
import aiohttp
from html.parser import HTMLParser


class HTMLStripper(HTMLParser):
    """
    An HTMLParser that only accepts data, effectively stripping the tags.
    """

    def __init__(self, *, convert_charrefs: bool = True) -> None:
        super().__init__(convert_charrefs=convert_charrefs)
        self.stripped_text: str = ""

    def handle_data(self, data):
        self.stripped_text += data


class PageLookup:
    """
    A MediaWiki description lookup utility.
    """

    def __init__(self, endpoint: str):
        """
        Initializes a page lookup given a MediaWiki endpoint URL and its query.
        """
        self.endpoint: str = endpoint

    async def query(self, title: str) -> str:
        """
        Queries the endpoint API and then returns a future description result.

        The HTML will be stripped.
        """
        # Query your wiki's JSON description
        async with aiohttp.ClientSession() as session:
            async with session.get(
                self.endpoint,
                params={
                    "action": "query",
                    "format": "json",
                    "prop": "description",
                    "titles": title,
                    "redirects": 1,
                    "formatversion": 2,
                },
            ) as r:
                # Get the result
                result = await r.json()
                query: dict = result["query"]["pages"][0]

                if "description" in query.keys():
                    # Strip the HTML
                    stripper: HTMLStripper = HTMLStripper()
                    stripper.feed(query["description"])
                    stripper.close()

                    # Return the retrieved and stripped description
                    return f"{query['title']} - {stripper.stripped_text}"

        return f"{query['title']} - Could not get synopsis"
