#!/bin/bash
# Copyright (c) 2022 Huawei Technologies Co., Ltd.
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

# ANSI escape codes for colorful echo.
# Usage: echo -e "${RED}LoL${NC} !" will print 'LoL' in red.
readonly BLACK='\033[0;30m'
readonly DARK_GRAY='\033[1;30m'
readonly RED='\033[0;31m'
readonly LIGHT_RED='\033[1;31m'
readonly GREEN='\033[0;32m'
readonly LIGHT_GREEN='\033[1;32m'
readonly BROWN='\033[0;33m'
readonly YELLOW='\033[1;33m'
readonly BLUE='\033[0;34m'
readonly LIGHT_BLUE='\033[1;34m'
readonly PURPLE='\033[0;35m'
readonly LIGHT_PURPLE='\033[1;35m'
readonly CYAN='\033[0;36m'
readonly LIGHT_CYAN='\033[1;36m'
readonly LIGHT_GRAY='\033[0;37m'
readonly WHITE='\033[1;37m'
readonly NC='\033[0m' # No Color

readonly SIG_TERMINATE="15"

LOCAL_IP_LIST=($(ifconfig -a|grep inet|grep -v inet6|awk '{print $2}'|tr -d "addr:"|tr -d "地址"|tr "\n" "\n"))

function print_ok_msg()
{
  local action="$1"
  local component="$2"
  local ip="$3"
  echo -e "---- ${GREEN}[  OK  ]${NC} The ${CYAN}${component}${NC} service @ ${PURPLE}${ip}${NC} ${action} success."
}

function print_fail_msg()
{
  local action="$1"
  local component="$2"
  local ip="$3"
  local log_dir="$4"
  echo -e "---- ${RED}[  FAILED  ]${NC} The ${CYAN}${component}${NC} service @ ${PURPLE}${ip}${NC} ${action} failed."
}

function is_array()
{
  local var_name="$1"
  local val
  val=$(eval "echo -e \$${var_name}")
  if [[ -z "${val}" ]]; then
    return 1
  fi
  if [[ "$(declare -p "${var_name}")" =~ "typeset -a" ]] || [[ "$(declare -p "${var_name}")" =~ "declare -a" ]]; then
    return 0
  else
    return 1
  fi
}

function is_local_ip()
{
  local ip="$1"
  for local_ip in "${LOCAL_IP_LIST[@]}";
  do
    if [[ "${ip}" = "${local_ip}" ]]; then
      return 0
    fi
  done
  return 1
}

function is_num()
{
  local re='^[0-9]+$'
  [[ "$1" =~ $re ]] && return 0 || return 1
}