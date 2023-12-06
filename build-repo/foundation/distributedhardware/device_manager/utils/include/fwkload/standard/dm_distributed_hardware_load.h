/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OHOS_DM_DISTRIBUTEED_HARDWARE_LOAD_H
#define OHOS_DM_DISTRIBUTEED_HARDWARE_LOAD_H

#include "single_instance.h"
#include "system_ability_load_callback_stub.h"
namespace OHOS { class IRemoteObject; }

namespace OHOS {
namespace DistributedHardware {
class DistributedHardwareLoadCallback : public SystemAbilityLoadCallbackStub {
public:
    void OnLoadSystemAbilitySuccess(int32_t systemAbilityId, const sptr<IRemoteObject>& remoteObject) override;
    void OnLoadSystemAbilityFail(int32_t systemAbilityId) override;
};

class DmDistributedHardwareLoad {
    DECLARE_SINGLE_INSTANCE(DmDistributedHardwareLoad);
public:
    void LoadDistributedHardwareFwk(void);
    void InitDistributedHardwareLoadCount(void);
    uint32_t GetDistributedHardwareLoadCount(void);
private:
    uint32_t distributedHardwareLoadCount_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_PERMISSION_MANAGER_H
