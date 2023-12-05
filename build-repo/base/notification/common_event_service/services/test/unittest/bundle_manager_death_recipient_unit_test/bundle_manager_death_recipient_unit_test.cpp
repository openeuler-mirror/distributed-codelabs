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
#include "bms_death_recipient.h"
#undef private
#undef protected

#include "iremote_broker.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::EventFwk;

extern bool IsClearBundleManagerHelperCalled();
extern void ResetBundleManagerHelperMock();

class BMSDeathRecipientUnitTest : public testing::Test {
public:
    BMSDeathRecipientUnitTest() {}

    virtual ~BMSDeathRecipientUnitTest() {}

    static void SetUpTestCase();

    static void TearDownTestCase();

    void SetUp();

    void TearDown();
};

void BMSDeathRecipientUnitTest::SetUpTestCase() {}

void BMSDeathRecipientUnitTest::TearDownTestCase() {}

void BMSDeathRecipientUnitTest::SetUp() {}

void BMSDeathRecipientUnitTest::TearDown() {}

class MockIRemoteObject : public IRemoteObject {
public:
    MockIRemoteObject() : IRemoteObject(u"mock_i_remote_object") {}

    ~MockIRemoteObject() {}

    int32_t GetObjectRefCount() override
    {
        return 0;
    }

    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        return 0;
    }

    bool IsProxyObject() const override
    {
        return true;
    }

    bool CheckObjectLegality() const override
    {
        return true;
    }

    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        return true;
    }

    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        return true;
    }

    bool Marshalling(Parcel &parcel) const override
    {
        return true;
    }

    sptr<IRemoteBroker> AsInterface() override
    {
        return nullptr;
    }

    int Dump(int fd, const std::vector<std::u16string> &args) override
    {
        return 0;
    }

    std::u16string GetObjectDescriptor() const
    {
        std::u16string descriptor = std::u16string();
        return descriptor;
    }
};

/*
 * @tc.name: OnRemoteDiedTest_0100
 * @tc.desc: test if BMSDeathRecipient's OnRemoteDied function executed as expected in normal case.
 * @tc.type: FUNC
 * @tc.require: #I5SJ62
 * 
 */
HWTEST_F(BMSDeathRecipientUnitTest, OnRemoteDiedTest_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "BMSDeathRecipientUnitTest, OnRemoteDiedTest_0100, TestSize.Level1";
    std::shared_ptr<BMSDeathRecipient> recipient = std::make_shared<BMSDeathRecipient>();
    ASSERT_NE(nullptr, recipient);
    sptr<MockIRemoteObject> sptrDeath = new (std::nothrow) MockIRemoteObject();
    ASSERT_NE(nullptr, sptrDeath);
    wptr<MockIRemoteObject> wptrDeath = sptrDeath;
    recipient->OnRemoteDied(wptrDeath);
    EXPECT_TRUE(IsClearBundleManagerHelperCalled());
    ResetBundleManagerHelperMock();
}

/*
 * @tc.name: OnRemoteDiedTest_0200
 * @tc.desc: test if BMSDeathRecipient's OnRemoteDied function executed as expected when param is nullptr.
 * @tc.type: FUNC
 * @tc.require: #I5SJ62
 * 
 */
HWTEST_F(BMSDeathRecipientUnitTest, OnRemoteDiedTest_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "BMSDeathRecipientUnitTest, OnRemoteDiedTest_0200, TestSize.Level1";
    std::shared_ptr<BMSDeathRecipient> recipient = std::make_shared<BMSDeathRecipient>();
    ASSERT_NE(nullptr, recipient);
    wptr<IRemoteObject> wptrDeath = nullptr;
    recipient->OnRemoteDied(wptrDeath);
    EXPECT_FALSE(IsClearBundleManagerHelperCalled());
    ResetBundleManagerHelperMock();
}

/*
 * @tc.name: OnRemoteDiedTest_0200
 * @tc.desc: test if BMSDeathRecipient's OnRemoteDied function executed as expected when
 *           sptr param is nullptr.
 * @tc.type: FUNC
 * @tc.require: #I5SJ62
 * 
 */
HWTEST_F(BMSDeathRecipientUnitTest, OnRemoteDiedTest_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "BMSDeathRecipientUnitTest, OnRemoteDiedTest_0300, TestSize.Level1";
    std::shared_ptr<BMSDeathRecipient> recipient = std::make_shared<BMSDeathRecipient>();
    ASSERT_NE(nullptr, recipient);
    sptr<IRemoteObject> sptrDeath = nullptr;
    ASSERT_EQ(nullptr, sptrDeath);
    wptr<IRemoteObject> wptrDeath = sptrDeath;
    recipient->OnRemoteDied(wptrDeath);
    EXPECT_FALSE(IsClearBundleManagerHelperCalled());
    ResetBundleManagerHelperMock();
}