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

#ifndef TEST_MOCK_COMMON_ONDEMAND_ABILITY_INCLUDE_IF_ONDEMAND_ABILITY_H_
#define TEST_MOCK_COMMON_ONDEMAND_ABILITY_INCLUDE_IF_ONDEMAND_ABILITY_H_

#include <string>
#include "iremote_broker.h"
#include "iremote_object.h"
#include "iremote_stub.h"
#include "iremote_proxy.h"

namespace OHOS {
class ITestOnDemandAbility : public IRemoteBroker {
public:
    virtual int AddVolume(int volume) = 0;

    enum {
        ADD_VOLUME = 1,
    };
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"test.safwk.ITestOnDemandAbility");
};
}
#endif // TEST_MOCK_COMMON_ONDEMAND_ABILITY_INCLUDE_IF_ONDEMAND_ABILITY_H_