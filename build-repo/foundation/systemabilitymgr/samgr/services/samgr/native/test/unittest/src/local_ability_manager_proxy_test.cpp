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
#include "local_ability_manager_proxy_test.h"

#include "itest_transaction_service.h"
#include "local_ability_manager_proxy.h"
#include "mock_iro_sendrequest.h"
#include "string_ex.h"
#include "test_log.h"

#define private public

using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS;

namespace OHOS {
namespace {
constexpr int32_t TEST_SAID_INVAILD = -1;
constexpr int32_t TEST_SAID_VAILD = 9999;
}
void LocalAbilityManagerProxyTest::SetUpTestCase()
{
    DTEST_LOG << "SetUpTestCase" << std::endl;
}

void LocalAbilityManagerProxyTest::TearDownTestCase()
{
    DTEST_LOG << "TearDownTestCase" << std::endl;
}

void LocalAbilityManagerProxyTest::SetUp()
{
    DTEST_LOG << "SetUp" << std::endl;
}

void LocalAbilityManagerProxyTest::TearDown()
{
    DTEST_LOG << "TearDown" << std::endl;
}

/**
 * @tc.name: LocalAbilityManagerProxy001
 * @tc.desc: LocalAbilityManagerProxy and check StartAbility
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(LocalAbilityManagerProxyTest, LocalAbilityManagerProxy001, TestSize.Level1)
{
    sptr<IRemoteObject> testAbility(new TestTransactionService());
    sptr<LocalAbilityManagerProxy> localAbility(new LocalAbilityManagerProxy(testAbility));
    bool res = localAbility->StartAbility(TEST_SAID_INVAILD);
    EXPECT_EQ(res, false);
}

/**
 * @tc.name: LocalAbilityManagerProxy002
 * @tc.desc: LocalAbilityManagerProxy and check StartAbility
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(LocalAbilityManagerProxyTest, LocalAbilityManagerProxy002, TestSize.Level1)
{
    sptr<LocalAbilityManagerProxy> localAbility(new LocalAbilityManagerProxy(nullptr));
    bool res = localAbility->StartAbility(TEST_SAID_VAILD);
    EXPECT_EQ(res, false);
}

/**
 * @tc.name: LocalAbilityManagerProxy003
 * @tc.desc: LocalAbilityManagerProxy and check StartAbility
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(LocalAbilityManagerProxyTest, LocalAbilityManagerProxy003, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<LocalAbilityManagerProxy> localAbility(new LocalAbilityManagerProxy(testAbility));
    bool res = localAbility->StartAbility(TEST_SAID_VAILD);
    EXPECT_EQ(res, true);
}

/**
 * @tc.name: LocalAbilityManagerProxy004
 * @tc.desc: LocalAbilityManagerProxy and check StartAbility
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(LocalAbilityManagerProxyTest, LocalAbilityManagerProxy004, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    testAbility->result_ = 1;
    sptr<LocalAbilityManagerProxy> localAbility(new LocalAbilityManagerProxy(testAbility));
    EXPECT_NE(localAbility, nullptr);
    bool res = localAbility->StartAbility(TEST_SAID_VAILD);
    EXPECT_EQ(res, false);
}
}