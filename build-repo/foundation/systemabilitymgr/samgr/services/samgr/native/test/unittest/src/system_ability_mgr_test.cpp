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

#include "system_ability_mgr_test.h"
#include "hisysevent_adapter.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "itest_transaction_service.h"
#include "sam_mock_permission.h"
#include "parameter.h"
#include "parameters.h"
#include "sa_profiles.h"
#include "sa_status_change_mock.h"
#include "string_ex.h"
#include "system_ability_definition.h"
#include "test_log.h"

#define private public
#include "system_ability_manager.h"

using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS;

namespace OHOS {
namespace {
constexpr int32_t TEST_VALUE = 2021;
constexpr int32_t TEST_REVERSE_VALUE = 1202;
constexpr int32_t REPEAT = 10;
constexpr int32_t OVERFLOW_TIME = 257;
constexpr int32_t TEST_OVERFLOW_SAID = 99999;
constexpr int32_t TEST_EXCEPTION_HIGH_SA_ID = LAST_SYS_ABILITY_ID + 1;
constexpr int32_t TEST_EXCEPTION_LOW_SA_ID = FIRST_SYS_ABILITY_ID - 1;
constexpr int32_t TEST_SYSTEM_ABILITY1 = 1491;
constexpr int32_t TEST_SYSTEM_ABILITY2 = 1492;
constexpr int32_t SHFIT_BIT = 32;
constexpr int32_t ONDEMAND_SLEEP_TIME = 600 * 1000; // us
constexpr int32_t MAX_COUNT = INT32_MAX - 1000000;

const std::u16string SAMANAGER_INTERFACE_TOKEN = u"ohos.samgr.accessToken";
const string ONDEMAND_PARAM = "persist.samgr.perf.ondemand";
}
void SystemAbilityMgrTest::SetUpTestCase()
{
    DTEST_LOG << "SetUpTestCase" << std::endl;
}

void SystemAbilityMgrTest::TearDownTestCase()
{
    DTEST_LOG << "TearDownTestCase" << std::endl;
}

void SystemAbilityMgrTest::SetUp()
{
    SamMockPermission::MockPermission();
    DTEST_LOG << "SetUp" << std::endl;
}

void SystemAbilityMgrTest::TearDown()
{
    DTEST_LOG << "TearDown" << std::endl;
}

/**
 * @tc.name: AddSystemAbility001
 * @tc.desc: add system ability, input invalid parameter
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, AddSystemAbility001, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    int32_t result = sm->AddSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID, nullptr);
    DTEST_LOG << "add TestTransactionService result = " << result << std::endl;
    EXPECT_TRUE(result != ERR_OK);
}

/**
 * @tc.name: AddSystemAbility002
 * @tc.desc: add system ability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, AddSystemAbility002, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    int32_t result = sm->AddSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID, testAbility);
    DTEST_LOG << "add TestTransactionService result = " << result << std::endl;
    EXPECT_EQ(result, ERR_OK);
    sm->RemoveSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID);
}

/**
 * @tc.name: AddSystemAbility003
 * @tc.desc: add system ability saId exception.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, AddSystemAbility003, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    int32_t result = sm->AddSystemAbility(TEST_EXCEPTION_HIGH_SA_ID, testAbility);
    EXPECT_TRUE(result != ERR_OK);
    result = sm->AddSystemAbility(TEST_EXCEPTION_LOW_SA_ID, testAbility);
    EXPECT_TRUE(result != ERR_OK);
    sm->RemoveSystemAbility(TEST_EXCEPTION_HIGH_SA_ID);
    sm->RemoveSystemAbility(TEST_EXCEPTION_LOW_SA_ID);
}

/**
 * @tc.name: AddSystemAbility004
 * @tc.desc: add system ability with empty capability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, AddSystemAbility004, TestSize.Level1)
{
    int32_t systemAbilityId = DISTRIBUTED_SCHED_TEST_TT_ID;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    ISystemAbilityManager::SAExtraProp saExtraProp(false, ISystemAbilityManager::DUMP_FLAG_PRIORITY_DEFAULT,
        u"", u"");
    int32_t ret = sm->AddSystemAbility(systemAbilityId, new TestTransactionService(), saExtraProp);
    EXPECT_EQ(ret, ERR_OK);
    sm->RemoveSystemAbility(systemAbilityId);
}

/**
 * @tc.name: AddSystemAbility005
 * @tc.desc: add system ability with validated capability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, AddSystemAbility005, TestSize.Level1)
{
    int32_t systemAbilityId = DISTRIBUTED_SCHED_TEST_SO_ID;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    std::u16string capability = u"{\"Capabilities\":{\"aaa\":\"[10.4, 20.5]\",\"bbb\":\"[11, 55]\",\
        \"ccc\":\"this is string\", \"ddd\":\"[aa, bb, cc, dd]\", \"eee\":5.60, \"fff\":4545, \"ggg\":true}}";
    ISystemAbilityManager::SAExtraProp saExtraProp(true, ISystemAbilityManager::DUMP_FLAG_PRIORITY_DEFAULT,
        capability, u"");
    int32_t ret = sm->AddSystemAbility(systemAbilityId, new TestTransactionService(), saExtraProp);
    EXPECT_EQ(ret, ERR_OK);
    sm->RemoveSystemAbility(systemAbilityId);
}

/**
 * @tc.name: RemoveSystemAbility001
 * @tc.desc: remove not exist system ability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, RemoveSystemAbility001, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    int32_t result = sm->RemoveSystemAbility(0);
    EXPECT_TRUE(result != ERR_OK);
}

/**
 * @tc.name: RemoveSystemAbility002
 * @tc.desc: remove system ability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, RemoveSystemAbility002, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    sm->AddSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID, testAbility);
    int32_t result = sm->RemoveSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: GetSystemAbility001
 * @tc.desc: get not exist system ability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, GetSystemAbility001, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    auto ability = sm->GetSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID);
    EXPECT_EQ(ability, nullptr);
}

/**
 * @tc.name: GetSystemAbility002
 * @tc.desc: get system ability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, GetSystemAbility002, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    sm->AddSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID, testAbility);
    auto ability = sm->GetSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID);
    EXPECT_TRUE(ability != nullptr);
    sm->RemoveSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID);
}

/**
 * @tc.name: GetSystemAbility003
 * @tc.desc: get system ability and then transaction.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, GetSystemAbility003, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    sm->AddSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID, testAbility);
    auto ability = sm->GetSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID);
    EXPECT_TRUE(ability != nullptr);
    sptr<ITestTransactionService> targetAblility = iface_cast<ITestTransactionService>(ability);
    EXPECT_TRUE(targetAblility != nullptr);
    int32_t rep = 0;
    int32_t result = targetAblility->ReverseInt(TEST_VALUE, rep);
    DTEST_LOG << "testAbility ReverseInt result = " << result << ", get reply = " << rep << std::endl;
    EXPECT_EQ(rep, TEST_REVERSE_VALUE);
    sm->RemoveSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID);
}

/**
 * @tc.name: GetSystemAbility004
 * @tc.desc: get system ability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, GetSystemAbility004, TestSize.Level2)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    for (int32_t i = 0; i < REPEAT; ++i) {
        auto result = sm->AddSystemAbility((DISTRIBUTED_SCHED_TEST_SO_ID + i), new TestTransactionService());
        EXPECT_EQ(result, ERR_OK);
    }
    for (int32_t i = 0; i < REPEAT; ++i) {
        int32_t saId = DISTRIBUTED_SCHED_TEST_SO_ID + i;
        auto saObject = sm->GetSystemAbility(saId);
        EXPECT_TRUE(saObject != nullptr);
        sm->RemoveSystemAbility(saId);
    }
}

/**
 * @tc.name: GetSystemAbility005
 * @tc.desc: get remote device system ability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, GetSystemAbility005, TestSize.Level2)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    string fakeDeviceId = "fake_dev";
    auto abilityObj = sm->GetSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID, fakeDeviceId);
    EXPECT_EQ(abilityObj, nullptr);
}

/**
 * @tc.name: CheckSystemAbility001
 * @tc.desc: check system ability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, CheckSystemAbility001, TestSize.Level1)
{
    int32_t systemAbilityId = DISTRIBUTED_SCHED_TEST_TT_ID;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    sm->AddSystemAbility(systemAbilityId, testAbility);
    sptr<IRemoteObject> abilityObj = sm->CheckSystemAbility(systemAbilityId);
    EXPECT_TRUE(abilityObj != nullptr);
    sm->RemoveSystemAbility(systemAbilityId);
}

/**
 * @tc.name: CheckOnDemandSystemAbility001
 * @tc.desc: check on demand system ability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, CheckOnDemandSystemAbility001, TestSize.Level1)
{
    int32_t systemAbilityId = DISTRIBUTED_SCHED_TEST_TT_ID;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    sm->AddSystemAbility(systemAbilityId, new TestTransactionService());
    int32_t ret = sm->AddOnDemandSystemAbilityInfo(systemAbilityId, u"test_localmanagername");
    EXPECT_TRUE(ret != ERR_OK);
    sm->RemoveSystemAbility(systemAbilityId);
}

/**
 * @tc.name: CheckOnDemandSystemAbility002
 * @tc.desc: check on demand system ability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, CheckOnDemandSystemAbility002, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    ISystemAbilityManager::SAExtraProp saExtraProp(false, 0, u"", u"");
    int32_t systemAbilityId = DISTRIBUTED_SCHED_TEST_TT_ID;
    int32_t result = sm->AddSystemAbility(systemAbilityId, new TestTransactionService(), saExtraProp);
    EXPECT_EQ(result, ERR_OK);
    sptr<IRemoteObject> saObject = sm->CheckSystemAbility(systemAbilityId);
    result = sm->AddOnDemandSystemAbilityInfo(systemAbilityId, u"just_test");
    EXPECT_TRUE(result != ERR_OK);
    sm->RemoveSystemAbility(systemAbilityId);
}

/**
 * @tc.name: ListSystemAbility001
 * @tc.desc: list all system abilities.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, ListSystemAbility001, TestSize.Level1)
{
    int32_t systemAbilityId = DISTRIBUTED_SCHED_TEST_TT_ID;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    sm->AddSystemAbility(systemAbilityId, new TestTransactionService());
    auto saList = sm->ListSystemAbilities();
    EXPECT_TRUE(!saList.empty());
    auto iter = std::find(saList.begin(), saList.end(), to_utf16(std::to_string(systemAbilityId)));
    EXPECT_TRUE(iter != saList.end());
    sm->RemoveSystemAbility(systemAbilityId);
}

/**
 * @tc.name: LoadSystemAbility001
 * @tc.desc: load system ability with invalid systemAbilityId.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, LoadSystemAbility001, TestSize.Level0)
{
    int32_t systemAbilityId = TEST_EXCEPTION_LOW_SA_ID;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    int32_t result = sm->LoadSystemAbility(systemAbilityId, nullptr);
    EXPECT_TRUE(result != ERR_OK);
}

/**
 * @tc.name: LoadSystemAbility002
 * @tc.desc: load system ability with invalid systemAbilityId.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, LoadSystemAbility002, TestSize.Level0)
{
    int32_t systemAbilityId = TEST_EXCEPTION_HIGH_SA_ID;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    int32_t result = sm->LoadSystemAbility(systemAbilityId, nullptr);
    EXPECT_TRUE(result != ERR_OK);
}

/**
 * @tc.name: LoadSystemAbility003
 * @tc.desc: load system ability with invalid callback.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, LoadSystemAbility003, TestSize.Level0)
{
    int32_t systemAbilityId = DISTRIBUTED_SCHED_TEST_SO_ID;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    int32_t result = sm->LoadSystemAbility(systemAbilityId, nullptr);
    EXPECT_TRUE(result != ERR_OK);
}

/**
 * @tc.name: LoadSystemAbility004
 * @tc.desc: load system ability with not exist systemAbilityId.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, LoadSystemAbility004, TestSize.Level0)
{
    int32_t systemAbilityId = DISTRIBUTED_SCHED_TEST_SO_ID;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    sptr<ISystemAbilityLoadCallback> callback = new SystemAbilityLoadCallbackMock();
    int32_t result = sm->LoadSystemAbility(systemAbilityId, callback);
    EXPECT_TRUE(result != ERR_OK);
}

/**
 * @tc.name: LoadSystemAbility005
 * @tc.desc: test OnRemoteRequest, invalid interface token.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, LoadSystemAbility005, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    int32_t result = saMgr->OnRemoteRequest(ISystemAbilityManager::LOAD_SYSTEM_ABILITY_TRANSACTION,
        data, reply, option);
    EXPECT_TRUE(result != ERR_NONE);
}

/**
 * @tc.name: LoadSystemAbility006
 * @tc.desc: test OnRemoteRequest, invalid systemAbilityId.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, LoadSystemAbility006, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN);
    MessageParcel reply;
    MessageOption option;
    int32_t result = saMgr->OnRemoteRequest(ISystemAbilityManager::LOAD_SYSTEM_ABILITY_TRANSACTION,
        data, reply, option);
    EXPECT_TRUE(result != ERR_NONE);
}

/**
 * @tc.name: LoadSystemAbility007
 * @tc.desc: test OnRemoteRequest, invalid systemAbilityId.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, LoadSystemAbility007, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN);
    data.WriteInt32(TEST_EXCEPTION_HIGH_SA_ID);
    MessageParcel reply;
    MessageOption option;
    int32_t result = saMgr->OnRemoteRequest(ISystemAbilityManager::LOAD_SYSTEM_ABILITY_TRANSACTION,
        data, reply, option);
    EXPECT_TRUE(result != ERR_NONE);
}

/**
 * @tc.name: LoadSystemAbility008
 * @tc.desc: test OnRemoteRequest, null callback.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, LoadSystemAbility008, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN);
    data.WriteInt32(DISTRIBUTED_SCHED_TEST_SO_ID);
    MessageParcel reply;
    MessageOption option;
    int32_t result = saMgr->OnRemoteRequest(ISystemAbilityManager::LOAD_SYSTEM_ABILITY_TRANSACTION,
        data, reply, option);
    EXPECT_TRUE(result != ERR_NONE);
}

/**
 * @tc.name: LoadSystemAbility009
 * @tc.desc: test OnRemoteRequest, not exist systemAbilityId.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, LoadSystemAbility009, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN);
    data.WriteInt32(DISTRIBUTED_SCHED_TEST_SO_ID);
    sptr<ISystemAbilityLoadCallback> callback = new SystemAbilityLoadCallbackMock();
    data.WriteRemoteObject(callback->AsObject());
    MessageParcel reply;
    MessageOption option;
    int32_t result = saMgr->OnRemoteRequest(ISystemAbilityManager::LOAD_SYSTEM_ABILITY_TRANSACTION,
        data, reply, option);
    EXPECT_TRUE(result != ERR_NONE);
}

/**
 * @tc.name: OnRemoteDied001
 * @tc.desc: test OnRemoteDied, remove registered callback.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, OnRemoteDied001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    sptr<ISystemAbilityLoadCallback> callback = new SystemAbilityLoadCallbackMock();
    saMgr->OnAbilityCallbackDied(callback->AsObject());
    EXPECT_TRUE(saMgr->startingAbilityMap_.empty());
}

/**
 * @tc.name: StartOnDemandAbility001
 * @tc.desc: test StartOnDemandAbility, invalid systemAbilityId.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, StartOnDemandAbility001, TestSize.Level0)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    int32_t result = saMgr->StartOnDemandAbility(TEST_EXCEPTION_LOW_SA_ID);
    EXPECT_TRUE(result != ERR_NONE);
}

/**
 * @tc.name: StartOnDemandAbility002
 * @tc.desc: test StartOnDemandAbility, invalid systemAbilityId.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, StartOnDemandAbility002, TestSize.Level0)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    int32_t result = saMgr->StartOnDemandAbility(TEST_EXCEPTION_HIGH_SA_ID);
    EXPECT_TRUE(result != ERR_NONE);
}

/**
 * @tc.name: StartOnDemandAbility003
 * @tc.desc: test StartOnDemandAbility, not exist systemAbilityId.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, StartOnDemandAbility003, TestSize.Level0)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    int32_t result = saMgr->StartOnDemandAbility(DISTRIBUTED_SCHED_TEST_SO_ID);
    EXPECT_TRUE(result != ERR_NONE);
}

/**
 * @tc.name: StartOnDemandAbility004
 * @tc.desc: test StartOnDemandAbility, not on-demand systemAbilityId.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, StartOnDemandAbility004, TestSize.Level0)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    int32_t result = saMgr->StartOnDemandAbility(DISTRIBUTED_SCHED_SA_ID);
    EXPECT_TRUE(result != ERR_NONE);
}

/**
 * @tc.name: AddOnDemandSystemAbilityInfo001
 * @tc.desc: test AddOnDemandSystemAbilityInfo, invalid systemAbilityId.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, AddOnDemandSystemAbilityInfo001, TestSize.Level0)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    int32_t result = saMgr->AddOnDemandSystemAbilityInfo(TEST_EXCEPTION_LOW_SA_ID, u"");
    EXPECT_TRUE(result != ERR_NONE);
}

/**
 * @tc.name: AddOnDemandSystemAbilityInfo002
 * @tc.desc: test AddOnDemandSystemAbilityInfo, invalid systemAbilityId.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, AddOnDemandSystemAbilityInfo002, TestSize.Level0)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    int32_t result = saMgr->AddOnDemandSystemAbilityInfo(TEST_EXCEPTION_HIGH_SA_ID, u"");
    EXPECT_TRUE(result != ERR_NONE);
}

/**
 * @tc.name: AddOnDemandSystemAbilityInfo003
 * @tc.desc: test AddOnDemandSystemAbilityInfo, invalid procName.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, AddOnDemandSystemAbilityInfo003, TestSize.Level0)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    int32_t result = saMgr->AddOnDemandSystemAbilityInfo(DISTRIBUTED_SCHED_TEST_SO_ID, u"");
    EXPECT_TRUE(result != ERR_NONE);
}

/**
 * @tc.name: AddOnDemandSystemAbilityInfo004
 * @tc.desc: test AddOnDemandSystemAbilityInfo, procName not registered.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, AddOnDemandSystemAbilityInfo004, TestSize.Level0)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    int32_t result = saMgr->AddOnDemandSystemAbilityInfo(DISTRIBUTED_SCHED_TEST_SO_ID, u"fake_process_name");
    EXPECT_TRUE(result != ERR_NONE);
}

/**
 * @tc.name: OnLoadSystemAbilitySuccess001
 * @tc.desc: test OnLoadSystemAbilitySuccess, null callback.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, OnLoadSystemAbilitySuccess001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    saMgr->NotifySystemAbilityLoaded(DISTRIBUTED_SCHED_TEST_SO_ID, nullptr, nullptr);
    EXPECT_TRUE(callback->GetSystemAbilityId() == 0);
}

/**
 * @tc.name: OnLoadSystemAbilitySuccess002
 * @tc.desc: test OnLoadSystemAbilitySuccess, null IRemoteObject.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, OnLoadSystemAbilitySuccess002, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    saMgr->NotifySystemAbilityLoaded(DISTRIBUTED_SCHED_TEST_SO_ID, nullptr, callback);
    EXPECT_TRUE(callback->GetSystemAbilityId() == DISTRIBUTED_SCHED_TEST_SO_ID);
    EXPECT_TRUE(callback->GetRemoteObject() == nullptr);
}

/**
 * @tc.name: OnLoadSystemAbilitySuccess003
 * @tc.desc: test OnLoadSystemAbilitySuccess.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, OnLoadSystemAbilitySuccess003, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    sptr<IRemoteObject> remoteObject = new TestTransactionService();
    saMgr->NotifySystemAbilityLoaded(DISTRIBUTED_SCHED_TEST_SO_ID, remoteObject, callback);
    EXPECT_TRUE(callback->GetSystemAbilityId() == DISTRIBUTED_SCHED_TEST_SO_ID);
    EXPECT_TRUE(callback->GetRemoteObject() == remoteObject);
}

/**
 * @tc.name: ReportSubscribeOverflow001
 * @tc.desc: ReportSubscribeOverflow001
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, ReportSubscribeOverflow001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    ASSERT_TRUE(saMgr != nullptr);
    std::vector<sptr<SaStatusChangeMock>> tmpCallbak;
    sptr<SaStatusChangeMock> callback = nullptr;
    for (int i = 0; i < OVERFLOW_TIME; ++i) {
        callback = new SaStatusChangeMock();
        tmpCallbak.emplace_back(callback);
        saMgr->SubscribeSystemAbility(TEST_OVERFLOW_SAID, callback);
    }
    for (const auto& callback : tmpCallbak) {
        saMgr->UnSubscribeSystemAbility(TEST_OVERFLOW_SAID, callback);
    }
}

/**
 * @tc.name: ReportLoadSAOverflow001
 * @tc.desc: ReportLoadSAOverflow001
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, ReportLoadSAOverflow001, TestSize.Level1)
{
    sptr<ISystemAbilityManager> saMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_TRUE(saMgr != nullptr);
    for (int i = 0; i < OVERFLOW_TIME; ++i) {
        sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
        saMgr->LoadSystemAbility(TEST_OVERFLOW_SAID, callback);
    }
}

/**
 * @tc.name: LoadRemoteSystemAbility001
 * @tc.desc: load system ability with invalid systemAbilityId.
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, LoadRemoteSystemAbility001, TestSize.Level2)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    int32_t systemAbilityId = TEST_EXCEPTION_LOW_SA_ID;
    std::string deviceId = "";
    int32_t result = sm->LoadSystemAbility(systemAbilityId, deviceId, nullptr);
    EXPECT_TRUE(result != ERR_OK);
}

/**
 * @tc.name: LoadRemoteSystemAbility002
 * @tc.desc: load system ability with invalid systemAbilityId.
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, LoadRemoteSystemAbility002, TestSize.Level2)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    int32_t systemAbilityId = TEST_EXCEPTION_LOW_SA_ID;
    std::string deviceId = "123456789";
    int32_t result = sm->LoadSystemAbility(systemAbilityId, deviceId, nullptr);
    EXPECT_TRUE(result != ERR_OK);
}

/**
 * @tc.name: LoadRemoteSystemAbility002
 * @tc.desc: load system ability with invalid systemAbilityId.
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, LoadRemoteSystemAbility003, TestSize.Level2)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    int32_t systemAbilityId = -1;
    std::string deviceId = "123456789";
    int32_t result = sm->LoadSystemAbility(systemAbilityId, deviceId, nullptr);
    EXPECT_TRUE(result != ERR_OK);
}

/**
 * @tc.name: LoadRemoteSystemAbility004
 * @tc.desc: load system ability with invalid systemAbilityId.
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, LoadRemoteSystemAbility004, TestSize.Level2)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    int32_t systemAbilityId = 0;
    std::string deviceId = "123456789";
    int32_t result = sm->LoadSystemAbility(systemAbilityId, deviceId, nullptr);
    EXPECT_TRUE(result != ERR_OK);
}

/**
 * @tc.name: LoadRemoteSystemAbility004
 * @tc.desc: load system ability with invalid systemAbilityId.
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, LoadRemoteSystemAbility005, TestSize.Level2)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    int32_t systemAbilityId = 0;
    std::string deviceId = "";
    int32_t result = sm->LoadSystemAbility(systemAbilityId, deviceId, nullptr);
    EXPECT_TRUE(result != ERR_OK);
}

/**
 * @tc.name: LoadRemoteSystemAbility004
 * @tc.desc: load system ability with invalid systemAbilityId.
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, LoadRemoteSystemAbility006, TestSize.Level2)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    int32_t systemAbilityId = -1;
    std::string deviceId = "";
    int32_t result = sm->LoadSystemAbility(systemAbilityId, deviceId, nullptr);
    EXPECT_TRUE(result != ERR_OK);
}

/**
 * @tc.name: LoadSystemAbilityFromRpc001
 * @tc.desc: load system ability with invalid systemAbilityId.
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, LoadSystemAbilityFromRpc001, TestSize.Level2)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    std::string deviceId = "";
    int32_t systemAbilityId = -1;
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    bool ret = saMgr->LoadSystemAbilityFromRpc(deviceId, systemAbilityId, callback);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: LoadSystemAbilityFromRpc002
 * @tc.desc: load system ability with invalid systemAbilityId.
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, LoadSystemAbilityFromRpc002, TestSize.Level2)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    std::string deviceId = "";
    int32_t systemAbilityId = 0;
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    bool ret = saMgr->LoadSystemAbilityFromRpc(deviceId, systemAbilityId, callback);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: LoadSystemAbilityFromRpc003
 * @tc.desc: load system ability with invalid systemAbilityId.
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, LoadSystemAbilityFromRpc003, TestSize.Level2)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    std::string deviceId = "";
    int32_t systemAbilityId = 0;
    bool ret = saMgr->LoadSystemAbilityFromRpc(deviceId, systemAbilityId, nullptr);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: LoadSystemAbilityFromRpc004
 * @tc.desc: load system ability with invalid systemAbilityId.
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, LoadSystemAbilityFromRpc004, TestSize.Level2)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    std::string deviceId = "1111111";
    int32_t systemAbilityId = 0;
    bool ret = saMgr->LoadSystemAbilityFromRpc(deviceId, systemAbilityId, nullptr);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: DoMakeRemoteBinder001
 * @tc.desc: load system ability with invalid systemAbilityId.
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, DoMakeRemoteBinder001, TestSize.Level2)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    std::string deviceId = "1111111";
    int32_t systemAbilityId = 0;
    auto remoteObject = saMgr->DoMakeRemoteBinder(systemAbilityId, 0, 0, deviceId);
    EXPECT_TRUE(remoteObject == nullptr);
}

/**
 * @tc.name: startingAbilityMap_ test
 * @tc.desc: startingAbilityMap_ init
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, startingAbilityMapTest001, TestSize.Level1)
{
    DTEST_LOG << " startingAbilityMapTest001 start " << std::endl;
    /**
     * @tc.steps: step1. init startingAbilityMap_
     * @tc.expected: step1. init startingAbilityMap_
     */
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->startingAbilityMap_.clear();

    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback3 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback4 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback5 = new SystemAbilityLoadCallbackMock();

    std::map<std::string, SystemAbilityManager::CallbackList> mockCallbackMap1 = {
        {"111111", {{mockLoadCallback1, 0}}},
        {"222222", {{mockLoadCallback1, 0}, {mockLoadCallback2, 0}}},
        {"333333", {{mockLoadCallback2, 0}, {mockLoadCallback3, 1}}}
    };
    std::map<std::string, SystemAbilityManager::CallbackList> mockCallbackMap2 = {
        {"111111", {{mockLoadCallback1, 0}}},
        {"222222", {{mockLoadCallback1, 0}, {mockLoadCallback2, 0}}},
        {"333333", {{mockLoadCallback2, 0}, {mockLoadCallback3, 1}}}
    };
    std::map<std::string, SystemAbilityManager::CallbackList> mockCallbackMap3 = {
        {"111111", {{mockLoadCallback2, 0}}},
        {"222222", {{mockLoadCallback3, 0}, {mockLoadCallback2, 0}}},
        {"333333", {{mockLoadCallback4, 0}, {mockLoadCallback5, 1}}}
    };
    SystemAbilityManager::AbilityItem mockAbilityItem1 = {
        SystemAbilityManager::AbilityState::INIT, mockCallbackMap1
    };
    SystemAbilityManager::AbilityItem mockAbilityItem2 = {
        SystemAbilityManager::AbilityState::INIT, mockCallbackMap2
    };

    saMgr->startingAbilityMap_.emplace(TEST_SYSTEM_ABILITY2, mockAbilityItem1);
    ASSERT_EQ(saMgr->startingAbilityMap_.size(), 1);
    saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY1] = mockAbilityItem1;
    ASSERT_TRUE(saMgr->startingAbilityMap_.size() > 1);
}

/**
 * @tc.name: startingAbilityMap_ test
 * @tc.desc: test for callback dead, with one device, one callback
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, startingAbilityMapTest002, TestSize.Level1)
{
    DTEST_LOG << " startingAbilityMapTest002 start " << std::endl;
    /**
     * @tc.steps: step1. init startingAbilityMap_
     * @tc.expected: step1. init startingAbilityMap_
     */
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->startingAbilityMap_.clear();

    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();

    std::map<std::string, SystemAbilityManager::CallbackList> mockCallbackMap1 = {
        {"111111", {{mockLoadCallback1, 0}}}
    };
    SystemAbilityManager::AbilityItem mockAbilityItem1 = {
        SystemAbilityManager::AbilityState::INIT, mockCallbackMap1
    };

    saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY1] = mockAbilityItem1;

    saMgr->OnAbilityCallbackDied(mockLoadCallback1->AsObject());
    ASSERT_EQ(saMgr->startingAbilityMap_.size(), 0);
}

/**
 * @tc.name: startingAbilityMap_ test
 * @tc.desc: test for callback dead, with one device, some callback
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, startingAbilityMapTest003, TestSize.Level1)
{
    DTEST_LOG << " startingAbilityMapTest003 start " << std::endl;
    /**
     * @tc.steps: step1. init startingAbilityMap_
     * @tc.expected: step1. init startingAbilityMap_
     */
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->startingAbilityMap_.clear();

    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();

    std::map<std::string, SystemAbilityManager::CallbackList> mockCallbackMap1 = {
        {"111111", {{mockLoadCallback1, 0}, {mockLoadCallback2, 1}}}
    };
    SystemAbilityManager::AbilityItem mockAbilityItem1 = {
        SystemAbilityManager::AbilityState::INIT, mockCallbackMap1
    };

    saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY1] = mockAbilityItem1;
    saMgr->OnAbilityCallbackDied(mockLoadCallback1->AsObject());
    ASSERT_EQ(saMgr->startingAbilityMap_.size(), 1);
    ASSERT_EQ(saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY1].callbackMap["111111"].size(), 1);
}

/**
 * @tc.name: startingAbilityMap_ test
 * @tc.desc: test for callback dead, with no registered callback
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, startingAbilityMapTest004, TestSize.Level1)
{
    DTEST_LOG << " startingAbilityMapTest004 start " << std::endl;
    /**
     * @tc.steps: step1. init startingAbilityMap_
     * @tc.expected: step1. init startingAbilityMap_
     */
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->startingAbilityMap_.clear();

    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();

    std::map<std::string, SystemAbilityManager::CallbackList> mockCallbackMap1 = {
        {"111111", {{mockLoadCallback1, 1}}}
    };
    SystemAbilityManager::AbilityItem mockAbilityItem1 = {
        SystemAbilityManager::AbilityState::INIT, mockCallbackMap1
    };

    saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY1] = mockAbilityItem1;
    saMgr->OnAbilityCallbackDied(mockLoadCallback2->AsObject());
    ASSERT_EQ(saMgr->startingAbilityMap_.size(), 1);
    ASSERT_EQ(saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY1].callbackMap["111111"].size(), 1);
}

/**
 * @tc.name: startingAbilityMap_ test
 * @tc.desc: test for callback dead, with some device, some callback
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, startingAbilityMapTest005, TestSize.Level1)
{
    DTEST_LOG << " startingAbilityMapTest004 start " << std::endl;
    /**
     * @tc.steps: step1. init startingAbilityMap_
     * @tc.expected: step1. init startingAbilityMap_
     */
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->startingAbilityMap_.clear();

    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();
    std::map<std::string, SystemAbilityManager::CallbackList> mockCallbackMap1 = {
        {"111111", {{mockLoadCallback1, 1}}},
        {"222222", {{mockLoadCallback2, 1}}}
    };
    SystemAbilityManager::AbilityItem mockAbilityItem1 = {
        SystemAbilityManager::AbilityState::INIT, mockCallbackMap1
    };

    saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY1] = mockAbilityItem1;
    saMgr->OnAbilityCallbackDied(mockLoadCallback2->AsObject());
    ASSERT_EQ(saMgr->startingAbilityMap_.size(), 1);
    ASSERT_EQ(saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY1].callbackMap.size(), 1);
    ASSERT_EQ(saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY1].callbackMap["111111"].size(), 1);
}

/**
 * @tc.name: startingAbilityMap_ test
 * @tc.desc: test for callback dead, with some device, one callback
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, startingAbilityMapTest006, TestSize.Level1)
{
    DTEST_LOG << " startingAbilityMapTest006 start " << std::endl;
    /**
     * @tc.steps: step1. init startingAbilityMap_
     * @tc.expected: step1. init startingAbilityMap_
     */
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->startingAbilityMap_.clear();

    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();

    std::map<std::string, SystemAbilityManager::CallbackList> mockCallbackMap1 = {
        {"111111", {{mockLoadCallback1, 1}}},
        {"222222", {{mockLoadCallback1, 0}}}
    };
    SystemAbilityManager::AbilityItem mockAbilityItem1 = {
        SystemAbilityManager::AbilityState::INIT, mockCallbackMap1
    };

    saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY1] = mockAbilityItem1;
    saMgr->OnAbilityCallbackDied(mockLoadCallback1->AsObject());
    ASSERT_EQ(saMgr->startingAbilityMap_.size(), 0);
}

/**
 * @tc.name: startingAbilityMap_ test
 * @tc.desc: test for callback dead, with one device, some callback, some sa
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, startingAbilityMapTest007, TestSize.Level1)
{
    DTEST_LOG << " startingAbilityMapTest007 start " << std::endl;
    /**
     * @tc.steps: step1. init startingAbilityMap_
     * @tc.expected: step1. init startingAbilityMap_
     */
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->startingAbilityMap_.clear();

    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();

    std::map<std::string, SystemAbilityManager::CallbackList> mockCallbackMap1 = {
        {"111111", {{mockLoadCallback1, 1}, {mockLoadCallback2, 1}}}
    };
    SystemAbilityManager::AbilityItem mockAbilityItem1 = {
        SystemAbilityManager::AbilityState::INIT, mockCallbackMap1
    };

    saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY1] = mockAbilityItem1;
    saMgr->OnAbilityCallbackDied(mockLoadCallback1->AsObject());
    ASSERT_EQ(saMgr->startingAbilityMap_.size(), 1);
    ASSERT_EQ(saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY1].callbackMap.size(), 1);
}

/**
 * @tc.name: startingAbilityMap_ test
 * @tc.desc: test for callback dead, with one device, some callback
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, startingAbilityMapTest008, TestSize.Level1)
{
    DTEST_LOG << " startingAbilityMapTest007 start " << std::endl;
    /**
     * @tc.steps: step1. init startingAbilityMap_
     * @tc.expected: step1. init startingAbilityMap_
     */
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->startingAbilityMap_.clear();

    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();

    std::map<std::string, SystemAbilityManager::CallbackList> mockCallbackMap1 = {
        {"111111", {{mockLoadCallback1, 1}}}
    };
    SystemAbilityManager::AbilityItem mockAbilityItem1 = {
        SystemAbilityManager::AbilityState::INIT, mockCallbackMap1
    };

    saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY1] = mockAbilityItem1;
    saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY2] = mockAbilityItem1;
    saMgr->OnAbilityCallbackDied(mockLoadCallback1->AsObject());
    ASSERT_EQ(saMgr->startingAbilityMap_.size(), 0);
    saMgr->OnAbilityCallbackDied(mockLoadCallback2->AsObject());
    ASSERT_EQ(saMgr->startingAbilityMap_.size(), 0);
}

/**
 * @tc.name: startingAbilityMap_ test
 * @tc.desc: test for callback dead, with one device, some callback, some sa
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, startingAbilityMapTest009, TestSize.Level1)
{
    DTEST_LOG << " startingAbilityMapTest009 start " << std::endl;
    /**
     * @tc.steps: step1. init startingAbilityMap_
     * @tc.expected: step1. init startingAbilityMap_
     */
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->startingAbilityMap_.clear();

    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();

    std::map<std::string, SystemAbilityManager::CallbackList> mockCallbackMap1 = {
        {"111111", {{mockLoadCallback1, 1}, {mockLoadCallback2, 1}}}
    };
    SystemAbilityManager::AbilityItem mockAbilityItem1 = {
        SystemAbilityManager::AbilityState::INIT, mockCallbackMap1
    };

    saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY1] = mockAbilityItem1;
    saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY2] = mockAbilityItem1;
    saMgr->OnAbilityCallbackDied(mockLoadCallback1->AsObject());
    ASSERT_TRUE(saMgr->startingAbilityMap_.size() > 1);
    ASSERT_EQ(saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY1].callbackMap.size(), 1);
}

/**
 * @tc.name: startingAbilityMap_ test
 * @tc.desc: test for callback dead, with some device, some callback, some sa
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, startingAbilityMapTest010, TestSize.Level1)
{
    DTEST_LOG << " startingAbilityMapTest010 start " << std::endl;
    /**
     * @tc.steps: step1. init startingAbilityMap_
     * @tc.expected: step1. init startingAbilityMap_
     */
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->startingAbilityMap_.clear();

    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();

    std::map<std::string, SystemAbilityManager::CallbackList> mockCallbackMap1 = {
        {"111111", {{mockLoadCallback1, 1}}}
    };

    std::map<std::string, SystemAbilityManager::CallbackList> mockCallbackMap2 = {
        {"111111", {{mockLoadCallback1, 1}}}
    };
    SystemAbilityManager::AbilityItem mockAbilityItem1 = {
        SystemAbilityManager::AbilityState::INIT, mockCallbackMap1
    };
    SystemAbilityManager::AbilityItem mockAbilityItem2 = {
        SystemAbilityManager::AbilityState::INIT, mockCallbackMap2
    };

    saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY1] = mockAbilityItem1;
    saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY2] = mockAbilityItem2;
    saMgr->OnAbilityCallbackDied(mockLoadCallback1->AsObject());
    ASSERT_EQ(saMgr->startingAbilityMap_.size(), 0);
    saMgr->OnAbilityCallbackDied(mockLoadCallback2->AsObject());
    ASSERT_EQ(saMgr->startingAbilityMap_.size(), 0);
}

/**
 * @tc.name: startingAbilityMap_ test
 * @tc.desc: test for callback dead, with one device, some callback, some sa
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, startingAbilityMapTest011, TestSize.Level1)
{
    DTEST_LOG << " startingAbilityMapTest010 start " << std::endl;
    /**
     * @tc.steps: step1. init startingAbilityMap_
     * @tc.expected: step1. init startingAbilityMap_
     */
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->startingAbilityMap_.clear();

    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();

    std::map<std::string, SystemAbilityManager::CallbackList> mockCallbackMap1 = {
        {"111111", {{mockLoadCallback1, 1}}}
    };

    std::map<std::string, SystemAbilityManager::CallbackList> mockCallbackMap2 = {
        {"111111", {{mockLoadCallback2, 1}}}
    };
    SystemAbilityManager::AbilityItem mockAbilityItem1 = {
        SystemAbilityManager::AbilityState::INIT, mockCallbackMap1
    };

    SystemAbilityManager::AbilityItem mockAbilityItem2 = {
        SystemAbilityManager::AbilityState::INIT, mockCallbackMap2
    };

    saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY1] = mockAbilityItem1;
    saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY2] = mockAbilityItem2;
    saMgr->OnAbilityCallbackDied(mockLoadCallback1->AsObject());
    ASSERT_EQ(saMgr->startingAbilityMap_.size(), 1);
    saMgr->OnAbilityCallbackDied(mockLoadCallback2->AsObject());
    ASSERT_EQ(saMgr->startingAbilityMap_.size(), 0);
}

/**
 * @tc.name: startingAbilityMap_ test
 * @tc.desc: test for callback dead, with one device, some callback, some sa
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, startingAbilityMapTest012, TestSize.Level1)
{
    DTEST_LOG << " startingAbilityMapTest010 start " << std::endl;
    /**
     * @tc.steps: step1. init startingAbilityMap_
     * @tc.expected: step1. init startingAbilityMap_
     */
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->startingAbilityMap_.clear();

    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();

    std::map<std::string, SystemAbilityManager::CallbackList> mockCallbackMap1 = {
        {"222222", {{mockLoadCallback1, 0}}},
        {"111111", {{mockLoadCallback1, 1}}}
    };

    std::map<std::string, SystemAbilityManager::CallbackList> mockCallbackMap2 = {
        {"22222", {{mockLoadCallback2, 1}}}
    };
    SystemAbilityManager::AbilityItem mockAbilityItem1 = {
        SystemAbilityManager::AbilityState::INIT, mockCallbackMap1
    };

    SystemAbilityManager::AbilityItem mockAbilityItem2 = {
        SystemAbilityManager::AbilityState::INIT, mockCallbackMap2
    };

    saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY1] = mockAbilityItem1;
    saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY2] = mockAbilityItem2;
    saMgr->OnAbilityCallbackDied(mockLoadCallback1->AsObject());
    ASSERT_EQ(saMgr->startingAbilityMap_.size(), 1);
    saMgr->OnAbilityCallbackDied(mockLoadCallback2->AsObject());
    ASSERT_EQ(saMgr->startingAbilityMap_.size(), 0);
}

/**
 * @tc.name: OnRemoteCallbackDied001 test
 * @tc.desc: test for callback dead, with one device, some callback
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, OnRemoteCallbackDied001, TestSize.Level1)
{
    DTEST_LOG << " OnRemoteCallbackDied001 start " << std::endl;
    /**
     * @tc.steps: step1. init remoteCallbacks_
     * @tc.expected: step1. init remoteCallbacks_
     */
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();

    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();
    saMgr->remoteCallbacks_.clear();
    saMgr->remoteCallbacks_ = {
        {"11111", {mockLoadCallback1, mockLoadCallback2}}
    };
    /**
     * @tc.steps: step2. remove nullptr
     * @tc.expected: step2. remove nothing and not crash
     */
    saMgr->OnAbilityCallbackDied(nullptr);
    ASSERT_EQ(saMgr->remoteCallbacks_.size(), 1);
}

/**
 * @tc.name: OnRemoteCallbackDied002 test
 * @tc.desc: test for callback dead, with one device, some callback
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, OnRemoteCallbackDied002, TestSize.Level1)
{
    DTEST_LOG << " OnRemoteCallbackDied002 start " << std::endl;
    /**
     * @tc.steps: step1. init remoteCallbacks_ with one device and one callback
     * @tc.expected: step1. init remoteCallbacks_
     */
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->remoteCallbacks_.clear();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    saMgr->remoteCallbacks_ = {
        {"11111", {mockLoadCallback1}}
    };
    /**
     * @tc.steps: step2. remove one callback
     * @tc.expected: step2. remoteCallbacks_ size 0
     */
    saMgr->OnRemoteCallbackDied(mockLoadCallback1->AsObject());
    ASSERT_EQ(saMgr->remoteCallbacks_.size(), 0);
}

/**
 * @tc.name: OnRemoteCallbackDied003 test
 * @tc.desc: test for callback dead, with one device, some callback
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, OnRemoteCallbackDied003, TestSize.Level1)
{
    DTEST_LOG << " OnRemoteCallbackDied003 start " << std::endl;
    /**
     * @tc.steps: step1. init remoteCallbacks_ with one device and one callback
     * @tc.expected: step1. init remoteCallbacks_
     */
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->remoteCallbacks_.clear();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();
    saMgr->remoteCallbacks_ = {
        {"11111", {mockLoadCallback1}}
    };
    /**
     * @tc.steps: step2. remove other callback
     * @tc.expected: step2. remove nothing
     */
    saMgr->OnRemoteCallbackDied(mockLoadCallback2->AsObject());
    ASSERT_EQ(saMgr->remoteCallbacks_.size(), 1);
}

/**
 * @tc.name: OnRemoteCallbackDied004 test
 * @tc.desc: test for callback dead, with one device, some callback
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, OnRemoteCallbackDied004, TestSize.Level1)
{
    DTEST_LOG << " OnRemoteCallbackDied004 start " << std::endl;
    /**
     * @tc.steps: step1. init remoteCallbacks_
     * @tc.expected: step1. init remoteCallbacks_
     */
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->remoteCallbacks_.clear();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();

    saMgr->remoteCallbacks_ = {
        {"11111", {mockLoadCallback1, mockLoadCallback2}}
    };
    /**
     * @tc.steps: step2. remove one callback
     * @tc.expected: step2. remoteCallbacks_ size 1
     */
    saMgr->OnRemoteCallbackDied(mockLoadCallback1->AsObject());
    ASSERT_EQ(saMgr->remoteCallbacks_["11111"].size(), 1);
    ASSERT_EQ(saMgr->remoteCallbacks_.size(), 1);
}

/**
 * @tc.name: OnRemoteCallbackDied005 test
 * @tc.desc: test for callback dead, with one device, some callback
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, OnRemoteCallbackDied005, TestSize.Level1)
{
    DTEST_LOG << " OnRemoteCallbackDied005 start " << std::endl;
    /**
     * @tc.steps: step1. init remoteCallbacks_
     * @tc.expected: step1. init remoteCallbacks_
     */
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->remoteCallbacks_.clear();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();

    saMgr->remoteCallbacks_ = {
        {"11111", {mockLoadCallback1, mockLoadCallback2}}
    };
    /**
     * @tc.steps: step2. remove all callback
     * @tc.expected: step2. remoteCallbacks_ empty
     */
    saMgr->OnRemoteCallbackDied(mockLoadCallback1->AsObject());
    saMgr->OnRemoteCallbackDied(mockLoadCallback2->AsObject());
    ASSERT_EQ(saMgr->remoteCallbacks_.size(), 0);
}

/**
 * @tc.name: OnRemoteCallbackDied006 test
 * @tc.desc: test for callback dead, with one device, some callback
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, OnRemoteCallbackDied006, TestSize.Level1)
{
    DTEST_LOG << " OnRemoteCallbackDied006 start " << std::endl;
    /**
     * @tc.steps: step1. init remoteCallbacks_
     * @tc.expected: step1. init remoteCallbacks_
     */
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->remoteCallbacks_.clear();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();

    saMgr->remoteCallbacks_ = {
        {"11111", {mockLoadCallback1}},
        {"22222", {mockLoadCallback2}}
    };
    /**
     * @tc.steps: step2. remove all callback
     * @tc.expected: step2. remoteCallbacks_ empty
     */
    saMgr->OnRemoteCallbackDied(mockLoadCallback1->AsObject());
    ASSERT_EQ(saMgr->remoteCallbacks_["22222"].size(), 1);
    ASSERT_EQ(saMgr->remoteCallbacks_.size(), 1);
}

/**
 * @tc.name: OnRemoteCallbackDied007 test
 * @tc.desc: test for callback dead, with one device, some callback
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, OnRemoteCallbackDied007, TestSize.Level1)
{
    DTEST_LOG << " OnRemoteCallbackDied007 start " << std::endl;
    /**
     * @tc.steps: step1. init remoteCallbacks_
     * @tc.expected: step1. init remoteCallbacks_
     */
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->remoteCallbacks_.clear();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();

    saMgr->remoteCallbacks_ = {
        {"11111", {mockLoadCallback1, mockLoadCallback2}},
        {"22222", {mockLoadCallback2}}
    };
    /**
     * @tc.steps: step2. remove mockLoadCallback1
     * @tc.expected: step2. remoteCallbacks_ empty
     */
    saMgr->OnRemoteCallbackDied(mockLoadCallback1->AsObject());
    ASSERT_TRUE(saMgr->remoteCallbacks_.size() > 1);
}

/**
 * @tc.name: OnRemoteCallbackDied008 test
 * @tc.desc: test for callback dead, with one device, some callback
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, OnRemoteCallbackDied008, TestSize.Level1)
{
    DTEST_LOG << " OnRemoteCallbackDied008 start " << std::endl;
    /**
     * @tc.steps: step1. init remoteCallbacks_
     * @tc.expected: step1. init remoteCallbacks_
     */
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->remoteCallbacks_.clear();

    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();

    saMgr->remoteCallbacks_ = {
        {"11111", {mockLoadCallback1, mockLoadCallback2}},
        {"22222", {mockLoadCallback2}}
    };
    /**
     * @tc.steps: step2. remove one mockLoadCallback2
     * @tc.expected: step2. remoteCallbacks_ remove all mockLoadCallback2
     */
    saMgr->OnRemoteCallbackDied(mockLoadCallback2->AsObject());
    ASSERT_EQ(saMgr->remoteCallbacks_.size(), 1);
}

/**
 * @tc.name: OnRemoteCallbackDied008 test
 * @tc.desc: test for callback dead, with one device, some callback
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, DoLoadRemoteSystemAbility001, TestSize.Level1)
{
    DTEST_LOG << " DoLoadRemoteSystemAbility001 start " << std::endl;
    /**
     * @tc.steps: step1. init remoteCallbacks_
     * @tc.expected: step1. init remoteCallbacks_
     */
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->remoteCallbacks_.clear();

    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();

    saMgr->remoteCallbacks_ = {
        {"11111_111", {mockLoadCallback1, mockLoadCallback2}},
        {"11111_222", {mockLoadCallback2}}
    };
    /**
     * @tc.steps: step2. mockLoadCallback1 load complete
     * @tc.expected: step2. remoteCallbacks_ remove mockLoadCallback1
     */
    saMgr->DoLoadRemoteSystemAbility(11111, 0, 0, "111", mockLoadCallback1);
    ASSERT_EQ(saMgr->remoteCallbacks_["11111_111"].size(), 1);
    ASSERT_TRUE(saMgr->remoteCallbacks_.size() > 1);
}

/**
 * @tc.name: DoLoadRemoteSystemAbility002 test
 * @tc.desc: test for load complete, with one device, one callback
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, DoLoadRemoteSystemAbility002, TestSize.Level1)
{
    DTEST_LOG << " DoLoadRemoteSystemAbility002 start " << std::endl;
    /**
     * @tc.steps: step1. init remoteCallbacks_
     * @tc.expected: step1. init remoteCallbacks_
     */
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->remoteCallbacks_.clear();

    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();

    saMgr->remoteCallbacks_ = {
        {"11111_222", {mockLoadCallback2}}
    };
    /**
     * @tc.steps: step2. remove one mockLoadCallback2
     * @tc.expected: step2. remoteCallbacks_ remove all mockLoadCallback2
     */
    saMgr->DoLoadRemoteSystemAbility(11111, 0, 0, "222", mockLoadCallback2);
    ASSERT_EQ(saMgr->remoteCallbacks_.size(), 0);
}

/**
 * @tc.name: DoLoadRemoteSystemAbility003 test
 * @tc.desc: test for load complete, with one device, some callback
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, DoLoadRemoteSystemAbility003, TestSize.Level1)
{
    DTEST_LOG << " DoLoadRemoteSystemAbility003 start " << std::endl;
    /**
     * @tc.steps: step1. init remoteCallbacks_
     * @tc.expected: step1. init remoteCallbacks_
     */
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->remoteCallbacks_.clear();

    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();

    saMgr->remoteCallbacks_ = {
        {"11111_111", {mockLoadCallback1, mockLoadCallback2}},
        {"11111_222", {mockLoadCallback2}}
    };
    /**
     * @tc.steps: step2. remove one mockLoadCallback2
     * @tc.expected: step2. remoteCallbacks_ remove all mockLoadCallback2
     */
    saMgr->DoLoadRemoteSystemAbility(11111, 0, 0, "222", mockLoadCallback2);
    ASSERT_EQ(saMgr->remoteCallbacks_.size(), 1);
}

/**
 * @tc.name: param check samgr ready event
 * @tc.desc: param check samgr ready event
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, SamgrReady001, TestSize.Level1)
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
 * @tc.name: ReportGetSAFre001
 * @tc.desc: ReportGetSAFre001
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, ReportGetSAFre001, TestSize.Level3)
{
    DTEST_LOG << " ReportGetSAFre001 start " << std::endl;
    ReportGetSAFrequency(1, 1, 1);
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    uint32_t realPid = 1;
    uint32_t readSaid = 1;
    uint64_t key = saMgr->GenerateFreKey(realPid, readSaid);
    DTEST_LOG << " key 001 :  " << key << std::endl;
    uint32_t expectSid = static_cast<uint32_t>(key);
    uint32_t expectPid = key >> SHFIT_BIT;
    DTEST_LOG << " key 002 :  " << key << std::endl;
    ASSERT_EQ(expectPid, realPid);
    ASSERT_EQ(readSaid, expectSid);
}

/**
 * @tc.name: ReportGetSAFre002
 * @tc.desc: ReportGetSAFre002
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, ReportGetSAFre002, TestSize.Level3)
{
    DTEST_LOG << " ReportGetSAFre002 start " << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    int32_t pid = 1;
    saMgr->saFrequencyMap_.clear();
    saMgr->UpdateSaFreMap(pid, TEST_SYSTEM_ABILITY1);
    ASSERT_EQ(saMgr->saFrequencyMap_.size(), 1);
    saMgr->ReportGetSAPeriodically();
    ASSERT_EQ(saMgr->saFrequencyMap_.size(), 0);
}

/**
 * @tc.name: ReportGetSAFre003
 * @tc.desc: ReportGetSAFre003
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, ReportGetSAFre003, TestSize.Level3)
{
    DTEST_LOG << " ReportGetSAFre003 start " << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    int32_t pid = -1;
    saMgr->saFrequencyMap_.clear();
    saMgr->UpdateSaFreMap(pid, TEST_SYSTEM_ABILITY1);
    saMgr->ReportGetSAPeriodically();
    ASSERT_EQ(saMgr->saFrequencyMap_.size(), 0);
}

/**
 * @tc.name: ReportGetSAFre004
 * @tc.desc: ReportGetSAFre004
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, ReportGetSAFre004, TestSize.Level3)
{
    DTEST_LOG << " ReportGetSAFre004 start " << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    int32_t pid = 1;
    uint64_t key = saMgr->GenerateFreKey(pid, TEST_SYSTEM_ABILITY1);
    saMgr->saFrequencyMap_[key] = MAX_COUNT;
    saMgr->UpdateSaFreMap(pid, TEST_SYSTEM_ABILITY1);
    EXPECT_EQ(saMgr->saFrequencyMap_[key], MAX_COUNT);
    saMgr->saFrequencyMap_.clear();
}

/**
 * @tc.name: Get param debug
 * @tc.desc: ReportGetSAFre002
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, GetParamDebug001, TestSize.Level1)
{
    DTEST_LOG << " GetParamDebug001 " << std::endl;
    bool value = system::GetBoolParameter(ONDEMAND_PARAM, false);
    ASSERT_FALSE(value);
}

/**
 * @tc.name: Test OndemandLoadForPerf
 * @tc.desc: OndemandLoadForPerf001
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, OndemandLoadForPerf001, TestSize.Level3)
{
    DTEST_LOG << " OndemandLoadForPerf001 " << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->OndemandLoadForPerf();
    saMgr->Init();
    saMgr->OndemandLoadForPerf();
    usleep(ONDEMAND_SLEEP_TIME);
    bool value = system::GetBoolParameter(ONDEMAND_PARAM, false);
    ASSERT_FALSE(value);
}

/**
 * @tc.name: Test DoLoadForPerf
 * @tc.desc: DoLoadForPerf001
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, DoLoadForPerf001, TestSize.Level1)
{
    DTEST_LOG << " DoLoadForPerf001 " << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->DoLoadForPerf();
    bool value = system::GetBoolParameter(ONDEMAND_PARAM, false);
    ASSERT_FALSE(value);
}

/**
 * @tc.name: Test GetAllOndemandSa001
 * @tc.desc: GetAllOndemandSa001
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, GetAllOndemandSa001, TestSize.Level3)
{
    DTEST_LOG << " GetAllOndemandSa001 " << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    SaProfile saProfile;
    saMgr->saProfileMap_[1] = saProfile;
    saMgr->GetAllOndemandSa();
    bool value = system::GetBoolParameter(ONDEMAND_PARAM, false);
    EXPECT_FALSE(value);
    saMgr->saProfileMap_.clear();
}

/**
 * @tc.name: Test GetAllOndemandSa002
 * @tc.desc: GetAllOndemandSa002
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, GetAllOndemandSa002, TestSize.Level3)
{
    DTEST_LOG << " GetAllOndemandSa002 " << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    SaProfile saProfile;
    saMgr->saProfileMap_[1] = saProfile;
    SAInfo saInfo;
    saMgr->abilityMap_[1] = saInfo;
    saMgr->GetAllOndemandSa();
    bool value = system::GetBoolParameter(ONDEMAND_PARAM, false);
    EXPECT_FALSE(value);
    saMgr->saProfileMap_.clear();
    saMgr->abilityMap_.clear();
}
} // namespace OHOS
