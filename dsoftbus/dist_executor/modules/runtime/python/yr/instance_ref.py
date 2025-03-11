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
"""InstanceRef"""
import concurrent.futures
from typing import Union


class InstanceRef:
    """InstanceRef

    Notify: InstanceRef not thread safe
    """
    _id = None
    _id_future = None
    _task_id = None

    def __init__(self, instance_id: Union[str, concurrent.futures.Future], task_id=None, is_user_defined_id=False):
        if isinstance(instance_id, str):
            self._id = instance_id
        elif isinstance(instance_id, concurrent.futures.Future):
            self._id_future = instance_id
        else:
            raise TypeError(f"Unexpected type for id {instance_id}")
        self._task_id = task_id
        self._is_user_defined_id = is_user_defined_id

    def __str__(self):
        return self.id

    @property
    def is_user_defined_id(self):
        """task id"""
        return self._is_user_defined_id

    @property
    def task_id(self):
        """task id"""
        return self._task_id

    @property
    def id(self):
        """object id"""
        self._wait_for_id()
        return self._id

    @property
    def is_failed(self) -> bool:
        """return task result"""
        f = self._id_future
        if not f or not f.done():
            return False
        if f.exception():
            return True
        return False

    def get_id(self, timeout=None):
        """get object id"""
        self._wait_for_id(timeout)
        return self.id

    def get_future(self):
        """get future"""
        return self._id_future

    def wait(self, timeout=None):
        """wait task done"""
        self._wait_for_id(timeout)

    def done(self):
        """Return True if the obj future was cancelled or finished executing."""
        if self._id_future:
            return self._id_future.done()
        return True

    def cancel(self):
        """cancel the obj future"""
        if self._id_future:
            self._id_future.cancel()

    def on_complete(self, callback):
        """register callback"""
        if self._id_future and not self._id_future.done():
            self._id_future.add_done_callback(callback)
            return
        callback(self._id_future)

    def _wait_for_id(self, timeout=None):
        if self._id_future:
            self._id = self._id_future.result(timeout=timeout)
            self._id_future = None
