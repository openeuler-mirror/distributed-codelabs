#!/usr/bin/env python
# coding=utf-8
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
class instance manager
"""
from yr.utils import Singleton


@Singleton
class InstanceManager:
    """
    InstanceManager stores class instance
    """

    def __init__(self):
        self.__instance_map = {}

    def load_instance(self, instance_id):
        """
        load class instance
        """
        return self.__instance_map.get(instance_id)

    def save_instance(self, instance_id, instance_object):
        """
        save class instance
        """
        self.__instance_map[instance_id] = instance_object
