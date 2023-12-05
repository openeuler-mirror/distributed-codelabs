#!/bin/bash

# 此脚本用在实际运行环境：树莓派
# 卸载dfs_service需要使用的hmdfs共享目录

DST_BASE_PATH="/mnt/hmdfs/100"
SRC_BASE_PATH="/data/service/el2/100"

sudo umount ${DST_BASE_PATH}"/non_account"
sudo umount ${DST_BASE_PATH}"/account"

rm -rf ${DST_BASE_PATH}
rm -rf ${SRC_BASE_PATH}