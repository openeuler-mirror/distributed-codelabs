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
#include <sys/time.h>
#include <thread>

#define private public
#define protected public
#include "bundle_manager_helper.h"
#include "common_event_control_manager.h"
#include "common_event.h"
#include "common_event_manager.h"
#include "common_event_manager_service.h"
#include "inner_common_event_manager.h"
#undef private
#undef protected
#include "datetime_ex.h"
#include "mock_bundle_manager.h"
#include "singleton.h"
#include "want.h"

#include <gtest/gtest.h>

using namespace testing::ext;

using namespace OHOS::EventFwk;

using OHOS::Parcel;

namespace {
static OHOS::sptr<OHOS::AppExecFwk::MockBundleMgrService> p_bundleObject = nullptr;
const std::string EVENTCASE1 = "com.ces.test.event.case1";
const std::string EVENTCASE2 = "com.ces.test.event.case2";
const std::string EVENTCASE3 = "com.ces.test.event.case3";
const std::string EVENTCASE4 = "com.ces.test.event.case4";
const std::string EVENTCASE5 = "com.ces.test.event.case5";
const std::string EVENTCASE6 = "com.ces.test.event.case6";
const std::string INNITDATA = "com.ces.test.initdata";
const std::string CHANGEDATA = "com.ces.test.changedata";
std::mutex g_mtx;
std::mutex g_mtxAnother;
const time_t TIME_OUT_SECONDS_LIMIT = 5;
const time_t TIME_OUT_SECONDS_MIDDLE = 15;
const int32_t INITCODE = 0;
const int32_t CHANGECODE = 1;
const int32_t LOWPRIORITY = 0;
const int32_t HIGHPRIORITY = 100;
}  // namespace

class CesPublishOrderedEventModuleTest : public testing::Test {
public:
    CesPublishOrderedEventModuleTest()
    {}
    ~CesPublishOrderedEventModuleTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    bool SubscribeCommonEventTest(std::shared_ptr<CommonEventSubscriber> subscriber) const;
    bool UnSubscribeCommonEventTest(const std::shared_ptr<CommonEventSubscriber> &subscriber) const;
    bool PublishOrderedCommonEventTest(const CommonEventData &data, const CommonEventPublishInfo &publishInfo,
        const std::shared_ptr<CommonEventSubscriber> &subscriber) const;
};

class TestHandler : public EventHandler {
public:
    explicit TestHandler(const std::shared_ptr<EventRunner> &runner) : EventHandler(runner)
    {
        GTEST_LOG_(INFO) << "instance created";
    }

    ~TestHandler() override
    {
        GTEST_LOG_(INFO) << "instance destroyed";
    }
};

class Subscriber : public CommonEventSubscriber {
public:
    explicit Subscriber(const CommonEventSubscribeInfo &subscribeInfo) : CommonEventSubscriber(subscribeInfo)
    {}

    ~Subscriber()
    {}

    virtual void OnReceiveEvent(const CommonEventData &data)
    {
        GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::Subscriber OnReceiveEvent ";
        std::string action = data.GetWant().GetAction();
        if (action == EVENTCASE5) {
            GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::Subscriber OnReceiveEvent EVENTCASE5";
            EXPECT_EQ(GetCode() == INITCODE, true);
            EXPECT_EQ(GetData() == INNITDATA, true);
        } else if (action == EVENTCASE6) {
            GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::Subscriber OnReceiveEvent EVENTCASE6";
            EXPECT_EQ(GetCode() == INITCODE, true);
            EXPECT_EQ(GetData() == INNITDATA, true);
        } else {
            GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::Subscriber OnReceiveEvent do nothing";
        }
    };
};

class SubscriberTest : public CommonEventSubscriber {
public:
    explicit SubscriberTest(const CommonEventSubscribeInfo &subscribeInfo) : CommonEventSubscriber(subscribeInfo)
    {}

    ~SubscriberTest()
    {}

    virtual void OnReceiveEvent(const CommonEventData &data)
    {
        GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::SubscriberTest OnReceiveEvent ";
        std::string action = data.GetWant().GetAction();
        if (action == EVENTCASE1) {
            ProcessSubscriberTestCase1();
        } else if (action == EVENTCASE2) {
            ProcessSubscriberTestCase2();
        } else if (action == EVENTCASE3) {
            ProcessSubscriberTestCase3();
        } else if (action == EVENTCASE4) {
            ProcessSubscriberTestCase4();
        } else if (action == EVENTCASE5) {
            ProcessSubscriberTestCase5();
        } else if (action == EVENTCASE6) {
            ProcessSubscriberTestCase6();
        } else {
            GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::SubscriberTest  OnReceiveEvent do nothing";
        }
    };

private:
    void ProcessSubscriberTestCase1()
    {
        GTEST_LOG_(INFO)
            << "CesPublishOrderedEventModuleTest::SubscriberTest OnReceiveEvent ProcessSubscriberTestCase1";
        g_mtx.unlock();
        SetCode(CHANGECODE);
        SetData(CHANGEDATA);
    }

    void ProcessSubscriberTestCase2()
    {
        GTEST_LOG_(INFO)
            << "CesPublishOrderedEventModuleTest::SubscriberTest OnReceiveEvent ProcessSubscriberTestCase2";
        EXPECT_EQ(GetCode() == CHANGECODE, true);
        EXPECT_EQ(GetData() == CHANGEDATA, true);
        g_mtx.unlock();
    }

    void ProcessSubscriberTestCase3()
    {
        GTEST_LOG_(INFO)
            << "CesPublishOrderedEventModuleTest::SubscriberTest OnReceiveEvent ProcessSubscriberTestCase3";
        struct tm startTime = {0};
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
        struct tm doingTime = {0};
        int64_t seconds = 0;

        while (!g_mtx.try_lock()) {
            // get current time and compare it with the start time
            EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
            seconds = OHOS::GetSecondsBetween(startTime, doingTime);
            if (seconds >= TIME_OUT_SECONDS_MIDDLE) {
                break;
            }
        }
        g_mtx.unlock();
        SetCode(CHANGECODE);
        SetData(CHANGEDATA);
    }

    void ProcessSubscriberTestCase4()
    {
        GTEST_LOG_(INFO)
            << "CesPublishOrderedEventModuleTest::SubscriberTest OnReceiveEvent ProcessSubscriberTestCase4";
        std::shared_ptr<AsyncCommonEventResult> result = GoAsyncCommonEvent();

        std::function<void()> asyncProcessFunc =
            std::bind(&SubscriberTest::AsyncProcess, this, result, TIME_OUT_SECONDS_MIDDLE);
        handlerPtr->PostTask(asyncProcessFunc);
    }

    void ProcessSubscriberTestCase5()
    {
        GTEST_LOG_(INFO)
            << "CesPublishOrderedEventModuleTest::SubscriberTest OnReceiveEvent ProcessSubscriberTestCase5";
        struct tm startTime = {0};
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
        struct tm doingTime = {0};
        int64_t seconds = 0;

        while (!g_mtx.try_lock()) {
            // get current time and compare it with the start time
            EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
            seconds = OHOS::GetSecondsBetween(startTime, doingTime);
            if (seconds >= TIME_OUT_SECONDS_MIDDLE) {
                break;
            }
        }
        g_mtx.unlock();
        SetCode(CHANGECODE);
        SetData(CHANGEDATA);
    }

    void ProcessSubscriberTestCase6()
    {
        GTEST_LOG_(INFO)
            << "CesPublishOrderedEventModuleTest::SubscriberTest OnReceiveEvent ProcessSubscriberTestCase6";
        std::shared_ptr<AsyncCommonEventResult> result = GoAsyncCommonEvent();

        std::function<void()> asyncProcessFunc =
            std::bind(&SubscriberTest::AsyncProcess, this, result, TIME_OUT_SECONDS_MIDDLE);
        handlerPtr->PostTask(asyncProcessFunc);
    }

    void AsyncProcess(const std::shared_ptr<AsyncCommonEventResult> &result, time_t outtime)
    {
        struct tm startTime = {0};
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
        struct tm doingTime = {0};
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        int64_t seconds = 0;

        while (!g_mtx.try_lock()) {
            // get current time and compare it with the start time
            EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
            seconds = OHOS::GetSecondsBetween(startTime, doingTime);
            if (seconds >= outtime) {
                break;
            }
        }
        g_mtx.unlock();

        result->SetCode(CHANGECODE);
        result->SetData(CHANGEDATA);

        result->FinishCommonEvent();
    }

private:
    std::shared_ptr<EventHandler> handlerPtr = std::make_shared<EventHandler>(EventRunner::Create());
};

class SubscriberAnotherTest : public CommonEventSubscriber {
public:
    explicit SubscriberAnotherTest(const CommonEventSubscribeInfo &subscribeInfo)
        : CommonEventSubscriber(subscribeInfo)
    {}

    ~SubscriberAnotherTest()
    {}

    virtual void OnReceiveEvent(const CommonEventData &data)
    {
        GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::SubscriberAnotherTest OnReceiveEvent ";
        std::string action = data.GetWant().GetAction();
        if (action == EVENTCASE1) {
            ProcessSubscriberAnotherTestCase1();
        } else if (action == EVENTCASE2) {
            ProcessSubscriberAnotherTestCase2();
        } else if (action == EVENTCASE3) {
            ProcessSubscriberAnotherTestCase3();
        } else if (action == EVENTCASE4) {
            ProcessSubscriberAnotherTestCase4();
        } else if (action == EVENTCASE5) {
            ProcessSubscriberAnotherTestCase5();
        } else if (action == EVENTCASE6) {
            ProcessSubscriberAnotherTestCase6();
        } else {
            GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::SubscriberAnotherTest do nothing";
        }
    };

private:
    void ProcessSubscriberAnotherTestCase1()
    {
        GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::OnReceiveEvent ProcessSubscriberAnotherTestCase1";
        g_mtxAnother.unlock();
        EXPECT_EQ(GetCode() == CHANGECODE, true);
        EXPECT_EQ(GetData() == CHANGEDATA, true);
    }

    void ProcessSubscriberAnotherTestCase2()
    {
        GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::OnReceiveEvent ProcessSubscriberAnotherTestCase2";
        g_mtxAnother.unlock();
        EXPECT_EQ(GetCode() == INITCODE, true);
        EXPECT_EQ(GetData() == INNITDATA, true);
        SetCode(CHANGECODE);
        SetData(CHANGEDATA);
    }

    void ProcessSubscriberAnotherTestCase3()
    {
        GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::OnReceiveEvent ProcessSubscriberAnotherTestCase3";
        g_mtxAnother.unlock();
        EXPECT_EQ(GetCode() == INITCODE, true);
        EXPECT_EQ(GetData() == INNITDATA, true);
    }

    void ProcessSubscriberAnotherTestCase4()
    {
        g_mtxAnother.unlock();
        GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::OnReceiveEvent ProcessSubscriberAnotherTestCase4";
        EXPECT_EQ(GetCode() == INITCODE, true);
        EXPECT_EQ(GetData() == INNITDATA, true);
    }

    void ProcessSubscriberAnotherTestCase5()
    {
        GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::OnReceiveEvent ProcessSubscriberAnotherTestCase5";
        EXPECT_EQ(GetCode() == INITCODE, true);
        EXPECT_EQ(GetData() == INNITDATA, true);
        struct tm startTime = {0};
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
        struct tm doingTime = {0};
        int64_t seconds = 0;

        while (!g_mtxAnother.try_lock()) {
            // get current time and compare it with the start time
            EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
            seconds = OHOS::GetSecondsBetween(startTime, doingTime);
            if (seconds >= TIME_OUT_SECONDS_MIDDLE) {
                break;
            }
        }
        g_mtxAnother.unlock();
        SetCode(CHANGECODE);
        SetData(CHANGEDATA);
    }

    void ProcessSubscriberAnotherTestCase6()
    {
        GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::OnReceiveEvent ProcessSubscriberAnotherTestCase6";
        EXPECT_EQ(GetCode() == INITCODE, true);
        EXPECT_EQ(GetData() == INNITDATA, true);
        std::shared_ptr<AsyncCommonEventResult> result = GoAsyncCommonEvent();

        std::function<void()> asyncProcessFunc =
            std::bind(&SubscriberAnotherTest::AsyncProcess, this, result, TIME_OUT_SECONDS_MIDDLE);
        handlerPtr->PostTask(asyncProcessFunc);
    }

    void AsyncProcess(const std::shared_ptr<AsyncCommonEventResult> &result, time_t outtime)
    {
        struct tm startTime = {0};
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
        struct tm doingTime = {0};
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        int64_t seconds = 0;

        while (!g_mtxAnother.try_lock()) {
            // get current time and compare it with the start time
            EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
            seconds = OHOS::GetSecondsBetween(startTime, doingTime);
            if (seconds >= outtime) {
                break;
            }
        }
        g_mtxAnother.unlock();

        result->SetCode(CHANGECODE);
        result->SetData(CHANGEDATA);

        result->FinishCommonEvent();
    }

private:
    std::shared_ptr<EventHandler> handlerPtr = std::make_shared<EventHandler>(EventRunner::Create());
};

void CesPublishOrderedEventModuleTest::SetUpTestCase(void)
{
    p_bundleObject = new OHOS::AppExecFwk::MockBundleMgrService();
    OHOS::DelayedSingleton<BundleManagerHelper>::GetInstance()->sptrBundleMgr_ =
        OHOS::iface_cast<OHOS::AppExecFwk::IBundleMgr>(p_bundleObject);
}

void CesPublishOrderedEventModuleTest::TearDownTestCase(void)
{}

void CesPublishOrderedEventModuleTest::SetUp(void)
{}

void CesPublishOrderedEventModuleTest::TearDown(void)
{}

bool CesPublishOrderedEventModuleTest::SubscribeCommonEventTest(std::shared_ptr<CommonEventSubscriber> subscriber) const
{
    GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::SubscribeCommonEventTest begain:";
    return CommonEventManager::SubscribeCommonEvent(subscriber);
}

bool CesPublishOrderedEventModuleTest::UnSubscribeCommonEventTest(
    const std::shared_ptr<CommonEventSubscriber> &subscriber) const
{
    return CommonEventManager::UnSubscribeCommonEvent(subscriber);
}

bool CesPublishOrderedEventModuleTest::PublishOrderedCommonEventTest(const CommonEventData &data,
    const CommonEventPublishInfo &publishInfo, const std::shared_ptr<CommonEventSubscriber> &subscriber) const
{
    GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::PublishOrderedCommonEventTest begain:";
    return CommonEventManager::PublishCommonEvent(data, publishInfo, subscriber);
}

/*
 * @tc.number: CommonEventPublishOrderedEventTest_0100
 * @tc.name: CommonEventManager PublishCommonEvent
 * @tc.desc: Verify that the ordered common event was published successfully
 */
HWTEST_F(CesPublishOrderedEventModuleTest, CommonEventPublishOrderedEventTest_0100, Function | MediumTest | Level1)
{
    /* Subscribe */
    // make matching skills
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE1);

    // make subscriber info
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    // make a subscriber object
    std::shared_ptr<SubscriberTest> subscriberTest = std::make_shared<SubscriberTest>(subscribeInfo);

    // subscribe a common event
    bool subscribeResult = SubscribeCommonEventTest(subscriberTest);
    EXPECT_EQ(true, subscribeResult);

    // make another matching skills
    MatchingSkills matchingSkillsAnother;
    matchingSkillsAnother.AddEvent(EVENTCASE1);

    // make another subscriber info
    CommonEventSubscribeInfo subscribeInfoAnother(matchingSkillsAnother);

    // make another subscriber object
    std::shared_ptr<SubscriberAnotherTest> subscriberTestAnother =
        std::make_shared<SubscriberAnotherTest>(subscribeInfoAnother);

    // subscribe another event
    bool subscribeResultAnother = SubscribeCommonEventTest(subscriberTestAnother);
    EXPECT_EQ(true, subscribeResultAnother);

    /* Publish */

    // make a want
    Want want;
    want.SetAction(EVENTCASE1);

    // make common event data
    CommonEventData data;
    data.SetWant(want);
    data.SetCode(INITCODE);
    data.SetData(INNITDATA);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    // lock the mutex
    g_mtx.lock();
    g_mtxAnother.lock();

    std::shared_ptr<Subscriber> subscriber = std::make_shared<Subscriber>(subscribeInfo);

    // publish ordered event
    bool publishResult = PublishOrderedCommonEventTest(data, publishInfo, subscriber);

    EXPECT_EQ(true, publishResult);

    // record start time
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);

    // record current time
    struct tm doingTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
    int64_t seconds = 0;

    GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::g_mtx.try_lock begin";
    while (!g_mtx.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_LIMIT) {
            GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::g_mtx.try_lock overtime";
            break;
        }
    }
    GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::g_mtx.try_lock use time(s): "
                     << OHOS::GetSecondsBetween(startTime, doingTime);

    // expect the subscriber could receive the event within 5 seconds.
    EXPECT_LT(seconds, TIME_OUT_SECONDS_LIMIT);

    // record start time of another
    struct tm startTimeAnother = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTimeAnother), true);

    // record current time
    struct tm doingTimeAnother = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTimeAnother), true);
    int64_t secondsAnother = 0;

    GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::g_mtxAnother.try_lock begin";
    while (!g_mtxAnother.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTimeAnother), true);
        secondsAnother = OHOS::GetSecondsBetween(startTimeAnother, doingTimeAnother);
        if (secondsAnother >= TIME_OUT_SECONDS_LIMIT) {
            GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::g_mtxAnother.try_lock overtime";
            break;
        }
    }

    GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::g_mtxAnother.try_lock use time(s): "
                     << OHOS::GetSecondsBetween(startTimeAnother, doingTimeAnother);
    EXPECT_LT(secondsAnother, TIME_OUT_SECONDS_LIMIT);

    g_mtx.unlock();
    g_mtxAnother.unlock();
}

/*
 * @tc.number: CommonEventPublishOrderedEventTest_0200
 * @tc.name: CommonEventManager PublishCommonEvent
 * @tc.desc: Verify that the ordered common event was published successfully
 */
HWTEST_F(CesPublishOrderedEventModuleTest, CommonEventPublishOrderedEventTest_0200, Function | MediumTest | Level1)
{
    /* Subscribe */
    // make matching skills
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE2);

    // make subscriber info
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(LOWPRIORITY);

    // make a subscriber object
    std::shared_ptr<SubscriberTest> subscriberTest = std::make_shared<SubscriberTest>(subscribeInfo);

    // subscribe a common event
    bool subscribeResult = SubscribeCommonEventTest(subscriberTest);
    EXPECT_EQ(true, subscribeResult);

    // make another matching skills
    MatchingSkills matchingSkillsAnother;
    matchingSkillsAnother.AddEvent(EVENTCASE2);

    // make another subscriber info
    CommonEventSubscribeInfo subscribeInfoAnother(matchingSkillsAnother);
    subscribeInfoAnother.SetPriority(HIGHPRIORITY);

    // make another subscriber object
    std::shared_ptr<SubscriberAnotherTest> subscriberTestAnother =
        std::make_shared<SubscriberAnotherTest>(subscribeInfoAnother);

    // subscribe another event
    bool subscribeResultAnother = SubscribeCommonEventTest(subscriberTestAnother);
    EXPECT_EQ(true, subscribeResultAnother);

    /* Publish */
    // make a want
    Want want;
    want.SetAction(EVENTCASE2);

    // make common event data
    CommonEventData data;
    data.SetWant(want);
    data.SetCode(INITCODE);
    data.SetData(INNITDATA);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    // lock the mutex
    g_mtx.lock();
    g_mtxAnother.lock();

    std::shared_ptr<Subscriber> subscriber = std::make_shared<Subscriber>(subscribeInfo);

    // publish ordered event
    bool publishResult = PublishOrderedCommonEventTest(data, publishInfo, subscriber);

    EXPECT_EQ(true, publishResult);

    // record start time of publishing
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);

    // record current time
    struct tm doingTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
    int64_t seconds = 0;

    GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::g_mtx.try_lock begin";
    while (!g_mtx.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_LIMIT) {
            GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::g_mtx.try_lock overtime";
            break;
        }
    }
    GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::g_mtx.try_lock use time(s): "
                     << OHOS::GetSecondsBetween(startTime, doingTime);

    // expect the subscriber could receive the event within 5 seconds.
    EXPECT_LT(seconds, TIME_OUT_SECONDS_LIMIT);

    // record start time of another
    struct tm startTimeAnother = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTimeAnother), true);

    // record current time
    struct tm doingTimeAnother = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTimeAnother), true);
    int64_t secondsAnother = 0;

    GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::g_mtxAnother.try_lock begin";
    while (!g_mtxAnother.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTimeAnother), true);
        secondsAnother = OHOS::GetSecondsBetween(startTimeAnother, doingTimeAnother);
        if (secondsAnother >= TIME_OUT_SECONDS_LIMIT) {
            GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::g_mtxAnother.try_lock overtime";
            break;
        }
    }
    GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::g_mtxAnother.try_lock use time(s): "
                     << OHOS::GetSecondsBetween(startTimeAnother, doingTimeAnother);
    EXPECT_LT(secondsAnother, TIME_OUT_SECONDS_LIMIT);

    g_mtx.unlock();
    g_mtxAnother.unlock();
}

/*
 * @tc.number: CommonEventPublishOrderedEventTest_0300
 * @tc.name: CommonEventManager PublishCommonEvent
 * @tc.desc: Verify that the ordered common event was published successfully
 */
HWTEST_F(CesPublishOrderedEventModuleTest, CommonEventPublishOrderedEventTest_0300, Function | MediumTest | Level1)
{
    /* Subscribe */
    // make matching skills
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE3);

    // make subscriber info
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    // make a subscriber object
    std::shared_ptr<SubscriberTest> subscriberTest = std::make_shared<SubscriberTest>(subscribeInfo);

    // subscribe a common event
    bool subscribeResult = SubscribeCommonEventTest(subscriberTest);
    EXPECT_EQ(true, subscribeResult);

    // make another matching skills
    MatchingSkills matchingSkillsAnother;
    matchingSkillsAnother.AddEvent(EVENTCASE3);

    // make another subscriber info
    CommonEventSubscribeInfo subscribeInfoAnother(matchingSkillsAnother);

    // make another subscriber object
    std::shared_ptr<SubscriberAnotherTest> subscriberTestAnother =
        std::make_shared<SubscriberAnotherTest>(subscribeInfoAnother);

    // subscribe another event
    bool subscribeResultAnother = SubscribeCommonEventTest(subscriberTestAnother);
    EXPECT_EQ(true, subscribeResultAnother);

    /* Publish */
    // make a want
    Want want;
    want.SetAction(EVENTCASE3);

    // make common event data
    CommonEventData data;
    data.SetWant(want);
    data.SetCode(INITCODE);
    data.SetData(INNITDATA);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    // lock the mutex
    g_mtx.lock();
    g_mtxAnother.lock();

    // publish ordered event
    std::shared_ptr<Subscriber> subscriber = std::make_shared<Subscriber>(subscribeInfo);
    bool publishResult = PublishOrderedCommonEventTest(data, publishInfo, subscriber);

    EXPECT_EQ(true, publishResult);

    // record start time of publishing
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
    int64_t seconds = 0;

    GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::g_mtx.try_lock begin";
    while (!g_mtx.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_MIDDLE + TIME_OUT_SECONDS_LIMIT) {
            GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::g_mtx.try_lock overtime";
            break;
        }
    }

    GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::g_mtx.try_lock use time(s): "
                     << OHOS::GetSecondsBetween(startTime, doingTime);
    // expect the subscriber could receive the event within 5 seconds.
    EXPECT_LT(seconds, TIME_OUT_SECONDS_MIDDLE + TIME_OUT_SECONDS_LIMIT);

    // record start time of another
    struct tm startTimeAnother = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTimeAnother), true);
    struct tm doingTimeAnother = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTimeAnother), true);
    int64_t secondsAnother = 0;

    GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::g_mtxAnother.try_lock begin";
    while (!g_mtxAnother.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTimeAnother), true);
        secondsAnother = OHOS::GetSecondsBetween(startTimeAnother, doingTimeAnother);
        if (secondsAnother >= TIME_OUT_SECONDS_LIMIT) {
            GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::g_mtxAnother.try_lock overtime";
            break;
        }
    }

    GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::g_mtxAnother.try_lock use time(s): "
                     << OHOS::GetSecondsBetween(startTimeAnother, doingTimeAnother);

    g_mtx.unlock();
    g_mtxAnother.unlock();
}

/*
 * @tc.number: CommonEventPublishOrderedEventTest_0400
 * @tc.name: CommonEventManager PublishCommonEvent
 * @tc.desc: SubscriberA process time out 15s by Asynchronization
 *           SubscriberB can receive event
 *           Verify that the ordered common event was published successfully
 */
HWTEST_F(CesPublishOrderedEventModuleTest, CommonEventPublishOrderedEventTest_0400, Function | MediumTest | Level1)
{
    /* Subscribe */
    // make matching skills
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE4);

    // make subscriber info
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    // make a subcriber object
    std::shared_ptr<SubscriberTest> subscriberTest = std::make_shared<SubscriberTest>(subscribeInfo);

    // subscribe a common event
    bool subscribeResult = SubscribeCommonEventTest(subscriberTest);
    EXPECT_EQ(true, subscribeResult);

    // make another matching skills
    MatchingSkills matchingSkillsAnother;
    matchingSkillsAnother.AddEvent(EVENTCASE4);

    // make another subscriber info
    CommonEventSubscribeInfo subscribeInfoAnother(matchingSkillsAnother);

    // make another subscriber object
    std::shared_ptr<SubscriberAnotherTest> subscriberTestAnother =
        std::make_shared<SubscriberAnotherTest>(subscribeInfoAnother);

    // subscribe another event
    bool subscribeResultAnother = SubscribeCommonEventTest(subscriberTestAnother);
    EXPECT_EQ(true, subscribeResultAnother);

    /* Publish */
    // make a want
    Want want;
    want.SetAction(EVENTCASE4);

    // make common event data
    CommonEventData data;
    data.SetWant(want);
    data.SetCode(INITCODE);
    data.SetData(INNITDATA);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    // lock the mutex
    g_mtx.lock();
    g_mtxAnother.lock();

    std::shared_ptr<Subscriber> subscriber = std::make_shared<Subscriber>(subscribeInfo);

    // publish ordered event
    bool publishResult = PublishOrderedCommonEventTest(data, publishInfo, subscriber);

    EXPECT_EQ(true, publishResult);

    // record start time of publishing
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
    int64_t seconds = 0;

    GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::g_mtx.try_lock begin";
    while (!g_mtx.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_MIDDLE + TIME_OUT_SECONDS_LIMIT) {
            GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::g_mtx.try_lock overtime";
            break;
        }
    }

    GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::g_mtx.try_lock use time(s): "
                     << OHOS::GetSecondsBetween(startTime, doingTime);

    // record start time of another
    struct tm startTimeAnother = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTimeAnother), true);
    struct tm doingTimeAnother = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTimeAnother), true);
    int64_t secondsAnother = 0;

    GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::g_mtxAnother.try_lock begin";
    while (!g_mtxAnother.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTimeAnother), true);
        secondsAnother = OHOS::GetSecondsBetween(startTimeAnother, doingTimeAnother);
        if (secondsAnother >= TIME_OUT_SECONDS_LIMIT) {
            GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::g_mtxAnother.try_lock overtime";
            break;
        }
    }

    GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::g_mtxAnother.try_lock use time(s): "
                     << OHOS::GetSecondsBetween(startTimeAnother, doingTimeAnother);
    EXPECT_LT(secondsAnother, TIME_OUT_SECONDS_LIMIT);

    g_mtx.unlock();
    g_mtxAnother.unlock();
}

/*
 * @tc.number: CommonEventPublishOrderedEventTest_0500
 * @tc.name: CommonEventManager PublishCommonEvent
 * @tc.desc: SubscriberA process time out 15s by synchronization
 *           SubscriberB process time out 15s by synchronization
 *           lastSubscriber can receive event
 *           Verify that the ordered common event was published successfully
 */
HWTEST_F(CesPublishOrderedEventModuleTest, CommonEventPublishOrderedEventTest_0500, Function | MediumTest | Level1)
{
    /* Subscribe */
    // make matching skills
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE5);

    // make subscriber info
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    // make a subscriber object
    std::shared_ptr<SubscriberTest> subscriberTest = std::make_shared<SubscriberTest>(subscribeInfo);

    // subscribe a common event
    bool subscribeResult = SubscribeCommonEventTest(subscriberTest);
    EXPECT_EQ(true, subscribeResult);

    // make another matching skills
    MatchingSkills matchingSkillsAnother;
    matchingSkillsAnother.AddEvent(EVENTCASE5);

    // make another subscriber info
    CommonEventSubscribeInfo subscribeInfoAnother(matchingSkillsAnother);

    // make another subscriber object
    std::shared_ptr<SubscriberAnotherTest> subscriberTestAnother =
        std::make_shared<SubscriberAnotherTest>(subscribeInfoAnother);

    // subscribe another event
    bool subscribeResultAnother = SubscribeCommonEventTest(subscriberTestAnother);
    EXPECT_EQ(true, subscribeResultAnother);

    /* Publish */
    // make a want
    Want want;
    want.SetAction(EVENTCASE5);

    // make common event data
    CommonEventData data;
    data.SetWant(want);
    data.SetCode(INITCODE);
    data.SetData(INNITDATA);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    // lock the mutex
    g_mtx.lock();
    g_mtxAnother.lock();

    // publish ordered event
    std::shared_ptr<Subscriber> subscriber = std::make_shared<Subscriber>(subscribeInfo);
    bool publishResult = PublishOrderedCommonEventTest(data, publishInfo, subscriber);

    EXPECT_EQ(true, publishResult);

    // record start time of publishing
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
    int64_t seconds = 0;

    GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::g_mtx.try_lock begin";
    while (!g_mtx.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_MIDDLE + TIME_OUT_SECONDS_LIMIT) {
            GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::g_mtx.try_lock overtime";
            break;
        }
    }

    GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::g_mtx.try_lock use time(s): "
                     << OHOS::GetSecondsBetween(startTime, doingTime);
    // expect the subscriber could receive the event within 5 seconds.
    EXPECT_LT(seconds, TIME_OUT_SECONDS_MIDDLE + TIME_OUT_SECONDS_LIMIT);

    // record start time of another
    struct tm startTimeAnother = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTimeAnother), true);
    struct tm doingTimeAnother = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTimeAnother), true);
    int64_t secondsAnother = 0;

    GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::g_mtxAnother.try_lock begin";
    while (!g_mtxAnother.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTimeAnother), true);
        secondsAnother = OHOS::GetSecondsBetween(startTimeAnother, doingTimeAnother);
        if (secondsAnother >= TIME_OUT_SECONDS_MIDDLE + TIME_OUT_SECONDS_LIMIT) {
            GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::g_mtxAnother.try_lock overtime";
            break;
        }
    }

    GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::g_mtxAnother.try_lock use time(s): "
                     << OHOS::GetSecondsBetween(startTimeAnother, doingTimeAnother);
    EXPECT_LT(secondsAnother, TIME_OUT_SECONDS_MIDDLE + TIME_OUT_SECONDS_LIMIT);

    g_mtx.unlock();
    g_mtxAnother.unlock();
}

/*
 * @tc.number: CommonEventPublishOrderedEventTest_0600
 * @tc.name: CommonEventManager PublishCommonEvent
 * @tc.desc: SubscriberA process time out 15s by Asynchronization
 *           SubscriberB process time out 15s by Asynchronization
 *           lastSubscriber can receive event
 *           Verify that the ordered common event was published successfully
 */
HWTEST_F(CesPublishOrderedEventModuleTest, CommonEventPublishOrderedEventTest_0600, Function | MediumTest | Level1)
{
    /* Subscribe */
    // make matching skills
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE6);

    // make subscriber info
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    // make a subscriber object
    std::shared_ptr<SubscriberTest> subscriberTest = std::make_shared<SubscriberTest>(subscribeInfo);

    // subscribe a common event
    bool subscribeResult = SubscribeCommonEventTest(subscriberTest);
    EXPECT_EQ(true, subscribeResult);

    // make another matching skills
    MatchingSkills matchingSkillsAnother;
    matchingSkillsAnother.AddEvent(EVENTCASE6);

    // make another subscriber info
    CommonEventSubscribeInfo subscribeInfoAnother(matchingSkillsAnother);

    // make another subscriber object
    std::shared_ptr<SubscriberAnotherTest> subscriberTestAnother =
        std::make_shared<SubscriberAnotherTest>(subscribeInfoAnother);

    // subscribe another event
    bool subscribeResultAnother = SubscribeCommonEventTest(subscriberTestAnother);
    EXPECT_EQ(true, subscribeResultAnother);

    /* Publish */
    // make a want
    Want want;
    want.SetAction(EVENTCASE6);

    // make common event data
    CommonEventData data;
    data.SetWant(want);
    data.SetCode(INITCODE);
    data.SetData(INNITDATA);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    // lock the mutex
    g_mtx.lock();
    g_mtxAnother.lock();

    std::shared_ptr<Subscriber> subscriber = std::make_shared<Subscriber>(subscribeInfo);

    // publish ordered event
    bool publishResult = PublishOrderedCommonEventTest(data, publishInfo, subscriber);

    EXPECT_EQ(true, publishResult);

    // record start time of publishing
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
    int64_t seconds = 0;

    GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::g_mtx.try_lock begin";
    while (!g_mtx.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_MIDDLE + TIME_OUT_SECONDS_LIMIT) {
            GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::g_mtx.try_lock overtime";
            break;
        }
    }

    GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::g_mtx.try_lock use time(s): "
                     << OHOS::GetSecondsBetween(startTime, doingTime);
    // expect the subscriber could receive the event within 5 seconds.
    EXPECT_LT(seconds, TIME_OUT_SECONDS_MIDDLE + TIME_OUT_SECONDS_LIMIT);

    // record start time of another
    struct tm startTimeAnother = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTimeAnother), true);
    struct tm doingTimeAnother = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTimeAnother), true);
    int64_t secondsAnother = 0;

    GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::g_mtxAnother.try_lock begin";
    while (!g_mtxAnother.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTimeAnother), true);
        secondsAnother = OHOS::GetSecondsBetween(startTimeAnother, doingTimeAnother);
        if (secondsAnother >= TIME_OUT_SECONDS_MIDDLE + TIME_OUT_SECONDS_LIMIT) {
            GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::g_mtxAnother.try_lock overtime";
            break;
        }
    }

    GTEST_LOG_(INFO) << "CesPublishOrderedEventModuleTest::g_mtxAnother.try_lock use time(s): "
                     << OHOS::GetSecondsBetween(startTimeAnother, doingTimeAnother);
    EXPECT_LT(secondsAnother, TIME_OUT_SECONDS_MIDDLE + TIME_OUT_SECONDS_LIMIT);

    g_mtx.unlock();
    g_mtxAnother.unlock();
}

/*
 * @tc.number: CommonEventPublishOrderedEventTest_0700
 * @tc.name: CommonEventControlManager EnqueueOrderedRecord
 * @tc.desc: test EnqueueOrderedRecord eventRecordPtr is null, verify EnqueueOrderedRecord eventRecordPtr is null
 *           orderedEventQueue_ size is 0
 */
HWTEST_F(CesPublishOrderedEventModuleTest, CommonEventPublishOrderedEventTest_0700, Function | MediumTest | Level1)
{
    CommonEventControlManager commonEventControlManager;
    commonEventControlManager.EnqueueOrderedRecord(nullptr);
    EXPECT_EQ(0, commonEventControlManager.orderedEventQueue_.size());
}

/*
 * @tc.number: CommonEventPublishOrderedEventTest_0800
 * @tc.name: CommonEventControlManager FinishReceiver
 * @tc.desc: test FinishReceiver recordPtr is null, verify FinishReceiver recordPtr is return false
 */
HWTEST_F(CesPublishOrderedEventModuleTest, CommonEventPublishOrderedEventTest_0800, Function | MediumTest | Level1)
{
    CommonEventControlManager commonEventControlManager;
    std::string receiverData = "receiverData";
    EXPECT_EQ(false, commonEventControlManager.FinishReceiver(nullptr, 0, receiverData, false));
}

/*
 * @tc.number: CommonEventPublishOrderedEventTest_0900
 * @tc.name: CommonEventControlManager GetOrderedEventHandler
 * @tc.desc: test GetOrderedEventHandler handlerOrdered_ is not null, verify GetOrderedEventHandler handlerOrdered_ is
 *           not return true
 */
HWTEST_F(CesPublishOrderedEventModuleTest, CommonEventPublishOrderedEventTest_0900, Function | MediumTest | Level1)
{
    CommonEventControlManager commonEventControlManager;
    std::shared_ptr<CommonEventControlManager> controlManager = std::make_shared<CommonEventControlManager>();
    commonEventControlManager.handlerOrdered_ =
        std::make_shared<OrderedEventHandler>(EventRunner::Create(), controlManager);
    EXPECT_EQ(true, commonEventControlManager.GetOrderedEventHandler());
}

/*
 * @tc.number: CommonEventPublishOrderedEventTest_1000
 * @tc.name: verify Freeze
 * @tc.desc: don't set innerCommonEventManager_ verify Freeze false
 * @tc.require: issueI5NGO7
 */
HWTEST_F(CesPublishOrderedEventModuleTest, CommonEventPublishOrderedEventTest_1000, Function | MediumTest | Level1)
{
    uint16_t systemUid = 1000;
    bool freeze = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->Freeze(systemUid);
    EXPECT_EQ(false, freeze);
}

/*
 * @tc.number: CommonEventPublishOrderedEventTest_1100
 * @tc.name: verify Unfreeze
 * @tc.desc: don't set innerCommonEventManager_ verify Unfreeze false
 * @tc.require: issueI5NGO7
 */
HWTEST_F(CesPublishOrderedEventModuleTest, CommonEventPublishOrderedEventTest_1100, Function | MediumTest | Level1)
{
    uint16_t systemUid = 1000;
    bool unfreeze = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->Unfreeze(systemUid);
    EXPECT_EQ(false, unfreeze);
}

/*
 * @tc.number: CommonEventPublishOrderedEventTest_1200
 * @tc.name: verify UnfreezeAll
 * @tc.desc: don't set innerCommonEventManager_ verify UnfreezeAll false
 * @tc.require: issueI5NGO7
 */
HWTEST_F(CesPublishOrderedEventModuleTest, CommonEventPublishOrderedEventTest_1200, Function | MediumTest | Level1)
{
    bool unfreezeAll = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->UnfreezeAll();
    EXPECT_EQ(false, unfreezeAll);
}

/*
 * @tc.number: AsyncCommonEventResult_0100
 * @tc.name: verify IsStickyCommonEvent
 * @tc.desc: Invoke IsStickyCommonEvent interface verify whether it is normal
 */
HWTEST_F(CesPublishOrderedEventModuleTest, AsyncCommonEventResult_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<AsyncCommonEventResult> result = std::make_shared<AsyncCommonEventResult>(
        11, "aa", true, true, nullptr);
    if (result != nullptr) {
        bool isStickyCommonEvent = result->IsStickyCommonEvent();
        EXPECT_EQ(true, isStickyCommonEvent);
    }
}

/*
 * @tc.number: CommonEventControlManager_0100
 * @tc.name: verify PublishAllFreezeCommonEvents
 * @tc.desc: Invoke PublishAllFreezeCommonEvents interface verify whether it is normal
 */
HWTEST_F(CesPublishOrderedEventModuleTest, CommonEventControlManager_0100, Function | MediumTest | Level1)
{
    CommonEventControlManager commonEventControlManager;
    bool result = commonEventControlManager.PublishAllFreezeCommonEvents();
    EXPECT_EQ(true, result);
}

/*
 * @tc.number: CommonEventManagerService_0100
 * @tc.name: verify DumpState
 * @tc.desc: don't set innerCommonEventManager_ verify DumpState false
 */
HWTEST_F(CesPublishOrderedEventModuleTest, CommonEventManagerService_0100, Function | MediumTest | Level1)
{
    uint8_t dumpType = 10;
    std::string event = "aa";
    int32_t userId = 20;
    std::vector<std::string> state;
    bool result = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->DumpState(
        dumpType, event, userId, state);
    EXPECT_EQ(false, result);
}

/*
 * @tc.number: CommonEventManagerService_0200
 * @tc.name: verify FinishReceiver
 * @tc.desc: don't set innerCommonEventManager_ verify FinishReceiver false
 */
HWTEST_F(CesPublishOrderedEventModuleTest, CommonEventManagerService_0200, Function | MediumTest | Level1)
{
    std::string receiverData = "aa";
    int32_t code = 20;
    bool abortEvent = true;
    bool result = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->FinishReceiver(
        nullptr, code, receiverData, abortEvent);
    EXPECT_EQ(false, result);
}

/*
 * @tc.number: CommonEventManagerService_0300
 * @tc.name: verify Dump
 * @tc.desc: don't set Dump verify FinishReceiver false
 */
HWTEST_F(CesPublishOrderedEventModuleTest, CommonEventManagerService_0300, Function | MediumTest | Level1)
{
    int fd = 22;
    std::vector<std::u16string> args;
    int result = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->Dump(fd, args);
    EXPECT_EQ(OHOS::ERR_INVALID_VALUE, result);
}

/*
 * @tc.number: CommonEventManager_0100
 * @tc.name: verify PublishCommonEvent
 * @tc.desc: Invoke PublishCommonEvent interface verify whether it is normal
 */
HWTEST_F(CesPublishOrderedEventModuleTest, CommonEventManager_0100, Function | MediumTest | Level1)
{
    std::vector<std::u16string> args;
    std::string results = "hh";
    auto record = std::make_shared<FrozenEventRecord>();
    CommonEventSubscriberManager commonEventSubscriberManager;
    commonEventSubscriberManager.UpdateAllFreezeInfos(true);
    std::shared_ptr<InnerCommonEventManager> innerCommonEventManager_ = std::make_shared<InnerCommonEventManager>();
    if (nullptr != innerCommonEventManager_) {
        innerCommonEventManager_->UnfreezeAll();
        innerCommonEventManager_->HiDump(args, results);
    }
    CommonEventData data;
    CommonEventPublishInfo publishInfo;
    uid_t uid = 1;
    int32_t callerToken = 0;
    bool result = CommonEventManager::PublishCommonEvent(data, publishInfo, nullptr, uid, callerToken);
    EXPECT_EQ(false, result);
}
