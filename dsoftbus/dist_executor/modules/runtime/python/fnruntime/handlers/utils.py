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

"""python runtime handlers common utils"""
import importlib.util
from os import path

from fnruntime import log
from yr.utils import Singleton

_DEFAULT_ADMIN_FUNC_PATH = "/adminfunc/"


@Singleton
class InstanceManager:
    """
    InstanceManager stores class instance
    """

    def __init__(self):
        self.__instance = None
        self.__config = None
        self.__class_code = None

    @property
    def config(self):
        """get config"""
        return self.__config

    @config.setter
    def config(self, value):
        """save config"""
        self.__config = value

    @property
    def class_code(self):
        """get class code"""
        return self.__class_code

    @class_code.setter
    def class_code(self, code):
        """set class code"""
        self.__class_code = code

    def init(self, instance=None):
        """"
        save instance
        """
        self.__instance = instance

    def instance(self):
        """"
        get instance
        """
        return self.__instance


@Singleton
class CodeManager:
    """
    CodeManager provides code loading service
    it will try to cache the code and load from the cache as possible
    so it will only load
    * one module at most once (identified by the module file path)
    * one function at most once (identified by the module and function name)
    """

    def __init__(self):
        self.code_map = {}
        # module cache is a dict with { code_file_path : module }
        self.module_cache = {}

    def register(self, function_key, function_obj):
        """
        register function code to code manager
        """
        self.code_map[function_key] = function_obj

    def load(self, function_key):
        """
        load function code
        """
        return self.code_map.get(function_key)

    def get_code_from_local(self, code_dir, module_name, entry_name):
        """get code from local,
        throw error if module not exists
        return None if module exists but entry not exists
        """
        log.get_logger().debug("get python code [%s] from local file [%s/%s.py]", entry_name, code_dir, module_name)
        code_key = module_name + "%%" + entry_name
        code = self.load(code_key)
        if code is not None:
            return code

        module = self.load_module(code_dir, module_name)
        code = getattr(module, entry_name, None)
        if code is not None:
            self.register(code_key, code)
        return code

    def load_module(self, code_dir, module_name):
        """load module using cache"""
        file_path = path.join(code_dir, module_name + ".py")

        if not path.exists(file_path):
            admin_path = path.join(_DEFAULT_ADMIN_FUNC_PATH, module_name + ".py")
            if not path.exists(admin_path):
                raise ValueError("entry file does not exist: {}".format(file_path))
            file_path = admin_path

        module = self.module_cache.get(file_path, None)
        if module is not None:
            log.get_logger().debug("successfully load module [%s] from cache", file_path)
            return module

        log.get_logger().debug("loading module [%s] from file system", file_path)
        module_spec = importlib.util.spec_from_file_location(module_name, file_path)
        try:
            module = importlib.util.module_from_spec(module_spec)
        except ImportError as exp:
            log.get_logger().warning("failed to import user python module, %s", str(exp))
            raise

        module_spec.loader.exec_module(module)
        self.module_cache[file_path] = module
        return module
