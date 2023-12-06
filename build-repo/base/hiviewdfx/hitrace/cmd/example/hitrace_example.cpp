/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <__config>
#include <cstdint>
#include <cstring>
#include <unistd.h>
#include <iosfwd>
#include <iostream>
#include <ostream>
#include <thread>
#include <vector>

#include "hitrace_meter.h"

using namespace std;
namespace {
constexpr int SLEEP_ONE_SECOND = 1;
constexpr int SLEEP_TWO_SECOND = 2;
constexpr int USLEEP_HALF_SECOND = 500000;
constexpr int CYCLE_TIMES = 3;
constexpr int32_t TASK_ID = 111;
const vector<uint64_t> LABEL_TAGS = {
    HITRACE_TAG_OHOS,
    HITRACE_TAG_DLP_CREDENTIAL,
    HITRACE_TAG_ACCESS_CONTROL,
    HITRACE_TAG_NET,
    HITRACE_TAG_NWEB,
    HITRACE_TAG_HUKS,
    HITRACE_TAG_USERIAM,
    HITRACE_TAG_DISTRIBUTED_AUDIO,
    HITRACE_TAG_DLSM,
    HITRACE_TAG_FILEMANAGEMENT,
    HITRACE_TAG_ABILITY_MANAGER,
    HITRACE_TAG_ZCAMERA,
    HITRACE_TAG_ZMEDIA,
    HITRACE_TAG_ZIMAGE,
    HITRACE_TAG_ZAUDIO,
    HITRACE_TAG_DISTRIBUTEDDATA,
    HITRACE_TAG_MDFS,
    HITRACE_TAG_GRAPHIC_AGP,
    HITRACE_TAG_ACE,
    HITRACE_TAG_NOTIFICATION,
    HITRACE_TAG_MISC,
    HITRACE_TAG_MULTIMODALINPUT,
    HITRACE_TAG_SENSORS,
    HITRACE_TAG_MSDP,
    HITRACE_TAG_DSOFTBUS,
    HITRACE_TAG_RPC,
    HITRACE_TAG_ARK,
    HITRACE_TAG_WINDOW_MANAGER,
    HITRACE_TAG_ACCOUNT_MANAGER,
    HITRACE_TAG_DISTRIBUTED_SCREEN,
    HITRACE_TAG_DISTRIBUTED_CAMERA,
    HITRACE_TAG_DISTRIBUTED_HARDWARE_FWK,
    HITRACE_TAG_GLOBAL_RESMGR,
    HITRACE_TAG_DEVICE_MANAGER,
    HITRACE_TAG_SAMGR,
    HITRACE_TAG_POWER,
    HITRACE_TAG_DISTRIBUTED_SCHEDULE,
    HITRACE_TAG_DEVICE_PROFILE,
    HITRACE_TAG_DISTRIBUTED_INPUT,
    HITRACE_TAG_BLUETOOTH,
    HITRACE_TAG_ACCESSIBILITY_MANAGER,
    HITRACE_TAG_APP
};

void FuncA()
{
    cout << "funcA" << endl;
    usleep(USLEEP_HALF_SECOND);
}

void FuncB()
{
    cout << "funcB" << endl;
    usleep(USLEEP_HALF_SECOND);
}

void FuncC(uint64_t label)
{
    cout << "funcC" << endl;
    int num = 0;
    for (int i = 0; i < CYCLE_TIMES; i++) {
        CountTrace(label, "count number", ++num);
        usleep(USLEEP_HALF_SECOND);
    }
}

void ThreadFunc1(uint64_t label)
{
    StartAsyncTrace(label, "testAsync", TASK_ID);
    for (int i = 0; i < CYCLE_TIMES; ++i) {
        cout << "t1" << endl;
        usleep(USLEEP_HALF_SECOND);
    }
}

void ThreadFunc2(uint64_t label)
{
    for (int i = 0; i < CYCLE_TIMES; ++i) {
        cout << "t2" << endl;
        usleep(USLEEP_HALF_SECOND);
    }
    FinishAsyncTrace(label, "testAsync", TASK_ID);
}

void FuncMain(uint64_t label)
{
    thread t1(ThreadFunc1, label);
    t1.join();

    StartTrace(label, "testStart");
    usleep(USLEEP_HALF_SECOND);

    StartTrace(label, "funcAStart", SLEEP_ONE_SECOND); // 打印起始点
    FuncA();
    FinishTrace(label);
    usleep(USLEEP_HALF_SECOND);

    thread t2(ThreadFunc2, label);
    t2.join();

    StartTrace(label, "funcBStart", SLEEP_TWO_SECOND);
    FuncB();
    FinishTrace(label);
    sleep(SLEEP_ONE_SECOND);

    FinishTrace(label);
    FuncC(label);
}
} // namespace

int main()
{
    for (auto tag : LABEL_TAGS) {
        FuncMain(tag);
    }
    return 0;
}
