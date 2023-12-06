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

#include "dslm_ipc_process.h"

#include <map>
#include <utility>

#include "ipc_skeleton.h"
#include "iremote_broker.h"
#include "securec.h"

#include "dslm_callback_info.h"
#include "dslm_callback_proxy.h"
#include "dslm_core_process.h"
#include "utils_log.h"

namespace {
constexpr uint32_t DFT_TIMEOUT = 45;
constexpr uint32_t MAX_TIMEOUT = 60;
constexpr uint32_t MIN_TIMEOUT = 1;
constexpr uint32_t WARNING_GATE = 64;
constexpr uint32_t COOKIE_SHIFT = 32;
} // namespace

namespace OHOS {
namespace Security {
namespace DeviceSecurityLevel {
static void ProcessCallback(uint32_t owner, uint32_t cookie, uint32_t result, const DslmCallbackInfo *info)
{
    if ((cookie == 0) || (info == nullptr)) {
        return;
    }
    auto object = Singleton<DslmIpcProcess::RemoteHolder>::GetInstance().Pop(owner, cookie);
    if (object == nullptr) {
        SECURITY_LOG_ERROR("Pop failed");
        return;
    }

    auto proxy = iface_cast<DslmCallbackProxy>(object);
    if (proxy == nullptr) {
        SECURITY_LOG_ERROR("iface_cast failed");
        return;
    }
    DslmCallbackProxy::ResponseInfo resInfo = {result, info->level, info->extraBuff, info->extraLen};
    proxy->ResponseDeviceSecurityLevel(cookie, resInfo);
}

int32_t DslmIpcProcess::DslmGetRequestFromParcel(MessageParcel &data, DeviceIdentify &identify, RequestOption &option,
    sptr<IRemoteObject> &object, uint32_t &cookie)
{
    uint32_t expected = sizeof(DeviceIdentify) + sizeof(RequestOption) + sizeof(uint32_t);
    uint32_t actual = data.GetReadableBytes();
    if (expected >= actual) {
        SECURITY_LOG_ERROR("unexpected input, length error");
        return ERR_INVALID_PARA;
    }

    identify.length = data.ReadUint32();

    const uint8_t *dataRead = data.ReadBuffer(DEVICE_ID_MAX_LEN);
    if (dataRead == nullptr) {
        SECURITY_LOG_ERROR("unexpected input, buffer error");
        return ERR_INVALID_PARA;
    }
    if (memcpy_s(identify.identity, DEVICE_ID_MAX_LEN, dataRead, DEVICE_ID_MAX_LEN) != EOK) {
        SECURITY_LOG_ERROR("unexpected input, buffer copy error");
        return ERR_INVALID_PARA;
    }

    option.challenge = data.ReadUint64();
    option.timeout = data.ReadUint32();
    if (option.timeout < MIN_TIMEOUT || option.timeout > MAX_TIMEOUT) {
        option.timeout = DFT_TIMEOUT;
    }

    option.extra = data.ReadUint32();

    object = data.ReadRemoteObject();
    if (object == nullptr) {
        SECURITY_LOG_ERROR("unexpected input, callback ipc error");
        return ERR_INVALID_PARA;
    }
    cookie = data.ReadUint32();
    if (cookie == 0) {
        SECURITY_LOG_ERROR("unexpected input, cookie error");
        return ERR_INVALID_PARA;
    }

    return SUCCESS;
}

int32_t DslmIpcProcess::DslmSetResponseToParcel(MessageParcel &reply, uint32_t status)
{
    auto success = reply.WriteUint32(status);
    if (!success) {
        return ERR_IPC_RET_PARCEL_ERR;
    }
    return SUCCESS;
}

int32_t DslmIpcProcess::DslmProcessGetDeviceSecurityLevel(MessageParcel &data, MessageParcel &reply)
{
    DeviceIdentify identity;
    RequestOption option;
    sptr<IRemoteObject> callback;
    uint32_t cookie;

    int32_t ret = DslmGetRequestFromParcel(data, identity, option, callback, cookie);
    if (ret != SUCCESS) {
        SECURITY_LOG_ERROR("DslmGetRequestFromParcel failed, ret = %{public}d", ret);
        return ret;
    }

    auto owner = IPCSkeleton::GetCallingPid();
    Singleton<RemoteHolder>::GetInstance().Push(owner, cookie, callback);

    ret = OnRequestDeviceSecLevelInfo(&identity, &option, owner, cookie, ProcessCallback);
    if (ret != SUCCESS) {
        Singleton<RemoteHolder>::GetInstance().Pop(owner, cookie);
        SECURITY_LOG_ERROR("OnRequestDeviceSecLevelInfo failed, ret = %{public}d", ret);
        return ret;
    }

    ret = DslmSetResponseToParcel(reply, cookie);
    if (ret != SUCCESS) {
        Singleton<RemoteHolder>::GetInstance().Pop(owner, cookie);
        SECURITY_LOG_ERROR("DslmSetResponseToParcel failed, ret = %{public}d", ret);
        return ret;
    }
    return SUCCESS;
}

bool DslmIpcProcess::RemoteHolder::Push(uint32_t owner, uint32_t cookie, const sptr<IRemoteObject> &object)
{
    std::lock_guard<std::mutex> lock(mutex_);
    uint64_t key = (static_cast<uint64_t>(owner) << COOKIE_SHIFT) | cookie;
    map_[key] = object;
    if (map_.size() > WARNING_GATE) {
        SECURITY_LOG_WARN("remote objects max warning");
    }
    return true;
}

sptr<IRemoteObject> DslmIpcProcess::RemoteHolder::Pop(uint32_t owner, uint32_t cookie)
{
    std::lock_guard<std::mutex> lock(mutex_);
    uint64_t key = (static_cast<uint64_t>(owner) << COOKIE_SHIFT) | cookie;
    auto iter = map_.find(key);
    if (iter == map_.end()) {
        return nullptr;
    }
    auto obj = iter->second;
    map_.erase(iter);
    return obj;
}
} // namespace DeviceSecurityLevel
} // namespace Security
} // namespace OHOS
