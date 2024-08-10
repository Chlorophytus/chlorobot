import asyncio
import logging
import requests


class UptimeTimer:
    def __init__(self, interval_seconds: int, uri: str):
        """
        Initializes a heartbeat timer per the specified seconds interval
        """
        self.interval_seconds = interval_seconds
        self.uri = uri
        self.task = asyncio.ensure_future(self.heartbeat())
        self.logger = logging.getLogger(__class__.__name__)

    async def ping(self):
        """
        Heartbeats to an UptimeRobot/etc. URL
        """
        requests.get(self.uri)
        self.logger.info("Sending heartbeat")
        await asyncio.sleep(self.interval_seconds)
        await self.ping()

    def cancel(self):
        """
        Cancels the heartbeat timer task.
        """
        self.logger.info("Cancelling heartbeat task")
        self.task.cancel()
