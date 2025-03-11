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
common tools
"""
import enum
import json
import threading
import re
import logging
import uuid
import inspect
import binascii
import sys
from dataclasses import dataclass

import cloudpickle
from google.protobuf.json_format import MessageToDict

# http headers
HEADER_TRACE_ID = 'X-Trace-ID'
HEADER_EVENT_SOURCE = "X-Event-Source-Id"
HEADER_INVOKE_URN = "X-Tag-VersionUrn"
CONTENT_TYPE = "Content-Type"
CONTENT_TYPE_APPLICATION_JSON = "application/json"
AUTHORIZATION = "authorization"
HEADER_INVOKE_TYPE = "X-Invoke-Type"
HEADER_STATE_KEY = "X-State-Key"

NORMAL_FUNCTION = "normal function"
MEMBER_FUNCTION = "member function"
DEFAULT_TIMEOUT = 900
OBJECT_ID_PREFIX = "yr-api-obj-"
_URN_SEPARATOR = ":"
_TRACE_ID = "-trace-"
_NAME_SEPARATOR = "-"

LANGUAGE_PYTHON = "python"
LANGUAGE_CPP = "cpp"
FUNC_NAME = "functionName"
MODULE_NAME = "moduleName"
CLASS_NAME = "className"
TARGET_LANGUAGE = "targetLanguage"
SRC_LANGUAGE = "srcLanguage"
INSTANCE_ID = "instanceID"
FUNCTION_KEY = "functionKey"
CLASS_METHOD = "classMethod"
NEED_ORDER = "needOrder"
_logger = logging.getLogger(__name__)

_JOB_ID = ""
_RUNTIME_ID_INDEX = -2
_SERIAL_NUM_INDEX = -1


class Singleton:
    """
    Used as metaclass to implement Singleton.
    """

    def __init__(self, cls):
        self._cls = cls
        self._instance = {}
        self.lock = threading.Lock()

    def __call__(self, *args, **kw):
        if self._cls not in self._instance:
            with self.lock:
                if self._cls not in self._instance:
                    self._instance[self._cls] = self._cls(*args, **kw)
        return self._instance.get(self._cls)


def validate_ip(input_ip: str):
    """
    This is a checker for input ip string

    Checks validity of input ip string

    Returns:
        True, the input ip string is valid
        False, the input ip string is invalid
    """
    ip_regex = \
        r"^((25[0-5]|2[0-4]\d|[01]?\d\d?)\.){3}(25[0-5]|2[0-4]\d|[01]?\d\d?)$"
    compile_ip = re.compile(ip_regex)
    return compile_ip.match(input_ip)


def validate_address(address):
    """
    Validates address parameter
    Args:
        address: string address of the full <host:port> address

    Returns:
        ip: string of ip
        port: integer of port

    """
    address_parts = address.split(":")
    if len(address_parts) != 2:
        raise ValueError("address format is wrong, '<ip>:<port>' is expected.")
    ip = address_parts[0]
    try:
        port = int(address_parts[1])
    except ValueError as e:
        raise ValueError("port format is wrong, must be an integer.") from e
    else:
        if not 1 <= port <= 65535:
            raise ValueError(f"port value {port} is out of range.")
    if not validate_ip(ip):
        raise ValueError(f"invalid ip {ip}")
    return ip, port


def set_job_id(job_id):
    """set global job id"""
    global _JOB_ID
    _JOB_ID = job_id


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


def generate_task_id():
    """
    This is a wrapper generating random task id for user stateless invoke functions

    Gets a random id string

    Example: job-xxx-task-433ec3c1-ba11-5a16-ad97-ee4e68db67d5

    Returns:
        Unique uuid string with prefix for stateless invoke functions
    """
    uuid_str = str(uuid.uuid4())
    return _JOB_ID + "-task-" + uuid_str


def generate_runtime_id():
    """
    Generating random runtime id

    :return: Unique 8-bit uuid string
    """
    return str(uuid.uuid4().hex)[:8]


def generate_task_id_with_serial_num(runtime_id, serial_num):
    """
    Generating random task id with last 8 characters recording serial number
    for user class methods

    Example: job-xxx-task-433ec3c1-ba11-5a16-ad97-ee4e68db67d5-00000001

    :param runtime_id: runtime id
    :param serial_num: serial number of invoke task
    :return: Unique uuid string with prefix and serial number
             for user class methods
    """
    uuid_str = str(uuid.uuid4())
    return f"{_JOB_ID}-task-{uuid_str}-{runtime_id}-{str(serial_num)}"


def extract_serial_num(task_id):
    """
    Extract serial number from task id

    :param task_id: string of task id
    :return: serial number of the task (int)
    """
    return int(task_id.split("-")[_SERIAL_NUM_INDEX])


def extract_runtime_id(task_id):
    """
    Extract runtime id from task id

    :param task_id: string of task id
    :return: runtime id of the task (str)
    """
    return task_id.split("-")[_RUNTIME_ID_INDEX]


def generate_trace_id(job_id: str) -> str:
    """
    TraceID is used to analyze the call chain between functions

    Example: job-fa60ccbb-trace-adc3f0b94c89457e8fedce36c0d0dc20

    Returns:
        Unique uuid string with prefix for stateless invoke functions
    """
    trace_id = str(uuid.uuid4().hex)
    return job_id + _TRACE_ID + trace_id


def get_module_name(obj):
    """
    Get the module name from object. If the module is __main__,
    get the module name from file.

    Returns:
        Module name of object.
    """
    module_name = obj.__module__
    n = None
    if module_name == "__main__":
        try:
            file_path = inspect.getfile(obj)
            n = inspect.getmodulename(file_path)
        except TypeError:
            pass
    if n:
        module_name = n
    return module_name


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


def package_args(args, kwargs):
    """
    Package user invoke args.
    """
    send_param = {
        "args": binary_to_hex(cloudpickle.dumps(args)),
        "kwargs": binary_to_hex(cloudpickle.dumps(kwargs)),
    }

    param_str = json.dumps(send_param)

    return param_str


class ObjectDescriptor:
    """
    object descriptor
    """
    src_language = LANGUAGE_PYTHON
    target_language = LANGUAGE_PYTHON

    def __init__(self, module_name="",
                 class_name="",
                 function_name="",
                 target_language=LANGUAGE_PYTHON,
                 src_language=LANGUAGE_PYTHON):
        self.module_name = module_name
        self.class_name = class_name
        self.function_name = function_name
        self.target_language = target_language
        self.src_language = src_language

    @classmethod
    def get_from_function(cls, func):
        """
        get the function descriptor
        """
        self = ObjectDescriptor.__new__(cls)
        self.module_name = get_module_name(func)
        self.function_name = func.__qualname__
        self.class_name = ""
        return self

    @classmethod
    def get_from_class(cls, obj):
        """
        get the class descriptor
        """
        self = ObjectDescriptor.__new__(cls)
        self.module_name = get_module_name(obj)
        self.class_name = obj.__qualname__
        self.function_name = ""
        return self

    @classmethod
    def parse(cls, data):
        """parse from json or dict"""
        tmp = data
        if isinstance(tmp, bytes):
            tmp = tmp.decode()
        if isinstance(tmp, str):
            tmp = json.loads(tmp)
        if isinstance(tmp, dict):
            self = cls(module_name=tmp.get(MODULE_NAME, ""),
                       class_name=tmp.get(CLASS_NAME, ""),
                       function_name=tmp.get(FUNC_NAME, ""),
                       target_language=tmp.get(TARGET_LANGUAGE, ""),
                       src_language=tmp.get(SRC_LANGUAGE, ""))
        else:
            raise TypeError(f"can not parse from type {type(data)}")
        return self

    def to_dict(self):
        """
        export the object descriptor to dict
        """
        return {MODULE_NAME: self.module_name,
                CLASS_NAME: self.class_name,
                FUNC_NAME: self.function_name,
                TARGET_LANGUAGE: self.target_language,
                SRC_LANGUAGE: self.src_language}


def is_function_or_method(obj):
    """
    judge the obj type
    """
    return inspect.isfunction(obj) or inspect.ismethod(obj)


class _URNIndex(enum.IntEnum):
    prefix = 0
    zone = 1
    business_id = 2
    tenant_id = 3
    name = 5
    version = 6


def get_function_from_urn(urn: str):
    """
    get function name which used by posix
    example: 7e1ad6a6cc5c44fabd5425873f72a86a/0-test-helloclass/$latest
    """
    parts = urn.split(_URN_SEPARATOR)
    name = [parts[_URNIndex.tenant_id], parts[_URNIndex.name], parts[_URNIndex.version]]
    return "/".join(name)


def serialize_code(code):
    """
    Serialize the original user invoke function
    """
    res = cloudpickle.dumps(code)
    serial_func_bytes = binascii.hexlify(res)
    serial_func_str = str(serial_func_bytes, encoding="utf-8")
    return serial_func_str


def create_payload(args_list):
    """args list to payload"""
    args_list = [MessageToDict(i) for i in args_list]
    for i in args_list:
        if 'type' in i:
            i['type'] = 1
    return json.dumps(args_list)


@dataclass
class CrossLanguageInfo:
    """
    CrossLanguageFunctionInfo
    """
    function_name: str
    function_key: str
    target_language: str
