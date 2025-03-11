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
"""global timer"""
import logging
import sched
import time
from threading import Thread
from typing import Callable

from yr.utils import Singleton

_logger = logging.getLogger(__name__)


@Singleton
class Timer:
    """timer for task schedule"""
    __slots__ = ["__scheduler", "__is_running", "__thread", "__interval"]

    def __init__(self):
        self.__scheduler = sched.scheduler(time.time, time.sleep)
        self.__is_running = False
        self.__thread = None
        self.__interval = 0.1

    def init(self, interval=0.1):
        """start thread to scheduler timer"""
        if self.__is_running and self.__thread.is_alive():
            return
        self.__interval = interval
        self.__thread = Thread(target=self.run, name="YRTimer", daemon=True)
        self.__is_running = True
        self.__thread.start()

    def clear(self):
        """clear tasks"""
        if self.__scheduler.empty():
            return
        for event in self.__scheduler.queue:
            try:
                self.__scheduler.cancel(event)
            except ValueError:
                continue

    def stop(self):
        """stop timer thread"""
        if self.__is_running:
            self.clear()
            self.__is_running = False
            self.__thread.join()

    def run(self) -> None:
        """timer loop"""
        while self.__is_running:
            try:
                self.__scheduler.run(blocking=False)
            except Exception as e:
                _logger.exception(e)
            time.sleep(self.__interval)

    def after(self, delay: float, action: Callable, *args, **kwargs) -> None:
        """run a task after some time"""
        self.__scheduler.enter(delay, 0, action, argument=args, kwargs=kwargs)
