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

#include "dslm_messenger_wrapper.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "messenger.h"
#include "utils_mutex.h"
#include "device_security_defines.h"

Messenger *g_messenger = NULL;
static Mutex g_mutex = INITED_MUTEX;

uint32_t InitMessenger(const MessageReceiver messageReceiver, const StatusReceiver statusReceiver,
    const SendResultNotifier notifier)
{
    MessengerConfig config = {
        .pkgName = GetMessengerPackageName(),
        .primarySessName = GetMessengerPrimarySessionName(),
        .secondarySessName = GetMessengerSecondarySessionName(),
        .messageReceiver = messageReceiver,
        .statusReceiver = statusReceiver,
        .sendResultNotifier = notifier,
    };
    InitMutex(&g_mutex);
    LockMutex(&g_mutex);
    g_messenger = CreateMessenger(&config);
    UnlockMutex(&g_mutex);
    if (g_messenger == NULL) {
        return ERR_MSG_NOT_INIT;
    }

    return SUCCESS;
}

uint32_t DeinitMessenger(void)
{
    LockMutex(&g_mutex);
    if (g_messenger == NULL) {
        UnlockMutex(&g_mutex);
        return SUCCESS;
    }
    DestroyMessenger(g_messenger);
    UnlockMutex(&g_mutex);
    return SUCCESS;
}

bool GetMessengerStatus(void)
{
    LockMutex(&g_mutex);
    if (g_messenger == NULL) {
        UnlockMutex(&g_mutex);
        return false;
    }
    bool ret = IsMessengerReady(g_messenger);
    UnlockMutex(&g_mutex);
    return ret;
}

void SendMsgToDevice(uint64_t transNo, const DeviceIdentify *devId, const uint8_t *msg, uint32_t msgLen)
{
    LockMutex(&g_mutex);
    if (g_messenger == NULL) {
        UnlockMutex(&g_mutex);
        return;
    }
    SendMsgTo(g_messenger, transNo, devId, msg, msgLen);
    UnlockMutex(&g_mutex);
    return;
}

bool GetPeerDeviceOnlineStatus(const DeviceIdentify *devId, uint32_t *devType)
{
    LockMutex(&g_mutex);
    if (g_messenger == NULL) {
        UnlockMutex(&g_mutex);
        return false;
    }
    if (devId == NULL || devType == NULL) {
        UnlockMutex(&g_mutex);
        return false;
    }
    bool ret = GetDeviceOnlineStatus(g_messenger, devId, devType);
    UnlockMutex(&g_mutex);
    return ret;
}

const DeviceIdentify *GetSelfDevice(uint32_t *devType)
{
    LockMutex(&g_mutex);
    static uint32_t type = 0;
    static DeviceIdentify deviceId = {0, {0}};
    if (deviceId.length == 0 || deviceId.identity[0] == 0) {
        if (g_messenger != NULL) {
            GetSelfDeviceIdentify(g_messenger, &deviceId, &type);
        }
    }
    if (devType != NULL) {
        *devType = type;
    }
    UnlockMutex(&g_mutex);
    return &deviceId;
}

__attribute__((weak)) const char *GetMessengerPackageName(void)
{
    return "ohos.dslm";
}

__attribute__((weak)) const char *GetMessengerPrimarySessionName(void)
{
    return "device.security.level";
}

__attribute__((weak)) const char *GetMessengerSecondarySessionName(void)
{
    return NULL;
}