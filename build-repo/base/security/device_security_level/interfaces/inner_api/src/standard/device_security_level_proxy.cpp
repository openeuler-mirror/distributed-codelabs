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

#include "device_security_level_proxy.h"

#include <cstdint>
#include <functional>
#include <new>
#include <type_traits>

#include "hilog/log_cpp.h"
#include "ipc_types.h"
#include "iremote_broker.h"
#include "iremote_object.h"
#include "message_option.h"
#include "message_parcel.h"
#include "refbase.h"
#include "singleton.h"

#include "device_security_defines.h"
#include "device_security_level_defines.h"
#include "idevice_security_level.h"
namespace OHOS {
namespace Security {
namespace DeviceSecurityLevel {
using namespace OHOS::HiviewDFX;
DeviceSecurityLevelProxy::DeviceSecurityLevelProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IDeviceSecurityLevel>(impl)
{
}

int32_t DeviceSecurityLevelProxy::RequestDeviceSecurityLevel(const DeviceIdentify &identify,
    const RequestOption &option, const sptr<IRemoteObject> &callback, uint64_t cookie)
{
    MessageParcel data;
    MessageParcel reply;

    auto length = identify.length;
    if (length == 0 || length > DEVICE_ID_MAX_LEN) {
        HiLog::Error(LABEL, "RequestDeviceSecurityLevel invalid para len.");
        return ERR_INVALID_LEN_PARA;
    }

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        HiLog::Error(LABEL, "RequestDeviceSecurityLevel write descriptor failed");
        return ERR_INVALID_PARA;
    }

    /* DeviceIdentify */
    data.WriteUint32(length);
    data.WriteBuffer(identify.identity, DEVICE_ID_MAX_LEN);
    /* option */
    data.WriteUint64(option.challenge);
    data.WriteUint32(option.timeout);
    data.WriteUint32(option.extra);

    /* callback */
    data.WriteRemoteObject(callback);
    /* cookie */
    data.WriteUint32(cookie);

    MessageOption ipcOption = {MessageOption::TF_SYNC};
    auto result = Remote()->SendRequest(CMD_GET_DEVICE_SECURITY_LEVEL, data, reply, ipcOption);
    if (result != ERR_NONE) {
        HiLog::Error(LABEL, "RequestDeviceSecurityLevelSendRequest send failed, ret is %{public}d", result);
        return result;
    }

    if (reply.GetReadableBytes() < sizeof(uint32_t)) {
        HiLog::Error(LABEL, "RequestDeviceSecurityLevelSendRequest result length error");
        return ERR_IPC_RET_PARCEL_ERR;
    }

    auto status = reply.ReadUint32();
    if (status != cookie) {
        HiLog::Error(LABEL, "RequestDeviceSecurityLevelSendRequest result value error, ret is %{public}u", status);
        return ERR_IPC_REMOTE_OBJ_ERR;
    }

    return SUCCESS;
}
} // namespace DeviceSecurityLevel
} // namespace Security
} // namespace OHOS
