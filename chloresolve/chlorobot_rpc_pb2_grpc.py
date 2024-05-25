# Generated by the gRPC Python protocol compiler plugin. DO NOT EDIT!
"""Client and server classes corresponding to protobuf-defined services."""
import grpc
import warnings

import chlorobot_rpc_pb2 as chlorobot__rpc__pb2

GRPC_GENERATED_VERSION = '1.64.0'
GRPC_VERSION = grpc.__version__
EXPECTED_ERROR_RELEASE = '1.65.0'
SCHEDULED_RELEASE_DATE = 'June 25, 2024'
_version_not_supported = False

try:
    from grpc._utilities import first_version_is_lower
    _version_not_supported = first_version_is_lower(GRPC_VERSION, GRPC_GENERATED_VERSION)
except ImportError:
    _version_not_supported = True

if _version_not_supported:
    warnings.warn(
        f'The grpc package installed is at version {GRPC_VERSION},'
        + f' but the generated code in chlorobot_rpc_pb2_grpc.py depends on'
        + f' grpcio>={GRPC_GENERATED_VERSION}.'
        + f' Please upgrade your grpc module to grpcio>={GRPC_GENERATED_VERSION}'
        + f' or downgrade your generated code using grpcio-tools<={GRPC_VERSION}.'
        + f' This warning will become an error in {EXPECTED_ERROR_RELEASE},'
        + f' scheduled for release on {SCHEDULED_RELEASE_DATE}.',
        RuntimeWarning
    )


class ChlorobotRPCStub(object):
    """A Chlorobot IRC RPC interface
    """

    def __init__(self, channel):
        """Constructor.

        Args:
            channel: A grpc.Channel.
        """
        self.Listen = channel.unary_stream(
                '/ChlorobotRPC/Listen',
                request_serializer=chlorobot__rpc__pb2.ChlorobotAuthentication.SerializeToString,
                response_deserializer=chlorobot__rpc__pb2.ChlorobotPacket.FromString,
                _registered_method=True)
        self.Send = channel.unary_unary(
                '/ChlorobotRPC/Send',
                request_serializer=chlorobot__rpc__pb2.ChlorobotRequest.SerializeToString,
                response_deserializer=chlorobot__rpc__pb2.ChlorobotAcknowledgement.FromString,
                _registered_method=True)


class ChlorobotRPCServicer(object):
    """A Chlorobot IRC RPC interface
    """

    def Listen(self, request, context):
        """Listens in on the IRC interface

        Streams parsed IRC packets
        """
        context.set_code(grpc.StatusCode.UNIMPLEMENTED)
        context.set_details('Method not implemented!')
        raise NotImplementedError('Method not implemented!')

    def Send(self, request, context):
        """Sends a packet to the IRC interface

        Must send authentication data for security
        Returns an acknowledgement token indicating if it was successful
        """
        context.set_code(grpc.StatusCode.UNIMPLEMENTED)
        context.set_details('Method not implemented!')
        raise NotImplementedError('Method not implemented!')


def add_ChlorobotRPCServicer_to_server(servicer, server):
    rpc_method_handlers = {
            'Listen': grpc.unary_stream_rpc_method_handler(
                    servicer.Listen,
                    request_deserializer=chlorobot__rpc__pb2.ChlorobotAuthentication.FromString,
                    response_serializer=chlorobot__rpc__pb2.ChlorobotPacket.SerializeToString,
            ),
            'Send': grpc.unary_unary_rpc_method_handler(
                    servicer.Send,
                    request_deserializer=chlorobot__rpc__pb2.ChlorobotRequest.FromString,
                    response_serializer=chlorobot__rpc__pb2.ChlorobotAcknowledgement.SerializeToString,
            ),
    }
    generic_handler = grpc.method_handlers_generic_handler(
            'ChlorobotRPC', rpc_method_handlers)
    server.add_generic_rpc_handlers((generic_handler,))
    server.add_registered_method_handlers('ChlorobotRPC', rpc_method_handlers)


 # This class is part of an EXPERIMENTAL API.
class ChlorobotRPC(object):
    """A Chlorobot IRC RPC interface
    """

    @staticmethod
    def Listen(request,
            target,
            options=(),
            channel_credentials=None,
            call_credentials=None,
            insecure=False,
            compression=None,
            wait_for_ready=None,
            timeout=None,
            metadata=None):
        return grpc.experimental.unary_stream(
            request,
            target,
            '/ChlorobotRPC/Listen',
            chlorobot__rpc__pb2.ChlorobotAuthentication.SerializeToString,
            chlorobot__rpc__pb2.ChlorobotPacket.FromString,
            options,
            channel_credentials,
            insecure,
            call_credentials,
            compression,
            wait_for_ready,
            timeout,
            metadata,
            _registered_method=True)

    @staticmethod
    def Send(request,
            target,
            options=(),
            channel_credentials=None,
            call_credentials=None,
            insecure=False,
            compression=None,
            wait_for_ready=None,
            timeout=None,
            metadata=None):
        return grpc.experimental.unary_unary(
            request,
            target,
            '/ChlorobotRPC/Send',
            chlorobot__rpc__pb2.ChlorobotRequest.SerializeToString,
            chlorobot__rpc__pb2.ChlorobotAcknowledgement.FromString,
            options,
            channel_credentials,
            insecure,
            call_credentials,
            compression,
            wait_for_ready,
            timeout,
            metadata,
            _registered_method=True)
