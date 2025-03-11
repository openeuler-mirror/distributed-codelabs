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
"""Instance class for scheduler"""
import time
from dataclasses import dataclass, field
from typing import Optional, Dict

from yr.instance_ref import InstanceRef
from yr.utils import LANGUAGE_PYTHON


@dataclass
class Resource:
    """instance resource"""
    comparable_res: tuple = ('cpu', 'memory', 'concurrency', 'language', 'resources')
    cpu: int = 500
    memory: int = 500
    concurrency: int = 1
    language: str = LANGUAGE_PYTHON
    resources: Dict[str, float] = field(default_factory=dict)

    def __hash__(self):
        return hash(str(self))

    def __eq__(self, other: Optional['Resource']):
        if not other:
            return False
        for res in self.comparable_res:
            if getattr(self, res) != getattr(other, res):
                return False
        return True

    def __str__(self):
        res = "".join(map(lambda x: f"({x[0]},{x[1]:.4f})", sorted(self.resources.items(), key=lambda x: x[0])))
        return f"{self.cpu}-{self.memory}-{self.concurrency}-{self.language}{res}"


class Instance:
    """Instance used by Scheduler"""
    __slots__ = ["__resource", "__instance_id", "__tasks", "__last_activate_time", "__is_recycled"]

    def __init__(self, instance_id: InstanceRef, resource: Resource):
        self.__instance_id = instance_id
        self.__resource = resource
        self.__tasks = set()
        self.__last_activate_time = time.time()
        self.__is_recycled = False

    def __str__(self):
        return f"instance id: {self.__instance_id.id if self.is_activate else 'not activate'} " \
               f"task id: {self.__instance_id.task_id} " \
               f"task count: {self.task_count} " \
               f"resource: {self.__resource} " \
               f"last activate time: {time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(self.__last_activate_time))}"

    @property
    def last_activate_time(self) -> float:
        """last activate time. used to recycle"""
        return self.__last_activate_time

    @property
    def is_activate(self) -> bool:
        """is instance activate"""
        if self.__is_recycled:
            return False
        return self.__instance_id.done() and not self.__instance_id.is_failed

    @property
    def resource(self) -> Resource:
        """return instance resource"""
        return self.__resource

    @property
    def task_count(self) -> int:
        """return task count"""
        return len(self.__tasks)

    @property
    def instance_id(self):
        """return instance id"""
        return self.__instance_id

    def set_recycled(self):
        """set recycled status"""
        self.__is_recycled = True

    def add_task(self, task_id: str):
        """add a task to instance"""
        self.__tasks.add(task_id)

    def delete_task(self, task_id: str):
        """delete task"""
        self.__tasks.remove(task_id)
        self.refresh()

    def refresh(self):
        """refresh last activate time"""
        self.__last_activate_time = time.time()
