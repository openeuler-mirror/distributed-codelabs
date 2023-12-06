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

#ifndef AUDIO_ABILITY_H_
#define AUDIO_ABILITY_H_

#include <string>

#include "refbase.h"
#include "system_ability.h"
#include "test_audio_ability_stub.h"

namespace OHOS {
class TestAudioAbility : public SystemAbility, public TestAudioAbilityStub {
    DECLARE_SYSTEM_ABILITY(TestAudioAbility);

public:
    TestAudioAbility(int32_t saId, bool runOnCreate);
    ~TestAudioAbility();

    int AddVolume(int volume) override;
    int32_t ReduceVolume(int volume) override;
    int32_t TestRpcInt32(int32_t value) override;
    uint32_t TestRpcUInt32(uint32_t value) override;
    int64_t TestRpcInt64(int64_t value) override;
    uint64_t TestRpcUInt64(uint64_t value) override;
    float TestRpcFloat(float value) override;
    double TestRpcDouble(double value) override;
    const std::u16string TestRpcString16(const std::u16string& name) override;
    sptr<IRemoteObject> GetBaseSystemAbility(int32_t systemAbilityId);

protected:
    void OnStart() override;
    void OnStop() override;
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
};
}

#endif // !defined(AUDIO_ABILITY_H_)
