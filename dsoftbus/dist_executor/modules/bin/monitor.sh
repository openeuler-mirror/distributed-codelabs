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


BASE_DIR=$(
    cd "$(dirname "$0")"
    pwd
)

. ${BASE_DIR}/utils.sh

function check_admin_service() {
    local pid=$(ps -ef | grep "admin/bin/distribute-executor" | grep -v grep | grep -v PPID | grep -v clear.sh | grep -v deploy.sh | awk '{ print $2 }')
    if [ -z "$pid" ]; then
        log_warning "admin-service not exist"
        return 1
    fi
    return 0
}

function check_function_repo() {
    local pid=$(ps -ef | grep "function-repo/bin/distribute-executor" | grep -v grep | grep -v PPID | grep -v clear.sh | grep -v deploy.sh | awk '{ print $2 }')
    if [ -z "$pid" ]; then
        log_warning "function-repo not exist"
        return 1
    fi
    return 0
}

function check_worker_manager() {
    local pid=$(ps -ef | grep "worker-manager/bin/distribute-executor" | grep -v grep | grep -v PPID | grep -v clear.sh | grep -v deploy.sh | awk '{ print $2 }')
    if [ -z "$pid" ]; then
        log_warning "worker-manager not exist"
        return 1
    fi
    return 0
}

function check_frontend() {
    local pid=$(ps -ef | grep "frontend/bin/distribute-executor" | grep -v grep | grep -v PPID | grep -v clear.sh | grep -v deploy.sh | awk '{ print $2 }')
    if [ -z "$pid" ]; then
        log_warning "frontend not exist"
        return 1
    fi
    return 0
}

function check_dsmaster() {
    local pid=$(ps -ef | grep "datasystem/service/master" | grep -v grep | grep -v PPID | grep -v clear.sh | grep -v deploy.sh | awk '{ print $2 }')
    if [ -z "$pid" ]; then
        log_warning "datasystem master not exist"
        return 1
    fi
    return 0
}

function check_ds_agent() {
    local pid=$(ps -ef | grep "datasystem/service/agent" | grep -v grep | grep -v PPID | grep -v clear.sh | grep -v deploy.sh | awk '{ print $2 }')
    if [ -z "$pid" ]; then
        log_warning "datasystem agent not exist"
        return 1
    fi
    return 0
}

function check_bus_proxy() {
    local pid=$(ps -ef | grep "functiontask/bin/distribute-executor" | grep -v grep | grep -v PPID | grep -v clear.sh | grep -v deploy.sh | awk '{ print $2 }')
    if [ -z "$pid" ]; then
        log_warning "busproxy not exist"
        return 1
    fi
    return 0
}

function check_runtime_mgr() {
    local pid=$(ps -ef | grep "runtime-manager/bin/distribute-executor" | grep -v grep | grep -v PPID | grep -v clear.sh | grep -v deploy.sh | awk '{ print $2 }')
    if [ -z "$pid" ]; then
        log_warning "runtime-manager not exist"
        return 1
    fi
    return 0
}

function check_worker() {
    local pid=$(ps -ef | grep "worker/bin/distribute-executor" | grep -v grep | grep -v PPID | grep -v clear.sh | grep -v deploy.sh | awk '{ print $2 }')
    if [ -z "$pid" ]; then
        log_warning "worker not exist"
        return 1
    fi
    return 0
}

function check_ds_worker() {
    local pid=$(ps -ef | grep "datasystem/service/worker" | grep -v grep | grep -v PPID | grep -v clear.sh | grep -v deploy.sh | awk '{ print $2 }')
    if [ -z "$pid" ]; then
        log_warning "datasystem worker not exist"
        return 1
    fi
    return 0
}

function main() {
    check_admin_service
    check_function_repo
    check_worker_manager
    check_frontend
    check_dsmaster
    check_ds_agent
    check_bus_proxy
    check_runtime_mgr
    check_worker
    check_ds_worker


    log_info "check mp success"
}

main
