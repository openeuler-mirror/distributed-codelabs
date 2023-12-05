/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HKS_HISYSEVENT_WRAPPER_H
#define HKS_HISYSEVENT_WRAPPER_H

#include "hks_type_inner.h"

#ifdef __cplusplus
extern "C" {
#endif

struct EventValues {
    uint32_t userId;
    uint32_t processName;
    uint32_t keyType;
    int32_t errorCode;
};

enum EventType {
    FAULT,
    STATISTIC,
    SECURITY,
    BEHAVIOR,
};

int WriteEvent(enum EventType eventType, const char *functionName, const struct EventValues *eventValues,
    const char *extra);

#ifdef __cplusplus
}
#endif

#endif