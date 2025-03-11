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

"""Invoke client"""
import decimal
import json
import logging
import time
from abc import abstractmethod, ABCMeta

from google.protobuf import json_format

from yr import utils
from yr.invoke_client.yr_client import auth_tools
from yr.runtime.task_spec import TaskSpec

_logger = logging.getLogger(__name__)


class InvokeClient(metaclass=ABCMeta):
    """Invoke client for in cluster or out cluster"""

    @abstractmethod
    def create(self, task: TaskSpec) -> None:
        """create instance"""

    @abstractmethod
    def invoke(self, task: TaskSpec) -> None:
        """invoke"""

    @abstractmethod
    def kill(self, task: TaskSpec) -> None:
        """kill"""

    @abstractmethod
    def exit(self) -> None:
        """exit instance"""

    @abstractmethod
    def save_state(self, state) -> str:
        """save instance state"""

    @abstractmethod
    def load_state(self, checkpoint_id) -> bytes:
        """load instance state"""

    @abstractmethod
    def clear(self) -> None:
        """clear"""


def create_headers(path, func_info, payload, invoke_id):
    """create headers"""
    sign_struct = {
        "method": "POST",
        "path": path,
        "body": payload,
        "appid": func_info.app_id,
        "timestamp": str(decimal.Decimal(time.time() * 1000).quantize(decimal.Decimal('0'), decimal.ROUND_HALF_UP))
    }
    authorization = auth_tools.get_authorization(func_info.app_key.encode(),
                                                 sign_struct)
    headers = {utils.CONTENT_TYPE: utils.CONTENT_TYPE_APPLICATION_JSON,
               utils.HEADER_EVENT_SOURCE: func_info.app_id,
               utils.HEADER_TRACE_ID: invoke_id,
               utils.HEADER_INVOKE_URN: func_info.function_id,
               utils.AUTHORIZATION: authorization}
    return headers


def create_payload(task: TaskSpec, invoke_type):
    """create payload in faas"""
    args = [json_format.MessageToDict(arg) for arg in task.args]
    payload = {
        "functionUrn": task.function_info.function_id,
        "args": args,
        "type": invoke_type,
        "returnObjectIDs": task.object_ids
    }
    return json.dumps(payload)
