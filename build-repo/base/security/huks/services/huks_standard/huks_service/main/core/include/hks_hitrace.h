/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef HKS_HITRACE_DEFINE_H
#define HKS_HITRACE_DEFINE_H

#include <stdint.h>

#ifdef L2_STANDARD
#include "hitrace/trace.h"
#endif

#define HKS_HITRACE_FLAG_DEFAULT 0

#ifdef __cplusplus
extern "C" {
#endif

struct HksHitraceId {
#ifdef L2_STANDARD
    HiTraceIdStruct traceId;
#else
    uint8_t id;
#endif
};

struct HksHitraceId HksHitraceBegin(const char *name, int flag);

void HksHitraceEnd(struct HksHitraceId *hitraceId);

#ifdef __cplusplus
}
#endif

#endif  // HKS_RW_LOCK_H
