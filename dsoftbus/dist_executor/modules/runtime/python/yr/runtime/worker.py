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
"""local mode worker"""
import concurrent.futures
import logging
import queue
import threading
import traceback

import yr
from yr import signature
from yr.exception import YRInvokeError, YRError
from yr.object_ref import ObjectRef
from yr.rpc import common_pb2
from yr.config import ConfigManager
from yr.rpc.core_service_pb2 import CreateResponse, InvokeResponse
from yr.runtime.local_object_store import LocalObjectStore
from yr.runtime.task_spec import InvokeType
from yr.runtime.task_spec import TaskSpec
from yr.serialization import SerializeObject, Serialization
from yr.utils import binary_to_hex

_logger = logging.getLogger(__name__)


class Worker(threading.Thread):
    """Local mode worker"""
    _instance = None
    _queue = None
    _running = False
    _instance_id = ''
    pool = None

    def init(self, instance_id: str, concurrency):
        """init worker and create instance"""
        self.pool = concurrent.futures.ThreadPoolExecutor(max_workers=concurrency, thread_name_prefix=instance_id)
        self._instance_id = instance_id
        self._queue = queue.Queue()
        self._running = True
        self.setDaemon(True)
        self.start()
        _logger.debug(f"init {self._instance_id} finished.")

    def run(self) -> None:
        """main loop"""
        while self._running:
            task = self._queue.get()
            if task is None:
                break
            future = self.pool.submit(self._execute, task)
            future.add_done_callback(_error_logging)

    def submit(self, task: TaskSpec):
        """submit a invoke task"""
        self._queue.put(task)

    def stop(self, timeout=None):
        """stop worker"""
        self._running = False
        self._queue.put(None)
        self.join(timeout)
        while not self._queue.empty():
            item = self._queue.get_nowait()
            if item is None:
                break
            item.callback(None, RuntimeError("worked is stopped."))
        self.pool.shutdown(wait=False)

    def _execute(self, task: TaskSpec):
        try:
            result = self._invoke_function(task)
        except Exception as err:
            if isinstance(err, YRInvokeError):
                result = YRInvokeError(err.cause, traceback.format_exc())
            else:
                result = YRInvokeError(err, traceback.format_exc())

        try:
            if task.invoke_type in (
                    InvokeType.CREATE_INSTANCE,
                    InvokeType.CREATE_NORMAL_FUNCTION_INSTANCE):
                if isinstance(result, YRError):
                    code = common_pb2.ERR_USER_FUNCTION_EXCEPTION
                    message = binary_to_hex(Serialization().serialize(result, "", ConfigManager.in_cluster).data)
                else:
                    code = common_pb2.ERR_NONE
                    message = ""
                task.callback(CreateResponse(code=code, message=message, instanceID=self._instance_id), None)
            else:
                object_id = task.object_ids[0]
                LocalObjectStore().put(object_id, result)
                task.callback(InvokeResponse(returnObjectID=object_id), None)
        except Exception as e:
            task.callback(None, YRInvokeError(e, traceback.format_exc()))
        _logger.debug(f"succeed to call, traceID: {task.task_id}")

    def _invoke_function(self, task: TaskSpec):
        task.args = _process_args(task.args[1:])
        args, kwargs = signature.recover_args(task.args)
        if task.invoke_type == InvokeType.CREATE_INSTANCE:
            return self._create_instance(task, *args, **kwargs)
        if task.invoke_type == InvokeType.INVOKE_MEMBER_FUNCTION:
            return self._instance_function(task, *args, **kwargs)
        if task.invoke_type == InvokeType.CREATE_NORMAL_FUNCTION_INSTANCE:
            return None
        return _normal_function(task, *args, **kwargs)

    def _instance_function(self, task: TaskSpec, *args, **kwargs):
        instance_function_name = task.object_descriptor.function_name
        return getattr(self._instance, instance_function_name)(*args, **kwargs)

    def _create_instance(self, task: TaskSpec, *args, **kwargs):
        code = LocalObjectStore().get(task.code_id)
        self._instance = code(*args, **kwargs)


def _error_logging(future: concurrent.futures.Future):
    if future.exception() is not None:
        _logger.exception(future.exception())


def _normal_function(task: TaskSpec, *args, **kwargs):
    code = LocalObjectStore().get(task.code_id)
    return code(*args, **kwargs)


def _process_args(args_list):
    def func(arg):
        if isinstance(arg, ObjectRef):
            return yr.get(arg)
        if isinstance(arg, SerializeObject):
            return arg.origin_object
        return arg

    return list(map(func, args_list))
