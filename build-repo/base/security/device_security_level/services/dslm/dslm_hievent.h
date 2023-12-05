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

#ifndef DSLM_BIGDATA_EVENT_H
#define DSLM_BIGDATA_EVENT_H

#include "dslm_core_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

void ReportHiEventServiceStartFailed(const uint32_t errorType);
void ReportHiEventInitSelfFailed(const char *errorString);
void ReportHiEventInfoSync(const DslmDeviceInfo *info);
void ReportHiEventAppInvoke(const DslmDeviceInfo *info);

#ifdef __cplusplus
}
#endif

#endif // DSLM_BIGDATA_EVENT_H
