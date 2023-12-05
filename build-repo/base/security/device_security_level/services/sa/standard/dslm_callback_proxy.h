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

#ifndef DSLM_CALLBACK_PROXY
#define DSLM_CALLBACK_PROXY

#include <cstdint>
#include <functional>

#include "iremote_broker.h"
#include "iremote_object.h"
#include "iremote_proxy.h"
#include "nocopyable.h"
#include "refbase.h"

#include "idevice_security_level.h"

namespace OHOS {
namespace Security {
namespace DeviceSecurityLevel {
using namespace OHOS;
class DslmCallbackProxy : public IRemoteProxy<IDeviceSecurityLevelCallback>, public NoCopyable {
public:
    explicit DslmCallbackProxy(const sptr<IRemoteObject> &impl);
    ~DslmCallbackProxy() override = default;

    struct ResponseInfo {
        uint32_t result {0};
        uint32_t level {0};
        const uint8_t *extraBuff {nullptr};
        uint32_t extraLen {0};
    };

    int32_t ResponseDeviceSecurityLevel(uint32_t cookie, const ResponseInfo &response);

private:
    static inline BrokerDelegator<DslmCallbackProxy> delegator_;
};
} // namespace DeviceSecurityLevel
} // namespace Security
} // namespace OHOS

#endif // DSLM_CALLBACK_PROXY