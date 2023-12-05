/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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


#include "hisysevent_wrapper.h"
// #include "hisysevent.h"

#include "hks_log.h"
#include "hks_template.h"

// using namespace OHOS::HiviewDFX;

// static constexpr const char g_eventName[] = "HUKS_FAULT";
// static constexpr const char g_tagFunction[] = "FUNCTION";
// static constexpr const char g_tagUserId[] = "USER_ID";
// static constexpr const char g_tagProcessUID[] = "PROCESS_UID";
// static constexpr const char g_tagKeyType[] = "KEY_TYPE";
// static constexpr const char g_tagErrorCode[] = "ERROR_CODE";
// static constexpr const char g_tagExtra[] = "EXTRA";

static int32_t ConvertToHiSysEventType(enum EventType inEventType,
    int32_t *outEventTypeInt)
{
    // switch (inEventType) {
    //     case FAULT:
    //         *outEventTypeInt = HiSysEvent::EventType::FAULT;
    //         break;
    //     case STATISTIC:
    //         *outEventTypeInt = HiSysEvent::EventType::STATISTIC;
    //         break;
    //     case SECURITY:
    //         *outEventTypeInt = HiSysEvent::EventType::SECURITY;
    //         break;
    //     case BEHAVIOR:
    //         *outEventTypeInt = HiSysEvent::EventType::BEHAVIOR;
    //         break;
    //     default:
    //         HKS_LOG_E("Invalid inEventType!");
    //         return HKS_ERROR_INVALID_ARGUMENT;
    // }
    return HKS_SUCCESS;
}

int WriteEvent(enum EventType eventType, const char *functionName, const struct EventValues *eventValues,
    const char *extra)
{
    return 0;
    int32_t outEventTypeInt = 0;
    int32_t ret = ConvertToHiSysEventType(eventType, &outEventTypeInt);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "convert to hiSysEvent event type failed!")

    // enum HiSysEvent::EventType outEventType = static_cast<enum HiSysEvent::EventType>(outEventTypeInt);

    // ret = HiSysEventWrite(HiSysEvent::Domain::HUKS, g_eventName,
    //     outEventType, g_tagFunction, functionName, g_tagUserId, eventValues->userId, g_tagProcessUID,
    //     eventValues->processName, g_tagKeyType, eventValues->keyType, g_tagErrorCode, eventValues->errorCode,
    //     g_tagExtra, extra);
    // return ret;
}
