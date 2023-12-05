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

#include "ability_manager_helper.h"

#include "event_log_wrapper.h"
// #include "hitrace_meter.h"
#include "iservice_registry.h"
#include "static_subscriber_connection.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace EventFwk {
int AbilityManagerHelper::ConnectAbility(
    const Want &want, const CommonEventData &event, const sptr<IRemoteObject> &callerToken, const int32_t &userId)
{
    // HITRACE_METER_NAME(HITRACE_TAG_NOTIFICATION, __PRETTY_FUNCTION__);
    EVENT_LOGI("enter, target bundle = %{public}s", want.GetBundle().c_str());
    std::lock_guard<std::mutex> lock(mutex_);

    if (!GetAbilityMgrProxy()) {
        EVENT_LOGE("failed to get ability manager proxy!");
        return -1;
    }

    sptr<StaticSubscriberConnection> connection = new (std::nothrow) StaticSubscriberConnection(event);
    if (connection == nullptr) {
        EVENT_LOGE("failed to create obj!");
        return -1;
    }
    return abilityMgr_->ConnectAbility(want, connection, callerToken, userId);
}

bool AbilityManagerHelper::GetAbilityMgrProxy()
{
    EVENT_LOGI("GetAbilityMgrProxy enter");
    if (abilityMgr_ == nullptr) {
        sptr<ISystemAbilityManager> systemAbilityManager =
            SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (systemAbilityManager == nullptr) {
            EVENT_LOGE("Failed to get system ability mgr.");
            return false;
        }

        sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
        if (remoteObject == nullptr) {
            EVENT_LOGE("Failed to get ability manager service.");
            return false;
        }

        abilityMgr_ = iface_cast<AAFwk::IAbilityManager>(remoteObject);
        if ((abilityMgr_ == nullptr) || (abilityMgr_->AsObject() == nullptr)) {
            EVENT_LOGE("Failed to get system ability manager services ability");
            return false;
        }

        deathRecipient_ = new (std::nothrow) AbilityManagerDeathRecipient();
        if (deathRecipient_ == nullptr) {
            EVENT_LOGE("Failed to create AbilityManagerDeathRecipient");
            return false;
        }
        if (!abilityMgr_->AsObject()->AddDeathRecipient(deathRecipient_)) {
            EVENT_LOGW("Failed to add AbilityManagerDeathRecipient");
        }
    }

    return true;
}

void AbilityManagerHelper::Clear()
{
    EVENT_LOGI("enter");
    std::lock_guard<std::mutex> lock(mutex_);

    if ((abilityMgr_ != nullptr) && (abilityMgr_->AsObject() != nullptr)) {
        abilityMgr_->AsObject()->RemoveDeathRecipient(deathRecipient_);
    }
    abilityMgr_ = nullptr;
}
}  // namespace EventFwk
}  // namespace OHOS
