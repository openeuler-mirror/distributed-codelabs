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

#ifndef DSLM_CORE_PROCESS_H
#define DSLM_CORE_PROCESS_H

#include <stdbool.h>
#include <stdint.h>

#include "device_security_defines.h"

#include "dslm_callback_info.h"

#ifdef __cplusplus
extern "C" {
#endif

int32_t OnPeerMsgRequestInfoReceived(const DeviceIdentify *deviceId, const uint8_t *msg, uint32_t len);

int32_t OnPeerMsgResponseInfoReceived(const DeviceIdentify *deviceId, const uint8_t *msg, uint32_t len);

int32_t OnRequestDeviceSecLevelInfo(const DeviceIdentify *deviceId, const RequestOption *option, uint32_t owner,
    uint32_t cookie, RequestCallback callback);

int32_t OnMsgSendResultNotifier(const DeviceIdentify *deviceId, uint64_t transNo, uint32_t result);

int32_t OnPeerStatusReceiver(const DeviceIdentify *deviceId, uint32_t status, uint32_t devType);

bool InitDslmProcess(void);

bool DeinitDslmProcess(void);

bool InitSelfDeviceSecureLevel(void);

#ifdef __cplusplus
}
#endif

#endif // DSLM_CORE_PROCESS_H
