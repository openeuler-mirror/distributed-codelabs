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

#include "dslm_fuzzer.h"

#include "securec.h"
#include "parcel.h"

#include "device_security_defines.h"
#include "utils_log.h"
#include "device_security_level_callback_stub.h"
#include "dslm_service.h"

extern "C" int32_t OnPeerMsgReceived(const DeviceIdentify *devId, const uint8_t *msg, uint32_t len);

namespace OHOS {
namespace Security {
namespace DeviceSecurityLevel {
namespace {
const uint8_t mockBuffer[DEVICE_ID_MAX_LEN] = {0};

DslmService g_dslmService(DEVICE_SECURITY_LEVEL_MANAGER_SA_ID, true);

void OnPeerMsgReceivedFuzzer(Parcel &parcel)
{
    SECURITY_LOG_INFO("begin");
    DeviceIdentify deviceIdentify = {};
    deviceIdentify.length = parcel.ReadUint32();
    const uint8_t *buffer = parcel.ReadBuffer(DEVICE_ID_MAX_LEN);
    if (buffer != nullptr) {
        (void)memcpy_s(deviceIdentify.identity, DEVICE_ID_MAX_LEN, buffer, DEVICE_ID_MAX_LEN);
    }
    OnPeerMsgReceived(&deviceIdentify,
        reinterpret_cast<const uint8_t *>(parcel.GetData() + parcel.GetReadPosition()), parcel.GetReadableBytes());
    SECURITY_LOG_INFO("end");
}

void OnRemoteRequestFuzzer(Parcel &parcel)
{
    SECURITY_LOG_INFO("begin");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(IDeviceSecurityLevel::GetDescriptor());

    /* DeviceIdentify */
    data.WriteUint32(parcel.ReadUint32());
    const uint8_t *buffer = parcel.ReadBuffer(DEVICE_ID_MAX_LEN);
    if (buffer == nullptr) {
        data.WriteBuffer(mockBuffer, DEVICE_ID_MAX_LEN);
    } else {
        data.WriteBuffer(buffer, DEVICE_ID_MAX_LEN);
    }

    /* option */
    data.WriteUint64(parcel.ReadUint64());
    data.WriteUint32(parcel.ReadUint32());
    data.WriteUint32(parcel.ReadUint32());

    sptr<IRemoteObject> callback = new (std::nothrow) DeviceSecurityLevelCallbackStub(
        [](uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) {
            SECURITY_LOG_INFO("DeviceSecurityLevelCallbackStub called");
            return 0;
        });
    /* callback */
    data.WriteRemoteObject(callback);
    /* cookie */
    data.WriteUint32(parcel.ReadUint32());

    g_dslmService.OnRemoteRequest(parcel.ReadUint32(), data, reply, option);
    SECURITY_LOG_INFO("end");
}

void DslmFuzzTest(const uint8_t *data, size_t size)
{
    Parcel parcel;
    parcel.WriteBuffer(data, size);
    parcel.RewindRead(0);
    if (parcel.ReadBool()) {
        OnPeerMsgReceivedFuzzer(parcel);
    } else {
        OnRemoteRequestFuzzer(parcel);
    }
}
}
} // namespace DeviceSecurityLevel
} // namespace Security
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int32_t LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::Security::DeviceSecurityLevel::DslmFuzzTest(data, size);
    return 0;
}
