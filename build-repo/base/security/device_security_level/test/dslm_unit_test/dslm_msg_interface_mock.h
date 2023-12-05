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

#ifndef DSLM_MSG_INTERFACE_MOCK_H
#define DSLM_MSG_INTERFACE_MOCK_H

#include <gmock/gmock.h>
#include <mutex>

#include "messenger.h"

#include "dslm_callback_info.h"

namespace OHOS {
namespace Security {
namespace DslmUnitTest {
class DslmMsgInterface {
public:
    DslmMsgInterface() {};
    virtual ~DslmMsgInterface() {};

    virtual bool IsMessengerReady(const Messenger *messenger) = 0;

    virtual uint64_t SendMsgTo(const Messenger *messenger, uint64_t transNo, const DeviceIdentify *devId,
        const uint8_t *msg, uint32_t msgLen) = 0;

    virtual bool GetDeviceOnlineStatus(const Messenger *messenger, const DeviceIdentify *devId, uint32_t *devType) = 0;

    virtual bool GetSelfDeviceIdentify(const Messenger *messenger, DeviceIdentify *devId, uint32_t *devType) = 0;

    virtual void ForEachDeviceProcess(const Messenger *messenger, const DeviceProcessor processor, void *para) = 0;
};

class DslmMsgInterfaceMock : public DslmMsgInterface {
public:
    DslmMsgInterfaceMock();
    ~DslmMsgInterfaceMock() override;
    MOCK_METHOD1(IsMessengerReady, bool(const Messenger *messenger));
    MOCK_METHOD5(SendMsgTo, uint64_t(const Messenger *messenger, uint64_t transNo, const DeviceIdentify *devId,
                                const uint8_t *msg, uint32_t msgLen));
    MOCK_METHOD3(GetDeviceOnlineStatus,
        bool(const Messenger *messenger, const DeviceIdentify *devId, uint32_t *devType));
    MOCK_METHOD3(GetSelfDeviceIdentify, bool(const Messenger *messenger, DeviceIdentify *devId, uint32_t *devType));
    MOCK_METHOD3(ForEachDeviceProcess, void(const Messenger *messenger, const DeviceProcessor processor, void *para));
    void MakeMsgLoopback() const;
    void MakeSelfDeviceId(const DeviceIdentify *devId) const;
    void MakeDeviceOnline(const DeviceIdentify *devId) const;
    void MakeDeviceOffline(const DeviceIdentify *devId) const;
    void MakeMsgReceivedFrom(const DeviceIdentify *devId, const uint8_t *msg, uint32_t msgLen) const;
};
} // namespace DslmUnitTest
} // namespace Security
} // namespace OHOS

#endif // DSLM_MSG_INTERFACE_MOCK_H
