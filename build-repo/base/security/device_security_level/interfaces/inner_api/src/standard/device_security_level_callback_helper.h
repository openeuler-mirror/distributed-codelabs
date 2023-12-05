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

#ifndef DEVICE_SECURITY_LEVEL_CALLBACK_HELPER
#define DEVICE_SECURITY_LEVEL_CALLBACK_HELPER

#include <cstdint>
#include <map>
#include <mutex>

#include "message_option.h"
#include "message_parcel.h"
#include "nocopyable.h"
#include "refbase.h"
#include "singleton.h"
#include "timer.h"

#include "device_security_defines.h"
#include "device_security_level_callback_stub.h"
#include "device_security_level_defines.h"

namespace OHOS {
namespace Security {
namespace DeviceSecurityLevel {
using namespace OHOS;

class DeviceSecurityLevelCallbackHelper : public DelayedRefSingleton<DeviceSecurityLevelCallbackHelper> {
public:
    DeviceSecurityLevelCallbackHelper();
    ~DeviceSecurityLevelCallbackHelper() override;
    bool Publish(const DeviceIdentify &identity, const ResultCallback &callback, uint32_t keep,
        sptr<DeviceSecurityLevelCallbackStub> &stub, uint32_t &cookie);
    bool Withdraw(uint32_t cookie);

private:
    class CallbackInfoHolder final : public NoCopyable {
        struct CallbackInfo {
            DeviceIdentify identity;
            ResultCallback callback;
            uint64_t cookie {0};
        };

    public:
        CallbackInfoHolder();
        ~CallbackInfoHolder() override;
        bool PushCallback(const DeviceIdentify &identity, const ResultCallback &callback, uint32_t keep,
            uint32_t &cookie);
        bool PopCallback(uint32_t cookie, uint32_t result, uint32_t level);
        bool PopCallback(uint32_t cookie);

    private:
        std::map<uint32_t, CallbackInfo> map_;
        uint32_t generate_ {0};
        std::mutex mutex_;
        OHOS::Utils::Timer timer_;
    };

    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    CallbackInfoHolder holder_;
    sptr<DeviceSecurityLevelCallbackStub> stub_;
};
} // namespace DeviceSecurityLevel
} // namespace Security
} // namespace OHOS

#endif // DEVICE_SECURITY_LEVEL_CALLBACK_HELPER