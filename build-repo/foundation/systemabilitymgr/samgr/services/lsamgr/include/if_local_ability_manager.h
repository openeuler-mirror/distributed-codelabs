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

#ifndef IF_LOCAL_ABILITY_MANAGER_H_
#define IF_LOCAL_ABILITY_MANAGER_H_

#include <string>
#include "iremote_broker.h"
#include "iremote_object.h"
#include "iremote_stub.h"
#include "iremote_proxy.h"

namespace OHOS {
class ILocalAbilityManager : public IRemoteBroker {
public:
    virtual bool StartAbility(int32_t saId) = 0;

    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.ILocalAbilityManager");
protected:
    enum {
        START_ABILITY_TRANSACTION = 1,
    };
    static inline const std::u16string LOCAL_ABILITY_MANAGER_INTERFACE_TOKEN = u"ohos.localabilitymanager.accessToken";
};
}
#endif // !defined(IF_LOCAL_ABILITY_MANAGER_H_)
