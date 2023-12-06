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

#include "dslm_bigdata.h"
#include "dslm_hidumper.h"
#include "dslm_hitrace.h"

#include <stdio.h>

#include "inttypes.h"

#include "utils_log.h"

#ifdef __cplusplus
extern "C" {
#endif

#define END_LINE "\n"

// bigdata
__attribute__((weak)) void ReportServiceStartFailedEvent(const uint32_t errorType)
{
}

__attribute__((weak)) void ReportInitSelfFailedEvent(const char *errorString)
{
}

__attribute__((weak)) void ReportAppInvokeEvent(const AppInvokeEvent *event)
{
}

__attribute__((weak)) void ReportSecurityInfoSyncEvent(const SecurityInfoSyncEvent *event)
{
}

// dslm_hidumper
__attribute__((weak)) void DslmDumper(int fd)
{
}

// hitrace
__attribute__((weak)) void DslmStartProcessTrace(const char *value)
{
}

__attribute__((weak)) void DslmStartStateMachineTrace(uint32_t machineId, uint32_t event)
{
}

__attribute__((weak)) void DslmFinishProcessTrace(void)
{
}

__attribute__((weak)) void DslmStartProcessTraceAsync(const char *value, uint32_t owner, uint32_t cookie)
{
}

__attribute__((weak)) void DslmFinishProcessTraceAsync(const char *value, uint32_t owner, uint32_t cookie)
{
}

__attribute__((weak)) void DslmCountTrace(const char *name, int64_t count)
{
}

#ifdef __cplusplus
}
#endif
