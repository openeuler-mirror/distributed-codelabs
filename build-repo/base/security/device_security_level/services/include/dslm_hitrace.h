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

#ifndef SERVICE_DSLM_HITRACE_H
#define SERVICE_DSLM_HITRACE_H

#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

void DslmStartProcessTrace(const char *value);
void DslmStartStateMachineTrace(uint32_t machineId, uint32_t event);
void DslmFinishProcessTrace(void);

void DslmStartProcessTraceAsync(const char *value, uint32_t owner, uint32_t cookie);
void DslmFinishProcessTraceAsync(const char *value, uint32_t owner, uint32_t cookie);

void DslmCountTrace(const char *name, int64_t count);

#ifdef __cplusplus
}
#endif

#endif // SERVICE_DSLM_HITRACE_H
