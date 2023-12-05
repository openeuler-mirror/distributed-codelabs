/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "hks_hitrace.h"
#ifdef L2_STANDARD
#include "hitrace_meter_wrapper.h"

static const uint64_t huksLabel = (1ULL << 25);
#endif

struct HksHitraceId HksHitraceBegin(const char *name, int flag)
{
#ifdef L2_STANDARD
    HiTraceIdStruct traceId = HiTraceChainBegin(name, flag);
    struct HksHitraceId hitraceId = {
        .traceId = traceId,
    };
    HksTraceMeterStart(huksLabel, name, -1);
    return hitraceId;
#else
    (void)name;
    (void)flag;
    struct HksHitraceId hitraceId = {0};
    return hitraceId;
#endif
}

void HksHitraceEnd(struct HksHitraceId *hitraceId)
{
#ifdef L2_STANDARD
    HiTraceChainEnd(&hitraceId->traceId);
    HksTraceMeterFinish(huksLabel);
#else
    (void)hitraceId;
#endif
}