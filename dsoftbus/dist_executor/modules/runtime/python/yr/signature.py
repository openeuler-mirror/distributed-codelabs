#!/usr/bin/env python
# coding=utf-8
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
"""signature"""
import inspect

PLACEHOLDER = b"__YR_PLACEHOLDER__"


def get_signature(func, ignore_first=False):
    """get func signature"""
    if not ignore_first:
        return inspect.signature(func)
    return inspect.Signature(parameters=list(inspect.signature(func).parameters.values())[1:])


def package_args(signature, args, kwargs):
    """check and package args to a list"""
    if signature is not None:
        try:
            signature.bind(*args, **kwargs)
        except TypeError as e:
            raise TypeError(str(e)) from None

    args_list = []
    for arg in args:
        args_list += [PLACEHOLDER, arg]

    for key, value in kwargs.items():
        args_list += [key, value]
    return args_list


def recover_args(args_list):
    """recover args from list"""
    args = []
    kwargs = {}
    for i in range(0, len(args_list), 2):
        key, value = args_list[i], args_list[i + 1]
        if key == PLACEHOLDER:
            args.append(value)
        else:
            kwargs[key] = value

    return args, kwargs
