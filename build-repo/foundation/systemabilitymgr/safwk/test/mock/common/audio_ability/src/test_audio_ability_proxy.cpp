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

#include "test_audio_ability_proxy.h"

#include <cinttypes>

#include "iremote_object.h"
#include "message_option.h"
#include "message_parcel.h"

using namespace OHOS::HiviewDFX;

namespace OHOS {
namespace {
    const int DEFAULT_INT_RET = -1;
    const int32_t DEFAULT_INT32_RET = -1;
    const uint32_t DEFAULT_UINT32_RET = 9999;
    const int64_t DEFAULT_INT64_RET = -1;
    const int64_t DEFAULT_UINT64_RET = 9999;
    const float DEFAULT_FLOAT_RET = -1.0f;
    const double DEFAULT_DOUBLE_RET = -1.0;
    const std::u16string DEFAULT_U16STRING_RET = u"";
}

int TestAudioAbilityProxy::AddVolume(int volume)
{
    HiLog::Info(label_, "%{public}s called", __func__);
    auto remote = Remote();
    if (remote == nullptr) {
        HiLog::Error(label_, "%{public}s: object is null", __func__);
        return DEFAULT_INT_RET;
    }
    MessageParcel data;
    data.WriteInt32(volume);
    MessageParcel reply;
    MessageOption option;
    remote->SendRequest(ADD_VOLUME, data, reply, option);

    int32_t result = reply.ReadInt32();
    HiLog::Info(label_, "%{public}s:finish = %{public}d", __func__, result);
    return result;
}

int32_t TestAudioAbilityProxy::ReduceVolume(int volume)
{
    HiLog::Info(label_, "%{public}s called", __func__);
    auto remote = Remote();
    if (remote == nullptr) {
        HiLog::Error(label_, "ReduceVolume remote is NULL !");
        return DEFAULT_INT32_RET;
    }

    MessageParcel data;
    data.WriteInt32(volume);

    MessageParcel reply;
    MessageOption option { MessageOption::TF_ASYNC };
    int32_t ret = remote->SendRequest(REDUCE_VOLUME, data, reply, option);

    HiLog::Info(label_, "%{public}s:finish = %{public}d", __func__, ret);
    return ret;
}

int32_t TestAudioAbilityProxy::TestRpcInt32(int32_t value)
{
    HiLog::Info(label_, "%{public}s called", __func__);
    auto remote = Remote();
    if (remote == nullptr) {
        HiLog::Error(label_, "TestRpcInt32 remote is NULL !");
        return DEFAULT_INT32_RET;
    }
    MessageParcel data;
    data.WriteInt32(value);
    MessageParcel reply;
    MessageOption option;
    remote->SendRequest(TEST_RPCINT32, data, reply, option);

    int32_t result = reply.ReadInt32();
    HiLog::Info(label_, "%{public}s:finish = %{public}d", __func__, result);
    return result;
}

uint32_t TestAudioAbilityProxy::TestRpcUInt32(uint32_t value)
{
    HiLog::Info(label_, "%{public}s called", __func__);
    auto remote = Remote();
    if (remote == nullptr) {
        HiLog::Error(label_, "TestRpcUInt32 remote is NULL !");
        return DEFAULT_UINT32_RET;
    }
    MessageParcel data;
    data.WriteUint32(value);
    MessageParcel reply;
    MessageOption option;
    remote->SendRequest(TEST_RPCUINT32, data, reply, option);

    uint32_t result = reply.ReadUint32();
    HiLog::Info(label_, "%{public}s:finish = %{public}u", __func__, result);
    return result;
}

int64_t TestAudioAbilityProxy::TestRpcInt64(int64_t value)
{
    HiLog::Info(label_, "%{public}s called", __func__);
    auto remote = Remote();
    if (remote == nullptr) {
        HiLog::Error(label_, "TestRpcInt64 remote is NULL !");
        return DEFAULT_INT64_RET;
    }
    MessageParcel data;
    data.WriteInt64(value);
    MessageParcel reply;
    MessageOption option;
    remote->SendRequest(TEST_RPCINT64, data, reply, option);

    int64_t result = reply.ReadInt64();
    HiLog::Info(label_, "%{public}s:finish = %{public}" PRId64, __func__, result);
    return result;
}

uint64_t TestAudioAbilityProxy::TestRpcUInt64(uint64_t value)
{
    HiLog::Info(label_, "%{public}s called", __func__);
    auto remote = Remote();
    if (remote == nullptr) {
        HiLog::Error(label_, "TestRpcInt64 remote is NULL !");
        return DEFAULT_UINT64_RET;
    }
    MessageParcel data;
    data.WriteUint64(value);
    MessageParcel reply;
    MessageOption option;
    remote->SendRequest(TEST_RPCUINT64, data, reply, option);

    uint64_t result = reply.ReadUint64();
    HiLog::Info(label_, "%{public}s:finish = %{public}" PRIu64, __func__, result);
    return result;
}

float TestAudioAbilityProxy::TestRpcFloat(float value)
{
    HiLog::Info(label_, "%{public}s called", __func__);
    auto remote = Remote();
    if (remote == nullptr) {
        HiLog::Error(label_, "TestRpcFloat remote is NULL !");
        return DEFAULT_FLOAT_RET;
    }
    MessageParcel data;
    data.WriteFloat(value);
    MessageParcel reply;
    MessageOption option;
    remote->SendRequest(TEST_RPCFLOAT, data, reply, option);

    float result = reply.ReadFloat();
    HiLog::Info(label_, "%{public}s:finish = %{public}f", __func__, result);
    return result;
}

double TestAudioAbilityProxy::TestRpcDouble(double value)
{
    HiLog::Info(label_, "%{public}s called", __func__);
    auto remote = Remote();
    if (remote == nullptr) {
        HiLog::Error(label_, "TestRpcDouble remote is NULL !");
        return DEFAULT_DOUBLE_RET;
    }
    MessageParcel data;
    data.WriteDouble(value);
    MessageParcel reply;
    MessageOption option;
    remote->SendRequest(TEST_RPCDOUBLE, data, reply, option);

    double result = reply.ReadDouble();
    HiLog::Info(label_, "%{public}s:finish = %{public}lf", __func__, result);
    return result;
}

const std::u16string TestAudioAbilityProxy::TestRpcString16(const std::u16string& name)
{
    HiLog::Info(label_, "%{public}s called", __func__);
    auto remote = Remote();
    if (remote == nullptr) {
        HiLog::Error(label_, "TestRpcString16 remote is NULL !");
        return DEFAULT_U16STRING_RET;
    }
    MessageParcel data;
    std::u16string strTmp = name;
    data.WriteString16(strTmp.c_str());
    MessageParcel reply;
    MessageOption option;
    remote->SendRequest(TEST_RPCSTRING16, data, reply, option);

    return reply.ReadString16();
}
}
