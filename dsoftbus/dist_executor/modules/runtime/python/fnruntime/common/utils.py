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

"""util common methods"""
import binascii
import sys
import time
import uuid
from typing import List

import yr
import yr.storage.reference_count
from fnruntime import log
from yr.rpc.common_pb2 import Arg
from yr.object_ref import ObjectRef
from yr.serialization import Serialization
from yr.signature import recover_args
from yr.runtime.runtime import Runtime

OBJECT_ID_PREFIX = "yr-api-obj-"
LEN_OBJECT_ID_PREFIX = len(OBJECT_ID_PREFIX)
LEN_OBJECT_ID = LEN_OBJECT_ID_PREFIX + 36
DEFAULT_TIME_OUT = 5 * 60

METADATA_INDEX = 0
ARGS_INDEX = 1


def generate_random_id():
    """
    This is a wrapper generating random id for user functions and objects

    Gets a random id string

    Example: yrobj-433ec3c1-ba11-5a16-ad97-ee4e68db67d5

    Returns:
        Unique uuid string with prefix for user functions and objects
    """
    uuid_str = str(uuid.uuid4())
    return OBJECT_ID_PREFIX + uuid_str


def binary_to_hex(value):
    """
    bytes to hex
    """
    hex_id = binascii.hexlify(value)
    if sys.version_info >= (3, 0):
        hex_id = hex_id.decode()
    return hex_id


def hex_to_binary(hex_id):
    """
    hex to bytes
    """
    return binascii.unhexlify(hex_id)


def get_param(param_args, use_msgpack=False):
    """get param from list"""
    params = Serialization().multi_deserialize([arg.value for arg in param_args], use_msgpack)
    new_params = []
    for param in params:
        if isinstance(param, str) and len(param) == LEN_OBJECT_ID and param.startswith(OBJECT_ID_PREFIX, 0,
                                                                                       LEN_OBJECT_ID_PREFIX):
            new_params.append(param)

    objects = Runtime().rt.get_id(new_params, DEFAULT_TIME_OUT) if len(new_params) > 0 else []
    res = Serialization().multi_deserialize(objects, use_msgpack)

    ref_index = 0
    for i in range(len(params)):
        if isinstance(params[i], str) and len(params[i]) == LEN_OBJECT_ID \
                and params[i].startswith(OBJECT_ID_PREFIX, 0, LEN_OBJECT_ID_PREFIX):
            params[i] = res[ref_index]
            ref_index += 1

    # which means a invocation without place_holder args from cpp runtime
    if use_msgpack:
        return params, {}
    args, kwargs = recover_args(params)
    return args, kwargs


def get_code(code_id):
    """get code from datasystem"""
    log.get_logger().debug(f"code id: {code_id}")
    code = yr.get(ObjectRef(object_id=code_id, need_incre=False, need_decre=False))
    return code


def get_time_offset():
    """
    get time offset
    """
    is_dst = time.daylight and time.localtime().tm_isdst > 0
    return - (time.altzone if is_dst else time.timezone)


def get_formated_time_offset():
    """
    get formatting time offset
    """
    result = "+"
    offset = get_time_offset()
    if offset < 0:
        result = "-"
        offset = -offset

    result += f"%02d:%02d" % (int(offset // 3600), offset % 60)
    return result


def dependency_objref_process(posix_args: List[Arg], is_increase):
    """process dependency obj ref count"""
    ref_ids = []
    for arg in posix_args:
        ref_ids += arg.nested_refs

    log.get_logger().debug(
        f"[Reference Counting] preprocess dependency ref count, object_ids: {ref_ids}, increase: {is_increase} ")

    if is_increase:
        yr.storage.reference_count.increase_reference_count(ref_ids)
    else:
        yr.storage.reference_count.decrease_reference_count(ref_ids)
