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

#include <stddef.h>
#include <stdint.h>
#include "securec.h"
#include "hc_types.h"

#include "hisysevent_adapter.h"
#include "device_auth_defines.h"
#include "device_auth.h"

void ReportHiEventCoreFuncInvoke(int32_t eventId, int32_t osAccountId, CJson *params, int32_t result)
{
    if (params == NULL) {
        return;
    }

    InvokeEvent event;
    (void)memset_s(&event, sizeof(InvokeEvent), 0, sizeof(InvokeEvent));

    int32_t groupType = PEER_TO_PEER_GROUP;
    if (GetIntFromJson(params, FIELD_GROUP_TYPE, &groupType) != HC_SUCCESS) {
        return;
    }

    event.eventId = eventId;
    event.type = groupType;
    event.batchNumber = 0;
    event.result = result;
    event.osAccountId = osAccountId;

    const char *appId = GetStringFromJson(params, FIELD_APP_ID);
    if (appId == NULL) {
        return;
    }
    if (memcpy_s(event.appId, EVENT_APPID_LEN, appId, HcStrlen(appId)) != EOK) {
        (void)memset_s(event.appId, EVENT_APPID_LEN, 0, EVENT_APPID_LEN);
    }

    ReportCoreFuncInvokeEvent(&event);
}