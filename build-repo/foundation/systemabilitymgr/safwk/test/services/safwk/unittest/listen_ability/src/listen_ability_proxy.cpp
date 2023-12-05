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

#include "listen_ability_proxy.h"

#include "iremote_object.h"
#include "message_option.h"
#include "message_parcel.h"

namespace OHOS {
int32_t ListenAbilityProxy::AddVolume(int32_t volume)
{
    MessageParcel data, reply;
    MessageOption option;
    data.WriteInterfaceToken(GetDescriptor());
    data.WriteInt32(volume);
    Remote()->SendRequest(ADD_VOLUME, data, reply, option);
    int32_t result = reply.ReadInt32();
    return result;
}
}
