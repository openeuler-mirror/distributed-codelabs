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

#ifndef SAFWK_TEST_SERVICES_SAFWK_UNITTEST_LISTEN_ABILITY_INCLUDE_LISTEN_ABILITY_PROXY_H_
#define SAFWK_TEST_SERVICES_SAFWK_UNITTEST_LISTEN_ABILITY_INCLUDE_LISTEN_ABILITY_PROXY_H_

#include "iremote_proxy.h"
#include "ilisten_ability.h"
#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "refbase.h"

namespace OHOS {
class ListenAbilityProxy : public IRemoteProxy<IListenAbility> {
public:
    int32_t AddVolume(int32_t volume) override;

    explicit ListenAbilityProxy(const sptr<IRemoteObject>& impl)
        : IRemoteProxy<IListenAbility>(impl)
    {
    }
    ~ListenAbilityProxy() = default;

private:
    static inline BrokerDelegator<ListenAbilityProxy> delegator_;
};
} // namespace OHOS
#endif /* SAFWK_TEST_SERVICES_SAFWK_UNITTEST_LISTEN_ABILITY_INCLUDE_LISTEN_ABILITY_PROXY_H_ */
