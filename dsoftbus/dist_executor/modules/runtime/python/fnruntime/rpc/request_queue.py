#!/usr/bin/env python3
# coding=UTF-8
# Copyright (c) 2022 Huawei Technologies Co., Ltd
#
# This software is licensed under Mulan PSL v2.
# You can use this software according to the terms
# and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#
# http://license.coscl.org.cn/MulanPSL2
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT
# WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.

"""request queue"""
import ctypes
import queue
import threading
import traceback
from collections.abc import Callable
from concurrent.futures import ThreadPoolExecutor
from dataclasses import dataclass

from fnruntime.common.utils import dependency_objref_process
from fnruntime import log
from fnruntime.rpc.sdk import RPClient, get_call_map
from fnruntime.rpc.inorder_manager import InOrderManager
from yr import utils
from yr.rpc import common_pb2 as common, runtime_rpc_pb2 as rt_rpc, runtime_service_pb2 as rt
from yr.rpc import core_service_pb2 as core
from yr.runtime.task_spec import Signal

# The request id length, split by "-"
_LEN_IN_ORDER_REQUEST_ID = 10

_MAX_RETRY_TIME = 3
_REQ_TABLE = {
    "callReq": "call",
    "checkpointReq": "checkpoint",
    "recoverReq": "recover",
    "shutdownReq": "shutdown",
    "signalReq": "signal"
}


@dataclass
class ReqData:
    """ReqData"""
    request: rt_rpc.StreamingMessage
    callback: Callable


def _send_call_ack(response: core.CallResult):
    """send call result req """
    rsp = None
    try:
        rsp = RPClient().send(
            rt_rpc.StreamingMessage(callResultReq=response, messageID=utils.generate_task_id()))
    except TimeoutError as e:
        log.get_logger().warning(f"failed to get callResultAck: {e}")
    if rsp.HasField("callResultAck") and rsp.callResultAck.code == 0:
        log.get_logger().debug(f"succeed to callResultAck: {rsp}")
    else:
        log.get_logger().warning(f"failed to get callResultAck: {rsp}")


def _get_concurrency(create_options):
    """get concurrency """
    pool_size = 100
    if 'Concurrency' in create_options:
        concurrency = int(create_options['Concurrency'])
        if 1 <= concurrency <= 1000:
            pool_size = concurrency
    return pool_size


class RuntimeThreadPoolExecutor(ThreadPoolExecutor):
    """
    Runtime thread pool
    """

    def __init__(self, max_workers=None, thread_name_prefix=""):
        super(RuntimeThreadPoolExecutor, self).__init__(max_workers=max_workers, thread_name_prefix=thread_name_prefix)
        self._task_thread = {}
        # cancel may be faster than call, should record it
        self._canceled = []

    def cancel_task(self, request_id: str) -> str:
        """
        Cancel task
        """
        thread = self._task_thread.pop(request_id, None)
        if thread is not None:
            log.get_logger().debug("found thread of requestID: %s", request_id)
            # discard threads from ThreadPoolExecutor
            self._threads.discard(thread)
            tid = ctypes.c_long(thread.ident)
            res = ctypes.pythonapi.PyThreadState_SetAsyncExc(
                tid, ctypes.py_object(KeyboardInterrupt))
            if res == 0:
                msg = f"failed to cancel: invalid tid: {tid}, requestID: {request_id}"
                log.get_logger().warning(msg)
            elif res != 1:
                ctypes.pythonapi.PyThreadState_SetAsyncExc(tid, None)
                msg = f"failed to cancel: PyThreadState_SetAsyncExc failed, requestID: {request_id}"
                log.get_logger().warning(msg)
            else:
                msg = f"success to cancel, thread: {thread}, requestID: {request_id}"
                log.get_logger().info(msg)

            sender_id = get_call_map().pop(request_id, None)
            if sender_id is not None:
                _send_call_ack(core.CallResult(
                    code=common.ERR_NONE,
                    message=msg,
                    instanceID=sender_id,
                    requestID=request_id))
            else:
                log.get_logger().error(
                    "failed to send call result: can't find sender_id, thread: {thread}, requestID: {request_id}",
                    thread, request_id)
        else:
            msg = f"failed to cancel,thread has been finished or cancel faster than invoke, requestID: {request_id}"
            self._canceled.append(request_id)
            log.get_logger().warning(msg)
        return msg

    def wrapper(self, fn: Callable, request_id: str, sender_id: str):
        """
        Wrapper process a task
        """
        call_map = get_call_map()
        thread = threading.currentThread()
        call_map[request_id] = sender_id
        self._task_thread[request_id] = thread
        try:
            fn()
        except KeyboardInterrupt:
            # When we cancel a thread, this throws an KeyboardInterrupt exception. This operation may cause a deadlock.
            # Such as logger, we avoid this by generating a new lock.
            for h in log.get_logger().logger.handler:
                h.createLock()
            log.get_logger().info(f"task has been canceled: {request_id}")
        finally:
            call_map.pop(request_id, None)
            self._task_thread.pop(request_id, None)

    def submit(self, *args, **kwargs):
        """
        submit a task to thread pool
        """
        fn = kwargs.pop("fn")
        request_id = kwargs.pop("request_id")
        sender_id = kwargs.pop("sender_id")
        # cancel may be faster than call, no need to execute, send ack directly
        if request_id in self._canceled:
            _send_call_ack(core.CallResult(
                code=common.ERR_NONE,
                message="success to cancel",
                instanceID=sender_id,
                requestID=request_id))
            self._canceled.remove(request_id)
            raise RuntimeError("cancel faster than call, call task cancel directly")

        return super(RuntimeThreadPoolExecutor, self).submit(
            self.wrapper, fn, request_id, sender_id)


class RequestManager(threading.Thread):
    """
    Runtime Request Queue, receive request from rpc server
    """

    def __init__(self, mgr):
        """init"""
        super().__init__(name="RequestManager")
        self._queue = queue.Queue()
        self._mgr = mgr
        self._inorder_manager = InOrderManager()
        self._lock = threading.Lock()
        self.invoke_pool = None
        self._running = True
        self.setDaemon(True)
        self.start()

    @staticmethod
    def check_in_order(req: rt.CallRequest):
        """check if the request needs to be processed with in order queue"""
        if req.isCreate:
            return False
        request_id_parts = req.requestID.split("-")
        if len(request_id_parts) == _LEN_IN_ORDER_REQUEST_ID:
            return True
        return False

    def cancel_thread(self, request: rt.SignalRequest) -> rt.SignalResponse:
        """
        cancel_thread
        """
        if self.invoke_pool is None:
            return rt.SignalResponse(code=common.ERR_INSTANCE_NOT_FOUND,
                                     message="instance should be created before canceling")

        if len(request.payload) == 0:
            log.get_logger().warnning(f"no request_id from payload: {request.payload}")
            return rt.SignalResponse(code=common.ERR_NONE,
                                     message=f"no request_id from payload: {request.payload}")
        request_id = request.payload.decode('utf-8')
        log.get_logger().debug(f"start to cancel task, requestID: {request_id}")
        message = self.invoke_pool.cancel_task(request_id)
        log.get_logger().debug(f"end to cancel task, requestID: {request_id}, message: {message}")
        return rt.SignalResponse(code=common.ERR_NONE, message=message)

    def init_invoke_pool(self, concurrency=100):
        """init invoke pool"""
        if self.invoke_pool is None:

            self.invoke_pool = RuntimeThreadPoolExecutor(max_workers=concurrency,
                                                         thread_name_prefix="invoke_pool")

    def run(self) -> None:
        """main loop"""
        while self._running:
            req_data = self._queue.get()
            if req_data is None:
                break
            try:
                self._process_request(req_data)
            except Exception as e:
                log.get_logger().warning(f"{e} {traceback.format_exc()}")
        log.get_logger().warning(f"RequestManager exit")

    def init_call_env(self, req: rt.CallRequest, req_id: str,
                      callback: Callable):
        """init call environment"""
        if req.isCreate:
            pool_size = _get_concurrency(req.createOptions)
            log.get_logger().debug(f"pool_size {pool_size}")
            self.init_invoke_pool(concurrency=pool_size)
        else:
            if self.invoke_pool is None:
                callback(rt_rpc.StreamingMessage(
                    callRsp=rt.CallResponse(code=common.ERR_INSTANCE_NOT_FOUND,
                                            message="instance should be "
                                                    "created before invoking"),
                    messageID=req_id))
                return

    def call(self, req: rt.CallRequest, req_id: str, callback: Callable):
        """call"""
        self.init_call_env(req, req_id, callback)

        need_in_order = self.check_in_order(req)
        if need_in_order:
            runtime_key = utils.extract_runtime_id(req.requestID)
            request_queue = \
                self._inorder_manager.get_instance_request_queue(runtime_key)

        def call_task(request=req, need_order=False):
            log.get_logger().debug(f"start to call "
                                   f"{request.requestID} {request.traceID}")
            response = self._mgr.call(request)
            if need_order:
                request_queue.set_next_serial_num()
            log.get_logger().debug(f"end to call "
                                   f"{request.requestID} {request.traceID}")
            _send_call_ack(response)

        def process_current():
            temp_serial_num = utils.extract_serial_num(req.requestID)
            if temp_serial_num != request_queue.next_serial_num():
                request_queue.store_request(temp_serial_num, req)
            else:
                call_task(req, True)

        def process_next(_):
            while not request_queue.empty():
                self._lock.acquire()
                if request_queue.head_serial_num() == \
                        request_queue.next_serial_num():
                    next_req = request_queue.pop_head_request()
                    self._lock.release()
                    call_task(next_req, True)
                else:
                    self._lock.release()
                    break

        call_ack = increase_objref(req)
        callback(rt_rpc.StreamingMessage(callRsp=call_ack, messageID=req_id))
        if call_ack.code == common.ERR_NONE:
            if req.isCreate:
                call_task()
            elif need_in_order:
                self.invoke_pool.submit(fn=process_current,
                                        request_id=req.requestID, sender_id=req.senderID). \
                    add_done_callback(process_next)
            else:
                self.invoke_pool.submit(fn=call_task,
                                        request_id=req.requestID, sender_id=req.senderID)

    def checkpoint(self, req: rt.CheckpointRequest, req_id: str, callback: Callable):
        """checkpoint"""
        resp = self._mgr.checkpoint(req)
        log.get_logger().debug(f"end to checkpoint {resp}")
        callback(rt_rpc.StreamingMessage(checkpointRsp=resp, messageID=req_id))

    def recover(self, req: rt.RecoverRequest, req_id: str, callback: Callable):
        """recover"""
        self.init_invoke_pool()
        resp = self._mgr.recover(req)
        callback(rt_rpc.StreamingMessage(recoverRsp=resp, messageID=req_id))

    def shutdown(self, req: rt.ShutdownRequest, req_id: str, callback: Callable):
        """shutdown"""
        resp = self._mgr.shutdown(req)
        callback(rt_rpc.StreamingMessage(shutdownRsp=resp, messageID=req_id))

    def signal(self, req: rt.SignalRequest, req_id: str, callback: Callable):
        """signal"""
        if req.signal == Signal.CANCEL.value:
            resp = self.cancel_thread(req)
        else:
            resp = self._mgr.signal(req)
        callback(rt_rpc.StreamingMessage(signalRsp=resp, messageID=req_id))

    def submit(self, request: rt_rpc.StreamingMessage, callback: Callable):
        """submit a request"""
        self._queue.put(ReqData(request, callback))

    def clear(self, timeout=None):
        """ clear """
        self._running = False
        self._queue.put(None)
        self.join(timeout)
        self.invoke_pool.shutdown(wait=False)
        self._inorder_manager.clear()

    def _process_request(self, req_data):
        """process request"""
        request = req_data.request
        req_name = request.WhichOneof("body")
        try:
            req_attr = getattr(request, req_name)
        except AttributeError:
            log.get_logger().error(f"invalid request: {request}")
        else:
            func_name = _REQ_TABLE.get(req_name, None)
            try:
                func = getattr(self, func_name)
            except AttributeError:
                log.get_logger().error(f"can't find request from request table: {request}")
            else:
                func(req_attr, request.messageID, req_data.callback)


def increase_objref(request: rt.CallRequest):
    """preprocess for call request """
    code = common.ERR_NONE
    message = None
    try:
        dependency_objref_process(request.args, is_increase=True)
    except TimeoutError as err:
        code = common.ERR_INNER_SYSTEM_ERROR
        message = f"failed to increase obj ref count of request: {request.requestID}, err: {err}"
        log.get_logger().error(message)
    return rt.CallResponse(code=code, message=message)
