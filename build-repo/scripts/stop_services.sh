#!/bin/bash

# 停止所有启动的分布式服务

# 定义map，保存参数和对应执行的shell命令
declare -A COMMAND_MAP=(
  # ["dfs"]="pkill -f distributedfile"
  ["dslm"]="pkill -f dslm_service"
  ["device_manager"]="pkill -f device_manager"
  ["softbus"]="pkill -f softbus_server"
  ["deviceauth"]="pkill -f deviceauth_service"
  ["huks"]="pkill -f huks_service"
  ["samgr"]="pkill -f samgr"
  ["datamgr"]="pkill -f distributeddata"\
  ["kv_demo"]="pkill -f kv_manager_demo"
)

# 日志打印函数
log() {
  case $1 in
    error)
      echo "[ERROR] $2"
      ;;
    info)
      echo "[INFO] $2"
      ;;
    debug)
      if [ "$DEBUG_MODE" == "true" ]; then
        echo "[DEBUG] $2"
      fi
      ;;
    *)
      echo "Invalid log level: $1"
      ;;
  esac
}

# 判断参数是否为空
if [ -z "$1" ]; then
  log error "Usage: $0 [all|samgr|huks|deviceauth|softbus|device_manager|dfs]"
  exit 1
fi

# 验证参数并执行相应命令
if [ "$1" == "all" ]; then
  log info "Stop all services..."
  for key in "${!COMMAND_MAP[@]}"; do
    log info "Stop $key service..."
    # 执行命令
    eval ${COMMAND_MAP[$key]}
  done
else
  if [ "${COMMAND_MAP[$1]+isset}" ]; then
    log info "Stop $1 service..."
    # 执行命令
    eval ${COMMAND_MAP[$1]}
  else
    log error "Invalid parameter: $1"
    exit 1
  fi
fi

log info "Done."
