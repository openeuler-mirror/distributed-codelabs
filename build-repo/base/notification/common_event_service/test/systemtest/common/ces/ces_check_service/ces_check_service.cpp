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

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include "common_event_manager.h"
#define private public
#define protected public
#include "common_event_manager_service.h"
#undef private
#undef protected
#include "common_event_subscriber.h"
#include "common_event_support.h"
#include "datetime_ex.h"
#include "event_log_wrapper.h"
#include "iservice_registry.h"
#include "singleton.h"
#include "system_ability_definition.h"

#include <gtest/gtest.h>

using namespace testing::ext;

namespace OHOS {
namespace EventFwk {
namespace {
std::mutex mtx_;
const time_t TIME_OUT_SECONDS_LIMIT = 5;
const std::string COMPARE_STR_FALSE = "cesComparesStrForCaseFalse";
const int32_t g_CODE_COMPARE3 = 200;
}  // namespace

class CommonEventServicesSystemTest : public CommonEventSubscriber {
public:
    explicit CommonEventServicesSystemTest(const CommonEventSubscribeInfo &subscribeInfo);
    virtual ~CommonEventServicesSystemTest() {};
    virtual void OnReceiveEvent(const CommonEventData &data);
};

CommonEventServicesSystemTest::CommonEventServicesSystemTest(const CommonEventSubscribeInfo &subscribeInfo)
    : CommonEventSubscriber(subscribeInfo)
{}

void CommonEventServicesSystemTest::OnReceiveEvent(const CommonEventData &data)
{
    std::string action = data.GetWant().GetAction();
    if (action == COMPARE_STR_FALSE) {
        EXPECT_TRUE(data.GetCode() == g_CODE_COMPARE3);
    }
    mtx_.unlock();
}

class cesCheckService : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void cesCheckService::SetUpTestCase()
{}

void cesCheckService::TearDownTestCase()
{}

void cesCheckService::SetUp()
{}

void cesCheckService::TearDown()
{}

/*
 * @tc.number: CES_GetServiceObject_0100
 * @tc.name: GetServiceObject
 * @tc.desc: Successfully obtained the service object
 */
HWTEST_F(cesCheckService, CES_GetServiceObject_0100, Function | MediumTest | Level0)
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(systemAbilityManager, nullptr);

    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(COMMON_EVENT_SERVICE_ID);
    EXPECT_NE(remoteObject, nullptr);
}

/*
 * @tc.number: CES_ReceiveEvent_0100
 * @tc.name:OnReceiveEvent
 * @tc.desc: Verify the function when only set action
 */
HWTEST_F(cesCheckService, CES_ReceiveEvent_0100, Function | MediumTest | Level0)
{
    std::string eventName = "TESTEVENT_RECEIVE_ACTIONReceiveEvent_0100";
    std::string eventAction = "TESTEVENT_RECEIVE_ACTIONReceiveEvent_0100";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    CommonEventData commonEventData(wantTest);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    mtx_.lock();
    EXPECT_EQ(CommonEventManager::PublishCommonEvent(commonEventData), true);

    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);

    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!mtx_.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_LIMIT) {
            break;
        }
    }
    // expect the subscriber could receive the event within 5 seconds.
    EXPECT_LT(seconds, TIME_OUT_SECONDS_LIMIT);
    mtx_.unlock();
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}
}  // namespace EventFwk
}  // namespace OHOS