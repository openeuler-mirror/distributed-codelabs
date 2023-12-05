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

#include "dslm_callback_proxy.h"

#include "hilog/log_cpp.h"
#include "message_option.h"
#include "message_parcel.h"

#include "device_security_defines.h"

namespace OHOS {
namespace Security {
namespace DeviceSecurityLevel {
using namespace OHOS::HiviewDFX;
DslmCallbackProxy::DslmCallbackProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IDeviceSecurityLevelCallback>(impl)
{
}

int32_t DslmCallbackProxy::ResponseDeviceSecurityLevel(uint32_t cookie, const ResponseInfo &response)
{
    MessageParcel data;
    MessageParcel reply;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return ERR_INVALID_PARA;
    }

    data.WriteUint32(cookie);
    data.WriteUint32(response.result);
    data.WriteUint32(response.level);

    if (response.extraBuff != nullptr && response.extraLen != 0) {
        data.WriteUint32(response.extraLen);
        data.WriteBuffer(response.extraBuff, response.extraLen);
    }

    MessageOption ipcOption = {MessageOption::TF_ASYNC};
    return Remote()->SendRequest(CMD_SET_DEVICE_SECURITY_LEVEL, data, reply, ipcOption);
}
} // namespace DeviceSecurityLevel
} // namespace Security
} // namespace OHOS
