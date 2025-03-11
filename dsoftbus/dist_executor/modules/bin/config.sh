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
# Perform hot backups of Oracle databases.

set -e
readonly BASE_DIR=$(dirname "$(readlink -f "$0")")
readonly CONFIG_FILE="${BASE_DIR}/../config/config.xml"

[[ ! -f "${CONFIG_FILE}" ]] && echo "The ${CONFIG_FILE} is not exist. This program is exiting" && exit 1
. ${BASE_DIR}/utils.sh

# get node IP automatically
function get_ip_auto()
{
    log_info "Try to get IP address of this device"
    local ip_list_len=$(ifconfig | grep inet | grep -v "127.0.0.1" | grep -v "172.17.0.1" | grep -v inet6 | awk '{print $2}' | wc -l)
    if [ "$ip_list_len" -ge 2 ]; then
        log_info "Cannot get IP address of this device. Please choose the appropriate one manually"
        for i in $(seq 1 "$ip_list_len"); do
            local ip=$(ifconfig | grep inet | grep -v "127.0.0.1" | grep -v "172.17.0.1" | grep -v inet6 | awk '{print $2}' | head -n "$i" | tail -n 1)
            read -rp "Local IP address is ${ip}. Press y to ensure or press Enter to skip:" conf
            if [ "x$conf" == 'xy' ]; then
                local_ip=$ip
                break
            fi
        done
    fi
    if [ "$ip_list_len" -eq 1 ]; then
        local_ip=$(ifconfig | grep inet | grep -v "127.0.0.1" | grep -v "172.17.0.1" | grep -v inet6 | awk '{print $2}')
    fi
}

# enter node IP manually
function get_ip_manually()
{
    local_ip="$1"
    log_info "The IP address entered manually is ${local_ip}"
}

# ----------------------------------------------------------------------
# funcname: usage
# description:  help of this script
# parameters:   void
# return value: void
# ----------------------------------------------------------------------
usage()
{
    echo -e "Usage: bash config.sh [-p local_ip] [-h help]"
    echo -e "Options:"
    echo -e "     -p local_ip, such as 127.0.0.1"
    echo -e "     -h usage help"
    echo -e "      "
    echo -e "example:"
    echo -e "   bash config.sh -p \"127.0.0.1\" "
    echo -e "   bash config.sh -h"
    echo -e ""
}

while getopts 'p:h' opt; do
    case "${opt}" in
    p)
        OPTION="$OPTARG"
        get_ip_manually "$OPTION"
        ;;
    h)
        usage
        exit 0
        ;;
    *)
        ;;
    esac
done

# check whether there is a profile in the directory
# it will delete this profile if there is
if [ -z "${OPTION}" ]; then
    get_ip_auto
    log_info "The selected IP address is ${local_ip}"
fi

if [ "x${local_ip}" == 'x' ]; then
    log_error "The IP address is not configured. This program is exiting"
    exit 1
fi

if [ "$(echo "$local_ip" | grep ":")" != "" ]; then
    log_info "The IP address contains a mark ':', which needs to be deleted"
    local_ip=$(echo "$local_ip" | awk -F":" '{print $2}')
    log_info "The IP address has been modified to ${local_ip}"
fi

# synchronize the IP address into the profile
log_info "The IP address writen into the profile is ${local_ip}"
if [ "x$local_ip" != "x" ]; then
    sed -i "s#<local_ip>.*</local_ip>#<local_ip>${local_ip}</local_ip>#g" "$CONFIG_FILE"
    sed -i "s#<redis_ip>.*</redis_ip>#<redis_ip>${local_ip}</redis_ip>#g" "$CONFIG_FILE"
    sed -i "s#<minio_ip>.*</minio_ip>#<minio_ip>${local_ip}</minio_ip>#g" "$CONFIG_FILE"
    sed -i "s#<etcd_ip>.*</etcd_ip>#<etcd_ip>${local_ip}</etcd_ip>#g" "$CONFIG_FILE"
    sed -i "s#<master_ip>.*</master_ip>#<master_ip>${local_ip}</master_ip>#g" "$CONFIG_FILE"
    log_info "please set password by yourself!!!"
else
    log_error "Got none IP!!!"
    exit 1
fi

exit 0
