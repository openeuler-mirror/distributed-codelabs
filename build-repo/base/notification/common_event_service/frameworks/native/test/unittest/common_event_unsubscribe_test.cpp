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

// redefine private and protected since testcase need to invoke and test private function
#define private public
#define protected public
#include "common_event.h"
#include "common_event_manager.h"
#include "common_event_stub.h"
#include "common_event_subscriber_manager.h"
#include "inner_common_event_manager.h"
#undef private
#undef protected

#include <gtest/gtest.h>

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::EventFwk;
using OHOS::Parcel;

namespace {
constexpr int32_t ERR_COMMON = -1;
}

class CommonEventUnSubscribeTest : public testing::Test {
public:
    CommonEventUnSubscribeTest()
    {}
    ~CommonEventUnSubscribeTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

public:
    MatchingSkills matchingSkills_;
    void SetMatchingSkillsWithEvent(const std::string &event);
    void SetMatchingSkillsWithEntity(const std::string &entity);
    void SetMatchingSkillsWithScheme(const std::string &scheme);
};

class DreivedSubscriber : public CommonEventSubscriber {
public:
    explicit DreivedSubscriber(const CommonEventSubscribeInfo &sp) : CommonEventSubscriber(sp)
    {}

    ~DreivedSubscriber()
    {}

    virtual void OnReceiveEvent(const CommonEventData &data)
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

class SubscriberTest : public CommonEventSubscriber {
public:
    explicit SubscriberTest(const CommonEventSubscribeInfo &sp) : CommonEventSubscriber(sp)
    {}

    ~SubscriberTest()
    {}

    virtual void OnReceiveEvent(const CommonEventData &data)
    {}
};

void CommonEventUnSubscribeTest::SetUpTestCase(void)
{}

void CommonEventUnSubscribeTest::TearDownTestCase(void)
{}

void CommonEventUnSubscribeTest::SetUp(void)
{}

void CommonEventUnSubscribeTest::TearDown(void)
{}

void CommonEventUnSubscribeTest::SetMatchingSkillsWithEvent(const std::string &event)
{
    matchingSkills_.AddEvent(event);
}

void CommonEventUnSubscribeTest::SetMatchingSkillsWithEntity(const std::string &entity)
{
    matchingSkills_.AddEntity(entity);
}

void CommonEventUnSubscribeTest::SetMatchingSkillsWithScheme(const std::string &scheme)
{
    matchingSkills_.AddScheme(scheme);
}

/*
 * Feature: CommonEventManager
 * Function: UnSubscribeCommonEvent
 * SubFunction: NA
 * FunctionPoints: normal
 * EnvConditions: system running normally
 * CaseDescription: Unsubscribe normally and verify UnSubscribeCommonEvent function return value.
 */
HWTEST_F(CommonEventUnSubscribeTest, UnSubscribe_001, TestSize.Level1)
{
    CommonEventUnSubscribeTest::SetMatchingSkillsWithEvent("event");
    CommonEventSubscribeInfo subscribeInfo(matchingSkills_);
    std::shared_ptr<DreivedSubscriber> subscriber = std::make_shared<DreivedSubscriber>(subscribeInfo);
    std::shared_ptr<CommonEventManager> helper;
    EXPECT_EQ(true, helper->UnSubscribeCommonEvent(subscriber));
}

/*
 * Feature: CommonEvent
 * Function: UnSubscribeCommonEvent
 * SubFunction: NA
 * FunctionPoints: normal
 * EnvConditions: system running normally
 * CaseDescription: Verify UnSubscribeCommonEvent function return value with eventListener not exist.
 */
HWTEST_F(CommonEventUnSubscribeTest, UnSubscribe_002, TestSize.Level1)
{
    CommonEventUnSubscribeTest::SetMatchingSkillsWithEvent("event");
    CommonEventSubscribeInfo subscribeInfo(matchingSkills_);
    std::shared_ptr<DreivedSubscriber> subscriber = std::make_shared<DreivedSubscriber>(subscribeInfo);
    EXPECT_EQ(ERR_OK, DelayedSingleton<CommonEvent>::GetInstance()->UnSubscribeCommonEvent(subscriber));
}

/*
 * Feature: CommonEvent
 * Function: UnSubscribeCommonEvent
 * SubFunction: NA
 * FunctionPoints: normal
 * EnvConditions: system running normally
 * CaseDescription: Verify UnSubscribeCommonEvent function return value with eventListener existed.
 */
HWTEST_F(CommonEventUnSubscribeTest, UnSubscribe_003, TestSize.Level1)
{
    CommonEventUnSubscribeTest::SetMatchingSkillsWithEvent("event");
    CommonEventSubscribeInfo subscribeInfo(matchingSkills_);
    std::shared_ptr<DreivedSubscriber> subscriber = std::make_shared<DreivedSubscriber>(subscribeInfo);
    DelayedSingleton<CommonEvent>::GetInstance()->SubscribeCommonEvent(subscriber);
    EXPECT_EQ(ERR_OK, DelayedSingleton<CommonEvent>::GetInstance()->UnSubscribeCommonEvent(subscriber));
}

/*
 * Feature: InnerCommonEventManager
 * Function: UnsubscribeCommonEvent
 * SubFunction: NA
 * FunctionPoints: normal
 * EnvConditions: system running normally
 * CaseDescription: Verify UnSubscribeCommonEvent function return value.
 */
HWTEST_F(CommonEventUnSubscribeTest, UnSubscribe_004, TestSize.Level1)
{
    CommonEventUnSubscribeTest::SetMatchingSkillsWithEvent("event");
    CommonEventSubscribeInfo subscribeInfo(matchingSkills_);
    std::shared_ptr<DreivedSubscriber> subscriber = std::make_shared<DreivedSubscriber>(subscribeInfo);
    sptr<IRemoteObject> commonEventListener = new CommonEventListener(subscriber);
    std::shared_ptr<InnerCommonEventManager> innerCommonEventManager = std::make_shared<InnerCommonEventManager>();
    EXPECT_EQ(true, innerCommonEventManager->UnsubscribeCommonEvent(commonEventListener));
}

/*
 * Feature: CommonEventSubscriberManager
 * Function: RemoveSubscriber
 * SubFunction: NA
 * FunctionPoints: normal
 * EnvConditions: system running normally
 * CaseDescription: Verify RemoveSubscriber function return value.
 */
HWTEST_F(CommonEventUnSubscribeTest, UnSubscribe_005, TestSize.Level1)
{
    CommonEventUnSubscribeTest::SetMatchingSkillsWithEvent("event");
    CommonEventSubscribeInfo subscribeInfo(matchingSkills_);
    std::shared_ptr<DreivedSubscriber> subscriber = std::make_shared<DreivedSubscriber>(subscribeInfo);
    sptr<IRemoteObject> commonEventListener = new CommonEventListener(subscriber);
    EXPECT_EQ(
        ERR_OK, DelayedSingleton<CommonEventSubscriberManager>::GetInstance()->RemoveSubscriber(commonEventListener));
}

/*
 * Feature: CommonEvent
 * Function: UnSubscribeCommonEvent
 * SubFunction: NA
 * FunctionPoints: exception
 * EnvConditions: system running normally
 * CaseDescription: When the subscriber does not exist, verify UnSubscribeCommonEvent function return value.
 */
HWTEST_F(CommonEventUnSubscribeTest, UnSubscribe_006, TestSize.Level1)
{
    std::shared_ptr<DreivedSubscriber> subscriber = nullptr;
    EXPECT_NE(ERR_OK, DelayedSingleton<CommonEvent>::GetInstance()->UnSubscribeCommonEvent(subscriber));
}

/*
 * Feature: InnerCommonEventManager
 * Function: UnsubscribeCommonEvent
 * SubFunction: NA
 * FunctionPoints: exception
 * EnvConditions: system running normally
 * CaseDescription: When the CommonEventListener does not exist,
 *                  verify UnsubscribeCommonEvent function return value.
 */
HWTEST_F(CommonEventUnSubscribeTest, UnSubscribe_009, TestSize.Level1)
{
    std::shared_ptr<InnerCommonEventManager> innerCommonEventManager = std::make_shared<InnerCommonEventManager>();
    sptr<IRemoteObject> sp(nullptr);
    EXPECT_EQ(false, innerCommonEventManager->UnsubscribeCommonEvent(sp));
}

/*
 * Feature: CommonEventSubscriberManager
 * Function: RemoveSubscriber
 * SubFunction: NA
 * FunctionPoints: exception
 * EnvConditions: system running normally
 * CaseDescription: When the CommonEventListener does not exist,
 *                  verify RemoveSubscriber function return value.
 */
HWTEST_F(CommonEventUnSubscribeTest, UnSubscribe_010, TestSize.Level1)
{
    EXPECT_EQ(
        ERR_INVALID_VALUE, DelayedSingleton<CommonEventSubscriberManager>::GetInstance()->RemoveSubscriber(nullptr));
}

/*
 * Feature: CommonEventSubscriberManager
 * Function: RemoveSubscriber
 * SubFunction: NA
 * FunctionPoints: exception
 * EnvConditions: system running normally
 * CaseDescription: When the death_ is not null,
 *                  verify RemoveSubscriber function return value.
 */
HWTEST_F(CommonEventUnSubscribeTest, UnSubscribe_011, TestSize.Level1)
{
    CommonEventUnSubscribeTest::SetMatchingSkillsWithEvent("event");
    CommonEventSubscribeInfo subscribeInfo(matchingSkills_);
    // make a subscriber object
    std::shared_ptr<SubscriberTest> subscriberTest = std::make_shared<SubscriberTest>(subscribeInfo);
    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberTest);
    DelayedSingleton<CommonEventSubscriberManager>::GetInstance()->death_ = nullptr;
    EXPECT_EQ(
        ERR_OK, DelayedSingleton<CommonEventSubscriberManager>::GetInstance()->RemoveSubscriber(commonEventListener));
}