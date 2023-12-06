/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "system_ability_status_change_stub.h"

#include "errors.h"
#include "ipc_object_stub.h"
#include "ipc_types.h"
#include "message_option.h"
#include "message_parcel.h"
#include "refbase.h"
#include "sam_log.h"
#include "system_ability_definition.h"

namespace OHOS {
SystemAbilityStatusChangeStub::SystemAbilityStatusChangeStub()
{
    memberFuncMap_[ON_ADD_SYSTEM_ABILITY] =
        &SystemAbilityStatusChangeStub::OnAddSystemAbilityInner;
    memberFuncMap_[ON_REMOVE_SYSTEM_ABILITY] =
        &SystemAbilityStatusChangeStub::OnRemoveSystemAbilityInner;
}

int32_t SystemAbilityStatusChangeStub::OnRemoteRequest(uint32_t code,
    MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    HILOGI("SystemAbilityStatusChangeStub::code:%{public}u, flags:%{public}d", code, option.GetFlags());
    if (!EnforceInterceToken(data)) {
        HILOGW("check interface token failed!");
        return ERR_PERMISSION_DENIED;
    }
    auto iter = memberFuncMap_.find(code);
    if (iter != memberFuncMap_.end()) {
        auto memberFunc = iter->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data, reply);
        }
    }
    HILOGW("unknown request code!");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t SystemAbilityStatusChangeStub::OnAddSystemAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    int32_t systemAbilityId = data.ReadInt32();
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("read systemAbilityId failed!");
        return ERR_NULL_OBJECT;
    }
    std::string deviceId = data.ReadString();
    OnAddSystemAbility(systemAbilityId, deviceId);
    return ERR_NONE;
}

int32_t SystemAbilityStatusChangeStub::OnRemoveSystemAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    int32_t systemAbilityId = data.ReadInt32();
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("read systemAbilityId failed!");
        return ERR_NULL_OBJECT;
    }
    std::string deviceId = data.ReadString();
    OnRemoveSystemAbility(systemAbilityId, deviceId);
    return ERR_NONE;
}

bool SystemAbilityStatusChangeStub::CheckInputSysAbilityId(int32_t systemAbilityId)
{
    return (systemAbilityId >= FIRST_SYS_ABILITY_ID) && (systemAbilityId <= LAST_SYS_ABILITY_ID);
}

bool SystemAbilityStatusChangeStub::EnforceInterceToken(MessageParcel& data)
{
    std::u16string interfaceToken = data.ReadInterfaceToken();
    return interfaceToken == GetDescriptor();
}
}
