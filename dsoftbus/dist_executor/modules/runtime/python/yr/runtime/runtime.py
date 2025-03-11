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

"""
Runtime singleton
"""
import logging
from abc import ABCMeta, abstractmethod
from concurrent.futures import Future
from typing import Union, Callable, List
from urllib3.exceptions import HTTPError

from datasystem.stream_client import StreamClient
from yr import utils, serialization
from yr.common.response_helper import make_callback_for_kill
from yr.config import ConfigManager, DEFAULT_CONCURRENCY
from yr.exception import YRInvokeError
from yr.instance_ref import InstanceRef
from yr.invoke_client.invoke_client import InvokeClient
from yr.object_ref import ObjectRef
from yr.objref_counting_manager import ObjRefCountingManager
from yr.rpc.common_pb2 import Arg
from yr.runtime.dependency_manager import resolve_dependency
from yr.runtime.local_object_store import LocalObjectStore
from yr.runtime.task_manager import TaskManager
from yr.runtime.task_spec import Task, TaskSpec, InvokeType, Signal, TaskMetadata, callback_hook
from yr.serialization import Serialization
from yr.signature import recover_args
from yr.utils import Singleton

_logger = logging.getLogger(__name__)

_MAX_SERIAL_LENGTH = 10 ** 6


@Singleton
class Runtime:
    """
    Runtime for user actions
    """

    def __init__(self):
        self.__runtime = None

    @property
    def rt(self):
        """
        Get current runtime
        """
        return self.__runtime

    def init(self, runtime):
        """
        initialize runtime
        """
        self.__runtime = runtime


class BaseRuntime(metaclass=ABCMeta):
    """runtime interface"""

    def __init__(self, invoke_client: InvokeClient):
        self.task_mgr = TaskManager(invoke_client)
        self.__invoke_client = invoke_client

    @abstractmethod
    def put(self, ref: Union[str, ObjectRef], value, use_msgpack=False):
        """
        put object to datasystem with objectref
        """

    @abstractmethod
    def clear(self):
        """
        clean object in storage
        """

    @abstractmethod
    def get_trace_id(self) -> str:
        """get_trace_id"""

    @abstractmethod
    def get_stream_client(self) -> StreamClient:
        """
        get stream client for data system.
        """

    @abstractmethod
    def get_id(self, ref_id, timeout: int):
        """
        get object from datasystem with objectid
        """

    def get(self, object_ids: List[ObjectRef], timeout):
        """
        get object from datasystem
        """
        objects = self._get_object(object_ids, timeout)
        for obj in objects:
            if isinstance(obj, YRInvokeError):
                raise obj.origin_error()
        return objects

    def cancel(self, object_refs: List[ObjectRef]):
        """cancel the stateless invoke"""
        for object_ref in object_refs:
            self.task_mgr.cancel(task_id=object_ref.task_id)

    def submit_task(self, task: Task, callback: Callable):
        """submit normal function"""
        func_info = ConfigManager().get_function_info()
        if task.object_descriptor.target_language == utils.LANGUAGE_CPP:
            func_info.function_name = task.target_function_key
        args = self.__create_args(task)

        task_spec = TaskSpec(task_id=task.task_id,
                             job_id=ConfigManager().job_id,
                             object_descriptor=task.object_descriptor,
                             function_info=func_info,
                             code_id=task.code_id,
                             callback=callback_hook(callback, task.task_id, task.invoke_type,
                                                    object_counting_mgr=ObjRefCountingManager()),
                             invoke_type=task.invoke_type,
                             args=args,
                             trace_id=task.trace_id,
                             invoke_options=task.invoke_options,
                             instance_id=task.instance_id,
                             invoke_timeout=ConfigManager().invoke_timeout)
        for return_obj in task.return_obj_list:
            task_spec.object_ids.append(return_obj.id)

        def on_complete(instance_err, dependency_errors):
            if instance_err:
                callback(None, instance_err)
                return
            if len(dependency_errors) != 0:
                callback(None, RuntimeError(f"dependent task failed: {[str(e) for e in dependency_errors]}"))
                return
            task_spec.args = self._package_args_post(task_spec.args)
            self.task_mgr.submit_task(task_spec)

        resolve_dependency(task_spec, on_complete)

    def kill_instance(self, instance_id):
        """
        kill instance
        """
        if isinstance(instance_id, str):
            instance_id = InstanceRef(instance_id)
        func_info = ConfigManager().get_function_info()
        _logger.info(f"instance killed, instance_id: {instance_id}")
        future = Future()

        self.__invoke_client.kill(TaskSpec(task_id=utils.generate_task_id(),
                                           job_id=ConfigManager().job_id,
                                           invoke_type=InvokeType.KILL_INSTANCE,
                                           invoke_timeout=ConfigManager().invoke_timeout,
                                           function_info=func_info,
                                           callback=make_callback_for_kill(future, instance_id, Signal.KILL_INSTANCE),
                                           instance_id=instance_id,
                                           signal=Signal.KILL_INSTANCE.value))
        future.result()

    def kill_all_instances(self):
        """
        Kill all instances from job
        """
        # Not support on cloud.
        # That in_cluster is true means driver mode is being used,
        # instances will be killed when driver disconnection happens.
        if ConfigManager().on_cloud or ConfigManager().in_cluster:
            return
        job_id = ConfigManager().job_id
        future = Future()
        self.__invoke_client.kill(TaskSpec(task_id=utils.generate_task_id(),
                                           job_id=job_id,
                                           invoke_type=InvokeType.KILL_INSTANCE,
                                           invoke_timeout=ConfigManager().invoke_timeout,
                                           function_info=ConfigManager().get_function_info(),
                                           callback=make_callback_for_kill(future, job_id,
                                                                           Signal.KILL_ALL_INSTANCES),
                                           instance_id=InstanceRef(job_id),
                                           signal=Signal.KILL_ALL_INSTANCES.value))
        try:
            future.result()
        except (HTTPError, RuntimeError) as e:
            _logger.exception(e)

    @abstractmethod
    def _get_object(self, object_refs: list, timeout: int):
        """
        get object from request
        """

    def _package_python_args(self, args_list):
        """package python args"""
        args_list_new = []
        for arg in args_list:
            if isinstance(arg, ObjectRef):
                serialized_arg = serialization.Serialization().serialize(arg, arg.id, True)
            else:
                serialized_arg = serialization.Serialization().serialize(arg, "", True)
            if len(serialized_arg.data) >= _MAX_SERIAL_LENGTH:
                key = utils.generate_random_id()
                ref = self.put(key, serialized_arg)
                serialized_arg = serialization.Serialization().serialize(ref, ref.id, True)

            args_list_new.append(serialized_arg)
        return args_list_new

    def _package_args_post(self, args_list):
        args_list_new = []
        for arg in args_list:
            if isinstance(arg, Arg):
                args_list_new.append(arg)
                continue
            if not isinstance(arg, serialization.SerializeObject):
                continue
            for ref in arg.refs:
                self._put_local_object(ref)
            args_list_new.append(Arg(type=0, value=arg.data, nested_refs=[ref.id for ref in arg.refs]))
        return args_list_new

    def _put_local_object(self, ref):
        obj = LocalObjectStore().get(ref.id)
        if obj is not None:
            # if obj is in local, put it to data system
            self.put(ref.id, serialization.SerializeObject(object_type=serialization.OBJECT_TYPE_PYTHON, data=obj))
            LocalObjectStore().release(ref.id)

    def __create_args(self, task):
        if task.invoke_options:
            concurrency = task.invoke_options.concurrency
        else:
            concurrency = DEFAULT_CONCURRENCY
        metadata = \
            TaskMetadata.create(object_descriptor=task.object_descriptor,
                                code_id=task.code_id,
                                invoke_type=task.invoke_type,
                                concurrency=concurrency)
        meta_refs = [task.code_id] if task.code_id.startswith(utils.OBJECT_ID_PREFIX) else []
        args = [Arg(type=0, value=metadata.to_json().encode(), nested_refs=meta_refs)]
        if task.object_descriptor.target_language == utils.LANGUAGE_CPP:
            args += _package_cpp_args(task.args_list)
        else:
            args += self._package_python_args(task.args_list)
        return args


def _package_cpp_args(args_list):
    args_list_new = []
    args, _ = recover_args(args_list)
    for arg in args:
        serialized_arg = serialization.Serialization().serialize(arg, "", True, True)
        args_list_new.append(Arg(type=0, value=serialized_arg.data, nested_refs=[]))
    return args_list_new


def package_python_args(args_list):
    """package python args"""
    args_list_new = []
    for arg in args_list:
        if isinstance(arg, ObjectRef):
            args_list_new.append(arg)
            continue
        serialized_arg = Serialization().serialize(arg, "", True)
        args_list_new.append(serialized_arg)
    return args_list_new
