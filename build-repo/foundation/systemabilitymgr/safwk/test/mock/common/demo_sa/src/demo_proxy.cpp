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

#include "demo_proxy.h"

#include "iremote_object.h"
#include "message_option.h"
#include "message_parcel.h"

namespace OHOS {
namespace {
    const int DEFAULT_INT_RET = -1;
}

int DemoProxy::AddVolume(int volume)
{
    auto remote = Remote();
    if (remote == nullptr) {
        return DEFAULT_INT_RET;
    }
    MessageParcel data;
    data.WriteInt32(volume);
    MessageParcel reply;
    MessageOption option;
    remote->SendRequest(ADD_VOLUME, data, reply, option);

    int32_t result = reply.ReadInt32();
    return result;
}
}
