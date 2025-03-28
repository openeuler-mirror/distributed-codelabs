# Generated by the gRPC Python protocol compiler plugin. DO NOT EDIT!
"""Client and server classes corresponding to protobuf-defined services."""
import grpc

from yr.rpc import runtime_service_pb2 as rpc_dot_runtime__service__pb2


class RuntimeServiceStub(object):
    """Runtime service provides APIs to core,
    """

    def __init__(self, channel):
        """Constructor.

        Args:
            channel: A grpc.Channel.
        """
        self.Call = channel.unary_unary(
                '/runtime_service.RuntimeService/Call',
                request_serializer=rpc_dot_runtime__service__pb2.CallRequest.SerializeToString,
                response_deserializer=rpc_dot_runtime__service__pb2.CallResponse.FromString,
                )
        self.NotifyResult = channel.unary_unary(
                '/runtime_service.RuntimeService/NotifyResult',
                request_serializer=rpc_dot_runtime__service__pb2.NotifyRequest.SerializeToString,
                response_deserializer=rpc_dot_runtime__service__pb2.NotifyResponse.FromString,
                )
        self.Checkpoint = channel.unary_unary(
                '/runtime_service.RuntimeService/Checkpoint',
                request_serializer=rpc_dot_runtime__service__pb2.CheckpointRequest.SerializeToString,
                response_deserializer=rpc_dot_runtime__service__pb2.CheckpointResponse.FromString,
                )
        self.Recover = channel.unary_unary(
                '/runtime_service.RuntimeService/Recover',
                request_serializer=rpc_dot_runtime__service__pb2.RecoverRequest.SerializeToString,
                response_deserializer=rpc_dot_runtime__service__pb2.RecoverResponse.FromString,
                )
        self.GracefulExit = channel.unary_unary(
                '/runtime_service.RuntimeService/GracefulExit',
                request_serializer=rpc_dot_runtime__service__pb2.GracefulExitRequest.SerializeToString,
                response_deserializer=rpc_dot_runtime__service__pb2.GracefulExitResponse.FromString,
                )
        self.Shutdown = channel.unary_unary(
                '/runtime_service.RuntimeService/Shutdown',
                request_serializer=rpc_dot_runtime__service__pb2.ShutdownRequest.SerializeToString,
                response_deserializer=rpc_dot_runtime__service__pb2.ShutdownResponse.FromString,
                )
        self.Heartbeat = channel.unary_unary(
                '/runtime_service.RuntimeService/Heartbeat',
                request_serializer=rpc_dot_runtime__service__pb2.HeartbeatRequest.SerializeToString,
                response_deserializer=rpc_dot_runtime__service__pb2.HeartbeatResponse.FromString,
                )
        self.Signal = channel.unary_unary(
                '/runtime_service.RuntimeService/Signal',
                request_serializer=rpc_dot_runtime__service__pb2.SignalRequest.SerializeToString,
                response_deserializer=rpc_dot_runtime__service__pb2.SignalResponse.FromString,
                )


class RuntimeServiceServicer(object):
    """Runtime service provides APIs to core,
    """

    def Call(self, request, context):
        """Call a method or init state of instance
        """
        context.set_code(grpc.StatusCode.UNIMPLEMENTED)
        context.set_details('Method not implemented!')
        raise NotImplementedError('Method not implemented!')

    def NotifyResult(self, request, context):
        """NotifyResult is applied to async notify result of create or invoke request invoked by runtime
        """
        context.set_code(grpc.StatusCode.UNIMPLEMENTED)
        context.set_details('Method not implemented!')
        raise NotImplementedError('Method not implemented!')

    def Checkpoint(self, request, context):
        """Checkpoint request a state to save for failure recovery and state migration
        """
        context.set_code(grpc.StatusCode.UNIMPLEMENTED)
        context.set_details('Method not implemented!')
        raise NotImplementedError('Method not implemented!')

    def Recover(self, request, context):
        """Recover state
        """
        context.set_code(grpc.StatusCode.UNIMPLEMENTED)
        context.set_details('Method not implemented!')
        raise NotImplementedError('Method not implemented!')

    def GracefulExit(self, request, context):
        """GracefulExit request an instance graceful exit
        """
        context.set_code(grpc.StatusCode.UNIMPLEMENTED)
        context.set_details('Method not implemented!')
        raise NotImplementedError('Method not implemented!')

    def Shutdown(self, request, context):
        """Shutdown request an instance shutdown
        """
        context.set_code(grpc.StatusCode.UNIMPLEMENTED)
        context.set_details('Method not implemented!')
        raise NotImplementedError('Method not implemented!')

    def Heartbeat(self, request, context):
        """check whether the runtime is alive
        """
        context.set_code(grpc.StatusCode.UNIMPLEMENTED)
        context.set_details('Method not implemented!')
        raise NotImplementedError('Method not implemented!')

    def Signal(self, request, context):
        """Signal the signal to instance
        """
        context.set_code(grpc.StatusCode.UNIMPLEMENTED)
        context.set_details('Method not implemented!')
        raise NotImplementedError('Method not implemented!')


def add_RuntimeServiceServicer_to_server(servicer, server):
    rpc_method_handlers = {
            'Call': grpc.unary_unary_rpc_method_handler(
                    servicer.Call,
                    request_deserializer=rpc_dot_runtime__service__pb2.CallRequest.FromString,
                    response_serializer=rpc_dot_runtime__service__pb2.CallResponse.SerializeToString,
            ),
            'NotifyResult': grpc.unary_unary_rpc_method_handler(
                    servicer.NotifyResult,
                    request_deserializer=rpc_dot_runtime__service__pb2.NotifyRequest.FromString,
                    response_serializer=rpc_dot_runtime__service__pb2.NotifyResponse.SerializeToString,
            ),
            'Checkpoint': grpc.unary_unary_rpc_method_handler(
                    servicer.Checkpoint,
                    request_deserializer=rpc_dot_runtime__service__pb2.CheckpointRequest.FromString,
                    response_serializer=rpc_dot_runtime__service__pb2.CheckpointResponse.SerializeToString,
            ),
            'Recover': grpc.unary_unary_rpc_method_handler(
                    servicer.Recover,
                    request_deserializer=rpc_dot_runtime__service__pb2.RecoverRequest.FromString,
                    response_serializer=rpc_dot_runtime__service__pb2.RecoverResponse.SerializeToString,
            ),
            'GracefulExit': grpc.unary_unary_rpc_method_handler(
                    servicer.GracefulExit,
                    request_deserializer=rpc_dot_runtime__service__pb2.GracefulExitRequest.FromString,
                    response_serializer=rpc_dot_runtime__service__pb2.GracefulExitResponse.SerializeToString,
            ),
            'Shutdown': grpc.unary_unary_rpc_method_handler(
                    servicer.Shutdown,
                    request_deserializer=rpc_dot_runtime__service__pb2.ShutdownRequest.FromString,
                    response_serializer=rpc_dot_runtime__service__pb2.ShutdownResponse.SerializeToString,
            ),
            'Heartbeat': grpc.unary_unary_rpc_method_handler(
                    servicer.Heartbeat,
                    request_deserializer=rpc_dot_runtime__service__pb2.HeartbeatRequest.FromString,
                    response_serializer=rpc_dot_runtime__service__pb2.HeartbeatResponse.SerializeToString,
            ),
            'Signal': grpc.unary_unary_rpc_method_handler(
                    servicer.Signal,
                    request_deserializer=rpc_dot_runtime__service__pb2.SignalRequest.FromString,
                    response_serializer=rpc_dot_runtime__service__pb2.SignalResponse.SerializeToString,
            ),
    }
    generic_handler = grpc.method_handlers_generic_handler(
            'runtime_service.RuntimeService', rpc_method_handlers)
    server.add_generic_rpc_handlers((generic_handler,))


 # This class is part of an EXPERIMENTAL API.
class RuntimeService(object):
    """Runtime service provides APIs to core,
    """

    @staticmethod
    def Call(request,
            target,
            options=(),
            channel_credentials=None,
            call_credentials=None,
            insecure=False,
            compression=None,
            wait_for_ready=None,
            timeout=None,
            metadata=None):
        return grpc.experimental.unary_unary(request, target, '/runtime_service.RuntimeService/Call',
            rpc_dot_runtime__service__pb2.CallRequest.SerializeToString,
            rpc_dot_runtime__service__pb2.CallResponse.FromString,
            options, channel_credentials,
            insecure, call_credentials, compression, wait_for_ready, timeout, metadata)

    @staticmethod
    def NotifyResult(request,
            target,
            options=(),
            channel_credentials=None,
            call_credentials=None,
            insecure=False,
            compression=None,
            wait_for_ready=None,
            timeout=None,
            metadata=None):
        return grpc.experimental.unary_unary(request, target, '/runtime_service.RuntimeService/NotifyResult',
            rpc_dot_runtime__service__pb2.NotifyRequest.SerializeToString,
            rpc_dot_runtime__service__pb2.NotifyResponse.FromString,
            options, channel_credentials,
            insecure, call_credentials, compression, wait_for_ready, timeout, metadata)

    @staticmethod
    def Checkpoint(request,
            target,
            options=(),
            channel_credentials=None,
            call_credentials=None,
            insecure=False,
            compression=None,
            wait_for_ready=None,
            timeout=None,
            metadata=None):
        return grpc.experimental.unary_unary(request, target, '/runtime_service.RuntimeService/Checkpoint',
            rpc_dot_runtime__service__pb2.CheckpointRequest.SerializeToString,
            rpc_dot_runtime__service__pb2.CheckpointResponse.FromString,
            options, channel_credentials,
            insecure, call_credentials, compression, wait_for_ready, timeout, metadata)

    @staticmethod
    def Recover(request,
            target,
            options=(),
            channel_credentials=None,
            call_credentials=None,
            insecure=False,
            compression=None,
            wait_for_ready=None,
            timeout=None,
            metadata=None):
        return grpc.experimental.unary_unary(request, target, '/runtime_service.RuntimeService/Recover',
            rpc_dot_runtime__service__pb2.RecoverRequest.SerializeToString,
            rpc_dot_runtime__service__pb2.RecoverResponse.FromString,
            options, channel_credentials,
            insecure, call_credentials, compression, wait_for_ready, timeout, metadata)

    @staticmethod
    def GracefulExit(request,
            target,
            options=(),
            channel_credentials=None,
            call_credentials=None,
            insecure=False,
            compression=None,
            wait_for_ready=None,
            timeout=None,
            metadata=None):
        return grpc.experimental.unary_unary(request, target, '/runtime_service.RuntimeService/GracefulExit',
            rpc_dot_runtime__service__pb2.GracefulExitRequest.SerializeToString,
            rpc_dot_runtime__service__pb2.GracefulExitResponse.FromString,
            options, channel_credentials,
            insecure, call_credentials, compression, wait_for_ready, timeout, metadata)

    @staticmethod
    def Shutdown(request,
            target,
            options=(),
            channel_credentials=None,
            call_credentials=None,
            insecure=False,
            compression=None,
            wait_for_ready=None,
            timeout=None,
            metadata=None):
        return grpc.experimental.unary_unary(request, target, '/runtime_service.RuntimeService/Shutdown',
            rpc_dot_runtime__service__pb2.ShutdownRequest.SerializeToString,
            rpc_dot_runtime__service__pb2.ShutdownResponse.FromString,
            options, channel_credentials,
            insecure, call_credentials, compression, wait_for_ready, timeout, metadata)

    @staticmethod
    def Heartbeat(request,
            target,
            options=(),
            channel_credentials=None,
            call_credentials=None,
            insecure=False,
            compression=None,
            wait_for_ready=None,
            timeout=None,
            metadata=None):
        return grpc.experimental.unary_unary(request, target, '/runtime_service.RuntimeService/Heartbeat',
            rpc_dot_runtime__service__pb2.HeartbeatRequest.SerializeToString,
            rpc_dot_runtime__service__pb2.HeartbeatResponse.FromString,
            options, channel_credentials,
            insecure, call_credentials, compression, wait_for_ready, timeout, metadata)

    @staticmethod
    def Signal(request,
            target,
            options=(),
            channel_credentials=None,
            call_credentials=None,
            insecure=False,
            compression=None,
            wait_for_ready=None,
            timeout=None,
            metadata=None):
        return grpc.experimental.unary_unary(request, target, '/runtime_service.RuntimeService/Signal',
            rpc_dot_runtime__service__pb2.SignalRequest.SerializeToString,
            rpc_dot_runtime__service__pb2.SignalResponse.FromString,
            options, channel_credentials,
            insecure, call_credentials, compression, wait_for_ready, timeout, metadata)
