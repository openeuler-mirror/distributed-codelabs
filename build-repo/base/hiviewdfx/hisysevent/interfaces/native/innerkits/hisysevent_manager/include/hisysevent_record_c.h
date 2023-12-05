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
#ifndef HISYSEVENT_RECORD_C_H
#define HISYSEVENT_RECORD_C_H

#include "hisysevent_c.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_LENGTH_OF_EVENT_DOMAIN 17
#define MAX_LENGTH_OF_EVENT_NAME 33
#define MAX_LENGTH_OF_TIME_ZONE 6

struct HiSysEventRecord {
    char domain[MAX_LENGTH_OF_EVENT_DOMAIN];
    char eventName[MAX_LENGTH_OF_EVENT_NAME];
    HiSysEventEventType type;
    uint64_t time;
    char tz[MAX_LENGTH_OF_TIME_ZONE];
    int64_t pid;
    int64_t tid;
    int64_t uid;
    uint64_t traceId;
    uint64_t spandId;
    uint64_t pspanId;
    int traceFlag;
    char* level;
    char* tag;
    char* jsonStr;
};
typedef struct HiSysEventRecord HiSysEventRecord;

void OH_HiSysEvent_GetParamNames(
    const HiSysEventRecord& record, char*** params, size_t& len);
int OH_HiSysEvent_GetParamInt64Value(
    const HiSysEventRecord& record, const char* name, int64_t& value);
int OH_HiSysEvent_GetParamUint64Value(
    const HiSysEventRecord& record, const char* name, uint64_t& value);
int OH_HiSysEvent_GetParamDoubleValue(
    const HiSysEventRecord& record, const char* name, double& value);
int OH_HiSysEvent_GetParamStringValue(
    const HiSysEventRecord& record, const char* name, char** value);
int OH_HiSysEvent_GetParamInt64Values(
    const HiSysEventRecord& record, const char* name, int64_t** value, size_t& len);
int OH_HiSysEvent_GetParamUint64Values(
    const HiSysEventRecord& record, const char* name, uint64_t** value, size_t& len);
int OH_HiSysEvent_GetParamDoubleValues(
    const HiSysEventRecord& record, const char* name, double** value, size_t& len);
int OH_HiSysEvent_GetParamStringValues(
    const HiSysEventRecord& record, const char* name, char*** value, size_t& len);
#ifdef __cplusplus
}
#endif
#endif // HISYSEVENT_RECORD_C_H
