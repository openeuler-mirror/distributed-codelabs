# !/usr/bin/env python3
# coding=UTF-8
# Copyright (c) 2022 Huawei Technologies Co., Ltd
#
# This software is licensed under Mulan PSL v2. You can use this software
# according to the terms and conditions of the Mulan PSL v2. You may obtain a
# copy of Mulan PSL v2 at:
#
# http://license.coscl.org.cn/MulanPSL2
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.

"""util common methods"""
import json
import os
import sys

from fnruntime.tls import read_cert_from_cluster


def get_tls_config():
    """
    get tls configuration
    """
    config_path = "/home/snuser/config/runtime.json"
    if os.getenv("YR_BARE_MENTAL") is not None:
        config_path = sys.path[0] + '/../config/runtime.json'
    with open(config_path, 'r') as config_file:
        config_json = json.load(config_file)

        tls_config = None
        if "tlsEnable" in config_json:
            tls_enable = config_json["tlsEnable"]
            if tls_enable:
                tls_config = read_cert_from_cluster(config_json)
        else:
            tls_enable = False
    return tls_enable, tls_config
