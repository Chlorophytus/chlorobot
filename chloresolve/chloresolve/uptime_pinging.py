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
        self.loop = asyncio.get_event_loop()
        self.task = self.loop.create_task(self.heartbeat)
        self.logger = logging.getLogger(__class__.__name__)

    async def heartbeat(self):
        """
        Sends in a loop a heartbeat to an UptimeRobot/etc. URL

        You should call start() instead
        """
        while True:
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

    async def start():
        """
        Starts the heartbeat loop
        """
        try:
            self.logger.info("Heartbeat task starting")
            self.loop.run_until_complete(self.task)
        except Exception as exc:
            self.logger.info("Heartbeat task failed or cancelled")
            self.logger.info(
                f"Exception was {type(exc).__name__} with args '{exc.args}'"
            )

    def cancel():
        """
        Cancels the heartbeat loop
        """
        self.logger.info("Cancelling heartbeat task")
        self.task.cancel()
