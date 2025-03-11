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

""" handler example """
import sys

import yr


@yr.invoke
def hello_world():
    """ example """
    return "hello world"


def usage():
    """ print usage """
    print("""
        Usage:
            python handler.py {MASTER_IP_ADDRESS}
    """)


if __name__ == '__main__':
    if len(sys.argv) != 2:
        print(f"invalid args: {sys.argv}")
        usage()
        sys.exit(1)

    ip_addr = sys.argv[-1]

    conf = yr.Config(
        function_id="sn:cn:yrk:12345678901234561234567890123456:function:0-default-func:$latest",
        server_address=f"{ip_addr}:31220",
        ds_address=f"{ip_addr}:31502",
        log_level="INFO"
    )

    yr.init(conf)

    res = [hello_world.invoke() for i in range(1)]
    print(yr.get(res))
    yr.finalize()
