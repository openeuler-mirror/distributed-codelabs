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

#include "test_audio_ability.h"

#include "hilog/log.h"
#include "iremote_object.h"
#include "system_ability_definition.h"

using namespace OHOS;
using namespace OHOS::HiviewDFX;

REGISTER_SYSTEM_ABILITY_BY_ID(TestAudioAbility, 1499, true);

TestAudioAbility::TestAudioAbility(int32_t saId, bool runOnCreate) : SystemAbility(saId, runOnCreate)
{
    HiLog::Info(label_, "%{public}s called", __func__);
}

TestAudioAbility::~TestAudioAbility()
{
    HiLog::Info(label_, "%{public}s called", __func__);
}

sptr<IRemoteObject> TestAudioAbility::GetBaseSystemAbility(int32_t systemAbilityId)
{
    sptr<IRemoteObject> object = GetSystemAbility(systemAbilityId);
    return object;
}

int TestAudioAbility::AddVolume(int volume)
{
    HiLog::Info(label_, "TestAudioAbility:: %{public}s called, volume = %{public}d", __func__, volume);
    return (volume + 1);
}

int32_t TestAudioAbility::ReduceVolume(int volume)
{
    HiLog::Info(label_, "TestAudioAbility:: %{public}s called, volume = %{public}d", __func__, volume);
    return (volume - 1);
}

int32_t TestAudioAbility::TestRpcInt32(int32_t value)
{
    return value;
}

uint32_t TestAudioAbility::TestRpcUInt32(uint32_t value)
{
    return value;
}

int64_t TestAudioAbility::TestRpcInt64(int64_t value)
{
    return value;
}

uint64_t TestAudioAbility::TestRpcUInt64(uint64_t value)
{
    return value;
}

float TestAudioAbility::TestRpcFloat(float value)
{
    return value;
}

double TestAudioAbility::TestRpcDouble(double value)
{
    return value;
}

const std::u16string TestAudioAbility::TestRpcString16(const std::u16string& name)
{
    return name;
}

void TestAudioAbility::OnStart()
{
    HiLog::Info(label_, "%{public}s called", __func__);
    bool res = Publish(this);
    if (res == false) {
        HiLog::Error(label_, "%{public}s:fail to onstart res = %{public}d", __func__, res);
    }
    AddSystemAbilityListener(DISTRIBUTED_SCHED_SA_ID);
    AddSystemAbilityListener(SOFTBUS_SERVER_SA_ID);
    AddSystemAbilityListener(RES_SCHED_SYS_ABILITY_ID);
    AddSystemAbilityListener(BACKGROUND_TASK_MANAGER_SERVICE_ID);
    return;
}

void TestAudioAbility::OnStop()
{
}

void TestAudioAbility::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    HiLog::Info(label_, "OnAddSystemAbility systemAbilityId:%{public}d added!", systemAbilityId);
}

void TestAudioAbility::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    HiLog::Info(label_, "OnRemoveSystemAbility systemAbilityId:%{public}d removed!", systemAbilityId);
}