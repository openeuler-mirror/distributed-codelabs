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

#include "device_security_level_loader.h"

#include <future>
#include <type_traits>

#include "hilog/log_cpp.h"
#include "if_system_ability_manager.h"
#include "isystem_ability_load_callback.h"
#include "iservice_registry.h"

#include "device_security_level_defines.h"
#include "idevice_security_level.h"

namespace OHOS {
namespace Security {
namespace DeviceSecurityLevel {
using namespace OHOS::HiviewDFX;

sptr<IRemoteObject> DeviceSecurityLevelLoader::LoadDslmService()
{
    auto registry = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (registry == nullptr) {
        HiLog::Error(LABEL, "GetDeviceSecurityInfo get registry error.");
        return {};
    }
    auto object = registry->GetSystemAbility(DEVICE_SECURITY_LEVEL_MANAGER_SA_ID);
    if (object != nullptr) {
        return object;
    }

    sptr<LoadCallback> callback = new (std::nothrow) LoadCallback();
    int32_t result = registry->LoadSystemAbility(DEVICE_SECURITY_LEVEL_MANAGER_SA_ID, callback);
    if (result != ERR_OK) {
        HiLog::Error(LABEL, "GetDeviceSecurityInfo LoadSystemAbility error.");
        return {};
    }
    return callback->Promise();
}

void DeviceSecurityLevelLoader::LoadCallback::OnLoadSystemAbilitySuccess(int32_t sid, const sptr<IRemoteObject> &object)
{
    HiLog::Info(LABEL, "OnLoadSystemAbilitySuccess = %{public}d.", sid);
    promise_.set_value(object);
}

void DeviceSecurityLevelLoader::LoadCallback::OnLoadSystemAbilityFail(int32_t sid)
{
    HiLog::Error(LABEL, "OnLoadSystemAbilityFail = %{public}d.", sid);
}

sptr<IRemoteObject> DeviceSecurityLevelLoader::LoadCallback::Promise()
{
    return promise_.get_future().get();
}
} // namespace DeviceSecurityLevel
} // namespace Security
} // namespace OHOS
