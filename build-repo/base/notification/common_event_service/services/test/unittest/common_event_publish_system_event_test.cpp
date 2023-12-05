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

#include "common_event_listener.h"
#include "common_event_subscriber.h"
#include "common_event_support.h"
#include "inner_common_event_manager.h"
#include "mock_bundle_manager.h"

using namespace testing::ext;
using namespace OHOS::EventFwk;
using namespace OHOS::AppExecFwk;

namespace {
constexpr uint8_t PID = 0;
constexpr uint16_t SYSTEM_APP_UID = 1000;  // system app
constexpr uint32_t NON_SYSTEM_APP_UID = 20010099;  // third party app
constexpr uint8_t PUBLISH_SLEEP = 1;
}  // namespace

static OHOS::sptr<OHOS::IRemoteObject> bundleObject = nullptr;

class CommonEventPublishSystemEventTest : public testing::Test {
public:
    CommonEventPublishSystemEventTest()
    {}
    ~CommonEventPublishSystemEventTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

public:
    InnerCommonEventManager innerCommonEventManager;
};

void CommonEventPublishSystemEventTest::SetUpTestCase(void)
{
    bundleObject = new MockBundleMgrService();
    OHOS::DelayedSingleton<BundleManagerHelper>::GetInstance()->sptrBundleMgr_ =
        OHOS::iface_cast<OHOS::AppExecFwk::IBundleMgr>(bundleObject);
}

void CommonEventPublishSystemEventTest::TearDownTestCase(void)
{}

void CommonEventPublishSystemEventTest::SetUp(void)
{}

void CommonEventPublishSystemEventTest::TearDown(void)
{}

class SubscriberTest : public CommonEventSubscriber {
public:
    explicit SubscriberTest(const CommonEventSubscribeInfo &sp) : CommonEventSubscriber(sp)
    {}

    ~SubscriberTest()
    {}

    virtual void OnReceiveEvent(const CommonEventData &data)
    {
        GTEST_LOG_(INFO) << "CESPublishOrderedEventSystmTest::Subscriber OnReceiveEvent ";
        std::string action = data.GetWant().GetAction();
        std::string event = GetSubscribeInfo().GetMatchingSkills().GetEvent(0);
        EXPECT_EQ(action, event);
    }
};

/*
 * @tc.number: CommonEventPublishSystemEventTest_0100
 * @tc.name: test PublishCommonEvent
 * @tc.desc: Verify Publish System CommonEvent success
 */
HWTEST_F(CommonEventPublishSystemEventTest, CommonEventPublishSystemEventTest_0100, Function | MediumTest | Level1)
{
    /* Publish */
    OHOS::Security::AccessToken::AccessTokenID tokenID = 0;

    // make a want
    Want want;
    want.SetAction(CommonEventSupport::COMMON_EVENT_LOCKED_BOOT_COMPLETED);

    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(false);

    struct tm curTime {0};
    // publish system event
    bool publishResult = innerCommonEventManager.PublishCommonEvent(
        data, publishInfo, nullptr, curTime, PID, SYSTEM_APP_UID, tokenID, UNDEFINED_USER, "bundlename");
    sleep(PUBLISH_SLEEP);
    EXPECT_TRUE(publishResult);
}

/*
 * @tc.number: CommonEventPublishSystemEventTest_0200
 * @tc.name: test PublishCommonEvent
 * @tc.desc: Verify Publish System CommonEvent fail because is not systemapp
 */
HWTEST_F(CommonEventPublishSystemEventTest, CommonEventPublishSystemEventTest_0200, Function | MediumTest | Level1)
{
    /* Publish */
    OHOS::Security::AccessToken::AccessTokenID tokenID = 1;

    // make a want
    Want want;
    want.SetAction(CommonEventSupport::COMMON_EVENT_LOCKED_BOOT_COMPLETED);

    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(false);

    struct tm curTime {0};
    // publish system event
    bool publishResult = innerCommonEventManager.PublishCommonEvent(
        data, publishInfo, nullptr, curTime, PID, NON_SYSTEM_APP_UID, tokenID, UNDEFINED_USER, "bundlename");
    sleep(PUBLISH_SLEEP);
    EXPECT_FALSE(publishResult);
}

/*
 * @tc.number: CommonEventPublishSystemEventTest_0300
 * @tc.name: test PublishCommonEvent
 * @tc.desc: Verify Publish mapped System CommonEvent
 */
HWTEST_F(CommonEventPublishSystemEventTest, CommonEventPublishSystemEventTest_0300, Function | MediumTest | Level1)
{
    /* Subscribe */
    OHOS::Security::AccessToken::AccessTokenID tokenID = 0;

    // make subscriber info
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_TEST_ACTION1);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    // make subscriber
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);

    // make common event listener
    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);
    OHOS::sptr<OHOS::IRemoteObject> commonEventListenerPtr(commonEventListener);

    // SubscribeCommonEvent
    struct tm curTime{0};
    InnerCommonEventManager innerCommonEventManager;
    EXPECT_TRUE(innerCommonEventManager.SubscribeCommonEvent(
        subscribeInfo, commonEventListenerPtr, curTime, PID, SYSTEM_APP_UID, tokenID, "bundlename"));

    /* Publish */

    // make a want
    Want want;
    want.SetAction(CommonEventSupport::COMMON_EVENT_TEST_ACTION2);

    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(false);

    // publish system event
    bool publishResult = innerCommonEventManager.PublishCommonEvent(
        data, publishInfo, nullptr, curTime, PID, SYSTEM_APP_UID, tokenID, UNDEFINED_USER, "bundlename");
    sleep(PUBLISH_SLEEP);
    EXPECT_TRUE(publishResult);

    innerCommonEventManager.UnsubscribeCommonEvent(commonEventListenerPtr);
}

/*
 * @tc.number: CommonEventPublishSystemEventTest_0400
 * @tc.name: test PublishCommonEvent
 * @tc.desc: Verify Publish mapped System CommonEvent
 */
HWTEST_F(CommonEventPublishSystemEventTest, CommonEventPublishSystemEventTest_0400, Function | MediumTest | Level1)
{
    /* Subscribe */
    OHOS::Security::AccessToken::AccessTokenID tokenID = 0;

    // make subscriber info
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_TEST_ACTION2);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    // make subscriber
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);

    // make common event listener
    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);
    OHOS::sptr<OHOS::IRemoteObject> commonEventListenerPtr(commonEventListener);

    // SubscribeCommonEvent
    struct tm curTime{0};
    InnerCommonEventManager innerCommonEventManager;
    EXPECT_TRUE(innerCommonEventManager.SubscribeCommonEvent(
        subscribeInfo, commonEventListenerPtr, curTime, PID, SYSTEM_APP_UID, tokenID, "bundlename"));

    /* Publish */

    // make a want
    Want want;
    want.SetAction(CommonEventSupport::COMMON_EVENT_TEST_ACTION1);

    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(false);

    // publish system event
    bool publishResult = innerCommonEventManager.PublishCommonEvent(
        data, publishInfo, nullptr, curTime, PID, SYSTEM_APP_UID, tokenID, UNDEFINED_USER, "bundlename");
    sleep(PUBLISH_SLEEP);
    EXPECT_TRUE(publishResult);

    innerCommonEventManager.UnsubscribeCommonEvent(commonEventListenerPtr);
}
