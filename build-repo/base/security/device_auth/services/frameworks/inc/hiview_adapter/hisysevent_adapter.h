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

#ifndef HISYSEVENT_ADAPTER_H
#define HISYSEVENT_ADAPTER_H

#include <stdint.h>

#define EVENT_APPID_LEN 128


enum InvokeEventId {
    EVENT_ID_CREATE_GROUP = 0,
    EVENT_ID_DELETE_GROUP = 1,
    EVENT_ID_ADD_MEMBER = 2,
    EVENT_ID_DELETE_MEMBER = 3,
    EVENT_ID_GET_REGISTER_INFO = 4,
    EVENT_ID_ADD_MULTI_MEMBERS = 5,
    EVENT_ID_DELETE_MULTI_MEMBERS = 6,
    EVENT_ID_AUTH_DEVICE = 7,
};

typedef struct InvokeEvent {
    int32_t eventId;
    char appId[EVENT_APPID_LEN];
    int32_t type;
    int32_t batchNumber;
    int32_t result;
    int32_t osAccountId;
} InvokeEvent;

#ifdef __cplusplus
extern "C" {
#endif

void ReportCoreFuncInvokeEvent(const InvokeEvent *event);

#ifdef __cplusplus
}
#endif

#endif // SERVICE_DSLM_BIGDATA_H
