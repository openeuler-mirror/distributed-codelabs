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
"""ReferenceCount"""
import logging
import threading
from collections import deque
from typing import List, Union

from yr.storage.ds_client import DSClient
from yr.utils import Singleton

_logger = logging.getLogger(__name__)


class BlockingQueue:
    """BlockingQueue"""
    __slots__ = ["__queue", "__not_empty"]

    def __init__(self):
        self.__queue = deque()
        self.__not_empty = threading.Event()

    def set_no_wait(self):
        """set queue no wait"""
        self.__not_empty.set()

    def len(self):
        """get length of queue"""
        return len(self.__queue)

    def append(self, elem: list):
        """append an element to queue"""
        if len(elem) == 0:
            return
        self.__queue.append(elem)
        self.__not_empty.set()

    def pop_all(self) -> list:
        """wait and pop all elements"""
        if len(self.__queue) == 0:
            self.__not_empty.wait()
            self.__not_empty.clear()

        elems = []
        for _ in range(len(self.__queue)):
            elems += self.__queue.popleft()
        return elems


@Singleton
class ReferenceCount:
    """global reference count"""
    __slots__ = ["__ds_client", "__queue", "__is_running", "__send_thread"]

    def __init__(self):
        self.__ds_client = None
        self.__queue = None
        self.__is_running = False
        self.__send_thread = None

    @property
    def is_init(self):
        """return reference count state"""
        return self.__ds_client is not None

    def init(self, ds_client: DSClient):
        """init reference count"""
        self.__ds_client = ds_client
        self.__queue = BlockingQueue()
        self.__is_running = True
        self.__send_thread = threading.Thread(target=self.__process, name="YRReferenceCount", daemon=True)
        self.__send_thread.start()

    def stop(self):
        """stop reference count"""
        if not self.is_init:
            return

        _logger.debug(f"[Reference Counting] ReferenceCount stop")
        self.__is_running = False
        self.__queue.set_no_wait()
        self.__send_thread.join()
        self.__ds_client = None

    def increase(self, object_ids: list):
        """increase global reference"""
        _logger.debug(f"[Reference Counting] datasystem incre ref count, object_ids: {object_ids}")
        self.__ds_client.increase_global_reference(object_ids)

    def decrease(self, object_ids: list):
        """decrease global reference"""
        self.__queue.append(object_ids)

    def is_obj_in_ctx(self, object_ref) -> bool:
        """is objectRef in context"""
        if self.__ds_client is None or object_ref is None:
            return False
        return self.__ds_client.is_obj_id_in_ctx(object_ref.id)

    def __process(self) -> None:
        """decrease global reference"""
        while self.__is_running or self.__queue.len() != 0:
            object_ids = self.__queue.pop_all()
            if len(object_ids) == 0:
                continue
            _logger.debug(f"[Reference Counting] datasystem decre ref count, object_ids: {object_ids}")
            self.__ds_client.decrease_global_reference(object_ids)


def increase_reference_count(object_ids: Union[str, List]):
    """increase global reference"""
    if not ReferenceCount().is_init or len(object_ids) == 0:
        return
    if isinstance(object_ids, str):
        object_ids = [object_ids]

    ReferenceCount().increase(object_ids)


def decrease_reference_count(object_ids: Union[str, List]):
    """decrease global reference"""
    if not ReferenceCount().is_init or len(object_ids) == 0:
        return
    if isinstance(object_ids, str):
        object_ids = [object_ids]

    ReferenceCount().decrease(object_ids)
