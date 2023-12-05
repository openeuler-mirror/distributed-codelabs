#!/bin/bash

# 此脚本用于拉起分布式各个服务
BIN_PATH="/system/bin"

# 定义map，保存参数和对应执行的shell命令
# 如果需要增加服务启动，直接往此map中增加字段即可
declare -A COMMAND_MAP=(
  ["samgr"]="./samgr > /system/log/samgr.log 2>&1 &"
  ["huks"]="./sa_main /system/profile/huks_service.xml > /system/log/huks_service.log 2>&1 &"
  ["deviceauth"]="./deviceauth_service > /system/log/device_auth.log 2>&1 &"
  ["softbus"]="./sa_main /system/profile/softbus_server.xml > /system/log/softbus_server.log 2>&1 &"
  ["device_manager"]="./sa_main /system/profile/device_manager.xml > /system/log/device_manager.log 2>&1 &"
  ["dfs"]="./sa_main /system/profile/distributedfiledaemon.xml > /system/log/distributedfiledaemon.log 2>&1 &"
  ["datamgr"]="./sa_main /system/profile/distributeddata.xml > /system/log/datamgr.log 2>&1 &"
)

# 不同服务的依赖关系
declare -A CMD_REQUIRES=(
  ["samgr"]=""
  ["huks"]="samgr"
  ["deviceauth"]="samgr huks"
  ["softbus"]="samgr huks deviceauth"
  ["device_manager"]="samgr huks deviceauth softbus"
  ["dfs"]="samgr huks deviceauth softbus device_manager"
  ["datamgr"]="samgr huks deviceauth softbus device_manager"
)

start_order=("samgr" "huks" "deviceauth" "softbus" "device_manager" "dfs" "datamgr")

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

check_process_running() {
  if pgrep -x "$1" > /dev/null; then
    return 0
  else
    return 1
  fi
}

start_process() {
  process_name=$1
  deps="${CMD_REQUIRES[$process_name]}"

  for dep in $deps; do
    if check_process_running "$dep"; then
      log info "Process '$dep' is already running"
      continue
    fi
    if [ "${COMMAND_MAP[$dep]+isset}" ]; then
      log info "Starting process '$dep'..."
      eval ${COMMAND_MAP[$dep]}
      sleep 1
    else
      log error "Invalid dependency: $dep"
      exit 1
    fi
  done

  if check_process_running "$process_name"; then
    log info "Process '$process_name' is already running"
  else
    log info "Starting ddd process '$process_name'..."
    if [ "${COMMAND_MAP[$process_name]+isset}" ]; then
      eval ${COMMAND_MAP[$process_name]}
    else
      log error "Invalid process: $process_name"
      exit 1
    fi
  fi
}

# 如果有进程还在，暂停服务防止双进程存在
if pgrep -x "samgr" > /dev/null; then
  log info "service is running, stop it"
  /system/bin/stop_services.sh all
  sleep 2
fi

# 软链接动态库
if [ ! -e "/usr/lib64/libsec_shared.z.so" ]; then
  log info "libsec_shared.z.so not exist, create it"
  ln -s /usr/lib64/libboundscheck.so /usr/lib64/libsec_shared.z.so
fi

# 新建日志目录
if [ ! -d "/system/log" ]; then
  log info "/system/log directory not exist, create it"
  sudo mkdir -p /system/log
fi

# 以IP地址生成SN号
if [ ! -e "/etc/SN" ]; then
  log info "SN file not exist, add content to it."
  ipv4_address=$(ip -4 addr show scope global | grep inet | awk '{print $2}' | cut -d '/' -f 1)
  echo "$ipv4_address" | sudo tee /etc/SN > /dev/null
fi

# 判断参数是否为空
if [ -z "$1" ]; then
  log error "Usage: $0 [all|samgr|huks|deviceauth|softbus|device_manager|dfs|datamgr]"
  exit 1
fi

# 检查 $LD_LIBRARY_PATH 是否包含 "/system/lib64"，如果不包含则加入
if [[ ":$LD_LIBRARY_PATH:" != *":/system/lib64:"* ]]; then
  log info "/system/lib64 not in LD_LIBRARY_PATH, add it."
  export LD_LIBRARY_PATH="/system/lib64:$LD_LIBRARY_PATH"
fi

# 检查是否关闭selinux，如果未关闭则关闭
selinux_status=$(getenforce)
if [ "$selinux_status" = "Enforcing" ]; then
    echo "SELinux set to Permissive..."
    sudo setenforce 0
fi

# 如果未开启binder，开启binder
if [ ! -e "/dev/binder" ]; then
  log info "Starting insert binder"
  # 如果当前内核已经自带了binder，则直接进行binder的挂载
  if grep -q '^CONFIG_ANDROID_BINDER_IPC=y$' /boot/config-$(uname -r); then
    mkdir /dev/binderfs
    sudo mount -t binder binder /dev/binderfs
    sudo ln -s /dev/binderfs/binder /dev/binder
  else # 内核没有开启binder，插入相关binder的ko
    if [ ! -f "/lib/modules/$(uname -r)/binder/binder_linux.ko" ]; then
        log error "no binder ko find, please install binder."
        exit 1
    fi
    insmod /lib/modules/$(uname -r)/binder/binder_linux.ko
  fi
fi

cd ${BIN_PATH} || exit
# 验证参数并执行相应命令
if [ "$1" == "all" ]; then
  log info "Starting all services..."
  for key in "${start_order[@]}"; do
    log info "Starting $key service..."
    # 执行命令
    eval ${COMMAND_MAP[$key]}
    sleep 1
  done
else
  if [ "${COMMAND_MAP[$1]+isset}" ]; then
    log info "Starting $1 service..."
    # 执行命令
    start_process "$1"
  else
    log error "Invalid parameter: $1"
    exit 1
  fi
fi

log info "Done."
