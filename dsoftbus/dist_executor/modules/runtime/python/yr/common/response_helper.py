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
"""response helper"""
import logging
from concurrent.futures import Future
from typing import Union

from yr import exception
from yr.rpc import core_service_pb2
from yr.runtime.task_spec import Signal

_logger = logging.getLogger(__name__)


def make_callback_for_kill(future: Future, instance_id: str, signal: Signal = Signal.EXIT):
    """deal with kill response"""

    def callback(data: Union[core_service_pb2.KillResponse, str, bytes], err: Exception = None):
        if future.done():
            return
        if err:
            exception.deal_with_yr_error(future, err)
            return
        if isinstance(data, core_service_pb2.KillResponse):
            rsp = data
        else:
            rsp = core_service_pb2.KillResponse()
            rsp.ParseFromString(data)
        if rsp.code != 0:
            msg = f"signal failed: {instance_id} {signal} code:{rsp.code}, msg: {rsp.message}"
            future.set_exception(RuntimeError(msg))
            return
        future.set_result(rsp.code)

    return callback


def make_callback_for_create(future: Future):
    """deal with create response"""

    def callback(response: Union[core_service_pb2.CreateResponse, str, bytes], err: Exception = None):
        if future.done():
            return
        if err:
            exception.deal_with_yr_error(future, err)
            return
        if not isinstance(response, core_service_pb2.CreateResponse):
            resp = core_service_pb2.CreateResponse()
            resp.ParseFromString(response)
        else:
            resp = response
        if resp.code == 0:
            future.set_result(resp.instanceID)
        else:
            exception.deal_with_error(future, resp.code, resp.message)

    return callback
