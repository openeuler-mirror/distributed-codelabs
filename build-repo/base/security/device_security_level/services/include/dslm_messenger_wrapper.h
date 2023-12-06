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

#ifndef SERVICE_MSG_MESSENGER_WRAPPER_H
#define SERVICE_MSG_MESSENGER_WRAPPER_H

#include <stdbool.h>
#include <stdint.h>

#include "device_security_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ONLINE 1
#define OFFLINE 0

typedef int32_t (*MessageReceiver)(const DeviceIdentify *devId, const uint8_t *msg, uint32_t len);

typedef int32_t (*StatusReceiver)(const DeviceIdentify *devId, uint32_t status, uint32_t devType);

typedef int32_t (*SendResultNotifier)(const DeviceIdentify *devId, uint64_t transNo, uint32_t result);

typedef int32_t (*DeviceProcessor)(const DeviceIdentify *devId, uint32_t devType, void *para);

void SendMsgToDevice(uint64_t transNo, const DeviceIdentify *devId, const uint8_t *msg, uint32_t msgLen);

uint32_t InitMessenger(const MessageReceiver messageReceiver, const StatusReceiver statusReceiver,
    const SendResultNotifier notifier);

uint32_t DeinitMessenger(void);

bool GetMessengerStatus(void);

bool GetPeerDeviceOnlineStatus(const DeviceIdentify *devId, uint32_t *devType);

const DeviceIdentify *GetSelfDevice(uint32_t *devType);

const char *GetMessengerPackageName(void) __attribute__((weak));
const char *GetMessengerPrimarySessionName(void) __attribute__((weak));
const char *GetMessengerSecondarySessionName(void) __attribute__((weak));

#ifdef __cplusplus
}
#endif

#endif // SERVICE_MSG_MESSENGER_WRAPPER_H
