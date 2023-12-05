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
#include "common_event_subscriber_manager.h"
#include "static_subscriber_manager.h"
#include "ability_manager_helper.h"
#undef private
#undef protected

#include "common_event_listener.h"
#include "common_event_stub.h"
#include "common_event_subscriber.h"
#include "inner_common_event_manager.h"
#include "mock_bundle_manager.h"
#include "bundle_manager_helper.h"
#include "common_event_manager_service.h"

using namespace testing::ext;
using namespace OHOS::EventFwk;
using namespace OHOS::AppExecFwk;

namespace {
const std::string EVENT = "com.ces.test.event";
const std::string ENTITY = "com.ces.test.entity";
const std::string SCHEME = "com.ces.test.scheme";
const std::string PERMISSION = "com.ces.test.permission";
const std::string DEVICEDID = "deviceId";
constexpr int32_t ERR_COMMON = -1;
}  // namespace

static OHOS::sptr<OHOS::IRemoteObject> bundleObject = nullptr;
class CommonEventSubscribeUnitTest : public testing::Test {
public:
    CommonEventSubscribeUnitTest()
    {}
    ~CommonEventSubscribeUnitTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

class SubscriberTest : public CommonEventSubscriber {
public:
    SubscriberTest(const CommonEventSubscribeInfo &sp) : CommonEventSubscriber(sp)
    {}

    ~SubscriberTest()
    {}

    virtual void OnReceiveEvent(const CommonEventData &data)
    {}
};

class CommonEventStubTest : public CommonEventStub {
public:
    CommonEventStubTest()
    {}

    virtual int32_t PublishCommonEvent(const CommonEventData &event, const CommonEventPublishInfo &publishinfo,
        const OHOS::sptr<OHOS::IRemoteObject> &commonEventListener, const int32_t &userId)
    {
        return ERR_COMMON;
    }

    virtual int32_t SubscribeCommonEvent(
        const CommonEventSubscribeInfo &subscribeInfo, const OHOS::sptr<OHOS::IRemoteObject> &commonEventListener)
    {
        return ERR_COMMON;
    }

    virtual int32_t UnsubscribeCommonEvent(const OHOS::sptr<OHOS::IRemoteObject> &commonEventListener)
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

    virtual bool FinishReceiver(const OHOS::sptr<OHOS::IRemoteObject> &proxy, const int &code,
        const std::string &receiverData, const bool &abortEvent)
    {
        return false;
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

void CommonEventSubscribeUnitTest::SetUpTestCase(void)
{
    bundleObject = new MockBundleMgrService();
    OHOS::DelayedSingleton<BundleManagerHelper>::GetInstance()->sptrBundleMgr_ =
        OHOS::iface_cast<OHOS::AppExecFwk::IBundleMgr>(bundleObject);
}

void CommonEventSubscribeUnitTest::TearDownTestCase(void)
{}

void CommonEventSubscribeUnitTest::SetUp(void)
{}

void CommonEventSubscribeUnitTest::TearDown(void)
{}

/*
 * @tc.number: CommonEventSubscribeUnitTest_0300
 * @tc.name: test subscribe event
 * @tc.desc: InnerCommonEventManager subscribe common event success
 */
HWTEST_F(CommonEventSubscribeUnitTest, CommonEventSubscribeUnitTest_0300, Function | MediumTest | Level1)
{
    // make subscriber info
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    // make subscriber
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);

    // make common event listener
    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);
    OHOS::sptr<OHOS::IRemoteObject> commonEventListenerPtr(commonEventListener);

    // SubscribeCommonEvent
    struct tm curTime {0};
    OHOS::Security::AccessToken::AccessTokenID tokenID = 0;
    InnerCommonEventManager innerCommonEventManager;
    EXPECT_TRUE(innerCommonEventManager.SubscribeCommonEvent(
        subscribeInfo, commonEventListenerPtr, curTime, 0, 0, tokenID, ""));
}

/*
 * @tc.number: CommonEventSubscribeUnitTest_0400
 * @tc.name: test subscribe event
 * @tc.desc: InnerCommonEventManager subscribe common event fail because subscribeInfo has not event
 */
HWTEST_F(CommonEventSubscribeUnitTest, CommonEventSubscribeUnitTest_0400, Function | MediumTest | Level1)
{
    // make subscriber info
    MatchingSkills matchingSkills;
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    // make subscriber
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);

    // make common event listener
    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);
    OHOS::sptr<OHOS::IRemoteObject> commonEventListenerPtr(commonEventListener);

    // SubscribeCommonEvent
    struct tm curTime {0};
    OHOS::Security::AccessToken::AccessTokenID tokenID = 0;
    InnerCommonEventManager innerCommonEventManager;
    EXPECT_FALSE(innerCommonEventManager.SubscribeCommonEvent(
        subscribeInfo, commonEventListenerPtr, curTime, 0, 0, tokenID, ""));
}

/*
 * @tc.number: CommonEventSubscribeUnitTest_0500
 * @tc.name: test subscribe event
 * @tc.desc: 1. subscribe common event
 *                      2. fail subscribe common event, inner common event manager
 *                          common event listener is null
 */
HWTEST_F(CommonEventSubscribeUnitTest, CommonEventSubscribeUnitTest_0500, Function | MediumTest | Level1)
{
    // make subscriber info
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    // SubscribeCommonEvent
    OHOS::sptr<OHOS::IRemoteObject> sp(nullptr);
    struct tm curTime {0};
    OHOS::Security::AccessToken::AccessTokenID tokenID = 0;
    InnerCommonEventManager innerCommonEventManager;
    EXPECT_FALSE(innerCommonEventManager.SubscribeCommonEvent(subscribeInfo, sp, curTime, 0, 0, tokenID, ""));
}

/*
 * @tc.number: CommonEventSubscribeUnitTest_0600
 * @tc.name: test subscribe event
 * @tc.desc: subscribe common event success
 */
HWTEST_F(CommonEventSubscribeUnitTest, CommonEventSubscribeUnitTest_0600, Function | MediumTest | Level1)
{
    // make subscriber info
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    std::shared_ptr<CommonEventSubscribeInfo> subscribeInfoPtr =
        std::make_shared<CommonEventSubscribeInfo>(subscribeInfo);

    // make subscriber
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);

    // make common event listener
    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);
    OHOS::sptr<OHOS::IRemoteObject> commonEventListenerPtr(commonEventListener);

    // InsertSubscriber
    struct tm curTime {0};
    EventRecordInfo eventRecordInfo;
    eventRecordInfo.pid = 0;
    eventRecordInfo.uid = 0;
    eventRecordInfo.bundleName = "bundleName";
    CommonEventSubscriberManager commonEventSubscriberManager;
    auto result = commonEventSubscriberManager.InsertSubscriber(
        subscribeInfoPtr, commonEventListener, curTime, eventRecordInfo);
    EXPECT_NE(nullptr, result);
}

/*
 * @tc.number: CommonEventSubscribeUnitTest_0700
 * @tc.name: test subscribe event
 * @tc.desc: 1. subscribe common event
 *                      2. fail subscribe common event , common event subscriber manager
 *                          event subscriber info is null
 */
HWTEST_F(CommonEventSubscribeUnitTest, CommonEventSubscribeUnitTest_0700, Function | MediumTest | Level1)
{
    // make subscriber info
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    // make subscriber
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);

    // make common event listener
    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);
    OHOS::sptr<OHOS::IRemoteObject> commonEventListenerPtr(commonEventListener);

    // InsertSubscriber
    struct tm curTime {0};
    EventRecordInfo eventRecordInfo;
    eventRecordInfo.pid = 0;
    eventRecordInfo.uid = 0;
    eventRecordInfo.bundleName = "bundleName";
    CommonEventSubscriberManager commonEventSubscriberManager;
    auto result =
        commonEventSubscriberManager.InsertSubscriber(nullptr, commonEventListenerPtr, curTime, eventRecordInfo);
    EXPECT_EQ(nullptr, result);
}

/*
 * @tc.number: CommonEventSubscribeUnitTest_0800
 * @tc.name: test subscribe event
 * @tc.desc: 1. subscribe common event
 *                      2. fail subscribe common event , common event subscriber manager
 *                          event common event listener is null
 */
HWTEST_F(CommonEventSubscribeUnitTest, CommonEventSubscribeUnitTest_0800, Function | MediumTest | Level1)
{
    // make subscriber info
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    std::shared_ptr<CommonEventSubscribeInfo> subscribeInfoPtr =
        std::make_shared<CommonEventSubscribeInfo>(subscribeInfo);

    // make subscriber
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);

    // InsertSubscriber
    struct tm curTime {0};
    EventRecordInfo eventRecordInfo;
    eventRecordInfo.pid = 0;
    eventRecordInfo.uid = 0;
    eventRecordInfo.bundleName = "bundleName";
    CommonEventSubscriberManager commonEventSubscriberManager;
    auto result = commonEventSubscriberManager.InsertSubscriber(subscribeInfoPtr, nullptr, curTime, eventRecordInfo);
    EXPECT_EQ(nullptr, result);
}

/*
 * @tc.number: CommonEventSubscribeUnitTest_0900
 * @tc.name: test subscribe event
 * @tc.desc: 1. subscribe common event
 *                      2. fail subscribe common event , common event subscriber manager
 *                          event size is null
 */
HWTEST_F(CommonEventSubscribeUnitTest, CommonEventSubscribeUnitTest_0900, Function | MediumTest | Level1)
{
    // make subscriber info
    MatchingSkills matchingSkills;
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    std::shared_ptr<CommonEventSubscribeInfo> subscribeInfoPtr =
        std::make_shared<CommonEventSubscribeInfo>(subscribeInfo);

    // make subscriber
    CommonEventStubTest CommonEventStubTest;
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);

    // make common event listener
    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);
    std::shared_ptr<InnerCommonEventManager> innerCommonEventManager = std::make_shared<InnerCommonEventManager>();
    OHOS::sptr<OHOS::IRemoteObject> commonEventListenerPtr(commonEventListener);

    // InsertSubscriber
    struct tm curTime {0};
    EventRecordInfo eventRecordInfo;
    eventRecordInfo.pid = 0;
    eventRecordInfo.uid = 0;
    eventRecordInfo.bundleName = "bundleName";
    CommonEventSubscriberManager commonEventSubscriberManager;
    auto result = commonEventSubscriberManager.InsertSubscriber(
        subscribeInfoPtr, commonEventListenerPtr, curTime, eventRecordInfo);
    EXPECT_EQ(nullptr, result);
}

/*
 * @tc.number: CommonEventSubscribeUnitTest_1000
 * @tc.name: ttest subscribe event
 * @tc.desc:  insert subscriber record locked success
 */
HWTEST_F(CommonEventSubscribeUnitTest, CommonEventSubscribeUnitTest_1000, Function | MediumTest | Level1)
{
    // make subscriber info
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);
    matchingSkills.AddEvent(ENTITY);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    // make subscriber
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);

    // make common event listener
    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);
    OHOS::sptr<OHOS::IRemoteObject> commonEventListenerPtr(commonEventListener);

    // make events
    std::vector<std::string> events = subscribeInfo.GetMatchingSkills().GetEvents();

    // make record
    struct tm recordTime {0};
    auto record = std::make_shared<EventSubscriberRecord>();
    record->eventSubscribeInfo = std::make_shared<CommonEventSubscribeInfo>(subscribeInfo);
    record->commonEventListener = commonEventListenerPtr;
    record->recordTime = recordTime;
    record->eventRecordInfo.pid = 0;
    record->eventRecordInfo.uid = 0;
    record->eventRecordInfo.bundleName = "bundleName";

    // InsertSubscriberRecordLocked
    CommonEventSubscriberManager commonEventSubscriberManager;
    EXPECT_TRUE(commonEventSubscriberManager.InsertSubscriberRecordLocked(events, record));
}

/*
 * @tc.number: CommonEventSubscribeUnitTest_1100
 * @tc.name: test subscribe event
 * @tc.desc:  insert subscriber record locked fail because events size is 0
 */
HWTEST_F(CommonEventSubscribeUnitTest, CommonEventSubscribeUnitTest_1100, Function | MediumTest | Level1)
{
    // make subscriber info
    MatchingSkills matchingSkills;
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    // make subscriber
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);

    // make common event listener
    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);
    OHOS::sptr<OHOS::IRemoteObject> commonEventListenerPtr(commonEventListener);

    // make events
    std::vector<std::string> events = subscribeInfo.GetMatchingSkills().GetEvents();

    // make record
    struct tm recordTime {0};
    auto record = std::make_shared<EventSubscriberRecord>();
    record->eventSubscribeInfo = std::make_shared<CommonEventSubscribeInfo>(subscribeInfo);
    record->commonEventListener = commonEventListenerPtr;
    record->recordTime = recordTime;
    record->eventRecordInfo.pid = 0;
    record->eventRecordInfo.uid = 0;
    record->eventRecordInfo.bundleName = "bundleName";

    // InsertSubscriberRecordLocked
    CommonEventSubscriberManager commonEventSubscriberManager;
    EXPECT_FALSE(commonEventSubscriberManager.InsertSubscriberRecordLocked(events, record));
}

/*
 * @tc.number: CommonEventSubscribeUnitTest_1200
 * @tc.name: test subscribe event
 * @tc.desc:  insert subscriber record locked fail because record is nullptr
 */
HWTEST_F(CommonEventSubscribeUnitTest, CommonEventSubscribeUnitTest_1200, Function | MediumTest | Level1)
{
    // make subscriber info
    MatchingSkills matchingSkills;
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    // make events
    std::vector<std::string> events = subscribeInfo.GetMatchingSkills().GetEvents();
    CommonEventSubscriberManager commonEventSubscriberManager;

    // InsertSubscriberRecordLocked
    EXPECT_FALSE(commonEventSubscriberManager.InsertSubscriberRecordLocked(events, nullptr));
}

/**
 * @tc.name: AbilityManagerHelper_0001
 * @tc.desc: GetAbilityMgrProxy
 * @tc.type: FUNC
 * @tc.require: I5R11Y
 */
HWTEST_F(CommonEventSubscribeUnitTest, AbilityManagerHelper_0001, Function | MediumTest | Level1)
{
    AbilityManagerHelper abilityManagerHelper;
    bool result = abilityManagerHelper.GetAbilityMgrProxy();
    abilityManagerHelper.Clear();
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: AbilityManagerHelper_0002
 * @tc.desc: ConnectAbility
 * @tc.type: FUNC
 * @tc.require: I5R11Y
 */
HWTEST_F(CommonEventSubscribeUnitTest, AbilityManagerHelper_0002, Function | MediumTest | Level1)
{
    // make a want
    Want want;
    want.SetAction(EVENT);

    // make common event data
    CommonEventData event;
    OHOS::sptr<OHOS::IRemoteObject> callerToken(nullptr);
    int32_t userId = 1;
    AbilityManagerHelper abilityManagerHelper;
    int result = abilityManagerHelper.ConnectAbility(want, event, callerToken, userId);
    EXPECT_EQ(result, 2097177);
}

/**
 * @tc.name: BundleManagerHelper_0001
 * @tc.desc: QueryExtensionInfos
 * @tc.type: FUNC
 * @tc.require: I5R11Y
 */
HWTEST_F(CommonEventSubscribeUnitTest, BundleManagerHelper_0001, Function | MediumTest | Level1)
{
    std::vector<OHOS::AppExecFwk::ExtensionAbilityInfo> extensionInfos;
    int32_t userId = 3;
    BundleManagerHelper bundleManagerHelper;
    bool result = bundleManagerHelper.QueryExtensionInfos(extensionInfos, userId);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: BundleManagerHelper_0002
 * @tc.desc: GetResConfigFile
 * @tc.type: FUNC
 * @tc.require: I5R11Y
 */
HWTEST_F(CommonEventSubscribeUnitTest, BundleManagerHelper_0002, Function | MediumTest | Level1)
{
    OHOS::AppExecFwk::ExtensionAbilityInfo extension;
    std::vector<std::string> profileInfos;
    BundleManagerHelper bundleManagerHelper;
    bool result = bundleManagerHelper.GetResConfigFile(extension, profileInfos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: CommonEventManagerService_0001
 * @tc.desc: PublishCommonEvent
 * @tc.type: FUNC
 * @tc.require: I5R11Y
 */
HWTEST_F(CommonEventSubscribeUnitTest, CommonEventManagerService_0001, Function | MediumTest | Level1)
{
    // make a want
    Want want;
    want.SetAction(EVENT);
    // make common event data
    CommonEventData event;
    // make publish info
    CommonEventPublishInfo publishinfo;
    OHOS::sptr<OHOS::IRemoteObject> commonEventListener(nullptr);
    uid_t uid = 1;
    int32_t userId = 1;
    int32_t tokenId = 0;
    CommonEventManagerService commonEventManagerService;
    bool result = commonEventManagerService.PublishCommonEvent(event, publishinfo, commonEventListener, uid,
        tokenId, userId);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: CommonEventManagerService_0002
 * @tc.desc: DumpState
 * @tc.type: FUNC
 * @tc.require: I5R11Y
 */
HWTEST_F(CommonEventSubscribeUnitTest, CommonEventManagerService_0002, Function | MediumTest | Level1)
{
    uint8_t dumpType = 1;
    std::string event = "Event";
    int32_t userId =2;
    std::vector<std::string> state;
    CommonEventManagerService commonEventManagerService;
    bool result = commonEventManagerService.DumpState(dumpType, event, userId, state);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: CommonEventManagerService_0003
 * @tc.desc: FinishReceiver
 * @tc.type: FUNC
 * @tc.require: I5R11Y
 */
HWTEST_F(CommonEventSubscribeUnitTest, CommonEventManagerService_0003, Function | MediumTest | Level1)
{
    OHOS::sptr<OHOS::IRemoteObject> proxy(nullptr);
    int32_t code = 0;
    std::string receiverData = "receiverData";
    CommonEventManagerService commonEventManagerService;
    bool result = commonEventManagerService.FinishReceiver(proxy, code, receiverData, false);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: CommonEventManagerService_0004
 * @tc.desc: Dump
 * @tc.type: FUNC
 * @tc.require: I5R11Y
 */
HWTEST_F(CommonEventSubscribeUnitTest, CommonEventManagerService_0004, Function | MediumTest | Level1)
{
    int fd = 1;
    std::vector<std::u16string> args;
    CommonEventManagerService commonEventManagerService;
    int result = commonEventManagerService.Dump(fd, args);
    EXPECT_EQ(result, int(OHOS::ERR_OK));
}