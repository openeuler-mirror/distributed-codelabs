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
"""Local object store"""
from yr.serialization import Serialization
from yr.utils import Singleton


@Singleton
class LocalObjectStore:
    """Local object store"""
    _object_map = {}

    def put(self, key, value):
        """put object"""
        value = Serialization().serialize(value).data
        self._object_map[key] = value

    def get(self, key):
        """get object"""
        if isinstance(key, list):
            value = [self._object_map.get(k) for k in key]
            return Serialization().multi_deserialize(value)
        return Serialization().deserialize(self._object_map.get(key))

    def release(self, key):
        """release keys"""
        keys = key if isinstance(key, list) else [key]
        for i in keys:
            self._object_map.pop(i)

    def release_all(self):
        """release all keys"""
        self._object_map.clear()
