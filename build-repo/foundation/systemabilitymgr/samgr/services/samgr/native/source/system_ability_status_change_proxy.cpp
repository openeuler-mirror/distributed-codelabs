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

#include "system_ability_status_change_proxy.h"

#include "ipc_types.h"
#include "iremote_object.h"
#include "message_option.h"
#include "message_parcel.h"
#include "refbase.h"
#include "sam_log.h"

namespace OHOS {
void SystemAbilityStatusChangeProxy::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    bool ret = SendRequestInner(ON_ADD_SYSTEM_ABILITY, systemAbilityId, deviceId);
    if (!ret) {
        HILOGE("OnAddSystemAbility SendRequest failed!");
    }
}

void SystemAbilityStatusChangeProxy::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    bool ret = SendRequestInner(ON_REMOVE_SYSTEM_ABILITY, systemAbilityId, deviceId);
    if (!ret) {
        HILOGE("OnRemoveSystemAbility SendRequest failed!");
    }
}

bool SystemAbilityStatusChangeProxy::SendRequestInner(uint32_t code, int32_t systemAbilityId,
    const std::string& deviceId)
{
    HILOGD("SystemAbilityStatusChangeProxy::SendRequestInner enter, systemAbilityId : %{public}d, code : %{public}u",
        systemAbilityId, code);
    if (systemAbilityId <= 0) {
        HILOGE("SendRequestInner saId:%{public}d is invalid!", systemAbilityId);
        return false;
    }

    sptr<IRemoteObject> iro = Remote();
    if (iro == nullptr) {
        HILOGE("SendRequestInner Remote return null");
        return false;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        HILOGE("SendRequestInner interface token check failed");
        return false;
    }
    bool ret = data.WriteInt32(systemAbilityId);
    if (!ret) {
        HILOGE("SendRequestInner write saId failed!");
        return false;
    }
    ret = data.WriteString(deviceId);
    if (!ret) {
        HILOGE("SendRequestInner write deviceId failed!");
        return false;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int32_t status = iro->SendRequest(code, data, reply, option);
    if (status != NO_ERROR) {
        HILOGE("SendRequestInner SendRequest failed, return value : %{public}d", status);
        return false;
    }
    return true;
}
}
