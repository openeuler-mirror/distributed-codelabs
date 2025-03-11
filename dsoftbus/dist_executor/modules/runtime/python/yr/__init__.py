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
yr api
"""

from yr.config import Config, DeploymentConfig, UserTLSConfig
from yr.apis import init
from yr.apis import is_initialized
from yr.apis import get
from yr.apis import put
from yr.apis import wait
from yr.apis import invoke
from yr.apis import instance
from yr.apis import method
from yr.apis import finalize
from yr.apis import exit
from yr.apis import cancel
from yr.apis import cpp_function
from yr.apis import cpp_instance_class
from yr.apis import is_on_cloud
from yr.apis import get_stream_client
from yr.apis import get_instance
from yr.runtime.task_spec import InvokeOptions
from yr.runtime.task_spec import AffinityType

__all__ = ["Config", "DeploymentConfig", "UserTLSConfig", "init", "is_initialized", "get", "put",
           "wait", "invoke", "instance", "method", "finalize", "exit",
           "cancel", "cpp_function", "cpp_instance_class", "is_on_cloud",
           "get_stream_client", "get_instance",
           "InvokeOptions", "AffinityType"]
