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

#include "messenger.h"
#include "messenger_impl.h"

Messenger *CreateMessenger(const MessengerConfig *config)
{
    return CreateMessengerImpl(config);
}

void DestroyMessenger(Messenger *messenger)
{
    DestroyMessengerImpl(messenger);
}

bool IsMessengerReady(const Messenger *messenger)
{
    return IsMessengerReadyImpl(messenger);
}

void SendMsgTo(const Messenger *messenger, uint64_t transNo, const DeviceIdentify *devId, const uint8_t *msg,
    uint32_t msgLen)
{
    SendMsgToImpl(messenger, transNo, devId, msg, msgLen);
}

bool GetDeviceOnlineStatus(const Messenger *messenger, const DeviceIdentify *devId, uint32_t *devType)
{
    return GetDeviceOnlineStatusImpl(messenger, devId, devType);
}

bool GetSelfDeviceIdentify(const Messenger *messenger, DeviceIdentify *devId, uint32_t *devType)
{
    return GetSelfDeviceIdentifyImpl(messenger, devId, devType);
}

void ForEachDeviceProcess(const Messenger *messenger, const DeviceProcessor processor, void *para)
{
    ForEachDeviceProcessImpl(messenger, processor, para);
}

bool GetDeviceStatisticInfo(const Messenger *messenger, const DeviceIdentify *devId, StatisticInformation *info)
{
    return GetDeviceStatisticInfoImpl(messenger, devId, info);
}