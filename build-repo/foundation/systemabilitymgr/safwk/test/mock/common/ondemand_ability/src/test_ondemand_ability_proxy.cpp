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

#include "test_ondemand_ability_proxy.h"

#include "errors.h"
#include "ipc_types.h"
#include "iremote_object.h"
#include "message_option.h"
#include "message_parcel.h"

using namespace OHOS::HiviewDFX;

namespace OHOS {
int TestOnDemandAbilityProxy::AddVolume(int volume)
{
    HiLog::Info(label_, "%{public}s called, volume = %{public}d", __func__, volume);

    auto remote = Remote();
    if (remote == nullptr) {
        HiLog::Error(label_, "AddVolume remote is NULL !");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel data;
    int32_t result = 0;
    bool ret = data.WriteInt32(volume);
    if (!ret) {
        HiLog::Error(label_, "AddVolume parcel write volume failed");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t res = remote->SendRequest(ADD_VOLUME, data, reply, option);
    if (res != ERR_OK) {
        HiLog::Error(label_, "AddVolume Transact error:%{public}d!", res);
        return ERR_FLATTEN_OBJECT;
    }

    ret = reply.ReadInt32(result);
    if (!ret) {
        HiLog::Error(label_, "AddVolume parcel read volume failed");
        return ERR_FLATTEN_OBJECT;
    }
    HiLog::Info(label_, "%{public}s:finish = %{public}d", __func__, result);
    return result;
}
}
