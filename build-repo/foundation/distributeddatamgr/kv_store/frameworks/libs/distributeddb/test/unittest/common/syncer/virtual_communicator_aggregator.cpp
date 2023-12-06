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
#include "virtual_communicator_aggregator.h"

#include <cstdint>
#include <thread>
#include <utility>

#include "db_common.h"
#include "db_errno.h"
#include "log_print.h"
#include "runtime_context.h"

namespace DistributedDB {
int VirtualCommunicatorAggregator::Initialize(IAdapter *inAdapter)
{
    return E_OK;
}

void VirtualCommunicatorAggregator::Finalize()
{
}

// If not success, return nullptr and set outErrorNo
ICommunicator *VirtualCommunicatorAggregator::AllocCommunicator(uint64_t commLabel, int &outErrorNo)
{
    if (isEnable_) {
        return AllocCommunicator(remoteDeviceId_, outErrorNo);
    }
    return nullptr;
}

ICommunicator *VirtualCommunicatorAggregator::AllocCommunicator(const LabelType &commLabel, int &outErrorNo)
{
    LOGI("[VirtualCommunicatorAggregator][Alloc] Label=%.6s.", VEC_TO_STR(commLabel));
    if (commLabel.size() != COMM_LABEL_LENGTH) {
        outErrorNo = -E_INVALID_ARGS;
        return nullptr;
    }

    if (isEnable_) {
        return AllocCommunicator(remoteDeviceId_, outErrorNo);
    }
    return nullptr;
}

void VirtualCommunicatorAggregator::ReleaseCommunicator(ICommunicator *inCommunicator)
{
    // Called in main thread only
    VirtualCommunicator *communicator = static_cast<VirtualCommunicator *>(inCommunicator);
    OfflineDevice(communicator->GetDeviceId());
    {
        std::lock_guard<std::mutex> lock(communicatorsLock_);
        communicators_.erase(communicator->GetDeviceId());
    }
    RefObject::KillAndDecObjRef(communicator);
    communicator = nullptr;
}

int VirtualCommunicatorAggregator::RegCommunicatorLackCallback(const CommunicatorLackCallback &onCommLack,
    const Finalizer &inOper)
{
    onCommLack_ = onCommLack;
    return E_OK;
}

int VirtualCommunicatorAggregator::RegOnConnectCallback(const OnConnectCallback &onConnect, const Finalizer &inOper)
{
    onConnect_ = onConnect;
    RunOnConnectCallback("deviceId", true);
    return E_OK;
}

void VirtualCommunicatorAggregator::RunCommunicatorLackCallback(const LabelType &commLabel)
{
    if (onCommLack_) {
        std::string userId;
        onCommLack_(commLabel, userId_);
    }
}

void VirtualCommunicatorAggregator::RunOnConnectCallback(const std::string &target, bool isConnect)
{
    if (onConnect_) {
        onConnect_(target, isConnect);
    }
}

int VirtualCommunicatorAggregator::GetLocalIdentity(std::string &outTarget) const
{
    outTarget = "DEVICES_A";
    return E_OK;
}

void VirtualCommunicatorAggregator::OnlineDevice(const std::string &deviceId) const
{
    if (!isEnable_) {
        return;
    }

    // Called in main thread only
    for (const auto &iter : communicators_) {
        VirtualCommunicator *communicatorTmp = static_cast<VirtualCommunicator *>(iter.second);
        if (iter.first != deviceId) {
            communicatorTmp->CallbackOnConnect(deviceId, true);
        }
    }
}

void VirtualCommunicatorAggregator::OfflineDevice(const std::string &deviceId) const
{
    if (!isEnable_) {
        return;
    }

    // Called in main thread only
    for (const auto &iter : communicators_) {
        VirtualCommunicator *communicatorTmp = static_cast<VirtualCommunicator *>(iter.second);
        if (iter.first != deviceId) {
            communicatorTmp->CallbackOnConnect(deviceId, false);
        }
    }
}

ICommunicator *VirtualCommunicatorAggregator::AllocCommunicator(const std::string &deviceId, int &outErrorNo)
{
    // Called in main thread only
    VirtualCommunicator *communicator = new (std::nothrow) VirtualCommunicator(deviceId, this);
    if (communicator == nullptr) {
        outErrorNo = -E_OUT_OF_MEMORY;
    }
    {
        std::lock_guard<std::mutex> lock(communicatorsLock_);
        communicators_.insert(std::pair<std::string, VirtualCommunicator *>(deviceId, communicator));
    }
    OnlineDevice(deviceId);
    return communicator;
}

ICommunicator *VirtualCommunicatorAggregator::GetCommunicator(const std::string &deviceId) const
{
    std::lock_guard<std::mutex> lock(communicatorsLock_);
    auto iter = communicators_.find(deviceId);
    if (iter != communicators_.end()) {
        VirtualCommunicator *communicator = static_cast<VirtualCommunicator *>(iter->second);
        return communicator;
    }
    return nullptr;
}

void VirtualCommunicatorAggregator::DispatchMessage(const std::string &srcTarget, const std::string &dstTarget,
    const Message *inMsg, const OnSendEnd &onEnd)
{
    if (VirtualCommunicatorAggregator::GetBlockValue()) {
        std::unique_lock<std::mutex> lock(blockLock_);
        conditionVar_.wait(lock);
    }

    if (!isEnable_) {
        LOGD("[VirtualCommunicatorAggregator] DispatchMessage, VirtualCommunicatorAggregator is disabled");
        delete inMsg;
        inMsg = nullptr;
        return CallSendEnd(-E_PERIPHERAL_INTERFACE_FAIL, onEnd);
    }
    std::lock_guard<std::mutex> lock(communicatorsLock_);
    auto iter = communicators_.find(dstTarget);
    if (iter != communicators_.end()) {
        LOGI("[VirtualCommunicatorAggregator] DispatchMessage, find dstTarget %s", dstTarget.c_str());
        VirtualCommunicator *communicator = static_cast<VirtualCommunicator *>(iter->second);
        if (!communicator->IsEnabled()) {
            LOGE("[VirtualCommunicatorAggregator] DispatchMessage, find dstTarget %s disabled", dstTarget.c_str());
            delete inMsg;
            inMsg = nullptr;
            return CallSendEnd(-E_PERIPHERAL_INTERFACE_FAIL, onEnd);
        }
        uint32_t messageId = inMsg->GetMessageId();
        Message *msg = const_cast<Message *>(inMsg);
        msg->SetTarget(srcTarget);
        RefObject::IncObjRef(communicator);
        auto onDispatch = onDispatch_;
        bool isNeedDelay = ((sendDelayTime_ > 0) && (delayTimes_ > 0) && (messageId == delayMessageId_) &&
            (delayDevices_.count(dstTarget) > 0) && (skipTimes_ == 0));
        uint32_t sendDelayTime = sendDelayTime_;
        std::thread thread([communicator, srcTarget, dstTarget, msg, isNeedDelay, sendDelayTime, onDispatch]() {
            if (isNeedDelay) {
                std::this_thread::sleep_for(std::chrono::milliseconds(sendDelayTime));
            }
            if (onDispatch) {
                onDispatch(dstTarget, msg);
            }
            communicator->CallbackOnMessage(srcTarget, msg);
            RefObject::DecObjRef(communicator);
        });
        DelayTimeHandle(messageId, dstTarget);
        thread.detach();
        CallSendEnd(E_OK, onEnd);
    } else {
        LOGE("[VirtualCommunicatorAggregator] DispatchMessage, can't find dstTarget %s", dstTarget.c_str());
        delete inMsg;
        inMsg = nullptr;
        CallSendEnd(-E_NOT_FOUND, onEnd);
    }
}

void VirtualCommunicatorAggregator::SetBlockValue(bool value)
{
    std::unique_lock<std::mutex> lock(blockLock_);
    isBlock_ = value;
    if (!value) {
        conditionVar_.notify_all();
    }
}

bool VirtualCommunicatorAggregator::GetBlockValue() const
{
    return isBlock_;
}

void VirtualCommunicatorAggregator::Disable()
{
    isEnable_ = false;
}

void VirtualCommunicatorAggregator::Enable()
{
    LOGD("[VirtualCommunicatorAggregator] enable");
    isEnable_ = true;
}

void VirtualCommunicatorAggregator::CallSendEnd(int errCode, const OnSendEnd &onEnd)
{
    if (onEnd) {
        (void)RuntimeContext::GetInstance()->ScheduleTask([errCode, onEnd]() {
            onEnd(errCode);
        });
    }
}

void VirtualCommunicatorAggregator::RegOnDispatch(
    const std::function<void(const std::string&, Message *inMsg)> &onDispatch)
{
    onDispatch_ = onDispatch;
}

void VirtualCommunicatorAggregator::SetCurrentUserId(const std::string &userId)
{
    userId_ = userId;
}

void VirtualCommunicatorAggregator::SetTimeout(const std::string &deviceId, uint32_t timeout)
{
    std::lock_guard<std::mutex> lock(communicatorsLock_);
    if (communicators_.find(deviceId) != communicators_.end()) {
        communicators_[deviceId]->SetTimeout(timeout);
    }
}

void VirtualCommunicatorAggregator::SetDropMessageTypeByDevice(const std::string &deviceId, MessageId msgid,
    uint32_t dropTimes)
{
    std::lock_guard<std::mutex> lock(communicatorsLock_);
    if (communicators_.find(deviceId) != communicators_.end()) {
        communicators_[deviceId]->SetDropMessageTypeByDevice(msgid, dropTimes);
    }
}

void VirtualCommunicatorAggregator::SetDeviceMtuSize(const std::string &deviceId, uint32_t mtuSize)
{
    std::lock_guard<std::mutex> lock(communicatorsLock_);
    if (communicators_.find(deviceId) != communicators_.end()) {
        communicators_[deviceId]->SetCommunicatorMtuSize(mtuSize);
    }
}

void VirtualCommunicatorAggregator::SetSendDelayInfo(uint32_t sendDelayTime, uint32_t delayMessageId,
    uint32_t delayTimes, uint32_t skipTimes, std::set<std::string> &delayDevices)
{
    sendDelayTime_ = sendDelayTime;
    delayMessageId_ = delayMessageId;
    delayTimes_ = delayTimes;
    delayDevices_ = delayDevices;
    skipTimes_ = skipTimes;
}

void VirtualCommunicatorAggregator::ResetSendDelayInfo()
{
    sendDelayTime_ = 0;
    delayMessageId_ = INVALID_MESSAGE_ID;
    delayTimes_ = 0;
    skipTimes_ = 0;
    delayDevices_.clear();
}

void VirtualCommunicatorAggregator::DelayTimeHandle(uint32_t messageId, const std::string &dstTarget)
{
    if ((skipTimes_ == 0) && delayTimes_ > 0 && (messageId == delayMessageId_) &&
        (delayDevices_.count(dstTarget) > 0)) {
        delayTimes_--;
    }
    if (skipTimes_ > 0 && (messageId == delayMessageId_) && (delayDevices_.count(dstTarget) > 0)) {
        skipTimes_--;
    }
}
} // namespace DistributedDB
