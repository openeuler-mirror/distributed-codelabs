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

#include "dev_auth_hievent.h"
#include "hidump_adapter.h"
#include "hisysevent_adapter.h"
#include "hitrace_adapter.h"

#ifdef __cplusplus
extern "C" {
#endif

// dev_auth_hievent
void ReportHiEventCoreFuncInvoke(int32_t eventId, int32_t osAccountId, CJson *params, int32_t result)
{
    (void)eventId;
    (void)osAccountId;
    (void)params;
    (void)result;
}

// hidump_adapter
void DevAuthDump(int fd)
{
    (void)fd;
}

void RegisterDumpFunc(DumpCallBack func)
{
    (void)func;
}

// hisysevent_adapter
void ReportCoreFuncInvokeEvent(const InvokeEvent *event)
{
    (void)event;
}

// hitrace_adapter
void DevAuthStartTrace(const char *value)
{
    (void)value;
}

void DevAuthFinishTrace(void)
{
}

#ifdef __cplusplus
}
#endif
