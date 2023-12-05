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

#ifndef TEST_MOCK_COMMON_ONDEMAND_ABILITY_INCLUDE_ONDEMAND_ABILITY_PROXY_H_
#define TEST_MOCK_COMMON_ONDEMAND_ABILITY_INCLUDE_ONDEMAND_ABILITY_PROXY_H_

#include "hilog/log.h"
#include "i_test_ondemand_ability.h"
#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "refbase.h"

namespace OHOS {
class TestOnDemandAbilityProxy : public IRemoteProxy<ITestOnDemandAbility> {
public:
    explicit TestOnDemandAbilityProxy(const sptr<IRemoteObject>& impl)
        : IRemoteProxy<ITestOnDemandAbility>(impl) {}
    ~TestOnDemandAbilityProxy() = default;

    int AddVolume(int volume);

private:
    static inline BrokerDelegator<TestOnDemandAbilityProxy> delegator_;
    OHOS::HiviewDFX::HiLogLabel label_ = { LOG_CORE, 0xD001800, "SA" };
};
}

#endif // TEST_MOCK_COMMON_ONDEMAND_ABILITY_INCLUDE_