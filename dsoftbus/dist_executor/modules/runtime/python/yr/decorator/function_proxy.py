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
Function decorator
"""
import logging
import types
from functools import wraps

import yr
from yr import exception
from yr import signature, utils
from yr.objref_counting_manager import ObjRefCountingManager
from yr.rpc import core_service_pb2
from yr.runtime.runtime import Runtime
from yr.runtime.task_spec import InvokeType, Task, InvokeOptions
from yr.storage.reference_count import ReferenceCount
from yr.utils import ObjectDescriptor, LANGUAGE_CPP, CrossLanguageInfo

_logger = logging.getLogger(__name__)


class FunctionProxy:
    """
    Use to decorate user function
    """

    def __init__(self, func, cross_language_info=None, invoke_options=None, return_nums=None):
        """Initialize an Initiator object"""
        self.cross_language_info = cross_language_info
        self.invoke_options = invoke_options
        self.__code_ref__ = None
        self.__original_func__ = func
        if return_nums is None:
            self.return_nums = 1
        else:
            if not isinstance(return_nums, int):
                raise TypeError(f"invalid return_nums type: {type(return_nums)}, should be an int")
            if return_nums < 0 or return_nums > 100:
                raise RuntimeError(f"invalid return_nums: {return_nums}, should be an integer between 0 and 100")
            self.return_nums = return_nums
        if self.cross_language_info is not None:
            def _cross_invoke_proxy(*args, **kwargs):
                return self._invoke(func, args=args, kwargs=kwargs)

            self.invoke = _cross_invoke_proxy
        else:
            @wraps(func)
            def _invoke_proxy(*args, **kwargs):
                return self._invoke(func, args=args, kwargs=kwargs)

            self.invoke = _invoke_proxy

    def __call__(self, *args, **kwargs):
        """
        invalid call
        """
        raise RuntimeError("invoke function cannot be called directly")

    def __getstate__(self):
        self.__code_ref__ = None
        return self.__dict__

    def options(self, invoke_options: InvokeOptions):
        """
        Set user invoke options
        Args:
            invoke_options: invoke options for users to set resources
        """
        self.invoke_options = invoke_options
        return self

    def get_original_func(self):
        """
        The original function
        """
        return self.__original_func__

    def _invoke(self, func, args=None, kwargs=None):
        """
        The real realization of the invoke function
        """
        if self.cross_language_info is None and not ReferenceCount().is_obj_in_ctx(self.__code_ref__):
            self.__code_ref__ = yr.put(func)
            _logger.debug(f"[Reference Counting] put code with id = {self.__code_ref__.id}, "
                          f"functionName={func.__qualname__}")

        sig = None
        if func is not None:
            sig = signature.get_signature(func)

        args_list = signature.package_args(sig, args, kwargs)
        task_id = utils.generate_task_id()
        future = Runtime().rt.task_mgr.add_task(task_id)

        def callback(response, err=None):
            if future.done():
                return
            if err is not None:
                exception.deal_with_yr_error(future, err)
                return

            if not isinstance(response, core_service_pb2.InvokeResponse):
                resp = core_service_pb2.InvokeResponse()
                resp.ParseFromString(response)
            else:
                resp = response

            if resp.code == 0:
                future.set_result(resp.returnObjectID)
            else:
                exception.deal_with_error(future, resp.code, resp.message)

        if self.cross_language_info is None:
            function_descriptor = ObjectDescriptor.get_from_function(func)
            target_function_key = ""
        else:
            function_descriptor = ObjectDescriptor(function_name=self.cross_language_info.function_name,
                                                   target_language=self.cross_language_info.target_language)
            target_function_key = self.cross_language_info.function_key

        obj_list = ObjRefCountingManager().create_object_ref(task_id, self.return_nums)
        ObjRefCountingManager().join_record(task_id, self.__code_ref__)
        task = Task(task_id=task_id,
                    object_descriptor=function_descriptor,
                    code_id=self.__code_ref__.id if self.__code_ref__ is not None else "",
                    invoke_options=self.invoke_options,
                    args_list=args_list,
                    target_function_key=target_function_key,
                    return_obj_list=obj_list,
                    invoke_type=InvokeType.INVOKE_NORMAL_FUNCTION,
                    trace_id=Runtime().rt.get_trace_id())
        Runtime().rt.submit_task(task, callback)
        return obj_list[0] if self.return_nums == 1 else obj_list


def make_decorator(invoke_options=None, return_nums=None):
    """
    Make decorator for invoke function
    """

    def decorator(func):
        if isinstance(func, types.FunctionType):
            return FunctionProxy(func, invoke_options=invoke_options, return_nums=return_nums)
        raise RuntimeError("@yr.invoke decorator must be applied to a function")

    return decorator


def make_cpp_function_proxy(function_name, function_key):
    """
    Make proxy for invoke cpp function
    """
    return FunctionProxy(None, CrossLanguageInfo(function_name, function_key, LANGUAGE_CPP))
