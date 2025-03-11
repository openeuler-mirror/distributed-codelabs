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

"""log and context"""

from abc import ABCMeta
from abc import abstractmethod
from enum import Enum
from enum import unique


@unique
class LogLevel(Enum):
    """
    LogLevel defines for log level
    """
    INFO = 'INFO'
    WARN = 'WARN'
    DEBUG = 'DEBUG'
    ERROR = 'ERROR'


class Logger(metaclass=ABCMeta):
    """
    Class log to write log
    """
    @abstractmethod
    def log(self, loglevel: LogLevel, message: str):
        """
        write log
        """


class Context(metaclass=ABCMeta):
    """
    Runtime context
    """

    def __init__(self):
        """
        init
        """

    def get_env(self, key: str):
        """
        Get environment
        """

    def get_state(self):
        """
        Get state
        """

    def get_invoke_id(self):
        """
        Get invoke id
        """

    def http_response(self,
                      body=None,
                      headers=None,
                      content_type="text/plain",
                      status_code=200):
        """
        Get http response body
        """

    def get_state_id(self):
        """
        Get state id
        """

    def get_invoke_property(self):
        """
        Get invoke priority
        """

    def get_trace_id(self):
        """
        Get invoke trace id
        """
