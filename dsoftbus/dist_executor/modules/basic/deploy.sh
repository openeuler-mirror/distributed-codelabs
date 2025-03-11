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
readonly BASE_DIR=$(dirname "$(readlink -f "$0")")
readonly ARCH="$(echo "$(uname -m)")"
CONFIG_DIR="${BASE_DIR}/config"
CONFIG_FILE="${BASE_DIR}/../config/config.xml"
BIN_DIR="${BASE_DIR}/bin"
INSTALL_DIR="/opt"

[[ ! -f "${BASE_DIR}/utils.sh" ]] && echo "${BASE_DIR}/utils.sh is not exist" && exit 1
. ${BASE_DIR}/utils.sh

# identifier assignment
HOST_IP=$(hostname -I | awk '{print $1}')
PORT_MIN=25500
PORT_MAX=25800

unset http_proxy
unset https_proxy

# help of deploy.sh
function usage() {
    echo -e "Usage: bash deploy.sh [-o install/uninstall] [-m component_name/all] [-h help]"
    echo -e "Options:"
    echo -e "     -o install/uninstall, install component or uninstall component."
    echo -e "     -m component_name/all, such etcd, minio, redis"
    echo -e "     -n config file"
    echo -e "     -h usage help"
    echo -e "      "
    echo -e "example:"
    echo -e "     bash deploy.sh -o install -m all"
    echo -e "     bash deploy.sh -o uninstall -m all"
    echo -e "     bash deploy.sh -o install -m etcd"
    echo -e "     bash deploy.sh -o uninstall -m etcd"
    echo -e "     bash deploy.sh -h"
}

function install_etcd() {
    log_info "---- install etcd ----"
    local etcd_install_dir="${INSTALL_DIR}/basic/etcd"
    [ -d "${etcd_install_dir}" ] || mkdir -p "${etcd_install_dir}"
    rm -rf "${etcd_install_dir}"/*
    chmod 700 "${etcd_install_dir}"

    init_config_var "$CONFIG_FILE"
    [ -z "$FUNCTIONCORE_LOG_PATH" ] && FUNCTIONCORE_LOG_PATH="${etcd_install_dir}"
    mkdir -p "${FUNCTIONCORE_LOG_PATH}"
    log_file="${FUNCTIONCORE_LOG_PATH}"/etcd-run.log

    # load config
    local etcd_auth_type="$(xmllint --xpath "string(//config/etcd_auth_type)" "${CONFIG_FILE}")"
    ! [[ "${etcd_auth_type}X" == "NoauthX" || "${etcd_auth_type}X" == "TLSX" || "${etcd_auth_type}X" == "PWDX" ]] && log_error "please configure ${CONFIG_FILE} etcd_auth_type first!" && exit 1

    if [ "${etcd_auth_type}X" == "PWDX" ]; then
        local etcd_password="$(xmllint --xpath "string(//config/etcd_password)" "${CONFIG_FILE}")"
        [[ "${etcd_password}X" == "passwordX" ]] && log_error "please configure ${CONFIG_FILE} etcd_password first!" && exit 1
    fi
    local etcd_ip="$(xmllint --xpath "string(//config/etcd_ip)" "${CONFIG_FILE}")"
    [[ "${etcd_ip}X" == "IPX" ]] && log_error "please configure ${CONFIG_FILE} etcd_ip first!" && exit 1
    local etcd_port="$(xmllint --xpath "string(//config/etcd_port)" "${CONFIG_FILE}")"
    [[ "${etcd_port}X" == "etcd_portX" ]] && log_error "please configure ${CONFIG_FILE} etcd_port first!" && exit 1
    local etcd_peer_port="$(xmllint --xpath "string(//config/etcd_peer_port)" "${CONFIG_FILE}")"
    [[ "${etcd_peer_port}X" == "etcd_peer_portX" ]] && log_error "please configure ${CONFIG_FILE} etcd_peer_port first!" && exit 1
    local etcdctl_install_dir="$(xmllint --xpath "string(//config/etcdctl_install_dir)" "${CONFIG_FILE}")"
    [[ "${etcdctl_install_dir}X" == "etcdctl_install_dirX" ]] && etcdctl_install_dir=~/bin

    local etcd_ca_file="$(xmllint --xpath "string(//config/etcd_ca_file)" "${CONFIG_FILE}")"
    local etcd_cert_file="$(xmllint --xpath "string(//config/etcd_cert_file)" "${CONFIG_FILE}")"
    local etcd_key_file="$(xmllint --xpath "string(//config/etcd_key_file)" "${CONFIG_FILE}")"

    local etcd_proxy_enable="$(xmllint --xpath "string(//config/etcd_proxy_enable)" "${CONFIG_FILE}")"
    local client_cert_file="$(xmllint --xpath "string(//config/client_cert_file)" "${CONFIG_FILE}")"
    local client_key_file="$(xmllint --xpath "string(//config/client_key_file)" "${CONFIG_FILE}")"
    local etcd_proxy_nums="$(xmllint --xpath "string(//config/etcd_proxy_nums)" "${CONFIG_FILE}")"
    local etcd_proxy_ports="$(xmllint --xpath "string(//config/etcd_proxy_ports)" "${CONFIG_FILE}")"
    local etcd_no_fsync="$(xmllint --xpath "string(//config/etcd_no_fsync)" "${CONFIG_FILE}")"
    if ! check_port "${HOST_IP}" "${etcd_port}"; then
        log_error "${etcd_port} bind: address already in use"
        return 98
    fi
    if ! check_port "${HOST_IP}" "${etcd_peer_port}"; then
        log_error "${etcd_peer_port} bind: address already in use"
        return 98
    fi
    # start etcd`
    # WARNING: --unsafe-no-fsync will improve I/O performance, but might cause data lose when node crashed.
    if [ "${etcd_auth_type}" == "TLS" ]; then
        "${BIN_DIR}"/etcd/etcd \
            --name=etcd0 \
            --unsafe-no-fsync="${etcd_no_fsync}" \
            --data-dir="${etcd_install_dir}" \
            --initial-advertise-peer-urls="https://${etcd_ip}:${etcd_peer_port}" \
            --listen-client-urls="https://${etcd_ip}:${etcd_port}" \
            --advertise-client-urls="https://${etcd_ip}:${etcd_port}" \
            --listen-peer-urls="https://${etcd_ip}:${etcd_peer_port}" \
            --client-cert-auth \
            --trusted-ca-file="$etcd_ca_file" \
            --cert-file="$etcd_cert_file" \
            --key-file="$etcd_key_file" \
            --peer-client-cert-auth \
            --peer-trusted-ca-file "$etcd_ca_file" \
            --peer-cert-file "$etcd_cert_file" \
            --peer-key-file "$etcd_key_file" >>$log_file 2>&1 &
    else
        "${BIN_DIR}"/etcd/etcd \
            --name=etcd0 \
            --unsafe-no-fsync="${etcd_no_fsync}" \
            --data-dir="${etcd_install_dir}" \
            --listen-client-urls="http://${etcd_ip}:${etcd_port}" \
            --advertise-client-urls="http://${etcd_ip}:${etcd_port}" \
            --listen-peer-urls="http://${etcd_ip}:${etcd_peer_port}" >>$log_file 2>&1 &
    fi

    set +e
    count=0
    max_retry_num=7
    log_info "check etcd status" >>$log_file
    while [ "${count}" -lt "${max_retry_num}" ]; do
      if [ "${etcd_auth_type}" == "TLS" ]; then
        if "${BIN_DIR}"/etcd/etcdctl --cacert=${etcd_ca_file} --cert=${client_cert_file} --key=${client_key_file} --endpoints="https://${etcd_ip}:${etcd_port}" endpoint health; then
          log_info "etcd is ready"
          break
        else
            ((count++))
            log_info "number of retries: ${count}"
            sleep 2
        fi
      else
        if "${BIN_DIR}"/etcd/etcdctl --endpoints="${etcd_ip}:${etcd_peer_port}" endpoint health >>$log_file 2>&1; then
          log_info "etcd is ready" >>$log_file
          break
        else
          ((count++))
          log_info "number of retries: ${count}"
          sleep 2
        fi
      fi
    done

    if [ "${count}" -ge "${max_retry_num}" ]; then
        log_error "etcd is not ready" && exit 1
    fi
    set -e

    if [ "${etcd_auth_type}" == "Noauth" ]; then
        "${BIN_DIR}"/etcd/etcdctl --endpoints="${etcd_ip}:${etcd_peer_port}" auth disable
    elif [ "${etcd_auth_type}" == "PWD" ]; then
        # add root user and open auth
        "${BIN_DIR}"/etcd/etcdctl --endpoints="${etcd_ip}:${etcd_peer_port}" user add root:"${etcd_password}" >>$log_file 2>&1 &
        "${BIN_DIR}"/etcd/etcdctl --endpoints="${etcd_ip}:${etcd_peer_port}" auth enable >>$log_file 2>&1 &
    fi

    if [ "${etcd_proxy_enable}" = true ] && [ "$etcd_proxy_nums" -gt 0 ]; then
        log_info "etcd_proxy_enable:${etcd_proxy_enable}  etcd_proxy_nums:$etcd_proxy_nums etcd_proxy_ports:$etcd_proxy_ports" >>$log_file 2>&1
        for ((i = 1; i <= etcd_proxy_nums; i++)); do
            if ! check_port "${HOST_IP}" "${etcd_proxy_ports}"; then
                log_error "${etcd_proxy_ports} bind: address already in use"
                return 98
            fi
            if [ "${etcd_auth_type}" == "Noauth" ]; then
                log_info "start proxy-$i"
                "${BIN_DIR}"/etcd/etcd grpc-proxy start --endpoints="${etcd_ip}:${etcd_port}" \
                    --listen-addr="${etcd_ip}:${etcd_proxy_ports}" \
                    --advertise-client-url="${etcd_ip}:${etcd_proxy_ports}" \
                    --resolver-prefix="___grpc_proxy_endpoint" \
                    --resolver-ttl=60 >"${FUNCTIONCORE_LOG_PATH}"/etcd-proxy-"$i".log 2>&1 &
            elif [ "${etcd_auth_type}" == "TLS" ]; then
                log_info "start proxy-$i"
                "${BIN_DIR}"/etcd/etcd grpc-proxy start --endpoints="${etcd_ip}:${etcd_port}" \
                    --listen-addr="${etcd_ip}:${etcd_proxy_ports}" \
                    --advertise-client-url="${etcd_ip}:${etcd_proxy_ports}" \
                    --cert="$etcd_cert_file" --key="$etcd_key_file" --cacert="$etcd_ca_file" \
                    --cert-file="$client_cert_file" --key-file="$client_key_file" --trusted-ca-file="$etcd_ca_file" \
                    --resolver-prefix="___grpc_proxy_endpoint" \
                    --resolver-ttl=60 >"${FUNCTIONCORE_LOG_PATH}"/etcd-proxy-"$i".log 2>&1 &
            else
                log_error "proxy not support current auth type: ${etcd_auth_type}"
                exit 1
            fi
            get_free_port "$HOST_IP" "$PORT_MIN" "$PORT_MAX" >/dev/null
            etcd_proxy_ports=${ORDERED_PORTS[0]}
        done
    else
        log_info "etcd_proxy_enable:${etcd_proxy_enable}  etcd_proxy_nums:$etcd_proxy_nums" >>$log_file 2>&1
    fi

    log_info "---- install etcd success ----"
}

function uninstall_etcd() {
    log_info "---- uninstall etcd ----"
    local etcd_install_dir="${INSTALL_DIR}/basic/etcd"
    local etcd_ip="$(xmllint --xpath "string(//config/etcd_ip)" "${CONFIG_FILE}")"
    local etcd_port="$(xmllint --xpath "string(//config/etcd_port)" "${CONFIG_FILE}")"
    local etcd_proxy_ports="$(xmllint --xpath "string(//config/etcd_proxy_ports)" "${CONFIG_FILE}")"

    local etcd_proxy_pid=$(ps -ef | grep "${BIN_DIR}/etcd/etcd" | grep "${etcd_ip}:${etcd_port}" | grep "grpc-proxy" | awk '{ print $2 }')
    for i in ${etcd_proxy_pid}; do
        if ! kill -9 "$i"; then
            log_info "etcd proxy is not running"
        fi
    done

    local etcd_pid=$(ps -ef | grep "${BIN_DIR}/etcd/etcd" | grep "${etcd_ip}:${etcd_port}" | grep -v grep | grep -v PPID | grep -v deploy.sh | awk '{ print $2 }')
    local etcdctl_install_dir="$(xmllint --xpath "string(//config/etcdctl_install_dir)" "${CONFIG_FILE}")"
    [[ "${etcdctl_install_dir}X" == "etcdctl_install_dirX" ]] && etcdctl_install_dir=~/bin
    if ! kill -9 "${etcd_pid}"; then
        log_info "etcd is not running"
    fi

    [ -d "${etcd_install_dir}" ] && rm -rf "${etcd_install_dir}"
    log_info "---- uninstall etcd success ----"
}

function install_redis() {
    local state_storage_type="$(xmllint --xpath "string(//config/state_storage_type)" "${CONFIG_FILE}")"
    [[ "${state_storage_type}X" == "localX" ]] && log_info "state storage type is local, no need to start redis" && return 0
    [[ "${state_storage_type}X" != "redisX" ]] && log_error "please configure right ${CONFIG_FILE} state_storage_type" && exit 1

    log_info "---- install redis ----"

    local redis_install_dir="${INSTALL_DIR}/basic/redis"
    [ -d "${redis_install_dir}" ] || mkdir -p "${redis_install_dir}"
    init_config_var "$CONFIG_FILE"
    [ -z "$FUNCTIONCORE_LOG_PATH" ] && FUNCTIONCORE_LOG_PATH="${redis_install_dir}"
    mkdir -p "${FUNCTIONCORE_LOG_PATH}"
    rm -rf "${redis_install_dir}"/*
    cp -rf "${CONFIG_DIR}"/redis/* "${redis_install_dir}"

    # load config
    local redis_password="$(xmllint --xpath "string(//config/redis_password)" "${CONFIG_FILE}")"
    [[ "${redis_password}X" == "passwordX" ]] && log_error "please configure ${CONFIG_FILE} redis_password first!" && exit 1
    local redis_ip="$(xmllint --xpath "string(//config/redis_ip)" "${CONFIG_FILE}")"
    [[ "${redis_ip}X" == "IPX" ]] && log_error "please configure ${CONFIG_FILE} redis_ip first!" && exit 1
    local redis_port="$(xmllint --xpath "string(//config/redis_port)" "${CONFIG_FILE}")"
    [[ "${redis_port}X" == "redis_portX" ]] && log_error "please configure ${CONFIG_FILE} redis_port first!" && exit 1
    sed -i "/^dir/c\dir ${redis_install_dir}" "${redis_install_dir}"/redis.conf
    sed -i "s/core_redis_ip/${redis_ip}/g" "${redis_install_dir}"/redis.conf
    sed -i "s/core_redis_port/${redis_port}/g" "${redis_install_dir}"/redis.conf
    sed -i "s/core_redis_password/${redis_password}/g" "${redis_install_dir}"/redis.conf

    if ! check_port "${HOST_IP}" "${redis_port}"; then
        log_error "${redis_port} bind: address already in use"
        return 98
    fi
    # start redis
    "${BIN_DIR}"/redis/redis-server "${redis_install_dir}"/redis.conf >"${FUNCTIONCORE_LOG_PATH}"/redis-run.log 2>&1 &
    log_info "---- install redis success ----"
}

function uninstall_redis() {
    local state_storage_type="$(xmllint --xpath "string(//config/state_storage_type)" "${CONFIG_FILE}")"
    [[ "${state_storage_type}X" == "localX" ]] && log_info "state storage type is local, no need to uninstall redis" && return 0
    [[ "${state_storage_type}X" != "redisX" ]] && log_error "please configure right ${CONFIG_FILE} state_storage_type" && exit 1

    log_info "---- uninstall redis ----"
    local redis_install_dir="${INSTALL_DIR}/basic/redis"
    local redis_port="$(xmllint --xpath "string(//config/redis_port)" "${CONFIG_FILE}")"
    local redis_pid=$(ps -ef | grep "${BIN_DIR}"/redis/redis-server | grep "${redis_port}" | grep -v grep | grep -v PPID | grep -v deploy.sh | awk '{ print $2 }')
    if ! kill -9 "${redis_pid}"; then
        log_info "redis-server is not running"
    fi
    [ -d "${redis_install_dir}" ] && rm -rf "${redis_install_dir}"
    log_info "---- uninstall redis success ----"
}

function install_minio() {
    local function_storage_type="$(xmllint --xpath "string(//config/function_storage_type)" "${CONFIG_FILE}")"
    [[ "${function_storage_type}X" == "localX" ]] && log_info "function storage type is local, no need to start minio" && return 0
    [[ "${function_storage_type}X" != "s3X" ]] && log_error "please configure right ${CONFIG_FILE} function_storage_type" && exit 1

    log_info "---- install minio ----"
    local minio_install_dir="${INSTALL_DIR}/basic/minio"
    [ -d "${minio_install_dir}" ] || mkdir -p "${minio_install_dir}"
    init_config_var "$CONFIG_FILE"
    [ -z "$FUNCTIONCORE_LOG_PATH" ] && FUNCTIONCORE_LOG_PATH="${minio_install_dir}"
    mkdir -p "${FUNCTIONCORE_LOG_PATH}"
    rm -rf "${minio_install_dir}"/*

    # load config
    local minio_user="$(xmllint --xpath "string(//config/S3_AK)" "${CONFIG_FILE}")"
    [[ "${minio_user}X" == "accessKeyX" ]] && log_error "please configure ${CONFIG_FILE} S3_AK first!" && exit 1
    local minio_password="$(xmllint --xpath "string(//config/S3_SK)" "${CONFIG_FILE}")"
    [[ "${minio_password}X" == "secretKeyX" ]] && log_error "please configure ${CONFIG_FILE} S3_AK first!" && exit 1
    local minio_ip="$(xmllint --xpath "string(//config/minio_ip)" "${CONFIG_FILE}")"
    [[ "${minio_ip}X" == "IPX" ]] && log_error "please configure ${CONFIG_FILE} minio_ip first!" && exit 1
    local minio_port="$(xmllint --xpath "string(//config/minio_port)" "${CONFIG_FILE}")"
    [[ "${minio_port}X" == "minio_portX" ]] && log_error "please configure ${CONFIG_FILE} minio_port first!" && exit 1
    local console_port="$(xmllint --xpath "string(//config/minio_console_port)" "${CONFIG_FILE}")"
    [[ "${console_port}X" == "minio_console_portX" ]] && log_error "please configure ${CONFIG_FILE} minio_console_portX first!" && exit 1

    if ! check_port "${HOST_IP}" "${console_port}"; then
        log_error "${console_port} bind: address already in use"
        return 98
    fi
    if ! check_port "${HOST_IP}" "${minio_port}"; then
        log_error "${minio_port} bind: address already in use"
        return 98
    fi
    # start minio
    MINIO_ROOT_USER="${minio_user}" MINIO_ROOT_PASSWORD="${minio_password}" "${BIN_DIR}"/minio/minio server \
        "${minio_install_dir}"/data --console-address "${minio_ip}:${console_port}" --address "${minio_ip}:${minio_port}" >"${FUNCTIONCORE_LOG_PATH}"/minio-run.log 2>&1 &

    log_info "---- install minio success ----"
}

function uninstall_minio() {
    local function_storage_type="$(xmllint --xpath "string(//config/function_storage_type)" "${CONFIG_FILE}")"
    [[ "${function_storage_type}X" == "localX" ]] && log_info "function storage type is local, no need to uninstall minio" && return 0
    [[ "${function_storage_type}X" != "s3X" ]] && log_error "please configure right ${CONFIG_FILE} function_storage_type" && exit 1

    log_info "---- uninstall minio ----"
    local minio_install_dir="${INSTALL_DIR}/basic/minio"
    local minio_port="$(xmllint --xpath "string(//config/minio_port)" "${CONFIG_FILE}")"
    local minio_pid=$(ps -ef | grep "${BIN_DIR}"/minio/minio | grep "${minio_port}" | grep -v grep | grep -v PPID | grep -v deploy.sh | awk '{ print $2 }')
    if ! kill -9 "${minio_pid}"; then
        log_info "minio is not running"
    fi
    rm -rf "${minio_install_dir}"
    log_info "---- uninstall minio success ----"
}

# install module
function install_module() {
    case "$1" in
    "etcd")
        install_etcd
        ;;
    "redis")
        install_redis
        ;;
    "minio")
        install_minio
        ;;
    "all")
        install_etcd
        install_redis
        install_minio
        ;;
    esac
}

# uninstall module
function uninstall_module() {
    case "$1" in
    "etcd")
        uninstall_etcd
        ;;
    "redis")
        uninstall_redis
        ;;
    "minio")
        uninstall_minio
        ;;
    "all")
        uninstall_etcd
        uninstall_redis
        uninstall_minio
        ;;
    esac
}

function main() {
    while getopts "o:m:n:h" opt; do
        case "$opt" in
        o)
            OPTION=$OPTARG
            ;;
        m)
            MODULE=$OPTARG
            ;;
        n)
            CONFIG_FILE=$OPTARG
            ;;
        h)
            usage
            exit 0
            ;;
        *)
            log_error "Unknown parameter"
            echo -e ""
            usage
            exit 1
            ;;
        esac
    done

    [[ ! -f "${CONFIG_FILE}" ]] && log_error "${CONFIG_FILE} is not exist" && exit 1
    [[ ! -d "${CONFIG_DIR}" ]] && log_error "${CONFIG_DIR} is not exist" && exit 1
    [[ "${ARCH}X" != "aarch64X" && "${ARCH}X" != "x86_64X" ]] && log_error "Not supported ${ARCH}" exit 1
    INSTALL_DIR="$(xmllint --xpath "string(//config/deploy_path)" "${CONFIG_FILE}")"

    if [ "${OPTION}X" == "installX" ]; then
        install_module "${MODULE}"
    fi

    if [ "${OPTION}X" == "uninstallX" ]; then
        uninstall_module "${MODULE}"
    fi
}

main $@
