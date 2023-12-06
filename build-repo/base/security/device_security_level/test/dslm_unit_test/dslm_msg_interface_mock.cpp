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

#include "dslm_msg_interface_mock.h"

#include <memory>
#include <thread>
#include <vector>

using namespace OHOS::Security::DslmUnitTest;
using namespace testing;
using namespace testing::ext;

extern "C" {
// just for testing
extern Messenger *g_messenger;
extern int32_t OnPeerMsgReceived(const DeviceIdentify *devId, const uint8_t *msg, uint32_t len);
extern int32_t OnSendResultNotifier(const DeviceIdentify *devId, uint64_t transNo, uint32_t result);
extern int32_t OnPeerStatusReceiver(const DeviceIdentify *deviceId, uint32_t status, uint32_t devType);
}

namespace OHOS {
namespace Security {
namespace DslmUnitTest {
static DslmMsgInterface *GetDslmMsgInterface()
{
    return reinterpret_cast<DslmMsgInterfaceMock *>(g_messenger);
}

DslmMsgInterfaceMock::DslmMsgInterfaceMock()
{
    g_messenger = reinterpret_cast<Messenger *>(this);
    ON_CALL(*this, IsMessengerReady).WillByDefault(Return(true));
}

DslmMsgInterfaceMock::~DslmMsgInterfaceMock()
{
    g_messenger = nullptr;
}

void DslmMsgInterfaceMock::MakeMsgLoopback() const
{
    auto loopback = [this](const Messenger *messenger, uint64_t transNo, const DeviceIdentify *devId,
                        const uint8_t *msg, uint32_t msgLen) {
        this->MakeMsgReceivedFrom(devId, msg, msgLen);
        return 0;
    };

    ON_CALL(*this, SendMsgTo).WillByDefault(loopback);
}

void DslmMsgInterfaceMock::MakeSelfDeviceId(const DeviceIdentify *self) const
{
    auto loopback = [this, self](const Messenger *messenger, DeviceIdentify *devId, uint32_t *devType) {
        *devId = *self;
        return true;
    };

    ON_CALL(*this, GetSelfDeviceIdentify).WillByDefault(loopback);
}

void DslmMsgInterfaceMock::MakeDeviceOnline(const DeviceIdentify *devId) const
{
    OnPeerStatusReceiver(devId, 1, 0);
}

void DslmMsgInterfaceMock::MakeDeviceOffline(const DeviceIdentify *devId) const
{
    OnPeerStatusReceiver(devId, 0, 0);
}

void DslmMsgInterfaceMock::MakeMsgReceivedFrom(const DeviceIdentify *devId, const uint8_t *msg, uint32_t msgLen) const
{
    auto msgBuffer = std::make_shared<std::vector<uint8_t>>(msg, msg + msgLen);
    std::thread t([devId, msgBuffer]() { OnPeerMsgReceived(devId, msgBuffer->data(), msgBuffer->size()); });
    t.detach();
}

extern "C" {
Messenger *CreateMessengerImpl(const MessengerConfig *config)
{
    (void)config;
    return g_messenger;
}

void DestroyMessengerImpl(Messenger *messenger)
{
    (void)messenger;
}

bool IsMessengerReadyImpl(const Messenger *messenger)
{
    return GetDslmMsgInterface()->IsMessengerReady(messenger);
}

uint64_t SendMsgToImpl(const Messenger *messenger, uint64_t transNo, const DeviceIdentify *devId, const uint8_t *msg,
    uint32_t msgLen)
{
    return GetDslmMsgInterface()->SendMsgTo(messenger, transNo, devId, msg, msgLen);
}

bool GetDeviceOnlineStatusImpl(const Messenger *messenger, const DeviceIdentify *devId, uint32_t *devType)
{
    return GetDslmMsgInterface()->GetDeviceOnlineStatus(messenger, devId, devType);
}

bool GetSelfDeviceIdentifyImpl(const Messenger *messenger, DeviceIdentify *devId, uint32_t *devType)
{
    return GetDslmMsgInterface()->GetSelfDeviceIdentify(messenger, devId, devType);
}

void ForEachDeviceProcessImpl(const Messenger *messenger, const DeviceProcessor processor, void *para)
{
    return;
}

bool GetDeviceStatisticInfoImpl(const Messenger *messenger, const DeviceIdentify *devId, StatisticInformation *info)
{
    return true;
}
}
} // namespace DslmUnitTest
} // namespace Security
} // namespace OHOS
