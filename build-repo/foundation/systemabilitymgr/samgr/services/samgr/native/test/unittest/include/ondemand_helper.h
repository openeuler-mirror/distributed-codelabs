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
#ifndef SAMGR_TEST_UNITTEST_INCLUDE_ONDEMAND_HELPER_H
#define SAMGR_TEST_UNITTEST_INCLUDE_ONDEMAND_HELPER_H

#include "refbase.h"
#include "system_ability_load_callback_stub.h"
namespace OHOS {
class OnDemandHelper {
public:
    ~OnDemandHelper() = default;
    static OnDemandHelper& GetInstance();

    int32_t OnDemandAbility(int32_t systemAbilityId);
    void GetDeviceList();
    std::string GetFirstDevice();
    int32_t LoadRemoteAbility(int32_t systemAbilityId, const std::string& deviceId,
        const sptr<ISystemAbilityLoadCallback>& callback);
    void LoadRemoteAbilityMuti(int32_t systemAbilityId, const std::string& deviceId);
    void LoadRemoteAbilityMutiSA(int32_t systemAbilityId, const std::string& deviceId);
    void LoadRemoteAbilityMutiSACb(int32_t systemAbilityId, const std::string& deviceId);
    void LoadRemoteAbilityMutiCb(int32_t systemAbilityId, const std::string& deviceId);
    void LoadRemoteAbilityPressure(int32_t systemAbilityId, const std::string& deviceId);

    sptr<IRemoteObject> GetSystemAbility(int32_t systemAbilityId);
    void OnLoadSystemAbility(int32_t systemAbilityId);
protected:
    class OnDemandLoadCallback : public SystemAbilityLoadCallbackStub {
    public:
        void OnLoadSystemAbilitySuccess(int32_t systemAbilityId, const sptr<IRemoteObject>& remoteObject) override;
        void OnLoadSystemAbilityFail(int32_t systemAbilityId) override;
        void OnLoadSACompleteForRemote(const std::string& deviceId, int32_t systemAbilityId,
            const sptr<IRemoteObject>& remoteObject) override;
    };
private:
    OnDemandHelper();
    sptr<OnDemandLoadCallback> loadCallback_;
    sptr<OnDemandLoadCallback> loadCallback2_;
    sptr<OnDemandLoadCallback> loadCallback3_;
    sptr<OnDemandLoadCallback> loadCallback4_;
};
}
#endif /* SAMGR_TEST_UNITTEST_INCLUDE_ONDEMAND_HELPER_H */
