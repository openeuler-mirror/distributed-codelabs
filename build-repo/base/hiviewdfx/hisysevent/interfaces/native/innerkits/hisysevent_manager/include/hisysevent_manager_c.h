/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#ifndef HISYSEVENT_INTERFACES_NATIVE_INNERKITS_HISYSEVENT_MANAGER_INCLUDE_HISYSEVENT_MANAGER_C_H
#define HISYSEVENT_INTERFACES_NATIVE_INNERKITS_HISYSEVENT_MANAGER_INCLUDE_HISYSEVENT_MANAGER_C_H

#include "hisysevent_record_c.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_NUMBER_OF_EVENT_LIST 10

/**
 * @brief Define the argument of the query.
 */
struct HiSysEventQueryArg {
    int64_t beginTime;
    int64_t endTime;
    int32_t maxEvents;
};
typedef struct HiSysEventQueryArg HiSysEventQueryArg;

/**
 * @brief Define the rule of the query.
 */
struct HiSysEventQueryRule {
    char domain[MAX_LENGTH_OF_EVENT_DOMAIN];
    char eventList[MAX_NUMBER_OF_EVENT_LIST][MAX_LENGTH_OF_EVENT_NAME];
    size_t eventListSize;
    char* condition;
};
typedef struct HiSysEventQueryRule HiSysEventQueryRule;

/**
 * @brief Define the callback of the query.
 */
struct HiSysEventQueryCallback {
    void (*OnQuery)(HiSysEventRecord records[], size_t size);
    void (*OnComplete)(int32_t reason, int32_t total);
};
typedef struct HiSysEventQueryCallback HiSysEventQueryCallback;

/**
 * @brief Query event.
 * @param arg      arg of query.
 * @param rules    rules of query.
 * @param ruleSize rules size of query.
 * @param callback callback of query.
 * @return 0 means success, others means failure.
 */
int OH_HiSysEvent_Query(const HiSysEventQueryArg& arg, HiSysEventQueryRule rules[], size_t ruleSize,
    HiSysEventQueryCallback& callback);

#ifdef __cplusplus
}
#endif
#endif // HISYSEVENT_INTERFACES_NATIVE_INNERKITS_HISYSEVENT_MANAGER_INCLUDE_HISYSEVENT_MANAGER_C_H
