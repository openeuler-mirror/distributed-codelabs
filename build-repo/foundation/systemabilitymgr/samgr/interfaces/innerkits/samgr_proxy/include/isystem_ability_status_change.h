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

#ifndef SAMGR_INTERFACES_INNERKITS_SAMGR_PROXY_INCLUDE_IF_SYSTEM_ABILITY_STATUS_CHANGE_H
#define SAMGR_INTERFACES_INNERKITS_SAMGR_PROXY_INCLUDE_IF_SYSTEM_ABILITY_STATUS_CHANGE_H

#include "iremote_broker.h"
#include "iremote_object.h"
#include "iremote_proxy.h"

namespace OHOS {
class ISystemAbilityStatusChange : public IRemoteBroker {
public:
    virtual ~ISystemAbilityStatusChange() = default;
    virtual void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) = 0;
    virtual void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) = 0;
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.ISystemAbilityStatusChange");
    enum {
        ON_ADD_SYSTEM_ABILITY = 1,
        ON_REMOVE_SYSTEM_ABILITY = 2,
    };
};
}
#endif /* SAMGR_INTERFACES_INNERKITS_SAMGR_PROXY_INCLUDE_IF_SYSTEM_ABILITY_STATUS_CHANGE_H */
