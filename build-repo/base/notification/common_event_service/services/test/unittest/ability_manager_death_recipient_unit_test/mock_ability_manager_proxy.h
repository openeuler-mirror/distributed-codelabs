/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_CES_MOCK_ABILITY_MANAGER_PROXY_H
#define OHOS_CES_MOCK_ABILITY_MANAGER_PROXY_H

#include <iremote_proxy.h>
#include "imock_ability_manager.h"

namespace OHOS {
namespace EventFwk {
class MockAbilityManagerProxy : public IRemoteProxy<IMockAbilityManager> {
public:
    explicit MockAbilityManagerProxy(const sptr<IRemoteObject>& remote)
        : IRemoteProxy<IMockAbilityManager>(remote) {}

    virtual ~MockAbilityManagerProxy() {}

private:
    static inline BrokerDelegator<MockAbilityManagerProxy> delegator_;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_CES_MOCK_ABILITY_MANAGER_PROXY_H

