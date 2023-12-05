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

#include <gtest/gtest.h>

// redefine private and protected since testcase need to invoke and test private function
#define private public
#define protected public
#include "bundle_manager_helper.h"
#include "common_event_control_manager.h"
#undef private
#undef protected

#include "common_event_listener.h"
#include "common_event_subscriber.h"
#include "inner_common_event_manager.h"
#include "mock_bundle_manager.h"

using namespace testing::ext;
using namespace OHOS::EventFwk;
using namespace OHOS::AppExecFwk;

namespace {
constexpr uint8_t PID = 0;
constexpr uint16_t SYSTEM_UID = 1000;
const std::string EVENT = "com.ces.test.event";
const std::string ENTITY = "com.ces.test.entity";
const std::string ACTION = "acion";
std::mutex mtx;
static OHOS::sptr<OHOS::IRemoteObject> bundleObject = nullptr;
InnerCommonEventManager innerCommonEventManager;
std::shared_ptr<CommonEventControlManager> commonEventControlManager = std::make_shared<CommonEventControlManager>();
std::shared_ptr<EventHandler> handler_;
}  // namespace

class CommonEventPublishOrderedEventUnitTest : public testing::Test {
public:
    CommonEventPublishOrderedEventUnitTest()
    {}
    ~CommonEventPublishOrderedEventUnitTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

class SubscriberTest : public CommonEventSubscriber {
public:
    explicit SubscriberTest(const CommonEventSubscribeInfo &sp) : CommonEventSubscriber(sp)
    {}

    ~SubscriberTest()
    {}

    virtual void OnReceiveEvent(const CommonEventData &data)
    {
        GTEST_LOG_(INFO) << "OnReceiveEvent receive";
        mtx.unlock();
    }
};

void CommonEventPublishOrderedEventUnitTest::SetUpTestCase(void)
{
    handler_ = std::make_shared<EventHandler>(EventRunner::Create(true));
    auto task = []() {
        EventRunner::GetMainEventRunner()->Run();
    };
    handler_->PostTask(task);

    bundleObject = new MockBundleMgrService();
    OHOS::DelayedSingleton<BundleManagerHelper>::GetInstance()->sptrBundleMgr_ =
        OHOS::iface_cast<OHOS::AppExecFwk::IBundleMgr>(bundleObject);
}

void CommonEventPublishOrderedEventUnitTest::TearDownTestCase(void)
{
    EventRunner::GetMainEventRunner()->Stop();
}

void CommonEventPublishOrderedEventUnitTest::SetUp(void)
{}

void CommonEventPublishOrderedEventUnitTest::TearDown(void)
{}

/*
 * @tc.number: CommonEventPublishOrderedUnitTest_0700
 * @tc.name: test PublishCommonEvent
 * @tc.desc: Verify InnerCommonEventManager PublishCommonEvent success
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_0700, Function | MediumTest | Level1)
{
    // make a want
    Want want;
    want.SetAction(ACTION);

    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    MatchingSkills matchingSkills;

    // make subscriber info
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    // make a subscriber object
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);

    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);

    OHOS::Security::AccessToken::AccessTokenID tokenID = 0;

    mtx.lock();

    struct tm curTime {
        0
    };
    // publish ordered event
    bool result = innerCommonEventManager.PublishCommonEvent(
        data, publishInfo, commonEventListener, curTime, PID, SYSTEM_UID, tokenID, UNDEFINED_USER, "bundlename");

    EXPECT_TRUE(result);

    int count = 0;
    while (!mtx.try_lock()) {
        if (count == 0) {
            GTEST_LOG_(INFO) << "Wait OnReceive callback function process";
            count = 1;
        } else {
            usleep(100 * 1000);
        }
    }

    mtx.unlock();
    GTEST_LOG_(INFO) << "Testcase finished";
}

/*
 * @tc.number: CommonEventPublishOrderedUnitTest_0800
 * @tc.name: test PublishCommonEvent
 * @tc.desc: Verify InnerCommonEventManager PublishCommonEvent fail because data has no aciton
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_0800, Function | MediumTest | Level1)
{
    // make common event data
    CommonEventData data;

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    MatchingSkills matchingSkills;

    // make subscriber info
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    // make a subscriber object
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);

    // make commonEventListener
    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);

    struct tm curTime {
        0
    };
    OHOS::Security::AccessToken::AccessTokenID tokenID = 0;
    // publish ordered event
    bool result = innerCommonEventManager.PublishCommonEvent(
        data, publishInfo, commonEventListener, curTime, PID, SYSTEM_UID, tokenID, UNDEFINED_USER, "bundlename");
    EXPECT_FALSE(result);
}

/*
 * @tc.number: CommonEventPublishOrderedUnitTest_0900
 * @tc.name: test PublishCommonEvent
 * @tc.desc: Verify CommonEventControlManager PublishCommonEvent success
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_0900, Function | MediumTest | Level1)
{
    // make a commonEventRecord
    Want want;
    want.SetAction(ACTION);

    CommonEventData eventRef;
    eventRef.SetWant(want);

    CommonEventPublishInfo publishinfoRef;

    std::shared_ptr<CommonEventData> commonEventData = std::make_shared<CommonEventData>(eventRef);
    std::shared_ptr<CommonEventPublishInfo> publishInfo = std::make_shared<CommonEventPublishInfo>(publishinfoRef);
    publishInfo->SetOrdered(true);

    CommonEventRecord commonEventRecord;
    commonEventRecord.commonEventData = commonEventData;
    commonEventRecord.publishInfo = publishInfo;
    commonEventRecord.eventRecordInfo.pid = 0;
    commonEventRecord.eventRecordInfo.uid = 0;
    commonEventRecord.eventRecordInfo.bundleName = "bundleName";
    commonEventRecord.recordTime = {0};

    // make subscriber info
    MatchingSkills matchingSkills;
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    // make subscriber
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);

    // make common event listener
    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);
    OHOS::sptr<OHOS::IRemoteObject> commonEventListenerPtr(commonEventListener);

    mtx.lock();

    bool result = false;
    result = commonEventControlManager->PublishCommonEvent(commonEventRecord, commonEventListenerPtr);
    EXPECT_TRUE(result);

    int count = 0;
    while (!mtx.try_lock()) {
        if (count == 0) {
            GTEST_LOG_(INFO) << "Wait OnReceive callback function process";
            count = 1;
        } else {
            usleep(100 * 1000);
        }
    }
    mtx.unlock();
    GTEST_LOG_(INFO) << "Testcase finished";
}

/*
 * @tc.number: CommonEventPublishOrderedUnitTest_1000
 * @tc.name: test ProcessOrderedEvent
 * @tc.desc: Verify CommonEventControlManager ProcessOrderedEvent success
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_1000, Function | MediumTest | Level1)
{
    // make common event record
    std::shared_ptr<CommonEventData> commonEventData = std::make_shared<CommonEventData>();
    std::shared_ptr<CommonEventPublishInfo> publishInfo = std::make_shared<CommonEventPublishInfo>();
    publishInfo->SetOrdered(true);

    CommonEventRecord commonEventRecord;
    commonEventRecord.commonEventData = commonEventData;
    commonEventRecord.publishInfo = publishInfo;
    commonEventRecord.eventRecordInfo.pid = 0;
    commonEventRecord.eventRecordInfo.uid = 0;
    commonEventRecord.eventRecordInfo.bundleName = "bundleName";

    // make subscriber info
    MatchingSkills matchingSkills;
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    // make subscriber
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);

    // make common event listener
    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);
    OHOS::sptr<OHOS::IRemoteObject> commonEventListenerPtr(commonEventListener);
    mtx.lock();
    bool result = commonEventControlManager->ProcessOrderedEvent(commonEventRecord, commonEventListenerPtr);
    EXPECT_TRUE(result);

    int count = 0;
    while (!mtx.try_lock()) {
        if (count == 0) {
            GTEST_LOG_(INFO) << "Wait OnReceive callback function process";
            count = 1;
        } else {
            usleep(100 * 1000);
        }
    }
    mtx.unlock();
    GTEST_LOG_(INFO) << "Testcase finished";
}

/*
 * @tc.number: CommonEventPublishOrderedUnitTest_1100
 * @tc.name: test EnqueueOrderedRecord eventRecordPtr is null
 * @tc.desc: Verify EnqueueOrderedRecord eventRecordPtr is null orderedEventQueue_ size is 0
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_1100, Function | MediumTest | Level1)
{
    bool result = commonEventControlManager->EnqueueOrderedRecord(nullptr);
    EXPECT_FALSE(result);
}

/*
 * @tc.number: CommonEventPublishOrderedUnitTest_1200
 * @tc.name: test EnqueueOrderedRecord
 * @tc.desc: Verify EnqueueOrderedRecord success
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_1200, Function | MediumTest | Level1)
{
    // make common event record
    std::shared_ptr<CommonEventData> commonEventData = std::make_shared<CommonEventData>();
    std::shared_ptr<CommonEventPublishInfo> publishInfo = std::make_shared<CommonEventPublishInfo>();

    // make ordered event record
    std::shared_ptr<OrderedEventRecord> eventRecord = std::make_shared<OrderedEventRecord>();
    eventRecord->commonEventData = commonEventData;
    eventRecord->publishInfo = publishInfo;
    eventRecord->resultTo = nullptr;
    eventRecord->state = OrderedEventRecord::IDLE;
    eventRecord->nextReceiver = 0;

    bool result = commonEventControlManager->EnqueueOrderedRecord(eventRecord);
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CommonEventPublishOrderedUnitTest_1300
 * @tc.name: test ScheduleOrderedCommonEvent
 * @tc.desc: Verify ScheduleOrderedCommonEvent success when scheduled is true
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_1300, Function | MediumTest | Level1)
{
    commonEventControlManager->scheduled_ = true;
    bool result = commonEventControlManager->ScheduleOrderedCommonEvent();
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CommonEventPublishOrderedUnitTest_1400
 * @tc.name: test ScheduleOrderedCommonEvent
 * @tc.desc: Verify ScheduleOrderedCommonEvent success when scheduled is false
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_1400, Function | MediumTest | Level1)
{
    commonEventControlManager->scheduled_ = false;

    bool result = commonEventControlManager->ScheduleOrderedCommonEvent();
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "Testcase finished";
}

/*
 * @tc.number: CommonEventPublishOrderedUnitTest_1500
 * @tc.name: test FinishReceiver
 * @tc.desc: Verify FinishReceiver return true because eventRecord state is received
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_1500, Function | MediumTest | Level1)
{
    // make common event record
    std::shared_ptr<CommonEventData> commonEventData = std::make_shared<CommonEventData>();
    std::shared_ptr<CommonEventPublishInfo> publishInfo = std::make_shared<CommonEventPublishInfo>();

    // make ordered event record
    std::shared_ptr<OrderedEventRecord> eventRecord = std::make_shared<OrderedEventRecord>();
    eventRecord->commonEventData = commonEventData;
    eventRecord->publishInfo = publishInfo;
    eventRecord->resultTo = nullptr;
    eventRecord->state = OrderedEventRecord::RECEIVED;
    eventRecord->nextReceiver = 0;

    std::string receiverData = "receiverData";
    bool result = commonEventControlManager->FinishReceiver(eventRecord, 0, receiverData, false);
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CommonEventPublishOrderedUnitTest_1600
 * @tc.name: test FinishReceiver
 * @tc.desc: Verify FinishReceiver return false eventRecord state is idle
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_1600, Function | MediumTest | Level1)
{
    // make common event record
    std::shared_ptr<CommonEventData> commonEventData = std::make_shared<CommonEventData>();
    std::shared_ptr<CommonEventPublishInfo> publishInfo = std::make_shared<CommonEventPublishInfo>();

    // make ordered event record
    std::shared_ptr<OrderedEventRecord> eventRecord = std::make_shared<OrderedEventRecord>();
    eventRecord->commonEventData = commonEventData;
    eventRecord->publishInfo = publishInfo;
    eventRecord->resultTo = nullptr;
    eventRecord->state = OrderedEventRecord::IDLE;
    eventRecord->nextReceiver = 0;

    std::string receiverData = "receiverData";
    bool result = commonEventControlManager->FinishReceiver(eventRecord, 0, receiverData, false);
    EXPECT_FALSE(result);
}

/*
 * @tc.number: CommonEventPublishOrderedUnitTest_1700
 * @tc.name: test FinishReceiver recordPtr is null
 * @tc.desc: Verify FinishReceiver recordPtr is null return false
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_1700, Function | MediumTest | Level1)
{
    std::string receiverData = "receiverData";
    bool result = commonEventControlManager->FinishReceiver(nullptr, 0, receiverData, false);
    EXPECT_FALSE(result);
}

/*
 * @tc.number: CommonEventPublishOrderedUnitTest_1800
 * @tc.name: test GetOrderedEventHandler handlerOrdered_ is not null
 * @tc.desc: Verify GetOrderedEventHandler handlerOrdered_ is not return true
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_1800, Function | MediumTest | Level1)
{
    commonEventControlManager->handlerOrdered_ =
        std::make_shared<OrderedEventHandler>(EventRunner::Create(), commonEventControlManager);
    bool result = commonEventControlManager->GetOrderedEventHandler();
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CommonEventPublishOrderedUnitTest_1900
 * @tc.name: test CurrentOrderedEventTimeout orderedEventQueue_ is null
 * @tc.desc: Verify CurrentOrderedEventTimeout fail because orderedEventQueue_ is null
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_1900, Function | MediumTest | Level1)
{
    commonEventControlManager->orderedEventQueue_.clear();
    commonEventControlManager->CurrentOrderedEventTimeout(true);

    bool result = false;
    if (commonEventControlManager->orderedEventQueue_.size() == 0) {
        result = true;
    }
    EXPECT_TRUE(result);
    commonEventControlManager->orderedEventQueue_.clear();
}

/*
 * @tc.number: CommonEventPublishOrderedUnitTest_2000
 * @tc.name: test CurrentOrderedEventTimeout
 * @tc.desc: Verify CurrentOrderedEventTimeout success with eventRecord->nextReceiver = 0;
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_2000, Function | MediumTest | Level1)
{
    // make common event record
    std::shared_ptr<CommonEventData> commonEventData = std::make_shared<CommonEventData>();
    std::shared_ptr<CommonEventPublishInfo> publishInfo = std::make_shared<CommonEventPublishInfo>();

    // make ordered event record
    std::shared_ptr<OrderedEventRecord> eventRecord = std::make_shared<OrderedEventRecord>();
    eventRecord->commonEventData = commonEventData;
    eventRecord->publishInfo = publishInfo;
    eventRecord->resultTo = nullptr;
    eventRecord->state = OrderedEventRecord::IDLE;
    eventRecord->nextReceiver = 0;

    // enqueue ordered record
    commonEventControlManager->scheduled_ = true;
    commonEventControlManager->EnqueueOrderedRecord(eventRecord);
    commonEventControlManager->CurrentOrderedEventTimeout(true);

    bool result = false;
    if (commonEventControlManager->orderedEventQueue_.size() > 0) {
        result = true;
    }
    EXPECT_TRUE(result);
    commonEventControlManager->orderedEventQueue_.clear();
}

/*
 * @tc.number: CommonEventPublishOrderedUnitTest_2100
 * @tc.name: test CurrentOrderedEventTimeout
 * @tc.desc: Verify CurrentOrderedEventTimeout success with eventRecord->nextReceiver = 1;
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_2100, Function | MediumTest | Level1)
{
    // make event record
    std::shared_ptr<CommonEventData> commonEventData = std::make_shared<CommonEventData>();
    std::shared_ptr<CommonEventPublishInfo> publishInfo = std::make_shared<CommonEventPublishInfo>();

    std::shared_ptr<EventSubscriberRecord> subscriberRecord = std::make_shared<EventSubscriberRecord>();

    std::shared_ptr<OrderedEventRecord> eventRecord = std::make_shared<OrderedEventRecord>();
    eventRecord->commonEventData = commonEventData;
    eventRecord->publishInfo = publishInfo;
    eventRecord->resultTo = nullptr;
    eventRecord->state = OrderedEventRecord::IDLE;
    eventRecord->nextReceiver = 1;
    eventRecord->deliveryState.emplace_back(OrderedEventRecord::PENDING);
    eventRecord->receivers.emplace_back(subscriberRecord);

    commonEventControlManager->scheduled_ = true;
    bool ret = commonEventControlManager->EnqueueOrderedRecord(eventRecord);
    EXPECT_TRUE(ret);
    commonEventControlManager->CurrentOrderedEventTimeout(true);

    bool result = false;
    if (commonEventControlManager->orderedEventQueue_.front()->nextReceiver > 0) {
        GTEST_LOG_(INFO) << std::to_string(commonEventControlManager->orderedEventQueue_.front()->nextReceiver);
        result = true;
    }
    EXPECT_TRUE(result);
    commonEventControlManager->orderedEventQueue_.clear();
}

/*
 * @tc.number: CommonEventPublishOrderedUnitTest_2200
 * @tc.name: test CancelTimeout
 * @tc.desc: Verify CancelTimeout return true when pendingTimeoutMessage is true
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_2200, Function | MediumTest | Level1)
{
    commonEventControlManager->pendingTimeoutMessage_ = true;
    bool result = commonEventControlManager->CancelTimeout();
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CommonEventPublishOrderedUnitTest_2300
 * @tc.name: test CancelTimeout
 * @tc.desc: Verify CancelTimeout return true when pendingTimeoutMessage is false
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_2300, Function | MediumTest | Level1)
{
    bool result = false;
    commonEventControlManager->pendingTimeoutMessage_ = false;
    result = commonEventControlManager->CancelTimeout();
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CommonEventPublishOrderedUnitTest_2400
 * @tc.name: test PublishCommonEvent
 * @tc.desc: 1.Set thread mode handler
 *           2.Verify InnerCommonEventManager PublishCommonEvent success
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_2400, Function | MediumTest | Level1)
{
    // make a want
    Want want;
    want.SetAction(ACTION);

    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    MatchingSkills matchingSkills;

    // make subscriber info
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetThreadMode(CommonEventSubscribeInfo::ThreadMode::HANDLER);

    // make a subscriber object
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);

    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);

    mtx.lock();

    struct tm curTime {
        0
    };
    OHOS::Security::AccessToken::AccessTokenID tokenID = 0;
    // publish ordered event
    bool result = innerCommonEventManager.PublishCommonEvent(
        data, publishInfo, commonEventListener, curTime, PID, SYSTEM_UID, tokenID, UNDEFINED_USER, "bundlename");

    EXPECT_TRUE(result);

    int count = 0;
    while (!mtx.try_lock()) {
        if (count == 0) {
            GTEST_LOG_(INFO) << "Wait OnReceive callback function process";
            count = 1;
        } else {
            usleep(100 * 1000);
        }
    }

    mtx.unlock();
    GTEST_LOG_(INFO) << "Testcase finished";
}

/*
 * @tc.number: CommonEventPublishOrderedEventUnitTest_2500
 * @tc.name: test PublishCommonEvent
 * @tc.desc: 1.Set thread mode POST
 *           2.Verify InnerCommonEventManager PublishCommonEvent success
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_2500, Function | MediumTest | Level1)
{
    // make a want
    Want want;
    want.SetAction(ACTION);

    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    MatchingSkills matchingSkills;

    // make subscriber info
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetThreadMode(CommonEventSubscribeInfo::ThreadMode::POST);

    // make a subscriber object
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);

    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);

    mtx.lock();

    struct tm curTime {
        0
    };
    OHOS::Security::AccessToken::AccessTokenID tokenID = 0;
    // publish ordered event
    bool result = innerCommonEventManager.PublishCommonEvent(
        data, publishInfo, commonEventListener, curTime, PID, SYSTEM_UID, tokenID, UNDEFINED_USER, "bundlename");

    EXPECT_TRUE(result);

    int count = 0;
    while (!mtx.try_lock()) {
        if (count == 0) {
            GTEST_LOG_(INFO) << "Wait OnReceive callback function process";
            count = 1;
        } else {
            usleep(100 * 1000);
        }
    }

    mtx.unlock();
    GTEST_LOG_(INFO) << "Testcase finished";
}

/*
 * @tc.number: CommonEventPublishOrderedEventUnitTest_2600
 * @tc.name: test PublishCommonEvent
 * @tc.desc: 1.Set thread mode ASYNC
 *           2.Verify InnerCommonEventManager PublishCommonEvent success
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_2600, Function | MediumTest | Level1)
{
    // make a want
    Want want;
    want.SetAction(ACTION);

    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    MatchingSkills matchingSkills;

    // make subscriber info
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetThreadMode(CommonEventSubscribeInfo::ThreadMode::ASYNC);

    // make a subscriber object
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);

    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);

    mtx.lock();

    struct tm curTime {
        0
    };
    OHOS::Security::AccessToken::AccessTokenID tokenID = 0;
    // publish ordered event
    bool result = innerCommonEventManager.PublishCommonEvent(
        data, publishInfo, commonEventListener, curTime, PID, SYSTEM_UID, tokenID, UNDEFINED_USER, "bundlename");

    EXPECT_TRUE(result);

    int count = 0;
    while (!mtx.try_lock()) {
        if (count == 0) {
            GTEST_LOG_(INFO) << "Wait OnReceive callback function process";
            count = 1;
        } else {
            usleep(100 * 1000);
        }
    }

    mtx.unlock();
    GTEST_LOG_(INFO) << "Testcase finished";
}

/*
 * @tc.number: CommonEventPublishOrderedEventUnitTest_2700
 * @tc.name: test PublishCommonEvent
 * @tc.desc: 1.Set thread mode BACKGROUND
 *           2.Verify InnerCommonEventManager PublishCommonEvent success
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_2700, Function | MediumTest | Level1)
{
    // make a want
    Want want;
    want.SetAction(ACTION);

    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    MatchingSkills matchingSkills;

    // make subscriber info
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetThreadMode(CommonEventSubscribeInfo::ThreadMode::BACKGROUND);

    // make a subscriber object
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);

    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);

    mtx.lock();

    struct tm curTime {
        0
    };
    OHOS::Security::AccessToken::AccessTokenID tokenID = 0;
    // publish ordered event
    bool result = innerCommonEventManager.PublishCommonEvent(
        data, publishInfo, commonEventListener, curTime, PID, SYSTEM_UID, tokenID, UNDEFINED_USER, "bundlename");

    EXPECT_TRUE(result);

    int count = 0;
    while (!mtx.try_lock()) {
        if (count == 0) {
            GTEST_LOG_(INFO) << "Wait OnReceive callback function process";
            count = 1;
        } else {
            usleep(100 * 1000);
        }
    }

    mtx.unlock();
    GTEST_LOG_(INFO) << "Testcase finished";
}
