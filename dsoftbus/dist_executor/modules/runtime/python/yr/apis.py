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
function which need export
"""
import atexit
import logging
import os
import json
from typing import Union, List

from datasystem.agent_client import AgentClient
from datasystem.object_cache import ObjectClient
from datasystem.stream_client import StreamClient

from yr import Config
from yr import utils
from yr.common.timer import Timer
from yr.config import ClientInfo
from yr.config import ConfigManager
from yr.config import check_init
from yr.decorator import function_proxy
from yr.decorator import instance_proxy
from yr.object_ref import ObjectRef
from yr.objref_counting_manager import ObjRefCountingManager
from yr.runtime import cluster_mode_runtime
from yr.runtime.local_mode_runtime import LocalModeRuntime
from yr.runtime.runtime import Runtime
from yr.storage.reference_count import ReferenceCount
from yr.utils import get_function_from_urn, generate_runtime_id
from yr.custom import mpctl

_DATASYSTEM_DEFAULT_TIMEOUT = 5 * 60
_BASE_LOG_NAME = "yr"
_MAX_INT = 0x7FFFFFFF
_logger = logging.getLogger(__name__)
_HTTP_PROXY = "http_proxy"
_HTTPS_PROXY = "https_proxy"
_DATASYSTEM_CLIENT_LOG_DIR = "DATASYSTEM_CLIENT_LOG_DIR"


def _unset_http_proxy():
    if _HTTP_PROXY in os.environ:
        os.environ[_HTTP_PROXY] = ""

    if _HTTPS_PROXY in os.environ:
        os.environ[_HTTPS_PROXY] = ""


def _set_datasystem_client_log_dir():
    if _DATASYSTEM_CLIENT_LOG_DIR in os.environ:
        return

    config_path = os.path.realpath(os.path.join(
        os.path.dirname(os.path.abspath(__file__)), "../config/python-runtime-log.json")
    )
    if not os.path.isfile(config_path):
        return

    with open(config_path, "r") as fr:
        try:
            data = json.load(fr)
        except json.decoder.JSONDecodeError:
            return

    log_path = data.get("handlers", {}).get("file", {}).get("filename", "")
    if log_path != "":
        os.environ[_DATASYSTEM_CLIENT_LOG_DIR] = log_path


def _gen_random_address(mgr):
    host, port = mgr.server_address.split(":")
    return host, port


def _get_runtime(mgr: ConfigManager, logger):
    if mgr.local_mode:
        return LocalModeRuntime()

    if mgr.on_cloud:
        from yr.invoke_client.posix_client import PosixClient
        invoke_client = PosixClient()
        from fnruntime.rpc.sdk import RPClient
        ds_client = RPClient().ds_client
    else:
        host, port = mgr.ds_address.split(":")
        if mgr.in_cluster:
            client = ObjectClient(host=host, port=int(port))
            stream_client = StreamClient(host=host, port=int(port))
            stream_client.init()
            from yr.invoke_client.posix_client import PosixClient, start_server
            invoke_client = PosixClient()
            server_ip, _ = _gen_random_address(mgr)
            real_port = start_server(server_ip + ":0", logger)
            import grpc
            from yr.rpc import bus_service_pb2, bus_service_pb2_grpc
            channel = grpc.insecure_channel(mgr.server_address)
            stub = bus_service_pb2_grpc.BusServiceStub(channel)
            stub.DiscoverDriver(bus_service_pb2.DiscoverDriverRequest(
                driverIP=server_ip,
                driverPort=str(real_port),
                jobID=mgr.job_id)
            )
        else:
            client = AgentClient(host=host, port=int(port))
            stream_client = None
            from yr.invoke_client.outcluster_client import HttpInvokeClient
            invoke_client = HttpInvokeClient()
            invoke_client.init(mgr.server_address, mgr.invoke_timeout)

        client.init()
        from yr.storage.ds_client import DSClient
        ds_client = DSClient()
        ds_client.init(client, stream_client)
    ReferenceCount().init(ds_client)

    return cluster_mode_runtime.ClusterModeRuntime(ds_client=ds_client,
                                                   invoke_client=invoke_client)


def init(conf: Config = None) -> ClientInfo:
    """
    Init yr api

    Args:
        conf (Config) : This is the config which set by user, see the detail in class `yr.Config`.
    """
    if ConfigManager().is_init:
        raise RuntimeError("yr.init cannot be called twice")
    if conf is None:
        conf = Config()

    _unset_http_proxy()
    ConfigManager().init(conf)

    mpctl.start()
    _set_datasystem_client_log_dir()
    logger = logging.getLogger(_BASE_LOG_NAME)
    handler = logging.StreamHandler()
    fmt = logging.Formatter(
        fmt='[%(asctime)s.%(msecs)03d %(levelname)s %(funcName)s %(filename)s:%(lineno)d %(thread)d] %(message)s',
        datefmt='%Y-%m-%d %H:%M:%S')
    handler.setFormatter(fmt)
    logger.setLevel(ConfigManager().log_level)
    logger.handlers.clear()
    logger.addHandler(handler)
    logger.propagate = False
    runtime = _get_runtime(ConfigManager(), logger)
    Runtime().init(runtime)
    ConfigManager().runtime_id = generate_runtime_id()
    atexit.register(finalize)
    ConfigManager().is_init = True
    Timer().init()
    _logger.info(f"success to init, {ConfigManager().job_id}")
    return ClientInfo(ConfigManager().job_id)


def is_initialized() -> bool:
    """Check if yr.init has been called yet.

    Returns:
        True if yr.init has already been called and false otherwise.
    """
    return ConfigManager().is_init


def finalize():
    """
    Finalize the yr context, delete object and stateless instance
    """
    if ConfigManager().is_init:
        ReferenceCount().stop()
        Timer().stop()
        ObjRefCountingManager().clear()
        Runtime().rt.clear()
        ConfigManager().is_init = False
        mpctl.stop()


def exit():
    """
    Exit instance
    """
    if ConfigManager().on_cloud:
        Runtime().rt.exit_instance()
    else:
        _logger.warning("Not support exit out of cluster.")


@check_init
def put(obj: object):
    """
    Put object to datasystem

    Args:
        obj(object): This is a python object, will pickle and save to datasystem
    """
    # Make sure that the value is not an object ref.
    if isinstance(obj, ObjectRef):
        raise TypeError(
            "Calling 'put' on an ObjectRef is not allowed. If you really want to do this, "
            "you can wrap the ObjectRef in a list and call 'put' on it.")
    ref = ObjRefCountingManager().create_object_ref()
    return Runtime().rt.put(ref[0], obj)


def _check_object_ref(obj_refs):
    if not isinstance(obj_refs, list):
        raise TypeError(f"obj_refs type error, actual: [{type(obj_refs)}], element expect: <class 'ObjectRef'>")
    for ref in obj_refs:
        if not isinstance(ref, ObjectRef):
            raise TypeError(f"obj_refs type error, actual: [{type(obj_refs)}], element expect: <class 'ObjectRef'>")


def get(obj_refs: Union[ObjectRef, List], timeout=_DATASYSTEM_DEFAULT_TIMEOUT):
    """
    Get object from datasystem.

    This function will block when the key not in datasystem.

    Args:
        obj_refs (ObjectRef, List[ObjectRef]): The key of the object.
        timeout (int): Timeout(sec) for get, will be block when timeout set -1, default: 300, limit: -1, (0,∞)
    """
    if not ConfigManager().is_init:
        raise RuntimeError("system not initialized")
    lower_limit = 0
    if timeout <= lower_limit and timeout != -1:
        raise ValueError(f"timeout should be greater than {lower_limit}")
    is_single_obj = isinstance(obj_refs, ObjectRef)
    if is_single_obj:
        obj_refs = [obj_refs]
    _check_object_ref(obj_refs)
    objects = Runtime().rt.get(obj_refs, timeout)
    if is_single_obj:
        return objects[0]
    return objects


def is_on_cloud() -> bool:
    """
    Check weather running on cloud
    """
    return ConfigManager().on_cloud


@check_init
def wait(obj_refs: Union[ObjectRef, List], wait_num: int = 1, timeout: int = None) -> (List, List):
    """
    Wait for objectRefs complete

    Args:
        obj_refs(ObjectRef, List[ObjectRef]): List of objectRef to attempt to wait
        wait_num(int): Number of minimum objectRef completions required, default: 1, limit: [0, len(List[ObjectRef])]
        timeout(int): Timeout(sec), default: None，limit: None, -1, [0, MAX_INT]
            None, -1 as long as possible wait until the future is complete
    Returns:
        ready(list[ObjectRef]): Completed objectRef list
        unready(list[ObjectRef]): Uncompleted objectRef list
    """
    if isinstance(obj_refs, ObjectRef):
        obj_refs = [obj_refs]
    _check_object_ref(obj_refs)
    if len(obj_refs) != len(set(obj_refs)):
        raise ValueError("obj_refs value error: duplicate obj_ref exists in the list")
    if wait_num == 0 or timeout == 0:
        return [], obj_refs

    if not isinstance(wait_num, int):
        raise TypeError(f"'invalid wait_num type, actual: {type(wait_num)}, expect: <class 'int'>")
    if wait_num < 0 or wait_num > len(obj_refs):
        raise ValueError(f"invalid wait_num value, actual: {wait_num}, expect: [0, {len(obj_refs)}]")

    if timeout is not None:
        if not isinstance(timeout, int):
            raise TypeError(f"invalid timeout type, actual: {type(timeout)}, expect: <class 'int'>")
        if timeout != -1 and timeout < 0 or timeout > _MAX_INT:
            raise ValueError(f"invalid timeout value, actual: {timeout}, expect:None, -1, [0, {_MAX_INT}]")
        # -1, None, both blocking and waiting, adapting to the cpp
        timeout = None if timeout == -1 else timeout
    return cluster_mode_runtime.wait(obj_refs, wait_num, timeout)


@check_init
def cancel(obj_refs: Union[List[ObjectRef], ObjectRef]):
    """
    Cancel tasks

    Args:
        obj_refs(ObjectRef, list[ObjectRef]): List of objectRef to attempt to cancel
    """
    if isinstance(obj_refs, ObjectRef):
        obj_refs = [obj_refs]
    _check_object_ref(obj_refs)
    Runtime().rt.cancel(obj_refs)


def invoke(*args, **kwargs):
    """
    Function decorator

    This decorator will make a function invoked in cluster
    Args:
        func(Callable): function which need call in cluster
        invoke_options(InvokeOptions): invoke options for users to set resources
        return_nums(int): number of return values
    """
    if len(args) == 1 and len(kwargs) == 0 and callable(args[0]):
        return function_proxy.make_decorator()(args[0])
    invoke_options = kwargs.get("invoke_options", None)
    return_nums = kwargs.get("return_nums", None)
    return function_proxy.make_decorator(invoke_options, return_nums)


def instance(*args, **kwargs):
    """
    Class decorator

    This decorator will make a class created in cluster
    Args:
        invoke_options(InvokeOptions): invoke options for users to set resources
    """
    if len(args) == 1 and len(kwargs) == 0 and callable(args[0]):
        return instance_proxy.make_decorator()(args[0])
    invoke_options = kwargs.get("invoke_options", None)
    return instance_proxy.make_decorator(invoke_options)


def method(*args, **kwargs):
    """
    Class method decorator

    This decorator will make a class method in cluster
    Args:
        return_nums(int): number of return values of method
    """
    if len(args) != 0 or "return_nums" not in kwargs:
        raise ValueError("invalid params")

    def annotate_method(class_method):
        return_nums = kwargs.get("return_nums", None)
        if not isinstance(return_nums, int):
            raise TypeError(f"invalid return_nums type: {type(return_nums)}, should be an int")
        class_method.__return_nums__ = return_nums
        return class_method

    return annotate_method


def get_instance(name, namespace=None) -> instance_proxy.InstanceProxy:
    """
    Get a proxy of the named instance.

    Gets a proxy of an instance with the given name and namespace. The instance
    name and namespace must have been set by Instance.options()

    Args:
        name(str): the name of the instance
        namespace(str): the namespace of the instance

    Returns:
        InstanceProxy of the instance
    """
    if not isinstance(name, str):
        raise TypeError(f"invalid name type: {type(name)}, should be a str")
    if namespace and not isinstance(namespace, str):
        raise TypeError(f"invalid namespace type: {type(namespace)}, "
                        f"should be a str")
    try:
        ins_proxy = instance_proxy.get_instance_by_name(name, namespace)
    except Exception as e:
        raise RuntimeError(f"failed to get instance by name, {str(e)}") from e
    return ins_proxy


class cpp_instance_class:
    """
    Use to create a cpp instance class
    """

    def __init__(self, class_name: str, factory_name: str):
        """
        CPP Class Creator

        Args:
            class_name: CPP class name
            factory_name: CPP static constructor name
        """
        function_urn = ConfigManager().get_function_id_by_language(utils.LANGUAGE_CPP)
        self.__function_key__ = ''
        if function_urn is not None:
            self.__function_key__ = get_function_from_urn(function_urn)
        self.__class_name__ = class_name
        self.__factory_name__ = factory_name

    def invoke(self, *args, **kwargs):
        """
        Create a CPP instance in cluster
        """
        creator = instance_proxy.make_cpp_instance_creator(self)
        return creator.invoke(*args, **kwargs)

    def options(self, invoke_options):
        """
        Set user invoke options
        Args:
            invoke_options: invoke options for users to set resources
        """
        creator = instance_proxy.make_cpp_instance_creator(self)
        return creator.options(invoke_options)

    def get_function_key(self):
        """
        Get function key in this CPP instance
        """
        return self.__function_key__

    def get_class_name(self):
        """
        Get class name in this CPP instance
        """
        return self.__class_name__

    def get_factory_name(self):
        """
        Get factory function name in this CPP instance
        """
        return self.__factory_name__


def cpp_function(function_name: str):
    """
    Create proxy for CPP normal function

    Args:
        function_name: CPP normal function name

    Returns:
        FunctionProxy: the proxy for CPP function

    """
    function_urn = ConfigManager().get_function_id_by_language(utils.LANGUAGE_CPP)
    function_key = ''
    if function_urn is not None:
        function_key = get_function_from_urn(function_urn)
    return function_proxy.make_cpp_function_proxy(function_name, function_key)


@check_init
def get_stream_client() -> StreamClient:
    """ Get data system stream client.

    Return:
        StreamClient: The stream client for data system.

    Raise:
        RuntimeError: Raise a runtime error if stream client is None.
    """
    return Runtime().rt.get_stream_client()
