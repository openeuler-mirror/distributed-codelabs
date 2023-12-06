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
#include "system_ability_mgr_proxy_test.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "itest_transaction_service.h"
#include "mock_iro_sendrequest.h"
#include "sam_mock_permission.h"
#include "sa_status_change_mock.h"
#include "system_ability_load_callback_proxy.h"
#include "system_ability_status_change_proxy.h"
#include "string_ex.h"
#include "system_ability_manager_proxy.h"
#include "test_log.h"

#define private public

using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS;

namespace OHOS {
namespace {
constexpr int32_t TEST_ID_NORANGE_SAID = -1;
constexpr int32_t TEST_ID_VAILD = 9999;
constexpr int32_t TEST_ID_INVAILD = 9990;
}
void SystemAbilityMgrProxyTest::SetUpTestCase()
{
    DTEST_LOG << "SetUpTestCase" << std::endl;
}

void SystemAbilityMgrProxyTest::TearDownTestCase()
{
    DTEST_LOG << "TearDownTestCase" << std::endl;
}

void SystemAbilityMgrProxyTest::SetUp()
{
    SamMockPermission::MockPermission();
    DTEST_LOG << "SetUp" << std::endl;
}

void SystemAbilityMgrProxyTest::TearDown()
{
    DTEST_LOG << "TearDown" << std::endl;
}

/**
 * @tc.name: AddSystemProcess001
 * @tc.desc: check add process remoteobject
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, AddSystemProcess001, TestSize.Level1)
{
    DTEST_LOG << " AddSystemProcess001 start " << std::endl;
    /**
     * @tc.steps: step1. get samgr instance
     * @tc.expected: step1. samgr instance not nullptr
     */
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    sptr<IRemoteObject> remote(new TestTransactionService());
    EXPECT_NE(remote, nullptr);
    u16string procName(u"");
    int32_t ret = sm->AddSystemProcess(procName, remote);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AddSystemProcess002
 * @tc.desc: check add process remoteobject
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, AddSystemProcess002, TestSize.Level1)
{
    DTEST_LOG << " AddSystemProcess002 start " << std::endl;
    /**
     * @tc.steps: step1. get samgr instance
     * @tc.expected: step1. samgr instance not nullptr
     */
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    sptr<IRemoteObject> remote(new TestTransactionService());
    EXPECT_NE(remote, nullptr);
    u16string procName(u"test_process");
    int32_t ret = sm->AddSystemProcess(procName, remote);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: CheckSystemAbility001
 * @tc.desc: CheckSystemAbility, systemAbilityId: invalid!
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, CheckSystemAbility001, TestSize.Level1)
{
    DTEST_LOG << " CheckSystemAbility001 start " << std::endl;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    sptr<IRemoteObject> ret = sm->CheckSystemAbility(TEST_ID_NORANGE_SAID);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: CheckSystemAbility002
 * @tc.desc: CheckSystemAbility, CheckSystemAbility:systemAbilityId or deviceId is nullptr
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, CheckSystemAbility002, TestSize.Level1)
{
    DTEST_LOG << " CheckSystemAbility002 start " << std::endl;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    std::string deviceId = "";
    sptr<IRemoteObject> ret = sm->CheckSystemAbility(TEST_ID_NORANGE_SAID, deviceId);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: CheckSystemAbility003
 * @tc.desc: CheckSystemAbility, CheckSystemAbility:systemAbilityId or deviceId is nullptr
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, CheckSystemAbility003, TestSize.Level1)
{
    DTEST_LOG << " CheckSystemAbility003 start " << std::endl;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    std::string deviceId = "";
    sptr<IRemoteObject> ret = sm->CheckSystemAbility(TEST_ID_VAILD, deviceId);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: CheckSystemAbility004
 * @tc.desc: CheckSystemAbility, CheckSystemAbility:systemAbilityId  invalid!
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, CheckSystemAbility004, TestSize.Level1)
{
    DTEST_LOG << " CheckSystemAbility004 start " << std::endl;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    bool isExist = true;
    sptr<IRemoteObject> ret = sm->CheckSystemAbility(TEST_ID_VAILD, isExist);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: CheckSystemAbility005
 * @tc.desc: CheckSystemAbility, CheckSystemAbility:systemAbilityId  invalid!
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, CheckSystemAbility005, TestSize.Level1)
{
    DTEST_LOG << " CheckSystemAbility005 start " << std::endl;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    bool isExist = true;
    sptr<IRemoteObject> ret = sm->CheckSystemAbility(TEST_ID_NORANGE_SAID, isExist);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: AddOnDemandSystemAbilityInfo001
 * @tc.desc: AddOnDemandSystemAbilityInfo, AddOnDemandSystemAbilityInfo invalid params
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, AddOnDemandSystemAbilityInfo001, TestSize.Level1)
{
    DTEST_LOG << " AddOnDemandSystemAbilityInfo001 start " << std::endl;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    std::u16string localAbilityManagerName = u"";
    int32_t ret = sm->AddOnDemandSystemAbilityInfo(TEST_ID_NORANGE_SAID, localAbilityManagerName);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AddOnDemandSystemAbilityInfo002
 * @tc.desc: AddOnDemandSystemAbilityInfo, AddOnDemandSystemAbilityInfo invalid params
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, AddOnDemandSystemAbilityInfo002, TestSize.Level1)
{
    DTEST_LOG << " AddOnDemandSystemAbilityInfo002 start " << std::endl;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    std::u16string localAbilityManagerName = u"";
    int32_t ret = sm->AddOnDemandSystemAbilityInfo(TEST_ID_VAILD, localAbilityManagerName);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: SubscribeSystemAbility001
 * @tc.desc: SubscribeSystemAbility, SubscribeSystemAbility systemAbilityId: or listener invalid!
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, SubscribeSystemAbility001, TestSize.Level1)
{
    DTEST_LOG << " SubscribeSystemAbility001 start " << std::endl;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    int32_t ret = sm->SubscribeSystemAbility(TEST_ID_NORANGE_SAID, nullptr);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: SubscribeSystemAbility002
 * @tc.desc: SubscribeSystemAbility, SubscribeSystemAbility systemAbilityId: or listener invalid!
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, SubscribeSystemAbility002, TestSize.Level1)
{
    DTEST_LOG << " SubscribeSystemAbility002 start " << std::endl;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    int32_t ret = sm->SubscribeSystemAbility(TEST_ID_VAILD, nullptr);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: UnSubscribeSystemAbility001
 * @tc.desc: check UnSubscribeSystemAbility
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, UnSubscribeSystemAbility001, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    sptr<SaStatusChangeMock> callback(new SaStatusChangeMock());
    sm->SubscribeSystemAbility(TEST_ID_VAILD, callback);
    int32_t res = sm->UnSubscribeSystemAbility(TEST_ID_VAILD, callback);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: UnSubscribeSystemAbility002
 * @tc.desc: check UnSubscribeSystemAbility
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, UnSubscribeSystemAbility002, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    sptr<SaStatusChangeMock> callback(new SaStatusChangeMock());
    sm->SubscribeSystemAbility(TEST_ID_VAILD, callback);
    int32_t res = sm->UnSubscribeSystemAbility(TEST_ID_INVAILD, callback);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: UnSubscribeSystemAbility003
 * @tc.desc: check UnSubscribeSystemAbility
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, UnSubscribeSystemAbility003, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    sptr<SaStatusChangeMock> callback(new SaStatusChangeMock());
    int32_t res = sm->UnSubscribeSystemAbility(TEST_ID_NORANGE_SAID, callback);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: UnSubscribeSystemAbility004
 * @tc.desc: check UnSubscribeSystemAbility
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, UnSubscribeSystemAbility004, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    int32_t res = sm->UnSubscribeSystemAbility(TEST_ID_VAILD, nullptr);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: LoadSystemAbility001
 * @tc.desc: check LoadSystemAbility
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, LoadSystemAbility001, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    std::string deviceId = "deviceId";
    sptr<SystemAbilityLoadCallbackMock> callback(new SystemAbilityLoadCallbackMock());
    int32_t res = sm->LoadSystemAbility(TEST_ID_NORANGE_SAID, deviceId, callback);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: LoadSystemAbility002
 * @tc.desc: check LoadSystemAbility
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, LoadSystemAbility002, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    std::string deviceId = "";
    sptr<SystemAbilityLoadCallbackMock> callback(new SystemAbilityLoadCallbackMock());
    int32_t res = sm->LoadSystemAbility(TEST_ID_VAILD, deviceId, callback);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: LoadSystemAbility003
 * @tc.desc: check LoadSystemAbility
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, LoadSystemAbility003, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    std::string deviceId = "deviceId";
    int32_t res = sm->LoadSystemAbility(TEST_ID_VAILD, deviceId, nullptr);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: LoadSystemAbility004
 * @tc.desc: check LoadSystemAbility
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, LoadSystemAbility004, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    std::string deviceId = "deviceId";
    sptr<SystemAbilityLoadCallbackMock> callback(new SystemAbilityLoadCallbackMock());
    int32_t res = sm->LoadSystemAbility(TEST_ID_VAILD, deviceId, callback);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: ServiceRegistry001
 * @tc.desc: check ServiceRegistry
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, ServiceRegistry001, TestSize.Level1)
{
    sptr<IServiceRegistry> sr = ServiceRegistry::GetInstance();
    EXPECT_EQ(sr, nullptr);
}

/**
 * @tc.name: OnLoadSystemAbilitySuccess001
 * @tc.desc: check OnLoadSystemAbilitySuccess001
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnLoadSystemAbilitySuccess001, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<SystemAbilityLoadCallbackProxy> systemAbility(new SystemAbilityLoadCallbackProxy(testAbility));
    systemAbility->OnLoadSystemAbilitySuccess(TEST_ID_NORANGE_SAID, testAbility);
    EXPECT_EQ(testAbility->flag_, false);
}

/**
 * @tc.name: OnLoadSystemAbilitySuccess002
 * @tc.desc: check OnLoadSystemAbilitySuccess002
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnLoadSystemAbilitySuccess002, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<SystemAbilityLoadCallbackProxy> systemAbility(new SystemAbilityLoadCallbackProxy(nullptr));
    systemAbility->OnLoadSystemAbilitySuccess(TEST_ID_VAILD, testAbility);
    EXPECT_EQ(testAbility->flag_, false);
}

/**
 * @tc.name: OnLoadSystemAbilitySuccess003
 * @tc.desc: check OnLoadSystemAbilitySuccess003
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnLoadSystemAbilitySuccess003, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<SystemAbilityLoadCallbackProxy> systemAbility(new SystemAbilityLoadCallbackProxy(testAbility));
    systemAbility->OnLoadSystemAbilitySuccess(TEST_ID_VAILD, nullptr);
    EXPECT_EQ(testAbility->flag_, false);
}

/**
 * @tc.name: OnLoadSystemAbilitySuccess004
 * @tc.desc: check OnLoadSystemAbilitySuccess004
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnLoadSystemAbilitySuccess004, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<SystemAbilityLoadCallbackProxy> systemAbility(new SystemAbilityLoadCallbackProxy(testAbility));
    systemAbility->OnLoadSystemAbilitySuccess(TEST_ID_VAILD, testAbility);
    EXPECT_EQ(testAbility->flag_, true);
}

/**
 * @tc.name: OnLoadSystemAbilitySuccess005
 * @tc.desc: check OnLoadSystemAbilitySuccess005
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnLoadSystemAbilitySuccess005, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    testAbility->result_ = 1;
    sptr<SystemAbilityLoadCallbackProxy> systemAbility(new SystemAbilityLoadCallbackProxy(testAbility));
    systemAbility->OnLoadSystemAbilitySuccess(TEST_ID_VAILD, testAbility);
    EXPECT_EQ(testAbility->flag_, true);
}

/**
 * @tc.name: OnLoadSystemAbilityFail001
 * @tc.desc: check OnLoadSystemAbilityFail001
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnLoadSystemAbilityFail001, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<SystemAbilityLoadCallbackProxy> systemAbility(new SystemAbilityLoadCallbackProxy(testAbility));
    systemAbility->OnLoadSystemAbilityFail(TEST_ID_NORANGE_SAID);
    EXPECT_EQ(testAbility->flag_, false);
}

/**
 * @tc.name: OnLoadSystemAbilityFail002
 * @tc.desc: check OnLoadSystemAbilityFail002
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnLoadSystemAbilityFail002, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<SystemAbilityLoadCallbackProxy> systemAbility(new SystemAbilityLoadCallbackProxy(nullptr));
    systemAbility->OnLoadSystemAbilityFail(TEST_ID_VAILD);
    EXPECT_EQ(testAbility->flag_, false);
}

/**
 * @tc.name: OnLoadSystemAbilityFail003
 * @tc.desc: check OnLoadSystemAbilityFail003
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnLoadSystemAbilityFail003, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<SystemAbilityLoadCallbackProxy> systemAbility(new SystemAbilityLoadCallbackProxy(testAbility));
    systemAbility->OnLoadSystemAbilityFail(TEST_ID_VAILD);
    EXPECT_EQ(testAbility->flag_, true);
}

/**
 * @tc.name: OnLoadSystemAbilityFail004
 * @tc.desc: check OnLoadSystemAbilityFail004
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnLoadSystemAbilityFail004, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    testAbility->result_ = 1;
    sptr<SystemAbilityLoadCallbackProxy> systemAbility(new SystemAbilityLoadCallbackProxy(testAbility));
    systemAbility->OnLoadSystemAbilityFail(TEST_ID_VAILD);
    EXPECT_EQ(testAbility->flag_, true);
}

/**
 * @tc.name: OnLoadSACompleteForRemote001
 * @tc.desc: check OnLoadSACompleteForRemote001
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnLoadSACompleteForRemote001, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<SystemAbilityLoadCallbackProxy> systemAbility(new SystemAbilityLoadCallbackProxy(testAbility));
    string deviceId = "test";
    systemAbility->OnLoadSACompleteForRemote(deviceId, TEST_ID_NORANGE_SAID, testAbility);
    EXPECT_EQ(testAbility->flag_, false);
}

/**
 * @tc.name: OnLoadSACompleteForRemote002
 * @tc.desc: check OnLoadSACompleteForRemote002
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnLoadSACompleteForRemote002, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<SystemAbilityLoadCallbackProxy> systemAbility(new SystemAbilityLoadCallbackProxy(nullptr));
    string deviceId = "test";
    systemAbility->OnLoadSACompleteForRemote(deviceId, TEST_ID_VAILD, testAbility);
    EXPECT_EQ(testAbility->flag_, false);
}

/**
 * @tc.name: OnLoadSACompleteForRemote003
 * @tc.desc: check OnLoadSACompleteForRemote003
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnLoadSACompleteForRemote003, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<SystemAbilityLoadCallbackProxy> systemAbility(new SystemAbilityLoadCallbackProxy(testAbility));
    string deviceId = "test";
    systemAbility->OnLoadSACompleteForRemote(deviceId, TEST_ID_VAILD, nullptr);
    EXPECT_EQ(testAbility->flag_, true);
}

/**
 * @tc.name: OnLoadSACompleteForRemote004
 * @tc.desc: check OnLoadSACompleteForRemote004
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnLoadSACompleteForRemote004, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<SystemAbilityLoadCallbackProxy> systemAbility(new SystemAbilityLoadCallbackProxy(testAbility));
    string deviceId = "test";
    systemAbility->OnLoadSACompleteForRemote(deviceId, TEST_ID_VAILD, testAbility);
    EXPECT_EQ(testAbility->flag_, true);
}

/**
 * @tc.name: OnLoadSACompleteForRemote005
 * @tc.desc: check OnLoadSACompleteForRemote005
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnLoadSACompleteForRemote005, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    testAbility->result_ = 1;
    sptr<SystemAbilityLoadCallbackProxy> systemAbility(new SystemAbilityLoadCallbackProxy(testAbility));
    string deviceId = "test";
    systemAbility->OnLoadSACompleteForRemote(deviceId, TEST_ID_VAILD, testAbility);
    EXPECT_EQ(testAbility->flag_, true);
}

/**
 * @tc.name: OnAddSystemAbility001
 * @tc.desc: check SystemAbilityStatusChangeProxy OnAddSystemAbility001
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnAddSystemAbility001, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<SystemAbilityStatusChangeProxy> systemAbility(new SystemAbilityStatusChangeProxy(testAbility));
    string deviceId = "test";
    systemAbility->OnAddSystemAbility(TEST_ID_NORANGE_SAID, deviceId);
    EXPECT_EQ(testAbility->flag_, false);
}

/**
 * @tc.name: OnAddSystemAbility002
 * @tc.desc: check SystemAbilityStatusChangeProxy OnAddSystemAbility002
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnAddSystemAbility002, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<SystemAbilityStatusChangeProxy> systemAbility(new SystemAbilityStatusChangeProxy(nullptr));
    string deviceId = "test";
    systemAbility->OnAddSystemAbility(TEST_ID_VAILD, deviceId);
    EXPECT_EQ(testAbility->flag_, false);
}

/**
 * @tc.name: OnAddSystemAbility003
 * @tc.desc: check SystemAbilityStatusChangeProxy OnAddSystemAbility003
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnAddSystemAbility003, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<SystemAbilityStatusChangeProxy> systemAbility(new SystemAbilityStatusChangeProxy(testAbility));
    string deviceId = "test";
    systemAbility->OnAddSystemAbility(TEST_ID_VAILD, deviceId);
    EXPECT_EQ(testAbility->flag_, true);
}

/**
 * @tc.name: OnAddSystemAbility004
 * @tc.desc: check SystemAbilityStatusChangeProxy OnAddSystemAbility004
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnAddSystemAbility004, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    testAbility->result_ = 1;
    sptr<SystemAbilityStatusChangeProxy> systemAbility(new SystemAbilityStatusChangeProxy(testAbility));
    string deviceId = "test";
    systemAbility->OnAddSystemAbility(TEST_ID_VAILD, deviceId);
    EXPECT_EQ(testAbility->flag_, true);
}

/**
 * @tc.name: OnRemoveSystemAbility001
 * @tc.desc: check SystemAbilityStatusChangeProxy OnRemoveSystemAbility001
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnRemoveSystemAbility001, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    testAbility->result_ = 1;
    sptr<SystemAbilityStatusChangeProxy> systemAbility(new SystemAbilityStatusChangeProxy(testAbility));
    string deviceId = "test";
    systemAbility->OnRemoveSystemAbility(TEST_ID_VAILD, deviceId);
    EXPECT_EQ(testAbility->flag_, true);
}

/**
 * @tc.name: OnRemoveSystemAbility002
 * @tc.desc: check SystemAbilityStatusChangeProxy OnRemoveSystemAbility002
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnRemoveSystemAbility002, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<SystemAbilityStatusChangeProxy> systemAbility(new SystemAbilityStatusChangeProxy(testAbility));
    string deviceId = "test";
    systemAbility->OnRemoveSystemAbility(TEST_ID_VAILD, deviceId);
    EXPECT_EQ(testAbility->flag_, true);
}

/**
 * @tc.name: DestroySystemAbilityManagerObject001
 * @tc.desc: check  DestroySystemAbilityManagerObject
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, DestroySystemAbilityManagerObject001, TestSize.Level4)
{
    SystemAbilityManagerClient::GetInstance().DestroySystemAbilityManagerObject();
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
}
}
