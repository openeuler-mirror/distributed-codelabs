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

#include <thread>
#include <chrono>

#include <sys/time.h>

// redefine private and protected since testcase need to invoke and test private function
#define private public
#define protected public
#include "common_event.h"
#include "common_event_manager.h"
#include "common_event_stub.h"
#include "common_event_subscriber_manager.h"
#include "inner_common_event_manager.h"
#include "common_event_publish_info.h"
#include "common_event_subscribe_info.h"
#include "matching_skills.h"
#undef private
#undef protected

#include "datetime_ex.h"

#include <gtest/gtest.h>

using namespace testing::ext;
using namespace OHOS::EventFwk;
using OHOS::Parcel;

namespace {
const std::string EVENT = "com.ces.test.event";
std::mutex mtx;
const time_t TIME_OUT_SECONDS_LIMIT = 5;
constexpr uint16_t SYSTEM_UID = 1000;
constexpr int32_t ERR_COMMON = -1;
}  // namespace

class SubscriberTest;

class CommonEventSubscribeTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

public:
    static constexpr int TEST_WAIT_TIME = 100000;
    CommonEventManager commonEventManager;
    MatchingSkills matchingSkills;
};

class SubscriberTest : public CommonEventSubscriber {
public:
    explicit SubscriberTest(const CommonEventSubscribeInfo &sp) : CommonEventSubscriber(sp)
    {}

    ~SubscriberTest()
    {}

    virtual void OnReceiveEvent(const CommonEventData &data)
    {
        mtx.unlock();
    }
};

class EventReceiveStubTest : public EventReceiveStub {
public:
    EventReceiveStubTest()
    {}

    ~EventReceiveStubTest()
    {}

    virtual void NotifyEvent(const CommonEventData &commonEventData, const bool &ordered, const bool &sticky)
    {}
};

class CommonEventStubTest : public CommonEventStub {
public:
    CommonEventStubTest()
    {}

    virtual int32_t PublishCommonEvent(const CommonEventData &event, const CommonEventPublishInfo &publishinfo,
        const OHOS::sptr<IRemoteObject> &commonEventListener, const int32_t &userId)
    {
        return ERR_COMMON;
    }

    virtual int32_t SubscribeCommonEvent(
        const CommonEventSubscribeInfo &subscribeInfo, const OHOS::sptr<IRemoteObject> &commonEventListener)
    {
        return ERR_COMMON;
    }

    virtual int32_t UnsubscribeCommonEvent(const OHOS::sptr<IRemoteObject> &commonEventListener)
    {
        return ERR_COMMON;
    }

    virtual bool DumpState(const uint8_t &dumpType, const std::string &event, const int32_t &userId,
        std::vector<std::string> &state)
    {
        return false;
    }

    virtual ~CommonEventStubTest()
    {}

    virtual bool FinishReceiver(const OHOS::sptr<IRemoteObject> &proxy, const int32_t &code,
        const std::string &receiverData, const bool &abortEvent)
    {
        return false;
    }
};

void CommonEventSubscribeTest::SetUpTestCase(void)
{}

void CommonEventSubscribeTest::TearDownTestCase(void)
{}

void CommonEventSubscribeTest::SetUp(void)
{}

void CommonEventSubscribeTest::TearDown(void)
{}

/*
 * Feature: CommonEventSubscribeTest
 * Function:CommonEvent SubscribeCommonEvent
 * SubFunction: Subscribe common event
 * FunctionPoints: test subscribe event
 * EnvConditions: system run normally
 * CaseDescription:  1. subscribe common event
 *                   2. success subscribe common event with right parameters
 */

HWTEST_F(CommonEventSubscribeTest, CommonEventSubscribe_001, TestSize.Level1)
{
    /* Subscribe */
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);
    bool subscribeResult = CommonEventManager::SubscribeCommonEvent(subscriber);

    EXPECT_EQ(true, subscribeResult);

    /* Publish */

    // make a want
    Want want;
    want.SetAction(EVENT);

    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // lock the mutex
    mtx.lock();

    // publish a common event
    bool publishResult = CommonEventManager::PublishCommonEvent(data);

    EXPECT_EQ(true, publishResult);

    // record start time of publishing
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);

    // record current time
    struct tm doingTime = {0};
    int64_t seconds = 0;

    while (!mtx.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_LIMIT) {
            break;
        }
    }

    // expect the subscriber could receive the event within 5 seconds.
    EXPECT_LT(seconds, TIME_OUT_SECONDS_LIMIT);
    mtx.unlock();
}

/*
 * Feature: CommonEventSubscribeTest
 * Function:CommonEventSubscriberManager InsertSubscriber
 * SubFunction: Subscribe common event
 * FunctionPoints: test subscribe event
 * EnvConditions: system run normally
 * CaseDescription:  1. subscribe common event
 *                   2. different subscriber subscribe event
 *                   3. success subscribe common event with right parameters
 */
HWTEST_F(CommonEventSubscribeTest, CommonEventSubscribe_002, TestSize.Level1)
{
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    std::shared_ptr<SubscriberTest> subscribera = std::make_shared<SubscriberTest>(subscribeInfo);
    std::shared_ptr<SubscriberTest> subscriberb = std::make_shared<SubscriberTest>(subscribeInfo);

    bool subscribeResulta = CommonEventManager::SubscribeCommonEvent(subscribera);

    EXPECT_EQ(true, subscribeResulta);

    bool subscribeResultb = CommonEventManager::SubscribeCommonEvent(subscriberb);

    EXPECT_EQ(true, subscribeResultb);
}

/*
 * Feature: CommonEventSubscribeTest
 * Function:CommonEvent SubscribeCommonEvent
 * SubFunction: Subscribe common event
 * FunctionPoints: test subscribe event
 * EnvConditions: system run normally
 * CaseDescription:  1. subscribe common event
 *                   2. fail subscribe common event kit with null subscriber
 */

HWTEST_F(CommonEventSubscribeTest, CommonEventSubscribe_003, TestSize.Level1)
{
    bool subscribeResult = CommonEventManager::SubscribeCommonEvent(nullptr);

    EXPECT_EQ(false, subscribeResult);
}

/*
 * Feature: CommonEventSubscribeTest
 * Function:CommonEvent SubscribeCommonEvent
 * SubFunction: Subscribe common event
 * FunctionPoints: test subscribe event
 * EnvConditions: system run normally
 * CaseDescription:  1. subscribe common event
 *                   2. fail subscribe common event with no event
 */
HWTEST_F(CommonEventSubscribeTest, CommonEventSubscribe_004, TestSize.Level1)
{
    MatchingSkills matchingSkills;
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);

    bool subscribeResult = CommonEventManager::SubscribeCommonEvent(subscriber);

    EXPECT_EQ(false, subscribeResult);
}

/*
 * Feature: CommonEventSubscribeTest
 * Function:CommonEvent SubscribeCommonEvent
 * SubFunction: Subscribe common event
 * FunctionPoints: test subscribe event
 * EnvConditions: system run normally
 * CaseDescription:  1. subscribe common event
 *                   2. fail subscribe common event because common event listener has subsrciber
 */
HWTEST_F(CommonEventSubscribeTest, CommonEventSubscribe_005, TestSize.Level1)
{
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);

    CommonEventManager::SubscribeCommonEvent(subscriber);

    bool subscribeResult = CommonEventManager::SubscribeCommonEvent(subscriber);

    EXPECT_EQ(true, subscribeResult);
}

/*
 * Feature: CommonEventSubscribeTest
 * Function: InnerCommonEventManager SubscribeCommonEvent
 * SubFunction: Subscribe common event
 * FunctionPoints: test subscribe event
 * EnvConditions: system run normally
 * CaseDescription:  1. subscribe common event
 *                   2. fail subscribe common event, inner common event manager
 *                      common event listener is null
 */
HWTEST_F(CommonEventSubscribeTest, CommonEventSubscribe_009, TestSize.Level1)
{
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    std::shared_ptr<InnerCommonEventManager> innerCommonEventManager = std::make_shared<InnerCommonEventManager>();
    OHOS::sptr<OHOS::IRemoteObject> sp(nullptr);

    struct tm curTime {0};
    OHOS::Security::AccessToken::AccessTokenID tokenID = 1;

    EXPECT_EQ(false, innerCommonEventManager->SubscribeCommonEvent(subscribeInfo, sp, curTime, 0, 0, tokenID, ""));
}

/*
 * Feature: CommonEventSubscribeTest
 * Function: CommonEventSubscriberManager InsertSubscriber
 * SubFunction: Subscribe common event
 * FunctionPoints: test subscribe event
 * EnvConditions: system run normally
 * CaseDescription:  1. subscribe common event
 *                   2. fail subscribe common event , common event subscriber manager
 *                      event subscriber info is null
 */
HWTEST_F(CommonEventSubscribeTest, CommonEventSubscribe_010, TestSize.Level1)
{
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);
    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);
    std::shared_ptr<InnerCommonEventManager> innerCommonEventManager = std::make_shared<InnerCommonEventManager>();
    OHOS::sptr<OHOS::IRemoteObject> commonEventListenerPtr(commonEventListener);

    struct tm curTime {0};
    EventRecordInfo eventRecordInfo;
    eventRecordInfo.pid = 0;
    eventRecordInfo.uid = 0;
    eventRecordInfo.bundleName = "bundleName";

    auto result = OHOS::DelayedSingleton<CommonEventSubscriberManager>::GetInstance()->InsertSubscriber(
        nullptr, commonEventListenerPtr, curTime, eventRecordInfo);

    EXPECT_EQ(nullptr, result);
}

/*
 * Feature: CommonEventSubscribeTest
 * Function: CommonEventSubscriberManager InsertSubscriber
 * SubFunction: Subscribe common event
 * FunctionPoints: test subscribe event
 * EnvConditions: system run normally
 * CaseDescription:  1. subscribe common event
 *                   2. fail subscribe common event , common event subscriber manager
 *                      event common event listener is null
 */
HWTEST_F(CommonEventSubscribeTest, CommonEventSubscribe_011, TestSize.Level1)
{
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    std::shared_ptr<CommonEventSubscribeInfo> commonEventSubscribeInfo =
        std::make_shared<CommonEventSubscribeInfo>(subscribeInfo);

    struct tm curTime {0};
    EventRecordInfo eventRecordInfo;
    eventRecordInfo.pid = 0;
    eventRecordInfo.uid = 0;
    eventRecordInfo.bundleName = "bundleName";

    auto result = OHOS::DelayedSingleton<CommonEventSubscriberManager>::GetInstance()->InsertSubscriber(
        commonEventSubscribeInfo, nullptr, curTime, eventRecordInfo);

    EXPECT_EQ(nullptr, result);
}

/*
 * Feature: CommonEventSubscribeTest
 * Function: CommonEventSubscriberManager InsertSubscriber
 * SubFunction: Subscribe common event
 * FunctionPoints: test subscribe event
 * EnvConditions: system run normally
 * CaseDescription:  1. subscribe common event
 *                   2. fail subscribe common event , common event subscriber manager
 *                      event size is null
 */
HWTEST_F(CommonEventSubscribeTest, CommonEventSubscribe_012, TestSize.Level1)
{
    MatchingSkills matchingSkills;
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    std::shared_ptr<CommonEventSubscribeInfo> commonEventSubscribeInfo =
        std::make_shared<CommonEventSubscribeInfo>(subscribeInfo);
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);
    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);
    std::shared_ptr<InnerCommonEventManager> innerCommonEventManager = std::make_shared<InnerCommonEventManager>();
    OHOS::sptr<OHOS::IRemoteObject> commonEventListenerSp(commonEventListener);

    struct tm curTime {0};
    EventRecordInfo eventRecordInfo;
    eventRecordInfo.pid = 0;
    eventRecordInfo.uid = 0;
    eventRecordInfo.bundleName = "bundleName";

    auto result = OHOS::DelayedSingleton<CommonEventSubscriberManager>::GetInstance()->InsertSubscriber(
        commonEventSubscribeInfo, nullptr, curTime, eventRecordInfo);

    EXPECT_EQ(nullptr, result);
}

/*
 * Feature: CommonEventSubscribeTest
 * Function: CommonEventListener IsReady
 * SubFunction: Subscribe common event
 * FunctionPoints: test is ready
 * EnvConditions: system run normally
 * CaseDescription:  1. ready fail because runner is null
 */
HWTEST_F(CommonEventSubscribeTest, CommonEventSubscribe_014, TestSize.Level1)
{
    MatchingSkills matchingSkills;
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    std::shared_ptr<CommonEventSubscribeInfo> commonEventSubscribeInfo =
        std::make_shared<CommonEventSubscribeInfo>(subscribeInfo);
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);
    CommonEventListener commonEventListener(subscriber);
    commonEventListener.runner_ = nullptr;

    bool result = commonEventListener.IsReady();

    EXPECT_EQ(false, result);
}

/*
 * Feature: CommonEventSubscribeTest
 * Function: CommonEventListener IsReady
 * SubFunction: Subscribe common event
 * FunctionPoints: test is ready
 * EnvConditions: system run normally
 * CaseDescription:  1. ready fail because handler is null
 */
HWTEST_F(CommonEventSubscribeTest, CommonEventSubscribe_015, TestSize.Level1)
{
    MatchingSkills matchingSkills;
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    std::shared_ptr<CommonEventSubscribeInfo> commonEventSubscribeInfo =
        std::make_shared<CommonEventSubscribeInfo>(subscribeInfo);
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);
    CommonEventListener commonEventListener(subscriber);
    commonEventListener.handler_ = nullptr;

    bool result = commonEventListener.IsReady();

    EXPECT_EQ(false, result);
}

/*
 * tc.number: CommonEventManager_001
 * tc.name: test Freeze
 * tc.type: FUNC
 * tc.require: issueI5NGO7
 * tc.desc: Invoke Freeze interface verify whether it is normal
 */
HWTEST_F(CommonEventSubscribeTest, CommonEventManager_001, TestSize.Level1)
{
    CommonEventManager commonEventManager;
    bool freeze = commonEventManager.Freeze(SYSTEM_UID);
    EXPECT_EQ(true, freeze);
}

/*
 * tc.number: CommonEventManager_002
 * tc.name: test Unfreeze
 * tc.type: FUNC
 * tc.require: issueI5NGO7
 * tc.desc: Invoke Unfreeze interface verify whether it is normal
 */
HWTEST_F(CommonEventSubscribeTest, CommonEventManager_002, TestSize.Level1)
{
    CommonEventManager commonEventManager;
    bool unfreeze = commonEventManager.Unfreeze(SYSTEM_UID);
    EXPECT_EQ(true, unfreeze);
}

/*
 * tc.number: CommonEventManager_003
 * tc.name: test UnfreezeAll
 * tc.type: FUNC
 * tc.require: issueI5NGO7
 * tc.desc: Invoke UnfreezeAll interface verify whether it is normal
 */
HWTEST_F(CommonEventSubscribeTest, CommonEventManager_003, TestSize.Level1)
{
    CommonEventManager commonEventManager;
    bool unfreezeAll = commonEventManager.UnfreezeAll();
    EXPECT_EQ(true, unfreezeAll);
}

/*
 * tc.number: CommonEventPublishInfo_001
 * tc.name: test ReadFromParcel
 * tc.type: FUNC
 * tc.require: issueI5RULW
 * tc.desc: Invoke ReadFromParcel interface verify whether it is normal
 */
HWTEST_F(CommonEventSubscribeTest, CommonEventPublishInfo_001, TestSize.Level1)
{
    Parcel parcel;
    CommonEventPublishInfo commonEventPublishInfo;
    bool result = commonEventPublishInfo.ReadFromParcel(parcel);
    EXPECT_EQ(result, true);
}

/*
 * tc.number: CommonEventSubscribeInfo_001
 * tc.name: test ReadFromParcel
 * tc.type: FUNC
 * tc.require: issueI5RULW
 * tc.desc: Invoke ReadFromParcel interface verify whether it is normal
 */
HWTEST_F(CommonEventSubscribeTest, CommonEventSubscribeInfo_001, TestSize.Level1)
{
    Parcel parcel;
    CommonEventSubscribeInfo commonEventSubscribeInfo;
    bool result = commonEventSubscribeInfo.ReadFromParcel(parcel);
    EXPECT_EQ(result, false);
}

/*
 * tc.number: MatchingSkills_001
 * tc.name: test ReadFromParcel
 * tc.type: FUNC
 * tc.require: issueI5RULW
 * tc.desc: Invoke ReadFromParcel interface verify whether it is normal
 */
HWTEST_F(CommonEventSubscribeTest, MatchingSkills_001, TestSize.Level1)
{
    Parcel parcel;
    MatchingSkills matchingSkills;

    // write entity
    int32_t value = 1;
    parcel.WriteInt32(value);
    std::vector<std::u16string> actionU16Entity;
    for (std::vector<std::string>::size_type i = 0; i < 3; i++) {
        actionU16Entity.emplace_back(OHOS::Str8ToStr16("test"));
    }
    matchingSkills.WriteVectorInfo(parcel, actionU16Entity);

    //write event
    parcel.WriteInt32(value);
    std::vector<std::u16string> actionU16Event;
    for (std::vector<std::string>::size_type i = 0; i < 3; i++) {
        actionU16Event.emplace_back(OHOS::Str8ToStr16("test"));
    }
    matchingSkills.WriteVectorInfo(parcel, actionU16Event);

    // write scheme
    parcel.WriteInt32(value);
    std::vector<std::u16string> actionU16Scheme;
    for (std::vector<std::string>::size_type i = 0; i < 3; i++) {
        actionU16Scheme.emplace_back(OHOS::Str8ToStr16("test"));
    }
    matchingSkills.WriteVectorInfo(parcel, actionU16Scheme);

    bool result = matchingSkills.ReadFromParcel(parcel);
    EXPECT_EQ(result, true);
}
