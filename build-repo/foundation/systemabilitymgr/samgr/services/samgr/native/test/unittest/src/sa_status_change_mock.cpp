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

#include "sa_status_change_mock.h"

#include "sam_log.h"

namespace OHOS {
void SaStatusChangeMock::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    HILOGI("OnAddSystemAbility systemAbilityId:%{public}d added!", systemAbilityId);
}

void SaStatusChangeMock::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    HILOGI("OnRemoveSystemAbility systemAbilityId:%{public}d removed!", systemAbilityId);
}

void SystemAbilityLoadCallbackMock::OnLoadSystemAbilitySuccess(int32_t systemAbilityId,
    const sptr<IRemoteObject>& remoteObject)
{
    HILOGI("OnLoadSystemAbilitySuccess systemAbilityId:%{public}d loaded!", systemAbilityId);
    currSystemAbilityId = systemAbilityId;
    currRemoteObject = remoteObject;
}

void SystemAbilityLoadCallbackMock::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    HILOGI("OnLoadSystemAbilityFail systemAbilityId:%{public}d load fail!", systemAbilityId);
    currSystemAbilityId = systemAbilityId;
}
}
