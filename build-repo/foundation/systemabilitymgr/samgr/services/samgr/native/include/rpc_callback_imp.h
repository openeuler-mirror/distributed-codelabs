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

#ifndef SERVICES_SAMGR_NATIVE_INCLUDE_RPC_CALLBACK_IMP_H_
#define SERVICES_SAMGR_NATIVE_INCLUDE_RPC_CALLBACK_IMP_H_

#include "rpc_system_ability_callback.h"
#include "system_ability_load_callback_stub.h"

namespace OHOS {
class RpcCallbackImp : public RpcSystemAbilityCallback {
public:
    sptr<IRemoteObject> GetSystemAbilityFromRemote(int32_t systemAbilityId) override;
    bool LoadSystemAbilityFromRemote(const std::string& srcNetworkId, int32_t systemAbilityId) override;
    RpcCallbackImp() = default;
    ~RpcCallbackImp() override = default;
protected:
    class LoadCallbackImp : public SystemAbilityLoadCallbackStub {
    public:
        void OnLoadSystemAbilitySuccess(int32_t systemAbilityId, const sptr<IRemoteObject>& remoteObject) override;
        void OnLoadSystemAbilityFail(int32_t systemAbilityId) override;
        explicit LoadCallbackImp(const std::string& srcNetWorkId) : srcNetWorkId_(srcNetWorkId) {}
        ~LoadCallbackImp() override = default;
    private:
        std::string srcNetWorkId_;
    };
};
} // namespace OHOS

#endif // !defined(SERVICES_SAMGR_NATIVE_INCLUDE_ABILITY_DEATH_RECIPIENT_H_)
