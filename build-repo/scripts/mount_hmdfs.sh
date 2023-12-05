#!/bin/bash

# 此脚本用在实际运行环境：树莓派
# 挂载dfs_service需要使用的hmdfs共享目录

DST_BASE_PATH="/mnt/hmdfs/100"
SRC_BASE_PATH="/data/service/el2/100"

mkdir -p ${DST_BASE_PATH}"/non_account"
mkdir -p ${DST_BASE_PATH}"/account"
mkdir -p ${SRC_BASE_PATH}"/non_account"
mkdir -p ${SRC_BASE_PATH}"/account"

sudo mount -t hmdfs -o merge,local_dst=${DST_BASE_PATH}"/non_account" ${SRC_BASE_PATH}"/non_account" ${DST_BASE_PATH}"/non_account"
sudo mount -t hmdfs -o merge,local_dst=${DST_BASE_PATH}"/account" ${SRC_BASE_PATH}"/account" ${DST_BASE_PATH}"/account"