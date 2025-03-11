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

"""task manager"""
import logging
from collections import deque
from concurrent.futures import Future
from dataclasses import dataclass
from enum import Enum
from threading import RLock, Lock
from typing import List
from typing import Optional

from yr import utils
from yr.common.response_helper import make_callback_for_kill
from yr.common.timer import Timer
from yr.config import ConfigManager
from yr.exception import CancelError
from yr.instance_ref import InstanceRef
from yr.runtime.instance import Resource, Instance
from yr.runtime.instance_manager import InstanceManager
from yr.runtime.scheduler import NormalScheduler, ConcurrencyScorer
from yr.runtime.task_spec import InvokeType, InvokeOptions, Signal
from yr.runtime.task_spec import TaskSpec


class TaskState(Enum):
    """task state"""
    PENDING = 1
    RUNNING = 2
    CANCELED = 3


@dataclass
class TaskRecord:
    """record for task"""
    state: TaskState
    resource: Resource = None
    task: TaskSpec = None


_logger = logging.getLogger(__name__)


class TaskManager:
    """Task manager"""
    __slots__ = ["__futures", "__invoke_client", "__queue", "__ins_mgr", "__pending_tasks",
                 "__timer", "__sleep_time_sequence", "__queue_lock", "__task_lock"]

    def __init__(self, invoke_client):
        # task_id/future
        self.__futures = {}
        self.__invoke_client = invoke_client
        self.__queue = deque()
        self.__pending_tasks = dict()
        scheduler = NormalScheduler(scorers=[ConcurrencyScorer()])
        self.__ins_mgr = InstanceManager(scheduler, invoke_client, ConfigManager().recycle_time)
        self.__timer = Timer()
        # delay to schedule 0,1,2,5,10,30,60,60
        self.__sleep_time_sequence = [0, 1, 2, 5, 10, 30, 60]
        self.__queue_lock = RLock()
        self.__task_lock = Lock()

    def add_task(self, task_id: str) -> Future:
        """add task"""
        future = Future()
        self.__futures[task_id] = future
        future.add_done_callback(lambda f: self.__pop_task(task_id))
        return future

    def get_future(self, task_id: str) -> Future:
        """get future by task id"""
        return self.__futures.get(task_id)

    def done(self, task_id: str) -> bool:
        """get task status by task id"""
        if task_id not in self.__futures:
            return True
        return self.__futures.get(task_id).done()

    def on_complete(self, task_id, callback) -> None:
        """register callback when task finished"""
        future = self.__futures.get(task_id)
        if future is None:
            callback(None)
        else:
            future.add_done_callback(callback)

    def submit_task(self, task: TaskSpec):
        """submit task"""
        if not ConfigManager().is_init:
            _logger.warning(f"Can not submit task {task.task_id} before yr.init")
            return
        if task.invoke_options:
            invoke_options = task.invoke_options
        else:
            invoke_options = InvokeOptions()
        resource = Resource(cpu=invoke_options.cpu,
                            memory=invoke_options.memory,
                            concurrency=invoke_options.concurrency,
                            language=task.object_descriptor.target_language,
                            resources=invoke_options.custom_resources)
        if task.invoke_type in (InvokeType.CREATE_INSTANCE, InvokeType.CREATE_NORMAL_FUNCTION_INSTANCE):
            self.__invoke_client.create(task)
        elif task.invoke_type in (InvokeType.INVOKE_MEMBER_FUNCTION,
                                  InvokeType.GET_NAMED_INSTANCE_METADATA):
            self.__invoke_client.invoke(task)
        elif task.invoke_type == InvokeType.INVOKE_NORMAL_FUNCTION:
            task_record = TaskRecord(state=TaskState.PENDING, resource=resource, task=task)
            if self.__add_task(task_record):
                self.__add_task_id(task.task_id)
                self.__schedule()
            elif self.__is_canceled(task.task_id):
                task.callback(None, CancelError(task.task_id))
            else:
                _logger.warning(f"task already in schedule, {task.task_id}")
        else:
            task.callback(None, TypeError(f"unexpect invoke type {task.invoke_type}"))

    def cancel(self, task_id: str) -> None:
        """cancel a task"""
        _logger.info(f"task canceled: {task_id}")
        future = self.__futures.get(task_id)
        if not future or future.done():
            return
        state = self.__set_canceled(task_id)
        if state == TaskState.CANCELED:
            return
        task_record = self.__get_task(task_id)
        task_record.task.callback(None, CancelError(task_id))
        if state == TaskState.RUNNING:
            self.__cancel_task(task_record.task.instance_id, task_id)

    def clear(self):
        """clear tasks and instances"""
        self.__futures.clear()
        self.__queue.clear()
        self.__pending_tasks.clear()
        self.__ins_mgr.clear()

    def __schedule(self) -> None:
        task_id = self.__pop_task_id()
        if task_id == "":
            return
        task_record = self.__get_task(task_id)
        if not task_record:
            return
        ret, err = self.__ins_mgr.check_last_failed_reason(resource=task_record.resource)
        if not ret:
            task_record.task.callback(None, err)
            return
        _logger.debug(f"schedule task {task_record.task.task_id}")
        ins = self.__ins_mgr.schedule(task_record.task, task_record.resource)
        if ins and ins.is_activate:
            _logger.debug(f"schedule successfully {task_record.task.task_id} {ins.instance_id.id}")
            task_record.state = TaskState.RUNNING
            task_record.task.instance_id = ins.instance_id

            def callback(_):
                ins.delete_task(task_id)
                self.__schedule()

            self.on_complete(task_id, callback)
            self.__invoke_client.invoke(task_record.task)
            self.__schedule()
        else:
            self.__add_task_id(task_id, True)
        if scale_out(
                list(filter(lambda record: record.resource == task_record.resource, self.__get_all_tasks())),
                list(self.__ins_mgr.get_instances(task_record.resource))):
            self.__scale_out_after(self.__get_scale_out_delay(task_record.resource), task_record)

    def __scale_out_after(self, sleep_time: int, task_record: TaskRecord) -> None:
        def scale_out_inner():
            instance_id = self.__ins_mgr.scale_out(task_record.task, task_record.resource)
            instance_id.on_complete(lambda x: self.__schedule())

        if sleep_time == 0:
            scale_out_inner()
        else:
            self.__timer.after(sleep_time, scale_out_inner)

    def __get_scale_out_delay(self, resource: Resource) -> int:
        count = self.__ins_mgr.get_failed_count(resource)
        if count >= len(self.__sleep_time_sequence):
            return self.__sleep_time_sequence[-1]
        return self.__sleep_time_sequence[count]

    def __cancel_task(self, instance_id: InstanceRef, task_id: str):
        if isinstance(instance_id, str):
            instance_id = InstanceRef(instance_id)
        func_info = ConfigManager().get_function_info()
        future = Future()

        task_spec = TaskSpec(task_id=utils.generate_task_id(),
                             job_id=ConfigManager().job_id,
                             invoke_type=InvokeType.KILL_INSTANCE,
                             invoke_timeout=ConfigManager().invoke_timeout,
                             function_info=func_info,
                             callback=make_callback_for_kill(future, instance_id.id, Signal.CANCEL),
                             instance_id=instance_id,
                             signal=Signal.CANCEL.value,
                             request_id=task_id
                             )

        self.__invoke_client.kill(task_spec)
        try:
            future.result()
        except RuntimeError as e:
            _logger.exception(e)

    def __add_task(self, task_record: TaskRecord) -> bool:
        with self.__task_lock:
            if task_record.task.task_id in self.__pending_tasks:
                return False
            self.__pending_tasks[task_record.task.task_id] = task_record
            return True

    def __get_task(self, task_id: str) -> Optional[TaskRecord]:
        with self.__task_lock:
            return self.__pending_tasks.get(task_id, None)

    def __pop_task(self, task_id: str):
        with self.__task_lock:
            return self.__pending_tasks.pop(task_id, None)

    def __get_all_tasks(self) -> List[TaskRecord]:
        with self.__task_lock:
            return list(self.__pending_tasks.values())

    def __add_task_id(self, task_id: str, left: bool = False):
        with self.__queue_lock:
            if left:
                self.__queue.appendleft(task_id)
            else:
                self.__queue.append(task_id)

    def __pop_task_id(self) -> str:
        with self.__queue_lock:
            if len(self.__queue) != 0:
                return self.__queue.pop()
        return ""

    def __is_canceled(self, task_id: str):
        with self.__task_lock:
            task_record = self.__pending_tasks.get(task_id, None)
            if not task_record:
                return False
            if task_record.state == TaskState.CANCELED:
                return True
            return False

    def __set_canceled(self, task_id: str) -> TaskState:
        with self.__task_lock:
            task_record = self.__pending_tasks.get(task_id, None)
            if not task_record:
                self.__pending_tasks[task_id] = TaskRecord(TaskState.CANCELED)
                return TaskState.CANCELED
            state = task_record.state
            task_record.state = TaskState.CANCELED
            return state


def scale_out(tasks: List[TaskRecord], instances: List[Instance]):
    """judge whether scaling out"""
    task_sum = len(list(tasks))
    concurrency_sum = sum([ins.resource.concurrency for ins in instances])
    _logger.debug(f"start to judge scale out {task_sum} {concurrency_sum}")
    return task_sum > concurrency_sum
