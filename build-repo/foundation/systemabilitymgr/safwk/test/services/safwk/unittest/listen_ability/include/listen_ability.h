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

#ifndef SAFWK_TEST_SERVICES_SAFWK_UNITTEST_LISTEN_ABILITY_INCLUDE_LISTEN_ABILITY_H_
#define SAFWK_TEST_SERVICES_SAFWK_UNITTEST_LISTEN_ABILITY_INCLUDE_LISTEN_ABILITY_H_

#include <string>

#include "listen_ability_stub.h"
#include "system_ability.h"

namespace OHOS {
class ListenAbility : public SystemAbility, public ListenAbilityStub {
    DECLARE_SYSTEM_ABILITY(ListenAbility);

public:
    ListenAbility(int32_t systemAbilityId, bool runOnCreate);
    ~ListenAbility();
    int32_t AddVolume(int32_t volume) override;
protected:
    void OnStart() override;
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
};
}
#endif /* SAFWK_TEST_SERVICES_SAFWK_UNITTEST_LISTEN_ABILITY_INCLUDE_LISTEN_ABILITY_H_ */
