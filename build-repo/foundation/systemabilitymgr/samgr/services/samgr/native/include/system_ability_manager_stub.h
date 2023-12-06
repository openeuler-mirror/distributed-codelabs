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

#ifndef SERVICES_SAMGR_NATIVE_INCLUDE_SYSTEM_ABILITY_MANAGER_STUB_H_
#define SERVICES_SAMGR_NATIVE_INCLUDE_SYSTEM_ABILITY_MANAGER_STUB_H_

#include <map>
#include "if_system_ability_manager.h"
#include "iremote_object.h"
#include "iremote_stub.h"

namespace OHOS {
class SystemAbilityManagerStub : public IRemoteStub<ISystemAbilityManager> {
public:
    SystemAbilityManagerStub();
    ~SystemAbilityManagerStub() = default;
    int32_t OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption &option) override;

protected:
    static bool CanRequest();
    static bool EnforceInterceToken(MessageParcel& data);

private:
    int32_t ListSystemAbilityInner(MessageParcel& data, MessageParcel& reply);
    int32_t SubsSystemAbilityInner(MessageParcel& data, MessageParcel& reply);
    int32_t UnSubsSystemAbilityInner(MessageParcel& data, MessageParcel& reply);
    int32_t CheckRemtSystemAbilityInner(MessageParcel& data, MessageParcel& reply);
    int32_t AddOndemandSystemAbilityInner(MessageParcel& data, MessageParcel& reply);
    int32_t CheckSystemAbilityImmeInner(MessageParcel& data, MessageParcel& reply);
    int32_t AddSystemAbilityInner(MessageParcel& data, MessageParcel& reply);
    int32_t GetSystemAbilityInner(MessageParcel& data, MessageParcel& reply);
    int32_t CheckSystemAbilityInner(MessageParcel& data, MessageParcel& reply);
    int32_t AddSystemProcessInner(MessageParcel& data, MessageParcel& reply);
    int32_t RemoveSystemAbilityInner(MessageParcel& data, MessageParcel& reply);
    int32_t LoadSystemAbilityInner(MessageParcel& data, MessageParcel& reply);
    int32_t LoadRemoteSystemAbilityInner(MessageParcel& data, MessageParcel& reply);
    int32_t UnmarshalingSaExtraProp(MessageParcel& data, SAExtraProp& extraProp);
    static int32_t GetHapIdMultiuser(int32_t uid);

    using SystemAbilityManagerStubFunc =
        int32_t (SystemAbilityManagerStub::*)(MessageParcel& data, MessageParcel& reply);
    std::map<uint32_t, SystemAbilityManagerStubFunc> memberFuncMap_;
};
} // namespace OHOS

#endif // !defined(SERVICES_SAMGR_NATIVE_INCLUDE_SYSTEM_ABILITY_MANAGER_STUB_H_)
