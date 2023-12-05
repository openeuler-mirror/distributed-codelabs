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
#include "ability_manager_death_recipient.h"
#undef private
#undef protected
#include "mock_ability_manager_stub.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::EventFwk;

extern bool IsClearCalled();
extern void ResetAbilityManagerHelperState();

class AbilityManagerDeathRecipientUnitTest : public testing::Test {
public:
    AbilityManagerDeathRecipientUnitTest() {}

    virtual ~AbilityManagerDeathRecipientUnitTest() {}

    static void SetUpTestCase();

    static void TearDownTestCase();

    void SetUp();

    void TearDown();
};

void AbilityManagerDeathRecipientUnitTest::SetUpTestCase() {}

void AbilityManagerDeathRecipientUnitTest::TearDownTestCase() {}

void AbilityManagerDeathRecipientUnitTest::SetUp() {}

void AbilityManagerDeathRecipientUnitTest::TearDown() {}

/*
 * @tc.name: OnRemoteDiedTest_0100
 * @tc.desc: test if AbilityManagerDeathRecipient's OnRemoteDied function executed as expected in normal case.
 * @tc.type: FUNC
 * @tc.require: #I5SJ62
 * 
 */
HWTEST_F(AbilityManagerDeathRecipientUnitTest, OnRemoteDiedTest_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "AbilityManagerDeathRecipientUnitTest, OnRemoteDiedTest_0100, TestSize.Level1";
    std::shared_ptr<AbilityManagerDeathRecipient> recipient = std::make_shared<AbilityManagerDeathRecipient>();
    ASSERT_NE(nullptr, recipient);
    sptr<MockAbilityManagerStub> sptrDeath = new (std::nothrow) MockAbilityManagerStub();
    ASSERT_NE(nullptr, sptrDeath);
    wptr<MockAbilityManagerStub> wptrDeath = sptrDeath;
    recipient->OnRemoteDied(wptrDeath);
    EXPECT_TRUE(IsClearCalled());
    ResetAbilityManagerHelperState();
}

/*
 * @tc.name: OnRemoteDiedTest_0200
 * @tc.desc: test if AbilityManagerDeathRecipient's OnRemoteDied function executed as expected when param is nullptr.
 * @tc.type: FUNC
 * @tc.require: #I5SJ62
 * 
 */
HWTEST_F(AbilityManagerDeathRecipientUnitTest, OnRemoteDiedTest_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "AbilityManagerDeathRecipientUnitTest, OnRemoteDiedTest_0200, TestSize.Level1";
    std::shared_ptr<AbilityManagerDeathRecipient> recipient = std::make_shared<AbilityManagerDeathRecipient>();
    ASSERT_NE(nullptr, recipient);
    wptr<IRemoteObject> wptrDeath = nullptr;
    recipient->OnRemoteDied(wptrDeath);
    EXPECT_FALSE(IsClearCalled());
    ResetAbilityManagerHelperState();
}

/*
 * @tc.name: OnRemoteDiedTest_0200
 * @tc.desc: test if AbilityManagerDeathRecipient's OnRemoteDied function executed as expected when
 *           sptr param is nullptr.
 * @tc.type: FUNC
 * @tc.require: #I5SJ62
 * 
 */
HWTEST_F(AbilityManagerDeathRecipientUnitTest, OnRemoteDiedTest_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "AbilityManagerDeathRecipientUnitTest, OnRemoteDiedTest_0300, TestSize.Level1";
    std::shared_ptr<AbilityManagerDeathRecipient> recipient = std::make_shared<AbilityManagerDeathRecipient>();
    ASSERT_NE(nullptr, recipient);
    sptr<IRemoteObject> sptrDeath = nullptr;
    ASSERT_EQ(nullptr, sptrDeath);
    wptr<IRemoteObject> wptrDeath = sptrDeath;
    recipient->OnRemoteDied(wptrDeath);
    EXPECT_FALSE(IsClearCalled());
    ResetAbilityManagerHelperState();
}