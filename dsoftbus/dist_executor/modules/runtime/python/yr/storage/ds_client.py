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
Datasystem client
"""
import logging
import threading
from typing import List, Union

from datasystem import object_cache
from datasystem.agent_client import AgentClient
from datasystem.stream_client import StreamClient

_logger = logging.getLogger(__name__)
FLAG_LENGTH = 12


class DSClient:
    """
    Datasystem client
    """
    __slots__ = ["__ds_client", "__ctx_ref_map", "__ctx_lock", "__stream_client"]

    def __init__(self):
        self.__ds_client = None
        self.__stream_client = None
        self.__ctx_ref_map = {}
        self.__ctx_lock = threading.Lock()

    def init(self, client: Union[AgentClient, object_cache.ObjectClient], stream_client: StreamClient):
        """
        Initialize datasystem client
        """
        if not isinstance(client, (AgentClient, object_cache.ObjectClient)):
            raise RuntimeError(f"ds client not support: {type(client)}, only AgentClient or ObjectClient")
        self.__ds_client = client
        self.__stream_client = stream_client

    def create(self, object_id: str, dependency_refs: List[str], writer, inband):
        """
        create buffer
        """
        total_size = writer.get_total_bytes(inband) + FLAG_LENGTH
        buf = self.__ds_client.create(object_id, total_size,
                                      param={"write_mode": object_cache.WriteMode.NONE_L2_CACHE,
                                             "consistency_type": object_cache.ConsistencyType.PRAM})

        buf.wlatch()
        try:
            writer.write_to(inband, buf.mutable_data(), 1)
        except ValueError as e:
            buf.unwlatch()
            raise RuntimeError(f"failed to copy object {object_id} in create") from e
        try:
            buf.seal(dependency_refs)
        except (ValueError, RuntimeError) as e:
            raise RuntimeError(f"failed to put object {object_id} in create") from e
        finally:
            buf.unwlatch()

    def put(self, object_id: str, value: bytes, dependency_refs: List[str]):
        """
        Put object to datasystem
        """
        if isinstance(self.__ds_client, object_cache.ObjectClient):
            buf = self.__ds_client.create(object_id, len(value),
                                          param={"write_mode": object_cache.WriteMode.NONE_L2_CACHE,
                                                 "consistency_type": object_cache.ConsistencyType.PRAM})
            buf.wlatch()
            buf.memory_copy(value)
            try:
                buf.seal(dependency_refs)
            except (ValueError, RuntimeError) as e:
                raise RuntimeError(f"failed to put object {object_id}") from e
            finally:
                buf.unwlatch()
        else:
            try:
                self.__ds_client.put(object_id, value, dependency_refs)
            except (ValueError, RuntimeError) as e:
                raise RuntimeError(f"failed to put object {object_id}") from e

    def get(self, object_id: Union[str, List], timeout: int) -> List[Union[bytes, object_cache.Buffer]]:
        """
        Get object from datasystem
        """
        object_ids = object_id
        if isinstance(object_id, str):
            object_ids = [object_id]
        try:
            return self.__ds_client.get(object_ids, int(timeout * 1000))
        except RuntimeError as e:
            raise RuntimeError(f"get {object_id} failed") from e

    def increase_global_reference(self, object_ids: List[str]):
        """
        Increase global reference
        """
        self.__ds_client.g_increase_ref(object_ids)
        self.increase_ctx_reference(object_ids)

    def increase_ctx_reference(self, object_ids: List[str]):
        """
        Increase context reference
        """
        with self.__ctx_lock:
            for obj_id in object_ids:
                self.__ctx_ref_map[obj_id] = self.__ctx_ref_map.get(obj_id, 0) + 1

    def decrease_global_reference(self, object_ids: List[str]):
        """
        Decrease global reference
        """
        need_decrease_objs = self.decrease_ctx_reference(object_ids)
        if len(need_decrease_objs) == 0:
            return
        self.__ds_client.g_decrease_ref(need_decrease_objs)

    def decrease_ctx_reference(self, object_ids: List[str]) -> List[str]:
        """
        Decrease context reference
        """
        with self.__ctx_lock:
            need_decrease_objs = []
            for obj_id in object_ids:
                cnt = self.__ctx_ref_map.get(obj_id, None)
                if cnt is None:
                    continue
                if cnt > 0:
                    cnt -= 1
                    self.__ctx_ref_map[obj_id] -= 1
                    need_decrease_objs.append(obj_id)
                if cnt <= 0:
                    self.__ctx_ref_map.pop(obj_id)
            return need_decrease_objs

    def clear_ctx_reference(self):
        """
        Clear context reference
        """
        need_decrease_objs = []
        for object_id, cnt in self.__ctx_ref_map.items():
            need_decrease_objs += [object_id] * cnt
        _logger.debug(f"clear context reference objs:{need_decrease_objs}")
        if len(need_decrease_objs) == 0:
            return
        self.__ctx_ref_map.clear()
        self.__ds_client.g_decrease_ref(need_decrease_objs)

    def is_obj_id_in_ctx(self, object_id: str) -> bool:
        """
        Is object_id in context
        """
        return object_id in self.__ctx_ref_map.keys()

    def get_stream_client(self) -> StreamClient:
        """
        get stream client for data system.
        """
        if not self.__stream_client:
            raise RuntimeError("not support stream out of cluster")
        return self.__stream_client
