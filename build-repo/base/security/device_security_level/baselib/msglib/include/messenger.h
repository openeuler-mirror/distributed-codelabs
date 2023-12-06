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

#ifndef SEC_MESSENGER_INFO_H
#define SEC_MESSENGER_INFO_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DEVICE_SECURITY_DEFINES_H
#define DEVICE_ID_MAX_LEN 64
typedef struct DeviceIdentify {
    uint32_t length;
    uint8_t identity[DEVICE_ID_MAX_LEN];
} DeviceIdentify;
#endif

typedef int32_t (*DeviceMessageReceiver)(const DeviceIdentify *devId, const uint8_t *msg, uint32_t msgLen);

typedef int32_t (*DeviceStatusReceiver)(const DeviceIdentify *devId, uint32_t status, uint32_t devType);

typedef int32_t (*MessageSendResultNotifier)(const DeviceIdentify *devId, uint64_t transNo, uint32_t result);

typedef int32_t (*DeviceProcessor)(const DeviceIdentify *devId, uint32_t devType, void *para);

typedef struct MessengerConfig {
    const char *pkgName;
    const char *primarySessName;
    const char *secondarySessName;
    DeviceMessageReceiver messageReceiver;
    DeviceStatusReceiver statusReceiver;
    MessageSendResultNotifier sendResultNotifier;
    uint32_t threadCnt;
} MessengerConfig;

typedef struct StatisticInformation {
    uint64_t firstOnlineTime;
    uint64_t lastOnlineTime;
    uint64_t firstOfflineTime;
    uint64_t lastOfflineTime;
    uint64_t lastSessOpenTime;
    uint32_t lastSessOpenResult;
    uint32_t lastSessOpenCost;
    uint64_t lastSessCloseTime;
    uint64_t lastMsgSendTime;
    uint32_t lastMsgSendResult;
    uint32_t lastMsgSendCost;
    uint64_t lastMsgRecvTime;
    uint64_t packetRx;
    uint64_t packetTotalRx;
    uint64_t packetTx;
    uint64_t packetTotalTx;
    uint64_t packetTxFailed;
    uint64_t packetTotalTxFailed;
} StatisticInformation;

typedef struct Messenger Messenger;

Messenger *CreateMessenger(const MessengerConfig *config);

void DestroyMessenger(Messenger *messenger);

bool IsMessengerReady(const Messenger *messenger);

void SendMsgTo(const Messenger *messenger, uint64_t transNo, const DeviceIdentify *devId, const uint8_t *msg,
    uint32_t msgLen);

bool GetDeviceOnlineStatus(const Messenger *messenger, const DeviceIdentify *devId, uint32_t *devType);

bool GetSelfDeviceIdentify(const Messenger *messenger, DeviceIdentify *devId, uint32_t *devType);

void ForEachDeviceProcess(const Messenger *messenger, const DeviceProcessor processor, void *para);

bool GetDeviceStatisticInfo(const Messenger *messenger, const DeviceIdentify *devId, StatisticInformation *info);

#ifdef __cplusplus
}
#endif

#endif // SEC_MESSENGER_INFO_H
