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
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "itest_transaction_service.h"
#include "parameter.h"
#include "system_ability_definition.h"
#include "test_log.h"

#define private public
#include "system_ability_manager.h"

namespace OHOS {
namespace SAMGR {
namespace {
constexpr int32_t LOOP_SIZE = 1000;
}

class SystemAbilityManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void SystemAbilityManagerTest::SetUpTestCase()
{
    DTEST_LOG << "SystemAbilityManagerTest SetUpTestCase" << std::endl;
}

void SystemAbilityManagerTest::TearDownTestCase()
{
    DTEST_LOG << "SystemAbilityManagerTest TearDownTestCase" << std::endl;
}

void SystemAbilityManagerTest::SetUp()
{
    DTEST_LOG << "SystemAbilityManagerTest SetUp" << std::endl;
}

void SystemAbilityManagerTest::TearDown()
{
    DTEST_LOG << "SystemAbilityManagerTest TearDown" << std::endl;
}

/**
 * @tc.name: param check samgr ready event
 * @tc.desc: param check samgr ready event
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityManagerTest, SamgrReady001, TestSize.Level1)
{
    DTEST_LOG << " SamgrReady001 start " << std::endl;
    /**
     * @tc.steps: step1. param check samgr ready event
     * @tc.expected: step1. param check samgr ready event
     */
    auto ret = WaitParameter("bootevent.samgr.ready", "true", 1);
    ASSERT_EQ(ret, 0);
}

/**
 * @tc.name: CheckSystemAbilityTest001
 * @tc.desc: check system ability with deviceid
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityManagerTest, CheckSystemAbilityTest001, TestSize.Level1)
{
    DTEST_LOG << " CheckSystemAbilityTest001 start " << std::endl;
    /**
     * @tc.steps: step1. get samgr instance
     * @tc.expected: step1. samgr instance not nullptr
     */
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_TRUE(sm != nullptr);
    /**
     * @tc.steps: step2. get system ability with deviceid
     * @tc.expected: step2. get system ability with deviceid
     */
    std::string mockDeviceid = "12345678";
    sptr<IRemoteObject> ability = sm->CheckSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID, mockDeviceid);
    ASSERT_TRUE(ability == nullptr);
}

/**
 * @tc.name: CheckSystemAbilityTest002
 * @tc.desc: check system ability with deviceid
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityManagerTest, CheckSystemAbilityTest002, TestSize.Level1)
{
    DTEST_LOG << " CheckSystemAbilityTest002 start " << std::endl;
    /**
     * @tc.steps: step1. get samgr instance
     * @tc.expected: step1. samgr instance not nullptr
     */
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_TRUE(sm != nullptr);
    /**
     * @tc.steps: step2. get system ability with deviceid
     * @tc.expected: step2. get system ability with deviceid
     */
    std::string mockDeviceid = "012345679000111";
    int32_t mockSaid = 0;
    sptr<IRemoteObject> ability = sm->CheckSystemAbility(mockSaid, mockDeviceid);
    ASSERT_TRUE(ability == nullptr);
}

/**
 * @tc.name: CheckSystemAbilityTest003
 * @tc.desc: check system ability with deviceid
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityManagerTest, CheckSystemAbilityTest003, TestSize.Level1)
{
    DTEST_LOG << " CheckSystemAbilityTest003 start " << std::endl;
    /**
     * @tc.steps: step1. get samgr instance
     * @tc.expected: step1. samgr instance not nullptr
     */
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    /**
     * @tc.steps: step2. get system ability with deviceid
     * @tc.expected: step2. get system ability with deviceid
     */
    ASSERT_TRUE(sm != nullptr);
    int32_t mockSaid = -1;
    sptr<IRemoteObject> ability = sm->CheckSystemAbility(mockSaid, "");
    ASSERT_TRUE(ability == nullptr);
}

/**
 * @tc.name: CheckSystemAbilityTest004
 * @tc.desc: check system ability with deviceid
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityManagerTest, CheckSystemAbilityTest004, TestSize.Level3)
{
    DTEST_LOG << " CheckSystemAbilityTest004 start " << std::endl;
    /**
     * @tc.steps: step1. get samgr instance
     * @tc.expected: step1. samgr instance not nullptr
     */
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_TRUE(sm != nullptr);
    /**
     * @tc.steps: step2. get system ability with deviceid
     * @tc.expected: step2. get system ability with deviceid
     */
    for (int32_t i = 0; i < LOOP_SIZE; ++i) {
        sptr<IRemoteObject> ability = sm->CheckSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID, "");
        ASSERT_TRUE(ability == nullptr);
    }
}

/**
 * @tc.name: CheckSystemAbilityTest005
 * @tc.desc: check system ability with deviceid
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityManagerTest, CheckSystemAbilityTest005, TestSize.Level1)
{
    DTEST_LOG << " CheckSystemAbilityTest005 start " << std::endl;
    /**
     * @tc.steps: step1. get samgr instance
     * @tc.expected: step1. samgr instance not nullptr
     */
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_TRUE(sm != nullptr);
    /**
     * @tc.steps: step2. get system ability with mock said
     * @tc.expected: step2. not get sa
     */
    int32_t mockSaid = -1;
    bool isExist = true;
    sptr<IRemoteObject> ability = sm->CheckSystemAbility(mockSaid, isExist);
    ASSERT_TRUE(ability == nullptr);
}

/**
 * @tc.name: CheckSystemAbilityTest006
 * @tc.desc: check system ability with deviceid
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityManagerTest, CheckSystemAbilityTest006, TestSize.Level1)
{
    DTEST_LOG << " CheckSystemAbilityTest006 start " << std::endl;
    /**
     * @tc.steps: step1. get samgr instance
     * @tc.expected: step1. samgr instance not nullptr
     */
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_TRUE(sm != nullptr);
    /**
     * @tc.steps: step2. get system ability with mock said
     * @tc.expected: step2. not get sa
     */
    int32_t mockSaid = -1;
    bool isExist = false;
    sptr<IRemoteObject> ability = sm->CheckSystemAbility(mockSaid, isExist);
    ASSERT_TRUE(ability == nullptr);
}

/**
 * @tc.name: CheckSystemAbilityTest007
 * @tc.desc: check system ability with deviceid
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityManagerTest, CheckSystemAbilityTest007, TestSize.Level1)
{
    DTEST_LOG << " CheckSystemAbilityTest007 start " << std::endl;
    /**
     * @tc.steps: step1. get samgr instance
     * @tc.expected: step1. samgr instance not nullptr
     */
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_TRUE(sm != nullptr);
    /**
     * @tc.steps: step2. get system ability with mock said
     * @tc.expected: step2. not get sa
     */
    bool isExist = false;
    sptr<IRemoteObject> ability = sm->CheckSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID, isExist);
    ASSERT_TRUE(ability == nullptr);
}

/**
 * @tc.name: CheckSystemAbilityTest008
 * @tc.desc: check system ability with deviceid
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityManagerTest, CheckSystemAbilityTest008, TestSize.Level1)
{
    DTEST_LOG << " CheckSystemAbilityTest008 start " << std::endl;
    /**
     * @tc.steps: step1. get samgr instance
     * @tc.expected: step1. samgr instance not nullptr
     */
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_TRUE(sm != nullptr);
    /**
     * @tc.steps: step2. get system ability with mock said
     * @tc.expected: step2. not get sa
     */
    bool isExist = true;
    sptr<IRemoteObject> ability = sm->CheckSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID, isExist);
    ASSERT_TRUE(ability == nullptr);
}

/**
 * @tc.name: AddSystemProcess001
 * @tc.desc: check add process remoteobject
 */
HWTEST_F(SystemAbilityManagerTest, AddSystemProcess001, TestSize.Level1)
{
    DTEST_LOG << " AddSystemProcess001 start " << std::endl;
    /**
     * @tc.steps: step1. get samgr instance
     * @tc.expected: step1. samgr instance not nullptr
     */
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_TRUE(sm != nullptr);
    /**
     * @tc.steps: step2. add system process remote obejct
     * @tc.expected: step2. add system process remote obejct
     */
    int32_t ret = sm->AddSystemProcess("", nullptr);
    ASSERT_TRUE(ret != ERR_OK);
}

/**
 * @tc.name: AddSystemProcess002
 * @tc.desc: check add process remoteobject
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityManagerTest, AddSystemProcess002, TestSize.Level1)
{
    DTEST_LOG << " AddSystemProcess002 start " << std::endl;
    /**
     * @tc.steps: step1. get samgr instance
     * @tc.expected: step1. samgr instance not nullptr
     */
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_TRUE(sm != nullptr);
    /**
     * @tc.steps: step2. add system process remote obejct
     * @tc.expected: step2. add system process remote obejct
     */
    int32_t ret = sm->AddSystemProcess("test_process", nullptr);
    ASSERT_TRUE(ret != ERR_OK);
}

/**
 * @tc.name: AddSystemProcess003
 * @tc.desc: check add process remoteobject
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityManagerTest, AddSystemProcess003, TestSize.Level1)
{
    DTEST_LOG << " AddSystemProcess003 start " << std::endl;
    /**
     * @tc.steps: step1. get samgr instance
     * @tc.expected: step1. samgr instance not nullptr
     */
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_TRUE(sm != nullptr);
    /**
     * @tc.steps: step2. add system process remote obejct
     * @tc.expected: step2. add system process remote obejct
     */
    int32_t ret = sm->AddSystemProcess("test_process", nullptr);
    ASSERT_TRUE(ret != ERR_OK);
}

/**
 * @tc.name: AddSystemProcess004
 * @tc.desc: check add system process remoteobject
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityManagerTest, AddSystemProcess004, TestSize.Level1)
{
    DTEST_LOG << " AddSystemProcess004 start " << std::endl;
    /**
     * @tc.steps: step1. get samgr instance
     * @tc.expected: step1. samgr instance not nullptr
     */
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_TRUE(sm != nullptr);
    /**
     * @tc.steps: step2. add system process remote obejct
     * @tc.expected: step2. add system process remote obejct
     */
    for (int32_t i = 0; i < LOOP_SIZE; ++i) {
        sptr<IRemoteObject> ability = sm->AddSystemProcess("", nullptr);
        ASSERT_TRUE(ret != ERR_OK);
    }
}

/**
 * @tc.name: RemoveSystemProcess001
 * @tc.desc: check remove system process remoteobject
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityManagerTest, RemoveSystemProcess001, TestSize.Level1)
{
    DTEST_LOG << " RemoveSystemProcess001 start " << std::endl;
    /**
     * @tc.steps: step1. get samgr instance
     * @tc.expected: step1. samgr instance not nullptr
     */
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    ASSERT_TRUE(sm != nullptr);
    /**
     * @tc.steps: step2. remove system process remote obejct
     * @tc.expected: step2. remove system process remote obejct
     */
    int32_t ret = saMgr->RemoveSystemProcess(nullptr);
    ASSERT_TRUE(ret != ERR_OK);
}

/**
 * @tc.name: RemoveSystemProcess002
 * @tc.desc: check remove system process remoteobject
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityManagerTest, RemoveSystemProcess002, TestSize.Level1)
{
    DTEST_LOG << " RemoveSystemProcess001 star2 " << std::endl;
    /**
     * @tc.steps: step1. get samgr instance
     * @tc.expected: step1. samgr instance not nullptr
     */
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    ASSERT_TRUE(sm != nullptr);
    /**
     * @tc.steps: step2. remove system process remote obejct
     * @tc.expected: step2. remove system process remote obejct
     */
    for (int32_t i = 0; i < LOOP_SIZE; ++i) {
        int32_t ret = saMgr->RemoveSystemProcess(nullptr);
        ASSERT_TRUE(ret != ERR_OK);
    }
}

/**
 * @tc.name: GetSystemAbilityFromRemote001
 * @tc.desc: check get system ability from remote function
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityManagerTest, GetSystemAbilityFromRemote001, TestSize.Level1)
{
    DTEST_LOG << " GetSystemAbilityFromRemote001 start " << std::endl;
    /**
     * @tc.steps: step1. get samgr instance
     * @tc.expected: step1. samgr instance not nullptr
     */
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    ASSERT_TRUE(sm != nullptr);
    /**
     * @tc.steps: step2. GetSystemAbilityFromRemote with invalid said
     * @tc.expected: step2. not found sa
     */
    sptr<IRemoteObject> object = saMgr->GetSystemAbilityFromRemote(-1);
    ASSERT_TRUE(object == nullptr);
}

/**
 * @tc.name: GetSystemAbilityFromRemote002
 * @tc.desc: check get system ability from remote function
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityManagerTest, GetSystemAbilityFromRemote002, TestSize.Level1)
{
    DTEST_LOG << " GetSystemAbilityFromRemote002 start " << std::endl;
    /**
     * @tc.steps: step1. get samgr instance
     * @tc.expected: step1. samgr instance not nullptr
     */
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    ASSERT_TRUE(sm != nullptr);
    /**
     * @tc.steps: step2. GetSystemAbilityFromRemote with invalid said
     * @tc.expected: step2. not found sa
     */
    sptr<IRemoteObject> object = saMgr->GetSystemAbilityFromRemote(0);
    ASSERT_TRUE(object == nullptr);
}

/**
 * @tc.name: GetSystemAbilityFromRemote003
 * @tc.desc: check get system ability from remote function
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityManagerTest, GetSystemAbilityFromRemote003, TestSize.Level1)
{
    DTEST_LOG << " GetSystemAbilityFromRemote003 start " << std::endl;
    /**
     * @tc.steps: step1. get samgr instance
     * @tc.expected: step1. samgr instance not nullptr
     */
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    ASSERT_TRUE(sm != nullptr);
    /**
     * @tc.steps: step2. GetSystemAbilityFromRemote with not-exist said
     * @tc.expected: step2. not found sa
     */
    sptr<IRemoteObject> object = saMgr->GetSystemAbilityFromRemote(DISTRIBUTED_SCHED_TEST_TT_ID);
    ASSERT_TRUE(object == nullptr);
}

/**
 * @tc.name: GetSystemAbilityFromRemote004
 * @tc.desc: check get system ability from remote function
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityManagerTest, GetSystemAbilityFromRemote004, TestSize.Level1)
{
    DTEST_LOG << " GetSystemAbilityFromRemote004 start " << std::endl;
    /**
     * @tc.steps: step1. get samgr instance
     * @tc.expected: step1. samgr instance not nullptr
     */
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    ASSERT_TRUE(sm != nullptr);
    /**
     * @tc.steps: step2. mock add system ability with not distributed
     * @tc.expected: step2. add successfully
     */
    ISystemAbilityManager::SAExtraProp saExtraProp(false, ISystemAbilityManager::DUMP_FLAG_PRIORITY_DEFAULT,
        u"", u"");
    int32_t ret = sm->AddSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID, new TestTransactionService(), saExtraProp);
    ASSERT_TRUE(ret == ERR_OK);
    /**
     * @tc.steps: step3.  GetSystemAbilityFromRemote with mock said
     * @tc.expected: step3. not found sa
     */
    sptr<IRemoteObject> object = saMgr->GetSystemAbilityFromRemote(DISTRIBUTED_SCHED_TEST_TT_ID);
    ASSERT_TRUE(object == nullptr);
    /**
     * @tc.steps: step4. Remove mock sa
     * @tc.expected: step4. Remove mock sa
     */
    saMgr->RemoveSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID);
}

/**
 * @tc.name: GetSystemAbilityFromRemote005
 * @tc.desc: check get system ability from remote function
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityManagerTest, GetSystemAbilityFromRemote005, TestSize.Level1)
{
    DTEST_LOG << " GetSystemAbilityFromRemote005 start " << std::endl;
    /**
     * @tc.steps: step1. get samgr instance
     * @tc.expected: step1. samgr instance not nullptr
     */
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    ASSERT_TRUE(sm != nullptr);
    /**
     * @tc.steps: step2. mock add system ability with not distributed
     * @tc.expected: step2. add successfully
     */
    ISystemAbilityManager::SAExtraProp saExtraProp(true, ISystemAbilityManager::DUMP_FLAG_PRIORITY_DEFAULT,
        u"", u"");
    int32_t ret = sm->AddSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID, new TestTransactionService(), saExtraProp);
    ASSERT_TRUE(ret == ERR_OK);
    /**
     * @tc.steps: step3.  GetSystemAbilityFromRemote with mock said
     * @tc.expected: step3. not found sa
     */
    sptr<IRemoteObject> object = saMgr->GetSystemAbilityFromRemote(DISTRIBUTED_SCHED_TEST_TT_ID);
    ASSERT_TRUE(object != nullptr);
    /**
     * @tc.steps: step4. Remove mock sa
     * @tc.expected: step4. Remove mock sa
     */
    saMgr->RemoveSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID);
}

/**
 * @tc.name: GetSystemAbilityFromRemote006
 * @tc.desc: check get system ability from remote function
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityManagerTest, GetSystemAbilityFromRemote006, TestSize.Level3)
{
    DTEST_LOG << " GetSystemAbilityFromRemote006 start " << std::endl;
    /**
     * @tc.steps: step1. get samgr instance
     * @tc.expected: step1. samgr instance not nullptr
     */
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    ASSERT_TRUE(sm != nullptr);
    /**
     * @tc.steps: step2. GetSystemAbilityFromRemote with invalid said
     * @tc.expected: step2. not found sa
     */
    for (int32_t i = 0; i < LOOP_SIZE; ++i) {
        int32_t ret = saMgr->GetSystemAbilityFromRemote(0);
        ASSERT_TRUE(object == nullptr);
    }
}
}
}