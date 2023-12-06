/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef NSTACKX_DFINDER_MGT_MSG_LOG_H
#define NSTACKX_DFINDER_MGT_MSG_LOG_H

#include "coap_discover.h"
#include "cJSON.h"
#include "nstackx_error.h"

#ifdef DFINDER_MGT_MSG_LOG
#define DFINDER_MGT_UNPACK_LOG_LEN 400
#define DFINDER_MGT_UUID_LEN 14

typedef struct DeviceInfo DeviceInfo;

void DFinderSetMgtMsgLog(int enable);
void DFinderMgtReqLog(CoapRequest *coapRequest);
void DFinderMgtUnpackLog(DeviceInfo *deviceInfo);

#define DFINDER_MGT_REQ_LOG(coapRequest) DFinderMgtReqLog(coapRequest)
#define DFINDER_MGT_UNPACK_LOG(deviceInfo) DFinderMgtUnpackLog(deviceInfo)

#else
#define DFINDER_MGT_REQ_LOG(coapRequest)
#define DFINDER_MGT_UNPACK_LOG(deviceInfo)
#endif

#endif /* END OF NSTACKX_DFINDER_MGT_MSG_LOG_H */
