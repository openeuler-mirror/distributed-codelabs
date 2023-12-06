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

#ifndef SAMGR_SERVICES_SAMGR_NATIVE_TEST_UNITTEST_INCLUDE_SA_STATUS_CHANGE_MOCK_H
#define SAMGR_SERVICES_SAMGR_NATIVE_TEST_UNITTEST_INCLUDE_SA_STATUS_CHANGE_MOCK_H

#include "system_ability_status_change_stub.h"
#include "system_ability_load_callback_stub.h"

namespace OHOS {
class SaStatusChangeMock : public SystemAbilityStatusChangeStub {
public:
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
};

class SystemAbilityLoadCallbackMock : public SystemAbilityLoadCallbackStub {
public:
    void OnLoadSystemAbilitySuccess(int32_t systemAbilityId, const sptr<IRemoteObject>& remoteObject) override;

    void OnLoadSystemAbilityFail(int32_t systemAbilityId) override;

    int32_t GetSystemAbilityId() const
    {
        return currSystemAbilityId;
    }

    sptr<IRemoteObject> GetRemoteObject() const
    {
        return currRemoteObject;
    }
private:
    int32_t currSystemAbilityId = 0;
    sptr<IRemoteObject> currRemoteObject;
};
}
#endif /* SAMGR_SERVICES_SAMGR_NATIVE_TEST_UNITTEST_INCLUDE_SA_STATUS_CHANGE_MOCK_H */
