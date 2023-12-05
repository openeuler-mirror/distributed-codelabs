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

#include "system_ability_mgr_stub_test.h"

#include "ability_death_recipient.h"
#include "itest_transaction_service.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "sam_mock_permission.h"
#include "string_ex.h"
#include "system_ability_definition.h"
#include "test_log.h"

#define private public
#include "sa_status_change_mock.h"
#include "system_ability_manager.h"

using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS;

namespace OHOS {
namespace {
const std::u16string SAMANAGER_INTERFACE_TOKEN = u"ohos.samgr.accessToken";
constexpr uint32_t SAID = 1499;
constexpr uint32_t INVALID_SAID = -1;
constexpr uint32_t INVALID_CODE = 50;
}
void SystemAbilityMgrStubTest::SetUpTestCase()
{
    DTEST_LOG << "SetUpTestCase" << std::endl;
}

void SystemAbilityMgrStubTest::TearDownTestCase()
{
    DTEST_LOG << "TearDownTestCase" << std::endl;
}

void SystemAbilityMgrStubTest::SetUp()
{
    DTEST_LOG << "SetUp" << std::endl;
}

void SystemAbilityMgrStubTest::TearDown()
{
    DTEST_LOG << "TearDown" << std::endl;
}

/**
 * @tc.name: OnRemoteRequest001
 * @tc.desc: test OnRemoteRequest, code not exist
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, OnRemoteRequest001, TestSize.Level4)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN);
    MessageParcel reply;
    MessageOption option;
    int32_t result = saMgr->OnRemoteRequest(INVALID_CODE, data, reply, option);
    EXPECT_EQ(result, IPC_STUB_UNKNOW_TRANS_ERR);
}

/**
 * @tc.name: ListSystemAbilityInner001
 * @tc.desc: test ListSystemAbilityInner, permission denied!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, ListSystemAbilityInner001, TestSize.Level4)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t result = saMgr->ListSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: SubsSystemAbilityInner001
 * @tc.desc: test SubsSystemAbilityInner, permission denied!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, SubsSystemAbilityInner001, TestSize.Level4)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t result = saMgr->SubsSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: UnSubsSystemAbilityInner001
 * @tc.desc: test UnSubsSystemAbilityInner, permission denied!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, UnSubsSystemAbilityInner001, TestSize.Level4)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t result = saMgr->UnSubsSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: CheckRemtSystemAbilityInner001
 * @tc.desc: test CheckRemtSystemAbilityInner, permission denied!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, CheckRemtSystemAbilityInner001, TestSize.Level4)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t result = saMgr->CheckRemtSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: AddOndemandSystemAbilityInner001
 * @tc.desc: test AddOndemandSystemAbilityInner, permission denied!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddOndemandSystemAbilityInner001, TestSize.Level4)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t result = saMgr->AddOndemandSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: AddSystemAbilityInner001
 * @tc.desc: test AddSystemAbilityInner, permission denied!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddSystemAbilityInner001, TestSize.Level4)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t result = saMgr->AddSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: RemoveSystemAbilityInner001
 * @tc.desc: test RemoveSystemAbilityInner, permission denied!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, RemoveSystemAbilityInner001, TestSize.Level4)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t result = saMgr->RemoveSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: AddSystemProcessInner001
 * @tc.desc: test AddSystemProcessInner, permission denied!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddSystemProcessInner001, TestSize.Level4)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t result = saMgr->AddSystemProcessInner(data, reply);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: ListSystemAbilityInner002
 * @tc.desc: test ListSystemAbilityInner, read dumpflag failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, ListSystemAbilityInner002, TestSize.Level3)
{
    SamMockPermission::MockPermission();
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t result = saMgr->ListSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_FLATTEN_OBJECT);
}

/**
 * @tc.name: ListSystemAbilityInner003
 * @tc.desc: test ListSystemAbilityInner, list empty!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, ListSystemAbilityInner003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t invalidDump = 1;
    data.WriteInt32(invalidDump);
    int32_t result = saMgr->ListSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: ListSystemAbilityInner004
 * @tc.desc: test ListSystemAbilityInner, list no empty!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, ListSystemAbilityInner004, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    SAInfo saInfo;
    saMgr->abilityMap_[SAID] = saInfo;
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t invalidDump = 1;
    data.WriteInt32(invalidDump);
    int32_t result = saMgr->ListSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: SubsSystemAbilityInner002
 * @tc.desc: test SubsSystemAbilityInner, read systemAbilityId failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, SubsSystemAbilityInner002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(INVALID_SAID);
    int32_t result = saMgr->SubsSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: SubsSystemAbilityInner003
 * @tc.desc: test SubsSystemAbilityInner, read listener failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, SubsSystemAbilityInner003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    int32_t result = saMgr->SubsSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: SubsSystemAbilityInner004
 * @tc.desc: test SubsSystemAbilityInner!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, SubsSystemAbilityInner004, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<IRemoteObject> testAbility(new SaStatusChangeMock());
    SAInfo saInfo;
    saMgr->abilityMap_[SAID] = saInfo;
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    data.WriteRemoteObject(testAbility);
    int32_t result = saMgr->SubsSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: UnSubsSystemAbilityInner002
 * @tc.desc: test UnSubsSystemAbilityInner, read systemAbilityId failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, UnSubsSystemAbilityInner002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(INVALID_SAID);
    int32_t result = saMgr->UnSubsSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: UnSubsSystemAbilityInner003
 * @tc.desc: test UnSubsSystemAbilityInner, read listener failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, UnSubsSystemAbilityInner003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    int32_t result = saMgr->UnSubsSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: UnSubsSystemAbilityInner004
 * @tc.desc: test UnSubsSystemAbilityInner!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, UnSubsSystemAbilityInner004, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<IRemoteObject> testAbility(new SaStatusChangeMock());
    SAInfo saInfo;
    saMgr->abilityMap_[SAID] = saInfo;
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    data.WriteRemoteObject(testAbility);
    int32_t result = saMgr->UnSubsSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_OK);
}


/**
 * @tc.name: CheckRemtSystemAbilityInner002
 * @tc.desc: test CheckRemtSystemAbilityInner, read systemAbilityId failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, CheckRemtSystemAbilityInner002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(INVALID_SAID);
    int32_t result = saMgr->CheckRemtSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: CheckRemtSystemAbilityInner003
 * @tc.desc: test CheckRemtSystemAbilityInner, read deviceId failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, CheckRemtSystemAbilityInner003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    int32_t result = saMgr->CheckRemtSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_FLATTEN_OBJECT);
}

/**
 * @tc.name: CheckRemtSystemAbilityInner004
 * @tc.desc: test CheckRemtSystemAbilityInner!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, CheckRemtSystemAbilityInner004, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    std::string deviceId = "test";
    data.WriteString(deviceId);
    int32_t result = saMgr->CheckRemtSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_FLATTEN_OBJECT);
}

/**
 * @tc.name: AddOndemandSystemAbilityInner002
 * @tc.desc: test AddOndemandSystemAbilityInner, read systemAbilityId failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddOndemandSystemAbilityInner002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(INVALID_SAID);
    int32_t result = saMgr->AddOndemandSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: AddOndemandSystemAbilityInner003
 * @tc.desc: test AddOndemandSystemAbilityInner, read localName failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddOndemandSystemAbilityInner003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    int32_t result = saMgr->AddOndemandSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: AddOndemandSystemAbilityInner004
 * @tc.desc: test AddOndemandSystemAbilityInner!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddOndemandSystemAbilityInner004, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    u16string localManagerName = u"test";
    data.WriteString16(localManagerName);
    int32_t result = saMgr->AddOndemandSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: CheckSystemAbilityImmeInner001
 * @tc.desc: test CheckSystemAbilityImmeInner, read systemAbilityId failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, CheckSystemAbilityImmeInner001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(INVALID_SAID);
    int32_t result = saMgr->CheckSystemAbilityImmeInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: CheckSystemAbilityImmeInner002
 * @tc.desc: test CheckSystemAbilityImmeInner,  read isExist failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, CheckSystemAbilityImmeInner002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    int32_t result = saMgr->CheckSystemAbilityImmeInner(data, reply);
    EXPECT_EQ(result, ERR_FLATTEN_OBJECT);
}

/**
 * @tc.name: CheckSystemAbilityImmeInner003
 * @tc.desc: test CheckSystemAbilityImmeInner, WriteRemoteObject failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, CheckSystemAbilityImmeInner003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    data.WriteBool(false);
    int32_t result = saMgr->CheckSystemAbilityImmeInner(data, reply);
    EXPECT_EQ(result, ERR_FLATTEN_OBJECT);
}

/**
 * @tc.name: CheckSystemAbilityImmeInner004
 * @tc.desc: test CheckSystemAbilityImmeInner!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, CheckSystemAbilityImmeInner004, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    sptr<IRemoteObject> testAbility(new SaStatusChangeMock());
    EXPECT_TRUE(testAbility != nullptr);
    SAInfo saInfo;
    saInfo.remoteObj = testAbility;
    saMgr->abilityMap_[SAID] = saInfo;
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    data.WriteBool(false);
    int32_t result = saMgr->CheckSystemAbilityImmeInner(data, reply);
    EXPECT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: UnmarshalingSaExtraProp001
 * @tc.desc: test UnmarshalingSaExtraProp, read isDistributed failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, UnmarshalingSaExtraProp001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    SystemAbilityManager::SAExtraProp extraProp;
    int32_t result = saMgr->UnmarshalingSaExtraProp(data, extraProp);
    EXPECT_EQ(result, ERR_FLATTEN_OBJECT);
}

/**
 * @tc.name: UnmarshalingSaExtraProp002
 * @tc.desc: test UnmarshalingSaExtraProp,  UnmarshalingSaExtraProp dumpFlags failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, UnmarshalingSaExtraProp002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    SystemAbilityManager::SAExtraProp extraProp;
    data.WriteBool(false);
    int32_t result = saMgr->UnmarshalingSaExtraProp(data, extraProp);
    EXPECT_EQ(result, ERR_FLATTEN_OBJECT);
}

/**
 * @tc.name: UnmarshalingSaExtraProp003
 * @tc.desc: test UnmarshalingSaExtraProp!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, UnmarshalingSaExtraProp003, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    SystemAbilityManager::SAExtraProp extraProp;
    bool isExist = false;
    int32_t dumpFlags = 0;
    std::u16string capability = u"capability";
    std::u16string permission = u"permission";
    data.WriteBool(isExist);
    data.WriteInt32(dumpFlags);
    data.WriteString16(capability);
    data.WriteString16(permission);
    int32_t result = saMgr->UnmarshalingSaExtraProp(data, extraProp);
    EXPECT_EQ(result, ERR_OK);
}


/**
 * @tc.name: AddSystemAbilityInner002
 * @tc.desc: test AddSystemAbilityInner, read systemAbilityId failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddSystemAbilityInner002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(INVALID_SAID);
    int32_t result = saMgr->AddSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: AddSystemAbilityInner003
 * @tc.desc: test AddSystemAbilityInner, readParcelable failed!!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddSystemAbilityInner003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    int32_t result = saMgr->AddSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: AddSystemAbilityInner004
 * @tc.desc: test AddSystemAbilityInner, UnmarshalingSaExtraProp failed!!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddSystemAbilityInner004, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<IRemoteObject> testAbility(new SaStatusChangeMock());
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    data.WriteRemoteObject(testAbility);
    int32_t result = saMgr->AddSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_FLATTEN_OBJECT);
}

/**
 * @tc.name: AddSystemAbilityInner005
 * @tc.desc: test AddSystemAbilityInner!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddSystemAbilityInner005, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<IRemoteObject> testAbility(new SaStatusChangeMock());
    EXPECT_TRUE(saMgr != nullptr);
    SystemAbilityManager::SAExtraProp extraProp;
    bool isExist = false;
    int32_t dumpFlags = 0;
    std::u16string capability = u"capability";
    std::u16string permission = u"permission";
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    data.WriteRemoteObject(testAbility);
    data.WriteBool(isExist);
    data.WriteInt32(dumpFlags);
    data.WriteString16(capability);
    data.WriteString16(permission);
    int32_t result = saMgr->AddSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: GetSystemAbilityInner001
 * @tc.desc: test GetSystemAbilityInner, read systemAbilityId failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, GetSystemAbilityInner001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t result = saMgr->GetSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: GetSystemAbilityInner002
 * @tc.desc: test GetSystemAbilityInner,  GetSystemAbilityInner write reply failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, GetSystemAbilityInner002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    saMgr->abilityMap_.clear();
    int32_t result = saMgr->GetSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_FLATTEN_OBJECT);
}

/**
 * @tc.name: GetSystemAbilityInner003
 * @tc.desc: test GetSystemAbilityInner!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, GetSystemAbilityInner003, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    sptr<IRemoteObject> testAbility(new SaStatusChangeMock());
    MessageParcel data;
    MessageParcel reply;
    SAInfo saInfo;
    saInfo.remoteObj = testAbility;
    saMgr->abilityMap_[SAID] = saInfo;
    data.WriteInt32(SAID);
    int32_t result = saMgr->GetSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: CheckSystemAbilityInner001
 * @tc.desc: test CheckSystemAbilityInner, read systemAbilityId failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, CheckSystemAbilityInner001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t result = saMgr->CheckSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: CheckSystemAbilityInner002
 * @tc.desc: test CheckSystemAbilityInner!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, CheckSystemAbilityInner002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    saMgr->abilityMap_.clear();
    int32_t result = saMgr->CheckSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_FLATTEN_OBJECT);
}

/**
 * @tc.name: CheckSystemAbilityInner003
 * @tc.desc: test CheckSystemAbilityInner!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, CheckSystemAbilityInner003, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    sptr<IRemoteObject> testAbility(new SaStatusChangeMock());
    MessageParcel data;
    MessageParcel reply;
    SAInfo saInfo;
    saInfo.remoteObj = testAbility;
    saMgr->abilityMap_[SAID] = saInfo;
    data.WriteInt32(SAID);
    int32_t result = saMgr->CheckSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: RemoveSystemAbilityInner002
 * @tc.desc: test RemoveSystemAbilityInner, read systemAbilityId failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, RemoveSystemAbilityInner002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(INVALID_SAID);
    int32_t result = saMgr->RemoveSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: RemoveSystemAbilityInner003
 * @tc.desc: test RemoveSystemAbilityInner, RemoveSystemability not found!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, RemoveSystemAbilityInner003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    saMgr->abilityMap_.clear();
    int32_t result = saMgr->RemoveSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AddSystemProcessInner002
 * @tc.desc: test AddSystemProcessInner, read process name failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddSystemProcessInner002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t result = saMgr->AddSystemProcessInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: AddSystemProcessInner003
 * @tc.desc: test AddSystemProcessInner readParcelable failed!!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddSystemProcessInner003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    std::u16string procName = u"test";
    data.WriteString16(procName);
    int32_t result = saMgr->AddSystemProcessInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: AddSystemProcessInner004
 * @tc.desc: test AddSystemProcessInner!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddSystemProcessInner004, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    sptr<IRemoteObject> testAbility(new SaStatusChangeMock());
    MessageParcel data;
    MessageParcel reply;
    std::u16string procName = u"test";
    data.WriteString16(procName);
    data.WriteRemoteObject(testAbility);
    int32_t result = saMgr->AddSystemProcessInner(data, reply);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: LoadRemoteSystemAbilityInner001
 * @tc.desc: test LoadRemoteSystemAbilityInner, systemAbilityId invalid!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, LoadRemoteSystemAbilityInner001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t result = saMgr->LoadRemoteSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: LoadRemoteSystemAbilityInner002
 * @tc.desc: test LoadRemoteSystemAbilityInner,  LoadRemoteSystemAbilityInner read deviceId failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, LoadRemoteSystemAbilityInner002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    int32_t result = saMgr->LoadRemoteSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: LoadRemoteSystemAbilityInner003
 * @tc.desc: test LoadRemoteSystemAbilityInner, read callback failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, LoadRemoteSystemAbilityInner003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    string deviceId = "test";
    data.WriteString(deviceId);
    int32_t result = saMgr->LoadRemoteSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: LoadRemoteSystemAbilityInner004
 * @tc.desc: test LoadRemoteSystemAbilityInner!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, LoadRemoteSystemAbilityInner004, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    string deviceId = "test";
    data.WriteString(deviceId);
    data.WriteRemoteObject(callback);
    int32_t result = saMgr->LoadRemoteSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: InitSaProfile001
 * @tc.desc: test InitSaProfile! InitSaProfile.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, InitSaProfile001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    auto runner = AppExecFwk::EventRunner::Create("workHandler");
    saMgr->workHandler_ = make_shared<AppExecFwk::EventHandler>(runner);
    saMgr->InitSaProfile();
    EXPECT_NE(saMgr->workHandler_, nullptr);
}

/**
 * @tc.name: InitSaProfile002
 * @tc.desc: test InitSaProfile! parseHandler_ not init!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, InitSaProfile002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->workHandler_ = nullptr;
    saMgr->InitSaProfile();
    EXPECT_EQ(saMgr->workHandler_, nullptr);
}

/**
 * @tc.name: GetSaProfile001
 * @tc.desc: test GetSaProfile! return true
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, GetSaProfile001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    SaProfile saProfilein;
    SaProfile SaProfileout;
    saMgr->saProfileMap_[SAID] = saProfilein;
    bool res = saMgr->GetSaProfile(SAID, SaProfileout);
    saMgr->saProfileMap_.clear();
    EXPECT_EQ(res, true);
}

/**
 * @tc.name: GetSystemAbility001
 * @tc.desc: test GetSystemAbility! return nullptr
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, GetSystemAbility001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<IRemoteObject> res = saMgr->GetSystemAbility(INVALID_SAID);
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: GetSystemAbility002
 * @tc.desc: test GetSystemAbility! return nullptr
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, GetSystemAbility002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    string deviceId = "test";
    sptr<IRemoteObject> res = saMgr->GetSystemAbility(SAID, deviceId);
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: GetSystemAbilityFromRemote001
 * @tc.desc: test GetSystemAbilityFromRemote! GetSystemAbilityFromRemote invalid!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, GetSystemAbilityFromRemote001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<IRemoteObject> res = saMgr->GetSystemAbilityFromRemote(INVALID_SAID);
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: GetSystemAbilityFromRemote002
 * @tc.desc: test GetSystemAbilityFromRemote! not found service!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, GetSystemAbilityFromRemote002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<IRemoteObject> res = saMgr->GetSystemAbilityFromRemote(SAID);
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: GetSystemAbilityFromRemote003
 * @tc.desc: test GetSystemAbilityFromRemote! service not distributed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, GetSystemAbilityFromRemote003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    SAInfo saInfo;
    saInfo.isDistributed = false;
    saMgr->abilityMap_[SAID] = saInfo;
    sptr<IRemoteObject> res = saMgr->GetSystemAbilityFromRemote(SAID);
    saMgr->abilityMap_.clear();
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: GetSystemAbilityFromRemote004
 * @tc.desc: test GetSystemAbilityFromRemote! GetSystemAbilityFromRemote found service!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, GetSystemAbilityFromRemote004, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    SAInfo saInfo;
    saInfo.isDistributed = true;
    saInfo.remoteObj = saMgr;
    saMgr->abilityMap_[SAID] = saInfo;
    sptr<IRemoteObject> res = saMgr->GetSystemAbilityFromRemote(SAID);
    saMgr->abilityMap_.clear();
    EXPECT_NE(res, nullptr);
}

/**
 * @tc.name: CheckSystemAbility001
 * @tc.desc: test CheckSystemAbility! CheckSystemAbility invalid!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, CheckSystemAbility001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<IRemoteObject> res = saMgr->CheckSystemAbility(INVALID_SAID);
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: CheckSystemAbility002
 * @tc.desc: test CheckSystemAbility! found service!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, CheckSystemAbility002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    SAInfo saInfo;
    saInfo.remoteObj = saMgr;
    saMgr->abilityMap_[SAID] = saInfo;
    sptr<IRemoteObject> res = saMgr->CheckSystemAbility(SAID);
    saMgr->abilityMap_.clear();
    EXPECT_NE(res, nullptr);
}

/**
 * @tc.name: CheckDistributedPermission001
 * @tc.desc: test CheckDistributedPermission! return true!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, CheckDistributedPermission001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    bool res = saMgr->CheckDistributedPermission();
    EXPECT_EQ(res, true);
}

/**
 * @tc.name: NotifySystemAbilityChanged001
 * @tc.desc: test NotifySystemAbilityChanged! listener null pointer!!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, NotifySystemAbilityChanged001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    string deviceId = "test";
    int32_t code = 1;
    saMgr->NotifySystemAbilityChanged(SAID, deviceId, code, nullptr);
    EXPECT_NE(saMgr, nullptr);
}

/**
 * @tc.name: NotifySystemAbilityChanged002
 * @tc.desc: test NotifySystemAbilityChanged!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, NotifySystemAbilityChanged002, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<SaStatusChangeMock> testAbility(new SaStatusChangeMock());
    string deviceId = "test";
    int32_t code = 1;
    saMgr->NotifySystemAbilityChanged(SAID, deviceId, code, testAbility);
    EXPECT_NE(saMgr, nullptr);
}

/**
 * @tc.name: FindSystemAbilityNotify001
 * @tc.desc: test FindSystemAbilityNotify!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, FindSystemAbilityNotify001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    int32_t pid = 1;
    saMgr->listenerMap_[SAID].push_back(make_pair(nullptr, pid));
    string deviceId = "test";
    int32_t code = 1;
    bool res = saMgr->FindSystemAbilityNotify(SAID, deviceId, code);
    saMgr->listenerMap_.clear();
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: FindSystemAbilityNotify002
 * @tc.desc: test FindSystemAbilityNotify!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, FindSystemAbilityNotify002, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->listenerMap_.clear();
    string deviceId = "test";
    int32_t code = 1;
    bool res = saMgr->FindSystemAbilityNotify(SAID, deviceId, code);
    saMgr->listenerMap_.clear();
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: StartOnDemandAbility001
 * @tc.desc: test StartOnDemandAbility!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, StartOnDemandAbility001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    u16string procName = u"test";
    saMgr->StartOnDemandAbility(procName, SAID);
    EXPECT_NE(saMgr, nullptr);
}

/**
 * @tc.name: StartOnDemandAbility002
 * @tc.desc: test StartOnDemandAbility!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, StartOnDemandAbility002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    u16string procName = u"test";
    SystemAbilityManager::AbilityItem abilityItem;
    abilityItem.state = SystemAbilityManager::AbilityState::STARTING;
    saMgr->startingAbilityMap_[SAID] = abilityItem;
    saMgr->StartOnDemandAbility(procName, SAID);
    saMgr->startingAbilityMap_.clear();
    EXPECT_NE(saMgr, nullptr);
}

/**
 * @tc.name: StartOnDemandAbilityInner001
 * @tc.desc: test StartOnDemandAbilityInner!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, StartOnDemandAbilityInner001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    u16string procName = u"test";
    SystemAbilityManager::AbilityItem abilityItem;
    abilityItem.state = SystemAbilityManager::AbilityState::STARTING;
    saMgr->StartOnDemandAbilityInner(procName, SAID, abilityItem);
    EXPECT_EQ(abilityItem.state, SystemAbilityManager::AbilityState::STARTING);
}

/**
 * @tc.name: StartOnDemandAbilityInner002
 * @tc.desc: test StartOnDemandAbilityInner, get process fail!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, StartOnDemandAbilityInner002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    u16string procName = u"";
    SystemAbilityManager::AbilityItem abilityItem;
    abilityItem.state = SystemAbilityManager::AbilityState::INIT;
    saMgr->StartOnDemandAbilityInner(procName, SAID, abilityItem);
    EXPECT_EQ(abilityItem.state, SystemAbilityManager::AbilityState::INIT);
}

/**
 * @tc.name: AddOnDemandSystemAbilityInfo001
 * @tc.desc: test AddOnDemandSystemAbilityInfo, map size error!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddOnDemandSystemAbilityInfo001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    u16string procName = u"test";
    int maxService = 1002;
    for (int tempSaid = 1; tempSaid < maxService; tempSaid++) {
        saMgr->onDemandAbilityMap_[tempSaid] = procName;
    }
    int32_t res = saMgr->AddOnDemandSystemAbilityInfo(SAID, procName);
    saMgr->onDemandAbilityMap_.clear();
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AddOnDemandSystemAbilityInfo002
 * @tc.desc: test AddOnDemandSystemAbilityInfo, onDemand systemAbilityId!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddOnDemandSystemAbilityInfo002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    u16string procName = u"test";
    saMgr->systemProcessMap_[procName] = saMgr;
    int32_t res = saMgr->AddOnDemandSystemAbilityInfo(SAID, procName);
    saMgr->systemProcessMap_.clear();
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: AddOnDemandSystemAbilityInfo003
 * @tc.desc: test AddOnDemandSystemAbilityInfo!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddOnDemandSystemAbilityInfo003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    u16string procName = u"test";
    saMgr->systemProcessMap_[procName] = saMgr;
    SystemAbilityManager::AbilityItem abilityItem;
    abilityItem.state = SystemAbilityManager::AbilityState::STARTING;
    saMgr->startingAbilityMap_[SAID] = abilityItem;
    int32_t res = saMgr->AddOnDemandSystemAbilityInfo(SAID, procName);
    saMgr->systemProcessMap_.clear();
    saMgr->startingAbilityMap_.clear();
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: AddOnDemandSystemAbilityInfo004
 * @tc.desc: test AddOnDemandSystemAbilityInfo!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddOnDemandSystemAbilityInfo004, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    u16string procName = u"test";
    saMgr->systemProcessMap_[procName] = saMgr;
    SystemAbilityManager::AbilityItem abilityItem;
    abilityItem.state = SystemAbilityManager::AbilityState::STARTING;
    auto runner = AppExecFwk::EventRunner::Create("workHandler");
    saMgr->workHandler_ = make_shared<AppExecFwk::EventHandler>(runner);
    saMgr->startingAbilityMap_[SAID] = abilityItem;
    int32_t res = saMgr->AddOnDemandSystemAbilityInfo(SAID, procName);
    saMgr->systemProcessMap_.clear();
    saMgr->startingAbilityMap_.clear();
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: StartOnDemandAbility003
 * @tc.desc: test StartOnDemandAbility!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, StartOnDemandAbility003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    u16string procName = u"test";
    saMgr->onDemandAbilityMap_[SAID] = procName;
    int32_t res = saMgr->StartOnDemandAbility(SAID);
    saMgr->onDemandAbilityMap_.clear();
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: CheckSystemAbility003
 * @tc.desc: test CheckSystemAbility!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, CheckSystemAbility003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    bool isExist = false;
    sptr<IRemoteObject> res = saMgr->CheckSystemAbility(INVALID_SAID, isExist);
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: CheckSystemAbility004
 * @tc.desc: test CheckSystemAbility!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, CheckSystemAbility004, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    SAInfo saInfo;
    saInfo.remoteObj = saMgr;
    saMgr->abilityMap_[SAID] = saInfo;
    bool isExist = false;
    sptr<IRemoteObject> res = saMgr->CheckSystemAbility(SAID, isExist);
    saMgr->abilityMap_.clear();
    EXPECT_NE(res, nullptr);
}

/**
 * @tc.name: CheckSystemAbility005
 * @tc.desc: test CheckSystemAbility!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, CheckSystemAbility005, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    bool isExist = false;
    sptr<IRemoteObject> res = saMgr->CheckSystemAbility(SAID, isExist);
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: CheckSystemAbility006
 * @tc.desc: test CheckSystemAbility!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, CheckSystemAbility006, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    SystemAbilityManager::AbilityItem abilityItem;
    abilityItem.state = SystemAbilityManager::AbilityState::STARTING;
    saMgr->startingAbilityMap_[SAID] = abilityItem;
    bool isExist = false;
    sptr<IRemoteObject> res = saMgr->CheckSystemAbility(SAID, isExist);
    saMgr->startingAbilityMap_.clear();
    EXPECT_EQ(isExist, true);
}

/**
 * @tc.name: CheckSystemAbility007
 * @tc.desc: test CheckSystemAbility!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, CheckSystemAbility007, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    SystemAbilityManager::AbilityItem abilityItem;
    abilityItem.state = SystemAbilityManager::AbilityState::INIT;
    saMgr->startingAbilityMap_[SAID] = abilityItem;
    u16string proName = u"test";
    saMgr->onDemandAbilityMap_[SAID] = proName;
    bool isExist = false;
    sptr<IRemoteObject> res = saMgr->CheckSystemAbility(SAID, isExist);
    saMgr->startingAbilityMap_.clear();
    saMgr->onDemandAbilityMap_.clear();
    EXPECT_EQ(isExist, true);
}

/**
 * @tc.name: RemoveSystemAbility001
 * @tc.desc: test RemoveSystemAbility, RemoveSystemAbility systemAbilityId!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, RemoveSystemAbility001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    int32_t res = saMgr->RemoveSystemAbility(INVALID_SAID);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: RemoveSystemAbility002
 * @tc.desc: test RemoveSystemAbility, RemoveSystemAbility not found!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, RemoveSystemAbility002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    int32_t res = saMgr->RemoveSystemAbility(SAID);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: RemoveSystemAbility003
 * @tc.desc: test RemoveSystemAbility, ability nullptr!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, RemoveSystemAbility003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    SAInfo saInfo;
    saInfo.remoteObj = nullptr;
    saMgr->abilityMap_[SAID] = saInfo;
    int32_t res = saMgr->RemoveSystemAbility(SAID);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: RemoveSystemAbility004
 * @tc.desc: test RemoveSystemAbility, abilityDeath_ nullptr!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, RemoveSystemAbility004, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    SAInfo saInfo;
    saInfo.remoteObj = saMgr;
    saMgr->abilityMap_[SAID] = saInfo;
    saMgr->abilityDeath_ = nullptr;
    int32_t res = saMgr->RemoveSystemAbility(SAID);
    saMgr->abilityDeath_.clear();
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: RemoveSystemAbility005
 * @tc.desc: test RemoveSystemAbility, ability is nullptr!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, RemoveSystemAbility005, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    int32_t res = saMgr->RemoveSystemAbility(nullptr);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: RemoveSystemAbility006
 * @tc.desc: test RemoveSystemAbility, abilityDeath_ is nullptr!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, RemoveSystemAbility006, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    SAInfo saInfo;
    saInfo.remoteObj = saMgr;
    saMgr->abilityMap_[SAID] = saInfo;
    saMgr->abilityDeath_ = nullptr;
    int32_t res = saMgr->RemoveSystemAbility(saMgr);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: RemoveSystemAbility007
 * @tc.desc: test RemoveSystemAbility, saId is zero!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, RemoveSystemAbility007, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    SAInfo saInfo;
    saInfo.remoteObj = saMgr;
    uint32_t saId = 0;
    saMgr->abilityMap_[saId] = saInfo;
    saMgr->abilityDeath_ = nullptr;
    int32_t res = saMgr->RemoveSystemAbility(saMgr);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: SubscribeSystemAbility001
 * @tc.desc: test SubscribeSystemAbility, systemAbilityId or listener invalid!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, SubscribeSystemAbility001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<SaStatusChangeMock> listener(new SaStatusChangeMock());
    int32_t res = saMgr->SubscribeSystemAbility(INVALID_SAID, listener);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: SubscribeSystemAbility002
 * @tc.desc: test SubscribeSystemAbility, systemAbilityId or listener invalid!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, SubscribeSystemAbility002, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<SaStatusChangeMock> listener(nullptr);
    int32_t res = saMgr->SubscribeSystemAbility(SAID, listener);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: SubscribeSystemAbility003
 * @tc.desc: test SubscribeSystemAbility, systemAbilityId or listener invalid!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, SubscribeSystemAbility003, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<SaStatusChangeMock> listener(nullptr);
    int32_t res = saMgr->SubscribeSystemAbility(INVALID_SAID, listener);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: SubscribeSystemAbility004
 * @tc.desc: test SubscribeSystemAbility, already exist listener!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, SubscribeSystemAbility004, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<SaStatusChangeMock> listener(new SaStatusChangeMock());
    saMgr->listenerMap_[SAID].push_back(make_pair(listener, SAID));
    int32_t res = saMgr->SubscribeSystemAbility(SAID, listener);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: UnSubscribeSystemAbility001
 * @tc.desc: test UnSubscribeSystemAbility, UnSubscribeSystemAbility systemAbilityId or listener invalid!!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, UnSubscribeSystemAbility001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<SaStatusChangeMock> listener(new SaStatusChangeMock());
    int32_t res = saMgr->UnSubscribeSystemAbility(INVALID_SAID, listener);
    u16string name = u"device_saname";
    saMgr->NotifyRemoteSaDied(name);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: UnSubscribeSystemAbility002
 * @tc.desc: test UnSubscribeSystemAbility, UnSubscribeSystemAbility systemAbilityId or listener invalid!!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, UnSubscribeSystemAbility002, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<SaStatusChangeMock> listener(nullptr);
    int32_t res = saMgr->UnSubscribeSystemAbility(SAID, listener);
    u16string name = u"deviceSaname";
    saMgr->dBinderService_ = nullptr;
    saMgr->NotifyRemoteSaDied(name);
    saMgr->dBinderService_ = DBinderService::GetInstance();
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: UnSubscribeSystemAbility003
 * @tc.desc: test UnSubscribeSystemAbility, UnSubscribeSystemAbility systemAbilityId or listener invalid!!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, UnSubscribeSystemAbility003, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<SaStatusChangeMock> listener(nullptr);
    int32_t res = saMgr->UnSubscribeSystemAbility(INVALID_SAID, listener);
    string deviceId = "device";
    saMgr->NotifyRemoteDeviceOffline(deviceId);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: UnSubscribeSystemAbility004
 * @tc.desc: test UnSubscribeSystemAbility
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, UnSubscribeSystemAbility004, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<SaStatusChangeMock> listener(new SaStatusChangeMock());
    saMgr->listenerMap_[SAID].push_back(make_pair(listener, SAID));
    saMgr->abilityStatusDeath_ = nullptr;
    saMgr->subscribeCountMap_.clear();
    int32_t res = saMgr->UnSubscribeSystemAbility(SAID, listener);
    saMgr->dBinderService_ = nullptr;
    string deviceId = "device";
    saMgr->NotifyRemoteDeviceOffline(deviceId);
    saMgr->dBinderService_ = DBinderService::GetInstance();
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: UnSubscribeSystemAbility005
 * @tc.desc: test UnSubscribeSystemAbility
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, UnSubscribeSystemAbility005, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<SaStatusChangeMock> listener(new SaStatusChangeMock());
    saMgr->listenerMap_[SAID].push_back(make_pair(listener, SAID));
    saMgr->abilityStatusDeath_ = nullptr;
    int countNum = 2;
    saMgr->subscribeCountMap_[SAID] = countNum;
    int32_t res = saMgr->UnSubscribeSystemAbility(SAID, listener);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: AddSystemAbility001
 * @tc.desc: test AddSystemAbility, input params is invalid
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddSystemAbility001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<IRemoteObject> testAbility(new SaStatusChangeMock());
    SystemAbilityManager::SAExtraProp extraProp;
    int32_t res = saMgr->AddSystemAbility(INVALID_SAID, testAbility, extraProp);
    saMgr->workHandler_ = nullptr;
    saMgr->SendSystemAbilityRemovedMsg(SAID);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AddSystemAbility002
 * @tc.desc: test AddSystemAbility, input params is invalid, work handler not initialized
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddSystemAbility002, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<IRemoteObject> testAbility(nullptr);
    SystemAbilityManager::SAExtraProp extraProp;
    int32_t res = saMgr->AddSystemAbility(SAID, testAbility, extraProp);
    u16string name;
    string srcDeviceId;
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    saMgr->SendCheckLoadedMsg(SAID, name, srcDeviceId, callback);
    saMgr->SendLoadedSystemAblityMsg(SAID, testAbility, callback);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AddSystemAbility003
 * @tc.desc: test AddSystemAbility, input params is invalid
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddSystemAbility003, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<IRemoteObject> testAbility(nullptr);
    SystemAbilityManager::SAExtraProp extraProp;
    int32_t res = saMgr->AddSystemAbility(INVALID_SAID, testAbility, extraProp);
    auto runner = AppExecFwk::EventRunner::Create("workHandler");
    saMgr->workHandler_ = make_shared<AppExecFwk::EventHandler>(runner);
    u16string name;
    string srcDeviceId;
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    saMgr->SendCheckLoadedMsg(SAID, name, srcDeviceId, callback);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AddSystemAbility004
 * @tc.desc: test AddSystemAbility, return ERR_OK,SendCheckLoadedMsg SA loaded
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddSystemAbility004, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<IRemoteObject> testAbility(new SaStatusChangeMock());
    SystemAbilityManager::SAExtraProp extraProp;
    extraProp.isDistributed = false;
    saMgr->dBinderService_ = nullptr;
    int32_t res = saMgr->AddSystemAbility(SAID, testAbility, extraProp);
    saMgr->dBinderService_ = DBinderService::GetInstance();
    u16string name;
    string srcDeviceId;
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    SAInfo saInfo;
    saMgr->abilityMap_[SAID] = saInfo;
    saMgr->SendCheckLoadedMsg(SAID, name, srcDeviceId, callback);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: AddSystemAbility005
 * @tc.desc: test AddSystemAbility, return ERR_OK
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddSystemAbility005, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<IRemoteObject> testAbility(new SaStatusChangeMock());
    SystemAbilityManager::SAExtraProp extraProp;
    saMgr->dBinderService_ = nullptr;
    int32_t res = saMgr->AddSystemAbility(SOFTBUS_SERVER_SA_ID, testAbility, extraProp);
    saMgr->dBinderService_ = DBinderService::GetInstance();
    u16string name = u"test";
    string srcDeviceId;
    int64_t id = 1;
    saMgr->startingProcessMap_[name] = id;
    saMgr->startingAbilityMap_.clear();
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    saMgr->CleanCallbackForLoadFailed(SAID, name, srcDeviceId, callback);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: AddSystemAbility006
 * @tc.desc: test AddSystemAbility, return ERR_OK
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddSystemAbility006, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<IRemoteObject> testAbility(new SaStatusChangeMock());
    SystemAbilityManager::SAExtraProp extraProp;
    saMgr->rpcCallbackImp_ = nullptr;
    int32_t res = saMgr->AddSystemAbility(SOFTBUS_SERVER_SA_ID, testAbility, extraProp);
    saMgr->rpcCallbackImp_ = make_shared<RpcCallbackImp>();
    u16string name = u"test";
    string srcDeviceId = "srcDeviceId";
    saMgr->startingProcessMap_.clear();
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    SystemAbilityManager::AbilityItem abilityItem;
    abilityItem.callbackMap[srcDeviceId].push_back(make_pair(callback, SAID));
    saMgr->startingAbilityMap_[SAID] = abilityItem;
    saMgr->CleanCallbackForLoadFailed(SAID, name, srcDeviceId, callback);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: AddSystemProcess001
 * @tc.desc: test AddSystemProcess, return ERR_INVALID_VALUE
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddSystemProcess001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<IRemoteObject> testAbility(new SaStatusChangeMock());
    u16string procName = u"";
    int32_t res = saMgr->AddSystemProcess(procName, testAbility);
    u16string name = u"test";
    string srcDeviceId = "srcDeviceId";
    saMgr->startingProcessMap_.clear();
    sptr<SystemAbilityLoadCallbackMock> callbackOne = new SystemAbilityLoadCallbackMock();
    sptr<SystemAbilityLoadCallbackMock> callbackTwo = new SystemAbilityLoadCallbackMock();
    SystemAbilityManager::AbilityItem abilityItem;
    abilityItem.callbackMap[srcDeviceId].push_back(make_pair(callbackOne, SAID));
    abilityItem.callbackMap[srcDeviceId].push_back(make_pair(callbackTwo, SAID));
    saMgr->startingAbilityMap_[SAID] = abilityItem;
    saMgr->CleanCallbackForLoadFailed(SAID, name, srcDeviceId, callbackOne);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AddSystemProcess002
 * @tc.desc: test AddSystemProcess, return ERR_INVALID_VALUE
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddSystemProcess002, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<IRemoteObject> testAbility(nullptr);
    u16string procName = u"proname";
    int32_t res = saMgr->AddSystemProcess(procName, testAbility);
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    saMgr->SendLoadedSystemAblityMsg(SAID, testAbility, callback);
    u16string name = u"test";
    string srcDeviceId = "srcDeviceId";
    saMgr->startingProcessMap_.clear();
    sptr<SystemAbilityLoadCallbackMock> callbackOne = new SystemAbilityLoadCallbackMock();
    sptr<SystemAbilityLoadCallbackMock> callbackTwo = new SystemAbilityLoadCallbackMock();
    SystemAbilityManager::AbilityItem abilityItem;
    abilityItem.callbackMap[srcDeviceId].push_back(make_pair(callbackOne, SAID));
    abilityItem.callbackMap[srcDeviceId].push_back(make_pair(callbackTwo, SAID));
    saMgr->startingAbilityMap_[SAID] = abilityItem;
    saMgr->CleanCallbackForLoadFailed(SAID, name, srcDeviceId, callbackTwo);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: RemoveSystemProcess001
 * @tc.desc: test RemoveSystemProcess, return ERR_INVALID_VALUE
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, RemoveSystemProcess001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<IRemoteObject> testAbility(nullptr);
    int32_t res = saMgr->RemoveSystemProcess(testAbility);
    saMgr->NotifySystemAbilityLoadFail(SAID, nullptr);
    u16string name = u"test";
    string srcDeviceId = "srcDeviceId";
    saMgr->startingProcessMap_.clear();
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    SystemAbilityManager::AbilityItem abilityItem;
    saMgr->startingAbilityMap_[SAID] = abilityItem;
    saMgr->CleanCallbackForLoadFailed(SAID, name, srcDeviceId, callback);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: RemoveSystemProcess002
 * @tc.desc: test RemoveSystemProcess, return ERR_OK
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, RemoveSystemProcess002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<IRemoteObject> testAbility(new SaStatusChangeMock());
    u16string procName = u"proname";
    int32_t res = saMgr->AddSystemProcess(procName, testAbility);
    EXPECT_EQ(res, ERR_OK);
    saMgr->systemProcessDeath_ = nullptr;
    res = saMgr->RemoveSystemProcess(testAbility);
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    saMgr->NotifySystemAbilityLoadFail(SAID, callback);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: RemoveSystemProcess003
 * @tc.desc: test RemoveSystemProcess, return ERR_INVALID_VALUE
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, RemoveSystemProcess003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<IRemoteObject> testAbility(new SaStatusChangeMock());
    saMgr->systemProcessMap_.clear();
    int32_t res = saMgr->RemoveSystemProcess(testAbility);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: GetSystemProcess001
 * @tc.desc: test GetSystemProcess, process found
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, GetSystemProcess001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<IRemoteObject> testAbility(new SaStatusChangeMock());
    u16string procName = u"proTest";
    int32_t res = saMgr->AddSystemProcess(procName, testAbility);
    EXPECT_EQ(res, ERR_OK);
    sptr<IRemoteObject> resObj = saMgr->GetSystemProcess(procName);
    EXPECT_NE(resObj, nullptr);
}

/**
 * @tc.name: StartingSystemProcess001
 * @tc.desc: test StartingSystemProcess, process already starting!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, StartingSystemProcess001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    u16string procName = u"proTest";
    saMgr->startingProcessMap_.clear();
    int countNum = 2;
    saMgr->startingProcessMap_[procName] = countNum;
    int32_t res = saMgr->StartingSystemProcess(procName, SAID);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: StartingSystemProcess002
 * @tc.desc: test StartingSystemProcess, return ERR_OK!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, StartingSystemProcess002, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<IRemoteObject> testAbility(new SaStatusChangeMock());
    u16string procName = u"proTest";
    saMgr->startingProcessMap_.clear();
    saMgr->systemProcessMap_[procName] = testAbility;
    int32_t res = saMgr->StartingSystemProcess(procName, SAID);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: LoadSystemAbilityFromRpc001
 * @tc.desc: test LoadSystemAbilityFromRpc, not supported!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, LoadSystemAbilityFromRpc001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    SaProfile saProfile;
    string srcDeviceId;
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    saMgr->saProfileMap_.clear();
    bool res = saMgr->LoadSystemAbilityFromRpc(srcDeviceId, SAID, callback);
    EXPECT_FALSE(res);
}

/**
 * @tc.name: LoadSystemAbilityFromRpc002
 * @tc.desc: test LoadSystemAbilityFromRpc, not distributed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, LoadSystemAbilityFromRpc002, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    SaProfile saProfile;
    saProfile.distributed = false;
    string srcDeviceId;
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    saMgr->saProfileMap_[SAID] = saProfile;
    bool res = saMgr->LoadSystemAbilityFromRpc(srcDeviceId, SAID, callback);
    EXPECT_FALSE(res);
}

/**
 * @tc.name: LoadSystemAbilityFromRpc003
 * @tc.desc: test LoadSystemAbilityFromRpc, return true!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, LoadSystemAbilityFromRpc003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<IRemoteObject> testAbility(new SaStatusChangeMock());
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    SaProfile saProfile;
    saProfile.distributed = true;
    SAInfo saInfo;
    saInfo.remoteObj = testAbility;
    string srcDeviceId;
    saMgr->saProfileMap_[SAID] = saProfile;
    saMgr->abilityMap_[SAID] = saInfo;
    bool res = saMgr->LoadSystemAbilityFromRpc(srcDeviceId, SAID, callback);
    EXPECT_TRUE(res);
}

/**
 * @tc.name: LoadSystemAbilityFromRpc004
 * @tc.desc: test LoadSystemAbilityFromRpc, return true!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, LoadSystemAbilityFromRpc004, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    SaProfile saProfile;
    u16string procName = u"procname";
    int countNum = 2;
    saProfile.distributed = true;
    saProfile.process = procName;
    SAInfo saInfo;
    SystemAbilityManager::AbilityItem abilityItem;
    saInfo.remoteObj = nullptr;
    string srcDeviceId = "srcDeviceId";
    saMgr->saProfileMap_[SAID] = saProfile;
    saMgr->abilityMap_[SAID] = saInfo;
    saMgr->startingAbilityMap_[SAID] = abilityItem;
    saMgr->startingProcessMap_[procName] = countNum;
    bool res = saMgr->LoadSystemAbilityFromRpc(srcDeviceId, SAID, callback);
    EXPECT_TRUE(res);
}

/**
 * @tc.name: LoadSystemAbilityFromRpc005
 * @tc.desc: test LoadSystemAbilityFromRpc, LoadSystemAbility said or callback invalid!!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, LoadSystemAbilityFromRpc005, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<SystemAbilityLoadCallbackMock> callback = nullptr;
    string srcDeviceId = "srcDeviceId";
    bool res = saMgr->LoadSystemAbilityFromRpc(srcDeviceId, SAID, callback);
    EXPECT_FALSE(res);
}

/**
 * @tc.name: LoadSystemAbilityFromRpc006
 * @tc.desc: test LoadSystemAbilityFromRpc, LoadSystemAbility said or callback invalid!!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, LoadSystemAbilityFromRpc006, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    string srcDeviceId = "srcDeviceId";
    bool res = saMgr->LoadSystemAbilityFromRpc(srcDeviceId, INVALID_SAID, callback);
    EXPECT_FALSE(res);
}

/**
 * @tc.name: LoadSystemAbility001
 * @tc.desc: test LoadSystemAbility001, systemAbilityId or callback invalid!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, LoadSystemAbility001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    pair<sptr<ISystemAbilityLoadCallback>, int32_t> itemPair;
    int countNum = 2;
    itemPair.first = callback;
    itemPair.second = SAID;
    saMgr->callbackCountMap_[itemPair.second] = countNum;
    saMgr->RemoveStartingAbilityCallbackLocked(itemPair);
    int32_t res = saMgr->LoadSystemAbility(INVALID_SAID, callback);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: LoadSystemAbility002
 * @tc.desc: test LoadSystemAbility002, systemAbilityId or callback invalid!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, LoadSystemAbility002, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->OnRemoteCallbackDied(nullptr);
    int32_t res = saMgr->LoadSystemAbility(SAID, nullptr);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: LoadSystemAbility003
 * @tc.desc: test LoadSystemAbility003, systemAbilityId or callback invalid!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, LoadSystemAbility003, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    list<sptr<ISystemAbilityLoadCallback>> callbacks;
    callbacks.push_back(callback);
    saMgr->remoteCallbackDeath_ = sptr<IRemoteObject::DeathRecipient>(new RemoteCallbackDeathRecipient());
    saMgr->RemoveRemoteCallbackLocked(callbacks, callback);
    int32_t res = saMgr->LoadSystemAbility(INVALID_SAID, nullptr);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: LoadSystemAbility004
 * @tc.desc: test LoadSystemAbility004, LoadSystemAbility already existed callback object systemAbilityId!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, LoadSystemAbility004, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    SaProfile saProfile;
    SAInfo saInfo;
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    SystemAbilityManager::AbilityItem abilityItem;
    saMgr->saProfileMap_[SAID] = saProfile;
    saMgr->abilityMap_[SAID] = saInfo;
    saMgr->startingAbilityMap_[SAID] = abilityItem;
    abilityItem.callbackMap["local"].push_back(make_pair(callback, SAID));
    int32_t res = saMgr->LoadSystemAbility(SAID, callback);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: LoadSystemAbility005
 * @tc.desc: test LoadSystemAbility005, LoadSystemAbility systemAbilityId!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, LoadSystemAbility005, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    SaProfile saProfile;
    SAInfo saInfo;
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    SystemAbilityManager::AbilityItem abilityItem;
    saMgr->saProfileMap_[SAID] = saProfile;
    saMgr->abilityMap_[SAID] = saInfo;
    saMgr->startingAbilityMap_[SAID] = abilityItem;
    saMgr->abilityCallbackDeath_ = nullptr;
    int32_t res = saMgr->LoadSystemAbility(SAID, callback);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: LoadSystemAbility006
 * @tc.desc: test LoadSystemAbility006, LoadSystemAbility already existed callback!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, LoadSystemAbility006, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    string deviceId = "deviceId";
    saMgr->remoteCallbacks_["1499_deviceId"].push_back(callback);
    int32_t res = saMgr->LoadSystemAbility(SAID, deviceId, callback);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: LoadSystemAbility007
 * @tc.desc: test LoadSystemAbility007, LoadSystemAbility AddDeathRecipient succeed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, LoadSystemAbility007, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    string deviceId = "deviceId";
    saMgr->remoteCallbacks_.clear();
    saMgr->remoteCallbackDeath_ = sptr<IRemoteObject::DeathRecipient>(new RemoteCallbackDeathRecipient());
    int32_t res = saMgr->LoadSystemAbility(SAID, deviceId, callback);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: LoadSystemAbility008
 * @tc.desc: test LoadSystemAbility008, LoadSystemAbility already existed callback!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, LoadSystemAbility008, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    SaProfile saProfile;
    SAInfo saInfo;
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    sptr<SystemAbilityLoadCallbackMock> callbackTwo = new SystemAbilityLoadCallbackMock();
    SystemAbilityManager::AbilityItem abilityItem;
    abilityItem.callbackMap["local"].push_back(make_pair(callback, SAID));
    abilityItem.callbackMap["local"].push_back(make_pair(callbackTwo, SAID));
    saMgr->saProfileMap_[SAID] = saProfile;
    saMgr->startingAbilityMap_[SAID] = abilityItem;
    int32_t res = saMgr->LoadSystemAbility(SAID, callbackTwo);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: DoMakeRemoteBinder001
 * @tc.desc: test DoMakeRemoteBinder, callback is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, DoMakeRemoteBinder001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<SystemAbilityLoadCallbackMock> callback = nullptr;
    string deviceId = "deviceId";
    auto callingPid = IPCSkeleton::GetCallingPid();
    auto callingUid = IPCSkeleton::GetCallingUid();
    saMgr->dBinderService_ = nullptr;
    saMgr->DoLoadRemoteSystemAbility(SAID, callingPid, callingUid, deviceId, callback);
    sptr<DBinderServiceStub> res = saMgr->DoMakeRemoteBinder(SAID, callingPid, callingUid, deviceId);
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: DoMakeRemoteBinder002
 * @tc.desc: test DoMakeRemoteBinder, callback is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, DoMakeRemoteBinder002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    string deviceId = "deviceId";
    auto callingPid = IPCSkeleton::GetCallingPid();
    auto callingUid = IPCSkeleton::GetCallingUid();
    saMgr->dBinderService_ = nullptr;
    saMgr->remoteCallbackDeath_ = sptr<IRemoteObject::DeathRecipient>(new RemoteCallbackDeathRecipient());
    saMgr->DoLoadRemoteSystemAbility(SAID, callingPid, callingUid, deviceId, callback);
    sptr<DBinderServiceStub> res = saMgr->DoMakeRemoteBinder(SAID, callingPid, callingUid, deviceId);
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: TransformDeviceId001
 * @tc.desc: test TransformDeviceId, return string
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, TransformDeviceId001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    string deviceId = "deviceId";
    int32_t type = NODE_ID;
    sptr<IRemoteObject> testAbility(new SaStatusChangeMock());
    saMgr->dBinderService_ = nullptr;
    saMgr->NotifyRpcLoadCompleted(deviceId, SAID, testAbility);
    saMgr->dBinderService_ = DBinderService::GetInstance();
    saMgr->NotifyRpcLoadCompleted(deviceId, SAID, testAbility);
    string res = saMgr->TransformDeviceId(deviceId, type, true);
    EXPECT_EQ(res, "");
}

/**
 * @tc.name: OnRemoteRequest002
 * @tc.desc: test SystemAbilityStatusChangeStub::OnRemoteRequest002
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, OnRemoteRequest002, TestSize.Level3)
{
    sptr<SaStatusChangeMock> testAbility(new SaStatusChangeMock());
    EXPECT_TRUE(testAbility != nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    int32_t code = 1;
    int32_t result = testAbility->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: OnRemoteRequest003
 * @tc.desc: test SystemAbilityStatusChangeStub::OnRemoteRequest003
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, OnRemoteRequest003, TestSize.Level3)
{
    std::u16string StatuschangeToken = u"OHOS.ISystemAbilityStatusChange";
    uint32_t code = 3;
    sptr<SaStatusChangeMock> testAbility(new SaStatusChangeMock());
    EXPECT_TRUE(testAbility != nullptr);
    MessageParcel data;
    data.WriteInterfaceToken(StatuschangeToken);
    MessageParcel reply;
    MessageOption option;
    int32_t result = testAbility->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(result, IPC_STUB_UNKNOW_TRANS_ERR);
}

/**
 * @tc.name: OnRemoteRequest004
 * @tc.desc: test SystemAbilityLoadCallbackStub::OnRemoteRequest004
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, OnRemoteRequest004, TestSize.Level3)
{
    int32_t code = 1;
    sptr<SystemAbilityLoadCallbackMock> testAbility(new SystemAbilityLoadCallbackMock());
    EXPECT_TRUE(testAbility != nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    int32_t result = testAbility->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: OnRemoteRequest005
 * @tc.desc: test SystemAbilityLoadCallbackStub::OnRemoteRequest005
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, OnRemoteRequest005, TestSize.Level3)
{
    std::u16string loadToken = u"OHOS.ISystemAbilityLoadCallback";
    uint32_t code = 4;
    sptr<SystemAbilityLoadCallbackMock> testAbility(new SystemAbilityLoadCallbackMock());
    EXPECT_TRUE(testAbility != nullptr);
    MessageParcel data;
    data.WriteInterfaceToken(loadToken);
    MessageParcel reply;
    MessageOption option;
    int32_t result = testAbility->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(result, IPC_STUB_UNKNOW_TRANS_ERR);
}

/**
 * @tc.name: OnAddSystemAbilityInner001
 * @tc.desc: test SystemAbilityStatusChangeStub::OnAddSystemAbilityInner
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, OnAddSystemAbilityInner001, TestSize.Level3)
{
    sptr<SaStatusChangeMock> testAbility(new SaStatusChangeMock());
    EXPECT_TRUE(testAbility != nullptr);
    MessageParcel data;
    data.WriteInt32(INVALID_SAID);
    MessageParcel reply;
    int32_t result = testAbility->OnAddSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: OnRemoveSystemAbilityInner001
 * @tc.desc: test SystemAbilityStatusChangeStub::OnRemoveSystemAbilityInner001
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, OnRemoveSystemAbilityInner001, TestSize.Level3)
{
    sptr<SaStatusChangeMock> testAbility(new SaStatusChangeMock());
    EXPECT_TRUE(testAbility != nullptr);
    MessageParcel data;
    data.WriteInt32(INVALID_SAID);
    MessageParcel reply;
    int32_t result = testAbility->OnRemoveSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: OnRemoveSystemAbilityInner002
 * @tc.desc: test SystemAbilityStatusChangeStub::OnRemoveSystemAbilityInner002
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, OnRemoveSystemAbilityInner002, TestSize.Level3)
{
    sptr<SaStatusChangeMock> testAbility(new SaStatusChangeMock());
    EXPECT_TRUE(testAbility != nullptr);
    MessageParcel data;
    data.WriteInt32(SAID);
    std::string deviceId = "";
    data.WriteString(deviceId);
    MessageParcel reply;
    int32_t result = testAbility->OnRemoveSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: OnLoadSystemAbilitySuccessInner001
 * @tc.desc: test SystemAbilityLoadCallbackStub::OnLoadSystemAbilitySuccessInner
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, OnLoadSystemAbilitySuccessInner001, TestSize.Level3)
{
    sptr<SystemAbilityLoadCallbackMock> testAbility(new SystemAbilityLoadCallbackMock());
    EXPECT_TRUE(testAbility != nullptr);
    MessageParcel data;
    data.WriteInt32(INVALID_SAID);
    MessageParcel reply;
    int32_t result = testAbility-> OnLoadSystemAbilitySuccessInner(data, reply);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: OnLoadSystemAbilityFailInner001
 * @tc.desc: test SystemAbilityLoadCallbackStub::OnLoadSystemAbilityFailInner
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, OnLoadSystemAbilityFailInner001, TestSize.Level3)
{
    sptr<SystemAbilityLoadCallbackMock> testAbility(new SystemAbilityLoadCallbackMock());
    EXPECT_TRUE(testAbility != nullptr);
    MessageParcel data;
    data.WriteInt32(INVALID_SAID);
    MessageParcel reply;
    int32_t result = testAbility->OnLoadSystemAbilityFailInner(data, reply);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: OnLoadSACompleteForRemoteInner001
 * @tc.desc: test SystemAbilityLoadCallbackStub::OnLoadSACompleteForRemoteInner001
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, OnLoadSACompleteForRemoteInner001, TestSize.Level3)
{
    sptr<SystemAbilityLoadCallbackMock> testAbility(new SystemAbilityLoadCallbackMock());
    EXPECT_TRUE(testAbility != nullptr);
    MessageParcel data;
    data.WriteInt32(INVALID_SAID);
    MessageParcel reply;
    int32_t result = testAbility->OnLoadSACompleteForRemoteInner(data, reply);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}
}