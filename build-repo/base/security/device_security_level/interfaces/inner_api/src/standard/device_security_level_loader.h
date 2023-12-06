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

#ifndef DEVICE_SECURITY_LEVEL_LOADER
#define DEVICE_SECURITY_LEVEL_LOADER

#include <future>
#include <new>
#include <cstdint>

#include "refbase.h"
#include "iremote_object.h"
#include "singleton.h"
#include "system_ability_load_callback_stub.h"

namespace OHOS {
namespace Security {
namespace DeviceSecurityLevel {
using namespace OHOS;
class DeviceSecurityLevelLoader : public Singleton<DeviceSecurityLevelLoader> {
public:
    DeviceSecurityLevelLoader() = default;
    ~DeviceSecurityLevelLoader() override = default;
    sptr<IRemoteObject> LoadDslmService();

private:
    class LoadCallback : public SystemAbilityLoadCallbackStub {
    public:
        void OnLoadSystemAbilitySuccess(int32_t sid, const sptr<IRemoteObject> &object) override;
        void OnLoadSystemAbilityFail(int32_t sid) override;
        sptr<IRemoteObject> Promise();

    private:
        std::promise<sptr<IRemoteObject>> promise_;
    };
};
} // namespace DeviceSecurityLevel
} // namespace Security
} // namespace OHOS

#endif // DEVICE_SECURITY_LEVEL_LOADER