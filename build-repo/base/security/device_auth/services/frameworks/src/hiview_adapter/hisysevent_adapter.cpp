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

#include "hisysevent_adapter.h"
// #include "hisysevent.h"

#ifdef __cplusplus
extern "C" {
#endif

// constexpr char STR_EVENT_CORE_FUNCTION[] = "CORE_FUNCTION";
// constexpr char STR_EVENT[] = "EVENT";
// constexpr char STR_APP_ID[] = "APP_ID";
// constexpr char STR_BATCH_NUMBER[] = "BATCH_NUMBER";
// constexpr char STR_RESULT[] = "RESULT";
// constexpr char STR_OS_ACCOUNT_ID[] = "OS_ACCOUNT_ID";

void ReportCoreFuncInvokeEvent(const InvokeEvent *event)
{
    return;
    if (event == nullptr) {
        return;
    }
    // OHOS::HiviewDFX::HiSysEvent::Write(
    //     OHOS::HiviewDFX::HiSysEvent::Domain::DEVICE_AUTH,
    //     STR_EVENT_CORE_FUNCTION,
    //     OHOS::HiviewDFX::HiSysEvent::EventType::STATISTIC,
    //     STR_EVENT, event->eventId,
    //     STR_APP_ID, event->appId,
    //     STR_BATCH_NUMBER, event->batchNumber,
    //     STR_RESULT, event->result,
    //     STR_OS_ACCOUNT_ID, event->osAccountId);
}

#ifdef __cplusplus
}
#endif