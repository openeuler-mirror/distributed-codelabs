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
Object cache client python interface.
"""
from __future__ import absolute_import

from enum import Enum
from datasystem import libds_client_py as ds
from datasystem.util import Validator as validator


class WriteMode(Enum):
    """
    Features: Wrapping the write modes
    """
    NONE_L2_CACHE = ds.WriteMode.NONE_L2_CACHE
    WRITE_THROUGH_L2_CACHE = ds.WriteMode.WRITE_THROUGH_L2_CACHE


class ConsistencyType(Enum):
    """
    Features: Wrapping the consistency types
    """
    PRAM = ds.ConsistencyType.PRAM
    CAUSAL = ds.ConsistencyType.CAUSAL


class Buffer:
    """
    Features: Wrapping the Buffer objects
    """

    def __init__(self):
        self._buffer = None

    def wlatch(self, timeout=60):
        """ Acquire the write-lock to protect the buffer from concurrent reads and writes.

        Args:
            timeout(int): The try-lock timeout, the default value is 60 seconds.

        Raises:
            TypeError: Raise a type error if the input parameter is invalid.
            RuntimeError: Raise a runtime error if acquire write latch fails.
        """
        args = [["timeout", timeout, int]]
        validator.check_args_types(args)
        self._check_buffer()
        latch_status = self._buffer.wlatch(timeout)
        if latch_status.is_error():
            raise RuntimeError(latch_status.to_string())

    def unwlatch(self):
        """ Release the write-lock.

        Raises:
            RuntimeError: Raise a runtime error if release write latch fails.
        """
        self._check_buffer()
        unlatch_status = self._buffer.unwlatch()
        if unlatch_status.is_error():
            raise RuntimeError(unlatch_status.to_string())

    def rlatch(self, timeout=60):
        """ Acquire the read-lock to protect the buffer from concurrent writes.

        Args:
            timeout(int): The try-lock timeout, the default value is 60 seconds.

        Raises:
            TypeError: Raise a type error if the input parameter is invalid.
            RuntimeError: Raise a runtime error if acquire read latch fails.
        """
        args = [["timeout", timeout, int]]
        validator.check_args_types(args)
        self._check_buffer()
        latch_status = self._buffer.rlatch(timeout)
        if latch_status.is_error():
            raise RuntimeError(latch_status.to_string())

    def unrlatch(self):
        """ Release the read-lock.

        Raises:
            RuntimeError: Raise a runtime error if release read latch fails.
        """
        self._check_buffer()
        unlatch_status = self._buffer.unrlatch()
        if unlatch_status.is_error():
            raise RuntimeError(unlatch_status.to_string())

    def mutable_data(self):
        """ Get a mutable data memory view.

        Returns:
            The mutable memory view of the buffer.
        """
        self._check_buffer()
        return self._buffer.mutable_data()

    def immutable_data(self):
        """ Get an immutable data memory view.

        Returns:
            The immutable memory view of the buffer.
        """
        self._check_buffer()
        return memoryview(self._buffer.immutable_data())

    def memory_copy(self, value):
        """ Write data to the buffer.

        Args:
            value(memoryview, bytes or bytearray): the data to be copied to the buffer

        Raises:
            TypeError: Raise a type error if the input parameter is invalid.
            RuntimeError: Raise a runtime error if the copy fails.
        """
        args = [["value", value, memoryview, bytes, bytearray]]
        validator.check_args_types(args)
        self._check_buffer()
        copy_status = self._buffer.memory_copy(value)
        if copy_status.is_error():
            raise RuntimeError(copy_status.to_string())

    def publish(self, nested_object_ids=None):
        """ Publish mutable data to the server.

        Args:
            nested_object_ids(list): Nested object id list that buffer object depending on.

        Raises:
            TypeError: Raise a type error if the input parameter is invalid.
            RuntimeError: Raise a runtime error if publish fails.
        """
        if nested_object_ids is None:
            nested_object_ids = []
        args = [["nested_object_ids", nested_object_ids, list]]
        validator.check_args_types(args)
        if not all(map(lambda id: isinstance(id, str), nested_object_ids)):
            raise TypeError(r"The input of nested_object_ids should be a list of strings. There exists error type.")
        self._check_buffer()
        pub_status = self._buffer.publish(nested_object_ids)
        if pub_status.is_error():
            raise RuntimeError(pub_status.to_string())

    def seal(self, nested_object_ids=None):
        """ Publish immutable data to the server.

        Args:
            nested_object_ids(list):  Objects that depend on objectId.

        Raises:
            TypeError: Raise a type error if the input parameter is invalid.
            RuntimeError: Raise a runtime error if the seal fails.
        """
        if nested_object_ids is None:
            nested_object_ids = []
        args = [["nested_object_ids", nested_object_ids, list]]
        validator.check_args_types(args)
        if not all(map(lambda id: isinstance(id, str), nested_object_ids)):
            raise TypeError(r"The input of nested_object_ids should be a list of strings. There exists error type.")
        self._check_buffer()
        seal_status = self._buffer.seal(nested_object_ids)
        if seal_status.is_error():
            raise RuntimeError(seal_status.to_string())

    def invalidate_buffer(self):
        """ Invalidate data on the current host.

        Raises:
            RuntimeError: Raise a runtime error if invalidate fails.
        """
        self._check_buffer()
        invalidate_status = self._buffer.invalidate_buffer()
        if invalidate_status.is_error():
            raise RuntimeError(invalidate_status.to_string())

    def get_size(self):
        """ Get the size of the buffer.

        Returns:
            size(int): data size of the buffer.
        """
        self._check_buffer()
        return self._buffer.get_size()

    def set_buffer(self, buffer):
        """ Set buffer.

        Args:
            buffer: The buffer created by the client
        """
        self._buffer = buffer

    def _check_buffer(self):
        """ Check to make sure that self._buffer is not None.

        Raises:
            RuntimeError: Raise a runtime error if self._buffer is None
        """
        if self._buffer is None:
            raise RuntimeError(r"The buffer is None, please create it first.")


class ObjectClient:
    """
    Features: Data system Object Client management for python.
    """

    def __init__(
            self,
            host,
            port,
            connect_timeout=60000,
            token="",
            client_public_key="",
            client_private_key="",
            server_public_key=""
    ):
        """ Constructor of the ObjectClient class

        Args:
            host(str): The host of the worker address.
            port(int): The port of the worker address.
            connect_timeout(int): The timeout interval for the connection between the client and worker.
            token(str): A string used for authentication.
            client_public_key(str): The client's public key, for curve authentication.
            client_private_key(str): The client's private key, for curve authentication.
            server_public_key(str): The worker server's public key, for curve authentication.

        Raises:
            TypeError: Raise a type error if the input parameter is invalid.
        """
        args = [["host", host, str],
                ["port", port, int],
                ["connect_timeout", connect_timeout, int],
                ["token", token, str],
                ["client_public_key", client_public_key, str],
                ["client_private_key", client_private_key, str],
                ["server_public_key", server_public_key, str]]
        validator.check_args_types(args)
        self.client = ds.ObjectClient(host, port, connect_timeout, token,
                                      client_public_key, client_private_key, server_public_key)

    def init(self):
        """ Init a client to connect to a worker.

        Returns:
            status: the init status.

        Raises:
            RuntimeError: Raise a runtime error if the client fails to connect to the worker.
        """
        init_status = self.client.init()
        if init_status.is_error():
            raise RuntimeError(init_status.to_string())

    def create(self, object_id, size, param=None):
        """ Create an object buffer

        Args:
            object_id(str): The id of the object to be created.
            size(int): The size in bytes of object.
            param(dict): which contains the following three "key: value" pairs successively:
            (1) "write_mode", write_mode(Enum): Indicating whether the object will be written through L2 cache.
                              There are 2 options:
                              1) WriteMode.NONE_L2_CACHE;
                              2) WriteMode.WRITE_THROUGH_L2_CACHE;
            (2) "consistency_type": consistency_type(Enum): Indicating which consistency type will be used.
                              There are 2 options:
                              1) ConsistencyType.PRAM;
                              2) ConsistencyType.CAUSAL;

        Returns:
            buffer: The object buffer.

        Raises:
            TypeError: Raise a type error if the input parameter is invalid.
            RuntimeError: Raise a runtime error if the client fails to connect to the worker.
        """
        if param is None:
            param = {
                "write_mode": WriteMode.NONE_L2_CACHE,
                "consistency_type": ConsistencyType.PRAM
            }
        validator.check_args_types([["param", param, dict]])

        params = validator.check_key_exists(param, ["write_mode", "consistency_type"])
        write_mode, consistency_type = params[0], params[1]

        args = [
            ["object_id", object_id, str],
            ["size", size, int],
            ["write_mode", write_mode, type(WriteMode.NONE_L2_CACHE), type(WriteMode.WRITE_THROUGH_L2_CACHE)],
            ["consistency_type", consistency_type, type(ConsistencyType.PRAM), type(ConsistencyType.CAUSAL)]
        ]
        validator.check_args_types(args)

        create_status, buffer = self.client.create(object_id, size, write_mode.value, consistency_type.value)
        if create_status.is_error():
            raise RuntimeError(create_status.to_string())
        buf = Buffer()
        buf.set_buffer(buffer)
        return buf

    def put(self, object_id, value, param=None, nested_object_ids=None):
        """ Put the object data to the data system.

        Args:
            object_id(str): The id of the object to be created.
            value(memoryview, bytes or bytearray): the data to be put
            param(dict): which contains the following three "key: value" pairs successively:
            (1) "write_mode", write_mode(Enum): Indicating whether the object will be written through L2 cache.
                              There are 2 options:
                              1) WriteMode.NONE_L2_CACHE;
                              2) WriteMode.WRITE_THROUGH_L2_CACHE;
            (2) "consistency_type": consistency_type(Enum): Indicating which consistency type will be used.
                               There are 2 options:
                               1) ConsistencyType.PRAM;
                               2) ConsistencyType.CAUSAL;
            nested_object_ids(list):  Objects that depend on objectId.

        Raises:
            TypeError: Raise a type error if the input parameter is invalid.
            RuntimeError: Raise a runtime error if the put fails.
        """
        if param is None:
            param = {
                "write_mode": WriteMode.NONE_L2_CACHE,
                "consistency_type": ConsistencyType.PRAM
            }
        validator.check_args_types([["param", param, dict]])
        params = validator.check_key_exists(param, ["write_mode", "consistency_type"])
        write_mode, consistency_type = params[0], params[1]

        if nested_object_ids is None:
            nested_object_ids = []

        args = [["object_id", object_id, str],
                ["value", value, memoryview, bytes, bytearray],
                ["write_mode", write_mode, type(WriteMode.NONE_L2_CACHE), type(WriteMode.WRITE_THROUGH_L2_CACHE)],
                ["consistency_type", consistency_type, type(ConsistencyType.PRAM), type(ConsistencyType.CAUSAL)],
                ["nested_object_ids", nested_object_ids, list]]
        validator.check_args_types(args)
        put_status = self.client.put(object_id, value, write_mode.value, consistency_type.value,
                                     nested_object_ids)
        if put_status.is_error():
            raise RuntimeError(put_status.to_string())

    def get(self, object_ids, timeout):
        """ Get the buffers corresponding to the designated object ids

        Args:
            object_ids(list): The ids of the objects to get.
            timeout(int): The timeout of the get operation.

        Returns:
            buffers(list): list of buffers for the given object_ids.

        Raises:
            TypeError: Raise a type error if the input parameter is invalid.
            RuntimeError: Raise a runtime error if the RLatch fails.
        """
        buffer_list = []
        args = [["object_ids", object_ids, list], ["timeout", timeout, int]]
        validator.check_args_types(args)
        if not object_ids:
            raise RuntimeError(r"The input of object_ids list should not be empty")
        get_status, buffer_array = self.client.get(object_ids, timeout)
        if get_status.is_error():
            raise RuntimeError(get_status.to_string())
        for buffer in buffer_array:
            buf = Buffer()
            if not buffer.is_empty():
                buf.set_buffer(buffer)
            buffer_list.append(buf)
        return buffer_list

    def g_increase_ref(self, object_ids):
        """ Increase the global reference of the given objects.

        Args:
            object_ids(list): The ids of the objects to be increased. It cannot be empty.

        Raises:
            TypeError: Raise a type error if the input parameter is invalid.
            RuntimeError: Raise a runtime error if the increase fails.
        """
        args = [["object_ids", object_ids, list]]
        validator.check_args_types(args)
        if not object_ids:
            raise RuntimeError(r"The input of object_ids list should not be empty")
        g_inc_ref_status, failed_object_ids = self.client.g_increase_ref(object_ids)
        if g_inc_ref_status.is_error():
            raise RuntimeError(g_inc_ref_status.to_string() + r", failed objects: {obj_ids}".format(
                obj_ids=failed_object_ids))

    def g_decrease_ref(self, object_ids):
        """ Decrease the global reference of the given objects.

        Args:
            object_ids(list): The ids of the objects to be decreased. It cannot be empty.

        Raises:
            TypeError: Raise a type error if the input parameter is invalid.
            RuntimeError: Raise a runtime error if the decrease fails.
        """
        args = [["object_ids", object_ids, list]]
        validator.check_args_types(args)
        if not object_ids:
            raise RuntimeError(r"The input of object_ids list should not be empty")
        g_dec_ref_status, failed_object_ids = self.client.g_decrease_ref(object_ids)
        if g_dec_ref_status.is_error():
            raise RuntimeError(g_dec_ref_status.to_string() + r", failed objects: {obj_ids}".format(
                obj_ids=failed_object_ids))
