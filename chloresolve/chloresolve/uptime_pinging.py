import asyncio
import aiohttp
import logging


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
        Sends multiple heartbeats to an UptimeRobot/etc. URL
        """
        successful = False
        for i in range(5):
            # That's it, we're done with heartbeating
            if successful:
                break

            # Otherwise try a few times to heartbeat
            try:
                async with aiohttp.ClientSession() as session:
                    self.logger.info(f"Sending heartbeat try {i + 1}")
                    resp = await asyncio.wait_for(session.get(self.uri), 5)
                    self.logger.info(f"Heartbeat try {i + 1} success")
                    successful = True
            except Exception as exc:
                # Do not flood the heartbeat endpoint, wait 5 seconds
                self.logger.info(
                    f"Heartbeat try {i + 1} failed, retrying after a delay"
                )
                self.logger.info(
                    f"Exception was {type(exc).__name__} with args '{exc.args}'"
                )
                await asyncio.sleep(5)

        self.logger.info("Waiting for next heartbeat trigger")
        await asyncio.sleep(self.interval_seconds)
        await self.heartbeat()

    def cancel(self):
        """
        Cancels the heartbeat timer task.
        """
        self.logger.info("Cancelling heartbeat task")
        self.task.cancel()
