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

#ifndef DEVICE_SECURITY_LEVEL_CALLBACK_STUB
#define DEVICE_SECURITY_LEVEL_CALLBACK_STUB

#include <cstdint>
#include <functional>

#include "errors.h"
#include "iremote_stub.h"
#include "message_option.h"
#include "message_parcel.h"
#include "nocopyable.h"
#include "refbase.h"

#include "idevice_security_level.h"

namespace OHOS {
namespace Security {
namespace DeviceSecurityLevel {
using namespace OHOS;
class DeviceSecurityLevelCallbackStub : public IRemoteStub<IDeviceSecurityLevelCallback>, public NoCopyable {
    using RemoteRequest =
        std::function<int32_t(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)>;

public:
    explicit DeviceSecurityLevelCallbackStub(RemoteRequest request);
    ~DeviceSecurityLevelCallbackStub() override = default;

    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    RemoteRequest remoteRequest_;
};
} // namespace DeviceSecurityLevel
} // namespace Security
} // namespace OHOS

#endif // DEVICE_SECURITY_LEVEL_CALLBACK_STUB