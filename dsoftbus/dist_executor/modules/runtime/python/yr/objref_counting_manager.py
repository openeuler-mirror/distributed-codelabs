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

""" objref_counting_manager provide reference counting relative functions"""

import threading
from typing import Union
from collections import deque

from yr.utils import Singleton, generate_random_id
from yr.object_ref import ObjectRef
import yr.storage.reference_count

INVOKE_ACK_TIMEOUT = 15
OBJECT_REF_SCALE_STEP = 100


class ObjectRefPool:
    """ObjectRefPool"""

    def __init__(self, pool_size=1):
        self.__ref_pool = deque()
        self.__pool_size = pool_size
        self._pool_lock = threading.Lock()
        self.__scale(pool_size)

    def pop(self, task_id=None):
        """pop and return an ObjectRef"""
        with self._pool_lock:
            if len(self.__ref_pool) == 0:
                self.__scale(self.__pool_size)

            ref = self.__ref_pool.pop()
            ref.task_id = task_id
            return ref

    def clear(self):
        """clear object ref pool"""
        self.__ref_pool.clear()

    def __scale(self, size):
        ids = []
        for _ in range(size):
            object_id = generate_random_id()
            ref = ObjectRef(object_id, None, need_incre=False, need_decre=True)
            self.__ref_pool.append(ref)
            ids.append(object_id)
        yr.storage.reference_count.increase_reference_count(ids)


@Singleton
class ObjRefCountingManager:
    """ObjRefCountingManager"""

    def __init__(self):
        self._invoke_map = {}
        self._empty_signal = threading.Event()
        self._ref_pool = ObjectRefPool(OBJECT_REF_SCALE_STEP)

    def join_record(self, key: str, refs: Union[ObjectRef, list]):
        """add and merge record to the map"""
        if len(key) == 0 or refs is None:
            return
        if isinstance(refs, list) and len(refs) == 0:
            return
        if isinstance(refs, ObjectRef):
            refs = [refs]
        if key not in self._invoke_map:
            self._invoke_map[key] = []

        self._invoke_map[key] += refs
        self._empty_signal.clear()

    def del_record(self, key: str):
        """rmv record from the map"""
        self._invoke_map.pop(key, None)

        if self.is_empty():
            self._empty_signal.set()

    def is_empty(self):
        """check if the map is empty"""
        return len(self._invoke_map) == 0

    def wait_invoke_ack(self, timeout=INVOKE_ACK_TIMEOUT):
        """wait until the map is empty"""
        if not self.is_empty():
            self._empty_signal.wait(timeout)

    def create_object_ref(self, task_id=None, count=1):
        """generate object reference"""
        refs = []
        for _ in range(count):
            ref = self._ref_pool.pop(task_id)
            refs.append(ref)

        return refs

    def clear(self):
        """clear object ref pool"""
        self._ref_pool.clear()
