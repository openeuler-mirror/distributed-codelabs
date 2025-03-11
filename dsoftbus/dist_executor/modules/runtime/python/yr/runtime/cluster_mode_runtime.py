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
Cluster mode runtime
"""
import logging
import threading
import time
from abc import ABC
from concurrent import futures
from typing import Union, List
from inspect import isfunction

from datasystem.stream_client import StreamClient

import yr
from yr import serialization
from yr import utils
from yr.config import ConfigManager
from yr.invoke_client.invoke_client import InvokeClient
from yr.object_ref import ObjectRef
from yr.runtime.local_object_store import LocalObjectStore
from yr.runtime.runtime import BaseRuntime

_PUT_RECORD_TAG = "put.record.list"
_logger = logging.getLogger(__name__)


class ClusterModeRuntime(BaseRuntime, ABC):
    """
    Cluster mode runtime

    Attributes:
        ds_client: datasystem client
    """

    def __init__(self, ds_client, invoke_client: InvokeClient):
        """
        initialize cluster mode runtime

        Args:
            ds_client (DSClient): datasystem client
        """
        super().__init__(invoke_client)
        self.__ds_client = ds_client
        self.__invoke_client = invoke_client
        self.__future_map = {}
        self.__local_storage = {}
        self.__return_id_map = {}
        self.__thread_local = threading.local()
        self.__user_serial_manager = UserSerialManager()

    def get_id(self, ref_id, timeout: int):
        return self.__ds_client.get(ref_id, timeout)

    def put(self, ref: Union[str, ObjectRef], value, use_msgpack=False):
        """
        put object to datasystem with objectref
        """
        optimize = False
        if not use_msgpack and ConfigManager().in_cluster and not isinstance(value, serialization.SerializeObject) \
                and not isfunction(value):
            optimize = True
        ref_id = ref.id if isinstance(ref, ObjectRef) else ref

        if not isinstance(value, serialization.SerializeObject):
            value = serialization.Serialization().serialize(value, ref_id, optimize, use_msgpack)
        nested_refs = []
        for dependency_ref in value.refs:
            if isinstance(dependency_ref, ObjectRef):
                dependency_ref.wait()
                nested_refs.append(dependency_ref.id)

        if isinstance(ref, str):
            ref = ObjectRef(ref)

        if optimize:
            self.__ds_client.create(ref.id, nested_refs, value.writer, value.inband)
        else:
            self.__ds_client.put(ref.id, value.data, nested_refs)
        return ref

    def clear(self):
        """
        clean object in storage
        """
        self.task_mgr.clear()
        # the serial number of a stateful order queue needs to be reset.
        self.__user_serial_manager.clear()
        self.kill_all_instances()
        self.__invoke_client.clear()
        self.__ds_client.clear_ctx_reference()

    def exit_instance(self):
        """
        Exit instance
        """
        _logger.info("start to exit a function")
        self.__invoke_client.exit()

    def get_trace_id(self):
        """
        Get traceID
        """
        if yr.is_on_cloud():
            if hasattr(self.__thread_local, "trace_id"):
                return self.__thread_local.trace_id
            trace_id = utils.generate_trace_id(ConfigManager().job_id)
            _logger.warning(f"generate a new trace_id:{trace_id}")
            return trace_id
        return utils.generate_trace_id(ConfigManager().job_id)

    def get_serial_manager(self):
        """
        Get serial manager
        """
        return self.__user_serial_manager

    def set_trace_id(self, trace_id):
        """
        Set traceID
        """
        if yr.is_on_cloud():
            self.__thread_local.trace_id = trace_id

    def get_stream_client(self) -> StreamClient:
        """ Create one Producer to send element.
        """
        return self.__ds_client.get_stream_client()

    def _get_object(self, object_refs: List, timeout: int):
        iteration_timeout = timeout if timeout != -1 else None
        start = time.time()
        object_refs = object_refs if isinstance(object_refs, list) else [object_refs]
        wait_object_refs = list(set(object_refs))
        length = len(wait_object_refs)
        _, unready = yr.wait(list(set(wait_object_refs)), length, iteration_timeout)
        if len(unready) > 0:
            raise TimeoutError(f"get object timeout: {[ref.id for ref in unready]}")
        if iteration_timeout:
            iteration_timeout -= int(time.time() - start)
        else:
            # datasystem default get timeout is 60s
            iteration_timeout = 60
        ds_keys = [ref.id for ref in object_refs]
        objs = self._get_object_from_local(ds_keys, iteration_timeout)
        objs = objs if isinstance(objs, list) else [objs]
        return serialization.Serialization().multi_deserialize(objs)

    def _get_object_from_local(self, ds_keys: List, timeout):
        objs = LocalObjectStore().get(ds_keys)
        keys_not_in_local = []
        for key, obj in zip(ds_keys, objs):
            if obj is None:
                keys_not_in_local.append(key)
        if len(keys_not_in_local) != 0:
            objs_in_ds = self.__ds_client.get(keys_not_in_local, timeout)
            index = 0
            objs_in_ds = objs_in_ds if isinstance(objs_in_ds, list) else [objs_in_ds]
            for obj in objs_in_ds:
                while objs[index] is not None:
                    index += 1
                objs[index] = obj
        return objs


class UserSerialManager:
    """
    User Serial Manager
    """

    def __init__(self):
        # use it to store need_order instance send serial number
        self._task_serial_dict = {}
        self._lock = threading.Lock()

    def clear(self):
        """
        Clear serial dict
        """
        self._task_serial_dict.clear()

    def get_serial_number(self, instance_id: str):
        """
        Get serial number from instanceID
        """
        with self._lock:
            task_serial_num = self._task_serial_dict.get(instance_id, 0)
            self._task_serial_dict[instance_id] = task_serial_num + 1
        return task_serial_num

    def del_instance_serial_num(self, instance_id: str):
        """
        Delete instance serial number
        """
        with self._lock:
            self._task_serial_dict.pop(instance_id, None)


def _check_objs(obj_refs):
    exist_exception = False
    ready_objs = []
    unready_map = {}
    for obj_ref in obj_refs:
        if obj_ref.done():
            obj_ref.wait()
            ready_objs.append(obj_ref)
            if obj_ref.is_exception():
                exist_exception = True
            continue
        id_future = obj_ref.get_future()
        # multiple return value:one future contain multiple obj
        if id_future in unready_map:
            unready_map[id_future].append(obj_ref)
        else:
            unready_map[id_future] = [obj_ref]
    return ready_objs, unready_map, exist_exception


def wait(obj_refs: list, wait_num: int, timeout: int) -> (list, list):
    """
    wait obj_refs
    """
    start_time = time.time()
    ready_objs, unready_map, exist_exception = _check_objs(obj_refs)
    unready_futures = set(unready_map.keys())
    if len(unready_futures) == 0:
        return obj_refs[:wait_num], obj_refs[wait_num:]

    while len(ready_objs) < wait_num and not exist_exception:
        if timeout:
            iteration_timeout = timeout - time.time() + start_time
            if iteration_timeout <= 0:
                break
        else:
            iteration_timeout = None
        ready, _ = futures.wait(unready_futures, iteration_timeout, futures.FIRST_COMPLETED)
        if not ready:
            break
        unready_futures -= ready
        for r in ready:
            objs = unready_map.pop(r)
            # try to raise obj exception
            _ = [obj.wait() for obj in objs]
            ready_objs.extend(objs)

    ready_objs.sort(key=obj_refs.index)
    ready_objs = ready_objs[:wait_num]
    unready_objs = [obj for obj in obj_refs if obj not in ready_objs]
    return ready_objs, unready_objs
