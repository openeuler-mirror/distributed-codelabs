/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "UTTest_dm_auth_manager.h"

#include "nlohmann/json.hpp"

#include "dm_log.h"
#include "dm_constants.h"
#include "dm_auth_manager.h"
#include "auth_message_processor.h"
#include "device_manager_service_listener.h"

namespace OHOS {
namespace DistributedHardware {
void DmAuthManagerTest::SetUp()
{
}
void DmAuthManagerTest::TearDown()
{
}
void DmAuthManagerTest::SetUpTestCase()
{
}
void DmAuthManagerTest::TearDownTestCase()
{
}

namespace {
std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
std::shared_ptr<DeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
const int32_t MIN_PIN_CODE = 100000;
const int32_t MAX_PIN_CODE = 999999;

/**
 * @tc.name: DmAuthManager::UnAuthenticateDevice_001
 * @tc.desc: Call unauthenticateddevice to check whether the return value is DM_ FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, UnAuthenticateDevice_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DmAuthManager> authManager =
        std::make_shared<DmAuthManager>(softbusConnector, listener, hiChainConnector_);
    std::shared_ptr<SoftbusSession> sessionSession = std::shared_ptr<SoftbusSession>();
    std::shared_ptr<DmAuthRequestContext> authRequestContext = std::make_shared<DmAuthRequestContext>();
    std::string pkgName = "";
    std::string deviceId = "222";
    int32_t ret = authManager->UnAuthenticateDevice(pkgName, deviceId);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DmAuthManager::HandleAuthenticateTimeout_001
 * @tc.desc: authResponseContext_= nullptr; Call handleauthenticatemeout to check whether return value is ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, HandleAuthenticateTimeout_001, testing::ext::TestSize.Level0)
{
    std::string name = "test";
    std::shared_ptr<DmAuthManager> authManager =
        std::make_shared<DmAuthManager>(softbusConnector, listener, hiChainConnector_);
    std::shared_ptr<AuthRequestState> authRequestState = std::make_shared<AuthRequestNetworkState>();
    authManager->authRequestContext_ = std::make_shared<DmAuthRequestContext>();
    authManager->authRequestState_ = std::make_shared<AuthRequestNetworkState>();
    authManager->authResponseContext_ = nullptr;
    authManager->SetAuthRequestState(authRequestState);
    authManager->HandleAuthenticateTimeout(name);
    ASSERT_TRUE(authManager->authResponseContext_ != nullptr);
}

/**
 * @tc.name: DmAuthManager::HandleAuthenticateTimeout_002
 * @tc.desc: authResponseContext_= nullptr; Call handleauthenticatemeout to check whether the return value is DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, HandleAuthenticateTimeout_002, testing::ext::TestSize.Level0)
{
    std::string name = "test";
    std::shared_ptr<DmAuthManager> authManager =
        std::make_shared<DmAuthManager>(softbusConnector, listener, hiChainConnector_);
    std::shared_ptr<AuthRequestState> authRequestState = std::make_shared<AuthRequestFinishState>();
    authManager->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager->authRequestContext_ = std::make_shared<DmAuthRequestContext>();
    authManager->authRequestState_ = std::make_shared<AuthRequestFinishState>();
    authManager->SetAuthRequestState(authRequestState);
    authManager->HandleAuthenticateTimeout(name);
    ASSERT_TRUE(authManager->authRequestState_ != nullptr);
}

/**
 * @tc.name: DmAuthManager::EstablishAuthChannel_001
 * @tc.desc: Call establishauthchannel to check whether the return value is DM_ FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, EstablishAuthChannel_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DmAuthManager> authManager =
        std::make_shared<DmAuthManager>(softbusConnector, listener, hiChainConnector_);
    std::shared_ptr<SoftbusSession> sessionSession = std::shared_ptr<SoftbusSession>();
    std::shared_ptr<DmAuthResponseContext> authRequestContext = std::make_shared<DmAuthResponseContext>();
    authManager->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager->authRequestContext_ = std::make_shared<DmAuthRequestContext>();
    authManager->authRequestState_ = std::make_shared<AuthRequestFinishState>();
    std::string deviceId1;
    int32_t ret = authManager->EstablishAuthChannel(deviceId1);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: DmAuthManager::StartAuthProcess_001
 * @tc.desc: Whether the return value of calling startauthprocess is ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, StartAuthProcess_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DmAuthManager> authManager =
        std::make_shared<DmAuthManager>(softbusConnector, listener, hiChainConnector_);
    std::shared_ptr<AuthResponseState> authResponseState = std::make_shared<AuthResponseConfirmState>();
    authManager->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager->authRequestContext_ = std::make_shared<DmAuthRequestContext>();
    authManager->authResponseState_ = std::make_shared<AuthResponseConfirmState>();
    authManager->SetAuthResponseState(authResponseState);
    int32_t action = 0;
    authManager->StartAuthProcess(action);
    bool ret = authManager->authRequestContext_->deviceName.empty();
    ASSERT_EQ(ret, true);
}

/**
 * @tc.name: DmAuthManager::StartAuthProcess_002
 * @tc.desc: Whether the return value of calling startauthprocess is DM_ OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, StartAuthProcess_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DmAuthManager> authManager =
        std::make_shared<DmAuthManager>(softbusConnector, listener, hiChainConnector_);
    std::shared_ptr<AuthResponseState> authResponseState = std::make_shared<AuthResponseInitState>();
    authManager->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager->authRequestContext_ = std::make_shared<DmAuthRequestContext>();
    authManager->authResponseState_ = std::make_shared<AuthResponseInitState>();
    authManager->authMessageProcessor_ = std::make_shared<AuthMessageProcessor>(authManager);
    authManager->SetAuthResponseState(authResponseState);
    authManager->authResponseContext_->sessionId = 111;
    authManager->softbusConnector_->GetSoftbusSession()->RegisterSessionCallback(authManager);
    int32_t action = 1;
    int32_t ret = authManager->StartAuthProcess(action);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: DmAuthManager::CreateGroup_001
 * @tc.desc: Whether the return value of calling creategroup is DM_ OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, CreateGroup_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DmAuthManager> authManager =
        std::make_shared<DmAuthManager>(softbusConnector, listener, hiChainConnector_);
    std::shared_ptr<AuthResponseState> authResponseState = std::make_shared<AuthResponseConfirmState>();
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    authManager->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager->authRequestContext_ = std::make_shared<DmAuthRequestContext>();
    authManager->authResponseState_ = std::make_shared<AuthResponseConfirmState>();
    authManager->authMessageProcessor_ = std::make_shared<AuthMessageProcessor>(authManager);
    authManager->SetAuthResponseState(authResponseState);
    authManager->authResponseContext_->requestId = 111;
    authManager->authResponseContext_->groupName = "111";
    int32_t ret = authManager->CreateGroup();
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: DmAuthManager::CreateGroup_002
 * @tc.desc: Whether the return value of calling creategroup is ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, CreateGroup_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DmAuthManager> authManager =
        std::make_shared<DmAuthManager>(softbusConnector, listener, hiChainConnector_);
    authManager->authResponseContext_ = nullptr;
    int32_t ret = authManager->CreateGroup();
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DmAuthManager::AddMember_001
 * @tc.desc: Whether the return value of calling addmember is DM_ OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, AddMember_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DmAuthManager> authManager =
        std::make_shared<DmAuthManager>(softbusConnector, listener, hiChainConnector_);
    std::shared_ptr<AuthResponseState> authResponseState = std::make_shared<AuthResponseInitState>();
    authManager->authRequestContext_ = std::make_shared<DmAuthRequestContext>();
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    authManager->authMessageProcessor_ = std::make_shared<AuthMessageProcessor>(authManager);
    authManager->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    nlohmann::json jsonObject;
    authManager->authResponseContext_->groupId = "111";
    authManager->authResponseContext_->groupName = "222";
    authManager->authResponseContext_->code = 123;
    authManager->authResponseContext_->requestId = 234;
    authManager->authResponseContext_->deviceId = "234";
    int32_t pinCode = 444444;
    authManager->timer_ = std::make_shared<DmTimer>();
    authManager->hiChainConnector_->RegisterHiChainCallback(authManager);
    authManager->SetAuthResponseState(authResponseState);
    int32_t ret = authManager->AddMember(pinCode);
    ASSERT_NE(ret, -1);
}

/**
 * @tc.name: DmAuthManager::JoinNetwork_001
 * @tc.desc: Whether the return value of calling joinnetwork is DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, JoinNetwork_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DmAuthManager> authManager =
        std::make_shared<DmAuthManager>(softbusConnector, listener, hiChainConnector_);
    std::shared_ptr<AuthRequestState> authRequestState = std::make_shared<AuthRequestFinishState>();
    authManager->timer_ = std::make_shared<DmTimer>();
    authManager->authMessageProcessor_ = std::make_shared<AuthMessageProcessor>(authManager);
    authManager->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager->authRequestContext_ = std::make_shared<DmAuthRequestContext>();
    authManager->authRequestState_ = std::make_shared<AuthRequestFinishState>();
    const int32_t sessionId = 65;
    const std::string message = "messageTest";
    int64_t requestId = 444;
    const std::string groupId = "{}";
    int32_t status = 1;
    authManager->OnGroupCreated(requestId, groupId);
    authManager->OnMemberJoin(requestId, status);
    authManager->OnDataReceived(sessionId, message);
    authManager->SetAuthRequestState(authRequestState);
    int32_t ret = authManager->JoinNetwork();
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: DmAuthManager::JoinNetwork_002
 * @tc.desc: Whether the return value of calling joinnetwork is ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, JoinNetwork_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DmAuthManager> authManager =
        std::make_shared<DmAuthManager>(softbusConnector, listener, hiChainConnector_);
    authManager->authResponseContext_ = nullptr;
    int32_t userId = 1;
    bool isShow = false;
    authManager->UserSwitchEventCallback(userId);
    authManager->AuthenticateFinish();
    authManager->CancelDisplay();
    authManager->UpdateInputDialogDisplay(isShow);
    int32_t ret = authManager->JoinNetwork();
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DmAuthManager::SetAuthResponseState_001
 * @tc.desc: Is the authresponsestate assignment successful
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, SetAuthResponseState_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DmAuthManager> authManager =
        std::make_shared<DmAuthManager>(softbusConnector, listener, hiChainConnector_);
    std::shared_ptr<AuthResponseState> authResponseState = std::make_shared<AuthResponseFinishState>();
    authManager->authResponseState_ = std::make_shared<AuthResponseFinishState>();
    authManager->SetAuthResponseState(authResponseState);
    int32_t ret = authManager->SetAuthResponseState(authResponseState);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: DmAuthManager::GetPinCode_001
 * @tc.desc: Return authresponsecontext - > code
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, GetPinCode_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DmAuthManager> authManager =
        std::make_shared<DmAuthManager>(softbusConnector, listener, hiChainConnector_);
    authManager->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager->authResponseContext_->code = 123456;
    int32_t ret = authManager->GetPinCode();
    ASSERT_EQ(ret, 123456);
}

/**
 * @tc.name: DmAuthManager::SetPageId_001
 * @tc.desc: Return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, SetPageId_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DmAuthManager> authManager =
        std::make_shared<DmAuthManager>(softbusConnector, listener, hiChainConnector_);
    int32_t pageId = 123;
    authManager->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    int32_t ret = authManager->SetPageId(pageId);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: DmAuthManager::SetPageId_002
 * @tc.desc: Return ERR_DM_AUTH_NOT_START
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, SetPageId_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DmAuthManager> authManager =
        std::make_shared<DmAuthManager>(softbusConnector, listener, hiChainConnector_);
    int32_t pageId = 123;
    authManager->authResponseContext_ = nullptr;
    authManager->authMessageProcessor_ = nullptr;
    const int32_t sessionId = 65;
    const std::string message = "messageTest";
    int64_t requestId = 555;
    int32_t status = 2;
    authManager->OnMemberJoin(requestId, status);
    authManager->OnDataReceived(sessionId, message);
    int32_t ret = authManager->SetPageId(pageId);
    ASSERT_EQ(ret, ERR_DM_AUTH_NOT_START);
}

/**
 * @tc.name: DmAuthManager::SetReasonAndFinish_001
 * @tc.desc: Return ERR_DM_AUTH_NOT_START
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, SetReasonAndFinish_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DmAuthManager> authManager =
        std::make_shared<DmAuthManager>(softbusConnector, listener, hiChainConnector_);
    const int32_t sessionId = 78;
    int32_t reason = 123;
    int32_t state = 456;
    authManager->OnSessionClosed(sessionId);
    authManager->authResponseContext_ = nullptr;
    int64_t requestId = 333;
    const std::string groupId = "{}";
    authManager->OnGroupCreated(requestId, groupId);
    int32_t ret = authManager->SetReasonAndFinish(reason, state);
    ASSERT_EQ(ret, ERR_DM_AUTH_NOT_START);
}

/**
 * @tc.name: DmAuthManager::SetReasonAndFinish_002
 * @tc.desc: Return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, SetReasonAndFinish_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DmAuthManager> authManager =
        std::make_shared<DmAuthManager>(softbusConnector, listener, hiChainConnector_);
    authManager->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager->authRequestState_ = std::make_shared<AuthRequestFinishState>();
    int32_t reason = 1234;
    int32_t state = 5678;
    int64_t requestId = 22;
    const std::string groupId = "{}";
    authManager->OnGroupCreated(requestId, groupId);
    int32_t ret = authManager->SetReasonAndFinish(reason, state);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: DmAuthManager::SetReasonAndFinish_003
 * @tc.desc: Return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, SetReasonAndFinish_003, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DmAuthManager> authManager =
        std::make_shared<DmAuthManager>(softbusConnector, listener, hiChainConnector_);
    authManager->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager->authRequestState_ = nullptr;
    authManager->authResponseState_ = std::make_shared<AuthResponseFinishState>();
    int32_t reason = 12;
    int32_t state = 36;
    int32_t ret = authManager->SetReasonAndFinish(reason, state);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: DmAuthManager::IsIdenticalAccount_001
 * @tc.desc: Return false
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, IsIdenticalAccount_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DmAuthManager> authManager =
        std::make_shared<DmAuthManager>(softbusConnector, listener, hiChainConnector_);
    bool ret = authManager->IsIdenticalAccount();
    ASSERT_EQ(ret, false);
}

/**
 * @tc.name: DmAuthManager::GeneratePincode_001
 * @tc.desc: Return OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, GeneratePincode_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DmAuthManager> authManager =
        std::make_shared<DmAuthManager>(softbusConnector, listener, hiChainConnector_);
    int32_t openedSessionId = 66;
    int32_t sessionSide = 0;
    int32_t result = 3;
    const int32_t closedSessionId = 11;
    authManager->OnSessionOpened(openedSessionId, sessionSide, result);
    authManager->OnSessionClosed(closedSessionId);
    int32_t ret = authManager->GeneratePincode();
    ASSERT_LE(ret, MAX_PIN_CODE);
    ASSERT_GE(ret, MIN_PIN_CODE);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
