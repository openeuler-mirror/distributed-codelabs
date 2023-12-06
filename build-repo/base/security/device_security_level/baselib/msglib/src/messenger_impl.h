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

#ifndef SEC_MESSENGER_IMPL_H
#define SEC_MESSENGER_IMPL_H

#include "messenger.h"

#ifdef __cplusplus
extern "C" {
#endif

Messenger *CreateMessengerImpl(const MessengerConfig *config) __attribute__((weak));

void DestroyMessengerImpl(Messenger *messenger) __attribute__((weak));

void SendMsgToImpl(const Messenger *messenger, uint64_t transNo, const DeviceIdentify *devId, const uint8_t *msg,
    uint32_t msgLen) __attribute__((weak));

bool IsMessengerReadyImpl(const Messenger *messenger) __attribute__((weak));

bool GetDeviceOnlineStatusImpl(const Messenger *messenger, const DeviceIdentify *devId, uint32_t *devType)
    __attribute__((weak));

bool GetSelfDeviceIdentifyImpl(const Messenger *messenger, DeviceIdentify *devId, uint32_t *devType)
    __attribute__((weak));

void ForEachDeviceProcessImpl(const Messenger *messenger, const DeviceProcessor processor, void *para)
    __attribute__((weak));

bool GetDeviceStatisticInfoImpl(const Messenger *messenger, const DeviceIdentify *devId, StatisticInformation *info)
    __attribute__((weak));

#ifdef __cplusplus
}
#endif

#endif // SEC_MESSENGER_IMPL_H
