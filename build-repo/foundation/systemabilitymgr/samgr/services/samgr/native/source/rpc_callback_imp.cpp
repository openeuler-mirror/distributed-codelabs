/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "rpc_callback_imp.h"
#include "sam_log.h"
#include "system_ability_manager.h"
#include "tools.h"

namespace OHOS {
sptr<IRemoteObject> RpcCallbackImp::GetSystemAbilityFromRemote(int32_t systemAbilityId)
{
    return SystemAbilityManager::GetInstance()->GetSystemAbilityFromRemote(systemAbilityId);
}

bool RpcCallbackImp::LoadSystemAbilityFromRemote(const std::string& srcNetworkId, int32_t systemAbilityId)
{
    HILOGI("LoadSystemAbilityFromRemote! deviceId : %{public}s, said : %{public}d",
        AnonymizeDeviceId(srcNetworkId).c_str(), systemAbilityId);
    sptr<LoadCallbackImp> loadCallback = new LoadCallbackImp(srcNetworkId);
    return SystemAbilityManager::GetInstance()->LoadSystemAbilityFromRpc(srcNetworkId,
        systemAbilityId, loadCallback);
}

void RpcCallbackImp::LoadCallbackImp::OnLoadSystemAbilitySuccess(int32_t systemAbilityId,
    const sptr<IRemoteObject>& remoteObject)
{
    HILOGI("LoadCallbackImp OnLoadSystemAbilitySuccess! deviceId : %{public}s, said : %{public}d",
        AnonymizeDeviceId(srcNetWorkId_).c_str(), systemAbilityId);
    SystemAbilityManager::GetInstance()->NotifyRpcLoadCompleted(srcNetWorkId_, systemAbilityId, remoteObject);
}

void RpcCallbackImp::LoadCallbackImp::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    HILOGW("LoadCallbackImp OnLoadSystemAbilityFail! deviceId : %{public}s, said : %{public}d",
        AnonymizeDeviceId(srcNetWorkId_).c_str(), systemAbilityId);
    SystemAbilityManager::GetInstance()->NotifyRpcLoadCompleted(srcNetWorkId_, systemAbilityId, nullptr);
}
}
