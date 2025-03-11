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

set -e

function with_backoff() {
    local max_attempts=9
    local timeout=1
    local attempt=0

    while [[ "$attempt" < "$max_attempts" ]]; do
        if "$@"; then
            return 0
        fi
        echo "failure! retrying in after ${timeout}s..." 1>&2
        sleep "$timeout"
        timeout=$((timeout * 2))
        attempt=$((attempt + 1))
    done
    echo "failed to exe command "$@" for the last time!" 1>&2
    return 1
}

function lookup_ip() {
    with_backoff nslookup "$1" | grep Address | grep -v "#53" | awk '{print $2}'
}

dns_server=$(grep nameserver /etc/resolv.conf | awk 'NR==1{print $2}')
if [ "${ENABLE_MINIO}" == "true" ]; then
    minio_ip=$(lookup_ip "minio")
fi

iptables -P INPUT ACCEPT
iptables -P FORWARD ACCEPT
iptables -P OUTPUT ACCEPT

ETCD_INFO=$(cat /home/sn/config/init.conf | grep "etcdAddr" | awk -F '=' '{print $2}')
etcd_ip=$(echo ${ETCD_INFO} | awk -F ':' '{print $1}')
etcd_port=$(echo "${ETCD_INFO}" | awk -F ':' '{print $2}')
if [ -n "${etcd_ip}" ] && [ -n "${etcd_port}" ]; then
    iptables -A OUTPUT -d "${etcd_ip}"/32 -p tcp -m tcp --dport "${etcd_port}" -j ACCEPT
fi

iptables -A OUTPUT -d "${dns_server}"/32 -p udp -j ACCEPT
iptables -A OUTPUT -d "${dns_server}"/32 -p tcp  -j ACCEPT
if [ -n "${minio_ip}" ]; then
    iptables -A OUTPUT -d "${minio_ip}"/32 -p tcp -m tcp --dport 9000 -j ACCEPT
fi

# 31501 is dataSystem port, for storing
iptables -A OUTPUT -p tcp -m tcp --dport 31501 -j ACCEPT

if [ -n "${SVC_CIDR}" ]; then
  iptables -A OUTPUT -d "${SVC_CIDR}" -p icmp --icmp-type 8 -j DROP
  iptables -A OUTPUT -d "${SVC_CIDR}" -p udp -j DROP
  iptables -A OUTPUT -d "${SVC_CIDR}" -p tcp -m tcp --tcp-flags SYN SYN -m state --state NEW -j DROP
  iptables -A OUTPUT -d "${SVC_CIDR}" -j ACCEPT
fi

if [ -n "${POD_CIDR}" ]; then
  iptables -A OUTPUT -d "${POD_CIDR}" -p icmp --icmp-type 8 -j DROP
  iptables -A OUTPUT -d "${POD_CIDR}" -p udp -j DROP
  iptables -A OUTPUT -d "${POD_CIDR}" -p tcp -m tcp --tcp-flags SYN SYN -m state --state NEW -j DROP
  iptables -A OUTPUT -d "${POD_CIDR}" -j ACCEPT
fi

if [ -n "${HOST_CIDR}" ]; then
  arr=$(echo "${HOST_CIDR}" | tr "," "\n")
  for value in $arr; do
      iptables -A OUTPUT -d "${value}" -p icmp --icmp-type 8 -j DROP
      iptables -A OUTPUT -d "${value}" -p udp -j DROP
      iptables -A OUTPUT -d "${value}" -p tcp -m tcp --tcp-flags SYN SYN -m state --state NEW -j DROP
      iptables -A OUTPUT -d "${value}" -j ACCEPT
  done
fi
iptables-save

exit 0
