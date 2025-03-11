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

"""runtime server"""
import sys

from fnruntime import log
from fnruntime.rpc import rpc_server


def start_up():
    """
    Start up
    """
    runtime_id = ""
    if len(sys.argv) == 5:
        runtime_id = sys.argv[4]
    log.init_log(runtime_id)
    if len(sys.argv) < 4:
        log.get_logger().error("invalid starting parameters of python runtime")
        sys.exit(1)
    log.get_logger().debug(f"{sys.argv}")
    rpc_server.serve(sys.argv[2], sys.argv[3])


if __name__ == '__main__':
    start_up()
