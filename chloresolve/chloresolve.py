import os

import asyncio
import logging

import grpc
import chlorobot_rpc_pb2
import chlorobot_rpc_pb2_grpc


async def chlorobot_listen(
        stub: chlorobot_rpc_pb2_grpc.ChlorobotRPCStub,
        auth_token: str) -> None:
    authentication = chlorobot_rpc_pb2.ChlorobotAuthentication(token=auth_token)
    join_packet = chlorobot_rpc_pb2.ChlorobotPacket(
        non_numeric="JOIN", parameters=["##sweezero"])
    join_channel = chlorobot_rpc_pb2.ChlorobotRequest(
        auth=authentication, command_type=chlorobot_rpc_pb2.ChlorobotCommandEnum.SEND_VERSION)
    print(await stub.Send(join_channel))
    try:
        listener = stub.Listen(authentication)
        async for message in listener:
            print(message)
    except KeyboardInterrupt:
        listener.cancel()

async def main() -> None:
    async with grpc.aio.insecure_channel("127.0.0.1:50051") as channel:
        stub = chlorobot_rpc_pb2_grpc.ChlorobotRPCStub(channel)
        await chlorobot_listen(stub, os.environ["CHLOROBOT_RPC_TOKEN"])


if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO)
    asyncio.run(main())
