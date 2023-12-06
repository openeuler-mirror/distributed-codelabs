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

#include "ability_death_recipient.h"

#include "iremote_proxy.h"
#include "sam_log.h"
#include "system_ability_manager.h"

namespace OHOS {
void AbilityDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    HILOGI("AbilityDeathRecipient OnRemoteDied called");
    SystemAbilityManager::GetInstance()->RemoveSystemAbility(remote.promote());
    HILOGD("AbilityDeathRecipients death notice success");
}

void SystemProcessDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    HILOGI("SystemProcessDeathRecipient called!");
    SystemAbilityManager::GetInstance()->RemoveSystemProcess(remote.promote());
    HILOGD("SystemProcessDeathRecipient death notice success");
}

void AbilityStatusDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    HILOGI("AbilityStatusDeathRecipient called!");
    SystemAbilityManager::GetInstance()->UnSubscribeSystemAbility(remote.promote());
    HILOGD("AbilityStatusDeathRecipient death notice success");
}

void AbilityCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    HILOGI("AbilityCallbackDeathRecipient called!");
    SystemAbilityManager::GetInstance()->OnAbilityCallbackDied(remote.promote());
    HILOGD("AbilityCallbackDeathRecipient death notice success");
}

void RemoteCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    HILOGI("RemoteCallbackDeathRecipient called!");
    SystemAbilityManager::GetInstance()->OnRemoteCallbackDied(remote.promote());
    HILOGD("RemoteCallbackDeathRecipient death notice success");
}
} // namespace OHOS
