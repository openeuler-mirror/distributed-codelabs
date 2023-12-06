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

#ifndef TEST_MOCK_COMMON_ONDEMAND_ABILITY_INCLUDE_ONDEMAND_ABILITY_H_
#define TEST_MOCK_COMMON_ONDEMAND_ABILITY_INCLUDE_ONDEMAND_ABILITY_H_

#include <string>

#include "refbase.h"
#include "system_ability.h"
#include "test_ondemand_ability_stub.h"

namespace OHOS {
class TestOnDemandAbility : public SystemAbility, public TestOnDemandAbilityStub {
    DECLARE_SYSTEM_ABILITY(TestOnDemandAbility);

public:
    TestOnDemandAbility(int32_t saId, bool runOnCreate);
    TestOnDemandAbility();
    ~TestOnDemandAbility();

    int AddVolume(int volume) override;
    sptr<IRemoteObject> GetBaseSystemAbility(int32_t systemAbilityId);

protected:
    void OnStart() override;
    void OnStop() override;
};
}

#endif //  TEST_MOCK_COMMON_ONDEMAND_ABILITY_INCLUDE_ONDEMAND_ABILITY_H_
