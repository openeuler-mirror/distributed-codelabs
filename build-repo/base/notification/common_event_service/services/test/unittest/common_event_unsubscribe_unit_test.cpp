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
const std::string EVENT = "com.ces.test.event";
const std::string ENTITY = "com.ces.test.entity";
const std::string SCHEME = "com.ces.test.scheme";
const std::string PERMISSION = "com.ces.test.permission";
const std::string DEVICEDID = "deviceId";
const int PRIORITY = 1;
}  // namespace

static OHOS::sptr<OHOS::IRemoteObject> bundleObject = nullptr;

class CommonEventUnSubscribeUnitTest : public testing::Test {
public:
    CommonEventUnSubscribeUnitTest()
    {}
    ~CommonEventUnSubscribeUnitTest()
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
    {}
};

void CommonEventUnSubscribeUnitTest::SetUpTestCase(void)
{
    bundleObject = new MockBundleMgrService();
    OHOS::DelayedSingleton<BundleManagerHelper>::GetInstance()->sptrBundleMgr_ =
        OHOS::iface_cast<OHOS::AppExecFwk::IBundleMgr>(bundleObject);
}

void CommonEventUnSubscribeUnitTest::TearDownTestCase(void)
{}

void CommonEventUnSubscribeUnitTest::SetUp(void)
{}

void CommonEventUnSubscribeUnitTest::TearDown(void)
{}

/*
 * @tc.number: CommonEventUnSubscribeUnitTest_0300
 * @tc.name: exception
 * @tc.desc: verify InnerCommonEventManager UnsubscribeCommonEvent function success
 */
HWTEST_F(CommonEventUnSubscribeUnitTest, CommonEventUnSubscribeUnitTest_0300, Function | MediumTest | Level1)
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

    // UnsubscribeCommonEvent
    InnerCommonEventManager innerCommonEventManager;
    EXPECT_TRUE(innerCommonEventManager.UnsubscribeCommonEvent(commonEventListenerPtr));
}

/*
 * @tc.number: CommonEventUnSubscribeUnitTest_0400
 * @tc.name: exception
 * @tc.desc: When the CommonEventListener does not exist,
 *                      verify InnerCommonEventManager UnsubscribeCommonEvent function return value.
 */
HWTEST_F(CommonEventUnSubscribeUnitTest, CommonEventUnSubscribeUnitTest_0400, Function | MediumTest | Level1)
{
    std::shared_ptr<InnerCommonEventManager> innerCommonEventManager = std::make_shared<InnerCommonEventManager>();
    OHOS::sptr<OHOS::IRemoteObject> sp(nullptr);
    EXPECT_FALSE(innerCommonEventManager->UnsubscribeCommonEvent(sp));
}

/*
 * @tc.number: CommonEventUnSubscribeUnitTest_0500
 * @tc.name: exception
 * @tc.desc: verify CommonEventSubscriberManager RemoveSubscriber success
 */
HWTEST_F(CommonEventUnSubscribeUnitTest, CommonEventUnSubscribeUnitTest_0500, Function | MediumTest | Level1)
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

    // RemoveSubscriber
    CommonEventSubscriberManager commonEventSubscriberManager;
    int result = commonEventSubscriberManager.RemoveSubscriber(commonEventListenerPtr);
    EXPECT_EQ(OHOS::ERR_OK, result);
}

/*
 * @tc.number: CommonEventUnSubscribeUnitTest_0600
 * @tc.name: exception
 * @tc.desc: When the CommonEventListener does not exist,
 *                       verify RemoveSubscriber fail.
 */
HWTEST_F(CommonEventUnSubscribeUnitTest, CommonEventUnSubscribeUnitTest_0600, Function | MediumTest | Level1)
{
    CommonEventSubscriberManager commonEventSubscriberManager;
    EXPECT_EQ(OHOS::ERR_INVALID_VALUE, commonEventSubscriberManager.RemoveSubscriber(nullptr));
}

/*
 * @tc.number: CommonEventUnSubscribeUnitTest_0700
 * @tc.name: exception
 * @tc.desc: When the death_ is not null, verify RemoveSubscriber fail.
 */
HWTEST_F(CommonEventUnSubscribeUnitTest, CommonEventUnSubscribeUnitTest_0700, Function | MediumTest | Level1)
{
    // make matching skills
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);
    matchingSkills.AddEntity(ENTITY);
    matchingSkills.AddScheme(SCHEME);

    // make subscriber info
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(PRIORITY);
    subscribeInfo.SetPermission(PERMISSION);
    subscribeInfo.SetDeviceId(DEVICEDID);

    // make a subscriber
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);

    // make common event listener
    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);

    // RemoveSubscriber
    CommonEventSubscriberManager commonEventSubscriberManager;
    commonEventSubscriberManager.death_ = nullptr;
    EXPECT_EQ(OHOS::ERR_OK, commonEventSubscriberManager.RemoveSubscriber(commonEventListener));
}

/*
 * @tc.number: CommonEventUnSubscribeUnitTest_0800
 * @tc.name: exception
 * @tc.desc: verify RemoveSubscriberRecordLocked success.
 */
HWTEST_F(CommonEventUnSubscribeUnitTest, CommonEventUnSubscribeUnitTest_0800, Function | MediumTest | Level1)
{
    // make subscriber info
    MatchingSkills matchingSkills;
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    // make subscriber
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);

    // make common event listener
    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);
    OHOS::sptr<OHOS::IRemoteObject> commonEventListenerPtr(commonEventListener);

    // RemoveSubscriber
    CommonEventSubscriberManager commonEventSubscriberManager;
    EXPECT_EQ(OHOS::ERR_OK, commonEventSubscriberManager.RemoveSubscriberRecordLocked(commonEventListenerPtr));
}

/*
 * @tc.number: CommonEventUnSubscribeUnitTest_0900
 * @tc.name: exception
 * @tc.desc: When the commonEventListener is null, verify RemoveSubscriberRecordLocked success.
 */
HWTEST_F(CommonEventUnSubscribeUnitTest, CommonEventUnSubscribeUnitTest_0900, Function | MediumTest | Level1)
{
    // make subscriber info
    MatchingSkills matchingSkills;
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    // make subscriber
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);

    // make common event listener
    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);
    OHOS::sptr<OHOS::IRemoteObject> commonEventListenerPtr(commonEventListener);

    // RemoveSubscriber
    CommonEventSubscriberManager commonEventSubscriberManager;
    EXPECT_EQ(OHOS::ERR_INVALID_VALUE, commonEventSubscriberManager.RemoveSubscriberRecordLocked(nullptr));
}
