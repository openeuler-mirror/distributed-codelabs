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
#include "gtest/gtest.h"
#include "iservice_registry.h"
#include "local_ability_manager_stub.h"
#include "memory"
#include "sa_mock_permission.h"
#include "test_log.h"

#define private public
#define protected public
#include "local_ability_manager.h"
#include "mock_sa_realize.h"
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace SAFWK {
namespace {
    constexpr int32_t SAID = 1489;
    const std::string TEST_RESOURCE_PATH = "/data/test/resource/samgr/profile/";
    constexpr int32_t LISTENER_ID = 1488;
    constexpr int32_t MOCK_DEPEND_TIMEOUT = 1000;
}

class MockLocalAbilityManager : public LocalAbilityManagerStub {
public:
    MockLocalAbilityManager() = default;
    ~MockLocalAbilityManager() = default;

    bool StartAbility(int32_t systemAbilityId) override;
};
class SystemAbilityTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void SystemAbilityTest::SetUpTestCase()
{
    DTEST_LOG << "SetUpTestCase" << std::endl;
}

void SystemAbilityTest::TearDownTestCase()
{
    DTEST_LOG << "TearDownTestCase" << std::endl;
}

void SystemAbilityTest::SetUp()
{
    SaMockPermission::MockPermission();
    DTEST_LOG << "SetUp" << std::endl;
}

void SystemAbilityTest::TearDown()
{
    DTEST_LOG << "TearDown" << std::endl;
}

bool MockLocalAbilityManager::StartAbility(int32_t systemAbilityId)
{
    DTEST_LOG << "said : " << systemAbilityId <<std::endl;
    return true;
}

/**
 * @tc.name: RemoveSystemAbilityListener001
 * @tc.desc: Check RemoveSystemAbilityListener
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityTest, RemoveSystemAbilityListener001, TestSize.Level2)
{
    std::shared_ptr<SystemAbility> sysAby = std::make_shared<MockSaRealize>(SAID, false);
    sysAby->AddSystemAbilityListener(LISTENER_ID);
    bool res = sysAby->RemoveSystemAbilityListener(LISTENER_ID);
    EXPECT_EQ(res, true);
}

/**
 * @tc.name: MakeAndRegisterAbility001
 * @tc.desc: Check MakeAndRegisterAbility
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityTest, MakeAndRegisterAbility001, TestSize.Level2)
{
    bool res = SystemAbility::MakeAndRegisterAbility(new MockSaRealize(SAID, false));
    EXPECT_EQ(res, false);
}

/**
 * @tc.name: MakeAndRegisterAbility002
 * @tc.desc: Check MakeAndRegisterAbility
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityTest, MakeAndRegisterAbility002, TestSize.Level2)
{
    bool ret = LocalAbilityManager::GetInstance().profileParser_->ParseSaProfiles(TEST_RESOURCE_PATH + "1489.xml");
    EXPECT_EQ(ret, true);
    bool res = SystemAbility::MakeAndRegisterAbility(new MockSaRealize(SAID, false));
    EXPECT_EQ(res, true);
}

/**
 * @tc.name: Publish001
 * @tc.desc: Verify Publish when systemabitly is nullptr
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityTest, Publish001, TestSize.Level2)
{
    std::shared_ptr<SystemAbility> sysAby = std::make_shared<MockSaRealize>(SAID, false);
    bool ret = sysAby->Publish(nullptr);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: Publish002
 * @tc.desc: Verify Publish function
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityTest, Publish002, TestSize.Level2)
{
    std::shared_ptr<SystemAbility> sysAby = std::make_shared<MockSaRealize>(SAID, false);
    sptr<IRemoteObject> obj(new MockLocalAbilityManager());
    bool ret = sysAby->Publish(obj);
    ASSERT_TRUE(ret);
    sysAby->Stop();
    sysAby->StopAbility(-1);
    sysAby->Start();
    sysAby->Stop();
    EXPECT_FALSE(sysAby->isRunning_);
}

/**
 * @tc.name: SetDependTimeout001
 * @tc.desc: Verify SetDependTimeout
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityTest, SetDependTimeout001, TestSize.Level2)
{
    std::shared_ptr<SystemAbility> sysAby = std::make_shared<MockSaRealize>(SAID, false);
    sysAby->SetDependTimeout(0);
    sysAby->SetDependTimeout(MOCK_DEPEND_TIMEOUT);
    EXPECT_EQ(sysAby->GetDependTimeout(), MOCK_DEPEND_TIMEOUT);
}

/**
 * @tc.name: GetSystemAbility001
 * @tc.desc: Check GetSystemAbility
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityTest, GetSystemAbility001, TestSize.Level2)
{
    std::shared_ptr<SystemAbility> sysAby = std::make_shared<MockSaRealize>(SAID, false);
    sptr<IRemoteObject> obj = sysAby->GetSystemAbility(-1);
    EXPECT_TRUE(obj == nullptr);
}
}
}