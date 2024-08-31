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

    async def heartbeat(self):
        """
        Heartbeats to an UptimeRobot/etc. URL
        """
        for i in range(3):
            try:
                self.logger.info(f"Sending heartbeat try {i + 1}")
                requests.get(self.uri, timeout=5)
                break
            except requests.exceptions.Timeout:
                self.logger.info(f"Heartbeat try {i + 1} timed out")
        self.logger.info("Done trying heartbeat")
        await asyncio.sleep(self.interval_seconds)
        await self.heartbeat()

    def cancel(self):
        """
        Cancels the heartbeat timer task.
        """
        self.logger.info("Cancelling heartbeat task")
        self.task.cancel()
