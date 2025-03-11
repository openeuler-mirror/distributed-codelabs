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
"""instance manager"""
import concurrent.futures
import logging
import threading
import time
from collections import OrderedDict, defaultdict
from concurrent.futures import Future, wait
from typing import Optional, List

import yr
from yr import utils
from yr.common.response_helper import make_callback_for_kill, make_callback_for_create
from yr.common.timer import Timer
from yr.config import ConfigManager, DEFAULT_CONCURRENCY
from yr.exception import YRInvokeError, YRequestError
from yr.instance_ref import InstanceRef
from yr.rpc.common_pb2 import Arg, ERR_RESOURCE_NOT_ENOUGH
from yr.runtime.instance import Instance, Resource
from yr.runtime.scheduler import Scheduler
from yr.runtime.task_spec import InvokeType, Signal, callback_hook
from yr.runtime.task_spec import TaskMetadata
from yr.runtime.task_spec import TaskSpec
from yr.utils import generate_task_id

_logger = logging.getLogger(__name__)

_PRINT_ERROR_PERIOD = 60


def print_instance_create_error(future: Future):
    """print instance create error"""
    if future.exception():
        _logger.exception(future.exception())


def warning_if_failed(future: concurrent.futures.Future, describe: str):
    """warning for catching future exception"""
    try:
        future.result()
    except RuntimeError as e:
        _logger.warning(f"{describe}, err:{e}")


class InstanceManager:
    """manager stateless instance"""
    __slots__ = ["__instances", "__scheduler", "__invoke_client", "__recycle_scheduler", "__recycle_time",
                 "__recycle_period", "__lock", "__failed_count", "__last_failed_reason",
                 "__last_activate_instance_cnt", "__default_timeout"]

    def __init__(self, scheduler: Scheduler, invoke_client, recycle_time=2):
        self.__instances = dict()
        self.__scheduler = scheduler
        self.__invoke_client = invoke_client
        self.__recycle_time = recycle_time
        self.__recycle_period = float(recycle_time) / 10
        self.__recycle_scheduler = Timer()
        self.__recycle_scheduler.after(self.__recycle_period, self.__recycle)
        self.__recycle_scheduler.after(_PRINT_ERROR_PERIOD, self.__print_create_error)
        self.__lock = threading.RLock()
        self.__failed_count = defaultdict(int)
        self.__last_failed_reason = {}
        self.__default_timeout = 30
        self.__last_activate_instance_cnt = 0

    def get_instances(self, resource: Resource) -> List[Instance]:
        """get instance by resource"""
        with self.__lock:
            instances = self.__instances.get(resource)
            if instances:
                return list(instances.values())
        return list()

    def get_instance_count(self, resource: Resource) -> int:
        """get instance count by resource"""
        with self.__lock:
            instances = self.__instances.get(resource)
            if instances:
                return len(instances)
        return 0

    def get_failed_count(self, resource: Resource):
        """get instance create failed count by resource"""
        return self.__failed_count[resource]

    def get_last_failed_reason(self, resource: Resource):
        """get instance create failed count by resource"""
        return self.__failed_count.get(resource, None)

    def check_last_failed_reason(self, resource: Resource) -> (bool, Optional[Exception]):
        """
        check whether an exception occurs during instance creation.
        will not return true when instance can be create before but resource not enough now.
        """
        err = self.__last_failed_reason.get(resource)
        if not err:
            return True, err
        if isinstance(err, YRInvokeError):
            return False, err.origin_error()
        if isinstance(err, YRequestError):
            if err.code != ERR_RESOURCE_NOT_ENOUGH:
                return False, err
        return True, err

    def schedule(self, task: TaskSpec, resource: Resource) -> Optional[Instance]:
        """select a instance for task"""

        instance = None
        if resource not in self.__instances:
            return instance
        instances = self.get_instances(resource)
        if len(instances) != 0:
            instance = self.__scheduler.schedule(task, instances[::-1])
            with self.__lock:
                if instance and instance.is_activate:
                    instance.add_task(task_id=task.task_id)
                    return instance
        return instance

    def scale_out(self, task: TaskSpec, resource: Resource) -> InstanceRef:
        """scale out a instance"""
        future = concurrent.futures.Future()

        descriptor = utils.ObjectDescriptor.parse(task.object_descriptor.to_dict())
        if task.object_descriptor.target_language == utils.LANGUAGE_CPP:
            descriptor.function_name = ""
        if task.invoke_options:
            concurrency = task.invoke_options.concurrency
        else:
            concurrency = DEFAULT_CONCURRENCY
        metadata = TaskMetadata.create(object_descriptor=descriptor, code_id=utils.NORMAL_FUNCTION,
                                       invoke_type=InvokeType.CREATE_NORMAL_FUNCTION_INSTANCE, concurrency=concurrency)
        trace_id = task.trace_id if yr.is_on_cloud() else utils.generate_trace_id(task.job_id)
        create_task = TaskSpec(task_id=generate_task_id(),
                               job_id=ConfigManager().job_id,
                               invoke_type=InvokeType.CREATE_NORMAL_FUNCTION_INSTANCE,
                               invoke_timeout=self.__default_timeout,
                               callback=make_callback_for_create(future),
                               code_id=utils.NORMAL_FUNCTION,
                               function_info=task.function_info,
                               object_descriptor=descriptor,
                               args=[Arg(type=0, value=metadata.to_json().encode())],
                               trace_id=trace_id,
                               invoke_options=task.invoke_options
                               )
        create_task.callback = callback_hook(create_task.callback, create_task.task_id, create_task.invoke_type)
        instance_id = InstanceRef(instance_id=future, task_id=create_task.task_id)
        instance = Instance(instance_id=instance_id, resource=resource)
        self.__add_instance(resource, instance)
        future.add_done_callback(self.__scale_out_result_process(resource, instance))
        self.__invoke_client.create(create_task)
        return instance_id

    def clear(self):
        """clear instances"""
        with self.__lock:
            if _logger.isEnabledFor(logging.DEBUG):
                delete_ids = []
                for instances in self.__instances.values():
                    for instance_id in instances.keys():
                        delete_ids.append(instance_id.id)
                _logger.debug(f"instance killed: {delete_ids}")
            #  send kill(2) to clear stateless and stateful instances from job-id when finalize
            self.__instances.clear()

    def kill_instance(self, instance_id: InstanceRef):
        """kill stateless instance"""
        need_kill = False
        with self.__lock:
            for instances in self.__instances.values():
                if instance_id in instances:
                    ins = instances.pop(instance_id)
                    _logger.debug(f"instance need kill: {ins}")
                    need_kill = True
        if need_kill:
            future = self.__delete_instance(instance_id)
            warning_if_failed(future, "delete instance")

    def __add_instance(self, resource: Resource, instance: Instance):
        with self.__lock:
            self.__instances.setdefault(resource, OrderedDict())[instance.instance_id] = instance

    def __recycle(self):
        self.__recycle_scheduler.after(self.__recycle_period, self.__recycle)
        with self.__lock:
            need_recycle = self.__clear_instance()
        futures = [self.__delete_instance(ins.instance_id) for ins in need_recycle]
        for future in concurrent.futures.as_completed(futures):
            warning_if_failed(future, "delete instance")

    def __clear_instance(self) -> List[Instance]:
        now = time.time()
        need_recycle = []

        for res, instances in self.__instances.items():
            activate_cnt = 0
            for instance_id in list(instances.keys()):
                ins = instances.get(instance_id)
                if not ins or not ins.instance_id.done():
                    continue
                if now - ins.last_activate_time > self.__recycle_time and ins.task_count == 0:
                    ins = instances.pop(instance_id)
                    ins.set_recycled()
                    _logger.debug(f"instance need recycle: {ins}")
                    need_recycle.append(ins)
                activate_cnt += 1
            self.__last_activate_instance_cnt = activate_cnt
            _logger.debug(f"activate instance count: {activate_cnt} res: {res}")
        return need_recycle

    def __delete_instance(self, instance_id: InstanceRef):
        func_info = ConfigManager().get_function_info()
        future = concurrent.futures.Future()
        if not instance_id.done():
            wait([instance_id.get_future()])
        if instance_id.is_failed:
            return instance_id.get_future()
        _logger.info(f"instance killed: {instance_id.id}")
        self.__invoke_client.kill(TaskSpec(task_id=utils.generate_task_id(),
                                           job_id=ConfigManager().job_id,
                                           invoke_type=InvokeType.KILL_INSTANCE,
                                           invoke_timeout=self.__default_timeout,
                                           function_info=func_info,
                                           callback=make_callback_for_kill(future, instance_id.id,
                                                                           signal=Signal.KILL_INSTANCE),
                                           instance_id=instance_id,
                                           signal=Signal.KILL_INSTANCE.value))
        return future

    def __scale_out_result_process(self, resource: Resource, instance: Instance):

        def callback(future: Future):
            """print instance create error"""
            if future.exception():
                self.__failed_count[resource] += 1
                self.__last_failed_reason[resource] = future.exception()
                with self.__lock:
                    instances = self.__instances.get(resource)
                    if instances:
                        instances.pop(instance.instance_id, None)
            else:
                self.__failed_count[resource] = 0
                self.__last_failed_reason.pop(resource, None)
                instance.refresh()

        return callback

    def __print_create_error(self):
        self.__recycle_scheduler.after(_PRINT_ERROR_PERIOD, self.__print_create_error)
        for res in list(self.__last_failed_reason.keys()):
            err = self.__last_failed_reason.get(res, None)
            if err and isinstance(err, YRequestError):
                if err.code == ERR_RESOURCE_NOT_ENOUGH:
                    _logger.info(f"{res} resource not enough")
