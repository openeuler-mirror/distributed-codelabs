#!/usr/bin/env python3
# coding=UTF-8
# Copyright (c) 2022 Huawei Technologies Co., Ltd
#
# This software is licensed under Mulan PSL v2.
# You can use this software according to the terms and
# conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#
# http://license.coscl.org.cn/MulanPSL2
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS,
# WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.

"""tls for two-way authentication"""

from dataclasses import dataclass


@dataclass
class ClusterTLSConfig:
    """
    In cluster for ssl/tls
    """
    root_cert_data: bytes = None
    module_cert_data: bytes = None
    module_key_data: bytes = None


def read_cert_from_cluster(config_json) -> ClusterTLSConfig:
    """
    Read cert file from cluster config.json
    """
    config = ClusterTLSConfig()
    root_cert_file_path = config_json["rootCAFile"]
    module_cert_file_path = config_json["moduleCertFile"]
    module_key_file_path = config_json["moduleKeyFile"]
    with open(root_cert_file_path, 'rb') as f:
        config.root_cert_data = f.read()
    with open(module_cert_file_path, 'rb') as f:
        config.module_cert_data = f.read()
    with open(module_key_file_path, 'rb') as f:
        config.module_key_data = f.read()
    return config
