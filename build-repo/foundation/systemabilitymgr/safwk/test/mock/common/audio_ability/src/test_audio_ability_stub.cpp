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

#include "test_audio_ability_stub.h"

#include "errors.h"
#include "ipc_object_stub.h"
#include "ipc_types.h"
#include "message_option.h"
#include "message_parcel.h"

using namespace OHOS::HiviewDFX;

namespace OHOS {
int32_t TestAudioAbilityStub::OnRemoteRequest(uint32_t code,
    MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    HiLog::Info(label_, "TestAudioAbilityStub::OnReceived, code = %{public}u, flags= %{public}d",
        code, option.GetFlags());

    switch (code) {
        case ADD_VOLUME: {
            bool ret = reply.WriteInt32(AddVolume(data.ReadInt32()));
            return (ret ? ERR_OK : ERR_FLATTEN_OBJECT);
        }

        case REDUCE_VOLUME: {
            bool ret = reply.WriteInt32(ReduceVolume(data.ReadInt32()));
            return (ret ? ERR_OK : ERR_FLATTEN_OBJECT);
        }

        case TEST_RPCINT32: {
            bool ret = reply.WriteInt32(TestRpcInt32(data.ReadInt32()));
            return (ret ? ERR_OK : ERR_FLATTEN_OBJECT);
        }

        case TEST_RPCUINT32: {
            bool ret = reply.WriteUint32(TestRpcUInt32(data.ReadUint32()));
            return (ret ? ERR_OK : ERR_FLATTEN_OBJECT);
        }

        case TEST_RPCINT64: {
            bool ret = reply.WriteInt64(TestRpcInt64(data.ReadInt64()));
            return (ret ? ERR_OK : ERR_FLATTEN_OBJECT);
        }

        case TEST_RPCUINT64: {
            bool ret = reply.WriteUint64(TestRpcUInt64(data.ReadUint64()));
            return (ret ? ERR_OK : ERR_FLATTEN_OBJECT);
        }

        case TEST_RPCFLOAT: {
            bool ret = reply.WriteFloat(TestRpcFloat(data.ReadFloat()));
            return (ret ? ERR_OK : ERR_FLATTEN_OBJECT);
        }

        case TEST_RPCDOUBLE: {
            bool ret = reply.WriteDouble(TestRpcDouble(data.ReadDouble()));
            return (ret ? ERR_OK : ERR_FLATTEN_OBJECT);
        }

        case TEST_RPCSTRING16: {
            std::u16string strTmp = TestRpcString16(data.ReadString16());
            bool ret = reply.WriteString16(strTmp.c_str());
            return (ret ? ERR_OK : ERR_FLATTEN_OBJECT);
        }

        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
}
}
