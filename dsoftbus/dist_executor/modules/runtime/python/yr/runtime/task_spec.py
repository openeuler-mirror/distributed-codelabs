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
"""TaskSpec"""

import json
import logging
import time
import re
from dataclasses import dataclass, field
from enum import Enum
from typing import Callable, Optional, List, Union, Dict

from google.protobuf.message import Message

from yr import utils
from yr.config import DEFAULT_RECYCLE_TIME, DEFAULT_CONCURRENCY, \
    ConfigManager, FunctionInfo
from yr.instance_ref import InstanceRef
from yr.rpc import core_service_pb2
from yr.rpc.common_pb2 import Arg
from yr.serialization import SerializeObject
from yr.utils import ObjectDescriptor, CrossLanguageInfo

_logger = logging.getLogger(__name__)


class Signal(Enum):
    """signal for instance"""
    KILL_INSTANCE = 1
    KILL_ALL_INSTANCES = 2
    EXIT = 64
    CANCEL = 65


class AffinityType(Enum):
    """AffinityType for task"""
    PreferredAffinity = 0
    PreferredAntiAffinity = 1
    RequiredAffinity = 2
    RequiredAntiAffinity = 3


class Strdesc:
    """
    Descriptor for string fields in InvokeOptions
    """

    def __init__(self, default):
        self.__default = default
        self.__name = ""
        self.name = ""

    def __set_name__(self, _, name):
        self.__name = "__" + name
        self.name = name

    def __get__(self, obj, _):
        if obj is None:
            return self.__default
        return getattr(obj, self.__name, self.__default)

    def __set__(self, obj, value):
        if not isinstance(value, str):
            raise TypeError(f"{self.name} has wrong type {type(value)}, "
                            "'str' is expected.")
        if re.search("[/-]", value):
            raise ValueError(f"{self.name} contains illegal characters \"/\" or \"-\"")
        setattr(obj, self.__name, value)


class InvokeType(Enum):
    """InvokeType for task"""
    INVALID = -1
    CREATE_INSTANCE = 0
    INVOKE_MEMBER_FUNCTION = 1
    CREATE_NORMAL_FUNCTION_INSTANCE = 2
    INVOKE_NORMAL_FUNCTION = 3
    KILL_INSTANCE = 4
    GET_NAMED_INSTANCE_METADATA = 5


@dataclass
class InvokeOptions:
    """
    Invoke options for users to set resources

    Attributes:
        cpu: in 1/1000 cpu core, 300 to 16000 supported
        memory: in 1MB, 128 to 65536 supported

        custom_resources: GPU and NPU resources, legal key including
        "nvidia.com/gpu", "amd.com/gpu" and "huawei.com/Ascend910"

        concurrency: a int in the range of [1, 100]

        labels: a list contains affinity labels. The label as item of
        this list should be a string of less than 16 characters without
        special characters.

        affinity: a dict whose key-value mappings specify affinity labels
        and affinity types. The legal key (affinity label) should be
        a string of less than 16 characters without special characters.
        The legal value (affinity type) should be one of the valid values
        of yr.AffinityType, including "PreferredAffinity",
        "PreferredAntiAffinity", "RequiredAffinity" and "RequiredAntiAffinity".
    """
    cpu: int = 0
    memory: int = 0
    name: Strdesc = Strdesc("")
    namespace: Strdesc = Strdesc("")
    __concurrency: int = 1
    custom_resources: Dict = field(default_factory=dict)
    custom_extensions: Dict = field(default_factory=dict)
    __labels = []
    __affinity = {}

    @property
    def concurrency(self):
        """
        Get concurrency
        """
        return self.__concurrency

    @concurrency.setter
    def concurrency(self, value: int):
        """
        Set concurrency

        Args:
            value (int): concurrency
        """

        if isinstance(value, int) is False:
            raise TypeError(f"recycle_time {type(value)} type error, 'int' is expected.")
        if (1 <= value <= 1000) is False:
            raise ValueError(f"invalid concurrency value, expect 1 <= concurrency <= 1000, actual {value}")
        self.__concurrency = value

    @property
    def labels(self):
        """
        Get labels
        """
        return self.__labels

    @labels.setter
    def labels(self, labels: List):
        """
        Set labels

        Args:
            labels (List): labels
        """
        if not isinstance(labels, List):
            raise TypeError(f"labels has wrong type {type(labels)}, "
                            f"'list' is expected.")
        for label in labels:
            if not self._check_label_valid(label):
                raise ValueError("label should be a string of "
                                 "less than 16 characters without "
                                 "special characters.")
        self.__labels = labels

    @property
    def affinity(self):
        """
        Get affinity
        """
        return self.__affinity

    @affinity.setter
    def affinity(self, affinity: Dict):
        """
        Set affinity

        Args:
            affinity (Dict): affinity
        """
        if not isinstance(affinity, Dict):
            raise TypeError(f"affinity has wrong type {type(affinity)}, "
                            f"'dict' is expected.")
        for label, affinity_type in affinity.items():
            if not self._check_label_valid(label):
                raise ValueError("label should be a string of "
                                 "less than 16 characters without "
                                 "special characters.")
            if not isinstance(affinity_type, AffinityType):
                raise TypeError(f"affinity_type has wrong type "
                                f"{type(affinity_type)}, "
                                f"'AffinityType' is expected.")
        self.__affinity = affinity

    @staticmethod
    def _check_label_valid(label):
        """
        check if label is valid
        """
        if isinstance(label, str) is False:
            return False
        if len(label) > 16:
            return False
        if re.search(r"\W", label) is not None:
            return False
        return True

    def to_pb(self):
        """
        Transfer invoke options to protobuf
        """
        scheduling_ops = core_service_pb2.SchedulingOptions()
        if self.cpu > 0:
            scheduling_ops.resources["CPU"] = self.cpu
        if self.memory > 0:
            scheduling_ops.resources["Memory"] = self.memory
        for resource_key, resource_value in self.custom_resources.items():
            scheduling_ops.resources[resource_key] = resource_value
        for extension_key, extension_value in self.custom_extensions.items():
            scheduling_ops.extension[extension_key] = f"{extension_value}"
        for affinity_key, affinity_value in self.__affinity.items():
            scheduling_ops.affinity[affinity_key] = affinity_value.value
        return scheduling_ops

    def need_order(self):
        """
        Judge if need send request in order
        """
        return self.__concurrency == 1

    def build_designated_instance_id(self) -> str:
        """Builds an instanceID for the creation of an instance, depending on
        the value of name and namespace.

        Raises:
            ValueError: An error occurred when name is an empty string but namespace is not.

        Returns:
            str: Returns a string builded from name and namespace.
        """
        if not self.namespace:
            return self.name

        if not self.name:
            raise ValueError(
                f"name is not defined in InvokeOptions while namespace=\"{self.namespace}\"")
        return self.namespace + "-" + self.name


@dataclass
class Task:
    """invoke task"""
    invoke_type: InvokeType
    task_id: str
    object_descriptor: ObjectDescriptor
    args_list: list
    code_id: str
    trace_id: str = ""
    instance_id: InstanceRef = None
    invoke_options: InvokeOptions = None
    target_function_key: str = None
    return_obj_list: list = field(default_factory=list)


@dataclass
class TaskMetadata:
    """
    TaskMetadata is used to convey control information
    {
        "codeID": "yr-obj-xxx",
        "invokeType": 0,
        "concurrency": 1,
        "objectDescriptor": {
            "moduleName": "xxx",
            "className": "xxx",
            "functionName": "xxx",
            "srcLanguage": "python",
            "targetLanguage": "cpp"
        },
        "config": {
            "recycleTime": 10,
            "functionID": {
                "cpp": "xxx/xxx/$latest",
                "python": "xxx/xxx/$latest"
            },
            "jobID": "job-xxx",
            "logLevel": "WARNING"
        }
    }
    """
    function_id_python: str
    job_id: str
    object_descriptor: ObjectDescriptor
    invoke_type: InvokeType
    concurrency: int = DEFAULT_CONCURRENCY
    recycle_time: int = DEFAULT_RECYCLE_TIME
    code_id: str = ""
    function_id_cpp: str = ""
    function_id_java: str = ""
    log_level: str = "WARNING"
    __key_invoke_type = "invokeType"
    __key_code_id = "codeID"
    __key_object_descriptor = "objectDescriptor"
    __key_config = "config"
    __key_recycle_time = "recycleTime"
    __key_function_id = "functionID"
    __key_function_id_cpp = "cpp"
    __key_function_id_python = "python"
    __key_function_id_java = "java"
    __key_job_id = "jobID"
    __key_log_level = "logLevel"
    __key_concurrency = "concurrency"

    @classmethod
    def create(cls, object_descriptor, code_id, invoke_type,
               concurrency=DEFAULT_CONCURRENCY):
        """create metadata"""
        level = ConfigManager().log_level
        level = level if isinstance(level, str) else logging.getLevelName(level)
        return cls(function_id_python=ConfigManager().get_function_id_by_language(utils.LANGUAGE_PYTHON),
                   function_id_cpp=ConfigManager().get_function_id_by_language(utils.LANGUAGE_CPP),
                   job_id=ConfigManager().job_id,
                   object_descriptor=object_descriptor,
                   recycle_time=ConfigManager().recycle_time,
                   code_id=code_id,
                   invoke_type=invoke_type,
                   log_level=level,
                   concurrency=concurrency)

    @classmethod
    def parse(cls, data):
        """
        parse TaskMetadata from json or dict

        Returns: TaskMetadata object
        """
        tmp = data
        if isinstance(tmp, bytes):
            tmp = tmp.decode()
        if isinstance(tmp, str):
            tmp = json.loads(tmp)
        if isinstance(tmp, dict):
            if cls.__key_object_descriptor not in tmp or cls.__key_config not in tmp:
                raise TypeError(f"can not parse object: {tmp}")
            object_descriptor = ObjectDescriptor.parse(tmp.get(cls.__key_object_descriptor))
            config = tmp.get(cls.__key_config)
            function_id = config.get(cls.__key_function_id, {})
            log_level = config.get(cls.__key_log_level, "WARNING")
            concurrency = tmp.get(cls.__key_concurrency, DEFAULT_CONCURRENCY)
            self = cls(function_id_python=function_id.get(cls.__key_function_id_python, ""),
                       job_id=config.get(cls.__key_job_id, ""),
                       object_descriptor=object_descriptor,
                       recycle_time=config.get(cls.__key_recycle_time, cls.recycle_time),
                       code_id=tmp.get(cls.__key_code_id, ""),
                       function_id_cpp=function_id.get(cls.__key_function_id_cpp, ""),
                       function_id_java=function_id.get(cls.__key_function_id_java, ""),
                       invoke_type=InvokeType(tmp.get(cls.__key_invoke_type, InvokeType.INVALID.value)),
                       log_level=log_level,
                       concurrency=concurrency)
        else:
            raise TypeError(f"can not parse from type {type(data)}")
        return self

    def to_json(self):
        """
        convert to json
        Returns: json
        """
        tmp = {
            self.__key_code_id: self.code_id,
            self.__key_invoke_type: self.invoke_type.value,
            self.__key_concurrency: self.concurrency,
            self.__key_object_descriptor: self.object_descriptor.to_dict(),
            self.__key_config: {
                self.__key_recycle_time: self.recycle_time,
                self.__key_function_id: {
                    self.__key_function_id_cpp: self.function_id_cpp,
                    self.__key_function_id_python: self.function_id_python,
                    self.__key_function_id_java: self.function_id_java
                },
                self.__key_job_id: self.job_id,
                self.__key_log_level: self.log_level
            }
        }
        return json.dumps(tmp)


@dataclass
class TaskSpec:
    """TaskSpec"""
    task_id: str
    job_id: str
    invoke_type: InvokeType
    invoke_timeout: int
    callback: Callable[[Optional[Message], Optional[Exception]], None]
    object_ids: list = field(default_factory=list)
    trace_id: str = ""
    code_id: str = ""
    request_id: str = ""
    function_info: FunctionInfo = None
    object_descriptor: ObjectDescriptor = None
    instance_id: InstanceRef = None
    invoke_options: InvokeOptions = None
    cross_language_info: CrossLanguageInfo = None
    args: List[Union[SerializeObject, Arg]] = None
    signal: int = 0


def callback_hook(callback: Callable, task_id: str = '', invoke_type: InvokeType = InvokeType.INVALID,
                  object_counting_mgr=None):
    """process task exception"""
    start = time.time()
    _logger.info(f"task submitted: {task_id} type: {invoke_type}")

    def hook(data, err=None):
        _logger.info(f"task finished: {task_id} cost: {time.time() - start:.3f}")
        if object_counting_mgr:
            object_counting_mgr.del_record(task_id)
        callback(data, err)

    return hook
