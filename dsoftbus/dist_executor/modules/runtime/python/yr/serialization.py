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
"""Serialization"""
import threading
from dataclasses import dataclass
from dataclasses import field
from typing import Set, List, Union
import pickle

import msgpack
import cloudpickle
from cloudpickle import CloudPickler
from datasystem import object_cache

from yr.utils import Singleton
from yr.object_ref import ObjectRef
from yr.storage import reference_count
from yr.serialize.serialization import Pickle5Writer, unpack_pickle5_buffers

OBJECT_TYPE_NORMAL = 0
OBJECT_TYPE_PYTHON = 1
YR_ZEROCOPY_PREFIX = b'yr-zerocopy!'
global_thread_local = threading.local()


@dataclass
class SerializeObject:
    """
    SerializeObject
    """
    object_type: int
    data: bytes
    refs: Set[ObjectRef] = field(default_factory=set)
    origin_object: object = None
    writer: object = None
    inband: object = None


def _object_ref_deserializer(object_id):
    ref = ObjectRef(object_id=object_id, need_incre=False)
    global_thread_local.object_refs.append(ref)
    return ref


@Singleton
class Serialization:
    """serialization context"""
    _lock = threading.Lock()
    _protocol = None
    _msgpack_serialize_hooks = []
    _msgpack_deserialize_hook = []

    def __init__(self):
        self._thread_local = threading.local()

        def object_ref_reducer(obj: ObjectRef):
            global_thread_local.object_refs.append(obj)
            return _object_ref_deserializer, (obj.id,)

        CloudPickler.dispatch[ObjectRef] = object_ref_reducer

    @staticmethod
    def _serialize_to_pickle5(value):
        writer = Pickle5Writer()
        try:
            inband = cloudpickle.dumps(
                value, protocol=5, buffer_callback=writer.buffer_callback)
        except Exception as e:
            raise e
        return writer, inband

    @staticmethod
    def _deserialize_pickle5_data(data):
        try:
            in_band, buffers = unpack_pickle5_buffers(data)
        except ValueError as e:
            raise e
        try:
            if len(buffers) > 0:
                obj = cloudpickle.loads(in_band, buffers=buffers)
            else:
                obj = cloudpickle.loads(in_band)
        except cloudpickle.pickle.PicklingError as e:
            raise e
        except EOFError as e:
            raise ValueError("EOFError") from e
        return obj

    def serialize(self, value, ref="", zero_copy=False, use_msgpack=False) -> SerializeObject:
        """serialize"""
        if use_msgpack:
            return SerializeObject(object_type=OBJECT_TYPE_NORMAL, data=msgpack.packb(value, default=self._pack))

        global_thread_local.object_refs = []
        if zero_copy and ref != "":
            writer, inband = self._serialize_to_pickle5(value)
            data = cloudpickle.dumps(ref, self._protocol)
        else:
            data = cloudpickle.dumps(value, self._protocol)
        refs = set(global_thread_local.object_refs)
        global_thread_local.object_refs = []
        if zero_copy and ref != "":
            return SerializeObject(object_type=OBJECT_TYPE_PYTHON, data=data, refs=refs, origin_object=value,
                                   writer=writer,
                                   inband=inband)
        return SerializeObject(object_type=OBJECT_TYPE_PYTHON, data=data, refs=refs, origin_object=value, writer=None,
                               inband=None)

    def deserialize(self, value: bytes, use_msgpack=False):
        """deserialize"""
        result = self.multi_deserialize([value], use_msgpack)
        return result[0]

    def multi_deserialize(self, values: List[Union[bytes, object_cache.Buffer]], use_msgpack=False):
        """deserialize a list of value"""
        if use_msgpack:
            return [msgpack.unpackb(value, object_hook=self._unpack, use_list=False) for value in values]

        global_thread_local.object_refs = []
        result = []
        with self._lock:
            self._handle_deserialize_value(values, result)

        refs = [ref.id for ref in global_thread_local.object_refs]
        global_thread_local.object_refs = []
        reference_count.increase_reference_count(refs)
        return result

    def register_pack_hook(self, hook):
        """register pack hook to Serialization"""
        self._msgpack_serialize_hooks.append(hook)

    def register_unpack_hook(self, hook):
        """register unpack hook to Serialization"""
        self._msgpack_deserialize_hook.append(hook)

    def _handle_deserialize_value(self, values, result):
        for value in values:
            v = value
            if v is None:
                result.append(v)
                continue
            try:
                if isinstance(value, object_cache.Buffer):
                    v = value.immutable_data()
                    result.append(self._deserialize_pickle5_data(v))
                elif isinstance(value, bytes) and value[:len(YR_ZEROCOPY_PREFIX)] == YR_ZEROCOPY_PREFIX:
                    result.append(self._deserialize_pickle5_data(value))
                else:
                    result.append(cloudpickle.loads(v))
            except ValueError:
                result.append(cloudpickle.loads(v))
            except pickle.UnpicklingError:
                result.append(msgpack.unpackb(v, object_hook=self._unpack, use_list=False))

    def _pack(self, obj):
        for hook in self._msgpack_serialize_hooks:
            if callable(hook):
                obj = hook(obj)
        return obj

    def _unpack(self, obj):
        for hook in self._msgpack_deserialize_hook:
            if callable(hook):
                obj = hook(obj)
        return obj


def register_pack_hook(hook):
    """register pack hook helper"""
    Serialization().register_pack_hook(hook)


def register_unpack_hook(hook):
    """register unpack hook helper"""
    Serialization().register_unpack_hook(hook)
