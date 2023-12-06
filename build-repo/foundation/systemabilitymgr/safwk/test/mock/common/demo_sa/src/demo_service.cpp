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

#include "demo_service.h"

#include "iremote_object.h"
#include "system_ability_definition.h"

using namespace OHOS;

REGISTER_SYSTEM_ABILITY_BY_ID(DemoService, DISTRIBUTED_SCHED_TEST_MEDIA_ID, true);

DemoService::DemoService(int32_t saId, bool runOnCreate) : SystemAbility(saId, runOnCreate)
{
}

DemoService::~DemoService()
{
}

int DemoService::AddVolume(int volume)
{
    return (volume + 1);
}

void DemoService::OnStart()
{
    Publish(this);
}

void DemoService::OnStop()
{
}