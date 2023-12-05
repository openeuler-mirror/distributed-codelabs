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

#ifndef SAMGR_SERVICES_SAMGR_NATIVE_INCLUDE_SYSTEM_ABILITY_LOAD_CALLBACK_PROXY_H
#define SAMGR_SERVICES_SAMGR_NATIVE_INCLUDE_SYSTEM_ABILITY_LOAD_CALLBACK_PROXY_H

#include "isystem_ability_load_callback.h"

namespace OHOS {
class SystemAbilityLoadCallbackProxy : public IRemoteProxy<ISystemAbilityLoadCallback> {
public:
    explicit SystemAbilityLoadCallbackProxy(const sptr<IRemoteObject>& impl)
        : IRemoteProxy<ISystemAbilityLoadCallback>(impl) {}
    ~SystemAbilityLoadCallbackProxy() = default;

    void OnLoadSystemAbilitySuccess(int32_t systemAbilityId, const sptr<IRemoteObject>& remoteObject) override;
    void OnLoadSystemAbilityFail(int32_t systemAbilityId) override;
    void OnLoadSACompleteForRemote(const std::string& deviceId,
        int32_t systemAbilityId, const sptr<IRemoteObject>& remoteObject) override;
private:
    static inline BrokerDelegator<SystemAbilityLoadCallbackProxy> delegator_;
};
}
#endif /* SAMGR_SERVICES_SAMGR_NATIVE_INCLUDE_SYSTEM_ABILITY_LOAD_CALLBACK_PROXY_H */
