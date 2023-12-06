#!/usr/bin/env python
# coding: utf-8
# Copyright (c) 2021 Huawei Device Co., Ltd.
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

import json
import os
import sys
import subprocess
import shutil
import tempfile


def run_cmd(cmd):
    res = subprocess.Popen(cmd.split(), stdout=subprocess.PIPE,
                           stderr=subprocess.PIPE)
    sout, serr = res.communicate()

    return res.pid, res.returncode, sout, serr


def build_rootdir(src_dir):
    tmp_dir = tempfile.mkdtemp(prefix="tmp")
    index = src_dir.rfind('/')
    root_dir = "%sroot" % src_dir[:index + 1]

    if root_dir:
        shutil.rmtree(tmp_dir)
        shutil.copytree(root_dir, tmp_dir, symlinks=True)
    tmp_dir_system = os.path.join(tmp_dir, "system")
    shutil.rmtree(tmp_dir_system, ignore_errors=True)
    shutil.copytree(src_dir, tmp_dir_system, symlinks=True)
    return tmp_dir


def load_config(config_file):
    mk_configs = []
    with open(config_file, "r") as file:
        for line in file:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            mk_configs.append(line)
    mk_configs = " ".join(mk_configs)
    if "ext4" in mk_configs:
        fs_type = "ext4"
        mkfs_tools = "mkextimage.py"
    elif "f2fs" in mk_configs:
        mkfs_tools = "mkf2fsimage.py"
        fs_type = "f2fs"
    elif "cpio" in mk_configs:
        mkfs_tools = "mkcpioimage.py"
        fs_type = "cpio"
    else:
        print("not support filesystem type!!")
        sys.exit(1)
    return mkfs_tools, mk_configs, fs_type

def verify_ret(res):
    if res[1]:
        print(" ".join(["pid ", str(res[0]), " ret ", str(res[1]), "\n",
                        res[2].decode(), res[3].decode()]))
        print("MkImages failed errno: %s" % str(res[1]))
        sys.exit(2)

def sparse_img2simg(is_sparse, device):
    # we don't support sparse in mktools.
    if "sparse" in is_sparse:
        tmp_device = "%s.tmp" % device
        run_cmd(" ".join(["img2simg ", device, " ", tmp_device]))
        os.rename(tmp_device, device)

def mk_system_img(mkfs_tools, mk_configs, device, src_dir, is_sparse):
    src_dir = build_rootdir(src_dir)
    mk_configs = " ".join([src_dir, device, mk_configs])
    res = run_cmd(" ".join([mkfs_tools, mk_configs]))
    verify_ret(res)
    sparse_img2simg(is_sparse, device)

def mk_ramdisk_img(mkfs_tools, mk_configs, device, src_dir, is_sparse):
    # get ramdisk sieze frome ramdisk_image_conf.txt
    ramdisk_size = mk_configs.split(" ")[1]
    mk_configs = \
            " ".join([src_dir, device, ramdisk_size])
    res = run_cmd(" ".join([mkfs_tools, mk_configs]))
    verify_ret(res)

def mk_other_img(mkfs_tools, mk_configs, device, src_dir, is_sparse):
    mk_configs = " ".join([src_dir, device, mk_configs])
    res = run_cmd(" ".join([mkfs_tools, mk_configs]))
    verify_ret(res)
    sparse_img2simg(is_sparse, device)

def mk_images(args):
    if len(args) != 4:
        print("mk_images need 4 args!!!")
        sys.exit(1)
    config = {}
    with open("../../ohos_config.json") as f:
        config = json.load(f)
    src_dir = args[0]
    config_file = args[1]
    device = args[2]
    is_sparse = args[3]
    mkfs_tools, mk_configs, _ = load_config(config_file)
    if "system.img" in device:
        mk_system_img(mkfs_tools, mk_configs, device, src_dir, is_sparse)
    elif "ramdisk.img" == device:
        mk_ramdisk_img(mkfs_tools, mk_configs, device, src_dir, is_sparse)
    elif "updater_ramdisk.img" == device:
        if config.get('component_type', '') == 'system_component':
            return
        mk_ramdisk_img(mkfs_tools, mk_configs, device, src_dir, is_sparse)
    else:
        mk_other_img(mkfs_tools, mk_configs, device, src_dir, is_sparse)


if __name__ == '__main__':
    mk_images(sys.argv[1:])
