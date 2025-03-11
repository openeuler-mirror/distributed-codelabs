# !/usr/bin/env python3
# coding=UTF-8
# Copyright (c) 2022 Huawei Technologies Co., Ltd
#
# This software is licensed under Mulan PSL v2. You can use this software
# according to the terms and conditions of the Mulan PSL v2. You may obtain a
# copy of Mulan PSL v2 at:
#
# http://license.coscl.org.cn/MulanPSL2
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.

"""Inorder manager"""
import heapq


class RequestQueue:
    """
    RequestQueue is used to keep the sequence of call requests
    """
    def __init__(self):
        self._next_serial_num = 0
        self._queue = []

    def next_serial_num(self):
        """
        Get serial number indicating the next request to be processed
        """
        return self._next_serial_num

    def set_next_serial_num(self):
        """
        Set next serial number, auto increasing 1
        """
        self._next_serial_num += 1

    def head_serial_num(self):
        """
        Get the serial number of the head request in the queue
        """
        return self._queue[0][0]

    def pop_head_request(self):
        """
        Pop the head request in the queue
        """
        return heapq.heappop(self._queue)[-1]

    def store_request(self, serial_num, request):
        """
        Push
        """
        heapq.heappush(self._queue, (serial_num, request))

    def empty(self):
        """
        Check if the queue is empty
        """
        return not self._queue


class InOrderManager:
    """
    InOrderManager is used to store RequestQueue
    """
    def __init__(self):
        self._inorder_manager = {}

    def get_instance_request_queue(self, key):
        """
        Get RequestQueue by the key of runtime id

        :param key: runtime key
        :return: RequestQueue instance
        """
        return self._inorder_manager.setdefault(key, RequestQueue())

    def clear(self):
        """
        Clear the inorder manager
        """
        self._inorder_manager.clear()
