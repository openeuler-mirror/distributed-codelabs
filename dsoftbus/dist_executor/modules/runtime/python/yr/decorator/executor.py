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

"""
Function executor
"""

import logging
from dataclasses import dataclass

from yr.runtime.task_spec import Task

_MIN_CHECK_TIME = 0.1
_CHECK_RECYCLE_RATIO = 0.1
_MAX_RETRY_TIME = 3
_MAX_QUEUE_SIZE = 200
_TIME_CALIBRATE = 1E-6

_logger = logging.getLogger(__name__)


@dataclass
class Request:
    """
    Request includes the request parameter task and callback.
    """
    task: Task
    callback: None


def get_func_request(task, callback):
    """
    Assemble the request array
    """
    return Request(task=task, callback=callback)


def _callback(data, err=None, future_map=None, future=None, instance_id=None, request=None):
    future.set_result("")
    request.callback(data, err)
    future_map.pop(instance_id)
