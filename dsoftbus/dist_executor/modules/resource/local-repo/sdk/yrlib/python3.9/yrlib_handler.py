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

"""default handler for yr api"""
import traceback
from typing import List

import yr.utils
import yr.runtime.runtime
import yr.storage.reference_count
from fnruntime import log
from fnruntime.common import utils
from fnruntime.common.utils import dependency_objref_process
from fnruntime.handlers.utils import InstanceManager, CodeManager
from yr import apis
from yr import config
from yr.config import ConfigManager
from yr.exception import YRInvokeError
from yr.runtime.task_spec import InvokeType
from yr.runtime.task_spec import TaskMetadata
from yr.serialization import Serialization
from yr.rpc.common_pb2 import Arg

CUSTOM_SIGNAL_EXIT = 64


def _get_serialized_code(code_id):
    code = CodeManager().load(code_id)
    if code is not None:
        return code
    code = utils.get_code(code_id)
    if code is not None:
        CodeManager().register(code_id, code)
    return code


def _instance_create_function(posix_args: List[Arg], metadata: TaskMetadata):
    if metadata.invoke_type == InvokeType.INVALID and metadata.code_id == yr.utils.NORMAL_FUNCTION:
        return False
    if metadata.invoke_type == InvokeType.CREATE_NORMAL_FUNCTION_INSTANCE:
        return False

    log.get_logger().debug(
        f"start to create instance: {metadata.object_descriptor.module_name}.{metadata.object_descriptor.class_name}")
    class_code, use_msgpack = _parse_code(metadata, is_class=True)
    if class_code is None:
        raise RuntimeError("Not found code")
    InstanceManager().class_code = class_code

    args, kwargs = utils.get_param(posix_args[utils.ARGS_INDEX:], use_msgpack)
    dependency_objref_process(posix_args=posix_args, is_increase=False)
    instance = class_code(*args, **kwargs)
    InstanceManager().init(instance)
    return use_msgpack


def _parse_code(metadata: TaskMetadata, is_class=False):
    is_cross_language = False
    if metadata.object_descriptor.src_language != yr.utils.LANGUAGE_PYTHON:
        is_cross_language = True

    if is_cross_language:
        if is_class:
            code_name = metadata.object_descriptor.class_name
        else:
            code_name = metadata.object_descriptor.function_name

        src_language = metadata.object_descriptor.src_language
        if src_language == yr.utils.LANGUAGE_CPP:
            code = CodeManager().get_code_from_local(ConfigManager().code_dir,
                                                     metadata.object_descriptor.module_name,
                                                     code_name)
            if is_class:
                local_code = code.get_original_cls()
            else:
                local_code = code.get_original_func()
            return local_code, True
        raise RuntimeError(f'invalid srcLanguage value, expect srcLanguage= {yr.utils.LANGUAGE_CPP}, '
                           f'actual {src_language}')

    code = _get_serialized_code(metadata.code_id)
    return code, False


def _invoke_function(posix_args: List[Arg]):
    if len(posix_args) == 0:
        return None, False

    metadata = TaskMetadata.parse(posix_args[utils.METADATA_INDEX].value)
    log.get_logger().debug(
        f"start to create/invoke: {metadata.object_descriptor.module_name}.{metadata.object_descriptor.function_name}")

    if metadata.invoke_type == InvokeType.INVALID and metadata.code_id == yr.utils.MEMBER_FUNCTION:
        return _instance_function(posix_args, metadata)
    if metadata.invoke_type == InvokeType.INVOKE_MEMBER_FUNCTION:
        return _instance_function(posix_args, metadata)
    if metadata.invoke_type == InvokeType.GET_NAMED_INSTANCE_METADATA:
        return _get_instance_class_code()
    return _normal_function(posix_args, metadata)


def _normal_function(posix_args: List[Arg], metadata: TaskMetadata):
    code, use_msgpack = _parse_code(metadata)
    if code is None:
        raise RuntimeError("Not found code")

    args, kwargs = utils.get_param(posix_args[utils.ARGS_INDEX:], use_msgpack)
    dependency_objref_process(posix_args=posix_args, is_increase=False)
    result = code(*args, **kwargs)
    return result, use_msgpack


def _instance_function(posix_args: List[Arg], metadata: TaskMetadata):
    instance_function_name = metadata.object_descriptor.function_name
    use_msgpack = metadata.object_descriptor.src_language == yr.utils.LANGUAGE_CPP
    args, kwargs = utils.get_param(posix_args[utils.ARGS_INDEX:], use_msgpack)

    instance = InstanceManager().instance()
    if instance is None:
        raise RuntimeError("No init class instance")

    dependency_objref_process(posix_args=posix_args, is_increase=False)
    result = getattr(instance, instance_function_name)(*args, **kwargs)
    return result, use_msgpack


def _get_instance_class_code():
    class_code = InstanceManager().class_code
    return class_code, False


def init(posix_args: List[Arg], code_dir: str, cross_param=None):
    """init call request"""
    result = None
    if len(posix_args) == 0:
        return result

    metadata = TaskMetadata.parse(posix_args[utils.METADATA_INDEX].value)
    if config.ConfigManager().is_init is not True:
        cfg = config.Config(code_dir=code_dir,
                            on_cloud=True,
                            in_cluster=True,
                            job_id=metadata.job_id,
                            log_level=metadata.log_level,
                            recycle_time=metadata.recycle_time,
                            function_id=metadata.function_id_python,
                            cpp_function_id=metadata.function_id_cpp)
        InstanceManager().config = cfg
        apis.init(cfg)

    use_msgpack = False
    try:
        use_msgpack = _instance_create_function(posix_args, metadata)
    except Exception as err:
        if isinstance(err, YRInvokeError):
            result = YRInvokeError(err.cause, traceback.format_exc())
        else:
            result = YRInvokeError(err, traceback.format_exc())
        log.get_logger().error(f"failed to init, err: {repr(err)} {traceback.format_exc()}")

    if cross_param is not None:
        cross_param.use_msgpack = use_msgpack
    return result


def call(posix_args: List[Arg], cross_param=None):
    """call request"""
    if len(posix_args) == 0:
        log.get_logger().debug("yrlib call, get 0 args from request.")
        return None
    use_msgpack = False
    try:
        result, use_msgpack = _invoke_function(posix_args)
    except Exception as err:
        if isinstance(err, YRInvokeError):
            result = YRInvokeError(err.cause, traceback.format_exc())
        else:
            result = YRInvokeError(err, traceback.format_exc())
        log.get_logger().error(f"failed to call, err: {repr(err)} {traceback.format_exc()}")

    if cross_param is not None:
        cross_param.use_msgpack = use_msgpack
    return result


def checkpoint(checkpoint_id: str) -> bytes:
    """check point"""
    log.get_logger().info("start to checkpoint")
    instance = InstanceManager().instance()
    try:
        result = Serialization().serialize((instance, InstanceManager().config), "", False)
    except TypeError as e:
        log.get_logger().exception(e)
        return bytes()
    return result.data


def recover(state: bytes):
    """recover state"""
    log.get_logger().info('start to recover state')
    if len(state) == 0:
        log.get_logger().error("no instance when try to recover")
        raise RuntimeError("No recover state")
    instance, cfg = Serialization().deserialize(state)
    InstanceManager().init(instance)
    apis.init(cfg)


def shutdown(grace_period_second: int):
    """shutdown"""
    apis.finalize()


def signal(signal_num: int):
    """
    signal
    yr-api custom signal handler
    """
    if signal_num == CUSTOM_SIGNAL_EXIT:
        apis.finalize()
        apis.exit()
