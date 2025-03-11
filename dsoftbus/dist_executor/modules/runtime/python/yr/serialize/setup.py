#!/usr/bin/env python3
# coding=UTF-8
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

import os
from pathlib import Path

from setuptools import Extension, setup
from Cython.Build import cythonize

setup_abs_path = os.getcwd()

protoc = os.popen("which protoc").read()
protoc_bin = protoc.split("\n")[0]
protoc_bin_path = protoc_bin.split("bin")[0]
PROTOBUF_INCLUDE_PATH = str(Path(protoc_bin_path, "include"))
PROTOBUF_LIBRARY_PATH = str(Path(protoc_bin_path, "lib"))
PROTOBUF_SO_PATH = str(Path(PROTOBUF_LIBRARY_PATH, "libprotobuf.so.24*"))
cp_cmd = "cp  " + PROTOBUF_SO_PATH + " " + setup_abs_path
os.popen(cp_cmd)

extensions = [
    Extension("serialization", ["serialization.pyx", "serialization.pb.cc"],
              include_dirs=[setup_abs_path, PROTOBUF_INCLUDE_PATH],
              library_dirs=[setup_abs_path, PROTOBUF_LIBRARY_PATH],
              libraries=["protobuf"],
              runtime_library_dirs=['$ORIGIN'],
              language="c++"),
]

setup(
    name="serialization",
    ext_modules=cythonize(extensions)
)
