/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "listen_ability_stub.h"

#include "errors.h"
#include "ipc_object_stub.h"
#include "ipc_types.h"
#include "message_parcel.h"

namespace OHOS {
int32_t ListenAbilityStub::OnRemoteRequest(uint32_t code,
    MessageParcel& data, MessageParcel &reply, MessageOption &option)
{
    std::u16string interfaceToken = data.ReadInterfaceToken();
    if (interfaceToken != GetDescriptor()) {
        return ERR_PERMISSION_DENIED;
    }
    switch (code) {
        case ADD_VOLUME: {
            int32_t volume = data.ReadInt32();
            bool ret = reply.WriteInt32(AddVolume(volume));
            return (ret ? ERR_OK : ERR_FLATTEN_OBJECT);
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}
}
