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

set -ex

DST_RESOURCE_PATH="/home/sn/temp-resource"
SRC_RESOURCE_PATH="/home/sn/resource"

function recursive_copy_file()
{
  cd "$1"
  for name in *
  do
    if [ -f "$1"/"$name" ]; then
      if [ ! -f "$2"/"$name" ]; then
        cp "$1"/"$name" "$2"/"$name"
      fi
    elif [ -d "$1"/"$name" ]; then
      if [ ! -d "$2"/"$name" ]; then
        mkdir -m 700 "$2"/"$name"
      fi
      recursive_copy_file "$1"/"$name" "$2"/"$name"
    fi
  done
}

recursive_copy_file "$SRC_RESOURCE_PATH" "$DST_RESOURCE_PATH"