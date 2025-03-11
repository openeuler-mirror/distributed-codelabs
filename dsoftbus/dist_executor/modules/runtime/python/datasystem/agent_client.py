# Copyright (c) 2022 Huawei Technologies Co., Ltd.
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
Object cache agent client python interface.
"""
from __future__ import absolute_import

from datasystem import libds_client_py as ds
from datasystem.util import Validator as validator


class AgentClient:
    """
    Features: Data system object cache agent client management for python.
    """

    def __init__(self, host, port, timeout_ms=60000, client_public_key="", client_private_key="", server_public_key=""):
        """ Init a client to connect to a worker.

        Args:
            host(str): The host of the worker address.
            port(int): The port of the worker address.
            timeout_ms: The timeout of the init operation.
            client_public_key(str): The client's public key, for curve authentication.
            client_private_key(str): The client's private key, for curve authentication.
            server_public_key(str): The agent server's public keys, for curve authentication.

        Raises:
            TypeError: Raise a type error if the input parameter is invalid.
            RuntimeError: Raise a runtime error if the client fails to connect to the worker.
        """
        args = [
            ["host", host, str],
            ["port", port, int],
            ["timeout_ms", timeout_ms, int],
            ["client_public_key", client_public_key, str],
            ["client_private_key", client_private_key, str],
            ["server_public_key", server_public_key, str]
        ]
        validator.check_args_types(args)
        self._client = ds.AgentClient(host, port, timeout_ms, client_public_key, client_private_key, server_public_key)

    def init(self):
        """ Init an agent client.

        Returns:
            status: the init status.

        Raises:
            RuntimeError: Raise a runtime error if the client fails to connect to the server.
        """
        init_status = self._client.init()
        if init_status.is_error():
            raise RuntimeError(init_status.to_string())

    def put(self, object_id, value, nested_object_ids: list = None):
        """ Store an object into the storage.

        Args:
            object_id: The ID of the object to store.
            value: The value of object.
            nested_object_ids(list): An optional list that indicate specific nested_object_ids dependent on object_id.

        Raises:
            TypeError: Raise a type error if the input parameter is invalid.
            RuntimeError: Raise a runtime error if it fails to put an object in the storage.
        """
        if nested_object_ids is None:
            nested_object_ids = []
        args = [
            ["object_id", object_id, str],
            ["nested_object_ids", nested_object_ids, list],
            ["value", value, bytes]
        ]
        validator.check_args_types(args)
        if not all(map(lambda id: isinstance(id, str), nested_object_ids)):
            raise TypeError(r"The input of nested_object_ids should be list of strings. contains error type.")
        status = self._client.put(object_id, value, nested_object_ids)
        if status.is_error():
            raise RuntimeError(status.to_string())

    def get(self, object_ids, timeout_ms):
        """ Get objects from the storage.

        Args:
            object_ids: The IDs of objects to get.
            timeout_ms: The timeout of the get operation.

        Returns:
            values: list of values for the data associated with the object_ids.

        Raises:
            TypeError: Raise a type error if the input parameter is invalid.
            RuntimeError: Raise a runtime error if the client fails to get an object.
        """
        args = [["object_ids", object_ids, list], ["timeout_ms", timeout_ms, int]]
        validator.check_args_types(args)
        if not object_ids:
            raise RuntimeError(r"The input of object_ids list should not be empty")
        values = []
        status, values = self._client.get(object_ids, timeout_ms, values)
        if status.is_error():
            raise RuntimeError(status.to_string())
        return values

    def g_increase_ref(self, object_ids: list):
        """ Increase object(s)' global reference number by 1.

        Args:
            object_ids(list): The ids of objects to increase their global references.

        Raises:
            TypeError: Raise a type error if the input parameter is invalid.
            RuntimeError: Raise a runtime error if the client fails to increase the reference count.
        """
        args = [["object_ids", object_ids, list]]
        validator.check_args_types(args)
        if not object_ids:
            raise RuntimeError(r"The input of object_ids list should not be empty")
        status, failed_object_ids = self._client.g_increase_ref(object_ids)
        if status.is_error():
            raise RuntimeError(status.to_string() + r", failed objects: {obj_ids}".format(
                obj_ids=failed_object_ids))

    def g_decrease_ref(self, object_ids: list):
        """ Decrease object(s)' global reference number by 1.

        Args:
            object_ids(list): The ids of objects to decrease their global references.

        Raises:
            TypeError: Raise a type error if the input parameter is invalid.
            RuntimeError: Raise a runtime error if the client fails to decrease the reference count.
        """
        args = [["object_ids", object_ids, list]]
        validator.check_args_types(args)
        if not object_ids:
            raise RuntimeError(r"The input of object_ids list should not be empty")
        status, failed_object_ids = self._client.g_decrease_ref(object_ids)
        if status.is_error():
            raise RuntimeError(status.to_string() + r", failed objects: {obj_ids}".format(
                obj_ids=failed_object_ids))
