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

""" apply mpctl start/stop method """
import os
import subprocess
import secrets
import string
from xml.etree import ElementTree

from yr.config import ConfigManager
from yr.utils import Singleton

RUNTIME_MGR_PORT = "runtime_mgr_port"
DEFAULT_RANDOM_LENGTH = 16


@Singleton
class AutoDeploymentState:
    """
    DeploymentState singleton
    """

    def __init__(self):
        self.__state = False

    @property
    def state(self) -> bool:
        """ get state """
        return self.__state

    @state.setter
    def state(self, val: bool) -> bool:
        """ set state

        Args:
            val (bool): auto deployment state
        """
        self.__state = val

    def reset(self):
        """
        reset state
        """
        self.__state = False


def _gen_random_str(length=DEFAULT_RANDOM_LENGTH) -> str:
    system_random = secrets.SystemRandom()

    if length < DEFAULT_RANDOM_LENGTH // 2:
        return ''.join(system_random.sample(string.ascii_lowercase, length))

    digits_count = system_random.randint(1, length // 2)
    uppercase_count = system_random.randint(1, length - digits_count - 2)
    lowercase_count = length - digits_count - uppercase_count

    contents = system_random.sample(string.digits, digits_count) + \
               system_random.sample(string.ascii_uppercase, uppercase_count) + \
               system_random.sample(string.ascii_lowercase, lowercase_count)

    system_random.shuffle(contents)
    return ''.join(contents)


def _get_mpctl_path() -> str:
    """
    get mpctl path
    """
    current_path = os.path.abspath(__file__)
    mpctl_path = os.path.join(os.path.dirname(current_path), "../../../../bin/mpctl")
    mpctl_path = os.path.realpath(mpctl_path)
    if not os.path.isfile(mpctl_path):
        raise FileNotFoundError(f"{mpctl_path} not found")
    return mpctl_path


def _get_config_path() -> str:
    """
    get config.xml path
    """
    current_path = os.path.abspath(__file__)
    config_path = os.path.join(os.path.dirname(current_path), "../../../../config/config.xml")
    config_path = os.path.realpath(config_path)
    if not os.path.isfile(config_path):
        raise FileNotFoundError(f"{config_path} not found")
    return config_path


def _is_runtime_manager_running() -> bool:
    def _get_runtime_mgr_port() -> str:
        config_path = _get_config_path()
        tree_root = ElementTree.parse(config_path)
        element = tree_root.findall(RUNTIME_MGR_PORT)
        if not element:
            raise RuntimeError("parse runtime_mgr_port from config.xml failed")
        return element[0].text

    runtime_mgr_port = _get_runtime_mgr_port()
    cmd = f"lsof -i:{runtime_mgr_port}"
    exit_code, _ = subprocess.getstatusoutput(cmd)
    if exit_code != 0:
        return False
    return True


def _get_resource_option():
    resources = {
        "--cpu": ConfigManager().deployment_config.cpu,
        "--mem": ConfigManager().deployment_config.mem,
        "--datamem": ConfigManager().deployment_config.datamem,
    }
    return " ".join([f"{name} {value}" for name, value in resources.items() if value > 0])


def _get_spill_option():
    options = {
        "--spillPath": ConfigManager().deployment_config.spill_path,
        "--spillLimit": ConfigManager().deployment_config.spill_limit,
    }
    return " ".join([f"{name} {value}" for name, value in options.items() if value])


def start():
    """
    start distribute-executor
    """
    if not ConfigManager().auto:
        return

    if _is_runtime_manager_running():
        return

    AutoDeploymentState().state = True
    ip_addr, _ = ConfigManager().server_address.split(':')
    cmd = f"{_get_mpctl_path()} start -m -a {ip_addr} -l {ip_addr} -p {_gen_random_str()} " \
          f"{_get_resource_option()} {_get_spill_option()}"
    exit_code, _ = subprocess.getstatusoutput(cmd)
    if exit_code != 0:
        stop()
        raise RuntimeError("failed to start distribute-executor")


def stop():
    """
    stop distribute-executor
    """
    if not ConfigManager().auto:
        return

    if not AutoDeploymentState().state:
        return

    cmd = f"{_get_mpctl_path()} stop"
    exit_code, _ = subprocess.getstatusoutput(cmd)
    AutoDeploymentState().state = False
    if exit_code != 0:
        raise RuntimeError("failed to stop distribute-executor")
