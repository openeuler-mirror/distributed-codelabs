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
Instance decorator
"""

import concurrent.futures
import inspect
import logging
import weakref
from functools import partial

import yr
from yr import exception
from yr import signature
from yr import utils
from yr.common.response_helper import make_callback_for_create
from yr.config import ConfigManager
from yr.exception import deal_with_yr_error
from yr.instance_ref import InstanceRef
from yr.objref_counting_manager import ObjRefCountingManager
from yr.rpc.core_service_pb2 import InvokeResponse
from yr.runtime.runtime import Runtime
from yr.runtime.task_spec import InvokeType, Task, InvokeOptions
from yr.serialization import register_unpack_hook, register_pack_hook
from yr.storage.reference_count import ReferenceCount

_logger = logging.getLogger(__name__)


class InstanceCreator:
    """
    User instance creator
    """

    def __init__(self):
        self.__user_class__ = None
        self.__user_class_descriptor__ = None
        self.__user_class_methods__ = {}
        self.__code_ref__ = None
        self.__invoke_options__ = None
        self.__need_in_order__ = False
        self.__target_function_key__ = None

    @classmethod
    def create_from_user_class(cls, user_class, invoke_options):
        """
        Create from user class
        """

        class DerivedInstanceCreator(cls, user_class):
            pass

        name = f"YRInstance({user_class.__name__})"
        DerivedInstanceCreator.__module__ = user_class.__module__
        DerivedInstanceCreator.__name__ = name
        DerivedInstanceCreator.__qualname__ = name
        self = DerivedInstanceCreator.__new__(DerivedInstanceCreator)
        self.__user_class__ = user_class
        self.__invoke_options__ = invoke_options
        if invoke_options is not None:
            self.__need_in_order__ = invoke_options.need_order()
        else:
            self.__need_in_order__ = True
        self.__user_class_descriptor__ = utils.ObjectDescriptor.get_from_class(user_class)
        self.__user_class_descriptor__.target_language = utils.LANGUAGE_PYTHON
        class_methods = inspect.getmembers(user_class,
                                           utils.is_function_or_method)
        self.__user_class_methods__ = dict(class_methods)
        self.__code_ref__ = None

        function_urn = ConfigManager().get_function_id_by_language(utils.LANGUAGE_PYTHON)
        self.__target_function_key__ = None
        if function_urn is not None:
            self.__target_function_key__ = utils.get_function_from_urn(function_urn)
        return self

    @classmethod
    def create_cpp_user_class(cls, cpp_class):
        """
        Create a cpp user class
        """
        self = cls()
        self.__user_class__ = None
        self.__user_class_descriptor__ = utils.ObjectDescriptor(class_name=cpp_class.get_class_name(),
                                                                function_name=cpp_class.get_factory_name(),
                                                                target_language=utils.LANGUAGE_CPP)
        self.__user_class_methods__ = None
        self.__code_ref__ = None
        self.__target_function_key__ = cpp_class.get_function_key()
        return self

    def options(self, invoke_options: InvokeOptions):
        """
        Set user invoke options
        Args:
            invoke_options: invoke options for users to set resources
        """
        self.__invoke_options__ = invoke_options
        self.__need_in_order__ = invoke_options.need_order()
        return self

    def get_original_cls(self):
        """
        The original class
        """
        return self.__user_class__

    def invoke(self, *args, **kwargs):
        """
        Create a instance in cluster
        """
        return self._invoke(args=args, kwargs=kwargs)

    def _get_designated_instance_id(self, task_id):
        instance_id = None
        if self.__invoke_options__:
            designated_instance_id = self.__invoke_options__.build_designated_instance_id()
            instance_id = InstanceRef(designated_instance_id, task_id)
        return instance_id

    def _invoke(self, args=None, kwargs=None):
        is_cpp_invoke = self.__user_class_descriptor__.target_language == utils.LANGUAGE_CPP
        if not is_cpp_invoke and not ReferenceCount().is_obj_in_ctx(self.__code_ref__):
            self.__code_ref__ = yr.put(self.__user_class__)
            _logger.info(f"[Reference Counting] put code with id = {self.__code_ref__.id}, "
                         f"className={self.__user_class_descriptor__.class_name}")
        # __init__ existed when user-defined
        if self.__user_class_methods__ is not None and '__init__' in self.__user_class_methods__:
            sig = signature.get_signature(self.__user_class_methods__.get('__init__'),
                                          ignore_first=True)
        else:
            sig = None
        args_list = signature.package_args(sig, args, kwargs)

        future = concurrent.futures.Future()

        task_id = utils.generate_task_id()
        instance_id = self._get_designated_instance_id(task_id)
        ObjRefCountingManager().join_record(task_id, self.__code_ref__)
        Runtime().rt.submit_task(
            Task(task_id=task_id,
                 object_descriptor=self.__user_class_descriptor__,
                 code_id=self.__code_ref__.id if self.__code_ref__ is not None else "",
                 invoke_options=self.__invoke_options__,
                 args_list=args_list,
                 target_function_key=self.__target_function_key__,
                 invoke_type=InvokeType.CREATE_INSTANCE,
                 trace_id=Runtime().rt.get_trace_id(),
                 instance_id=instance_id),
            make_callback_for_create(future))

        ref = InstanceRef(future, task_id, is_user_defined_id=instance_id is not None)

        need_order = True
        if self.__invoke_options__:
            need_order = self.__invoke_options__.need_order()

        return InstanceProxy(ref,
                             self.__user_class_descriptor__,
                             self.__user_class_methods__,
                             self.__target_function_key__,
                             need_order)


class InstanceProxy:
    """
    Use to decorate a user class
    """

    def __init__(self, instance_id, class_descriptor, class_methods, target_function_key, need_order=True):
        self._class_descriptor = class_descriptor
        self.instance_id = instance_id
        self._class_methods = class_methods
        self._method_descriptor = {}
        self.task_serial_num = 0
        self.__instance_activate__ = True
        self.__target_function_key__ = target_function_key
        self.need_order = need_order

        if self._class_methods is not None:
            for method_name, value in self._class_methods.items():
                function_descriptor = utils.ObjectDescriptor(module_name=self._class_descriptor.module_name,
                                                             function_name=method_name,
                                                             class_name=self._class_descriptor.class_name)
                self._method_descriptor[method_name] = function_descriptor
                sig = signature.get_signature(value, ignore_first=True)
                return_nums = value.__return_nums__ if hasattr(value, "__return_nums__") else 1
                method = MethodProxy(self, self.instance_id,
                                     self._method_descriptor.get(method_name),
                                     sig, return_nums)
                setattr(self, method_name, method)

    def __getattr__(self, method_name):
        if self._class_descriptor.target_language == utils.LANGUAGE_PYTHON:
            raise AttributeError(f"'{self._class_descriptor.class_name}' object has "
                                 f"no attribute '{method_name}'")

        function_name = method_name
        if self._class_descriptor.target_language == utils.LANGUAGE_CPP:
            function_name = "&" + self._class_descriptor.class_name + "::" + method_name
        method_descriptor = utils.ObjectDescriptor(module_name=self._class_descriptor.module_name,
                                                   function_name=function_name,
                                                   class_name=self._class_descriptor.class_name,
                                                   target_language=self._class_descriptor.target_language)
        return MethodProxy(
            self,
            self.instance_id,
            method_descriptor,
            None,
            1)

    def __reduce__(self):
        state = self.serialization_(False)
        return InstanceProxy.deserialization_, (state,)

    @classmethod
    def deserialization_(cls, state):
        """
        deserialization to rebuild instance proxy
        """
        class_method = None
        if utils.CLASS_METHOD in state:
            class_method = state[utils.CLASS_METHOD]
        target_function_key = None
        if utils.FUNCTION_KEY in state:
            target_function_key = state[utils.FUNCTION_KEY]
        function_name = state[utils.FUNC_NAME] if utils.FUNC_NAME in state else ""
        need_order = state[utils.NEED_ORDER] if utils.NEED_ORDER in state else True
        return cls(InstanceRef(state[utils.INSTANCE_ID]),
                   utils.ObjectDescriptor(state[utils.MODULE_NAME], state[utils.CLASS_NAME],
                                          function_name, state[utils.TARGET_LANGUAGE]),
                   class_method,
                   target_function_key,
                   need_order)

    def serialization_(self, is_cross_language: False):
        """
        serialization of instance proxy
        """
        info_ = {utils.INSTANCE_ID: self.instance_id.id}
        if is_cross_language is False:
            info_[utils.CLASS_METHOD] = self._class_methods

        info_[utils.FUNCTION_KEY] = \
            self.__target_function_key__ if self.__target_function_key__ is not None else ""
        info_[utils.NEED_ORDER] = self.need_order
        self._class_descriptor.to_dict()
        state = {**info_, **self._class_descriptor.to_dict()}
        return state

    def terminate(self):
        """
        Terminate the instance
        """
        if not Runtime().rt or not self.is_activate():
            return

        if self.need_order and self.instance_id.is_user_defined_id:
            serial_manager = Runtime().rt.get_serial_manager()
            serial_manager.del_instance_serial_num(self.instance_id.id)

        Runtime().rt.kill_instance(self.instance_id)
        self.__instance_activate__ = False
        _logger.info(f"{self.instance_id} is terminated")

    def is_activate(self):
        """
        Return the instance status
        """
        return self.__instance_activate__

    def get_function_key(self):
        """
        Return the target function key
        """
        return self.__target_function_key__


@register_pack_hook
def msgpack_encode_hook(obj):
    """
    register msgpack encode hook
    """
    if isinstance(obj, InstanceProxy):
        return obj.serialization_(True)
    return obj


@register_unpack_hook
def msgpack_decode_hook(obj):
    """
    register msgpack decode hook
    """
    if utils.INSTANCE_ID in obj:
        return InstanceProxy.deserialization_(obj)
    return obj


class MethodProxy:
    """
    Use to decorate a user class method
    """

    def __init__(self, instance, instance_id, method_descriptor, sig, return_nums=1):
        self._instance_ref = weakref.ref(instance)
        self._instance_id = instance_id
        self._method_descriptor = method_descriptor
        self._signature = sig
        self.__target_function_key__ = instance.get_function_key()
        self._return_nums = return_nums
        if return_nums < 0 or return_nums > 100:
            raise RuntimeError(f"invalid return_nums: {return_nums}, should be an integer between 0 and 100")

    def invoke(self, *args, **kwargs):
        """
        invoke a class method in cluster
        """
        return self._invoke(args, kwargs)

    def _invoke(self, args, kwargs):
        if not self._instance_ref().is_activate():
            raise RuntimeError("this instance is terminated")
        args_list = signature.package_args(self._signature, args, kwargs)

        if self._instance_ref().need_order:
            if self._instance_id.is_user_defined_id:
                serial_manager = Runtime().rt.get_serial_manager()
                task_serial_num = serial_manager.get_serial_number(self._instance_id.id)
            else:
                task_serial_num = self._instance_ref().task_serial_num
                self._instance_ref().task_serial_num += 1

            runtime_id = ConfigManager().runtime_id
            task_id = utils.generate_task_id_with_serial_num(runtime_id, task_serial_num)
        else:
            task_id = utils.generate_task_id()

        future = Runtime().rt.task_mgr.add_task(task_id)
        callback = partial(deal_with_response, future)

        obj_list = ObjRefCountingManager().create_object_ref(task_id, self._return_nums)
        task = Task(task_id=task_id,
                    object_descriptor=self._method_descriptor,
                    instance_id=self._instance_id,
                    args_list=args_list,
                    code_id=utils.MEMBER_FUNCTION,
                    target_function_key=self.__target_function_key__,
                    invoke_type=InvokeType.INVOKE_MEMBER_FUNCTION,
                    return_obj_list=obj_list)
        Runtime().rt.submit_task(task, callback)
        return obj_list[0] if self._return_nums == 1 else obj_list


def make_decorator(invoke_options=None):
    """
    Make decorator for invoke function
    """

    def decorator(cls):
        if inspect.isclass(cls):
            return InstanceCreator.create_from_user_class(cls, invoke_options)
        raise RuntimeError("@yr.instance decorator must be applied to a class")

    return decorator


def make_cpp_instance_creator(cpp_class):
    """
    Make cpp_instance creator for invoke function
    """

    return InstanceCreator.create_cpp_user_class(cpp_class)


def get_instance_by_name(name, namespace):
    """
    Get instance proxy by name and namespace
    """

    task_id = utils.generate_task_id()
    future = Runtime().rt.task_mgr.add_task(task_id)
    callback = partial(deal_with_response, future)

    named_instance_id = namespace + "-" + name if namespace else name
    named_instance_ref = InstanceRef(named_instance_id, task_id, is_user_defined_id=True)

    target_function_key = None
    function_urn = ConfigManager().get_function_id_by_language(
        utils.LANGUAGE_PYTHON)
    if function_urn is not None:
        target_function_key = utils.get_function_from_urn(function_urn)
    obj_list = ObjRefCountingManager().create_object_ref(task_id)

    task = Task(task_id=task_id,
                object_descriptor=utils.ObjectDescriptor(),
                instance_id=named_instance_ref,
                args_list=[],
                code_id="",
                target_function_key=target_function_key,
                invoke_type=InvokeType.GET_NAMED_INSTANCE_METADATA,
                return_obj_list=obj_list)
    Runtime().rt.submit_task(task, callback)

    user_class = yr.get(obj_list[0])
    if not inspect.isclass(user_class):
        raise RuntimeError("get class metadata error")

    user_class_descriptor = utils.ObjectDescriptor.get_from_class(user_class)
    user_class_descriptor.target_language = utils.LANGUAGE_PYTHON
    class_methods = inspect.getmembers(user_class,
                                       utils.is_function_or_method)
    user_class_methods = dict(class_methods)

    return InstanceProxy(named_instance_ref, user_class_descriptor,
                         user_class_methods, target_function_key)


def deal_with_response(future, data, err=None):
    """
    As a callback function to deal with the response
    """

    if err is not None:
        deal_with_yr_error(future, err)
        return
    if isinstance(data, InvokeResponse):
        resp = data
    else:
        resp = InvokeResponse()
        resp.ParseFromString(data)
    if resp.code == 0:
        future.set_result(resp.returnObjectID)
    else:
        exception.deal_with_error(future, resp.code, resp.message)
