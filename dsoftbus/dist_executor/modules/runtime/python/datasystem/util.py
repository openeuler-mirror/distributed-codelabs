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
The status of the return result.
"""
from datasystem import libds_common_py


class Validator:
    """
    Features: Parse arguments
    """

    @staticmethod
    def check_args_types(args):
        """ Check the types of the input arguments

        Args:
            args(list): The input arguments, which is a list of lists. Each list inside contains an argument name,
            the argument value and its expected valid types.
            Example: args = [["value", value, bytes, memoryview], ["timeout", timeout, int]]. Which means the argument
            value should have the type of bytes or memoryview and the timeout argument should be an integer.

        Raises:
            TypeError: Raise a type error if the input parameter is invalid.
        """
        if not isinstance(args, list):
            raise TypeError(
                r"The input of args should be a list, error type: {err}".format(err=type(args)))
        for arguments in args:
            if not isinstance(arguments, list):
                raise TypeError(
                    r"Each element of the input of args should be a list, error type: {err}".format(
                        err=type(arguments)))
            if len(arguments) < 3:
                raise TypeError(
                    r"Each element of the input of args should have the length at least 3, which "
                    r"contains an argument name, the argument value and its expected valid types.")
            arg_name = arguments[0]
            arg_value = arguments[1]
            arg_types = arguments[2:]
            valid = False
            for t in arg_types:
                if isinstance(arg_value, t):
                    valid = True
                    break
            if valid is False:
                raise TypeError(
                    r"The input of {name} has invalid type, valid type: {type}".format(name=arg_name, type=arg_types))

    @staticmethod
    def check_key_exists(args, keys):
        """ Check the types of the input arguments

        Args:
            args(dict): The input arguments.
            keys(list): a list of strings

        Raises:
            TypeError: Raise a type error if the input parameter is invalid.

        Returns:
            res: A list of the values of the given keys
        """
        if not isinstance(args, dict):
            raise TypeError(r"The input of args should be dict, error type: {err}".format(err=type(args)))
        if not isinstance(keys, list):
            raise TypeError(r"The input of keys should be list, error type: {err}".format(err=type(keys)))
        res = []
        for key in keys:
            k = args.get(key)
            if k is None:
                raise TypeError(
                    r"The key '{k_val}' of the input param does not exist".format(k_val=key))
            res.append(k)
        return res


class Status:
    """
    Features: The status of the return result
    """

    def __init__(self, status):
        """ Wrap the status of result.

        Args:
            status: libds_client_py.Status
        """
        self._status = status

    def is_ok(self):
        """ Whether the result is ok.

        Returns:
            Return True if is ok.
        """
        return self._status.is_ok()

    def is_error(self):
        """ Whether the result is error.

        Returns:
            Return True if is error.
        """
        return self._status.is_error()

    def to_string(self):
        """ Get the message of status.

        Returns:
            Return the message of status.
        """
        return self._status.to_string()
