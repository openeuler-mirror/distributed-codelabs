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

#ifndef DSLM_IPC_PROCESS_H
#define DSLM_IPC_PROCESS_H

#include <cstdint>
#include <functional>
#include <map>
#include <mutex>

#include "iremote_object.h"
#include "message_parcel.h"
#include "refbase.h"
#include "singleton.h"

#include "device_security_defines.h"

namespace OHOS {
namespace Security {
namespace DeviceSecurityLevel {
class DslmIpcProcess : public Singleton<DslmIpcProcess> {
public:
    int32_t DslmProcessGetDeviceSecurityLevel(MessageParcel &data, MessageParcel &reply);
    class RemoteHolder : public Singleton<RemoteHolder> {
    public:
        bool Push(uint32_t owner, uint32_t cookie, const sptr<IRemoteObject> &object);
        sptr<IRemoteObject> Pop(uint32_t owner, uint32_t cookie);

    private:
        std::map<uint64_t, sptr<IRemoteObject>> map_;
        std::mutex mutex_;
    };

private:
    int32_t DslmGetRequestFromParcel(MessageParcel &data, DeviceIdentify &identify, RequestOption &option,
        sptr<IRemoteObject> &object, uint32_t &cookie);

    int32_t DslmSetResponseToParcel(MessageParcel &reply, uint32_t status);
};
} // namespace DeviceSecurityLevel
} // namespace Security
} // namespace OHOS

#endif // DSLM_IPC_PROCESS_H