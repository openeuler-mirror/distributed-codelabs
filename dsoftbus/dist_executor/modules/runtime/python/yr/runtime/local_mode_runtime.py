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
"""local mode runtime"""
import logging
from abc import ABC
from typing import Union

from datasystem.stream_client import StreamClient

import yr
from yr import utils
from yr.config import ConfigManager
from yr.invoke_client.local_client import LocalClient
from yr.object_ref import ObjectRef
from yr.runtime.local_object_store import LocalObjectStore
from yr.runtime.runtime import BaseRuntime, package_python_args

_logger = logging.getLogger(__name__)


class LocalModeRuntime(BaseRuntime, ABC):
    """local mode runtime"""

    def __init__(self):
        self.invoke_client = LocalClient()
        super().__init__(self.invoke_client)
        self.local_store = LocalObjectStore()

    def put(self, ref: Union[str, ObjectRef], value, use_msgpack=False):
        """
        put object to local store
        """
        if isinstance(ref, str):
            ref = ObjectRef(ref)
        self.local_store.put(ref.id, value)
        return ref

    def get_trace_id(self) -> str:
        """get_trace_id"""
        return utils.generate_trace_id(ConfigManager().job_id)

    def clear(self):
        """clear local object and stop workers"""
        self.invoke_client.clear()
        LocalObjectStore().release_all()

    def get_stream_client(self) -> StreamClient:
        """
        get stream client for data system.
        """
        raise RuntimeError("local mode not support stream")

    def get_id(self, ref_id, timeout: int):
        pass

    def _package_python_args(self, args_list):
        """package python args"""
        return package_python_args(args_list)

    def _package_args_post(self, args_list):
        return args_list

    def _get_object(self, object_refs: list, timeout: int):
        wait_object_refs = list(set(object_refs))
        length = len(wait_object_refs)
        timeout = timeout if timeout != -1 else None
        _, unready = yr.wait(wait_object_refs, length, timeout)
        if len(unready) > 0:
            raise TimeoutError(f"get object timeout: {[ref.id for ref in unready]}")
        objs = self.local_store.get([ref.id for ref in object_refs])
        if not isinstance(objs, list):
            objs = [objs]
        return objs
