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

#ifndef I_DEVICE_SECURITY_LEVEL_H
#define I_DEVICE_SECURITY_LEVEL_H

#include <cstdint>
#include <string.h>

#include "iremote_broker.h"
#include "message_parcel.h"

namespace OHOS {
namespace Security {
namespace DeviceSecurityLevel {
constexpr int32_t DEVICE_SECURITY_LEVEL_MANAGER_SA_ID = 3511;

class IDeviceSecurityLevel : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.Security.DeviceSecurityLevel");
    enum {
        CMD_GET_DEVICE_SECURITY_LEVEL = 1,
    };
};

class IDeviceSecurityLevelCallback : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.Security.DeviceSecurityLevel.Callback");
    enum {
        CMD_SET_DEVICE_SECURITY_LEVEL = 1,
    };
};
} // namespace DeviceSecurityLevel
} // namespace Security
} // namespace OHOS

#endif // I_DEVICE_SECURITY_LEVEL_H
