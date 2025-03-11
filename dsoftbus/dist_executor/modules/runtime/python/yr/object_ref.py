#!/usr/bin/env python
# coding=utf-8
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
"""ObjectRef"""

import yr
import yr.storage.reference_count


class ObjectRef:
    """ObjectRef"""
    _id = None
    _task_id = None

    def __init__(self, object_id: str, task_id=None, need_incre=True, need_decre=True):
        self._id = object_id
        self._task_id = task_id
        self._need_decre = need_decre

        if need_incre:
            yr.storage.reference_count.increase_reference_count(self._id)

    def __del__(self):
        if self._need_decre:
            yr.storage.reference_count.decrease_reference_count(self._id)

    def __copy__(self):
        return self

    def __deepcopy__(self, memo):
        return self

    def __str__(self):
        return self.id

    def __eq__(self, other):
        return self.id == other

    def __hash__(self):
        return hash(self.id)

    @property
    def task_id(self):
        """task id"""
        return self._task_id

    @task_id.setter
    def task_id(self, value):
        """task id"""
        if value is not None:
            self._task_id = value

    @property
    def id(self):
        """object id"""
        return self._id

    def get_future(self):
        """get future"""
        return yr.runtime.runtime.Runtime().rt.task_mgr.get_future(self.task_id)

    def wait(self, timeout=None):
        """wait task done"""
        future = self.get_future()
        if future is not None:
            future.result(timeout=timeout)

    def is_exception(self) -> bool:
        """is future exception"""
        future = self.get_future()
        if future is None:
            return False
        return future.exception() is not None

    def done(self):
        """Return True if the obj future was cancelled or finished executing."""
        future = self.get_future()
        if future:
            return future.done()
        return True

    def cancel(self):
        """cancel the obj future"""
        future = self.get_future()
        if future:
            future.cancel()

    def on_complete(self, callback):
        """register callback"""
        if self.task_id is None:
            callback(None)
        else:
            yr.runtime.runtime.Runtime().rt.task_mgr.on_complete(self.task_id, callback)
