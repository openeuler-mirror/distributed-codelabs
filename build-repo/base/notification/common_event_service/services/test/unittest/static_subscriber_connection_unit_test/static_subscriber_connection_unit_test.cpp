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

#include <gtest/gtest.h>

#define private public
#define protected public
#include "static_subscriber_connection.h"
#undef private
#undef protected

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::EventFwk;
using namespace OHOS::AppExecFwk;

extern bool IsOnReceiveEventCalled();
extern void ResetStaticSubscriberProxyMockState();

class StaticSubscriberConnectionUnitTest : public testing::Test {
public:
    StaticSubscriberConnectionUnitTest() {}

    virtual ~StaticSubscriberConnectionUnitTest() {}

    static void SetUpTestCase();

    static void TearDownTestCase();

    void SetUp();

    void TearDown();
};

void StaticSubscriberConnectionUnitTest::SetUpTestCase() {}

void StaticSubscriberConnectionUnitTest::TearDownTestCase() {}

void StaticSubscriberConnectionUnitTest::SetUp() {}

void StaticSubscriberConnectionUnitTest::TearDown() {}

/*
 * @tc.name: OnAbilityConnectDoneTest_0100
 * @tc.desc: test if StaticSubscriberConnection's OnAbilityConnectDone function executed as expected in normal case.
 * @tc.type: FUNC
 * @tc.require: #I5SJ62
 * 
 */
HWTEST_F(StaticSubscriberConnectionUnitTest, OnAbilityConnectDoneTest_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberConnectionUnitTest, OnAbilityConnectDoneTest_0100, TestSize.Level1";
    CommonEventData data;
    std::shared_ptr<StaticSubscriberConnection> conn = std::make_shared<StaticSubscriberConnection>(data);
    ASSERT_NE(nullptr, conn);
    ElementName element;
    sptr<IRemoteObject> remoteObject = nullptr;
    int resultCode = 0;
    conn->OnAbilityConnectDone(element, remoteObject, resultCode);
    conn->OnAbilityDisconnectDone(element, resultCode);
    EXPECT_TRUE(IsOnReceiveEventCalled());
    ResetStaticSubscriberProxyMockState();
}