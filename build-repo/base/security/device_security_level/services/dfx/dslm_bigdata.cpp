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
// #include "hisysevent.h"

#ifdef __cplusplus
extern "C" {
#endif
// namespace {
// constexpr char STR_EVENT_START_FAILED[] = "SERVICE_START_FAILED";
// constexpr char STR_EVENT_INIT_SELF_LEVEL_FAULT[] = "INIT_SELF_LEVEL_FAULT";
// constexpr char STR_EVENT_CALL_INTERFACE[] = "CALL_INTERFACE";
// constexpr char STR_EVENT_QUERY_INFO[] = "QUERY_INFO";
// constexpr char STR_ERROR_TYPE[] = "ERROR_TYPE";
// constexpr char STR_ERROR_STR[] = "ERROR_STR";
// constexpr char STR_USER_ID[] = "USER_ID";
// constexpr char STR_COST_TIME[] = "COST_TIME";
// constexpr char STR_RET_CODE[] = "RET_CODE";
// constexpr char STR_SEC_LEVEL[] = "SEC_LEVEL";
// constexpr char STR_RET_MODE[] = "RET_MODE";
// constexpr char STR_LOCAL_MODEL[] = "LOCAL_MODEL";
// constexpr char STR_TARGET_MODEL[] = "TARGET_MODEL";
// constexpr char STR_PKG_NAME[] = "PKG_NAME";
// constexpr char STR_LOCAL_VERSION[] = "LOCAL_VERSION";
// constexpr char STR_TARGET_VERSION[] = "TARGET_VERSION";
// constexpr char STR_CRED_TYPE[] = "CRED_TYPE";
// } // namespace
void ReportServiceStartFailedEvent(const uint32_t errorType)
{
    return;
    // HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::DSLM, STR_EVENT_START_FAILED,
    //     OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, STR_ERROR_TYPE, errorType);
}

void ReportInitSelfFailedEvent(const char *errorString)
{
    return;
    // HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::DSLM, STR_EVENT_INIT_SELF_LEVEL_FAULT,
    //     OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, STR_ERROR_STR, errorString);
}

void ReportAppInvokeEvent(const AppInvokeEvent *event)
{
    if (event == nullptr) {
        return;
    }

    return;
    // HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::DSLM, STR_EVENT_CALL_INTERFACE,
    //     OHOS::HiviewDFX::HiSysEvent::EventType::STATISTIC, STR_USER_ID, event->uid, STR_COST_TIME, event->costTime,
    //     STR_RET_CODE, event->retCode, STR_SEC_LEVEL, event->secLevel, STR_RET_MODE, event->retMode, STR_LOCAL_MODEL,
    //     event->localModel, STR_TARGET_MODEL, event->targetModel, STR_PKG_NAME, event->pkgName);
}

void ReportSecurityInfoSyncEvent(const SecurityInfoSyncEvent *event)
{
    if (event == nullptr) {
        return;
    }
    return;
    // HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::DSLM, STR_EVENT_QUERY_INFO,
    //     OHOS::HiviewDFX::HiSysEvent::EventType::STATISTIC, STR_LOCAL_MODEL, event->localModel, STR_TARGET_MODEL,
    //     event->targetModel, STR_LOCAL_VERSION, event->localVersion, STR_TARGET_VERSION, event->targetVersion,
    //     STR_CRED_TYPE, event->credType, STR_RET_CODE, event->retCode, STR_COST_TIME, event->costTime, STR_SEC_LEVEL,
    //     event->secLevel);
}

#ifdef __cplusplus
}
#endif
