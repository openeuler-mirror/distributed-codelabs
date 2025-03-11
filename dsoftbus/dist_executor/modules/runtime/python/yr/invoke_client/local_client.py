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
"""local handler"""
import logging
import threading
from abc import ABC

from yr.invoke_client.invoke_client import InvokeClient
from yr.rpc.core_service_pb2 import KillResponse
from yr.runtime.local_object_store import LocalObjectStore
from yr.runtime.task_spec import TaskSpec
from yr.runtime.worker import Worker
from yr.utils import generate_random_id

_logger = logging.getLogger(__name__)


class LocalClient(InvokeClient, ABC):
    """local handler"""

    def __init__(self):
        self.workers = {}
        self._running = True
        self._lock = threading.Lock()

    def create(self, task: TaskSpec):
        """create worker"""
        if not self._running:
            return
        instance_id = generate_random_id()
        concurrency = 100 if task.invoke_options is None else task.invoke_options.concurrency
        worker = Worker()
        worker.init(instance_id, concurrency)
        with self._lock:
            self.workers[instance_id] = worker
        worker.submit(task)

    def invoke(self, task: TaskSpec):
        """instance function invoke"""
        if not self._running:
            return
        worker = self.workers.get(task.instance_id.id)
        if worker is None:
            raise RuntimeError(f"No such instance: {task.instance_id.id}")
        worker.submit(task)

    def kill(self, task: TaskSpec):
        """kill worker"""
        with self._lock:
            if task.instance_id.id in self.workers:
                worker = self.workers.pop(task.instance_id.id)
                worker.stop()
        task.callback(KillResponse(), None)

    def exit(self) -> None:
        """exit instance"""
        _logger.warning("local mode not support exit")

    def save_state(self, state) -> str:
        """save instance state"""
        key = generate_random_id()
        LocalObjectStore().put(key, state)
        return key

    def load_state(self, checkpoint_id) -> bytes:
        """load instance state"""
        return LocalObjectStore().get(checkpoint_id)

    def clear(self):
        """stop all workers"""
        self._running = False
        with self._lock:
            for worker in self.workers.values():
                worker.stop()
            self.workers.clear()
