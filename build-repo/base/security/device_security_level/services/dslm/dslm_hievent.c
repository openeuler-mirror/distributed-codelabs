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

#include "dslm_hievent.h"

#include <stddef.h>
#include <stdint.h>

#include "securec.h"

#include "device_security_defines.h"
#include "dslm_core_defines.h"
#include "dslm_cred.h"
#include "dslm_bigdata.h"

void ReportHiEventServiceStartFailed(const uint32_t errorType)
{
    ReportServiceStartFailedEvent(errorType);
}

void ReportHiEventInitSelfFailed(const char *errorString)
{
    ReportInitSelfFailedEvent(errorString);
}

void ReportHiEventInfoSync(const DslmDeviceInfo *info)
{
    if (info == NULL) {
        return;
    }

    SecurityInfoSyncEvent event;
    (void)memset_s(&event, sizeof(SecurityInfoSyncEvent), 0, sizeof(SecurityInfoSyncEvent));

    if (info->lastResponseTime >= info->lastRequestTime) {
        event.costTime = (int32_t)(info->lastResponseTime - info->lastRequestTime);
    }

    event.retCode = (int32_t)info->result;
    event.secLevel = (int32_t)info->credInfo.credLevel;
    event.localVersion = GetCurrentVersion();
    event.targetVersion = info->version;
    if (memcpy_s(event.targetModel, MODEL_MAX_LEN, info->credInfo.model, CRED_INFO_MODEL_LEN) != EOK) {
        (void)memset_s(event.targetModel, MODEL_MAX_LEN, 0, MODEL_MAX_LEN);
    }

    event.credType = info->credInfo.credType;
    ReportSecurityInfoSyncEvent(&event);
}

void ReportHiEventAppInvoke(const DslmDeviceInfo *info)
{
    if (info == NULL) {
        return;
    }

    AppInvokeEvent event;
    (void)memset_s(&event, sizeof(AppInvokeEvent), 0, sizeof(AppInvokeEvent));
    event.costTime = 0;
    if (info->lastResponseTime >= info->lastRequestTime) {
        event.costTime = (int32_t)(info->lastResponseTime - info->lastRequestTime);
    }
    event.uid = 0;
    event.retCode = (int32_t)info->result;
    event.secLevel = (int32_t)info->credInfo.credLevel;
    event.retMode = (info->result == ERR_NEED_COMPATIBLE) ? 1 : 0;

    if (memcpy_s(event.targetModel, MODEL_MAX_LEN, info->credInfo.model, CRED_INFO_MODEL_LEN) != EOK) {
        (void)memset_s(event.targetModel, MODEL_MAX_LEN, 0, MODEL_MAX_LEN);
    }

    ReportAppInvokeEvent(&event);
}