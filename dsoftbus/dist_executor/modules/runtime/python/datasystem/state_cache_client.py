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
State cache client python interface.
"""
from datasystem import libds_client_py as ds
from datasystem.object_cache import WriteMode
from datasystem.util import Validator as validator


class StateCacheClient:
    """
    Features: Data system State Cache Client management for python.
    """

    def __init__(self):
        self._client = None

    def init(self, host: str, port: int, timeout_ms=60000, token: str = '', client_public_key: str = '',
             client_private_key: str = '', server_public_key: str = ""):
        """ Init a client to connect to a worker.

        Args:
            host(str): The host of the worker.
            port(int): The port of the worker.
            timeout_ms(int): The timeout interval for the connection between the client and worker.
            token(str): A string used for authentication.
            client_public_key(str): The client's public key, for curve authentication.
            client_private_key(str): The client's private key, for curve authentication.
            server_public_key(str): The worker server's public key, for curve authentication.
        Raises:
            RuntimeError: Raise a runtime error if the client fails to connect to the worker.
            TypeError: Raise a type error if the input parameter is invalid.
        """
        args = [["host", host, str], ["timeout_ms", timeout_ms, int], ["client_public_key", client_public_key, str],
                ["client_private_key", client_private_key, str], ["server_public_key", server_public_key, str]]
        validator.check_args_types(args)
        self._client = ds.StateCacheClient(host, port, timeout_ms, token,
                                           client_public_key, client_private_key, server_public_key)
        status = self._client.Init()
        if status.is_error():
            raise RuntimeError(status.to_string())

    def set(self, key, val, write_mode=WriteMode.NONE_L2_CACHE, ttl_second=0):
        """ Invoke worker client to set the value of a key.

        Args:
            key(str): The key of string data.
            val(memoryview, bytes, bytearray, str): The data to be set.
            write_mode(WriteMode): controls whether data is written to the L2 cache to enhance data reliability.
                The options are as follows:
                WriteMode.NONE_L2_CACHE: indicates that data reliability is not required,
                WriteMode.WRITE_THROUGH_L2_CACHE: indicates that data is synchronously written to the L2 cache
                to improve data reliability.
            ttl_second(uint32): controls the expire time of the data:
                If the value is greater than 0, the data will be deleted automatically after expired.
                If set to 0, the data need to be manually deleted.
        Raises:
            RuntimeError: Raise a runtime error if fails to set the value of the key.
            TypeError: Raise a type error if the input parameter is invalid.
        """
        args = [["key", key, str],
                ["val", val, memoryview, bytes, bytearray, str],
                ["write_mode", write_mode, type(WriteMode.NONE_L2_CACHE), type(WriteMode.WRITE_THROUGH_L2_CACHE)],
                ["ttl_second", ttl_second, int]]
        validator.check_args_types(args)

        if isinstance(val, str):
            val = str.encode(val)
        status = self._client.Set(key, val, write_mode.value, ttl_second)
        if status.is_error():
            raise RuntimeError(status.to_string())

    def get(self, keys: list = None, convert_to_str: bool = False):
        """ Get the values of all given keys.

        Args:
            keys(list): The key list of string type.
            convert_to_str(bool): True: convert the return value to string, False: return bytes directly.

        Returns:
        	values(list): The value list of keys. If the key is not found, it will raise RuntimeError.

        Raises:
            RuntimeError: Raise a runtime error if fails to get the value of the key.
            TypeError: Raise a type error if the input parameter is invalid.
        """
        args = [["keys", keys, list]]
        validator.check_args_types(args)
        if keys is None:
            raise RuntimeError(r"The input of keys list should not be empty")
        status, values = self._client.Get(keys)         # returned value is bytes type
        if status.is_error():
            raise RuntimeError(status.to_string())
        if convert_to_str is False:
            return values
        ret_vals = []
        for val in values:
            ret_vals.append(val.decode())
        return ret_vals

    def delete(self, keys: list = None):
        """ Delete the values of all given keys.

        Args:
            keys(list): The data list of string type.

        Returns:
        	success_count(int): Num of deleted string data. If the key is not found, it will return 0.

        Raises:
            RuntimeError: Raise a runtime error if fails to delete the value of the key.
            TypeError: Raise a type error if the input parameter is invalid.
        """
        args = [["keys", keys, list]]
        validator.check_args_types(args)
        if keys is None:
            raise RuntimeError(r"The input of keys list should not be empty")
        status, success_count = self._client.Del(keys)
        if status.is_error():
            raise RuntimeError(status.to_string())
        return success_count

    def lpush(self, key: str, values: list):
        """ Prepend one or multiple elements to a list.

        Args:
            key(str): The key of the list object.
            values(list): The values of the list to lpush.

        Raises:
            RuntimeError: Raise a runtime error if fails to lpush.
            TypeError: Raise a type error if the input parameter is invalid.
        """
        args = [["key", key, str], ["values", values, list]]
        validator.check_args_types(args)
        status = self._client.Lpush(key, values)
        if status.is_error():
            raise RuntimeError(status.to_string())

    def lpop(self, key: str):
        """ Remove and get the first element in a list.

        Args:
            key(str): The key of the list object.

        Returns:
            value(string): value.

        Raises:
            RuntimeError: Raise a runtime error if fails to put object in the storage.
            TypeError: Raise a type error if the input parameter is invalid.
        """
        args = [["key", key, str]]
        validator.check_args_types(args)
        status, value = self._client.Lpop(key)
        if status.is_error():
            raise RuntimeError(status.to_string())
        return value

    def lindex(self, key: str, index: int):
        """ Get an element from a list by its index.

        Args:
            key(str): The key of the list object.
            index(int): The index to get value of the list.

        Returns:
            value(string): value.

        Raises:
            RuntimeError: Raise a runtime error if fails to put object in the storage.
            TypeError: Raise a type error if the input parameter is invalid.
        """
        args = [["key", key, str], ["index", index, int]]
        validator.check_args_types(args)
        status, value = self._client.Lindex(key, index)
        if status.is_error():
            raise RuntimeError(status.to_string())
        return value

    def llen(self, key: str):
        """ Get the length of thelist.

        Args:
            key(str): The key of the list object.

        Returns:
            value(long long): value.

        Raises:
            RuntimeError: Raise a runtime error if fails to put object in the storage.
            TypeError: Raise a type error if the input parameter is invalid.
        """
        args = [["key", key, str]]
        validator.check_args_types(args)
        status, value = self._client.Llen(key)
        if status.is_error():
            raise RuntimeError(status.to_string())
        return value

    def hget(self, key: str, field: str):
        """ Get the value of a hash field.

        Args:
            key(str): The key of the hash object.
            field(str): The field to get value of the hash object.

        Returns:
            value(string): The value of the hash field.

        Raises:
            RuntimeError: Raise a runtime error if fails to hget.
            TypeError: Raise a type error if the input parameter is invalid.

        Examples:
            >>> from datasystem.state_cache_client import StateCacheClient
            >>> client = StateCacheClient()
            >>> client.init('127.0.0.1:18482')
            >>> client.hget('key','a')
            '1'
        """
        args = [["key", key, str], ["field", field, str]]
        validator.check_args_types(args)
        status, value = self._client.Hget(key, field)
        if status.is_error():
            raise RuntimeError(status.to_string())
        return value

    def hset(self, key: str, field: str, value: str):
        """ Set the string value of a hash field.

        Args:
            key(str): The key of the hash object.
            field(str): The field to set value of the hash object.
            value(str): The value of the field.

        Raises:
            RuntimeError: Raise a runtime error if fails to hset.
            TypeError: Raise a type error if the input parameter is invalid.

        Examples:
            >>> from datasystem.state_cache_client import StateCacheClient
            >>> client = StateCacheClient()
            >>> client.init('127.0.0.1:18482')
            >>> client.hset('key','a', '1')
            >>> client.hset('key','b', '2')
        """
        args = [["key", key, str], ["field", field, str], ["value", value, str]]
        validator.check_args_types(args)
        status = self._client.Hset(key, field, value)
        if status.is_error():
            raise RuntimeError(status.to_string())

    def hdel(self, key: str, field: str):
        """ Delete one hash field.

        Args:
            key(str): The key of the hash object.
            field(str): The field to delete of the hash object.

        Raises:
            RuntimeError: Raise a runtime error if fails to hdel.
            TypeError: Raise a type error if the input parameter is invalid.

        Examples:
            >>> from datasystem.state_cache_client import StateCacheClient
            >>> client = StateCacheClient()
            >>> client.init('127.0.0.1:18482')
            >>> client.hdel('key','c')
        """
        args = [["key", key, str], ["field", field, str]]
        validator.check_args_types(args)
        status = self._client.Hdel(key, field)
        if status.is_error():
            raise RuntimeError(status.to_string())

    def hgetall(self, key: str):
        """ Get all the fields and values in a hash.

        Args:
            key(str): The key of the hash object.

        Returns:
            dict: The value of the hashmap.

        Raises:
            RuntimeError: Raise a runtime error if fails to hset.
            TypeError: Raise a type error if the input parameter is invalid.

        Examples:
            >>> from datasystem.state_cache_client import StateCacheClient
            >>> client = StateCacheClient()
            >>> client.init('127.0.0.1:18482')
            >>> client.hgetall('key')
            {'a': '1', 'b': '2'}
        """
        args = [["key", key, str]]
        validator.check_args_types(args)
        status, values = self._client.Hgetall(key)
        if status.is_error():
            raise RuntimeError(status.to_string())
        return values
