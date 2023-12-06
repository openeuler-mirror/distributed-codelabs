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

#define private public
#define protected public
#include "bundle_manager_helper.h"
#undef private
#undef protected

#include "common_event_constant.h"
#include "common_event_listener.h"
#include "datetime_ex.h"
#include "inner_common_event_manager.h"
#include "mock_bundle_manager.h"

using namespace testing::ext;
using namespace OHOS::EventFwk;
using namespace OHOS::AppExecFwk;

namespace {
const std::string EVENT = "com.ces.test.event";
const std::string ENTITY = "com.ces.test.entity";
const std::string SCHEME = "com.ces.test.scheme";
const std::string PERMISSION = "com.ces.test.permission";
const std::string DEVICEDID = "deviceId";
const std::string TYPE = "type";
const std::string BUNDLE = "BundleName";
const std::string ABILITY = "AbilityName";
constexpr uint8_t PRIORITY = 1;
constexpr uint8_t FLAG = 1;
constexpr uint8_t CODE = 1;
const std::string DATA = "DATA";

const std::string EVENT2 = "com.ces.test.event2";
const std::string ENTITY2 = "com.ces.test.entity2";
const std::string SCHEME2 = "com.ces.test.scheme2";
const std::string PERMISSION2 = "com.ces.test.permission2";
const std::string DEVICEDID2 = "deviceId2";
const std::string TYPE2 = "type2";
constexpr uint8_t PRIORITY2 = 2;
constexpr uint8_t FLAG2 = 2;
constexpr uint8_t CODE2 = 2;
const std::string DATA2 = "DATA2";

const std::string EVENT3 = "com.ces.test.event3";
constexpr uint8_t INNITCODE = 0;
constexpr uint8_t CHANGECODE = 1;
constexpr uint8_t CHANGECODE2 = 2;
const std::string EVENTCASE1 = "com.ces.test.event.case1";
const std::string EVENTCASE2 = "com.ces.test.event.case2";
const std::string EVENTCASE3 = "com.ces.test.event.case3";
const std::string INNITDATA = "com.ces.test.initdata";
const std::string CHANGEDATA = "com.ces.test.changedata";
const std::string CHANGEDATA2 = "com.ces.test.changedata2";
constexpr uid_t UID = 10;
constexpr uid_t UID2 = 11;
constexpr uint8_t STATE_INDEX1 = 1;
constexpr uint8_t STATE_INDEX2 = 2;
constexpr uint8_t STATE_INDEX3 = 3;
constexpr uint8_t PUBLISH_COUNT = 60;
constexpr uint8_t DUMP_SUBSCRIBER_COUNT_ONE = 1;
constexpr uint8_t DUMP_SUBSCRIBER_COUNT_TWO = 2;
constexpr uint8_t DUMP_STICKY_COUNT_ONE = 1;
constexpr uint8_t DUMP_STICKY_COUNT_TWO = 2;
constexpr uint8_t DUMP_PENDING_COUNT_ONE = 1;
constexpr uint8_t DUMP_HISTORY_COUNT_ONE = 1;
constexpr uint8_t DUMP_HISTORY_COUNT_TWO = 2;
constexpr uint8_t DUMP_HISTORY_COUNT_THREE = 3;
constexpr uint8_t DUMP_HISTORY_COUNT_MAX = 100;

constexpr uint8_t PID = 0;
constexpr uint16_t SYSTEM_UID = 1000;

static OHOS::sptr<OHOS::IRemoteObject> bundleObject = nullptr;
OHOS::sptr<OHOS::IRemoteObject> commonEventListener;
OHOS::sptr<OHOS::IRemoteObject> commonEventListener2;
OHOS::sptr<OHOS::IRemoteObject> commonEventListener3;

class CommonEventDumpTest : public testing::Test {
public:
    CommonEventDumpTest()
    {}
    ~CommonEventDumpTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    bool SubscribeCommonEvent(const std::shared_ptr<CommonEventSubscriber> &subscriber, uid_t callingUid,
        OHOS::sptr<OHOS::IRemoteObject> &commonEventListener);
    bool PublishCommonEvent(const CommonEventData &data, const CommonEventPublishInfo &publishInfo,
        const std::shared_ptr<CommonEventSubscriber> &subscriber, OHOS::sptr<OHOS::IRemoteObject> &commonEventListener);
    static bool FinishReceiver(
        const OHOS::sptr<OHOS::IRemoteObject> &proxy, const int &code, const std::string &data, const bool &abortEvent);
    void AsyncProcess();
    std::shared_ptr<InnerCommonEventManager> GetInnerCommonEventManager();
    static void DumpInfoCount(const std::vector<std::string> &state, int desSubscribersCount, int desStickyCount,
        int desOrderedCount, int desHistoryCount);
    void SetPublishDataByOrdered(CommonEventData &data, CommonEventPublishInfo &publishInfo);
    void SetPublishDataByOrdered2(CommonEventData &data, CommonEventPublishInfo &publishInfo);
    void SetPublishDataByUnordered(CommonEventData &data, CommonEventPublishInfo &publishInfo);
    void SubscribeDoubleEvent(
        int priority, const std::string &permission, const std::string &deviceId, CommonEventListener *&listener);
    void PublishUnorderedEvent(
        const std::string &event, const std::string &type,
        const int code, const std::string &data, const std::string &permission);
    void PublishStickyEvent(
        const std::string &event, const std::string &type,
        const int code, const std::string &data, const std::string &permission);
    void PublishStickyEvent(
        const std::string &event, const std::string &type, const int flag, const std::string &permission);

private:
    std::shared_ptr<EventRunner> runner_;
    static std::shared_ptr<EventHandler> handler_;
    static std::shared_ptr<InnerCommonEventManager> innerCommonEventManager_;
};

class SubscriberTest : public CommonEventSubscriber {
public:
    explicit SubscriberTest(const CommonEventSubscribeInfo &sp,
        const std::shared_ptr<InnerCommonEventManager> &innerCommonEventManager = nullptr)
        : CommonEventSubscriber(sp)
    {
        innerCommonEventManager_ = innerCommonEventManager;
        handler_ = std::make_shared<EventHandler>(EventRunner::Create());
    }

    ~SubscriberTest()
    {}

    virtual void OnReceiveEvent(const CommonEventData &data)
    {
        std::string action = data.GetWant().GetAction();
        if (action == EVENTCASE1) {
            ProcessSubscriberTestCase1(data);
        } else if (action == EVENTCASE2) {
            ProcessSubscriberTestCase2(data);
        } else if (action == EVENTCASE3) {
            GTEST_LOG_(INFO) << "ProcessSubscriberTestCase3:  start";
        } else {
        }
    }

private:
    void ProcessSubscriberTestCase1(CommonEventData data)
    {
        GTEST_LOG_(INFO) << "ProcessSubscriberTestCase1:  start";
        std::shared_ptr<AsyncCommonEventResult> result = GoAsyncCommonEvent();
        std::function<void()> asyncProcessFunc = std::bind(&SubscriberTest::AsyncProcess, this, commonEventListener);
        handler_->PostTask(asyncProcessFunc);
    }
    void ProcessSubscriberTestCase2(CommonEventData data)
    {
        GTEST_LOG_(INFO) << "ProcessSubscriberTestCase2:  start";
        std::shared_ptr<AsyncCommonEventResult> result = GoAsyncCommonEvent();
        std::function<void()> asyncProcessFunc = std::bind(&SubscriberTest::AsyncProcess, this, commonEventListener);
        handler_->PostTask(asyncProcessFunc);
    }

    void AsyncProcess(OHOS::sptr<OHOS::IRemoteObject> commonEventListener)
    {
        EXPECT_TRUE(CommonEventDumpTest::FinishReceiver(commonEventListener, CHANGECODE, CHANGEDATA, false));
    }

private:
    std::shared_ptr<EventHandler> handler_;
    std::shared_ptr<InnerCommonEventManager> innerCommonEventManager_;
};

class SubscriberTest2 : public CommonEventSubscriber {
public:
    explicit SubscriberTest2(const CommonEventSubscribeInfo &sp,
        const std::shared_ptr<InnerCommonEventManager> &innerCommonEventManager = nullptr)
        : CommonEventSubscriber(sp)
    {
        innerCommonEventManager_ = innerCommonEventManager;
        handler_ = std::make_shared<EventHandler>(EventRunner::Create());
    }

    ~SubscriberTest2()
    {}

    virtual void OnReceiveEvent(const CommonEventData &data)
    {
        std::string action = data.GetWant().GetAction();
        if (action == EVENTCASE1) {
            ProcessSubscriberTest2Case1(data);
        } else if (action == EVENTCASE2) {
            ProcessSubscriberTest2Case2(data);
        } else if (action == EVENTCASE3) {
            GTEST_LOG_(INFO) << "ProcessSubscriberTest2Case3:  start";
        } else {
        }
    }

private:
    void ProcessSubscriberTest2Case1(CommonEventData data)
    {
        GTEST_LOG_(INFO) << "ProcessSubscriberTest2Case1:  start";
        std::shared_ptr<AsyncCommonEventResult> result = GoAsyncCommonEvent();
        if (innerCommonEventManager_) {
            std::vector<std::string> state;
            innerCommonEventManager_->DumpState(static_cast<int32_t>(DumpEventType::ALL), "", ALL_USER, state);
            CommonEventDumpTest::DumpInfoCount(state,
                DUMP_SUBSCRIBER_COUNT_TWO, DUMP_STICKY_COUNT_TWO, DUMP_PENDING_COUNT_ONE, 0);
        }
        std::function<void()> asyncProcessFunc = std::bind(&SubscriberTest2::AsyncProcess, this, commonEventListener2);
        handler_->PostTask(asyncProcessFunc);
    }
    void ProcessSubscriberTest2Case2(CommonEventData data)
    {
        GTEST_LOG_(INFO) << "ProcessSubscriberTest2Case2:  start";
        std::shared_ptr<AsyncCommonEventResult> result = GoAsyncCommonEvent();
        std::function<void()> asyncProcessFunc = std::bind(&SubscriberTest2::AsyncProcess, this, commonEventListener2);
        handler_->PostTask(asyncProcessFunc);
    }

    void AsyncProcess(OHOS::sptr<OHOS::IRemoteObject> commonEventListener)
    {
        EXPECT_TRUE(CommonEventDumpTest::FinishReceiver(commonEventListener, CHANGECODE2, CHANGEDATA2, false));
    }

private:
    std::shared_ptr<EventHandler> handler_;
    std::shared_ptr<InnerCommonEventManager> innerCommonEventManager_;
};

class SubscriberTestLast : public CommonEventSubscriber {
public:
    SubscriberTestLast() : CommonEventSubscriber()
    {
        handler_ = std::make_shared<EventHandler>(EventRunner::Create());
    }

    explicit SubscriberTestLast(const CommonEventSubscribeInfo &sp) : CommonEventSubscriber(sp)
    {
        handler_ = std::make_shared<EventHandler>(EventRunner::Create());
    }

    ~SubscriberTestLast()
    {}

    virtual void OnReceiveEvent(const CommonEventData &data)
    {
        std::string action = data.GetWant().GetAction();
        if (action == EVENTCASE1) {
            ProcessSubscriberTestLastCase1(data);
        } else if (action == EVENTCASE2) {
            ProcessSubscriberTestLastCase2(data);
        } else {
        }
    }

private:
    void ProcessSubscriberTestLastCase1(CommonEventData data)
    {
        GTEST_LOG_(INFO) << "ProcessSubscriberTestLastCase1:  start";
        std::shared_ptr<AsyncCommonEventResult> result = GoAsyncCommonEvent();
        std::function<void()> asyncProcessFunc =
            std::bind(&SubscriberTestLast::AsyncProcess, this, commonEventListener3);
        handler_->PostTask(asyncProcessFunc);
    }
    void ProcessSubscriberTestLastCase2(CommonEventData data)
    {
        GTEST_LOG_(INFO) << "ProcessSubscriberTestLastCase2:  start";
    }

    void AsyncProcess(OHOS::sptr<OHOS::IRemoteObject> commonEventListener)
    {
        EXPECT_TRUE(CommonEventDumpTest::FinishReceiver(commonEventListener, CHANGECODE2, CHANGEDATA2, false));
    }

private:
    std::shared_ptr<EventHandler> handler_;
};

std::shared_ptr<EventHandler> CommonEventDumpTest::handler_ = nullptr;
std::shared_ptr<InnerCommonEventManager> CommonEventDumpTest::innerCommonEventManager_ = nullptr;

void CommonEventDumpTest::SetUpTestCase(void)
{
    bundleObject = new MockBundleMgrService();
    OHOS::DelayedSingleton<BundleManagerHelper>::GetInstance()->sptrBundleMgr_ =
        OHOS::iface_cast<OHOS::AppExecFwk::IBundleMgr>(bundleObject);
}

void CommonEventDumpTest::TearDownTestCase(void)
{}

void CommonEventDumpTest::SetUp(void)
{
    runner_ = EventRunner::Create(true);
    if (!runner_) {
        return;
    }
    handler_ = std::make_shared<EventHandler>(runner_);
    innerCommonEventManager_ = std::make_shared<InnerCommonEventManager>();
}

void CommonEventDumpTest::TearDown(void)
{}

bool CommonEventDumpTest::SubscribeCommonEvent(const std::shared_ptr<CommonEventSubscriber> &subscriber,
    uid_t callingUid, OHOS::sptr<OHOS::IRemoteObject> &commonEventListener)
{
    OHOS::sptr<IEventReceive> listener = new CommonEventListener(subscriber);
    if (!listener) {
        return false;
    }
    commonEventListener = listener->AsObject();
    struct tm recordTime = {0};
    if (!OHOS::GetSystemCurrentTime(&recordTime)) {
        return false;
    }
    pid_t callingPid = 10;
    OHOS::Security::AccessToken::AccessTokenID tokenID = 0;

    std::string bundleName = "";
    std::function<void()> SubscribeCommonEventFunc = std::bind(&InnerCommonEventManager::SubscribeCommonEvent,
        innerCommonEventManager_,
        subscriber->GetSubscribeInfo(),
        commonEventListener,
        recordTime,
        callingPid,
        callingUid,
        tokenID,
        bundleName);
    return handler_->PostTask(SubscribeCommonEventFunc);
}

bool CommonEventDumpTest::PublishCommonEvent(const CommonEventData &data, const CommonEventPublishInfo &publishInfo,
    const std::shared_ptr<CommonEventSubscriber> &subscriber, OHOS::sptr<OHOS::IRemoteObject> &commonEventListener)
{
    if (commonEventListener == nullptr && publishInfo.IsOrdered()) {
        OHOS::sptr<IEventReceive> listener = new CommonEventListener(subscriber);
        if (!listener) {
            return false;
        }
        commonEventListener = listener->AsObject();
    } else if (!publishInfo.IsOrdered()) {
        commonEventListener = nullptr;
    }

    struct tm recordTime = {0};
    if (!OHOS::GetSystemCurrentTime(&recordTime)) {
        return false;
    }
    pid_t callingPid = 20;
    uid_t callingUid = 21;
    OHOS::Security::AccessToken::AccessTokenID tokenID = 0;
    int32_t userId = UNDEFINED_USER;
    std::string bundleName = "";

    std::function<void()> PublishCommonEventFunc = std::bind(&InnerCommonEventManager::PublishCommonEvent,
        innerCommonEventManager_,
        data,
        publishInfo,
        commonEventListener,
        recordTime,
        callingPid,
        callingUid,
        tokenID,
        userId,
        bundleName,
        nullptr);
    return handler_->PostTask(PublishCommonEventFunc);
}

bool CommonEventDumpTest::FinishReceiver(
    const OHOS::sptr<OHOS::IRemoteObject> &proxy, const int &code, const std::string &data, const bool &abortEvent)
{
    std::function<void()> FinishReceiverFunc =
        std::bind(&InnerCommonEventManager::FinishReceiver, innerCommonEventManager_, proxy, code, data, abortEvent);
    return handler_->PostTask(FinishReceiverFunc);
}

std::shared_ptr<InnerCommonEventManager> CommonEventDumpTest::GetInnerCommonEventManager()
{
    return innerCommonEventManager_;
}

void CommonEventDumpTest::DumpInfoCount(const std::vector<std::string> &state, int desSubscribersCount,
    int desStickyCount, int desPendingCount, int desHistoryCount)
{
    int subscribersNum = 0;
    int stickyNum = 0;
    int pendingCount = 0;
    int historyCount = 0;
    bool isSubscribers = false;
    bool isSticky = false;
    bool isPending = false;
    bool isHistory = false;
    for (auto vec : state) {
        GTEST_LOG_(INFO) << vec;
        auto pos = vec.find("No information");
        if (pos != string::npos) {
            auto pos2 = vec.find("\tSubscribers:\tNo information");
            if (pos2 != string::npos) {
            } else {
                continue;
            }
        }
        if (!isPending && !isHistory) {
            auto pos = vec.find("Subscribers:");
            if (pos != string::npos) {
                isSubscribers = true;
            }
        }
        pos = vec.find("Sticky Events:");
        if (pos != string::npos) {
            isSubscribers = false;
            isSticky = true;
        }
        pos = vec.find("Pending Events:");
        if (pos != string::npos) {
            isSubscribers = false;
            isSticky = false;
            isPending = true;
        }
        pos = vec.find("History Events:");
        if (pos != string::npos) {
            isSubscribers = false;
            isSticky = false;
            isPending = false;
            isHistory = true;
        }
        if (isSubscribers) {
            subscribersNum++;
        } else if (isSticky) {
            stickyNum++;
        } else if (isPending) {
            pendingCount++;
        } else if (isHistory) {
            historyCount++;
        }
    }

    EXPECT_EQ(subscribersNum, desSubscribersCount);
    EXPECT_EQ(stickyNum, desStickyCount);
    EXPECT_EQ(pendingCount, desPendingCount);
    EXPECT_EQ(historyCount, desHistoryCount);
}

void CommonEventDumpTest::SetPublishDataByOrdered(CommonEventData &data, CommonEventPublishInfo &publishInfo)
{
    // make a want
    Want want;
    want.SetAction(EVENTCASE1);
    want.AddEntity(ENTITY);
    want.AddEntity(ENTITY2);
    want.SetType(TYPE);
    want.SetFlags(FLAG);

    OHOS::AppExecFwk::ElementName element;
    element.SetBundleName(BUNDLE);
    element.SetAbilityName(ABILITY);
    element.SetDeviceID(DEVICEDID);
    want.SetElement(element);

    // make common event data
    data.SetWant(want);
    data.SetData(INNITDATA);
    data.SetCode(INNITCODE);

    publishInfo.SetOrdered(true);
}

void CommonEventDumpTest::SetPublishDataByOrdered2(CommonEventData &data, CommonEventPublishInfo &publishInfo)
{
    // make a want
    Want want;
    want.SetAction(EVENTCASE2);
    want.AddEntity(ENTITY);
    want.AddEntity(ENTITY2);
    want.SetType(TYPE);
    want.SetFlags(FLAG);

    OHOS::AppExecFwk::ElementName element;
    element.SetBundleName(BUNDLE);
    element.SetAbilityName(ABILITY);
    element.SetDeviceID(DEVICEDID);
    want.SetElement(element);

    // make common event data
    data.SetWant(want);
    data.SetData(INNITDATA);
    data.SetCode(INNITCODE);

    publishInfo.SetOrdered(true);
}

void CommonEventDumpTest::SetPublishDataByUnordered(CommonEventData &data, CommonEventPublishInfo &publishInfo)
{
    // make a want
    Want want;
    want.SetAction(EVENTCASE3);
    want.AddEntity(ENTITY);
    want.AddEntity(ENTITY2);
    want.SetType(TYPE);
    want.SetFlags(FLAG);

    OHOS::AppExecFwk::ElementName element;
    element.SetBundleName(BUNDLE);
    element.SetAbilityName(ABILITY);
    element.SetDeviceID(DEVICEDID);
    want.SetElement(element);

    // make common event data
    data.SetWant(want);
    data.SetData(INNITDATA);
    data.SetCode(INNITCODE);

    publishInfo.SetOrdered(false);
}

void CommonEventDumpTest::SubscribeDoubleEvent(
    int priority, const std::string &permission, const std::string &deviceId, CommonEventListener *&listener)
{
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);
    matchingSkills.AddEvent(EVENT2);
    matchingSkills.AddEntity(ENTITY);
    matchingSkills.AddEntity(ENTITY2);

    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(priority);
    subscribeInfo.SetPermission(permission);
    subscribeInfo.SetDeviceId(deviceId);

    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);
    listener = new (std::nothrow) CommonEventListener(subscriber);

    struct tm curTime {0};
    OHOS::Security::AccessToken::AccessTokenID tokenID = 1;
    innerCommonEventManager_->SubscribeCommonEvent(
        subscribeInfo, listener->AsObject(), curTime, PID, SYSTEM_UID, tokenID, "hello");
}

void CommonEventDumpTest::PublishUnorderedEvent(
    const std::string &event, const std::string &type,
    const int code, const std::string &data, const std::string &permission)
{
    Want want;
    want.SetAction(event);
    want.AddEntity(ENTITY);
    want.AddEntity(ENTITY2);
    want.SetType(type);

    OHOS::AppExecFwk::ElementName element;
    element.SetBundleName(BUNDLE);
    element.SetAbilityName(ABILITY);
    element.SetDeviceID(DEVICEDID);
    want.SetElement(element);

    // make common event data
    CommonEventData eventData;
    eventData.SetWant(want);
    eventData.SetCode(code);
    eventData.SetData(data);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(false);
    std::vector<std::string> permissions;
    permissions.emplace_back(permission);
    publishInfo.SetSubscriberPermissions(permissions);

    struct tm curTime {0};
    OHOS::Security::AccessToken::AccessTokenID tokenID = 1;
    innerCommonEventManager_->PublishCommonEvent(
        eventData, publishInfo, nullptr, curTime, PID, SYSTEM_UID, tokenID, UNDEFINED_USER, "hello");
}

void CommonEventDumpTest::PublishStickyEvent(
    const std::string &event, const std::string &type,
    const int code, const std::string &data, const std::string &permission)
{
    Want want;
    want.SetAction(event);
    want.AddEntity(ENTITY);
    want.AddEntity(ENTITY2);
    want.SetType(type);

    OHOS::AppExecFwk::ElementName element;
    element.SetBundleName(BUNDLE);
    element.SetAbilityName(ABILITY);
    element.SetDeviceID(DEVICEDID);
    want.SetElement(element);

    // make common event data
    CommonEventData eventData;
    eventData.SetWant(want);
    eventData.SetCode(code);
    eventData.SetData(data);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(true);
    std::vector<std::string> permissions;
    permissions.emplace_back(permission);
    publishInfo.SetSubscriberPermissions(permissions);

    struct tm curTime {0};
    OHOS::Security::AccessToken::AccessTokenID tokenID = 0;
    innerCommonEventManager_->PublishCommonEvent(
        eventData, publishInfo, nullptr, curTime, PID, SYSTEM_UID, tokenID, UNDEFINED_USER, "hello");
}

void CommonEventDumpTest::PublishStickyEvent(
    const std::string &event, const std::string &type, const int flag, const std::string &permission)
{
    Want want;
    want.SetAction(event);
    want.AddEntity(ENTITY);
    want.AddEntity(ENTITY2);
    want.SetType(type);
    want.SetFlags(flag);

    OHOS::AppExecFwk::ElementName element;
    element.SetBundleName(BUNDLE);
    element.SetAbilityName(ABILITY);
    element.SetDeviceID(DEVICEDID);
    want.SetElement(element);

    // make common event data
    CommonEventData eventData;
    eventData.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(true);
    std::vector<std::string> permissions;
    permissions.emplace_back(permission);
    publishInfo.SetSubscriberPermissions(permissions);

    struct tm curTime {0};
    OHOS::Security::AccessToken::AccessTokenID tokenID = 0;
    innerCommonEventManager_->PublishCommonEvent(
        eventData, publishInfo, nullptr, curTime, PID, SYSTEM_UID, tokenID, UNDEFINED_USER, "hello");
}

/*
 * @tc.number: CommonEventDumpTest_0100
 * @tc.name: test dump
 * @tc.desc: Verify dump information after publishing unordered events
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_0100, Function | MediumTest | Level1)
{
    /* Subscribe */
    CommonEventListener *listener = nullptr;
    CommonEventListener *listener2 = nullptr;
    SubscribeDoubleEvent(PRIORITY, PERMISSION, DEVICEDID, listener);
    SubscribeDoubleEvent(PRIORITY2, PERMISSION2, DEVICEDID2, listener2);
    EXPECT_NE(nullptr, listener);
    EXPECT_NE(nullptr, listener2);

    /* Publish */
    PublishUnorderedEvent(EVENT, TYPE, CODE, DATA, PERMISSION);
    PublishUnorderedEvent(EVENT2, TYPE2, CODE2, DATA2, PERMISSION2);

    usleep(50000);

    std::vector<std::string> state;
    GetInnerCommonEventManager()->DumpState(static_cast<int32_t>(DumpEventType::ALL), "", ALL_USER, state);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    ASSERT_NE(state.size(), 0);
    DumpInfoCount(state, DUMP_SUBSCRIBER_COUNT_TWO, 0, 0, DUMP_HISTORY_COUNT_TWO);
    EXPECT_EQ("Sticky Events:\tNo information", state[STATE_INDEX2]);

    GetInnerCommonEventManager()->UnsubscribeCommonEvent(listener->AsObject());
    GetInnerCommonEventManager()->UnsubscribeCommonEvent(listener2->AsObject());
}

/*
 * @tc.number: CommonEventDumpTest_0200
 * @tc.name: test dump
 * @tc.desc: Verify dump information after publishing sticky events
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_0200, Function | MediumTest | Level1)
{
    /* Subscribe */
    CommonEventListener *listener = nullptr;
    CommonEventListener *listener2 = nullptr;
    SubscribeDoubleEvent(PRIORITY, PERMISSION, DEVICEDID, listener);
    SubscribeDoubleEvent(PRIORITY2, PERMISSION2, DEVICEDID2, listener2);
    EXPECT_NE(nullptr, listener);
    EXPECT_NE(nullptr, listener2);

    /* Publish */
    PublishStickyEvent(EVENT, TYPE, CODE, DATA, PERMISSION);
    PublishStickyEvent(EVENT2, TYPE2, CODE2, DATA2, PERMISSION2);

    usleep(50000);

    std::vector<std::string> state;
    GetInnerCommonEventManager()->DumpState(static_cast<int32_t>(DumpEventType::ALL), "", ALL_USER, state);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    DumpInfoCount(state, DUMP_SUBSCRIBER_COUNT_TWO, DUMP_STICKY_COUNT_TWO, 0, DUMP_HISTORY_COUNT_TWO);

    GetInnerCommonEventManager()->UnsubscribeCommonEvent(listener->AsObject());
    GetInnerCommonEventManager()->UnsubscribeCommonEvent(listener2->AsObject());
}

/*
 * @tc.number: CommonEventDumpTest_0300
 * @tc.name: test dump
 * @tc.desc: Verify dump information after updating sticky events
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_0300, Function | MediumTest | Level1)
{
    /* Publish */
    PublishStickyEvent(EVENT, TYPE, FLAG, PERMISSION);
    PublishStickyEvent(EVENT2, TYPE2, FLAG2, PERMISSION2);

    usleep(50000);

    std::vector<std::string> state;
    GetInnerCommonEventManager()->DumpState(static_cast<int32_t>(DumpEventType::ALL), "", ALL_USER, state);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    ASSERT_NE(state.size(), 0);
    DumpInfoCount(state, 0, DUMP_STICKY_COUNT_TWO, 0, DUMP_HISTORY_COUNT_TWO);
    EXPECT_EQ("Subscribers:\tNo information", state[0]);
}

/*
 * @tc.number: CommonEventDumpTest_0400
 * @tc.name: test dump
 * @tc.desc: Verify dump information after subscribing
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_0400, Function | MediumTest | Level1)
{
    /* Subscribe */
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);
    matchingSkills.AddScheme(SCHEME);

    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(PRIORITY);
    subscribeInfo.SetPermission(PERMISSION);
    subscribeInfo.SetDeviceId(DEVICEDID);

    struct tm curTime {0};
    OHOS::Security::AccessToken::AccessTokenID tokenID = 1;

    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);
    CommonEventListener *listener = new CommonEventListener(subscriber);
    GetInnerCommonEventManager()->SubscribeCommonEvent(
        subscribeInfo, listener->AsObject(), curTime, PID, SYSTEM_UID, tokenID, "hello");

    usleep(50000);

    std::vector<std::string> state;
    GetInnerCommonEventManager()->DumpState(static_cast<int32_t>(DumpEventType::ALL), "", ALL_USER, state);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    DumpInfoCount(state, DUMP_SUBSCRIBER_COUNT_ONE, DUMP_STICKY_COUNT_TWO, 0, DUMP_HISTORY_COUNT_ONE);

    GetInnerCommonEventManager()->UnsubscribeCommonEvent(listener->AsObject());
}

/*
 * @tc.number: CommonEventDumpTest_0500
 * @tc.name: test dump
 * @tc.desc: Verify dump information after subscribing
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_0500, Function | MediumTest | Level1)
{
    /* Subscribe */
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);

    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(PRIORITY);
    subscribeInfo.SetPermission(PERMISSION);
    subscribeInfo.SetDeviceId(DEVICEDID);

    struct tm curTime {0};
    OHOS::Security::AccessToken::AccessTokenID tokenID = 1;

    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);
    CommonEventListener *listener = new CommonEventListener(subscriber);
    GetInnerCommonEventManager()->SubscribeCommonEvent(
        subscribeInfo, listener->AsObject(), curTime, PID, SYSTEM_UID, tokenID, "hello");

    usleep(50000);

    std::vector<std::string> state;
    GetInnerCommonEventManager()->DumpState(static_cast<int32_t>(DumpEventType::ALL), "", ALL_USER, state);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    DumpInfoCount(state, DUMP_SUBSCRIBER_COUNT_ONE, DUMP_STICKY_COUNT_TWO, 0, DUMP_HISTORY_COUNT_ONE);

    GetInnerCommonEventManager()->UnsubscribeCommonEvent(listener->AsObject());
}

/*
 * @tc.number: CommonEventDumpTest_0600
 * @tc.name: test dump
 * @tc.desc: Verify dump information after subscribing
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_0600, Function | MediumTest | Level1)
{
    /* Subscribe */
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);

    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPermission(PERMISSION);
    subscribeInfo.SetDeviceId(DEVICEDID);

    struct tm curTime {0};
    OHOS::Security::AccessToken::AccessTokenID tokenID = 1;

    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);
    CommonEventListener *listener = new CommonEventListener(subscriber);
    GetInnerCommonEventManager()->SubscribeCommonEvent(
        subscribeInfo, listener->AsObject(), curTime, PID, SYSTEM_UID, tokenID, "hello");

    usleep(50000);

    std::vector<std::string> state;
    GetInnerCommonEventManager()->DumpState(static_cast<int32_t>(DumpEventType::ALL), "", ALL_USER, state);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    DumpInfoCount(state, DUMP_SUBSCRIBER_COUNT_ONE, DUMP_STICKY_COUNT_TWO, 0, DUMP_HISTORY_COUNT_ONE);

    GetInnerCommonEventManager()->UnsubscribeCommonEvent(listener->AsObject());
}

/*
 * @tc.number: CommonEventDumpTest_0700
 * @tc.name: test dump
 * @tc.desc: Verify dump information after subscribing
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_0700, Function | MediumTest | Level1)
{
    /* Subscribe */
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);

    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetDeviceId(DEVICEDID);

    struct tm curTime {0};
    OHOS::Security::AccessToken::AccessTokenID tokenID = 1;

    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);
    CommonEventListener *listener = new CommonEventListener(subscriber);
    GetInnerCommonEventManager()->SubscribeCommonEvent(
        subscribeInfo, listener->AsObject(), curTime, PID, SYSTEM_UID, tokenID, "hello");

    usleep(50000);

    std::vector<std::string> state;
    GetInnerCommonEventManager()->DumpState(static_cast<int32_t>(DumpEventType::ALL), "", ALL_USER, state);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    DumpInfoCount(state, DUMP_SUBSCRIBER_COUNT_ONE, DUMP_STICKY_COUNT_TWO, 0, DUMP_HISTORY_COUNT_ONE);

    GetInnerCommonEventManager()->UnsubscribeCommonEvent(listener->AsObject());
}

/*
 * @tc.number: CommonEventDumpTest_0800
 * @tc.name: test dump
 * @tc.desc: Verify dump information after subscribing
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_0800, Function | MediumTest | Level1)
{
    /* Subscribe */
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);

    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    struct tm curTime {0};
    OHOS::Security::AccessToken::AccessTokenID tokenID = 1;

    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);
    CommonEventListener *listener = new CommonEventListener(subscriber);
    GetInnerCommonEventManager()->SubscribeCommonEvent(
        subscribeInfo, listener->AsObject(), curTime, PID, SYSTEM_UID, tokenID, "hello");

    usleep(50000);

    std::vector<std::string> state;
    GetInnerCommonEventManager()->DumpState(static_cast<int32_t>(DumpEventType::ALL), "", ALL_USER, state);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    DumpInfoCount(state, DUMP_SUBSCRIBER_COUNT_ONE, DUMP_STICKY_COUNT_TWO, 0, DUMP_HISTORY_COUNT_ONE);

    GetInnerCommonEventManager()->UnsubscribeCommonEvent(listener->AsObject());
}

/*
 * @tc.number: CommonEventDumpTest_0900
 * @tc.name: test dump
 * @tc.desc: Verify dump information for unpublished event
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_0900, Function | MediumTest | Level1)
{
    /* Subscribe */
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);

    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    struct tm curTime {0};
    OHOS::Security::AccessToken::AccessTokenID tokenID = 1;

    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);
    CommonEventListener *listener = new CommonEventListener(subscriber);
    GetInnerCommonEventManager()->SubscribeCommonEvent(
        subscribeInfo, listener->AsObject(), curTime, PID, SYSTEM_UID, tokenID, "hello");

    usleep(50000);

    std::vector<std::string> state;
    GetInnerCommonEventManager()->DumpState(static_cast<int32_t>(DumpEventType::ALL), EVENT3, ALL_USER, state);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    ASSERT_NE(state.size(), 0);
    DumpInfoCount(state, 0, 0, 0, 0);
    EXPECT_EQ("Subscribers:\tNo information", state[0]);
    EXPECT_EQ("Sticky Events:\tNo information", state[STATE_INDEX1]);

    GetInnerCommonEventManager()->UnsubscribeCommonEvent(listener->AsObject());
}

/*
 * @tc.number: CommonEventDumpTest_1000
 * @tc.name: test dump
 * @tc.desc: Verify dump information for specified event
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_1000, TestSize.Level1)
{
    /* Subscribe */
    CommonEventListener *listener = nullptr;
    CommonEventListener *listener2 = nullptr;
    SubscribeDoubleEvent(PRIORITY, PERMISSION, DEVICEDID, listener);
    SubscribeDoubleEvent(PRIORITY2, PERMISSION2, DEVICEDID2, listener2);
    EXPECT_NE(nullptr, listener);
    EXPECT_NE(nullptr, listener2);

    /* Publish */
    PublishStickyEvent(EVENT, TYPE, FLAG, PERMISSION);
    PublishStickyEvent(EVENT2, TYPE2, FLAG2, PERMISSION2);

    usleep(50000);

    std::vector<std::string> state;
    GetInnerCommonEventManager()->DumpState(static_cast<int32_t>(DumpEventType::ALL), EVENT2, ALL_USER, state);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    DumpInfoCount(state, DUMP_SUBSCRIBER_COUNT_TWO, DUMP_STICKY_COUNT_ONE, 0, DUMP_HISTORY_COUNT_THREE);

    GetInnerCommonEventManager()->UnsubscribeCommonEvent(listener->AsObject());
    GetInnerCommonEventManager()->UnsubscribeCommonEvent(listener2->AsObject());
}

/*
 * @tc.number: CommonEventDumpTest_1100
 * @tc.name: test dump
 * @tc.desc: Verify dump information for sticky event
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_1100, Function | MediumTest | Level1)
{
    /* Publish */
    PublishStickyEvent(EVENT, TYPE, FLAG, PERMISSION);

    usleep(50000);

    std::vector<std::string> state;
    GetInnerCommonEventManager()->DumpState(static_cast<int32_t>(DumpEventType::ALL), "", ALL_USER, state);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    DumpInfoCount(state, 0, DUMP_STICKY_COUNT_TWO, 0, DUMP_HISTORY_COUNT_ONE);
}

/*
 * @tc.number: CommonEventDumpTest_1200
 * @tc.name: test dump
 * @tc.desc: Verify dump information for sticky event
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_1200, Function | MediumTest | Level1)
{
    /* Publish */
    PublishStickyEvent(EVENT, "", FLAG, PERMISSION);

    usleep(50000);

    std::vector<std::string> state;
    GetInnerCommonEventManager()->DumpState(static_cast<int32_t>(DumpEventType::ALL), "", ALL_USER, state);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    DumpInfoCount(state, 0, DUMP_STICKY_COUNT_TWO, 0, DUMP_HISTORY_COUNT_ONE);
}

/*
 * @tc.number: CommonEventDumpTest_1300
 * @tc.name: test dump
 * @tc.desc: Verify dump information for sticky event
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_1300, Function | MediumTest | Level1)
{
    /* Publish */
    PublishStickyEvent(EVENT, "", 0, PERMISSION);

    usleep(50000);

    std::vector<std::string> state;
    GetInnerCommonEventManager()->DumpState(static_cast<int32_t>(DumpEventType::ALL), "", ALL_USER, state);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    DumpInfoCount(state, 0, DUMP_STICKY_COUNT_TWO, 0, DUMP_HISTORY_COUNT_ONE);
}

/*
 * @tc.number: CommonEventDumpTest_1400
 * @tc.name: test dump
 * @tc.desc: Verify dump information for sticky event
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_1400, Function | MediumTest | Level1)
{
    /* Publish */
    PublishStickyEvent(EVENT, "", 0, PERMISSION);

    usleep(50000);

    std::vector<std::string> state;
    GetInnerCommonEventManager()->DumpState(static_cast<int32_t>(DumpEventType::ALL), "", ALL_USER, state);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    DumpInfoCount(state, 0, DUMP_STICKY_COUNT_TWO, 0, DUMP_HISTORY_COUNT_ONE);
}

/*
 * @tc.number: CommonEventDumpTest_1500
 * @tc.name: test dump
 * @tc.desc: Verify dump information for sticky event
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_1500, Function | MediumTest | Level1)
{
    /* Publish */
    PublishStickyEvent(EVENT, "", 0, PERMISSION);

    usleep(50000);

    std::vector<std::string> state;
    GetInnerCommonEventManager()->DumpState(static_cast<int32_t>(DumpEventType::ALL), "", ALL_USER, state);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    DumpInfoCount(state, 0, DUMP_STICKY_COUNT_TWO, 0, DUMP_HISTORY_COUNT_ONE);
}

/*
 * @tc.number: CommonEventDumpTest_1600
 * @tc.name: test dump
 * @tc.desc: Verify dump information for sticky event
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_1600, Function | MediumTest | Level1)
{
    /* Publish */
    // make a want
    Want want;
    want.SetAction(EVENT);
    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(true);

    std::vector<std::string> permissions;
    permissions.emplace_back(PERMISSION);
    publishInfo.SetSubscriberPermissions(permissions);

    struct tm curTime {0};
    OHOS::Security::AccessToken::AccessTokenID tokenID = 0;

    GetInnerCommonEventManager()->PublishCommonEvent(
        data, publishInfo, nullptr, curTime, PID, SYSTEM_UID, tokenID, UNDEFINED_USER, "hello");

    usleep(50000);

    std::vector<std::string> state;
    GetInnerCommonEventManager()->DumpState(static_cast<int32_t>(DumpEventType::ALL), "", ALL_USER, state);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    DumpInfoCount(state, 0, DUMP_STICKY_COUNT_TWO, 0, DUMP_HISTORY_COUNT_ONE);
}

/*
 * @tc.number: CommonEventDumpTest_1700
 * @tc.name: test dump
 * @tc.desc: Verify dump information for sticky event
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_1700, Function | MediumTest | Level1)
{
    /* Publish */
    // make a want
    Want want;
    want.SetAction(EVENT);
    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(true);

    struct tm curTime {0};
    OHOS::Security::AccessToken::AccessTokenID tokenID = 0;

    GetInnerCommonEventManager()->PublishCommonEvent(
        data, publishInfo, nullptr, curTime, PID, SYSTEM_UID, tokenID, UNDEFINED_USER, "hello");

    usleep(50000);

    std::vector<std::string> state;
    GetInnerCommonEventManager()->DumpState(static_cast<int32_t>(DumpEventType::ALL), "", ALL_USER, state);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    DumpInfoCount(state, 0, DUMP_STICKY_COUNT_TWO, 0, DUMP_HISTORY_COUNT_ONE);
}

/*
 * @tc.number: CommonEventDumpTest_1800
 * @tc.name: test dump
 * @tc.desc: Verify dump information when the pending ordered event is empty
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_1800, TestSize.Level1)
{
    std::vector<std::string> state;
    GetInnerCommonEventManager()->DumpState(static_cast<int32_t>(DumpEventType::ALL), "", ALL_USER, state);
    ASSERT_NE(state.size(), 0);
    DumpInfoCount(state, 0, DUMP_STICKY_COUNT_TWO, 0, 0);
    EXPECT_EQ("Pending Events:\tNo information", state[STATE_INDEX3]);
}

/*
 * @tc.number: CommonEventDumpTest_1900
 * @tc.name: test dump
 * @tc.desc: Verify dump information for ordered event
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_1900, TestSize.Level1)
{
    /* Subscribe */
    // make matching skills
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE1);
    matchingSkills.AddEntity(ENTITY);
    matchingSkills.AddEntity(ENTITY2);
    // make subscriber info
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(1);
    subscribeInfo.SetDeviceId(DEVICEDID);
    // make a subscriber object
    std::shared_ptr<SubscriberTest> subscriberTest =
        std::make_shared<SubscriberTest>(subscribeInfo, GetInnerCommonEventManager());
    // subscribe a common event
    EXPECT_TRUE(SubscribeCommonEvent(subscriberTest, UID, commonEventListener));
    usleep(50000);

    // make another subscriber info
    CommonEventSubscribeInfo subscribeInfo2(matchingSkills);
    subscribeInfo2.SetPriority(0);
    subscribeInfo2.SetDeviceId(DEVICEDID2);
    // make another subscriber object
    std::shared_ptr<SubscriberTest2> subscriberTest2 =
        std::make_shared<SubscriberTest2>(subscribeInfo2, GetInnerCommonEventManager());
    // subscribe another event
    EXPECT_TRUE(SubscribeCommonEvent(subscriberTest2, UID2, commonEventListener2));
    usleep(50000);

    std::shared_ptr<SubscriberTestLast> subscriber = std::make_shared<SubscriberTestLast>();

    /* Publish */
    CommonEventData data;
    CommonEventPublishInfo publishInfo;
    SetPublishDataByOrdered(data, publishInfo);

    // publish order event
    EXPECT_TRUE(PublishCommonEvent(data, publishInfo, subscriber, commonEventListener3));

    usleep(500000);

    std::vector<std::string> state;
    GetInnerCommonEventManager()->DumpState(static_cast<int32_t>(DumpEventType::ALL), "", ALL_USER, state);
    DumpInfoCount(state, DUMP_SUBSCRIBER_COUNT_TWO, DUMP_STICKY_COUNT_TWO, 0, DUMP_HISTORY_COUNT_ONE);

    GetInnerCommonEventManager()->UnsubscribeCommonEvent(commonEventListener);
    GetInnerCommonEventManager()->UnsubscribeCommonEvent(commonEventListener2);
}
/*
 * @tc.number: CommonEventDumpTest_2000
 * @tc.name: test dump
 * @tc.desc: Verify dump information for maximum number of history events
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_2000, TestSize.Level1)
{
    /* Subscribe */
    // make matching skills
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE2);
    matchingSkills.AddEntity(ENTITY);
    matchingSkills.AddEntity(ENTITY2);

    // make subscriber info
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(1);
    subscribeInfo.SetDeviceId(DEVICEDID);
    // make a subscriber object
    std::shared_ptr<SubscriberTest> subscriberTest =
        std::make_shared<SubscriberTest>(subscribeInfo, GetInnerCommonEventManager());
    // subscribe a common event
    EXPECT_TRUE(SubscribeCommonEvent(subscriberTest, UID, commonEventListener));
    usleep(50000);

    // make another subscriber info
    CommonEventSubscribeInfo subscribeInfo2(matchingSkills);
    subscribeInfo2.SetPriority(0);
    subscribeInfo2.SetDeviceId(DEVICEDID2);
    // make another subscriber object
    std::shared_ptr<SubscriberTest2> subscriberTest2 =
        std::make_shared<SubscriberTest2>(subscribeInfo2, GetInnerCommonEventManager());
    // subscribe another event
    EXPECT_TRUE(SubscribeCommonEvent(subscriberTest2, UID2, commonEventListener2));
    usleep(50000);

    std::shared_ptr<SubscriberTestLast> subscriber = std::make_shared<SubscriberTestLast>();

    /* Publish */
    CommonEventData data;
    CommonEventPublishInfo publishInfo;

    int count = 0;
    while (count < PUBLISH_COUNT) {
        // publish order event
        SetPublishDataByOrdered2(data, publishInfo);
        EXPECT_TRUE(PublishCommonEvent(data, publishInfo, subscriber, commonEventListener3));
        usleep(50000);

        // publish unorder event
        SetPublishDataByUnordered(data, publishInfo);
        EXPECT_TRUE(PublishCommonEvent(data, publishInfo, subscriber, commonEventListener3));
        usleep(50000);
        count++;
    }
    usleep(500000);

    std::vector<std::string> state;
    GetInnerCommonEventManager()->DumpState(static_cast<int32_t>(DumpEventType::ALL), "", ALL_USER, state);
    DumpInfoCount(state, DUMP_STICKY_COUNT_TWO, DUMP_STICKY_COUNT_TWO, 0, DUMP_HISTORY_COUNT_MAX);

    GetInnerCommonEventManager()->UnsubscribeCommonEvent(commonEventListener);
    GetInnerCommonEventManager()->UnsubscribeCommonEvent(commonEventListener2);
}

/*
 * @tc.number: CommonEventDumpPartEventTest_0100
 * @tc.name: test dump
 * @tc.desc: Verify dump all subscriber
 * @tc.require: I582WG
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpPartEventTest_0100, Function | MediumTest | Level1)
{
    /* Subscribe */
    CommonEventListener *listener = nullptr;
    CommonEventListener *listener2 = nullptr;
    SubscribeDoubleEvent(PRIORITY, PERMISSION, DEVICEDID, listener);
    SubscribeDoubleEvent(PRIORITY2, PERMISSION2, DEVICEDID2, listener2);
    EXPECT_NE(nullptr, listener);
    EXPECT_NE(nullptr, listener2);

    usleep(50000);

    std::vector<std::string> state;
    GetInnerCommonEventManager()->DumpState(static_cast<int32_t>(DumpEventType::SUBSCRIBER), "", ALL_USER, state);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    ASSERT_EQ(state.size(), DUMP_SUBSCRIBER_COUNT_TWO);
    auto pos = state[0].find("Subscribers:");
    ASSERT_NE(pos, string::npos);

    GetInnerCommonEventManager()->UnsubscribeCommonEvent(listener->AsObject());
    GetInnerCommonEventManager()->UnsubscribeCommonEvent(listener2->AsObject());
}

/*
 * @tc.number: CommonEventDumpPartEventTest_0200
 * @tc.name: test dump
 * @tc.desc: Verify dump information after publishing sticky events
 * @tc.require: I582WG
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpPartEventTest_0200, Function | MediumTest | Level1)
{
    /* Subscribe */
    CommonEventListener *listener = nullptr;
    CommonEventListener *listener2 = nullptr;
    SubscribeDoubleEvent(PRIORITY, PERMISSION, DEVICEDID, listener);
    SubscribeDoubleEvent(PRIORITY2, PERMISSION2, DEVICEDID2, listener2);
    EXPECT_NE(nullptr, listener);
    EXPECT_NE(nullptr, listener2);

    /* Publish */
    PublishStickyEvent(EVENT, TYPE, CODE, DATA, PERMISSION);
    PublishStickyEvent(EVENT2, TYPE2, CODE2, DATA2, PERMISSION2);

    usleep(50000);

    std::vector<std::string> state;
    GetInnerCommonEventManager()->DumpState(static_cast<int32_t>(DumpEventType::STICKY), "", ALL_USER, state);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    ASSERT_EQ(state.size(), DUMP_STICKY_COUNT_TWO);
    auto pos = state[0].find("Sticky Events:");
    ASSERT_NE(pos, string::npos);

    GetInnerCommonEventManager()->UnsubscribeCommonEvent(listener->AsObject());
    GetInnerCommonEventManager()->UnsubscribeCommonEvent(listener2->AsObject());
}

/*
 * @tc.number: CommonEventDumpPartEventTest_0300
 * @tc.name: test dump
 * @tc.desc: Verify dump pending events
 * @tc.require: I582WG
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpPartEventTest_0300, Function | MediumTest | Level1)
{
    /* Subscribe */
    CommonEventListener *listener = nullptr;
    CommonEventListener *listener2 = nullptr;
    SubscribeDoubleEvent(PRIORITY, PERMISSION, DEVICEDID, listener);
    SubscribeDoubleEvent(PRIORITY2, PERMISSION2, DEVICEDID2, listener2);
    EXPECT_NE(nullptr, listener);
    EXPECT_NE(nullptr, listener2);

    /* Publish */
    PublishUnorderedEvent(EVENT, TYPE, CODE, DATA, PERMISSION);
    PublishUnorderedEvent(EVENT2, TYPE2, CODE2, DATA2, PERMISSION2);

    usleep(50000);

    std::vector<std::string> state;
    GetInnerCommonEventManager()->DumpState(static_cast<int32_t>(DumpEventType::PENDING), "", ALL_USER, state);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    ASSERT_EQ(state.size(), 1);
    EXPECT_EQ("Pending Events:\tNo information", state[0]);

    GetInnerCommonEventManager()->UnsubscribeCommonEvent(listener->AsObject());
    GetInnerCommonEventManager()->UnsubscribeCommonEvent(listener2->AsObject());
}

/*
 * @tc.number: CommonEventDumpPartEventTest_0400
 * @tc.name: test dump
 * @tc.desc: Verify dump information after publishing unordered events
 * @tc.require: I582WG
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpPartEventTest_0400, Function | MediumTest | Level1)
{
    /* Subscribe */
    CommonEventListener *listener = nullptr;
    CommonEventListener *listener2 = nullptr;
    SubscribeDoubleEvent(PRIORITY, PERMISSION, DEVICEDID, listener);
    SubscribeDoubleEvent(PRIORITY2, PERMISSION2, DEVICEDID2, listener2);
    EXPECT_NE(nullptr, listener);
    EXPECT_NE(nullptr, listener2);

    /* Publish */
    PublishUnorderedEvent(EVENT, TYPE, CODE, DATA, PERMISSION);
    PublishUnorderedEvent(EVENT2, TYPE2, CODE2, DATA2, PERMISSION2);

    usleep(50000);

    std::vector<std::string> state;
    GetInnerCommonEventManager()->DumpState(static_cast<int32_t>(DumpEventType::HISTORY), "", ALL_USER, state);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    ASSERT_EQ(state.size(), DUMP_HISTORY_COUNT_TWO * 3);
    auto pos = state[0].find("History Events:");
    ASSERT_NE(pos, string::npos);

    GetInnerCommonEventManager()->UnsubscribeCommonEvent(listener->AsObject());
    GetInnerCommonEventManager()->UnsubscribeCommonEvent(listener2->AsObject());
}
}  // namespace
