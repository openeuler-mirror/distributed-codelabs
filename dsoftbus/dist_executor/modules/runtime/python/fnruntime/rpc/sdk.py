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

"""RPC client"""
import os
import threading
import traceback
from concurrent.futures import ThreadPoolExecutor, Future

from datasystem.object_cache import ObjectClient
from datasystem.stream_client import StreamClient
from fnruntime import log
from yr.exception import YRInvokeError
from yr.objref_counting_manager import ObjRefCountingManager
from yr.rpc import common_pb2
from yr.rpc import core_service_pb2 as core
from yr.rpc import runtime_rpc_pb2 as rt_rpc
from yr.rpc import runtime_service_pb2 as rt
from yr.storage.ds_client import DSClient
from yr.utils import Singleton
from yr.utils import generate_task_id

_CALL_MAP = {}
_DEFAULT_BUS_PORT = "22769"
MAX_RETRY_TIME = 3
REQUEST_TIMEOUT = 5


def post_shutdown():
    """process request after shutdown"""
    for request_id in list(_CALL_MAP.keys()):
        sender_id = _CALL_MAP.get(request_id)
        if sender_id is None:
            continue
        log.get_logger().warning(f"processing request will be cancel, {request_id}")
        _ = RPClient().send(rt_rpc.StreamingMessage(callResultReq=core.CallResult(
            code=common_pb2.ERR_USER_FUNCTION_EXCEPTION,
            message="This instance has been shutdown",
            instanceID=sender_id,
            requestID=request_id), messageID=generate_task_id()))


def get_call_map():
    """get call map"""
    return _CALL_MAP


@Singleton
class RPClient:
    """RPC client"""

    def __init__(self):
        self.ds_client = None
        self.instance_id = os.environ.get("INSTANCE_ID")
        self._invoke_callback = {}
        self.ds_address = ''
        self._response_futures = {}
        self.queue = None
        self._pool = ThreadPoolExecutor(max_workers=1, thread_name_prefix="rpc_client")
        # Preserve the record of the first thread created by self._pool,
        # which would die out as a daemon thread, in order to block driver
        # stop thread and stop driver server as it's join() func is executed.
        thread_id = self._pool.submit(threading.get_ident).result()
        self.init_thread = threading._active.get(thread_id)

    def init_ds_client(self, ds_address: str):
        """Init dataSystem client"""
        self.ds_address = ds_address
        self.ds_client = DSClient()
        host, port = ds_address.split(":")
        client = ObjectClient(host=host, port=int(port), connect_timeout=5000)
        client.init()
        stream_client = StreamClient(host=host, port=int(port))
        stream_client.init()
        self.ds_client.init(client, stream_client)

    def send(self, request: rt_rpc.StreamingMessage, timeout=REQUEST_TIMEOUT) -> rt_rpc.StreamingMessage:
        """send request to core"""
        for _ in range(MAX_RETRY_TIME):
            future = Future()
            self._response_futures[request.messageID] = future
            self.queue.put(request)
            try:
                result = future.result(timeout=timeout)
            except TimeoutError:
                continue
            return result
        self._response_futures.pop(request.messageID)
        raise TimeoutError(f"failed to send request after {MAX_RETRY_TIME} time retry")

    def receive(self, response: rt_rpc.StreamingMessage):
        """receive response from core"""
        future = self._response_futures.get(response.messageID)
        if future is not None:
            future.set_result(response)
            self._response_futures.pop(response.messageID)
        else:
            log.get_logger().warning(f"response not found future, {response}")

    def register_invoke_callback(self, request_id: str, callback: Future):
        """
        register invoke callback
        callback must be a future because notify may be earlier than response
        """
        self._invoke_callback[request_id] = callback

    def unregister_invoke_callback(self, request_id: str):
        """
        unregister invoke callback
        """
        if request_id in self._invoke_callback:
            self._invoke_callback.pop(request_id)

    def notify(self, request: rt.NotifyRequest):
        """callback for invoke request"""
        # callback is a future, because NotifyRequest need wait for the corresponding response
        callback = self._invoke_callback.get(request.requestID)
        if callback is not None:
            self._pool.submit(lambda: callback.result()(request))
        else:
            log.get_logger().warning(f"callback not found, {request}")


def create(request: core.CreateRequest, callback, timeout=None):
    """
    Create instance request

    Args:
        request: create request
        callback: process response
        timeout: request timeout, default 900s

    Returns:
        str: instance id
    """
    log.get_logger().debug(f"start to create: {request}")
    future = Future()
    RPClient().register_invoke_callback(request.requestID, future)
    response = RPClient().send(rt_rpc.StreamingMessage(createReq=request, messageID=generate_task_id()), timeout)

    if not response.HasField("createRsp"):
        callback(None, RuntimeError(f"error response, {response}"))
    else:
        if response.createRsp.code != common_pb2.ERR_NONE:
            RPClient().unregister_invoke_callback(request.requestID)
            callback(response.createRsp)
            return

        def _notify(notify):
            try:
                callback(core.CreateResponse(code=notify.code, message=notify.message,
                                             instanceID=response.createRsp.instanceID))
            except (ValueError, AttributeError, RuntimeError) as e:
                callback("", YRInvokeError(e, traceback.format_exc()))

        future.set_result(_notify)


def invoke(request: core.InvokeRequest, callback, timeout=None):
    """
    send a invoke request to instance

    Args:
        request: invoke request
        callback: process response
        timeout: request timeout, default 900s

    Returns:
        str: object id
        asyncio.Future: future
    """
    log.get_logger().debug(f"start to invoke {request}")
    future = Future()
    RPClient().register_invoke_callback(request.requestID, future)
    response = RPClient().send(rt_rpc.StreamingMessage(invokeReq=request, messageID=generate_task_id()), timeout)

    if not response.HasField("invokeRsp"):
        callback(None, RuntimeError(f"error response, {response}"))
    else:
        if response.invokeRsp.code != common_pb2.ERR_NONE:
            RPClient().unregister_invoke_callback(request.requestID)
            callback(response.invokeRsp)
            return

        def _notify(notify):
            callback(core.InvokeResponse(code=notify.code, message=notify.message,
                                         returnObjectID=response.invokeRsp.returnObjectID))

        future.set_result(_notify)


def kill(instance_id: str, sig: int, request_id: str, callback, timeout=None):
    """
    kill instance with signal
    """
    kill_req = core.KillRequest(instanceID=instance_id, signal=sig, payload=request_id.encode(encoding='utf-8'))
    response = RPClient().send(rt_rpc.StreamingMessage(killReq=kill_req,
                                                       messageID=generate_task_id()), timeout)
    if not response.HasField("killRsp"):
        callback(None, RuntimeError(f"error response, {response}"))
    else:
        callback(response.killRsp)


def exit(timeout=None):
    """
    kill instance with signal
    """
    ObjRefCountingManager().wait_invoke_ack()
    try:
        _ = RPClient().send(rt_rpc.StreamingMessage(exitReq=core.ExitRequest(), messageID=generate_task_id()), timeout)
    except TimeoutError as e:
        log.get_logger().exception(str(e))


def save_state(state: bytes, timeout=None) -> str:
    """
    save the instance state to cluster

    Args:
        state: state info
        timeout: request timeout, default 900s

    Returns:
        str: checkpoint id,use to load state
    """
    response = RPClient().send(
        rt_rpc.StreamingMessage(saveReq=core.StateSaveRequest(state=state), messageID=generate_task_id()), timeout)
    if not response.HasField("saveRsp"):
        raise RuntimeError(f"error response, {response}")
    if response.saveRsp.code != common_pb2.ErrorCode.ERR_NONE:
        raise RuntimeError(
            f"failed to save state, code: {response.saveRsp.code}, message: {response.saveRsp.message}")
    return response.saveRsp.checkpointID


def load_state(checkpoint_id: str, timeout=None) -> bytes:
    """
    load a instance state from cluster

    Args:
        checkpoint_id: instance checkpoint id
        timeout: request timeout, default 900s

    Returns:
        bytes: state
    """
    response = RPClient().send(
        rt_rpc.StreamingMessage(loadReq=core.StateLoadRequest(checkpointID=checkpoint_id),
                                messageID=generate_task_id()),
        timeout)
    if not response.HasField("loadRsp"):
        raise RuntimeError(f"error response, {response}")
    if response.loadRsp.code != common_pb2.ErrorCode.ERR_NONE:
        raise RuntimeError(
            f"failed to load state, code: {response.loadRsp.code}, message: {response.loadRsp.message}")
    return response.loadRsp.state
