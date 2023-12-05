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

#ifndef SERVICE_DSLM_BIGDATA_H
#define SERVICE_DSLM_BIGDATA_H

#ifdef __cplusplus
#include <cstdint>
constexpr int MODEL_MAX_LEN = 128;
constexpr int PKG_NAME_MAX_LEN = 256;
#else
#include <stdint.h>
#define MODEL_MAX_LEN 128
#define PKG_NAME_MAX_LEN 256
#endif

typedef struct AppInvokeEvent {
    int32_t uid;
    int32_t costTime;
    int32_t retCode;
    int32_t secLevel;
    int32_t retMode;
    char localModel[MODEL_MAX_LEN];
    char targetModel[MODEL_MAX_LEN];
    char pkgName[PKG_NAME_MAX_LEN];
} AppInvokeEvent;

typedef struct SecurityInfoSyncEvent {
    char localModel[MODEL_MAX_LEN];
    char targetModel[MODEL_MAX_LEN];
    uint64_t localVersion;
    uint64_t targetVersion;
    int32_t credType;
    int32_t retCode;
    int32_t costTime;
    int32_t secLevel;
} SecurityInfoSyncEvent;

#ifdef __cplusplus
extern "C" {
#endif

void ReportServiceStartFailedEvent(const uint32_t errorType);

void ReportInitSelfFailedEvent(const char *errorString);

void ReportAppInvokeEvent(const AppInvokeEvent *event);

void ReportSecurityInfoSyncEvent(const SecurityInfoSyncEvent *event);

#ifdef __cplusplus
}
#endif

#endif // SERVICE_DSLM_BIGDATA_H
