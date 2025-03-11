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

# ----------------------------------------------------------------------
# funcname:     log_info.
# description:  Print build info log.
# parameters:   NA
# return value: NA
# ----------------------------------------------------------------------
log_info() {
    echo "[BUILD_INFO][$(date +%b\ %d\ %H:%M:%S)]$*"
}

# ----------------------------------------------------------------------
# funcname:     log_warning.
# description:  Print build warning log.
# parameters:   NA
# return value: NA
# ----------------------------------------------------------------------
log_warning() {
    echo "[BUILD_WARNING][$(date +%b\ %d\ %H:%M:%S)]$*"
}

# ----------------------------------------------------------------------
# funcname:     log_error.
# description:  Print build error log.
# parameters:   NA
# return value: NA
# ----------------------------------------------------------------------
log_error() {
    echo "[BUILD_ERROR][$(date +%b\ %d\ %H:%M:%S)]$*"
}

# ----------------------------------------------------------------------
# funcname:     die.
# description:  Print build error log.
# parameters:   NA
# return value: NA
# ----------------------------------------------------------------------
die() {
    log_error "$*"
    stty echo
    exit 1
}

# enter node IP manually
function get_ip_manually() {
    local_ip="$1"
    echo "The IP address entered manually is ${local_ip}"
}

# get node IP automatically
function get_ip_auto() {
    echo "Try to get IP address of this device"
    ip_list_len=$(ifconfig | grep inet | grep -v "127.0.0.1" | grep -v "172.17.0.1" | grep -v inet6 | awk '{print $2}' | wc -l)
    local_ip=0
    if [ "$ip_list_len" -ge 2 ]; then
        echo "Cannot get IP address of this device. Please choose the appropriate one manually"
        for i in $(seq 1 "$ip_list_len"); do
            ip=$(ifconfig | grep inet | grep -v "127.0.0.1" | grep -v "172.17.0.1" | grep -v inet6 | awk '{print $2}' | head -n "$i" | tail -n 1)
            read -rp "Local IP address is ${ip}. Press y to ensure or press Enter to skip:" conf
            conf_flag='x'$conf
            if [ "$conf_flag" == 'xy' ]; then
                local_ip=$ip
                break
            fi
        done
    fi
    if [ "$ip_list_len" -eq 1 ]; then
        local_ip=$(ifconfig | grep inet | grep -v "127.0.0.1" | grep -v "172.17.0.1" | grep -v inet6 | awk '{print $2}')
    fi
}

# please install libxml2 first
function init_config_var() {
    local config_file=$1
    # shellcheck disable=SC2155
    export DEPLOY_PATH="$(xmllint --xpath "string(//config/deploy_path)" "${config_file}")"
    # shellcheck disable=SC2155
    export LOCAL_IP="$(xmllint --xpath "string(//config/local_ip)" "${config_file}")"
    # shellcheck disable=SC2155
    export ETCD_AUTH_TYPE="$(xmllint --xpath "string(//config/etcd_auth_type)" "${config_file}")"
    # shellcheck disable=SC2155
    export ETCD_PORT="$(xmllint --xpath "string(//config/etcd_port)" "${config_file}")"
    # shellcheck disable=SC2155
    export ETCD_IP="$(xmllint --xpath "string(//config/etcd_ip)" "${config_file}")"
    # shellcheck disable=SC2155
    export ETCD_PROXY_PORT="$(xmllint --xpath "string(//config/etcd_proxy_ports)" "${config_file}")"
    # shellcheck disable=SC2155
    export ETCD_PROXY_ENABLE="$(xmllint --xpath "string(//config/etcd_proxy_enable)" "${config_file}")"
    # shellcheck disable=SC2155
    export MINIO_IP="$(xmllint --xpath "string(//config/minio_ip)" "${config_file}")"
    # shellcheck disable=SC2155
    export MINIO_PORT="$(xmllint --xpath "string(//config/minio_port)" "${config_file}")"
    # shellcheck disable=SC2155
    export REDIS_PORT="$(xmllint --xpath "string(//config/redis_port)" "${config_file}")"
    # shellcheck disable=SC2155
    export WORKERMGR_LISTEN_PORT="$(xmllint --xpath "string(//config/workermgr_listen_port)" "${config_file}")"
    # shellcheck disable=SC2155
    export CODE_DIR="$(xmllint --xpath "string(//config/code_dir)" "${config_file}")"
    # shellcheck disable=SC2155
    export DS_MASTER_IP="$(xmllint --xpath "string(//config/ds_master_ip)" "${config_file}")"
    # shellcheck disable=SC2155
    export DS_MASTER_PORT="$(xmllint --xpath "string(//config/ds_master_port)" "${config_file}")"
    # shellcheck disable=SC2155
    export LOG_LEVEL="$(xmllint --xpath "string(//config/log_level)" "${config_file}")"
    # shellcheck disable=SC2155
    export RUNTIME_LOG_LEVEL="$(xmllint --xpath "string(//config/runtime_log_level)" "${config_file}")"
    # shellcheck disable=SC2155
    export FUNCTIONCORE_LOG_PATH="$(xmllint --xpath "string(//config/functioncore_log_path)" "${config_file}")"
    if [ "X$FUNCTIONCORE_LOG_PATH" == "Xdefault" ]; then
        unset FUNCTIONCORE_LOG_PATH
    fi
    # shellcheck disable=SC2155
    export RUNTIME_LOG_PATH="$(xmllint --xpath "string(//config/runtime_log_path)" "${config_file}")"
    if [ "X$RUNTIME_LOG_PATH" == "Xdefault" ]; then
        unset RUNTIME_LOG_PATH
    fi
    # shellcheck disable=SC2155
    export GLOBAL_SCHEDULER_PORT="$(xmllint --xpath "string(//config/global_scheduler_port)" "${config_file}")"
    export LOKI_IP="$(xmllint --xpath "string(//config/loki_ip)" "${config_file}")"
    export LOKI_PORT="$(xmllint --xpath "string(//config/loki_port)" "${config_file}")"
}

declare -A PORT_HASH_MAP
declare -a ORDERED_PORTS=()
function get_free_port() {
    local BIND_IP="$1"
    local PORT_MIN="$2"
    local PORT_MAX="$3"

    if ! command -v nc &>/dev/null; then
        PORT=$(shuf -i "${PORT_MIN}"-"${PORT_MAX}" -n 1)
        while [[ ${PORT_HASH_MAP[$PORT]} ]]; do
            PORT=$(shuf -i "${PORT_MIN}"-"${PORT_MAX}" -n 1)
        done
        PORT_HASH_MAP[$PORT]=$PORT
        ORDERED_PORTS=($PORT "${ORDERED_PORTS[@]}")
        echo "$PORT"
        return 0
    fi

    CHECK="port not assigned"
    PORT="port"
    # shellcheck disable=SC2236
    while [[ ! -z $CHECK ]]; do
        PORT=$(shuf -i "${PORT_MIN}"-"${PORT_MAX}" -n 1)
        if [[ ! ${PORT_HASH_MAP[$PORT]} ]]; then
            set +e; CHECK=$(LD_LIBRARY_PATH="" timeout 0.2 nc -l "$BIND_IP" "$PORT" 2>&1 >/dev/null)
        fi
    done
    PORT_HASH_MAP[$PORT]=$PORT
    ORDERED_PORTS=($PORT "${ORDERED_PORTS[@]}")
    echo "$PORT"
    return 0
}


function check_port(){
    if ! command -v nc &>/dev/null; then
        return 0
    fi
    local BIND_IP="$1"
    local PORT="$2"
    CHECK=$(LD_LIBRARY_PATH="" timeout 0.2 nc -l "$BIND_IP" "$PORT" 2>&1 >/dev/null)
    if [[ ! -z $CHECK ]]; then
        return 1
    else
        return 0
    fi

}

function getJsonValuesByAwk() {
    awk -v json="$1" -v key="$2" -v defaultValue="$3" 'BEGIN{
        foundKeyCount = 0
        while (length(json) > 0) {
            pos = match(json, "\""key"\"[ \\t]*?:[ \\t]*");
            if (pos == 0) {if (foundKeyCount == 0) {print defaultValue;} exit 0;}

            ++foundKeyCount;
            start = 0; stop = 0; layer = 0;
            for (i = pos + length(key) + 1; i <= length(json); ++i) {
                lastChar = substr(json, i - 1, 1)
                currChar = substr(json, i, 1)

                if (start <= 0) {
                    if (lastChar == ":") {
                        start = currChar == " " ? i + 1: i;
                        if (currChar == "{" || currChar == "[") {
                            layer = 1;
                        }
                    }
                } else {
                    if (currChar == "{" || currChar == "[") {
                        ++layer;
                    }
                    if (currChar == "}" || currChar == "]") {
                        --layer;
                    }
                    if ((currChar == "," || currChar == "}" || currChar == "]") && layer <= 0) {
                        stop = currChar == "," ? i : i + 1 + layer;
                        break;
                    }
                }
            }

            if (start <= 0 || stop <= 0 || start > length(json) || stop > length(json) || start >= stop) {
                if (foundKeyCount == 0) {print defaultValue;} exit 0;
            } else {
                print substr(json, start, stop - start);
            }

            json = substr(json, stop + 1, length(json) - stop)
        }
    }'
}