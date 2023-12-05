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

#include "test_ondemand_ability.h"

#include "hilog/log.h"
#include "iremote_object.h"

using namespace OHOS;
using namespace OHOS::HiviewDFX;

REGISTER_SYSTEM_ABILITY_BY_ID(TestOnDemandAbility, 1497, false); // 1497 is test SA

namespace OHOS {
TestOnDemandAbility::TestOnDemandAbility(int32_t saId, bool runOnCreate) : SystemAbility(1497, false) // 1497 is test SA
{
    HiLog::Info(label_, "%{public}s called", __func__);
}

TestOnDemandAbility::~TestOnDemandAbility()
{
    HiLog::Info(label_, "%{public}s called", __func__);
}

sptr<IRemoteObject> TestOnDemandAbility::GetBaseSystemAbility(int32_t systemAbilityId)
{
    sptr<IRemoteObject> object = GetSystemAbility(systemAbilityId);
    return object;
}

int TestOnDemandAbility::AddVolume(int volume)
{
    HiLog::Info(label_, "TestOnDemandAbility:: %{public}s called, volume = %{public}d",
        __func__, volume);
    return (volume + 1);
}

void TestOnDemandAbility::OnStart()
{
    HiLog::Info(label_, "%{public}s called", __func__);
    bool res = Publish(this);
    if (!res) {
        HiLog::Error(label_, "%{public}s:fail to onstart res = %{public}d", __func__, res);
    }
    return;
}

void TestOnDemandAbility::OnStop()
{
}
}
