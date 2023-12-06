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

#ifndef LOCAL_ABILITY_MANAGER_PROXY_H_
#define LOCAL_ABILITY_MANAGER_PROXY_H_

#include <string>
#include "hilog/log.h"
#include "if_local_ability_manager.h"
#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "refbase.h"

namespace OHOS {
class LocalAbilityManagerProxy : public IRemoteProxy<ILocalAbilityManager> {
public:
    explicit LocalAbilityManagerProxy(const sptr<IRemoteObject>& impl)
        : IRemoteProxy<ILocalAbilityManager>(impl) {}
    ~LocalAbilityManagerProxy() = default;

    bool StartAbility(int32_t saId);
private:
    static inline BrokerDelegator<LocalAbilityManagerProxy> delegator_;
    OHOS::HiviewDFX::HiLogLabel label_ = { LOG_CORE, 0xD001800, "SA" };
};
}
#endif // !defined(LOCAL_ABILITY_MANAGER_PROXY_H_)
