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

#include "common_event_manager_service_ability.h"

#include "event_log_wrapper.h"

namespace OHOS {
namespace EventFwk {
namespace {
REGISTER_SYSTEM_ABILITY_BY_ID(CommonEventManagerServiceAbility, COMMON_EVENT_SERVICE_ID, true);
}

CommonEventManagerServiceAbility::CommonEventManagerServiceAbility(const int32_t systemAbilityId, bool runOnCreate)
    : SystemAbility(systemAbilityId, runOnCreate), service_(nullptr)
{}

CommonEventManagerServiceAbility::~CommonEventManagerServiceAbility()
{}

void CommonEventManagerServiceAbility::OnStart()
{
    EVENT_LOGD("OnStart called.");
    if (service_ != nullptr) {
        EVENT_LOGD("The CommonEventManagerService has existed.");
        return;
    }

    service_ = DelayedSingleton<CommonEventManagerService>::GetInstance();
    ErrCode errorCode = service_->Init();
    if (errorCode != ERR_OK) {
        EVENT_LOGE("Failed to init the commonEventManagerService instance.");
        return;
    }
    
    if (!Publish(service_.get())) {
        EVENT_LOGE("Failed to publish CommonEventManagerService to SystemAbilityMgr");
        return;
    }
}

void CommonEventManagerServiceAbility::OnStop()
{
    EVENT_LOGD("onStop called.");
    service_ = nullptr;
}
}  // namespace EventFwk
}  // namespace OHOS