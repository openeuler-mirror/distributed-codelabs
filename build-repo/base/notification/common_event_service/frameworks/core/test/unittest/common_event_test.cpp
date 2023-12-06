/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "common_event.h"
#include "common_event_death_recipient.h"
#include "common_event_stub.h"
#include "common_event_subscriber.h"
#include "event_receive_proxy.h"
#include "common_event_publish_info.h"
#include "matching_skills.h"

#include <gtest/gtest.h>

using namespace testing::ext;
using namespace OHOS::EventFwk;

namespace {
    const std::string EVENT = "com.ces.test.event";
    const std::string PERMISSION = "com.ces.test.permission";
    constexpr uint16_t SYSTEM_UID = 1000;
    constexpr int32_t ERR_OK = 0;
}

class CommonEventTest : public CommonEventSubscriber, public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    virtual void OnReceiveEvent(const CommonEventData &data)
    {}
};

void CommonEventTest::SetUpTestCase()
{}

void CommonEventTest::TearDownTestCase()
{}

void CommonEventTest::SetUp()
{}

void CommonEventTest::TearDown()
{}

/*
 * tc.number: CommonEvent_001
 * tc.name: test PublishCommonEvent
 * tc.type: FUNC
 * tc.require: issueI5NGO7
 * tc.desc: Invoke PublishCommonEvent interface verify whether it is normal
 */
HWTEST_F(CommonEventTest, CommonEvent_001, TestSize.Level1)
{
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

    std::shared_ptr<CommonEventSubscriber> subscriber = nullptr;
    
    CommonEvent commonEvent;
    bool publishCommonEvent = commonEvent.PublishCommonEvent(data, publishInfo, subscriber);
    EXPECT_EQ(true, publishCommonEvent);
}

/*
 * tc.number: CommonEvent_002
 * tc.name: test PublishCommonEvent
 * tc.type: FUNC
 * tc.require: issueI5NGO7
 * tc.desc: Invoke PublishCommonEvent interface verify whether it is normal
 */
HWTEST_F(CommonEventTest, CommonEvent_002, TestSize.Level1)
{
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

    std::shared_ptr<CommonEventSubscriber> subscriber = nullptr;
    
    // EVENT is normal common event
    CommonEvent commonEvent;
    bool publishCommonEvent = commonEvent.PublishCommonEvent(data, publishInfo, subscriber, SYSTEM_UID, 0);
    EXPECT_EQ(true, publishCommonEvent);
}

/*
 * tc.number: CommonEvent_003
 * tc.name: test Freeze
 * tc.type: FUNC
 * tc.require: issueI5NGO7
 * tc.desc: Invoke Freeze interface verify whether it is normal
 */
HWTEST_F(CommonEventTest, CommonEvent_003, TestSize.Level1)
{
    CommonEvent commonEvent;
    bool freeze = commonEvent.Freeze(SYSTEM_UID);
    EXPECT_EQ(true, freeze);
}

/*
 * tc.number: CommonEvent_004
 * tc.name: test Unfreeze
 * tc.type: FUNC
 * tc.require: issueI5NGO7
 * tc.desc: Invoke Unfreeze interface verify whether it is normal
 */
HWTEST_F(CommonEventTest, CommonEvent_004, TestSize.Level1)
{
    CommonEvent commonEvent;
    bool unfreeze = commonEvent.Unfreeze(SYSTEM_UID);
    EXPECT_EQ(true, unfreeze);
}

/*
 * tc.number: CommonEvent_005
 * tc.name: test Unfreeze
 * tc.type: FUNC
 * tc.require: issueI5NGO7
 * tc.desc: Invoke Unfreeze interface verify whether it is normal
 */
HWTEST_F(CommonEventTest, CommonEvent_005, TestSize.Level1)
{
    // make a want
    Want want;
    want.SetAction(EVENT);
    // make common event data
    CommonEventData data;
    data.SetWant(want);

    std::shared_ptr<EventReceiveProxy> result = std::make_shared<EventReceiveProxy>(nullptr);
    if (result != nullptr) {
        result->NotifyEvent(data, true, true);
    }

    CommonEvent commonEvent;
    commonEvent.ResetCommonEventProxy();
    bool unfreezeAll = commonEvent.UnfreezeAll();
    EXPECT_EQ(true, unfreezeAll);
}

/*
 * tc.number: CommonEventStub_001
 * tc.name: test PublishCommonEvent
 * tc.type: FUNC
 * tc.desc: Invoke PublishCommonEvent interface verify whether it is normal
 */
HWTEST_F(CommonEventTest, CommonEventStub_001, TestSize.Level1)
{
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

    const int32_t userId = 1;

    CommonEventStub commonEventStub;
    int32_t publishCommonEvent = commonEventStub.PublishCommonEvent(data, publishInfo, nullptr, userId);
    EXPECT_EQ(ERR_OK, publishCommonEvent);
}

/*
 * tc.number: CommonEventStub_002
 * tc.name: test PublishCommonEvent
 * tc.type: FUNC
 * tc.desc: Invoke PublishCommonEvent interface verify whether it is normal
 */
HWTEST_F(CommonEventTest, CommonEventStub_002, TestSize.Level1)
{
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

    const int32_t userId = 1;

    CommonEventStub commonEventStub;
    bool publishCommonEvent = commonEventStub.PublishCommonEvent(data, publishInfo, nullptr, SYSTEM_UID, 0, userId);
    EXPECT_EQ(true, publishCommonEvent);
}

/*
 * tc.number: CommonEventStub_003
 * tc.name: test SubscribeCommonEvent
 * tc.type: FUNC
 * tc.desc: Invoke SubscribeCommonEvent interface verify whether it is normal
 */
HWTEST_F(CommonEventTest, CommonEventStub_003, TestSize.Level1)
{
    CommonEventSubscribeInfo subscribeInfo;

    CommonEventStub commonEventStub;
    bool subscribeCommonEvent = commonEventStub.SubscribeCommonEvent(subscribeInfo, nullptr);
    EXPECT_EQ(ERR_OK, subscribeCommonEvent);
}

/*
 * tc.number: CommonEventStub_004
 * tc.name: test UnsubscribeCommonEvent
 * tc.type: FUNC
 * tc.desc: Invoke UnsubscribeCommonEvent interface verify whether it is normal
 */
HWTEST_F(CommonEventTest, CommonEventStub_004, TestSize.Level1)
{
    CommonEventStub commonEventStub;
    bool unsubscribeCommonEvent = commonEventStub.UnsubscribeCommonEvent(nullptr);
    EXPECT_EQ(true, unsubscribeCommonEvent);
}

/*
 * tc.number: CommonEventStub_005
 * tc.name: test GetStickyCommonEvent
 * tc.type: FUNC
 * tc.desc: Invoke GetStickyCommonEvent interface verify whether it is normal
 */
HWTEST_F(CommonEventTest, CommonEventStub_005, TestSize.Level1)
{
    const std::string event = "aa";

    // make a want
    Want want;
    want.SetAction(EVENT);
    // make common event data
    CommonEventData data;
    data.SetWant(want);

    CommonEventStub commonEventStub;
    bool getStickyCommonEvent = commonEventStub.GetStickyCommonEvent(event, data);
    EXPECT_EQ(true, getStickyCommonEvent);
}

/*
 * tc.number: CommonEventStub_006
 * tc.name: test DumpState
 * tc.type: FUNC
 * tc.desc: Invoke DumpState interface verify whether it is normal
 */
HWTEST_F(CommonEventTest, CommonEventStub_006, TestSize.Level1)
{
    const uint8_t dumpType = 1;
    const int32_t userId = 2;
    std::vector<std::string> state;

    CommonEventStub commonEventStub;
    bool dumpState = commonEventStub.DumpState(dumpType, EVENT, userId, state);
    EXPECT_EQ(true, dumpState);
}

/*
 * tc.number: CommonEventStub_007
 * tc.name: test FinishReceiver
 * tc.type: FUNC
 * tc.desc: Invoke FinishReceiver interface verify whether it is normal
 */
HWTEST_F(CommonEventTest, CommonEventStub_007, TestSize.Level1)
{
    const int32_t code = 1;
    const std::string receiverData = "bb";

    CommonEventStub commonEventStub;
    bool finishReceiver = commonEventStub.FinishReceiver(nullptr, code, receiverData, true);
    EXPECT_EQ(true, finishReceiver);
}

/*
 * tc.number: CommonEventStub_008
 * tc.name: test Freeze Unfreeze UnfreezeAll
 * tc.type: FUNC
 * tc.desc: Invoke Freeze Unfreeze UnfreezeAll interface verify whether it is normal
 */
HWTEST_F(CommonEventTest, CommonEventStub_008, TestSize.Level1)
{
    CommonEventStub commonEventStub;
    bool freeze = commonEventStub.Freeze(SYSTEM_UID);
    EXPECT_EQ(true, freeze);
    bool unfreeze = commonEventStub.Unfreeze(SYSTEM_UID);
    EXPECT_EQ(true, unfreeze);
    bool unfreezeAll = commonEventStub.UnfreezeAll();
    EXPECT_EQ(true, unfreezeAll);
}

/*
 * tc.number: CommonEventStub_009
 * tc.name: test OnRemoteRequest
 * tc.type: FUNC
 * tc.desc: Invoke OnRemoteRequest interface verify whether it is normal
 */
HWTEST_F(CommonEventTest, CommonEventStub_009, TestSize.Level1)
{
    const uint32_t code = -1;
    OHOS::MessageParcel dataParcel;
    OHOS::MessageParcel reply;
    OHOS::MessageOption option;

    CommonEventStub commonEventStub;
    int result = commonEventStub.OnRemoteRequest(code, dataParcel, reply, option);
    EXPECT_EQ(OHOS::ERR_TRANSACTION_FAILED, result);
}

/*
 * tc.number: MatchingSkills_001
 * tc.name: test Unmarshalling
 * tc.type: FUNC
 * tc.desc: Invoke Unmarshalling interface verify whether it is normal
 */
HWTEST_F(CommonEventTest, MatchingSkills_001, TestSize.Level1)
{
    bool unmarshalling = true;
    OHOS::Parcel parcel;
    std::shared_ptr<MatchingSkills> result = std::make_shared<MatchingSkills>();

    // write entity
    int32_t value = 1;
    parcel.WriteInt32(value);
    std::vector<std::u16string> actionU16Entity;
    for (std::vector<std::string>::size_type i = 0; i < 3; i++) {
        actionU16Entity.emplace_back(OHOS::Str8ToStr16("test"));
    }
    result->WriteVectorInfo(parcel, actionU16Entity);

    //write event
    parcel.WriteInt32(value);
    std::vector<std::u16string> actionU16Event;
    for (std::vector<std::string>::size_type i = 0; i < 3; i++) {
        actionU16Event.emplace_back(OHOS::Str8ToStr16("test"));
    }
    result->WriteVectorInfo(parcel, actionU16Event);

    // write scheme
    parcel.WriteInt32(value);
    std::vector<std::u16string> actionU16Scheme;
    for (std::vector<std::string>::size_type i = 0; i < 3; i++) {
        actionU16Scheme.emplace_back(OHOS::Str8ToStr16("test"));
    }
    result->WriteVectorInfo(parcel, actionU16Scheme);
    
    if (nullptr == result->Unmarshalling(parcel)) {
        unmarshalling = false;
    }
    EXPECT_EQ(true, unmarshalling);
}

/*
 * @tc.number: CommonEventPublishInfo_0100
 * @tc.name: verify SetBundleName
 * @tc.desc: Invoke SetBundleName interface verify whether it is normal
 */
HWTEST_F(CommonEventTest, CommonEventPublishInfo_0100, Function | MediumTest | Level1)
{
    const std::string bundleName = "aa";
    CommonEventPublishInfo publishInfo;
    publishInfo.SetBundleName(bundleName);
    std::string getBundleName = publishInfo.GetBundleName();
    EXPECT_EQ(bundleName, getBundleName);
}

/*
 * @tc.number: CommonEventPublishInfo_0200
 * @tc.name: verify Unmarshalling
 * @tc.desc: Invoke Unmarshalling interface verify whether it is normal
 */
HWTEST_F(CommonEventTest, CommonEventPublishInfo_0200, Function | MediumTest | Level1)
{
    bool result = true;
    OHOS::Parcel parcel;
    CommonEventPublishInfo publishInfo;
    if (nullptr == publishInfo.Unmarshalling(parcel)) {
        result = false;
    }
    EXPECT_EQ(true, result);
}

/*
 * @tc.number: CommonEventSubscribeInfo_0100
 * @tc.name: verify Unmarshalling
 * @tc.desc: Invoke Unmarshalling interface verify whether it is normal
 */
HWTEST_F(CommonEventTest, CommonEventSubscribeInfo_0100, Function | MediumTest | Level1)
{
    bool result = false;
    OHOS::Parcel parcel;
    std::string eventName = "";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    if (nullptr == subscribeInfo.Unmarshalling(parcel)) {
        result = true;
    }
    EXPECT_EQ(true, result);
}

/*
 * @tc.number: CommonEventSubscriber_0100
 * @tc.name: verify IsStickyCommonEvent
 * @tc.desc: Invoke IsStickyCommonEvent interface verify whether it is normal
 */
HWTEST_F(CommonEventTest, CommonEventSubscriber_0100, Function | MediumTest | Level1)
{
    CommonEventDeathRecipient commonEventDeathRecipient;
    commonEventDeathRecipient.OnRemoteDied(nullptr);
    bool result = CommonEventTest::IsStickyCommonEvent();
    EXPECT_EQ(false, result);
}