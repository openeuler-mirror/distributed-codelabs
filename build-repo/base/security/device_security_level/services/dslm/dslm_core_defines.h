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

#ifndef DSLM_CORE_DEFINES_H
#define DSLM_CORE_DEFINES_H

#include <stdbool.h>
#include <stdint.h>

#include "utils_list.h"
#include "utils_mutex.h"
#include "utils_state_machine.h"
#include "utils_timer.h"

#include "device_security_defines.h"
#include "dslm_cred.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VERSION_MAJOR 3U
#define VERSION_MINOR 0U
#define VERSION_PATCH 0U

typedef struct DslmDeviceInfo {
    ListNode linkNode;
    StateMachine machine;
    DeviceIdentify identity;
    uint32_t version;
    uint32_t onlineStatus;
    uint32_t deviceType;
    uint64_t nonce;
    uint64_t nonceTimeStamp;
    uint64_t lastOnlineTime;
    uint64_t lastOfflineTime;
    uint64_t lastRequestTime;
    uint64_t lastResponseTime;
    uint64_t lastVerifyTime;
    uint64_t transNum;
    TimerHandle timeHandle;
    uint32_t queryTimes;
    uint32_t result;
    DslmCredInfo credInfo;
    uint32_t notifyListSize;
    ListHead notifyList;
    uint32_t historyListSize;
    ListHead historyList;
} DslmDeviceInfo;

static inline uint32_t GetCurrentVersion(void)
{
    // shift major 16 bit, shift minor 8 bit
    return (VERSION_MAJOR << 16U) + (VERSION_MINOR << 8U) + VERSION_PATCH;
}

static inline uint8_t VersionToMajor(uint32_t version)
{
    // shift 16 bit to get version first 8 bit data as the main version
    return (version & 0xFF0000U) >> 16U;
}

static inline uint8_t VersionToMinor(uint32_t version)
{
    // get version middle 8 bit data as the minor version
    return (version & 0xFF00U) >> 8U;
}

static inline uint8_t VersionToPatch(uint32_t version)
{
    // get version last 8 bit data as the patch version
    return version & 0xFF;
}

#ifdef __cplusplus
}
#endif

#endif // DSLM_CORE_DEFINES_H
