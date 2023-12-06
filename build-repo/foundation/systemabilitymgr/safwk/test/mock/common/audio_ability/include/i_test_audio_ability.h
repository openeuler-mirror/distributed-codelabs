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

#ifndef TEST_MOCK_COMMON_AUDIO_ABILITY_INCLUDE_IF_AUDIO_ABILITY_H_
#define TEST_MOCK_COMMON_AUDIO_ABILITY_INCLUDE_IF_AUDIO_ABILITY_H_

#include <string>

#include "iremote_broker.h"
#include "iremote_object.h"
#include "iremote_stub.h"
#include "iremote_proxy.h"

namespace OHOS {
class ITestAudioAbility : public IRemoteBroker {
public:

    virtual int AddVolume(int volume) = 0;
    virtual int32_t ReduceVolume(int volume) = 0;
    virtual int32_t TestRpcInt32(int32_t value) = 0;
    virtual uint32_t TestRpcUInt32(uint32_t value) = 0;
    virtual int64_t TestRpcInt64(int64_t value) = 0;
    virtual uint64_t TestRpcUInt64(uint64_t value) = 0;
    virtual float TestRpcFloat(float value) = 0;
    virtual double TestRpcDouble(double value) = 0;
    virtual const std::u16string TestRpcString16(const std::u16string& name) = 0;

public:

    enum {
    ADD_VOLUME = 1,
    REDUCE_VOLUME,
    TEST_RPCINT32,
    TEST_RPCUINT32,
    TEST_RPCINT64,
    TEST_RPCUINT64,
    TEST_RPCFLOAT,
    TEST_RPCDOUBLE,
    TEST_RPCSTRING16,
    };
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.test.ITestAudioAbility");
};
}
#endif // TEST_MOCK_COMMON_AUDIO_ABILITY_INCLUDE_IF_AUDIO_ABILITY_H_