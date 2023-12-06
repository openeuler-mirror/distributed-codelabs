/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef VIRTUAL_COMMUNICATOR_H
#define VIRTUAL_COMMUNICATOR_H

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <map>
#include <mutex>
#include <string>

#include "icommunicator.h"
#include "ref_object.h"
#include "serial_buffer.h"
#include "sync_types.h"

namespace DistributedDB {
class VirtualCommunicatorAggregator;

class VirtualCommunicator : public ICommunicator {
public:
    VirtualCommunicator(const std::string &deviceId, VirtualCommunicatorAggregator *communicatorAggregator);
    ~VirtualCommunicator() override;

    DISABLE_COPY_ASSIGN_MOVE(VirtualCommunicator);

    int RegOnMessageCallback(const OnMessageCallback &onMessage, const Finalizer &inOper) override;
    int RegOnConnectCallback(const OnConnectCallback &onConnect, const Finalizer &inOper) override;
    int RegOnSendableCallback(const std::function<void(void)> &onSendable, const Finalizer &inOper) override;

    void Activate() override;

    uint32_t GetCommunicatorMtuSize() const override;
    uint32_t GetCommunicatorMtuSize(const std::string &target) const override;
    void SetCommunicatorMtuSize(uint32_t mtuSize);

    uint32_t GetTimeout() const override;
    uint32_t GetTimeout(const std::string &target) const override;
    void SetTimeout(uint32_t timeout);

    int GetLocalIdentity(std::string &outTarget) const override;

    int SendMessage(const std::string &dstTarget, const Message *inMsg, const SendConfig &config) override;
    int SendMessage(const std::string &dstTarget, const Message *inMsg, const SendConfig &config,
        const OnSendEnd &onEnd) override;

    int GetRemoteCommunicatorVersion(const std::string &deviceId, uint16_t &version) const override;

    void CallbackOnMessage(const std::string &srcTarget, Message *inMsg);

    void CallbackOnConnect(const std::string &target, bool isConnect) const;

    int GeneralVirtualSyncId();

    void Disable();

    void Enable();

    void SetDeviceId(const std::string &deviceId);

    std::string GetDeviceId() const;

    bool IsEnabled() const;

    bool IsDeviceOnline(const std::string &device) const override;

    void SetDropMessageTypeByDevice(MessageId msgid, uint32_t dropTimes = 1);

private:
    int TimeSync();
    int DataSync();
    int WaterMarkSync();
    static int TranslateMsg(const Message *inMsg, Message *&outMsg);

    mutable std::mutex onMessageLock_;
    OnMessageCallback onMessage_;

    mutable std::mutex onConnectLock_;
    OnConnectCallback onConnect_;
    mutable std::mutex devicesMapLock_;
    mutable std::map<std::string, bool> onlineDevicesMap_;

    std::string remoteDeviceId_ = "real_device";
    std::mutex syncIdLock_;
    int currentSyncId_ = 1000;
    bool isEnable_ = true;
    std::string deviceId_;

    std::mutex onAggregatorLock_;
    VirtualCommunicatorAggregator *communicatorAggregator_;

    uint32_t timeout_ = 5 * 1000; // 5 * 1000ms
    MessageId dropMsgId_ = MessageId::UNKNOW_MESSAGE;
    uint32_t dropMsgTimes_ = 0;
    uint32_t mtuSize_ = 5 * 1024 * 1024; // 5 * 1024 * 1024B
};
} // namespace DistributedDB

#endif // VIRTUAL_COMMUNICATOR_H
