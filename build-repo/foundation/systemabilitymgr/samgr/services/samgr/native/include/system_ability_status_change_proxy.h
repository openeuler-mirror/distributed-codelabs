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

#ifndef SAMGR_SERVICES_SAMGR_NATIVE_INCLUDE_SYSTEM_ABILITY_STATUS_CHANGE_PROXY_H_
#define SAMGR_SERVICES_SAMGR_NATIVE_INCLUDE_SYSTEM_ABILITY_STATUS_CHANGE_PROXY_H_

#include <string>
#include "isystem_ability_status_change.h"

namespace OHOS {
class SystemAbilityStatusChangeProxy : public IRemoteProxy<ISystemAbilityStatusChange> {
public:
    explicit SystemAbilityStatusChangeProxy(const sptr<IRemoteObject>& impl)
        : IRemoteProxy<ISystemAbilityStatusChange>(impl) {}
    ~SystemAbilityStatusChangeProxy() = default;

    void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId = "") override;
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId = "") override;
private:
    bool SendRequestInner(uint32_t code, int32_t systemAbilityId, const std::string& deviceId);
    static inline BrokerDelegator<SystemAbilityStatusChangeProxy> delegator_;
};
}
#endif /* SAMGR_SERVICES_SAMGR_NATIVE_INCLUDE_SYSTEM_ABILITY_STATUS_CHANGE_PROXY_H_ */
