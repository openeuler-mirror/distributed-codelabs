/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "test_ondemand_ability_stub.h"

#include "ipc_object_stub.h"
#include "ipc_types.h"
#include "message_option.h"
#include "message_parcel.h"

using namespace OHOS::HiviewDFX;

namespace OHOS {
int32_t TestOnDemandAbilityStub::OnRemoteRequest(uint32_t cmd,
    MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    HiLog::Info(label_, "TestOnDemandAbilityStub::OnReceived, cmd = %{public}u, flags= %{public}d",
        cmd, option.GetFlags());
    switch (cmd) {
        case ADD_VOLUME: {
            int32_t num = 0;
            bool ret = data.ReadInt32(num);
            if (!ret) {
                HiLog::Error(label_, "TestOnDemandAbilityStub::AddVolume read requestCode failed!");
                return ERR_FLATTEN_OBJECT;
            }
            ret = reply.WriteInt32(AddVolume(num));
            if (!ret) {
                HiLog::Error(label_, "TestOnDemandAbilityStub:AddVolume write reply failed.");
                return ERR_FLATTEN_OBJECT;
            }

            return ret;
        }

        default:
            return IPCObjectStub::OnRemoteRequest(cmd, data, reply, option);
    }
}
}