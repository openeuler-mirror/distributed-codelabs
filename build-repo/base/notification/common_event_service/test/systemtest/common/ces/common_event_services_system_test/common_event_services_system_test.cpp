/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "nativetoken_kit.h"
#include "token_setproc.h"

#include <gtest/gtest.h>

using namespace testing::ext;

namespace OHOS {
namespace EventFwk {
namespace {
std::mutex mtx_;
std::mutex mtx2_;
const time_t TIME_OUT_SECONDS_LIMIT = 5;
const time_t TIME_OUT_SECONDS_TWO = 2;
const time_t TIME_OUT_SECONDS_ = 3;
const time_t TIME_OUT_SECONDS_TEN = 10;
const time_t TIME_OUT_SECONDS_TWENTY = 20;
const size_t SUBSCRIBER_MAX_SIZE = 200;
const size_t SUBSCRIBER_MAX_SIZE_PLUS = 201;

const std::string COMPARE_STR = "cesComparesStrForCase";
const std::string COMPARE_STR_FALSE = "cesComparesStrForCaseFalse";

const int32_t g_CODE_COMPARE1 = 1;
const int32_t g_CODE_COMPARE2 = 2;
const int32_t g_CODE_COMPARE3 = 200;
const int32_t PRIORITY_HIGH = 80;
const int32_t PRIORITY_LOW = 20;
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
    GTEST_LOG_(INFO) << " cesSystemTest:CommonEventServicesSystemTest:OnReceiveEvent \n";
    std::string action = data.GetWant().GetAction();
    if (action == COMPARE_STR_FALSE) {
        EXPECT_TRUE(data.GetCode() == g_CODE_COMPARE3);
    }
    mtx_.unlock();
}

class CommonEventServicesSystemTestSubscriber : public CommonEventSubscriber {
public:
    explicit CommonEventServicesSystemTestSubscriber(const CommonEventSubscribeInfo &subscribeInfo);
    virtual ~CommonEventServicesSystemTestSubscriber() {};
    virtual void OnReceiveEvent(const CommonEventData &data);
};

CommonEventServicesSystemTestSubscriber::CommonEventServicesSystemTestSubscriber(
    const CommonEventSubscribeInfo &subscribeInfo)
    : CommonEventSubscriber(subscribeInfo)
{}

void CommonEventServicesSystemTestSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    GTEST_LOG_(INFO) << " cesSystemTest:CommonEventServicesSystemTestSubscriber:OnReceiveEvent \n";
    std::string action = data.GetWant().GetAction();
    if (action == COMPARE_STR) {
        EXPECT_TRUE(data.GetCode() == g_CODE_COMPARE1);
    } else if (action == COMPARE_STR_FALSE) {
        EXPECT_TRUE(data.GetCode() == g_CODE_COMPARE2);
    }
    mtx2_.unlock();
}

class CESPublishOrderTimeOutOne : public CommonEventSubscriber {
public:
    explicit CESPublishOrderTimeOutOne(const CommonEventSubscribeInfo &subscribeInfo);
    virtual ~CESPublishOrderTimeOutOne() {};
    virtual void OnReceiveEvent(const CommonEventData &data);
};

CESPublishOrderTimeOutOne::CESPublishOrderTimeOutOne(const CommonEventSubscribeInfo &subscribeInfo)
    : CommonEventSubscriber(subscribeInfo)
{}

void CESPublishOrderTimeOutOne::OnReceiveEvent(const CommonEventData &data)
{
    std::this_thread::sleep_for(std::chrono::seconds(TIME_OUT_SECONDS_TEN));
}

class CESPublishOrderTimeOutTwo : public CommonEventSubscriber {
public:
    explicit CESPublishOrderTimeOutTwo(const CommonEventSubscribeInfo &subscribeInfo);
    virtual ~CESPublishOrderTimeOutTwo() {};
    virtual void OnReceiveEvent(const CommonEventData &data);
};

CESPublishOrderTimeOutTwo::CESPublishOrderTimeOutTwo(const CommonEventSubscribeInfo &subscribeInfo)
    : CommonEventSubscriber(subscribeInfo)
{}

void CESPublishOrderTimeOutTwo::OnReceiveEvent(const CommonEventData &data)
{
    GTEST_LOG_(INFO) << " cesSystemTest:CESPublishOrderTimeOutTwo:OnReceiveEvent \n";
    EXPECT_TRUE(data.GetCode() == GetCode());
    mtx_.unlock();
}

class cesSystemTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    bool PublishCommonEventTest(const std::string &eventName);
};

void cesSystemTest::SetUpTestCase()
{
    uint64_t tokenId;
    const char **perms = new const char *[1];
    perms[0] = "ohos.permission.COMMONEVEVT_STICKY"; // system_core
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 1,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .aplStr = "system_basic",
    };

    infoInstance.processName = "SetUpTestCase";
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    delete[] perms;
}

void cesSystemTest::TearDownTestCase()
{}

void cesSystemTest::SetUp()
{}

void cesSystemTest::TearDown()
{}

/*
 * @tc.number: CES_SubscriptionEvent_0100
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the function when the input string is normal
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_0100, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_SUBSCRIBER";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SubscriptionEvent_0200
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the function when the input string is number
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_0200, Function | MediumTest | Level1)
{
    std::string eventName = "1";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SubscriptionEvent_0300
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the function when the input string is empty
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_0300, Function | MediumTest | Level1)
{
    std::string eventName = "";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SubscriptionEvent_0400
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the function when the input string is quotes
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_0400, Function | MediumTest | Level1)
{
    std::string eventName = ".............";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SubscriptionEvent_0500
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the function when the input string is normal
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_0500, Function | MediumTest | Level1)
{
    std::string eventName = "HELLO\0\0\0WORLD";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SubscriptionEvent_0600
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify that the ordered common event was subsribered successfully
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_0600, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_SUBSCRIBER_SETPRIORITY";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(100);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SubscriptionEvent_0700
 * @tc.name: UnsubscribeCommonEvent
 * @tc.desc: Verify the function when the input string is normal
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_0700, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_UNSUBSCRIBE_SETPRIORITY";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(100);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SubscriptionEvent_0800
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the same input string three times
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_0800, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT1";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    auto subscriberPtr1 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr1), true);

    auto subscriberPtr2 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr2), true);

    auto subscriberPtr3 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr3), true);

    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr1), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr2), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr3), true);
}

/*
 * @tc.number: CES_SubscriptionEvent_0900
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the same input empty string three times
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_0900, Function | MediumTest | Level1)
{
    std::string eventName = "";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    auto subscriberPtr1 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr1), true);

    auto subscriberPtr2 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr2), true);

    auto subscriberPtr3 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr3), true);

    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr1), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr2), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr3), true);
}

/*
 * @tc.number: CES_SubscriptionEvent_1000
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the normal input string three times
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_1000, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT3";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    auto subscriberPtr1 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr1), true);

    auto subscriberPtr2 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr2), true);

    auto subscriberPtr3 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr3), true);

    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr1), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr2), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr3), true);
}

/*
 * @tc.number: CES_SubscriptionEvent_1100
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the normal input string two times
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_1100, Function | MediumTest | Level1)
{
    std::string eventName1 = "TEST1";
    std::string eventName2 = "TEST2";

    MatchingSkills matchingSkills1;
    matchingSkills1.AddEvent(eventName1);
    CommonEventSubscribeInfo subscribeInfo1(matchingSkills1);
    auto subscriberPtr1 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo1);
    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr1), true);

    MatchingSkills matchingSkills2;
    matchingSkills2.AddEvent(eventName2);
    CommonEventSubscribeInfo subscribeInfo2(matchingSkills2);
    auto subscriberPtr2 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo2);
    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr2), true);

    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr1), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr2), true);
}

/*
 * @tc.number: CES_SubscriptionEvent_1200
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the normal, number and empty input string
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_1200, Function | MediumTest | Level1)
{
    std::string eventName1 = "TESTEVENT1";
    std::string eventName2 = "1";
    std::string eventName3 = "";

    MatchingSkills matchingSkills1;
    matchingSkills1.AddEvent(eventName1);
    CommonEventSubscribeInfo subscribeInfo1(matchingSkills1);
    auto subscriberPtr1 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo1);
    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr1), true);

    MatchingSkills matchingSkills2;
    matchingSkills2.AddEvent(eventName2);
    CommonEventSubscribeInfo subscribeInfo2(matchingSkills2);
    auto subscriberPtr2 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo2);
    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr2), true);

    MatchingSkills matchingSkills3;
    matchingSkills3.AddEvent(eventName3);
    CommonEventSubscribeInfo subscribeInfo3(matchingSkills3);
    auto subscriberPtr3 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo3);
    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr3), true);

    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr1), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr2), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr3), true);
}

/*
 * @tc.number: CES_SubscriptionEvent_1300
 * @tc.name: UnSubscribeCommonEvent
 * @tc.desc: Verify the function when the input string is normal
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_1300, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_UNSUBSCRIBE";

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SubscriptionEvent_1400
 * @tc.name: UnSubscribeCommonEvent
 * @tc.desc: Verify the function when the input string is number
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_1400, Function | MediumTest | Level1)
{
    std::string eventName = "2";

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SubscriptionEvent_1500
 * @tc.name: UnSubscribeCommonEvent
 * @tc.desc: Verify the function when the input string is empty
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_1500, Function | MediumTest | Level1)
{
    std::string eventName = "";

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SubscriptionEvent_1600
 * @tc.name: UnSubscribeCommonEvent
 * @tc.desc: Verify the function when the input string is quotes
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_1600, Function | MediumTest | Level1)
{
    std::string eventName = "..................";

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SubscriptionEvent_1700
 * @tc.name: UnSubscribeCommonEvent
 * @tc.desc: Verify the function when the input string with \0
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_1700, Function | MediumTest | Level1)
{
    std::string eventName = "HELLO\0\0\0WORLD";

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SubscriptionEvent_1800
 * @tc.name: UnSubscribeCommonEvent
 * @tc.desc:  Verify the normal input string three times
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_1800, Function | MediumTest | Level1)
{
    std::string eventName1 = "TESTEVENT4";
    std::string eventName2 = "TESTEVENT5";
    std::string eventName3 = "TESTEVENT6";

    MatchingSkills matchingSkills1;
    matchingSkills1.AddEvent(eventName1);
    CommonEventSubscribeInfo subscribeInfo1(matchingSkills1);
    auto subscriberPtr1 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo1);
    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr1), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr1), true);

    MatchingSkills matchingSkills2;
    matchingSkills2.AddEvent(eventName2);
    CommonEventSubscribeInfo subscribeInfo2(matchingSkills2);
    auto subscriberPtr2 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo2);
    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr2), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr2), true);

    MatchingSkills matchingSkills3;
    matchingSkills3.AddEvent(eventName3);
    CommonEventSubscribeInfo subscribeInfo3(matchingSkills3);
    auto subscriberPtr3 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo3);
    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr3), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr3), true);
}

/*
 * @tc.number: CES_SubscriptionEvent_1900
 * @tc.name: UnSubscribeCommonEvent
 * @tc.desc: Verify the normal input string two times
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_1900, Function | MediumTest | Level1)
{
    std::string eventName1 = "TEST3";
    std::string eventName2 = "TEST4";

    MatchingSkills matchingSkills1;
    matchingSkills1.AddEvent(eventName1);
    CommonEventSubscribeInfo subscribeInfo1(matchingSkills1);
    auto subscriberPtr1 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo1);
    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr1), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr1), true);

    MatchingSkills matchingSkills2;
    matchingSkills2.AddEvent(eventName2);
    CommonEventSubscribeInfo subscribeInfo2(matchingSkills2);
    auto subscriberPtr2 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo2);
    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr2), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr2), true);
}

/*
 * @tc.number: CES_SubscriptionEvent_2000
 * @tc.name: UnSubscribeCommonEvent
 * @tc.desc: Verify the normal, number and empty input string
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_2000, Function | MediumTest | Level1)
{
    std::string eventName1 = "TESTEVENT7";
    std::string eventName2 = "3";
    std::string eventName3 = "";

    MatchingSkills matchingSkills1;
    matchingSkills1.AddEvent(eventName1);
    CommonEventSubscribeInfo subscribeInfo1(matchingSkills1);
    auto subscriberPtr1 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo1);
    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr1), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr1), true);

    MatchingSkills matchingSkills2;
    matchingSkills2.AddEvent(eventName2);
    CommonEventSubscribeInfo subscribeInfo2(matchingSkills2);
    auto subscriberPtr2 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo2);
    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr2), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr2), true);

    MatchingSkills matchingSkills3;
    matchingSkills3.AddEvent(eventName3);
    CommonEventSubscribeInfo subscribeInfo3(matchingSkills3);
    auto subscriberPtr3 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo3);
    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr3), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr3), true);
}

/*
 * @tc.number: CES_SubscriptionEvent_2100
 * @tc.name: UnSubscribeCommonEvent
 * @tc.desc: Verify the same input string three times
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_2100, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT4";

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    auto subscriberPtr1 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr1), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr1), true);

    auto subscriberPtr2 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr2), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr2), true);

    auto subscriberPtr3 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr3), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr3), true);
}

/*
 * @tc.number: CES_SubscriptionEvent_2200
 * @tc.name: UnSubscribeCommonEvent
 * @tc.desc:  Verify the same input empty string three times
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_2200, Function | MediumTest | Level1)
{
    std::string eventName = "";

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    auto subscriberPtr1 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr1), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr1), true);

    auto subscriberPtr2 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr2), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr2), true);

    auto subscriberPtr3 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr3), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr3), true);
}

/*
 * @tc.number: CES_SendEvent_0100
 * @tc.name: PublishCommonEvent
 * @tc.desc: Verify the function when only set action
 */
HWTEST_F(cesSystemTest, CES_SendEvent_0100, Function | MediumTest | Level1)
{
    struct tm startTime = {0};
    std::string eventName = "TESTEVENT_PUBLISH_ACTION";

    Want wantTest;
    wantTest.SetAction(eventName);
    CommonEventData commonEventData(wantTest);

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    mtx_.lock();
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);

    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!mtx_.try_lock()) {
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_TWO) {
            break;
        }
    }
    mtx_.unlock();
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SendEvent_0200
 * @tc.name: PublishCommonEvent
 * @tc.desc: Verify the function when add entity
 */
HWTEST_F(cesSystemTest, CES_SendEvent_0200, Function | MediumTest | Level1)
{
    struct tm startTime = {0};
    std::string eventName = "TESTEVENT_PUBLISH_ENTITY";
    std::string entity = "ADDENTITY";

    Want wantTest;
    wantTest.SetAction(eventName);
    wantTest.AddEntity(entity);
    CommonEventData commonEventData(wantTest);

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    mtx_.lock();
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    EXPECT_EQ(CommonEventManager::PublishCommonEvent(commonEventData), true);

    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!mtx_.try_lock()) {
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_TWO) {
            break;
        }
    }
    mtx_.unlock();
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SendEvent_0300
 * @tc.name: PublishCommonEvent
 * @tc.desc: Verify the function when set scheme
 */
HWTEST_F(cesSystemTest, CES_SendEvent_0300, Function | MediumTest | Level1)
{
    struct tm startTime = {0};
    std::string eventName = "TESTEVENT_PUBLISH_SCHEME";
    std::string scheme = "SETSCHEME";

    Want wantTest;
    wantTest.SetAction(eventName);
    CommonEventData commonEventData(wantTest);

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    mtx_.lock();
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    EXPECT_EQ(CommonEventManager::PublishCommonEvent(commonEventData), true);

    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!mtx_.try_lock()) {
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_TWO) {
            break;
        }
    }
    mtx_.unlock();
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SendEvent_0400
 * @tc.name: PublishCommonEvent
 * @tc.desc: Verify the function when set scheme and add entity
 */
HWTEST_F(cesSystemTest, CES_SendEvent_0400, Function | MediumTest | Level1)
{
    struct tm startTime = {0};
    std::string eventName = "TESTEVENT_PUBLISH_SCHEME_ENTITY";
    std::string scheme = "SETSCHEME";
    std::string entity = "ADDENTITY";

    Want wantTest;
    wantTest.SetAction(eventName);
    wantTest.AddEntity(entity);
    CommonEventData commonEventData(wantTest);

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    mtx_.lock();
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    EXPECT_EQ(CommonEventManager::PublishCommonEvent(commonEventData), true);

    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!mtx_.try_lock()) {
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_TWO) {
            break;
        }
    }
    mtx_.unlock();
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SendEvent_0500
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify set action with sticky is false
 */
HWTEST_F(cesSystemTest, CES_SendEvent_0500, Function | MediumTest | Level1)
{
    struct tm startTime = {0};
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_INFO_FALSE";

    Want wantTest;
    wantTest.SetAction(eventName);
    CommonEventData commonEventData(wantTest);

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(false);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    mtx_.lock();
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    EXPECT_EQ(CommonEventManager::PublishCommonEvent(commonEventData, publishInfo), true);

    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!mtx_.try_lock()) {
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_TWO) {
            break;
        }
    }
    mtx_.unlock();
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SendEvent_0600
 * @tc.name: PublishCommonEvent
 * @tc.desc: Verify add entity with sticky is true
 */
HWTEST_F(cesSystemTest, CES_SendEvent_0600, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_ENTITY_INFO_TRUE";
    std::string entity = "ADDENTITY";

    Want wantTest;
    wantTest.SetAction(eventName);
    wantTest.AddEntity(entity);
    CommonEventData commonEventData(wantTest);

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(true);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    EXPECT_EQ(CommonEventManager::PublishCommonEvent(commonEventData, publishInfo), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SendEvent_0700
 * @tc.name: PublishCommonEvent
 * @tc.desc: Verify add entity with sticky is false
 */
HWTEST_F(cesSystemTest, CES_SendEvent_0700, Function | MediumTest | Level1)
{
    struct tm startTime = {0};
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_ENTITY_INFO_FALSE";
    std::string entity = "ADDENTITY";

    Want wantTest;
    wantTest.SetAction(eventName);
    wantTest.AddEntity(entity);
    CommonEventData commonEventData(wantTest);

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(false);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    mtx_.lock();
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    EXPECT_EQ(CommonEventManager::PublishCommonEvent(commonEventData, publishInfo), true);

    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!mtx_.try_lock()) {
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_TWO) {
            break;
        }
    }
    mtx_.unlock();
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SendEvent_0800
 * @tc.name: PublishCommonEvent
 * @tc.desc: Verify set action with sticky is true
 */
HWTEST_F(cesSystemTest, CES_SendEvent_0800, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_INFO_TRUE";

    Want wantTest;
    wantTest.SetAction(eventName);
    CommonEventData commonEventData(wantTest);

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(true);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    EXPECT_EQ(CommonEventManager::PublishCommonEvent(commonEventData, publishInfo), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SendEvent_0900
 * @tc.name: PublishCommonEvent
 * @tc.desc: Verify set scheme with sticky is true
 */
HWTEST_F(cesSystemTest, CES_SendEvent_0900, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_SCHEME_INFO_TRUE";
    std::string scheme = "SETSCHEME";

    Want wantTest;
    wantTest.SetAction(eventName);
    CommonEventData commonEventData(wantTest);

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(true);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    EXPECT_EQ(CommonEventManager::PublishCommonEvent(commonEventData, publishInfo), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SendEvent_1000
 * @tc.name: PublishCommonEvent
 * @tc.desc: Verify set scheme with sticky is false
 */
HWTEST_F(cesSystemTest, CES_SendEvent_1000, Function | MediumTest | Level1)
{
    struct tm startTime = {0};
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_SCHEME_INFO_FALSE";
    std::string scheme = "SETSCHEME";

    Want wantTest;
    wantTest.SetAction(eventName);
    CommonEventData commonEventData(wantTest);

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(false);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    mtx_.lock();
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    EXPECT_EQ(CommonEventManager::PublishCommonEvent(commonEventData, publishInfo), true);

    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!mtx_.try_lock()) {
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_TWO) {
            break;
        }
    }
    mtx_.unlock();
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SendEvent_1100
 * @tc.name: PublishCommonEvent
 * @tc.desc: Verify set scheme and add entity with sticky is true
 */
HWTEST_F(cesSystemTest, CES_SendEvent_1100, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_ENTITY_SCHEME_INFO_TRUE";
    std::string entity = "ADDENTITY";
    std::string scheme = "SETSCHEME";

    Want wantTest;
    wantTest.SetAction(eventName);
    wantTest.AddEntity(entity);
    CommonEventData commonEventData(wantTest);

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(true);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    EXPECT_EQ(CommonEventManager::PublishCommonEvent(commonEventData, publishInfo), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SendEvent_1200
 * @tc.name: PublishCommonEvent
 * @tc.desc: Verify set scheme and add entity with sticky is false
 */
HWTEST_F(cesSystemTest, CES_SendEvent_1200, Function | MediumTest | Level1)
{
    struct tm startTime = {0};
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_ENTITY_SCHEME_INFO_FALSE";
    std::string entity = "ADDENTITY";
    std::string scheme = "SETSCHEME";

    Want wantTest;
    wantTest.SetAction(eventName);
    wantTest.AddEntity(entity);
    CommonEventData commonEventData(wantTest);

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(false);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    mtx_.lock();
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    EXPECT_EQ(CommonEventManager::PublishCommonEvent(commonEventData, publishInfo), true);

    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!mtx_.try_lock()) {
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_TWO) {
            break;
        }
    }
    mtx_.unlock();
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SendEventSetViscosity_0100
 * @tc.name: SetSticky
 * @tc.desc: Verify set action with sticky is false
 */
HWTEST_F(cesSystemTest, CES_SendEventSetViscosity_0100, Function | MediumTest | Level1)
{
    struct tm startTime = {0};
    std::string eventName = "TESTEVENT_STICHY_ACTION_INFO_FALSE";

    Want wantTest;
    wantTest.SetAction(eventName);
    CommonEventData commonEventData(wantTest);

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(false);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    mtx_.lock();
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    EXPECT_EQ(CommonEventManager::PublishCommonEvent(commonEventData, publishInfo), true);

    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!mtx_.try_lock()) {
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_TWO) {
            break;
        }
    }
    mtx_.unlock();
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SendEventSetViscosity_0200
 * @tc.name: SetSticky
 * @tc.desc: Verify add entity with sticky is true
 */
HWTEST_F(cesSystemTest, CES_SendEventSetViscosity_0200, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_STICKY_ACTION_ENTITY_INFO_TRUE";
    std::string entity = "ADDENTITY";

    Want wantTest;
    wantTest.SetAction(eventName);
    wantTest.AddEntity(entity);
    CommonEventData commonEventData(wantTest);

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(true);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    EXPECT_EQ(CommonEventManager::PublishCommonEvent(commonEventData, publishInfo), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SendEventSetViscosity_0300
 * @tc.name: SetSticky
 * @tc.desc: Verify add entity with sticky is false
 */
HWTEST_F(cesSystemTest, CES_SendEventSetViscosity_0300, Function | MediumTest | Level1)
{
    struct tm startTime = {0};
    std::string eventName = "TESTEVENT_STICKY_ACTION_ENTITY_INFO_FALSE";
    std::string entity = "ADDENTITY";

    Want wantTest;
    wantTest.SetAction(eventName);
    wantTest.AddEntity(entity);
    CommonEventData commonEventData(wantTest);

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(false);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    mtx_.lock();
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    EXPECT_EQ(CommonEventManager::PublishCommonEvent(commonEventData, publishInfo), true);

    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!mtx_.try_lock()) {
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_TWO) {
            break;
        }
    }
    mtx_.unlock();
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SendEventSetViscosity_0400
 * @tc.name: SetSticky
 * @tc.desc: Verify set action with sticky is true
 */
HWTEST_F(cesSystemTest, CES_SendEventSetViscosity_0400, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_STICKY_ACTION_INFO_TRUE";

    Want wantTest;
    wantTest.SetAction(eventName);
    CommonEventData commonEventData(wantTest);

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(true);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    EXPECT_EQ(CommonEventManager::PublishCommonEvent(commonEventData, publishInfo), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SendEventSetViscosity_0500
 * @tc.name: SetSticky
 * @tc.desc: Verify set scheme with sticky is true
 */
HWTEST_F(cesSystemTest, CES_SendEventSetViscosity_0500, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_STICKY_ACTION_SCHEME_INFO_TRUE";
    std::string scheme = "SETSCHEME";

    Want wantTest;
    wantTest.SetAction(eventName);
    CommonEventData commonEventData(wantTest);

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(true);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    EXPECT_EQ(CommonEventManager::PublishCommonEvent(commonEventData, publishInfo), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SendEventSetViscosity_0600
 * @tc.name: SetSticky
 * @tc.desc: Verify set scheme with sticky is false
 */
HWTEST_F(cesSystemTest, CES_SendEventSetViscosity_0600, Function | MediumTest | Level1)
{
    struct tm startTime = {0};
    std::string eventName = "TESTEVENT_STICKY_ACTION_SCHEME_INFO_FALSE";
    std::string scheme = "SETSCHEME";

    Want wantTest;
    wantTest.SetAction(eventName);
    CommonEventData commonEventData(wantTest);

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(false);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    mtx_.lock();
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    EXPECT_EQ(CommonEventManager::PublishCommonEvent(commonEventData, publishInfo), true);

    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!mtx_.try_lock()) {
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_TWO) {
            break;
        }
    }
    mtx_.unlock();
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SendEventSetViscosity_0700
 * @tc.name: SetSticky
 * @tc.desc: Verify set scheme and add entity with sticky is true
 */
HWTEST_F(cesSystemTest, CES_SendEventSetViscosity_0700, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_STICKY_ACTION_ENTITY_SCHEME_INFO_TRUE";
    std::string entity = "ADDENTITY";
    std::string scheme = "SETSCHEME";

    Want wantTest;
    wantTest.SetAction(eventName);
    wantTest.AddEntity(entity);
    CommonEventData commonEventData(wantTest);

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(true);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    EXPECT_EQ(CommonEventManager::PublishCommonEvent(commonEventData, publishInfo), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SendEventSetViscosity_0800
 * @tc.name:SetSticky
 * @tc.desc: Verify set scheme and add entity with sticky is false
 */
HWTEST_F(cesSystemTest, CES_SendEventSetViscosity_0800, Function | MediumTest | Level1)
{
    struct tm startTime = {0};
    std::string eventName = "TESTEVENT_STICKY_ACTION_ENTITY_SCHEME_INFO_FALSE";
    std::string entity = "ADDENTITY";
    std::string scheme = "SETSCHEME";

    Want wantTest;
    wantTest.SetAction(eventName);
    wantTest.AddEntity(entity);
    CommonEventData commonEventData(wantTest);

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(false);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    mtx_.lock();
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    EXPECT_EQ(CommonEventManager::PublishCommonEvent(commonEventData, publishInfo), true);

    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!mtx_.try_lock()) {
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_TWO) {
            break;
        }
    }
    mtx_.unlock();
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SendEventSetViscosity_0900
 * @tc.name: GetStickyCommonEvent
 * @tc.desc: publish common event set sticky to true and verify the action of stickyData
 */
HWTEST_F(cesSystemTest, CES_SendEventSetViscosity_0900, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_GETSTICKY_";
    std::string eventActionStr = "TESTEVENT_GETSTICKY_Str";

    Want wantTest;
    wantTest.SetAction(eventName);
    CommonEventData commonEventData(wantTest);

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(true);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    EXPECT_EQ(CommonEventManager::PublishCommonEvent(commonEventData, publishInfo), true);

    CommonEventData stickyData;
    CommonEventManager::GetStickyCommonEvent(eventName, stickyData);
    EXPECT_FALSE(eventActionStr == stickyData.GetWant().GetAction());
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SendEventSetViscosity_1000
 * @tc.name: set sticky  and get another action
 * @tc.desc: publish common event set sticky to true and verify the action of stickyData
 */
HWTEST_F(cesSystemTest, CES_SendEventSetViscosity_1000, TestSize.Level2)
{
    std::string eventName = "TESTEVENT_GETSTICKY_FALSE";
    std::string actionTest = "CHECKTESTACTION";

    Want wantTest;
    wantTest.SetAction(eventName);
    CommonEventData commonEventData(wantTest);

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(true);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    EXPECT_EQ(CommonEventManager::PublishCommonEvent(commonEventData, publishInfo), true);

    CommonEventData stickyData;
    EXPECT_EQ(CommonEventManager::GetStickyCommonEvent(actionTest, stickyData), false);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_ReceiveEvent_0100
 * @tc.name:OnReceiveEvent
 * @tc.desc: Verify the function when only set action
 */
HWTEST_F(cesSystemTest, CES_ReceiveEvent_0100, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_RECEIVE_ACTIONReceiveEvent_0100";

    Want wantTest;
    wantTest.SetAction(eventName);
    CommonEventData commonEventData(wantTest);

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
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
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_ReceiveEvent_0200
 * @tc.name: OnReceiveEvent
 * @tc.desc: Verify the function when add entity
 */
HWTEST_F(cesSystemTest, CES_ReceiveEvent_0200, Function | MediumTest | Level1)
{
    bool result = true;
    std::string eventName = "TESTEVENT_RECEIVE_ENTITYReceiveEvent_0200";
    std::string entity = "ADDENTITY";

    Want wantTest;
    wantTest.SetAction(eventName);
    wantTest.AddEntity(entity);
    CommonEventData commonEventData(wantTest);

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
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
            result = false;
            break;
        }
    }
    // The publisher sets the Entity, the receiver must set it, otherwise the receiver will not receive the information
    EXPECT_FALSE(result);
    mtx_.unlock();
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_ReceiveEvent_0300
 * @tc.name: OnReceiveEvent
 * @tc.desc: Verify set action with sticky is false
 */
HWTEST_F(cesSystemTest, CES_ReceiveEvent_0300, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_RECEIVE_ACTION_INFO_FALSE";

    Want wantTest;
    wantTest.SetAction(eventName);
    CommonEventData commonEventData(wantTest);

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(false);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    mtx_.lock();
    EXPECT_EQ(CommonEventManager::PublishCommonEvent(commonEventData, publishInfo), true);

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
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_ReceiveEvent_0400
 * @tc.name: OnReceiveEvent
 * @tc.desc: Verify set action with sticky is true
 */
HWTEST_F(cesSystemTest, CES_ReceiveEvent_0400, Function | MediumTest | Level1)
{
    bool result = true;
    std::string eventName = "TESTEVENT_RECEIVE_ACTION_INFO_TRUEReceiveEvent_0400";

    Want wantTest;
    wantTest.SetAction(eventName);
    CommonEventData commonEventData(wantTest);

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(true);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    mtx_.lock();
    EXPECT_EQ(CommonEventManager::PublishCommonEvent(commonEventData, publishInfo), true);

    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!mtx_.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_LIMIT) {
            result = false;
            break;
        }
    }
    // expect the subscriber could receive the event within 5 seconds.
    EXPECT_TRUE(result);
    mtx_.unlock();
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_ReceiveEvent_0500
 * @tc.name: OnReceiveEvent
 * @tc.desc: Verify add entity with sticky is false
 */
HWTEST_F(cesSystemTest, CES_ReceiveEvent_0500, Function | MediumTest | Level1)
{
    bool result = true;
    std::string eventName = "TESTEVENT_RECEIVE_ENTITY_INFO_FALSE";
    std::string entity = "ADDENTITY";

    Want wantTest;
    wantTest.SetAction(eventName);
    wantTest.AddEntity(entity);
    CommonEventData commonEventData(wantTest);

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(false);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    mtx_.lock();
    EXPECT_EQ(CommonEventManager::PublishCommonEvent(commonEventData, publishInfo), true);

    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!mtx_.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_LIMIT) {
            result = false;
            break;
        }
    }
    // The publisher sets the Entity, the receiver must set it, otherwise the receiver will not receive the information
    EXPECT_FALSE(result);
    mtx_.unlock();
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_ReceiveEvent_0600
 * @tc.name:OnReceiveEvent
 * @tc.desc:  Verify add entity with sticky is true
 */
HWTEST_F(cesSystemTest, CES_ReceiveEvent_0600, Function | MediumTest | Level1)
{
    bool result = true;
    std::string eventName = "testEventReceiveEntityInfoFalse";
    std::string entity = "addEntity";

    Want wantTest;
    wantTest.SetAction(eventName);
    wantTest.AddEntity(entity);
    CommonEventData commonEventData(wantTest);

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(true);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    mtx_.lock();
    EXPECT_EQ(CommonEventManager::PublishCommonEvent(commonEventData, publishInfo), true);

    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!mtx_.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_LIMIT) {
            result = false;
            break;
        }
    }
    // expect the subscriber could receive the event within 5 seconds.
    EXPECT_TRUE(result);
    mtx_.unlock();
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SubscriptionEventTheme_0100
 * @tc.name: AddEvent
 * @tc.desc: Verify add an event Theme
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEventTheme_0100, Function | MediumTest | Level1)
{
    std::string eventName = "TESTADDTHEME";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SubscriptionEventTheme_0200
 * @tc.name: AddEvent
 * @tc.desc: Verify add multiple enent themes
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEventTheme_0200, Function | MediumTest | Level1)
{
    std::string eventName1 = "TESTADDTHEME1";
    std::string eventName2 = "TESTADDTHEME2";

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName1);
    matchingSkills.AddEvent(eventName2);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SubscriptionEventTheme_0300
 * @tc.name: MatchEvent
 * @tc.desc: Verify march an event theme
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEventTheme_0300, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_MATCHEVENTTEST";

    Want wantTest;
    wantTest.SetAction(eventName);
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);

    EXPECT_EQ(matchingSkills.Match(wantTest), true);
}

/*
 * @tc.number: CES_SubscriptionEventTheme_0400
 * @tc.name: MatchEvent
 * @tc.desc: Verify march other event theme
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEventTheme_0400, Function | MediumTest | Level1)
{
    std::string eventName = "TESTMATCHEVENTTOPICAL";
    std::string eventNameCompare = "TESTMATCHEVENTTOPICAL_COMPARE";

    Want wantTest;
    wantTest.SetAction(eventNameCompare);
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);

    EXPECT_EQ(matchingSkills.Match(wantTest), false);
}

/*
 * @tc.number: CES_SendEvent_1300
 * @tc.name: PublishCommonEvent
 * @tc.desc: The publisher can send normally, but does not have permission
 * to send system events and cannot receive published system events
 * This test case has been covered in the module test, and the system test
 * cannot simulate the non-subsystem scenario
 */
HWTEST_F(cesSystemTest, CES_SendEvent_1300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "start CES_SendEvent_1300";
    bool sysResult = false;
    std::string eventName = CommonEventSupport::COMMON_EVENT_ABILITY_ADDED;

    Want wantTest;
    wantTest.SetAction(eventName);
    CommonEventData commonEventData(wantTest);

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    CommonEventPublishInfo publishInfo;

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    mtx_.lock();
    // The publisher can send normally, but does not have permission to send system events
    EXPECT_EQ(CommonEventManager::PublishCommonEvent(commonEventData, publishInfo), true);

    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;

    while (!mtx_.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_) {
            sysResult = true;
            break;
        }
    }
    // Unable to receive published system events, failed to send system events
    EXPECT_FALSE(sysResult);
    mtx_.unlock();
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
    GTEST_LOG_(INFO) << "end CES_SendEvent_1300";
}

/*
 * @tc.number: CES_SendEvent_1400
 * @tc.name: PublishCommonEvent
 * @tc.desc: The publisher can send normally, but does not have permission
 * to send system events and cannot receive published system events
 * This test case has been covered in the module test, and the system test
 * cannot simulate the non-subsystem scenario
 */
HWTEST_F(cesSystemTest, CES_SendEvent_1400, Function | MediumTest | Level1)
{
    bool sysResult = false;
    std::string eventName = CommonEventSupport::COMMON_EVENT_ABILITY_REMOVED;

    Want wantTest;
    wantTest.SetAction(eventName);
    CommonEventData commonEventData(wantTest);

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    CommonEventPublishInfo publishInfo;

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    mtx_.lock();
    // The publisher can send normally, but does not have permission to send system events
    EXPECT_EQ(CommonEventManager::PublishCommonEvent(commonEventData, publishInfo), true);

    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;

    while (!mtx_.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_) {
            sysResult = true;
            break;
        }
    }
    // Unable to receive published system events, failed to send system events
    EXPECT_FALSE(sysResult);
    mtx_.unlock();
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SendEvent_1500
 * @tc.name: PublishCommonEvent
 * @tc.desc: The publisher can send normally, but does not have permission
 * to send system events and cannot receive published system events
 * This test case has been covered in the module test, and the system test
 * cannot simulate the non-subsystem scenario
 */
HWTEST_F(cesSystemTest, CES_SendEvent_1500, Function | MediumTest | Level1)
{
    bool sysResult = false;
    std::string eventName = CommonEventSupport::COMMON_EVENT_ABILITY_UPDATED;

    Want wantTest;
    wantTest.SetAction(eventName);
    CommonEventData commonEventData(wantTest);

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    CommonEventPublishInfo publishInfo;

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    mtx_.lock();
    // The publisher can send normally, but does not have permission to send system events
    EXPECT_EQ(CommonEventManager::PublishCommonEvent(commonEventData, publishInfo), true);

    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;

    while (!mtx_.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_) {
            sysResult = true;
            break;
        }
    }
    // Unable to receive published system events, failed to send system events
    EXPECT_FALSE(sysResult);
    mtx_.unlock();
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SendEvent_1600
 * @tc.name: PublishCommonEvent
 * @tc.desc: publisher cannot receive published system events
 * This test case has been covered in the module test, and the system test
 * cannot simulate the non-subsystem scenario
 */
HWTEST_F(cesSystemTest, CES_SendEvent_1600, Function | MediumTest | Level1)
{
    bool result = true;
    std::string eventName = CommonEventSupport::COMMON_EVENT_ACCOUNT_DELETED;

    Want wantTest;
    wantTest.SetAction(eventName);
    CommonEventData commonEventData(wantTest);

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    CommonEventPublishInfo publishInfo;

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    mtx_.lock();
    EXPECT_EQ(CommonEventManager::PublishCommonEvent(commonEventData, publishInfo), true);

    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!mtx_.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_LIMIT) {
            result = false;
            break;
        }
    }
    // System events published by ordinary publishers, the publication fails, and the receiver cannot receive it
    EXPECT_TRUE(result);
    mtx_.unlock();
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SendEvent_1700
 * @tc.name: PublishCommonEvent
 * @tc.desc: Both subscribers subscribe to the event, after the event is published, both the subscribers can receive the
 *           event
 */
HWTEST_F(cesSystemTest, CES_SendEvent_1700, Function | MediumTest | Level1)
{
    struct tm startTime = {0};
    std::string eventName = COMPARE_STR;

    Want wantTest;
    wantTest.SetAction(eventName);
    CommonEventData commonEventData(wantTest);
    commonEventData.SetCode(g_CODE_COMPARE1);

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    auto subscriberPtr1 = std::make_shared<CommonEventServicesSystemTestSubscriber>(subscribeInfo);

    CommonEventPublishInfo publishInfo;

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr1), true);
    mtx_.lock();
    mtx2_.lock();
    EXPECT_EQ(CommonEventManager::PublishCommonEvent(commonEventData, publishInfo), true);

    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!mtx_.try_lock()) {
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_TWO) {
            break;
        }
    }
    while (!mtx2_.try_lock()) {
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_TWO) {
            break;
        }
    }

    mtx_.unlock();
    mtx2_.unlock();
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr1), true);
}

/*
 * @tc.number: CES_SendEvent_1800
 * @tc.name: PublishCommonEvent
 * @tc.desc: One subscriber subscribe to the event and another subscriber does not subscribe to the event,
 *           after the event is published, subscribed subscriber can receive the event and unsubscribed
 *           subscribe can not receive the event
 */
HWTEST_F(cesSystemTest, CES_SendEvent_1800, Function | MediumTest | Level1)
{
    struct tm startTime = {0};
    std::string eventName = COMPARE_STR_FALSE;

    Want wantTest;
    wantTest.SetAction(eventName);
    CommonEventData commonEventData(wantTest);
    commonEventData.SetCode(g_CODE_COMPARE3);

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    CommonEventPublishInfo publishInfo;

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    mtx_.lock();
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    EXPECT_EQ(CommonEventManager::PublishCommonEvent(commonEventData, publishInfo), true);

    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!mtx_.try_lock()) {
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_TWO) {
            break;
        }
    }
    mtx_.unlock();
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SetEventAuthority_0100
 * @tc.name: SetPermission
 * @tc.desc: Set permission for common event subscribers and verify successfully subscribe to common events
 */
HWTEST_F(cesSystemTest, CES_SetEventAuthority_0100, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_SUBSCRIBER_PERMISSION";
    std::string permissin = "PERMISSION";

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPermission(permissin);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SetEventAuthority_0200
 * @tc.name:Set permission and priority for common event subscribers
 * @tc.desc: Set permission and priority for common event subscribers and  verify  successfully subscribe to
 * common events
 */
HWTEST_F(cesSystemTest, CES_SetEventAuthority_0200, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_SUBSCRIBER_PERMISSION_PRIORITY";
    std::string permissin = "PERMISSION";

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPermission(permissin);
    subscribeInfo.SetPriority(1);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SetEventAuthority_0300
 * @tc.name: SetPermission SetPriority and SetDeviceId
 * @tc.desc: Set permission and priority and DeviceId for common event subscribers and  verify  successfully
 * subscribe to common events
 */
HWTEST_F(cesSystemTest, CES_SetEventAuthority_0300, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_SUBSCRIBER_PERMISSION_PRIORITY_D";
    std::string permissin = "PERMISSION";
    std::string deviceId = "deviceId";

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPermission(permissin);
    subscribeInfo.SetPriority(1);
    subscribeInfo.SetDeviceId(deviceId);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SetEventAuthority_0400
 * @tc.name: SetPermission
 * @tc.desc: Set permission for common event subscribers and verify successfully Unsubscribe to common events
 */
HWTEST_F(cesSystemTest, CES_SetEventAuthority_0400, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_UNSUBSCRIBER_PERMISSION";
    std::string permissin = "PERMISSION";

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPermission(permissin);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SetEventAuthority_0500
 * @tc.name: SetPermission and  SetPriority
 * @tc.desc: Set permission and priority for common event subscribers and verify successfully Unsubscribe to
 * common events
 */
HWTEST_F(cesSystemTest, CES_SetEventAuthority_0500, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_UNSUBSCRIBER_PERMISSION_PRIORITY";
    std::string permissin = "PERMISSION";

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPermission(permissin);
    subscribeInfo.SetPriority(1);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SetEventAuthority_0600
 * @tc.name: SetPermission SetPriority and SetDeviceId
 * @tc.desc: Set permission and priority and DeviceId for common event subscribers and  verify  successfully
 * Unsubscribe to common events
 */
HWTEST_F(cesSystemTest, CES_SetEventAuthority_0600, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_UNSUBSCRIBER_PERMISSION_PRIORITY_D";
    std::string permissin = "PERMISSION";
    std::string deviceId = "deviceId";

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPermission(permissin);
    subscribeInfo.SetPriority(1);
    subscribeInfo.SetDeviceId(deviceId);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SetEventAuthority_0700
 * @tc.name: SetSubscriberPermissions
 * @tc.desc: Set permission for common event subscribers and  verify  successfully publish common events
 */
HWTEST_F(cesSystemTest, CES_SetEventAuthority_0700, Function | MediumTest | Level1)
{
    struct tm startTime = {0};
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_PERMISSION";
    std::string permissin = "PERMISSION";

    Want wantTest;
    wantTest.SetAction(eventName);
    CommonEventData commonEventData(wantTest);

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    std::vector<std::string> permissins;
    permissins.emplace_back(permissin);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSubscriberPermissions(permissins);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    mtx_.lock();
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    EXPECT_EQ(CommonEventManager::PublishCommonEvent(commonEventData, publishInfo), true);

    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!mtx_.try_lock()) {
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_TWO) {
            break;
        }
    }
    mtx_.unlock();
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SetEventAuthority_0800
 * @tc.name: SetSubscriberPermissions
 * @tc.desc: Set permission for common event subscribers and  verify  successfully receive common events
 */
HWTEST_F(cesSystemTest, CES_SetEventAuthority_0800, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_PERMISSION_R";
    std::string permissin = "PERMISSION";

    Want wantTest;
    wantTest.SetAction(eventName);
    CommonEventData commonEventData(wantTest);

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    std::vector<std::string> permissins;
    permissins.emplace_back(permissin);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSubscriberPermissions(permissins);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    mtx_.lock();
    EXPECT_EQ(CommonEventManager::PublishCommonEvent(commonEventData, publishInfo), true);

    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!mtx_.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_LIMIT) {
            result = true;
            break;
        }
    }
    EXPECT_TRUE(result);
    mtx_.unlock();
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SetEventAuthority_0900
 * @tc.name: SetThreadMode
 * @tc.desc: Set ThreadMode for common event subscribers and  verify  successfully subscribe to common events
 */
HWTEST_F(cesSystemTest, CES_SetEventAuthority_0900, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_SUBSCRIBER_SETTHREADMODE";

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetThreadMode(CommonEventSubscribeInfo::ThreadMode::HANDLER);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SetEventAuthority_1000
 * @tc.name: SetThreadMode
 * @tc.desc: Set ThreadMode for common event subscribers and  verify  successfully Unsubscribe to common events
 */
HWTEST_F(cesSystemTest, CES_SetEventAuthority_1000, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_UNSUBSCRIBER_SETTHREADMODE";

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetThreadMode(CommonEventSubscribeInfo::ThreadMode::HANDLER);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SetEventAuthority_1100
 * @tc.name: SetThreadMode
 * @tc.desc: Set ThreadMode for common event subscribers and  verify  successfully publish to common events
 */
HWTEST_F(cesSystemTest, CES_SetEventAuthority_1100, Function | MediumTest | Level1)
{
    struct tm startTime = {0};
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_SETTHREADMODE";

    Want wantTest;
    wantTest.SetAction(eventName);
    CommonEventData commonEventData(wantTest);

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetThreadMode(CommonEventSubscribeInfo::ThreadMode::HANDLER);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);

    EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtr), true);
    mtx_.lock();
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    EXPECT_EQ(CommonEventManager::PublishCommonEvent(commonEventData), true);

    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!mtx_.try_lock()) {
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_TWO) {
            break;
        }
    }
    mtx_.unlock();
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_SetEventAuthority_1200
 * @tc.name: SetThreadMode
 * @tc.desc: Set ThreadMode for common event subscribers and  verify  successfully receive  to common events
 */
HWTEST_F(cesSystemTest, CES_SetEventAuthority_1200, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_SETHANDLER_HANDLER_STCES";

    Want wantTest;
    wantTest.SetAction(eventName);
    CommonEventData commonEventData(wantTest);

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetThreadMode(CommonEventSubscribeInfo::ThreadMode::POST);
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
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtr), true);
}

/*
 * @tc.number: CES_VerifyMatchingSkills_0100
 * @tc.name: GetEvent
 * @tc.desc: check to get the added event
 */
HWTEST_F(cesSystemTest, CES_VerifyMatchingSkills_0100, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_GETMATCHINGSKILLS";

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    MatchingSkills testMatching = subscribeInfo.GetMatchingSkills();

    EXPECT_EQ(testMatching.GetEvent(0), eventName);
}

/*
 * @tc.number: CES_VerifyMatchingSkills_0200
 * @tc.name: GetEntity
 * @tc.desc: check to get the added entity
 */
HWTEST_F(cesSystemTest, CES_VerifyMatchingSkills_0200, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_ADDENTITY_GETENTITY";
    std::string entity = "entity";

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    matchingSkills.AddEntity(entity);

    EXPECT_EQ(matchingSkills.GetEntity(0), entity);
}

/*
 * @tc.number: CES_VerifyMatchingSkills_0300
 * @tc.name: HasEntity
 * @tc.desc: verify that entity is in MatchingSkills
 */
HWTEST_F(cesSystemTest, CES_VerifyMatchingSkills_0300, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_ADDENTITY_HASENTITY";
    std::string entity = "entity";

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    matchingSkills.AddEntity(entity);

    EXPECT_TRUE(matchingSkills.HasEntity(entity));
}

/*
 * @tc.number: CES_VerifyMatchingSkills_0400
 * @tc.name: RemoveEntity
 * @tc.desc: verify that the entity was successfully removed
 */
HWTEST_F(cesSystemTest, CES_VerifyMatchingSkills_0400, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_ADDENTITY_REMOVEENTITY";
    std::string entity = "entity";

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    matchingSkills.AddEntity(entity);
    matchingSkills.RemoveEntity(entity);

    EXPECT_FALSE(matchingSkills.HasEntity(entity));
}

/*
 * @tc.number: CES_VerifyMatchingSkills_0500
 * @tc.name: CountEntities
 * @tc.desc: verify that count correct number of entities
 */
HWTEST_F(cesSystemTest, CES_VerifyMatchingSkills_0500, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_ADDENTITY_ENTITYCOUNT";
    std::string entity = "entity";

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    matchingSkills.AddEntity(entity);

    EXPECT_TRUE((matchingSkills.CountEntities() >= 1));
}

/*
 * @tc.number: CES_SubscriberMaxSize_0200
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify that the maximum number of subscriptions for a single third-party application is 200
 */
HWTEST_F(cesSystemTest, CES_SubscriberMaxSize_0100, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_SUBSCRIBER";

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    std::vector<std::shared_ptr<CommonEventServicesSystemTest>> subscriberPtrs;

    for (size_t i = 0; i < SUBSCRIBER_MAX_SIZE; i++) {
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        subscriberPtrs.push_back(subscriberPtr);
        EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtrs.at(i)), true);
    }
    for (size_t i = 0; i < SUBSCRIBER_MAX_SIZE; i++) {
        EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtrs.at(i)), true);
    }
}

/*
 * @tc.number: CES_SubscriberMaxSize_0200
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify that the maximum number of subscriptions for a single third-party application is 200 and the 201st
 * subscription failed
 */
HWTEST_F(cesSystemTest, CES_SubscriberMaxSize_0200, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_SUBSCRIBER_PLUS";

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    std::vector<std::shared_ptr<CommonEventServicesSystemTest>> subscriberPtrs;

    for (size_t i = 0; i < SUBSCRIBER_MAX_SIZE_PLUS - 1; i++) {
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        subscriberPtrs.push_back(subscriberPtr);
        EXPECT_EQ(CommonEventManager::SubscribeCommonEvent(subscriberPtrs.at(i)), true);
    }

    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    EXPECT_FALSE(CommonEventManager::SubscribeCommonEvent(subscriberPtr));

    for (size_t i = 0; i < SUBSCRIBER_MAX_SIZE_PLUS - 1; i++) {
        EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberPtrs.at(i)), true);
    }
}

/*
 * @tc.number: CES_PublishOrderTimeOut_0100
 * @tc.name: OnReceiveEvent
 * @tc.desc: Verify allowing a receiver to run for ten seconds before giving up on it
 */
HWTEST_F(cesSystemTest, CES_PublishOrderTimeOut_0100, Function | MediumTest | Level1)
{
    struct tm startTime = {0};
    std::string eventNameTimeOut = "TESTEVENT_PUBLISHORDER_TIMEOUT";

    Want wantTimeOut;
    wantTimeOut.SetAction(eventNameTimeOut);
    CommonEventData commonEventDataTimeOut(wantTimeOut);

    CommonEventPublishInfo publishInfoTimeOut;
    publishInfoTimeOut.SetOrdered(true);

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventNameTimeOut);

    CommonEventSubscribeInfo subscribeInfoOne(matchingSkills);
    subscribeInfoOne.SetPriority(PRIORITY_HIGH);
    auto subscriberTimeOnePtr = std::make_shared<CESPublishOrderTimeOutOne>(subscribeInfoOne);
    EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberTimeOnePtr));

    CommonEventSubscribeInfo subscribeInfoTwo(matchingSkills);
    subscribeInfoTwo.SetPriority(PRIORITY_LOW);
    auto subscriberTimeTwoPtr = std::make_shared<CESPublishOrderTimeOutTwo>(subscribeInfoTwo);
    EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberTimeTwoPtr));

    mtx_.lock();
    EXPECT_TRUE(CommonEventManager::PublishCommonEvent(commonEventDataTimeOut, publishInfoTimeOut));

    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!mtx_.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_TWENTY) {
            break;
        }
    }
    // expect the subscriber could receive the event within 11 seconds.
    std::cout << "seconds: " << seconds << std::endl;
    EXPECT_GE(seconds, TIME_OUT_SECONDS_TEN);

    mtx_.unlock();
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberTimeOnePtr), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberTimeTwoPtr), true);
}

/*
 * @tc.number: CES_PublishOrderTimeOut_0200
 * @tc.name: OnReceiveEvent
 * @tc.desc: Set the last subscriber and Verify allowing a receiver to run for ten seconds before giving up on it
 */
HWTEST_F(cesSystemTest, CES_PublishOrderTimeOut_0200, Function | MediumTest | Level1)
{
    struct tm startTime = {0};
    std::string eventNameTimeOut = "TESTEVENT_PUBLISHORDER_TIMEOUT_SUBSCRIBER";

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventNameTimeOut);

    Want wantTimeOut;
    wantTimeOut.SetAction(eventNameTimeOut);
    CommonEventData commonEventDataTimeOut(wantTimeOut);

    CommonEventPublishInfo publishInfoTimeOut;
    publishInfoTimeOut.SetOrdered(true);

    CommonEventSubscribeInfo subscribeInfoOne(matchingSkills);
    subscribeInfoOne.SetPriority(PRIORITY_HIGH);
    auto subscriberTimeOnePtr = std::make_shared<CESPublishOrderTimeOutOne>(subscribeInfoOne);
    EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberTimeOnePtr));

    CommonEventSubscribeInfo subscribeInfoTwo(matchingSkills);
    subscribeInfoTwo.SetPriority(PRIORITY_LOW);
    auto subscriberTimeTwoPtr = std::make_shared<CESPublishOrderTimeOutTwo>(subscribeInfoTwo);
    EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberTimeTwoPtr));

    mtx_.lock();
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    EXPECT_TRUE(
        CommonEventManager::PublishCommonEvent(commonEventDataTimeOut, publishInfoTimeOut, subscriberTimeTwoPtr));
    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!mtx_.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_TWENTY) {
            break;
        }
    }
    // expect the subscriber could receive the event within 11 seconds.
    std::cout << "seconds: " << seconds << std::endl;
    EXPECT_GE(seconds, TIME_OUT_SECONDS_TEN);
    mtx_.unlock();
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberTimeOnePtr), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberTimeTwoPtr), true);
}

/*
 * @tc.number: CES_PublishOrderTimeOut_0300
 * @tc.name: OnReceiveEvent
 * @tc.desc: Without setting priority for subscribers and Verify allowing a receiver to run for ten seconds before
 * giving up on it
 */
HWTEST_F(cesSystemTest, CES_PublishOrderTimeOut_0300, Function | MediumTest | Level1)
{
    struct tm startTime = {0};
    std::string eventNameTimeOut = "TESTEVENT_PUBLISHORDER_TIMEOUT_NOPRIORITY";

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventNameTimeOut);

    Want wantTimeOut;
    wantTimeOut.SetAction(eventNameTimeOut);
    CommonEventData commonEventDataTimeOut(wantTimeOut);

    CommonEventPublishInfo publishInfoTimeOut;
    publishInfoTimeOut.SetOrdered(true);

    CommonEventSubscribeInfo subscribeInfoOne(matchingSkills);
    auto subscriberTimeOnePtr = std::make_shared<CESPublishOrderTimeOutOne>(subscribeInfoOne);
    EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberTimeOnePtr));

    CommonEventSubscribeInfo subscribeInfoTwo(matchingSkills);
    auto subscriberTimeTwoPtr = std::make_shared<CESPublishOrderTimeOutTwo>(subscribeInfoTwo);
    EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberTimeTwoPtr));

    mtx_.lock();
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    EXPECT_TRUE(CommonEventManager::PublishCommonEvent(commonEventDataTimeOut, publishInfoTimeOut));
    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!mtx_.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_TWENTY) {
            break;
        }
    }
    // expect the subscriber could receive the event within 11 seconds.
    std::cout << "seconds: " << seconds << std::endl;
    EXPECT_GE(seconds, TIME_OUT_SECONDS_TEN);
    mtx_.unlock();
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberTimeOnePtr), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberTimeTwoPtr), true);
}

/*
 * @tc.number: CES_PublishOrderTimeOut_0400
 * @tc.name: OnReceiveEvent
 * @tc.desc: Set the last subscriber and Without setting priority for subscribers, Verify allowing a receiver to run
 * for ten seconds before giving up on it
 */
HWTEST_F(cesSystemTest, CES_PublishOrderTimeOut_0400, Function | MediumTest | Level1)
{
    struct tm startTime = {0};
    std::string eventNameTimeOut = "TESTEVENT_PUBLISHORDER_TIMEOUT_SUBSCRIBER_NOPRIORITY";

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventNameTimeOut);

    Want wantTimeOut;
    wantTimeOut.SetAction(eventNameTimeOut);
    CommonEventData commonEventDataTimeOut(wantTimeOut);

    CommonEventPublishInfo publishInfoTimeOut;
    publishInfoTimeOut.SetOrdered(true);

    CommonEventSubscribeInfo subscribeInfoOne(matchingSkills);
    auto subscriberTimeOnePtr = std::make_shared<CESPublishOrderTimeOutOne>(subscribeInfoOne);
    EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberTimeOnePtr));

    CommonEventSubscribeInfo subscribeInfoTwo(matchingSkills);
    auto subscriberTimeTwoPtr = std::make_shared<CESPublishOrderTimeOutTwo>(subscribeInfoTwo);
    EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberTimeTwoPtr));

    mtx_.lock();
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    EXPECT_TRUE(
        CommonEventManager::PublishCommonEvent(commonEventDataTimeOut, publishInfoTimeOut, subscriberTimeTwoPtr));
    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!mtx_.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_TWENTY) {
            break;
        }
    }
    // expect the subscriber could receive the event within 11 seconds.
    std::cout << "seconds: " << seconds << std::endl;
    EXPECT_GE(seconds, TIME_OUT_SECONDS_TEN);

    mtx_.unlock();
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberTimeOnePtr), true);
    EXPECT_EQ(CommonEventManager::UnSubscribeCommonEvent(subscriberTimeTwoPtr), true);
}

/*
 * @tc.number: CES_PermissionAndOrDefault_0100
 * @tc.name: OnReceiveEvent
 * @tc.desc: Verify system applications handle system public events with permissions
 */
HWTEST_F(cesSystemTest, CES_PermissionAndOrDefault_0100, Function | MediumTest | Level1)
{
    std::string eventAndN = "COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_DISCOVERED";
    std::string eventDefaultN = "COMMON_EVENT_USER_SWITCHED";

    Want wantAnd;
    wantAnd.SetAction(eventAndN);
    CommonEventData commonEventDataAnd(wantAnd);
    CommonEventPublishInfo publishInfoAnd;
    EXPECT_EQ(CommonEventManager::PublishCommonEvent(commonEventDataAnd, publishInfoAnd), true);

    Want wantDefaultN;
    wantDefaultN.SetAction(eventDefaultN);
    CommonEventData commonEventDataDefaultN(wantDefaultN);
    CommonEventPublishInfo publishInfoDefaultN;
    EXPECT_EQ(CommonEventManager::PublishCommonEvent(commonEventDataDefaultN, publishInfoDefaultN), true);
}
}  // namespace EventFwk
}  // namespace OHOS