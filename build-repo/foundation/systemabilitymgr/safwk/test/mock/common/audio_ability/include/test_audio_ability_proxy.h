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

#ifndef TEST_MOCK_COMMON_AUDIO_ABILITY_INCLUDE_AUDIO_ABILITY_PROXY_H_
#define TEST_MOCK_COMMON_AUDIO_ABILITY_INCLUDE_AUDIO_ABILITY_PROXY_H_

#include <string>

#include "hilog/log.h"
#include "i_test_audio_ability.h"
#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "refbase.h"

namespace OHOS {
class TestAudioAbilityProxy : public IRemoteProxy<ITestAudioAbility> {
public:
    explicit TestAudioAbilityProxy(const sptr<IRemoteObject>& impl)
        : IRemoteProxy<ITestAudioAbility>(impl) {}
    ~TestAudioAbilityProxy() = default;

    int AddVolume(int volume);
    int32_t ReduceVolume(int volume);
    int32_t TestRpcInt32(int32_t value);
    uint32_t TestRpcUInt32(uint32_t value);
    int64_t TestRpcInt64(int64_t value);
    uint64_t TestRpcUInt64(uint64_t value);
    float TestRpcFloat(float value);
    double TestRpcDouble(double value);
    const std::u16string TestRpcString16(const std::u16string& name);

private:
    static inline BrokerDelegator<TestAudioAbilityProxy> delegator_;
    OHOS::HiviewDFX::HiLogLabel label_ = { LOG_CORE, 0xD001800, "SA" };
};
}

#endif // TEST_MOCK_COMMON_AUDIO_ABILITY_INCLUDE_AUDIO_ABILITY_PROXY_H_