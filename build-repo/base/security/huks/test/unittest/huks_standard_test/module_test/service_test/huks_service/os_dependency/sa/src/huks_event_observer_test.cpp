/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "huks_event_observer_test.h"

#include <gtest/gtest.h>
#include <cstring>

#include "hks_api.h"
#include "hks_log.h"
#include "hks_mem.h"
#include "hks_param.h"
#include "hks_event_observer.h"
#include "hks_type_inner.h"

#include "common_event_data.h"
#include "common_event_subscribe_info.h"
#include "common_event_support.h"
#include "matching_skills.h"

using namespace testing::ext;
using namespace OHOS::EventFwk;
using namespace OHOS::Security::Hks;
namespace Unittest::HksEventObserverTest {
class HksEventObserverTest : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};

void HksEventObserverTest::SetUpTestCase(void)
{
    HksInitialize();
}

void HksEventObserverTest::TearDownTestCase(void)
{
}

void HksEventObserverTest::SetUp()
{
}

void HksEventObserverTest::TearDown()
{
}

/**
 * @tc.name: HksEventObserverTest.HksEventObserverTest001
 * @tc.desc: tdd OnReceiveEvent with COMMON_EVENT_PACKAGE_REMOVED, expect HKS_SUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(HksEventObserverTest, HksEventObserverTest001, TestSize.Level0)
{
    HKS_LOG_I("enter HksEventObserverTest001");
    Want want;
    want.SetAction(OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
    CommonEventData data;
    data.SetWant(want);

    CommonEventSubscribeInfo subscribeInfo;
    SystemEventSubscriber eventSubscriber(subscribeInfo);
    eventSubscriber.OnReceiveEvent(data);
}

/**
 * @tc.name: HksEventObserverTest.HksEventObserverTest002
 * @tc.desc: tdd OnReceiveEvent with COMMON_EVENT_PACKAGE_REMOVED, expect HKS_SUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(HksEventObserverTest, HksEventObserverTest002, TestSize.Level0)
{
    HKS_LOG_I("enter HksEventObserverTest002");
    Want want;
    want.SetAction(OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_USER_REMOVED);
    
    CommonEventData data;
    data.SetWant(want);

    CommonEventSubscribeInfo subscribeInfo;
    SystemEventSubscriber eventSubscriber(subscribeInfo);
    eventSubscriber.OnReceiveEvent(data);
}
}
