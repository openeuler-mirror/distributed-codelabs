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
set -o pipefail

readonly USAGE="Usage: worker-launcher.sh command..."
readonly BASE_DIR=$(dirname "$(readlink -f "$0")")
readonly CONF_DIR=${BASE_DIR}/conf

. "${BASE_DIR}/deploy-common.sh"

IP_LIST=()

function set_default_address()
{
  case "worker" in
    master)
      export MASTER_ADDRESS="127.0.0.1:9089"
      ;;
    worker)
      export WORKER_ADDRESS="127.0.0.1:9088"
      export WORKER_MASTER_ADDRESS="127.0.0.1:9089"
      ;;
    agent)
      export AGENT_ADDRESS="127.0.0.1:9087"
      export AGENT_WORKER_ADDRESS="127.0.0.1:9088"
      ;;
    gcs)
      export GCS_ADDRESS="127.0.0.1:9090"
      export GCS_MASTER_ADDRESS="127.0.0.1:9089"
      export GCS_NODE_ID="NODE-ID"
      ;;
    *)
      echo -e "Error: worker not found, No such component for deployment!"
      ;;
  esac
}

function valid_ip()
{
  local ip=$1
  local ret=1

  if [[ "$ip" =~ ^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}:[0-9]{1,5}$ ]]; then
      ret=0
  fi
  return $ret
}

function parse_ip_list()
{
  . "${CONF_DIR}/worker-env.sh"
  if is_array "WORKER_ADDRESS"; then
    for((i=0;i<${#WORKER_ADDRESS[@]};i++))
    do
      if ! valid_ip "${WORKER_ADDRESS[i]}"; then
        echo -e "[$(date '+%F %T')] Invalid worker ip at $i: \"${WORKER_ADDRESS[i]}\"" >>"${deploy_log}"
        return 1
      fi
      IP_LIST+=("$(echo "${WORKER_ADDRESS[i]}" | cut -d : -f 1)")
    done
  else
    if [[ -z "${WORKER_ADDRESS}" ]]; then
      set_default_address
    fi
    if ! valid_ip "${WORKER_ADDRESS}"; then
      echo -e "[$(date '+%F %T')] Invalid worker ip: \"${WORKER_ADDRESS}\"" >>"${deploy_log}"
      return 1
    fi
    IP_LIST+=("$(echo "${WORKER_ADDRESS}" | cut -d : -f 1)")
  fi
}

function main()
{
  local launch_prefix="bash"
  # Add -x if debugging is enable.
  if [[ "$-" == *x* ]]; then
    launch_prefix="${launch_prefix} -x"
  fi

  if [[ -z "$@" ]]; then
    echo -e "${USAGE}" >&2
    exit 1
  fi

  local cmd_list=("$@")

  deploy_log="${BASE_DIR}/deploy.log"

  if ! parse_ip_list ; then
    echo -e "---- ${RED}[  FAILED  ]${NC} The ${CYAN}worker${NC} service parse ip list failed."
    echo -e "---- Task(s) on worker nodes failed! look at ${deploy_log} for details."
    exit 1
  fi

  # We split a large number of deployment requests into multiple batches to avoid very perverted deployment requests.
  local batch_num=5
  local ip_num="${#IP_LIST[@]}"
  local rounds="$[(ip_num+batch_num-1)/batch_num]"

  local pids=()
  local err_flag=0

  for((k=0;k<"${rounds}";k++))
  do
    local start="$[k*batch_num]"
    local end="$[(k+1)*batch_num]"
    if [[ "${end}" -gt "${ip_num}" ]]; then
      end="${ip_num}"
    fi
    for((i="${start}";i<"${end}";i++))
    do
      local ip="${IP_LIST[$i]}"
      local cmd=("${cmd_list[@]}")
      cmd+=("-n" "$i")
      echo -e "[$(date '+%F %T')] Execute the command: ${cmd[@]} on worker node [${ip}]." >>"${deploy_log}"
      if is_local_ip "${ip}"; then
        echo -e "[$(date '+%F %T')][${ip}] Running in local machine..." >>"${deploy_log}"
        nohup $"${cmd[@]// /\\ }" 2>&1 | \
          while read -r line; do echo "[$(date '+%F %T')][${ip}] ${line}"; done >> "${deploy_log}" &
      else
        echo -e "[$(date '+%F %T')][${ip}] Connecting as ${USER}..." >>"${deploy_log}"
        nohup ssh -o NumberOfPasswordPrompts=0 -o ConnectTimeout=5 -o StrictHostKeyChecking=no -tt "${ip}" \
          "${launch_prefix}" $"${cmd[@]// /\\ }" 2>&1 | while read -r line; do echo "[$(date '+%F %T')][${ip}] ${line}"; done >> "${deploy_log}" &
      fi
      pids[${#pids[@]}]=$!
    done

    echo -e "[$(date '+%F %T')] Waiting for all tasks finish..." >>"${deploy_log}"
    
    for ((i=0; i<${#pids[@]}; i++));
    do
      wait ${pids[$i]}
      ret_code=$?
      if [[ ${ret_code} -ne 0 ]]; then
        err_flag=1
        echo -e "---- ${RED}[  FAILED  ]${NC} The ${CYAN}worker${NC} service @ ${PURPLE}${IP_LIST[$i]}${NC} failed."
        echo -e "[$(date '+%F %T')][${IP_LIST[$i]}] Task on worker node '${IP_LIST[$i]}' failed, exit code: ${ret_code}" >>"${deploy_log}"
      else
        echo -e "---- ${GREEN}[  OK  ]${NC} The ${CYAN}worker${NC} service @ ${PURPLE}${IP_LIST[$i]}${NC} success."
        echo -e "[$(date '+%F %T')][${IP_LIST[$i]}] Task on worker node '${IP_LIST[$i]}' success!" >>"${deploy_log}"
      fi
    done
  done

  if [[ ${err_flag} -eq 0 ]]; then
    echo -e "[$(date '+%F %T')] All tasks run on worker nodes success!" >>"${deploy_log}"
  else
    echo -e "---- Task(s) on worker nodes failed! look at ${deploy_log} for details."
  fi
}

main "$@"
