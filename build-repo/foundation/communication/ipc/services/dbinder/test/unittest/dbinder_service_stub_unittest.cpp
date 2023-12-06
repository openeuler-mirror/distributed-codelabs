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
#include "ipc_types.h"
#include "iremote_object.h"
#include "log_tags.h"
#include "message_parcel.h"
#include "rpc_log.h"
#include "gtest/gtest.h"
#include <iostream>

#define private public
#define protected public
#include "dbinder_service.h"
#include "dbinder_service_stub.h"
#undef protected
#undef private

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::HiviewDFX;

typedef unsigned long long binder_uintptr_t;
class DBinderServiceStubUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DBinderServiceStubUnitTest::SetUp()
{}

void DBinderServiceStubUnitTest::TearDown()
{}

void DBinderServiceStubUnitTest::SetUpTestCase()
{}

void DBinderServiceStubUnitTest::TearDownTestCase()
{}

/**
 * @tc.name: DBinderServiceStub001
 * @tc.desc: Verify the DBinderServiceStub function
 * @tc.type: FUNC
 */
HWTEST_F(DBinderServiceStubUnitTest, DBinderServiceStub001, TestSize.Level1)
{
    const std::string service = "serviceTest";
    const std::string device = "deviceTest";
    binder_uintptr_t object = 11;
    DBinderServiceStub dBinderServiceStub(service, device, object);
}

/**
 * @tc.name: GetServiceName001
 * @tc.desc: Verify the GetServiceName function
 * @tc.type: FUNC
 */
HWTEST_F(DBinderServiceStubUnitTest, GetServiceName001, TestSize.Level1)
{
    const std::string service = "serviceTest";
    const std::string device = "deviceTest";
    binder_uintptr_t object = 11;
    DBinderServiceStub dBinderServiceStub(service, device, object);
    std::string ret = dBinderServiceStub.GetServiceName();
    EXPECT_EQ(ret, "serviceTest");
}

/**
 * @tc.name: GetDeviceID001
 * @tc.desc: Verify the GetDeviceID function
 * @tc.type: FUNC
 */
HWTEST_F(DBinderServiceStubUnitTest, GetDeviceID001, TestSize.Level1)
{
    const std::string service = "serviceTest";
    const std::string device = "deviceTest";
    binder_uintptr_t object = 11;
    DBinderServiceStub dBinderServiceStub(service, device, object);
    std::string ret = dBinderServiceStub.GetDeviceID();
    EXPECT_EQ(ret, "deviceTest");
}

/**
 * @tc.name: GetBinderObject001
 * @tc.desc: Verify the GetBinderObject function
 * @tc.type: FUNC
 */
HWTEST_F(DBinderServiceStubUnitTest, GetBinderObject001, TestSize.Level1)
{
    const std::string service = "serviceTest";
    const std::string device = "deviceTest";
    binder_uintptr_t object = 11;
    DBinderServiceStub dBinderServiceStub(service, device, object);
    binder_uintptr_t ret = dBinderServiceStub.GetBinderObject();
    EXPECT_EQ(ret, 11);
}

/**
 * @tc.name: ProcessProto001
 * @tc.desc: Verify the ProcessProto function
 * @tc.type: FUNC
 */
HWTEST_F(DBinderServiceStubUnitTest, ProcessProto001, TestSize.Level1)
{
    const std::string service = "serviceTest";
    const std::string device = "deviceTest";
    binder_uintptr_t object = 11;
    DBinderServiceStub dBinderServiceStub(service, device, object);
    uint32_t code = 11;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    int32_t ret = dBinderServiceStub.ProcessProto(code, data, reply, option);
    EXPECT_EQ(ret, DBINDER_SERVICE_PROCESS_PROTO_ERR);
}

/**
 * @tc.name: ProcessProto002
 * @tc.desc: Verify the ProcessProto function
 * @tc.type: FUNC
 */
HWTEST_F(DBinderServiceStubUnitTest, ProcessProto002, TestSize.Level1)
{
    const std::string service = "serviceTest";
    const std::string device = "deviceTest";
    binder_uintptr_t object = 11;
    DBinderServiceStub dBinderServiceStub(service, device, object);
    binder_uintptr_t key = reinterpret_cast<binder_uintptr_t>(&dBinderServiceStub);
    sptr<DBinderService> dBinderService = DBinderService::GetInstance();
    std::shared_ptr<struct SessionInfo> sessionInfo = std::make_shared<struct SessionInfo>();
    dBinderService->sessionObject_[key] = sessionInfo;
    uint32_t code = 11;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    int32_t ret = dBinderServiceStub.ProcessProto(code, data, reply, option);
    EXPECT_EQ(ret, DBINDER_SERVICE_PROCESS_PROTO_ERR);
}

/**
 * @tc.name: OnRemoteRequest001
 * @tc.desc: Verify the OnRemoteRequest function
 * @tc.type: FUNC
 */
HWTEST_F(DBinderServiceStubUnitTest, OnRemoteRequest001, TestSize.Level1)
{
    const std::string service = "serviceTest";
    const std::string device = "deviceTest";
    binder_uintptr_t object = 11;
    DBinderServiceStub dBinderServiceStub(service, device, object);

    uint32_t code = GET_PROTO_INFO;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    int32_t ret = dBinderServiceStub.OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, DBINDER_SERVICE_PROCESS_PROTO_ERR);
}

/**
 * @tc.name: OnRemoteRequest002
 * @tc.desc: Verify the OnRemoteRequest function
 * @tc.type: FUNC
 */
HWTEST_F(DBinderServiceStubUnitTest, OnRemoteRequest002, TestSize.Level1)
{
    const std::string service = "serviceTest";
    const std::string device = "deviceTest";
    binder_uintptr_t object = 11;
    DBinderServiceStub dBinderServiceStub(service, device, object);

    uint32_t code = DBINDER_OBITUARY_TRANSACTION;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    int32_t ret = dBinderServiceStub.OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, DBINDER_SERVICE_INVALID_DATA_ERR);
}

/**
 * @tc.name: OnRemoteRequest003
 * @tc.desc: Verify the OnRemoteRequest function
 * @tc.type: FUNC
 */
HWTEST_F(DBinderServiceStubUnitTest, OnRemoteRequest003, TestSize.Level1)
{
    const std::string service = "serviceTest";
    const std::string device = "deviceTest";
    binder_uintptr_t object = 11;
    DBinderServiceStub dBinderServiceStub(service, device, object);

    uint32_t code = 11;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    int32_t ret = dBinderServiceStub.OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, DBINDER_SERVICE_UNKNOW_TRANS_ERR);
}

/**
 * @tc.name: ProcessDeathRecipient001
 * @tc.desc: Verify the ProcessDeathRecipient function
 * @tc.type: FUNC
 */
HWTEST_F(DBinderServiceStubUnitTest, ProcessDeathRecipient001, TestSize.Level1)
{
    const std::string service = "serviceTest";
    const std::string device = "deviceTest";
    binder_uintptr_t object = 11;
    DBinderServiceStub dBinderServiceStub(service, device, object);

    MessageParcel data;
    data.WriteInt32(IRemoteObject::DeathRecipient::ADD_DEATH_RECIPIENT);
    MessageParcel reply;
    int32_t ret = dBinderServiceStub.ProcessDeathRecipient(data, reply);
    EXPECT_EQ(ret, DBINDER_SERVICE_INVALID_DATA_ERR);
}

/**
 * @tc.name: ProcessDeathRecipient002
 * @tc.desc: Verify the ProcessDeathRecipient function
 * @tc.type: FUNC
 */
HWTEST_F(DBinderServiceStubUnitTest, ProcessDeathRecipient002, TestSize.Level1)
{
    const std::string service = "serviceTest";
    const std::string device = "deviceTest";
    binder_uintptr_t object = 11;
    DBinderServiceStub dBinderServiceStub(service, device, object);

    MessageParcel data;
    data.WriteInt32(IRemoteObject::DeathRecipient::REMOVE_DEATH_RECIPIENT);
    MessageParcel reply;
    int32_t ret = dBinderServiceStub.ProcessDeathRecipient(data, reply);
    EXPECT_EQ(ret, DBINDER_SERVICE_REMOVE_DEATH_ERR);
}

/**
 * @tc.name: AddDbinderDeathRecipient001
 * @tc.desc: Verify the AddDbinderDeathRecipient function
 * @tc.type: FUNC
 */
HWTEST_F(DBinderServiceStubUnitTest, AddDbinderDeathRecipient001, TestSize.Level1)
{
    const std::string service = "serviceTest";
    const std::string device = "deviceTest";
    binder_uintptr_t object = 11;
    DBinderServiceStub dBinderServiceStub(service, device, object);
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = dBinderServiceStub.AddDbinderDeathRecipient(data, reply);
    EXPECT_EQ(ret, DBINDER_SERVICE_INVALID_DATA_ERR);
}

/**
 * @tc.name: AddDbinderDeathRecipient002
 * @tc.desc: Verify the AddDbinderDeathRecipient function
 * @tc.type: FUNC
 */
HWTEST_F(DBinderServiceStubUnitTest, AddDbinderDeathRecipient002, TestSize.Level1)
{
    const std::string service = "serviceTest";
    const std::string device = "deviceTest";
    binder_uintptr_t object = 11;
    DBinderServiceStub dBinderServiceStub(service, device, object);

    sptr<IPCObjectStub> callbackStub = new (std::nothrow) IPCObjectStub();
    MessageParcel data;
    data.WriteRemoteObject(callbackStub);
    data.WriteString("");
    MessageParcel reply;
    int32_t ret = dBinderServiceStub.AddDbinderDeathRecipient(data, reply);
    EXPECT_EQ(ret, DBINDER_SERVICE_INVALID_DATA_ERR);
}

/**
 * @tc.name: AddDbinderDeathRecipient003
 * @tc.desc: Verify the AddDbinderDeathRecipient function
 * @tc.type: FUNC
 */
HWTEST_F(DBinderServiceStubUnitTest, AddDbinderDeathRecipient003, TestSize.Level1)
{
    const std::string service = "serviceTest";
    const std::string device = "deviceTest";
    binder_uintptr_t object = 11;
    DBinderServiceStub dBinderServiceStub(service, device, object);

    sptr<IPCObjectStub> callbackStub = new (std::nothrow) IPCObjectStub();
    MessageParcel data;
    data.WriteRemoteObject(callbackStub);
    data.WriteString("test");
    MessageParcel reply;
    int32_t ret = dBinderServiceStub.AddDbinderDeathRecipient(data, reply);
    EXPECT_EQ(ret, DBINDER_SERVICE_ADD_DEATH_ERR);
}

/**
 * @tc.name: AddDbinderDeathRecipient004
 * @tc.desc: Verify the AddDbinderDeathRecipient function
 * @tc.type: FUNC
 */
HWTEST_F(DBinderServiceStubUnitTest, AddDbinderDeathRecipient004, TestSize.Level1)
{
    const std::string service = "serviceTest";
    const std::string device = "deviceTest";
    binder_uintptr_t object = 11;
    DBinderServiceStub dBinderServiceStub(service, device, object);

    sptr<IPCObjectProxy> callbackProxy = new (std::nothrow) IPCObjectProxy(1);
    MessageParcel data;
    data.WriteRemoteObject(callbackProxy);
    data.WriteString("test");
    MessageParcel reply;
    int32_t ret = dBinderServiceStub.AddDbinderDeathRecipient(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

/**
 * @tc.name: RemoveDbinderDeathRecipient001
 * @tc.desc: Verify the RemoveDbinderDeathRecipient function
 * @tc.type: FUNC
 */
HWTEST_F(DBinderServiceStubUnitTest, RemoveDbinderDeathRecipient001, TestSize.Level1)
{
    const std::string service = "serviceTest";
    const std::string device = "deviceTest";
    binder_uintptr_t object = 11;
    DBinderServiceStub dBinderServiceStub(service, device, object);

    MessageParcel data;
    MessageParcel reply;
    int32_t ret = dBinderServiceStub.RemoveDbinderDeathRecipient(data, reply);
    EXPECT_EQ(ret, DBINDER_SERVICE_REMOVE_DEATH_ERR);
}

/**
 * @tc.name: RemoveDbinderDeathRecipient002
 * @tc.desc: Verify the RemoveDbinderDeathRecipient function
 * @tc.type: FUNC
 */
HWTEST_F(DBinderServiceStubUnitTest, RemoveDbinderDeathRecipient002, TestSize.Level1)
{
    const std::string service = "serviceTest";
    const std::string device = "deviceTest";
    binder_uintptr_t object = 11;
    DBinderServiceStub dBinderServiceStub(service, device, object);

    sptr<IPCObjectProxy> callbackProxy = new (std::nothrow) IPCObjectProxy(1);
    MessageParcel data;
    data.WriteRemoteObject(callbackProxy);
    data.WriteString("test");
    MessageParcel reply;
    int32_t ret = dBinderServiceStub.RemoveDbinderDeathRecipient(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}
