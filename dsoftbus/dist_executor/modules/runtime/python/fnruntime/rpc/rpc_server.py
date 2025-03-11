#!/usr/bin/env python3
# coding=UTF-8
# Copyright (c) 2022 Huawei Technologies Co., Ltd
#
# This software is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#
# http://license.coscl.org.cn/MulanPSL2
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.

"""RPC Server"""
import queue
import traceback
import logging
import os
from concurrent import futures
from threading import Thread

import grpc

from fnruntime import log
from fnruntime.rpc.function_manager import FunctionManager, get_hooks_key
from fnruntime.rpc.request_queue import RequestManager
from fnruntime.rpc.sdk import RPClient
from fnruntime.util import get_tls_config
from yr.rpc import runtime_rpc_pb2 as rt_rpc
from yr.rpc import runtime_rpc_pb2_grpc as rt_rpc_grpc
from yr.rpc import runtime_service_pb2 as rt

MAX_MESSAGE_SIZE = 1024 * 1024 * 500
MAX_FRAME_SIZE = 1024 * 1024 * 2
MAX_CHUNK_SIZE = MAX_FRAME_SIZE

_STOPPING = False


class RuntimeService(rt_rpc_grpc.RuntimeRPCServicer):
    """Runtime rpc server"""
    __RESPONSE_FIELD = {"createRsp", "invokeRsp", "exitRsp", "saveRsp", "loadRsp", "killRsp", "callResultAck"}
    __REQUEST_FIELD = {"callReq", "checkpointReq", "recoverReq", "shutdownReq", "signalReq"}

    def __init__(self, task_queue=None):
        self._queue = queue.Queue()
        RPClient().queue = self._queue
        self._task_queue = task_queue

    def MessageStream(self, request_iterator, context):
        """streaming"""
        t = Thread(target=self._receive_request, args=(request_iterator,), name="streaming", daemon=True)
        t.start()

        while t.is_alive():
            try:
                request = self._queue.get(timeout=1)
            except queue.Empty:
                continue
            body_type = request.WhichOneof("body")
            if log.get_logger().isEnabledFor(logging.DEBUG) and body_type != "heartbeatRsp":
                log.get_logger().debug(f"send request: {body_type} {request.messageID}")
            yield request
        log.get_logger().info('end to streaming')

    def send_response(self, resp: rt_rpc.StreamingMessage):
        """send response by streaming"""
        self._queue.put(resp)

    def _receive_request(self, request_iterator):
        try:
            for request in request_iterator:
                self._process_request(request)
        except StopIteration:
            log.get_logger().warning('bus connection broke')
        except grpc.RpcError as e:
            global _STOPPING
            if not _STOPPING:
                log.get_logger().warning('rpc error')
                log.get_logger().exception(e)

    def _process_request(self, request: rt_rpc.StreamingMessage):
        body_type = request.WhichOneof("body")
        if log.get_logger().isEnabledFor(logging.DEBUG) and body_type != "heartbeatReq":
            log.get_logger().debug(f"receive request: {body_type} {request.messageID}")
        if body_type in self.__RESPONSE_FIELD:
            RPClient().receive(request)
        elif body_type == "notifyReq":
            RPClient().notify(request.notifyReq)
            self.send_response(rt_rpc.StreamingMessage(notifyRsp=rt.NotifyResponse(), messageID=request.messageID))
        elif body_type == "heartbeatReq":
            if self._task_queue.is_alive():
                self.send_response(
                    rt_rpc.StreamingMessage(heartbeatRsp=rt.HeartbeatResponse(), messageID=request.messageID))
        elif body_type in self.__REQUEST_FIELD:
            self._task_queue.submit(request, self.send_response)
        else:
            log.get_logger().warning(f"request invalid: {request}")


def _init_ds_client(mgr):
    ds_address = os.environ.get("DATASYSTEM_ADDR")
    log.get_logger().info(f"data-system worker addr: {ds_address}")
    try:
        RPClient().init_ds_client(ds_address)
    except Exception as e:
        mgr.set_init_exception(e, traceback.format_exc())
        log.get_logger().error(f"failed to init runtime, {e}, {traceback.format_exc()}")
        return
    log.get_logger().debug(f"succeed to init runtime, instanceID: {RPClient().instance_id}")
    return


def _init_function_manager(python_path, init_hooks=True):

    code_dir = os.environ.get("FUNCTION_LIB_PATH")
    instance_id = os.environ.get("INSTANCE_ID")
    layer_paths = os.environ.get("LAYER_LIB_PATH", "").split(",")
    hooks_key = get_hooks_key()
    hooks = {}
    for hook_name in hooks_key:
        if hook_name in os.environ:
            hooks[hook_name] = os.environ[hook_name]
    mgr = FunctionManager(instance_id, init_hooks)

    mgr.init(python_path, code_dir, layer_paths, hooks)
    return mgr


def serve(address, python_path, future=None) -> None:
    """
    start grpc server

    Args:
        address: bind address
        python_path: user code path
        future: to tell main thread that grpc server startup complete
    """
    log.get_logger().info("Starting server on %s", address)
    options = [
        ('grpc.max_receive_message_length', MAX_MESSAGE_SIZE),
        ('grpc.max_send_message_length', MAX_MESSAGE_SIZE),
        ('grpc.http2.max_frame_size', MAX_FRAME_SIZE),
        ('grpc.experimental.tcp_read_chunk_size', MAX_CHUNK_SIZE),
    ]
    mgr = _init_function_manager(python_path)
    task_queue = RequestManager(mgr)
    _init_ds_client(mgr)
    try:
        tls_enable, tls_config = get_tls_config()
    except Exception:
        tls_enable, tls_config = False, None

    server = grpc.server(futures.ThreadPoolExecutor(max_workers=100, thread_name_prefix="grpc_server"), options=options)
    if tls_enable:
        creds = grpc.ssl_server_credentials([(tls_config.module_key_data, tls_config.module_cert_data)],
                                            root_certificates=tls_config.root_cert_data)
        server.add_secure_port(address=address, server_credentials=creds)
        log.get_logger().info("Succeed to load tls config on adder:%s", address)
    else:
        server.add_insecure_port(address)
    rt_server = RuntimeService(task_queue)
    rt_rpc_grpc.add_RuntimeRPCServicer_to_server(rt_server, server)
    server.start()
    log.get_logger().info("Starting server successful.")
    if future is not None:
        future.set_result("")
    server.wait_for_termination()


def driver_serve(address) -> int:
    """
    start driver mode grpc server

    Args:
        address: bind address
    """
    options = [
        ('grpc.max_receive_message_length', MAX_MESSAGE_SIZE),
        ('grpc.max_send_message_length', MAX_MESSAGE_SIZE),
        ('grpc.http2.max_frame_size', MAX_FRAME_SIZE),
        ('grpc.experimental.tcp_read_chunk_size', MAX_CHUNK_SIZE),
    ]
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10, thread_name_prefix="grpc_server"), options=options)
    mgr = _init_function_manager("", init_hooks=False)
    task_queue = RequestManager(mgr)
    rt_server = RuntimeService(task_queue)
    rt_rpc_grpc.add_RuntimeRPCServicer_to_server(rt_server, server)
    port = server.add_insecure_port(address)
    Thread(target=_internal_start, args=(server,), name="_internal_start", daemon=True).start()
    Thread(target=_driver_stop, args=(server,), name="driver_stop", daemon=True).start()
    return port


def _internal_start(server):
    global _STOPPING
    _STOPPING = False

    server.start()
    server.wait_for_termination()


def _driver_stop(server):
    """
    stop driver mode gracefully

    Args:
        server: server to stop
    """
    RPClient().init_thread.join()
    global _STOPPING
    _STOPPING = True
    server.stop(None).wait()
