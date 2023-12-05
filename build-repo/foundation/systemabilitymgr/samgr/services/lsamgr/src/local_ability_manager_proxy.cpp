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

#include "local_ability_manager_proxy.h"

#include "ipc_types.h"
#include "iremote_object.h"
#include "message_option.h"
#include "message_parcel.h"
#include "refbase.h"

using namespace std;
using namespace OHOS::HiviewDFX;

namespace OHOS {
bool LocalAbilityManagerProxy::StartAbility(int32_t systemAbilityId)
{
    if (systemAbilityId <= 0) {
        HiLog::Warn(label_, "StartAbility systemAbilityId invalid.");
        return false;
    }

    sptr<IRemoteObject> iro = Remote();
    if (iro == nullptr) {
        HiLog::Error(label_, "StartAbility Remote return null");
        return false;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(LOCAL_ABILITY_MANAGER_INTERFACE_TOKEN)) {
        HiLog::Warn(label_, "StartAbility interface token check failed");
        return false;
    }
    bool ret = data.WriteInt32(systemAbilityId);
    if (!ret) {
        HiLog::Warn(label_, "StartAbility write systemAbilityId failed!");
        return false;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int32_t status = iro->SendRequest(START_ABILITY_TRANSACTION, data, reply, option);
    if (status != NO_ERROR) {
        HiLog::Error(label_, "StartAbility SendRequest failed, return value : %{public}d", status);
        return false;
    }
    return true;
}
}
