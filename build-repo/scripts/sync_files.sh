#!/bin/bash

# 在树莓派上同步蓝云虚拟机上编译生成的二进制/动态库等文件
# 此脚本运行在树莓派上

# 检查 rsync 是否已经安装
if ! command -v rsync &> /dev/null
then
    # 如果未安装，则使用 yum 安装 rsync
    echo "rsync is not installed. Installing it now..."
    sudo yum install -y rsync
fi

# 前提：树莓派上可无需密码连接蓝云机器
REMOTE_HOST="openEuler_sp2"
REMOTE_BUILD_PATH="/root/hp/distributed-build/"
REMOTE_LIBS_PATH=$REMOTE_BUILD_PATH"out/openeuler/linux_clang_arm64/"
REMOTE_BIN_PATH=$REMOTE_BUILD_PATH"out/openeuler/packages/phone/system/bin/"
REMOTE_SCRIPT_PATH=$REMOTE_BUILD_PATH"scripts/"

# 同步库文件
lib_files=$(ssh root@${REMOTE_HOST} "find ${REMOTE_LIBS_PATH} -type f \( -name '*.so' -o -name '*.a' \)")
rsync -avz --checksum -e "ssh" --no-relative --no-implied-dirs --files-from=<(echo "$lib_files") root@${REMOTE_HOST}:/ /system/lib64/

# 同步bin文件
rsync -avz --exclude='start_service' --exclude='stop_service' --checksum --no-relative --no-implied-dirs -e "ssh" root@${REMOTE_HOST}:$REMOTE_BIN_PATH /system/bin/
rsync -avz --checksum --no-relative --no-implied-dirs -e "ssh" root@${REMOTE_HOST}:$REMOTE_LIBS_PATH/distributeddatamgr/kv_store/kv_manager_demo /system/bin/
rsync -avz --checksum --no-relative --no-implied-dirs -e "ssh" root@${REMOTE_HOST}:$REMOTE_LIBS_PATH/distributeddatamgr/relational_store/rdb_distributed_demo /system/bin/
rsync -avz --checksum --no-relative --no-implied-dirs -e "ssh" root@${REMOTE_HOST}:$REMOTE_LIBS_PATH/distributeddatamgr/data_object/obj_manager_demo /system/bin/

# 同步脚本文件
rsync -avz --checksum --no-relative --no-implied-dirs -e "ssh" root@${REMOTE_HOST}:$REMOTE_SCRIPT_PATH /system/bin/