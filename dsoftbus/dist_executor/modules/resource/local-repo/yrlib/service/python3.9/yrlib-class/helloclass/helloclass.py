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

import time
import yr

yr.init()


@yr.instance
class Instance:
    sum = 0

    def add(self, a):
        self.sum += a

    def get(self):
        return self.sum


ins = [Instance.invoke() for i in range(1)]
yr.get([i.add.invoke(1) for i in ins])
res = [i.get.invoke() for i in ins]
yr.get(res)
[i.terminate() for i in ins]
yr.finalize()
