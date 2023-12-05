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
#include "common_event_proxy.h"
#include "common_event_data.h"
#include "common_event_subscriber.h"
#include "event_receive_proxy.h"
#include "common_event_publish_info.h"
#include "matching_skills.h"

#include "event_receive_stub.h"
#include "event_log_wrapper.h"

#include <gtest/gtest.h>

using namespace testing::ext;
using namespace OHOS::EventFwk;
using namespace OHOS;

namespace {
    const std::string EVENT = "com.ces.test.event";
    const std::string PERMISSION = "com.ces.test.permission";
}

class MockEventReceiveStub : public EventReceiveStub {
public:
    MockEventReceiveStub() = default;
    virtual ~MockEventReceiveStub() = default;

    void NotifyEvent(const CommonEventData &data, const bool &ordered, const bool &sticky) override
    {}
};

class EventReceiveStubTest : public CommonEventSubscriber, public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    virtual void OnReceiveEvent(const CommonEventData &data)
    {}
};

void EventReceiveStubTest::SetUpTestCase()
{}

void EventReceiveStubTest::TearDownTestCase()
{}

void EventReceiveStubTest::SetUp()
{}

void EventReceiveStubTest::TearDown()
{}

/*
 * tc.number: OnRemoteRequest_001
 * tc.name: test EventReceiveStubTest
 * tc.type: FUNC
 * tc.require: issueI5NGO7
 * tc.desc: Invoke CommonEventStub interface verify whether it is normal
 */
HWTEST_F(EventReceiveStubTest, OnRemoteRequest_001, TestSize.Level1)
{
    const uint32_t code = -1;
    OHOS::MessageParcel dataParcel;
    OHOS::MessageParcel reply;
    OHOS::MessageOption option;

    MockEventReceiveStub eventReceiveStub;
    int result = eventReceiveStub.OnRemoteRequest(code, dataParcel, reply, option);
    EXPECT_EQ(OHOS::ERR_TRANSACTION_FAILED, result);
}

/*
 * tc.number: OnRemoteRequest_002
 * tc.name: test EventReceiveStubTest
 * tc.type: FUNC
 * tc.require: issueI5NGO7
 * tc.desc: Invoke CommonEventStub interface verify whether it is normal
 */
HWTEST_F(EventReceiveStubTest, OnRemoteRequest_002, TestSize.Level1)
{
    const uint32_t code = 1;
    OHOS::MessageParcel dataParcel;
    OHOS::MessageParcel reply;
    OHOS::MessageOption option;

    MockEventReceiveStub eventReceiveStub;
    dataParcel.WriteInterfaceToken(MockEventReceiveStub::GetDescriptor());
    int result = eventReceiveStub.OnRemoteRequest(code, dataParcel, reply, option);
    EXPECT_NE(OHOS::NO_ERROR, result);
}
