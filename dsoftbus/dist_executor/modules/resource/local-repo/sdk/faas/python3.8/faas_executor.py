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

"""Faas executor, an adapter between posix and faas"""
from dataclasses import dataclass
import json
import traceback

from fnruntime import log
from fnruntime.handlers.utils import CodeManager

_KEY_USER_INIT_ENTRY = "userInitEntry"
_KEY_USER_CALL_ENTRY = "userCallEntry"

_INDEX_INIT_USER_ENTRIES = 0
_INDEX_CALL_CONTEXT = 0
_INDEX_CALL_USER_EVENT = 1


@dataclass
class FaasContext:
    """Faas context"""
    logger: object
    request_id: str


def faasInitHandler(posix_args, code_dir: str, *args, **kwargs) -> None:
    """
    raise error if fail
    """
    log.get_logger().debug("Faas init handler called.")
    user_handlers = json.loads(posix_args[_INDEX_INIT_USER_ENTRIES].value)
    log.get_logger().debug("Faas init handler extract user_handler: %s.", user_handlers)

    user_init_hook = user_handlers.get(_KEY_USER_INIT_ENTRY, None)
    user_call_hook = user_handlers.get(_KEY_USER_CALL_ENTRY, None)

    # Load and run user init code
    if user_init_hook is not None and len(user_init_hook) != 0:
        try:
            user_init_code = _load_module_and_entry(user_init_hook, code_dir)
        except (ValueError, ImportError, RuntimeError) as err:
            log.get_logger().error("faas failed to import user code. err: %s, traceback: %s",
                                   err, traceback.format_exc())
            raise RuntimeError(f"faas failed to import user code. err: {err}") from err
        else:
            CodeManager().register(_KEY_USER_INIT_ENTRY, user_init_code)
            try:
                user_init_code()
            except Exception as err:
                log.get_logger().exception("Fail to run user init handler. err: %s. "
                                           "traceback: %s", err, traceback.format_exc())
                raise

    # Load user call code, no run
    if user_call_hook is not None and len(user_call_hook) != 0:
        user_call_code = _load_module_and_entry(user_call_hook, code_dir)
        CodeManager().register(_KEY_USER_CALL_ENTRY, user_call_code)


def faasCallHandler(posix_args, *args, **kwargs) -> str:
    """faas call handler"""
    user_code = CodeManager().load(_KEY_USER_CALL_ENTRY)
    if user_code is None:
        err_msg = "Faas executor find empty user call code."
        log.get_logger().error(err_msg)
        raise RuntimeError(err_msg)

    event = json.loads(posix_args[_INDEX_CALL_USER_EVENT].value)

    # This is a "placeholder" context, will implement in future
    context = FaasContext(logger=log.get_logger(), request_id="")

    try:
        result = user_code(event, context)
    except Exception as err:
        err_msg = f"Fail to run user call handler. err: {err}. traceback: {traceback.format_exc()}"
        log.get_logger().exception(err_msg)
        raise

    try:
        result_str = transform_response_to_str(result)
    except Exception as err:
        # Can be RecursionError, RuntimeError, UnicodeError, MemoryError, etc...
        err_msg = f"Fail to stringify user call result. " \
                  f"err: {err}. traceback: {traceback.format_exc()}"
        log.get_logger().exception(err_msg)
        raise RuntimeError(err_msg) from err

    return result_str


# 按照一起打包的方案，以下方法faas暂不需要，当前留空


def faasCheckPointHandler(check_point_id: str = "") -> bytes:
    """faas checkpoint handler, leave empty"""
    return bytes()


def faasRecoverHandler(state: bytes = None):
    """faas recover handler, leave empty"""


def faasShutDownHandler(grace_period_second: int = -1):
    """faas shutdown handler, leave empty"""


def faasSignalHandler(signal_num: int = -1, payload: bytes = None):
    """faas signal handler, leave empty"""


# Helpers
def transform_response_to_str(response):
    """Method transform_response_to_str"""
    result = None
    if response is None:
        result = ""
    elif isinstance(response, dict) or is_instance_type(response):
        result = to_json_string(response)
    else:
        result = str(response)
    return result


def convert_obj_to_json(obj):
    """Method convert_obj_to_json"""
    return obj.__dict__


def is_instance_type(obj):
    """Method is_instance_type"""
    return hasattr(obj, '__dict__')


def to_json_string(obj, indent=None, sort_keys=False):
    """Method to_json_string"""
    if isinstance(obj, dict):
        return json.dumps(obj, indent=indent, sort_keys=sort_keys)
    return json.dumps(obj, indent=indent, default=convert_obj_to_json, sort_keys=sort_keys)


def _load_module_and_entry(user_hook, code_dir):
    """load module and the entry code, throw RuntimeError if failed."""
    log.get_logger().debug("Faas load module and entry [%s] from [%s]", user_hook, code_dir)
    user_hook_splits = user_hook.rsplit(".", maxsplit=1) if isinstance(user_hook, str) else None
    if len(user_hook_splits) != 2:
        raise RuntimeError("User hook not satisfy requirement, expect: xxx.xxx")

    user_module, user_entry = user_hook_splits[0], user_hook_splits[1]
    log.get_logger().debug("User module: %s, entry: %s", user_module, user_entry)

    try:
        user_code = CodeManager().get_code_from_local(code_dir, user_module, user_entry)
    except ValueError as err:
        log.get_logger().error(f"Missing user module. {user_hook}")
        raise RuntimeError(f"Missing user module. {user_hook}") from err

    if user_code is None:
        log.get_logger().error(f"Missing user entry. {user_hook}")
        raise RuntimeError(f"Missing user entry. {user_hook}")

    return user_code
