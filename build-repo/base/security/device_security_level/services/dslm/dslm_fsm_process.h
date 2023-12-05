/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef DSLM_FSM_PROCESS_H
#define DSLM_FSM_PROCESS_H

#include <stdint.h>

#include "dslm_core_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

// item state
enum {
    STATE_INIT = 0,
    STATE_WAITING_CRED_RSP = 1,
    STATE_SUCCESS = 2,
    STATE_FAILED = 3,
};

// item event
enum {
    EVENT_DEVICE_ONLINE = 0,
    EVENT_CRED_RSP = 1,
    EVENT_MSG_SEND_FAILED = 2,
    EVENT_DEVICE_OFFLINE = 3,
    EVENT_TIME_OUT = 4,
    EVENT_SDK_GET = 5,
    EVENT_SDK_TIMEOUT = 6,
    EVENT_CHECK = 7,
    EVENT_TO_SYNC = 8,
};

void InitDslmStateMachine(DslmDeviceInfo *info);
void ScheduleDslmStateMachine(DslmDeviceInfo *info, uint32_t event, const void *para);
uint32_t GetCurrentMachineState(const DslmDeviceInfo *info);
void LockDslmStateMachine(DslmDeviceInfo *info);
void UnLockDslmStateMachine(DslmDeviceInfo *info);

#ifdef __cplusplus
}
#endif
#endif // DSLM_FSM_PROCESS_H
