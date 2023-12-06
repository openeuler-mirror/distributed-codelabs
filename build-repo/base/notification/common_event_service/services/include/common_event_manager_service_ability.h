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

#ifndef FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_COMMON_EVENT_MANAGER_SERVICE_ABILITY_H
#define FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_COMMON_EVENT_MANAGER_SERVICE_ABILITY_H

#include "system_ability.h"

#include "common_event_manager_service.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace EventFwk {
class CommonEventManagerServiceAbility final : public SystemAbility {
public:
    /**
     * @brief The constructor of the common event manager service ability.
     * @param systemAbilityId Indicates the system ability id.
     * @param runOnCreate Run the system ability on created.
     */
    CommonEventManagerServiceAbility(const int32_t systemAbilityId, bool runOnCreate);

    /**
     * @brief The destructor.
     */
    virtual ~CommonEventManagerServiceAbility() final;

private:
    void OnStart() final;
    void OnStop() final;

    DISALLOW_COPY_AND_MOVE(CommonEventManagerServiceAbility);
    DECLARE_SYSTEM_ABILITY(CommonEventManagerServiceAbility);

private:
    std::shared_ptr<CommonEventManagerService> service_;
};
}  // namespace EventFwk
}  // namespace OHOS

#endif // FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_COMMON_EVENT_MANAGER_SERVICE_ABILITY_H