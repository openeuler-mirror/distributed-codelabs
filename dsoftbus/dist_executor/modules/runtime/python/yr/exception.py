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
"""yr exception type"""
import cloudpickle

from yr import utils


class YRError(Exception):
    """base Error"""


class CancelError(YRError):
    """task cancel error"""
    __slots__ = ["__task_id"]

    def __init__(self, task_id: str = ""):
        super().__init__()
        self.__task_id = task_id

    def __str__(self):
        return f"task request has been cancelled: {self.__task_id}"


class YRInvokeError(YRError):
    """Invoke error"""

    def __init__(self, cause, traceback_str: str):
        self.traceback_str = traceback_str
        self.cause = cause

    def __str__(self):
        return str(self.traceback_str)

    def origin_error(self):
        """Return a origin error for invoke task"""

        cause_cls = self.cause.__class__
        if issubclass(YRInvokeError, cause_cls):
            return self

        if issubclass(cause_cls, YRError):
            return self

        error_msg = str(self)

        class Cls(YRInvokeError, cause_cls):
            """New error inherit from origin cause"""

            def __init__(self, cause):
                self.cause = cause
                self.args = (cause,)

            def __getattr__(self, name):
                return getattr(self.cause, name)

            def __str__(self):
                return error_msg

        Cls.__name__ = f"YRInvokeError({cause_cls.__name__})"
        Cls.__qualname__ = Cls.__name__

        return Cls(self.cause)


class YRequestError(YRError, RuntimeError):
    """request failed error"""
    __slots__ = ["__code", "__message"]

    def __init__(self, code: int = 0, message: str = ""):
        self.__code = code
        self.__message = message
        super().__init__()

    def __str__(self):
        return str(f"failed to request, code: {self.__code}, message: {self.__message}")

    @property
    def code(self):
        """code"""
        return self.__code

    @property
    def message(self):
        """message"""
        return self.__message


def deal_with_yr_error(future, err):
    """deal with yr invoke error"""
    if isinstance(err, YRInvokeError):
        future.set_exception(err.origin_error())
    else:
        future.set_exception(err)


def deal_with_error(future, code, message):
    """
    processing request exceptions
    """
    try:
        obj = cloudpickle.loads(utils.hex_to_binary(message))
    except ValueError:
        future.set_exception(YRequestError(code, message))
        return
    if isinstance(obj, YRInvokeError):
        future.set_exception(obj.origin_error())
        return
    future.set_exception(YRequestError(code, str(obj)))
