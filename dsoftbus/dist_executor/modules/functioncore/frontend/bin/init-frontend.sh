#!/bin/bash
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

# Added the capability of Linux to receive TCP connections.


file="/proc/sys/net/core/somaxconn"
if [  -f "$file" ]; then
    sysctl -w net.core.somaxconn=10240
fi
file="/proc/sys/net/ipv4/tcp_max_syn_backlog"
if [  -f "$file" ]; then
    sysctl -w net.ipv4.tcp_max_syn_backlog=10240
fi
echo "config finished"
exit 0
