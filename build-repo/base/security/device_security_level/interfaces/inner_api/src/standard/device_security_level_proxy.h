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

#ifndef DEVICE_SECURITY_LEVEL_PROXY
#define DEVICE_SECURITY_LEVEL_PROXY

#include <cstdint>
#include <functional>

#include "errors.h"
#include "iremote_broker.h"
#include "iremote_object.h"
#include "iremote_proxy.h"
#include "nocopyable.h"
#include "refbase.h"

#include "device_security_defines.h"
#include "idevice_security_level.h"

namespace OHOS {
namespace Security {
namespace DeviceSecurityLevel {
using namespace OHOS;
class DeviceSecurityLevelProxy : public IRemoteProxy<IDeviceSecurityLevel>, public NoCopyable {
public:
    explicit DeviceSecurityLevelProxy(const sptr<IRemoteObject> &impl);
    ~DeviceSecurityLevelProxy() override = default;
    int32_t RequestDeviceSecurityLevel(const DeviceIdentify &identify, const RequestOption &option,
        const sptr<IRemoteObject> &callback, uint64_t cookie);

private:
    static inline BrokerDelegator<DeviceSecurityLevelProxy> delegator_;
};
} // namespace DeviceSecurityLevel
} // namespace Security
} // namespace OHOS

#endif // DEVICE_SECURITY_LEVEL_PROXY