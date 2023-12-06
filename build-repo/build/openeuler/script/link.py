#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Copyright (c) 2021 Hoperun Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import sys
import argparse

# create parent directory
def create_dir(dir):
    dir_items = dir.split("/")
    full_name = ""
    length = len(dir_items) - 1
    for i in range(length):
        full_name += "/" + dir_items[i]
        if not os.path.exists(full_name):
            os.mkdir(full_name)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--src_target', required=True)
    parser.add_argument('--dest_target', required=True)
    args = parser.parse_args()

    src_target = args.src_target
    dest_target = args.dest_target

    if os.path.exists(dest_target):
        os.unlink(dest_target)
    create_dir(dest_target)
    return os.symlink(src_target, dest_target)

if __name__ == '__main__':
    sys.exit(main())

