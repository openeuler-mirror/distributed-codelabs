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

"""Function manager"""
import sys
import traceback
from dataclasses import dataclass
from types import ModuleType

import yr.objref_counting_manager
import yr.storage.reference_count
from fnruntime import log
from fnruntime.common.utils import binary_to_hex, dependency_objref_process
from fnruntime.handlers import utils
from fnruntime.rpc.sdk import post_shutdown
from yr.exception import YRError, YRInvokeError
from yr.rpc import common_pb2 as common
from yr.rpc import core_service_pb2 as core
from yr.rpc import runtime_service_pb2 as rt
from yr.runtime.runtime import Runtime
from yr.serialization import Serialization
from yr.object_ref import ObjectRef


def _set_layer_path(layer_paths):
    """
    Set layer code path
    """
    for layer_path in layer_paths:
        log.get_logger().debug("add layer path %s to system", layer_path)
        sys.path.insert(1, layer_path)


@dataclass
class CrossLanguageParam:
    """
    CrossLanguageParam
    """
    use_msgpack: bool


@dataclass
class HookFunction:
    """
    Hook Function, including the module and the hook name
    """
    module: ModuleType = None
    entry: str = None


_INIT = "INIT_HANDLER"
_CALL = "CALL_HANDLER"
_CHECKPOINT = "CHECKPOINT_HANDLER"
_RECOVER = "RECOVER_HANDLER"
_SHUTDOWN = "SHUTDOWN_HANDLER"
_SIGNAL = "SIGNAL_HANDLER"


def get_hooks_key():
    """get all hooks key"""
    return [_INIT, _CALL, _CHECKPOINT, _RECOVER, _SHUTDOWN, _SIGNAL]


def _process_result(request: rt.CallRequest, use_msgpack, result):
    """process call result"""
    object_id_list = request.returnObjectIDs
    result_list = result
    if len(object_id_list) == 0:
        return common.ERR_INNER_SYSTEM_ERROR, f"object list is null, requestID: {request.requestID}"

    if len(object_id_list) == 1 or isinstance(result, YRInvokeError):
        result_list = [result] * len(object_id_list)

    log.get_logger().debug(f"[Reference Counting] put ret_val, object_ids: {object_id_list}")
    for (object_id, res) in zip(object_id_list, result_list):
        ref = ObjectRef(object_id=object_id, need_incre=False, need_decre=False)
        try:
            Runtime().rt.put(ref, res, use_msgpack)
        except (RuntimeError, TypeError, ValueError) as e:
            return common.ERR_INNER_SYSTEM_ERROR, f"put object failed, " \
                                                  f"id: {object_id}, requestID:{request.requestID}, err: {e}"

    return common.ERR_NONE, ""


class FunctionManager:
    """Function manager"""

    def __init__(self, instance_id, init_hooks=True):
        self._logic_instance_id = instance_id
        self._custom_handler = None
        self._module = None
        self._code_dir = None
        self._init_err = None
        self.function_kind = None
        self._init_hooks = init_hooks

        self._hooks = {}

    @staticmethod
    def process_obj_ref(request: rt.CallRequest):
        """preprocess for call request, mainly handle the obj ref counting"""
        code = common.ERR_NONE
        message = None

        log.get_logger().debug("[Reference Counting] Increase obj reference counting in call preprocess.")
        try:
            dependency_objref_process(request.args, is_increase=True)
        except RuntimeError as err:
            code = common.ERR_INNER_SYSTEM_ERROR
            message = f"fail to process (increase) object reference counting. " \
                      f"requestID:{request.requestID}, err: {err}, Traceback: {traceback.format_exc()}"
            log.get_logger().exception(message)

        return rt.CallResponse(code=code, message=message)

    def init(self, python_path, code_dir="", layer_paths=None, hooks=None):
        """
        init function manager
        """
        sys.path.insert(1, python_path)
        sys.path.insert(1, code_dir)
        self._code_dir = code_dir

        if layer_paths is not None:
            for i in layer_paths:
                sys.path.insert(1, i)

        if self._init_hooks:
            self._load_hooks(hooks)

    def call(self, request: rt.CallRequest):
        """call request"""
        if Runtime().rt is not None:
            Runtime().rt.set_trace_id(request.traceID)

        cross_param = CrossLanguageParam(use_msgpack=False)
        code, message, result = self._invoke_call_request(request, cross_param)

        if not request.isCreate and code == common.ERR_NONE:
            if self.function_kind == 'yrlib':
                # yrlib requires to put the result to datasystem
                code, message = _process_result(request, cross_param.use_msgpack, result)
            else:
                # faas promise the result is a str, has been converted and checked
                message = result

        # If result is a YRError instance, should also be treated as an error
        if code == common.ERR_NONE and isinstance(result, YRError):
            code, message = _package_yr_error(result=result, cross_param=cross_param)

        return core.CallResult(code=code, message=message, instanceID=request.senderID,
                               requestID=request.requestID)

    def checkpoint(self, request: rt.CheckpointRequest) -> rt.CheckpointResponse:
        """checkpoint request"""
        log.get_logger().info(f"begin checkpoint, id: {request.checkpointID}")
        code, message, result = self._invoke_hook(_CHECKPOINT,
                                                  request.checkpointID)
        return rt.CheckpointResponse(code=code, message=message, state=result)

    def recover(self, request: rt.RecoverRequest) -> rt.RecoverResponse:
        """recover request"""
        log.get_logger().info("begin to recover")
        code, message, _ = self._invoke_hook(_RECOVER,
                                             request.state)
        return rt.RecoverResponse(code=code, message=message)

    def shutdown(self, request) -> rt.ShutdownResponse:
        """shutdown request"""
        log.get_logger().info(f"begin shutdown, exit period second: {request.gracePeriodSecond}")
        yr.objref_counting_manager.ObjRefCountingManager().wait_invoke_ack()
        code, message, _ = self._invoke_hook(_SHUTDOWN,
                                             request.gracePeriodSecond)
        post_shutdown()
        return rt.ShutdownResponse(code=code, message=message)

    def signal(self, request: rt.SignalRequest) -> rt.SignalResponse:
        """kill request"""
        log.get_logger().info(f"receive the signal: {request.signal}")
        code, message, _ = self._invoke_hook(_SIGNAL,
                                             request.signal)
        return rt.SignalResponse(code=code, message=message)

    def set_init_exception(self, err, trace):
        """set init exception"""
        self._init_err = YRInvokeError(err, trace)

    def _load_hooks(self, hooks):
        """
        Will import and store user defined hooks to self._hooks
        If some hooks are not defined in environment,
        but custom_handler is defined and successfully imported,
        will try to use the method in self._module.
        Throw error if miss any hooks.
        """

        def _log_and_throw(_hook_name):
            error_info = f'failed to import handler \"{_hook_name}\"'
            log.get_logger().error(error_info)
            raise RuntimeError(error_info)

        all_hooks = [_INIT, _CALL, _CHECKPOINT, _RECOVER, _SHUTDOWN, _SIGNAL]
        for hook_name in all_hooks:
            if hook_name in hooks:
                user_hook_module, user_hook_name = self._import_hook(hooks[hook_name], self._code_dir)
                if user_hook_module is None:
                    _log_and_throw(hook_name)
                self._hooks[hook_name] = HookFunction(user_hook_module, user_hook_name)
            elif hook_name in [_INIT, _CALL]:
                _log_and_throw(hook_name)

    def _invoke_call_request(self, request, cross_param):
        """invoke user's hook, init or call"""
        if self._init_err is not None:
            code = common.ERR_USER_CODE_LOAD  # Not common.ERR_NONE
            message = str(self._init_err)
            result = None
            return code, message, result

        if request.isCreate:
            code, message, user_result = self._invoke_hook(_INIT,
                                                           request.args,
                                                           cross_param=cross_param,
                                                           code_dir=self._code_dir)
        else:
            code, message, user_result = self._invoke_hook(_CALL,
                                                           request.args,
                                                           cross_param=cross_param)
        try:
            result = _check_return_list(request, user_result)
        except (TypeError, ValueError) as e:
            result = YRInvokeError(e, traceback.format_exc())
        return code, message, result

    def _import_hook(self, hook_handler, code_dir):
        hook_args = hook_handler.split(".")
        log.get_logger().debug(f"import handler {hook_handler}")
        if len(hook_args) != 2:
            raise ValueError(f"{hook_handler} doesn't satisfy pattern 'module_name.entry_name'")
        module_name = hook_args[0]
        function_name = hook_args[1]
        if module_name == "yrlib_handler":
            self.function_kind = "yrlib"

        module = utils.CodeManager().load_module(code_dir, module_name)
        if hasattr(module, function_name) is False:
            error_info = f"failed to found {function_name} attribute in {module}"
            log.get_logger().error(error_info)
            raise RuntimeError(error_info)
        return module, function_name

    def _invoke_hook(self, hook_name, *args, **kwargs) -> (int, str, object):
        """
        Invoke hooks, hook signature expectation:
            init(*args, **kwargs) -> object
            call(*args, **kwargs) -> object
            checkpoint(checkpointID: str) -> bytes
            recover(state: bytes) -> None
            shutdown(gracePeriodSecond: int) -> None
            signal(signal: int, payload: bytes) -> None
        either return single object/bytes or None, the return value will be named "result"
        invoke hook will return (code, message, result)
        """
        # if invoke failed, message should be encoded by pickle
        code = common.ERR_NONE
        message = None
        result = None

        if hook_name not in self._hooks and \
                hook_name not in [_INIT, _CALL]:
            # didn't specify some optional hook, do nothing, just return
            return code, message, result

        hook_function = self._hooks.get(hook_name, HookFunction())
        module, entry = hook_function.module, hook_function.entry
        if module is None or entry is None:
            code = common.ERR_USER_CODE_LOAD
            message = f"failed to invoke hook function '{hook_name}', module: {module}, entry: {entry}"
            log.get_logger().exception(message)
            return code, message, result

        try:
            call = getattr(module, entry)
        except AttributeError:
            code = common.ERR_USER_CODE_LOAD
            message = f"failed to found {entry} attribute in {module}"
            log.get_logger().exception(message)
            return code, message, result

        try:
            result = call(*args, **kwargs)
        except Exception as err:
            code = common.ERR_USER_FUNCTION_EXCEPTION
            message = f"failed to invoke {entry} in {module},err: {err}, Traceback: {traceback.format_exc()}"
            log.get_logger().exception(message)

        return code, message, result


def _check_return_list(request, result):
    if isinstance(result, Exception):
        return result
    if len(request.returnObjectIDs) > 1:
        if not hasattr(result, "__len__"):
            raise TypeError(f"cannot unpack non-iterable {type(result)} object")
        if hasattr(result, "__len__") and len(result) != len(request.returnObjectIDs):
            raise ValueError(f"not enough values to unpack (expected {len(request.returnObjectIDs)}, "
                             f"got {len(result)})")
    return result


def _package_yr_error(result, cross_param):
    code = common.ERR_USER_FUNCTION_EXCEPTION
    message = None
    if not cross_param.use_msgpack:
        code = common.ERR_USER_FUNCTION_EXCEPTION
        message = binary_to_hex(Serialization().serialize(result, "",  False).data)

    if cross_param.use_msgpack:
        code = common.ERR_USER_FUNCTION_EXCEPTION
        message = str(result)
    return code, message
