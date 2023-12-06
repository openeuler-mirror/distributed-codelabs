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


#ifndef INTERFACES_INNERKITS_SAMGR_INCLUDE_SYSTEM_ABILITY_MANAGER_PROXY_H_
#define INTERFACES_INNERKITS_SAMGR_INCLUDE_SYSTEM_ABILITY_MANAGER_PROXY_H_

#include <string>
#include "if_system_ability_manager.h"

namespace OHOS {
class SystemAbilityManagerProxy : public IRemoteProxy<ISystemAbilityManager> {
public:
    explicit SystemAbilityManagerProxy(const sptr<IRemoteObject>& impl)
        : IRemoteProxy<ISystemAbilityManager>(impl) {}
    ~SystemAbilityManagerProxy() = default;
    std::vector<std::u16string> ListSystemAbilities(unsigned int dumpFlags) override;

    // IntToString adapter interface
    sptr<IRemoteObject> GetSystemAbility(int32_t systemAbilityId) override;
    sptr<IRemoteObject> CheckSystemAbility(int32_t systemAbilityId) override;
    int32_t RemoveSystemAbility(int32_t systemAbilityId) override;
    int32_t SubscribeSystemAbility(int32_t systemAbilityId, const sptr<ISystemAbilityStatusChange>& listener) override;
    int32_t UnSubscribeSystemAbility(int32_t systemAbilityId,
        const sptr<ISystemAbilityStatusChange> &listener) override;
    sptr<IRemoteObject> GetSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    sptr<IRemoteObject> CheckSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    int32_t AddOnDemandSystemAbilityInfo(int32_t systemAbilityId,
        const std::u16string& localAbilityManagerName) override;
    sptr<IRemoteObject> CheckSystemAbility(int32_t systemAbilityId, bool& isExist) override;
    int32_t AddSystemAbility(int32_t systemAbilityId, const sptr<IRemoteObject>& ability,
        const SAExtraProp& extraProp) override;

    int32_t AddSystemProcess(const std::u16string& procName, const sptr<IRemoteObject>& procObject) override;
    int32_t LoadSystemAbility(int32_t systemAbilityId, const sptr<ISystemAbilityLoadCallback>& callback) override;
    int32_t LoadSystemAbility(int32_t systemAbilityId, const std::string& deviceId,
        const sptr<ISystemAbilityLoadCallback>& callback) override;
private:
    sptr<IRemoteObject> GetSystemAbilityWrapper(int32_t systemAbilityId, const std::string& deviceId = "");
    sptr<IRemoteObject> CheckSystemAbilityWrapper(int32_t code, MessageParcel& data);
    int32_t MarshalSAExtraProp(const SAExtraProp& extraProp, MessageParcel& data) const;
    int32_t AddSystemAbilityWrapper(int32_t code, MessageParcel& data);
    int32_t RemoveSystemAbilityWrapper(int32_t code, MessageParcel& data);
private:
    static inline BrokerDelegator<SystemAbilityManagerProxy> delegator_;
};
} // namespace OHOS

#endif // !defined(INTERFACES_INNERKITS_SAMGR_INCLUDE_SYSTEM_ABILITY_MANAGER_PROXY_H_)
