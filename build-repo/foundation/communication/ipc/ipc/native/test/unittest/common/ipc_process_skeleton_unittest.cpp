/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
#include <gmock/gmock.h>

#define private public
#include "dbinder_session_object.h"
#include "mock_session_impl.h"
#include "ipc_process_skeleton.h"
#include "ipc_types.h"
#include "iremote_object.h"
#include "ipc_object_stub.h"
#include "ipc_thread_pool.h"
#include "stub_refcount_object.h"
#undef private

using namespace testing::ext;
using namespace OHOS;

namespace {
constexpr int THREAD_NUM_2 = 2;
constexpr uint32_t INDEX_2 = 2;
constexpr uint32_t INDEX_3 = 3;
constexpr uint32_t INDEX_4 = 4;
constexpr int32_t EXECUTE_TIME_TEST = 500;
}
class IPCProcessSkeletonUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void IPCProcessSkeletonUnitTest::SetUpTestCase()
{
}

void IPCProcessSkeletonUnitTest::TearDownTestCase()
{
}

void IPCProcessSkeletonUnitTest::SetUp() {}

void IPCProcessSkeletonUnitTest::TearDown() {}

/**
 * @tc.name: GetRegistryObjectTest001
 * @tc.desc: Verify the GetRegistryObject function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, GetRegistryObjectTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);
    sptr<IRemoteObject> object = skeleton->GetRegistryObject();
    EXPECT_NE(object, nullptr);
}

/**
 * @tc.name: MakeHandleDescriptorTest001
 * @tc.desc: Verify the MakeHandleDescriptor function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, MakeHandleDescriptorTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    std::u16string str = skeleton->MakeHandleDescriptor(1);
    unsigned int size = 0;
    EXPECT_NE(str.size(), size);
}

/**
 * @tc.name: FindOrNewObjectTest001
 * @tc.desc: Verify the FindOrNewObject function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, FindOrNewObjectTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    sptr<IRemoteObject> object = skeleton->FindOrNewObject(1);
    EXPECT_NE(object, nullptr);
}

/**
 * @tc.name: SetMaxWorkThreadTest001
 * @tc.desc: Verify the SetMaxWorkThread function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, SetMaxWorkThreadTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    bool ret = skeleton->SetMaxWorkThread(-1);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: SetMaxWorkThreadTest002
 * @tc.desc: Verify the SetMaxWorkThread function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, SetMaxWorkThreadTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    bool ret = skeleton->SetMaxWorkThread(1);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: SetRegistryObjectTest001
 * @tc.desc: Verify the SetRegistryObject function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, SetRegistryObjectTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    sptr<IRemoteObject> object = nullptr;
    bool ret = skeleton->SetRegistryObject(object);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: SetRegistryObjectTest002
 * @tc.desc: Verify the SetRegistryObject function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, SetRegistryObjectTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    sptr<IRemoteObject> object = new IPCObjectStub(u"test");
    bool ret = skeleton->SetRegistryObject(object);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: SpawnThreadTest001
 * @tc.desc: Verify the SpawnThread function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, SpawnThreadTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    skeleton->SetMaxWorkThread(1);
    int policy = 1;
    int proto = IRemoteObject::IF_PROT_DATABUS;
    bool ret = skeleton->SpawnThread(policy, proto);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: SpawnThreadTest002
 * @tc.desc: Verify the SpawnThread function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, SpawnThreadTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    skeleton->SetMaxWorkThread(1);
    int policy = 1;
    int proto = IRemoteObject::IF_PROT_ERROR;
    bool ret = skeleton->SpawnThread(policy, proto);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: OnThreadTerminatedTest001
 * @tc.desc: Verify the OnThreadTerminated function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, OnThreadTerminatedTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    skeleton->SetMaxWorkThread(1);
    std::string threadName = "threadname";
    bool ret = skeleton->OnThreadTerminated(threadName);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: QueryObjectTest001
 * @tc.desc: Verify the QueryObject function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, QueryObjectTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    std::u16string descriptor = u"test";
    auto object = skeleton->QueryObject(descriptor);
    EXPECT_EQ(object, nullptr);
}

/**
 * @tc.name: QueryObjectTest002
 * @tc.desc: Verify the QueryObject function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, QueryObjectTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    std::u16string descriptor = u"";
    auto object = skeleton->QueryObject(descriptor);
    EXPECT_EQ(object, nullptr);
}

/**
 * @tc.name: IsContainsObjectTest001
 * @tc.desc: Verify the IsContainsObject function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, IsContainsObjectTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    sptr<IRemoteObject> object = new IPCObjectStub(u"testObject");
    skeleton->AttachObjectInner(object);
    bool ret = skeleton->IsContainsObject(object.GetRefPtr());

    EXPECT_EQ(ret, true);
}

#ifndef CONFIG_IPC_SINGLE
/**
 * @tc.name: IsHandleMadeByUserTest001
 * @tc.desc: Verify the IsHandleMadeByUser function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, IsHandleMadeByUserTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    uint32_t handler = IPCProcessSkeleton::DBINDER_HANDLE_BASE + 1;

    bool ret = skeleton->IsHandleMadeByUser(handler);

    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: IsHandleMadeByUserTest002
 * @tc.desc: Verify the IsHandleMadeByUser function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, IsHandleMadeByUserTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    uint32_t handler = IPCProcessSkeleton::DBINDER_HANDLE_BASE;
    bool ret = skeleton->IsHandleMadeByUser(handler);

    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: GetDBinderIdleHandleTest001
 * @tc.desc: Verify the GetDBinderIdleHandle function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, GetDBinderIdleHandleTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    uint64_t stubIndex = 1;
    uint32_t ret = skeleton->GetDBinderIdleHandle(stubIndex);

    EXPECT_EQ(ret, IPCProcessSkeleton::DBINDER_HANDLE_BASE + 1);
}

/**
 * @tc.name: GetDBinderIdleHandleTest002
 * @tc.desc: Verify the GetDBinderIdleHandle function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, GetDBinderIdleHandleTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    skeleton->dBinderHandle_ = 1;
    uint64_t stubIndex = 1;
    uint32_t ret = skeleton->GetDBinderIdleHandle(stubIndex);

    EXPECT_EQ(ret, IPCProcessSkeleton::DBINDER_HANDLE_BASE + INDEX_2);
}

/**
 * @tc.name: GetDBinderIdleHandleTest003
 * @tc.desc: Verify the GetDBinderIdleHandle function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, GetDBinderIdleHandleTest003, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    skeleton->dBinderHandle_ =
        IPCProcessSkeleton::DBINDER_HANDLE_BASE + IPCProcessSkeleton::DBINDER_HANDLE_BASE + 1;
    uint64_t stubIndex = 1;
    uint32_t ret = skeleton->GetDBinderIdleHandle(stubIndex);

    EXPECT_EQ(ret, IPCProcessSkeleton::DBINDER_HANDLE_BASE + INDEX_3);
}

/**
 * @tc.name: GetDBinderIdleHandleTest004
 * @tc.desc: Verify the GetDBinderIdleHandle function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, GetDBinderIdleHandleTest004, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    skeleton->dBinderHandle_ = IPCProcessSkeleton::DBINDER_HANDLE_BASE;
    uint64_t stubIndex = 1;
    uint32_t ret = skeleton->GetDBinderIdleHandle(stubIndex);

    EXPECT_EQ(ret, IPCProcessSkeleton::DBINDER_HANDLE_BASE + INDEX_4);
}

/**
 * @tc.name: QueryHandleToIndexTest001
 * @tc.desc: Verify the QueryHandleToIndex function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, QueryHandleToIndexTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    skeleton->handleToStubIndex_.insert(std::pair<uint32_t, uint64_t>(1, 1));
    uint64_t handler = 1;
    auto it = skeleton->handleToStubIndex_.find(1);
    if (it != skeleton->handleToStubIndex_.end()) {
        handler = it->second;
    }

    EXPECT_EQ(handler, skeleton->QueryHandleToIndex(1));
}

/**
 * @tc.name: QueryHandleToIndexTest003
 * @tc.desc: Verify the QueryHandleToIndex function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, QueryHandleToIndexTest003, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    std::list<uint32_t> handleList { 1 };
    skeleton->handleToStubIndex_.insert(std::pair<uint32_t, uint64_t>(1, 1));
    uint32_t hd = 1;
    uint64_t ret = skeleton->QueryHandleToIndex(handleList, hd);
    uint64_t handler = 1;
    for (auto it = handleList.begin(); it != handleList.end(); it++) {
        auto mapIndex = skeleton->handleToStubIndex_.find(*it);
        if (mapIndex != skeleton->handleToStubIndex_.end()) {
            hd = mapIndex->first;
            handler = mapIndex->second;
            break;
        }
    }
    EXPECT_EQ(ret, handler);
}

/**
 * @tc.name: ProxyAttachDBinderSessionTest004
 * @tc.desc: Verify the ProxyAttachDBinderSession function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, ProxyAttachDBinderSessionTest004, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    std::string name("nameTest");
    std::string deviceId("deviceIdTest");
    std::shared_ptr<MockSessionImpl> sessionMock = std::make_shared<MockSessionImpl>();
    auto object = std::make_shared<DBinderSessionObject>(sessionMock, name, deviceId);

    uint32_t handler = 1;
    bool ret = skeleton->ProxyAttachDBinderSession(handler, object);

    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: ProxyQueryDBinderSessionTest001
 * @tc.desc: Verify the ProxyQueryDBinderSession function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, ProxyQueryDBinderSessionTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    std::string name("nameTest");
    std::string deviceId("deviceIdTest");
    std::shared_ptr<MockSessionImpl> sessionMock = std::make_shared<MockSessionImpl>();
    auto object = std::make_shared<DBinderSessionObject>(sessionMock, name, deviceId);

    uint32_t handler = 1;
    skeleton->proxyToSession_.insert(
        std::pair<uint32_t, std::shared_ptr<DBinderSessionObject>>(handler, object));
    auto ret = skeleton->ProxyQueryDBinderSession(handler);

    EXPECT_NE(ret, nullptr);
}

/**
 * @tc.name: QueryProxyBySessionHandleTest001
 * @tc.desc: Verify the QueryProxyBySessionHandle function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, QueryProxyBySessionHandleTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    std::string name("nameTest");
    std::string deviceId("deviceIdTest");
    std::shared_ptr<MockSessionImpl> sessionMock = std::make_shared<MockSessionImpl>();
    auto object = std::make_shared<DBinderSessionObject>(sessionMock, name, deviceId);

    uint32_t handler = 1;
    std::vector<uint32_t> proxyHandle { 1 };
    skeleton->proxyToSession_.insert(
        std::pair<uint32_t, std::shared_ptr<DBinderSessionObject>>(handler, object));
    auto ret = skeleton->QueryProxyBySessionHandle(handler, proxyHandle);

    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: QueryHandleByDatabusSessionTest001
 * @tc.desc: Verify the QueryHandleByDatabusSession function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, QueryHandleByDatabusSessionTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    const std::string name("nameTest");
    const std::string deviceId("deviceIdTest");
    uint64_t index = 1;
    skeleton->handleToStubIndex_.insert(std::pair<uint32_t, uint64_t>(1, 1));

    std::shared_ptr<MockSessionImpl> sessionMock = std::make_shared<MockSessionImpl>();
    auto object = std::make_shared<DBinderSessionObject>(sessionMock, name, deviceId);
    uint32_t handler = 1;
    skeleton->proxyToSession_.insert(
        std::pair<uint32_t, std::shared_ptr<DBinderSessionObject>>(handler, object));
    auto ret = skeleton->QueryHandleByDatabusSession(name, deviceId, index);

    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: QuerySessionByInfoTest001
 * @tc.desc: Verify the QuerySessionByInfo function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, QuerySessionByInfoTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    const std::string name("nameTest");
    const std::string deviceId("deviceIdTest");

    std::shared_ptr<MockSessionImpl> sessionMock = std::make_shared<MockSessionImpl>();
    auto object = std::make_shared<DBinderSessionObject>(sessionMock, name, deviceId);
    uint32_t handler = 1;
    skeleton->proxyToSession_.insert(
        std::pair<uint32_t, std::shared_ptr<DBinderSessionObject>>(handler, object));

    auto ret = skeleton->QuerySessionByInfo(name, deviceId);

    EXPECT_NE(ret, nullptr);
}

/**
 * @tc.name: QueryThreadLockInfoTest001
 * @tc.desc: Verify the SetRegistryObject function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, QueryThreadLockInfoTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    std::shared_ptr<SocketThreadLockInfo> object =
        std::make_shared<SocketThreadLockInfo>();
    std::thread::id threadId;
    skeleton->AttachThreadLockInfo(object, threadId);

    auto ret = skeleton->QueryThreadLockInfo(threadId);
    EXPECT_NE(ret, nullptr);
}

/**
 * @tc.name: DeleteDataThreadFromIdleTest001
 * @tc.desc: Verify the DeleteDataThreadFromIdle function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, DeleteDataThreadFromIdleTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    std::thread::id threadId;
    skeleton->AddDataThreadToIdle(threadId);

    bool ret = skeleton->DeleteDataThreadFromIdle(threadId);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: GetIdleDataThreadTest001
 * @tc.desc: Verify the GetIdleDataThread function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, GetIdleDataThreadTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    std::thread::id threadId;
    skeleton->AddDataThreadToIdle(threadId);
    auto ret = skeleton->GetIdleDataThread();
    EXPECT_EQ(threadId, ret);
}

/**
 * @tc.name: GetSocketIdleThreadNumTest001
 * @tc.desc: Verify the GetSocketIdleThreadNum function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, GetSocketIdleThreadNumTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);
    skeleton->SetMaxWorkThread(THREAD_NUM_2);
    ASSERT_TRUE(skeleton->threadPool_ != nullptr);

    int num = skeleton->threadPool_->GetSocketIdleThreadNum();
    int ret = skeleton->GetSocketIdleThreadNum();
    EXPECT_EQ(num, ret);
}

/**
 * @tc.name: GetSocketTotalThreadNumTest002
 * @tc.desc: Verify the GetSocketTotalThreadNum function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, GetSocketTotalThreadNumTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);
    skeleton->SetMaxWorkThread(THREAD_NUM_2);
    ASSERT_TRUE(skeleton->threadPool_ != nullptr);

    int ret = skeleton->GetSocketTotalThreadNum();
    int num = skeleton->threadPool_->GetSocketTotalThreadNum();
    EXPECT_EQ(num, ret);
}

/**
 * @tc.name: PopDataInfoFromThreadTest001
 * @tc.desc: Verify the PopDataInfoFromThread function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, PopDataInfoFromThreadTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    std::thread::id threadId;
    std::shared_ptr<ThreadProcessInfo> processInfo =
        std::make_shared<ThreadProcessInfo>();
    skeleton->AddDataInfoToThread(threadId, processInfo);

    auto ret = skeleton->PopDataInfoFromThread(threadId);
    EXPECT_NE(ret, nullptr);
}

/**
 * @tc.name: GetSeqNumberTest001
 * @tc.desc: Verify the GetSeqNumber function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, GetSeqNumberTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    skeleton->seqNumber_ = 0;
    uint64_t num = skeleton->GetSeqNumber();
    ASSERT_TRUE(num != 0);
}

/**
 * @tc.name: GetSeqNumberTest002
 * @tc.desc: Verify the GetSeqNumber function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, GetSeqNumberTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    skeleton->seqNumber_ = std::numeric_limits<uint64_t>::max();
    uint64_t num = skeleton->GetSeqNumber();
    ASSERT_TRUE(num != 0);
}

/**
 * @tc.name: QueryThreadBySeqNumberTest001
 * @tc.desc: Verify the QueryThreadBySeqNumber function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, QueryThreadBySeqNumberTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    uint64_t seqNumber = 1;
    std::shared_ptr<ThreadMessageInfo> messageInfo =
        std::make_shared<ThreadMessageInfo>();
    skeleton->AddThreadBySeqNumber(seqNumber, messageInfo);
    auto ret = skeleton->QueryThreadBySeqNumber(seqNumber);
    ASSERT_TRUE(ret != nullptr);
}

/**
 * @tc.name: WakeUpThreadBySeqNumberTest001
 * @tc.desc: Verify the WakeUpThreadBySeqNumber function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, WakeUpThreadBySeqNumberTest001, TestSize.Level1)
{
    uint64_t seqNumber = 1;
    uint32_t handler = 1;
    auto process = [](int32_t timeout) {
        std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
    };
    std::thread thread1(process, EXECUTE_TIME_TEST);
    thread1.detach();
    std::thread::id t1ID = thread1.get_id();

    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    skeleton->seqNumberToThread_.clear();

    auto messageInfo = std::make_shared<ThreadMessageInfo>();
    messageInfo->socketId = handler;
    messageInfo->threadId = t1ID;
    skeleton->AddThreadBySeqNumber(seqNumber, messageInfo);
    
    std::shared_ptr<SocketThreadLockInfo> object =
        std::make_shared<SocketThreadLockInfo>();
    skeleton->threadLockInfo_.clear();
    skeleton->AttachThreadLockInfo(object, t1ID);

    skeleton->WakeUpThreadBySeqNumber(seqNumber, handler);
    ASSERT_TRUE(skeleton->threadLockInfo_.size() != 0);
}

/**
 * @tc.name: WakeUpThreadBySeqNumberTest002
 * @tc.desc: Verify the WakeUpThreadBySeqNumber function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, WakeUpThreadBySeqNumberTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    uint64_t seqNumber = 1;
    uint32_t handler = 1;
    skeleton->seqNumberToThread_.clear();
    skeleton->WakeUpThreadBySeqNumber(seqNumber, handler);
    ASSERT_TRUE(skeleton->QueryThreadBySeqNumber(seqNumber) == nullptr);
}

/**
 * @tc.name: WakeUpThreadBySeqNumberTest003
 * @tc.desc: Verify the WakeUpThreadBySeqNumber function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, WakeUpThreadBySeqNumberTest003, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    skeleton->seqNumberToThread_.clear();
    uint64_t seqNumber = 1;
    uint32_t handler = 1;
    auto messageInfo = std::make_shared<ThreadMessageInfo>();
    messageInfo->socketId = 0;
    skeleton->AddThreadBySeqNumber(seqNumber, messageInfo);
    skeleton->WakeUpThreadBySeqNumber(seqNumber, handler);

    ASSERT_TRUE(skeleton->QueryThreadBySeqNumber(seqNumber) != nullptr);
}

/**
 * @tc.name: WakeUpThreadBySeqNumberTest004
 * @tc.desc: Verify the WakeUpThreadBySeqNumber function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, WakeUpThreadBySeqNumberTest004, TestSize.Level1)
{
    std::thread::id t1ID = std::thread::id();

    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    skeleton->seqNumberToThread_.clear();
    uint64_t seqNumber = 1;
    uint32_t handler = 1;
    auto messageInfo = std::make_shared<ThreadMessageInfo>();
    messageInfo->socketId = handler;
    messageInfo->threadId = t1ID;
    skeleton->AddThreadBySeqNumber(seqNumber, messageInfo);
    
    skeleton->threadLockInfo_.clear();
    std::shared_ptr<SocketThreadLockInfo> object =
        std::make_shared<SocketThreadLockInfo>();
    skeleton->AttachThreadLockInfo(object, t1ID);

    skeleton->WakeUpThreadBySeqNumber(seqNumber, handler);
    ASSERT_TRUE(skeleton->threadLockInfo_.size() != 0);
}

/**
 * @tc.name: WakeUpThreadBySeqNumberTest005
 * @tc.desc: Verify the WakeUpThreadBySeqNumber function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, WakeUpThreadBySeqNumberTest005, TestSize.Level1)
{
    uint64_t seqNumber = 1;
    uint32_t handler = 1;
    auto process = [](int32_t timeout) {
        std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
    };
    std::thread thread1(process, EXECUTE_TIME_TEST);
    thread1.detach();
    std::thread::id t1ID = thread1.get_id();

    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    skeleton->seqNumberToThread_.clear();

    auto messageInfo = std::make_shared<ThreadMessageInfo>();
    messageInfo->socketId = handler;
    messageInfo->threadId = t1ID;
    skeleton->AddThreadBySeqNumber(seqNumber, messageInfo);
    
    skeleton->threadLockInfo_.clear();
    std::shared_ptr<SocketThreadLockInfo> object = nullptr;
    skeleton->AttachThreadLockInfo(object, t1ID);

    skeleton->WakeUpThreadBySeqNumber(seqNumber, handler);
    ASSERT_TRUE(skeleton->threadLockInfo_.size() != 0);
}

/**
 * @tc.name: AddSendThreadInWaitTest001
 * @tc.desc: Verify the AddSendThreadInWait function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, AddSendThreadInWaitTest001, TestSize.Level1)
{
    uint64_t seqNumber = 1;
    int userWaitTime = 1;
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    skeleton->seqNumberToThread_.clear();
    auto messageInfo = std::make_shared<ThreadMessageInfo>();
    messageInfo->socketId = 1;
    skeleton->AddThreadBySeqNumber(seqNumber, messageInfo);

    bool ret = skeleton->AddSendThreadInWait(seqNumber, messageInfo, userWaitTime);
    ASSERT_TRUE(ret == false);
}

/**
 * @tc.name: AddSendThreadInWaitTest002
 * @tc.desc: Verify the AddSendThreadInWait function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, AddSendThreadInWaitTest002, TestSize.Level1)
{
    uint64_t seqNumber = 1;
    auto process = [](int32_t timeout) {
        std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
    };
    std::thread thread1(process, EXECUTE_TIME_TEST);
    std::thread::id t1ID = thread1.get_id();

    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    auto messageInfo = std::make_shared<ThreadMessageInfo>();
    messageInfo->socketId = 1;
    messageInfo->threadId = t1ID;
    skeleton->seqNumberToThread_.clear();
    skeleton->AddThreadBySeqNumber(seqNumber, messageInfo);

    std::shared_ptr<SocketThreadLockInfo> object =
        std::make_shared<SocketThreadLockInfo>();
    skeleton->threadLockInfo_.clear();
    skeleton->AttachThreadLockInfo(object, t1ID);

    int userWaitTime = 1;
    bool ret = skeleton->AddSendThreadInWait(seqNumber, messageInfo, userWaitTime);
    ASSERT_TRUE(ret == false);
    thread1.join();
}

/**
 * @tc.name: AddSendThreadInWaitTest003
 * @tc.desc: Verify the AddSendThreadInWait function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, AddSendThreadInWaitTest003, TestSize.Level1)
{
    uint64_t seqNumber = 1;
    auto process = [](int32_t timeout) {
        std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
    };
    std::thread thread1(process, EXECUTE_TIME_TEST);
    std::thread::id t1ID = thread1.get_id();

    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    auto messageInfo = std::make_shared<ThreadMessageInfo>();
    messageInfo->socketId = 1;
    messageInfo->threadId = t1ID;
    skeleton->seqNumberToThread_.clear();
    skeleton->AddThreadBySeqNumber(seqNumber, messageInfo);
    skeleton->threadLockInfo_.clear();

    int userWaitTime = 1;
    bool ret = skeleton->AddSendThreadInWait(seqNumber, messageInfo, userWaitTime);
    ASSERT_TRUE(ret == false);
    thread1.join();
}

/**
 * @tc.name: AddSendThreadInWaitTest004
 * @tc.desc: Verify the AddSendThreadInWait function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, AddSendThreadInWaitTest004, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    uint64_t seqNumber = 1;
    auto process = [skeleton](int32_t timeout) {
        std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
        skeleton->WakeUpThreadBySeqNumber(1, 1);
    };
    std::thread thread1(process, EXECUTE_TIME_TEST);
    std::thread::id t1ID = thread1.get_id();


    auto messageInfo = std::make_shared<ThreadMessageInfo>();
    messageInfo->socketId = 1;
    messageInfo->threadId = t1ID;
    skeleton->seqNumberToThread_.clear();
    skeleton->AddThreadBySeqNumber(seqNumber, messageInfo);
    skeleton->threadLockInfo_.clear();

    int userWaitTime = 1;
    bool ret = skeleton->AddSendThreadInWait(seqNumber, messageInfo, userWaitTime);
    thread1.join();
    ASSERT_TRUE(ret == false);
}

/**
 * @tc.name: QueryStubByIndexTest001
 * @tc.desc: Verify the QueryStubByIndex function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, QueryStubByIndexTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    uint64_t stubIndex = 1;
    skeleton->stubObjects_.clear();
    sptr<IRemoteObject> stubObject = new IPCObjectStub(u"testObject");
    skeleton->stubObjects_.insert(
        std::pair<uint64_t, IRemoteObject *>(stubIndex, stubObject.GetRefPtr()));
    auto ret = skeleton->QueryStubByIndex(stubIndex);
    EXPECT_NE(ret, nullptr);
    skeleton->stubObjects_.clear();
}

/**
 * @tc.name: QueryStubByIndexTest002
 * @tc.desc: Verify the QueryStubByIndex function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, QueryStubByIndexTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    uint64_t stubIndex = 1;
    skeleton->stubObjects_.clear();
    auto ret = skeleton->QueryStubByIndex(stubIndex);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: AddStubByIndexTest001
 * @tc.desc: Verify the AddStubByIndex function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, AddStubByIndexTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    uint64_t stubIndex = 1;
    skeleton->stubObjects_.clear();

    sptr<IRemoteObject> stubObject = new IPCObjectStub(u"testObject");
    skeleton->stubObjects_.insert(
        std::pair<uint64_t, IRemoteObject *>(stubIndex, stubObject.GetRefPtr()));

    auto ret = skeleton->AddStubByIndex(stubObject.GetRefPtr());
    EXPECT_EQ(ret, stubIndex);
}

/**
 * @tc.name: AddStubByIndexTest002
 * @tc.desc: Verify the AddStubByIndex function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, AddStubByIndexTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    uint64_t stubIndex = 1;
    skeleton->stubObjects_.clear();

    sptr<IRemoteObject> stubObject = new IPCObjectStub(u"testObject");
    skeleton->stubObjects_.insert(
        std::pair<uint64_t, IRemoteObject *>(stubIndex, stubObject.GetRefPtr()));

    sptr<IRemoteObject> object = new IPCObjectStub(u"bject");
    skeleton->randNum_ = 0;
    uint64_t index = skeleton->AddStubByIndex(object.GetRefPtr());
    uint64_t ret = 0;
    EXPECT_EQ(ret, index);
}

/**
 * @tc.name: AddStubByIndexTest003
 * @tc.desc: Verify the AddStubByIndex function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, AddStubByIndexTest003, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    uint64_t stubIndex = 1;
    skeleton->stubObjects_.clear();

    sptr<IRemoteObject> stubObject = new IPCObjectStub(u"testObject");
    skeleton->stubObjects_.insert(
        std::pair<uint64_t, IRemoteObject *>(stubIndex, stubObject.GetRefPtr()));

    sptr<IRemoteObject> object = new IPCObjectStub(u"bject");
    skeleton->randNum_ = 10;
    uint64_t index = skeleton->AddStubByIndex(object.GetRefPtr());
    uint64_t ret = 10;
    EXPECT_EQ(ret, index);
}

/**
 * @tc.name: EraseStubIndexTest001
 * @tc.desc: Verify the EraseStubIndex function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, EraseStubIndexTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    skeleton->stubObjects_.clear();
    uint64_t stubIndex = 1;
    sptr<IRemoteObject> stubObject = new IPCObjectStub(u"testObject");
    skeleton->stubObjects_.insert(
        std::pair<uint64_t, IRemoteObject *>(stubIndex, stubObject.GetRefPtr()));

    uint64_t ret = skeleton->EraseStubIndex(stubObject.GetRefPtr());
    EXPECT_EQ(ret, stubIndex);
    skeleton->stubObjects_.clear();
}

/**
 * @tc.name: EraseStubIndexTest002
 * @tc.desc: Verify the EraseStubIndex function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, EraseStubIndexTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    skeleton->stubObjects_.clear();
    sptr<IRemoteObject> stubObject = new IPCObjectStub(u"testObject");

    uint64_t ret = skeleton->EraseStubIndex(stubObject.GetRefPtr());
    uint64_t index = 0;
    EXPECT_EQ(ret, index);
}

/**
 * @tc.name: DetachAppInfoToStubIndexTest001
 * @tc.desc: Verify the DetachAppInfoToStubIndex function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, DetachAppInfoToStubIndexTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    skeleton->appInfoToStubIndex_.clear();
    uint32_t pid = 1;
    uint32_t uid = 1;
    std::string deviceId = "testDeviceId";
    uint64_t stubIndex = 1;
    std::string appInfo = deviceId + std::to_string(pid) + std::to_string(uid);
    std::map<uint64_t, bool> indexMap = {
        { stubIndex, true }
    };
    skeleton->appInfoToStubIndex_[appInfo] = indexMap;

    bool ret = skeleton->DetachAppInfoToStubIndex(pid, uid, deviceId, stubIndex);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: DetachAppInfoToStubIndexTest002
 * @tc.desc: Verify the DetachAppInfoToStubIndex function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, DetachAppInfoToStubIndexTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    skeleton->appInfoToStubIndex_.clear();
    uint32_t pid = 1;
    uint32_t uid = 1;
    std::string deviceId = "testDeviceId";
    uint64_t stubIndex = 1;
    uint64_t index = 0;
    std::string appInfo = deviceId + std::to_string(pid) + std::to_string(uid);
    std::map<uint64_t, bool> indexMap = {
        { stubIndex, true }
    };
    skeleton->appInfoToStubIndex_[appInfo] = indexMap;

    bool ret = skeleton->DetachAppInfoToStubIndex(pid, uid, deviceId, index);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: DetachAppInfoToStubIndexTest003
 * @tc.desc: Verify the DetachAppInfoToStubIndex function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, DetachAppInfoToStubIndexTest003, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    skeleton->appInfoToStubIndex_.clear();
    uint32_t pid = 1;
    uint32_t uid = 1;
    std::string deviceId = "testDeviceId";
    uint64_t index = 0;

    bool ret = skeleton->DetachAppInfoToStubIndex(pid, uid, deviceId, index);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: DetachAppInfoToStubIndexTest004
 * @tc.desc: Verify the DetachAppInfoToStubIndex function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, DetachAppInfoToStubIndexTest004, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    skeleton->appInfoToStubIndex_.clear();
    uint32_t pid = 1;
    uint32_t uid = 1;
    std::string deviceId = "testDeviceId";
    uint64_t stubIndex = 1;
    std::string appInfo = deviceId + std::to_string(pid) + std::to_string(uid);
    std::map<uint64_t, bool> indexMap = {
        { stubIndex, true }
    };
    skeleton->appInfoToStubIndex_[appInfo] = indexMap;

    skeleton->DetachAppInfoToStubIndex(stubIndex);
    auto index = skeleton->appInfoToStubIndex_[appInfo];
    ASSERT_TRUE(index.size() == 0);
}

/**
 * @tc.name: DetachAppInfoToStubIndexTest005
 * @tc.desc: Verify the DetachAppInfoToStubIndex function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, DetachAppInfoToStubIndexTest005, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    skeleton->appInfoToStubIndex_.clear();
    uint32_t pid = 1;
    uint32_t uid = 1;
    std::string deviceId = "testDeviceId";
    uint64_t stubIndex = 1;
    std::string appInfo = deviceId + std::to_string(pid) + std::to_string(uid);
    std::map<uint64_t, bool> indexMap = {
        { 0, true },
        { 1, true },
    };
    skeleton->appInfoToStubIndex_[appInfo] = indexMap;

    skeleton->DetachAppInfoToStubIndex(stubIndex);
    auto index = skeleton->appInfoToStubIndex_[appInfo];
    ASSERT_TRUE(index.size() != 0);
}

/**
 * @tc.name: AttachAppInfoToStubIndexTest001
 * @tc.desc: Verify the AttachAppInfoToStubIndex function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, AttachAppInfoToStubIndexTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    skeleton->appInfoToStubIndex_.clear();
    uint32_t pid = 1;
    uint32_t uid = 1;
    std::string deviceId = "testDeviceId";
    uint64_t stubIndex = 1;
    std::string appInfo = deviceId + std::to_string(pid) + std::to_string(uid);
    std::map<uint64_t, bool> indexMap = {
        { stubIndex, true }
    };
    skeleton->appInfoToStubIndex_[appInfo] = indexMap;

    bool ret = skeleton->AttachAppInfoToStubIndex(pid, uid, deviceId, stubIndex);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: AttachAppInfoToStubIndexTest002
 * @tc.desc: Verify the AttachAppInfoToStubIndex function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, AttachAppInfoToStubIndexTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    skeleton->appInfoToStubIndex_.clear();
    uint32_t pid = 1;
    uint32_t uid = 1;
    std::string deviceId = "testDeviceId";
    uint64_t stubIndex = 1;
    std::string appInfo = deviceId + std::to_string(pid) + std::to_string(uid);
    std::map<uint64_t, bool> indexMap = {
        { 0, true }
    };
    skeleton->appInfoToStubIndex_[appInfo] = indexMap;

    bool ret = skeleton->AttachAppInfoToStubIndex(pid, uid, deviceId, stubIndex);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: QueryAppInfoToStubIndexTest001
 * @tc.desc: Verify the QueryAppInfoToStubIndex function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, QueryAppInfoToStubIndexTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    skeleton->appInfoToStubIndex_.clear();
    uint32_t pid = 1;
    uint32_t uid = 1;
    std::string deviceId = "testDeviceId";
    uint64_t stubIndex = 1;
    std::string appInfo = deviceId + std::to_string(pid) + std::to_string(uid);
    std::map<uint64_t, bool> indexMap = {
        { 0, true }
    };
    skeleton->appInfoToStubIndex_[appInfo] = indexMap;

    bool ret = skeleton->QueryAppInfoToStubIndex(pid, uid, deviceId, stubIndex);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: QueryAppInfoToStubIndexTest002
 * @tc.desc: Verify the QueryAppInfoToStubIndex function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, QueryAppInfoToStubIndexTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    skeleton->appInfoToStubIndex_.clear();
    uint32_t pid = 1;
    uint32_t uid = 1;
    std::string deviceId = "testDeviceId";
    uint64_t stubIndex = 1;
    std::string appInfo = deviceId + std::to_string(pid) + std::to_string(uid);
    std::map<uint64_t, bool> indexMap = {
        { stubIndex, true }
    };
    skeleton->appInfoToStubIndex_[appInfo] = indexMap;

    bool ret = skeleton->QueryAppInfoToStubIndex(pid, uid, deviceId, stubIndex);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: DetachCallbackStubTest001
 * @tc.desc: Verify the DetachCallbackStub function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, DetachCallbackStubTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);
    skeleton->noticeStub_.clear();

    sptr<IPCObjectStub> objectStub = new IPCObjectStub(u"testObject");
    sptr<IPCObjectProxy> objectProxy = new IPCObjectProxy(1);
    skeleton->noticeStub_[objectProxy.GetRefPtr()] = objectStub;
    bool ret = skeleton->DetachCallbackStub(objectStub.GetRefPtr());
    EXPECT_EQ(ret, true);
    skeleton->noticeStub_.clear();
}

/**
 * @tc.name: DetachCallbackStubTest002
 * @tc.desc: Verify the DetachCallbackStub function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, DetachCallbackStubTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    sptr<IPCObjectStub> objectStub = new IPCObjectStub(u"testObject");
    skeleton->noticeStub_.clear();
    bool ret = skeleton->DetachCallbackStub(objectStub.GetRefPtr());
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: QueryCallbackProxyTest001
 * @tc.desc: Verify the QueryCallbackProxy function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, QueryCallbackProxyTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);
    skeleton->noticeStub_.clear();

    sptr<IPCObjectProxy> objectProxy = new IPCObjectProxy(1);
    sptr<IPCObjectStub> objectStub = new IPCObjectStub(u"testObject");
    skeleton->noticeStub_[objectProxy.GetRefPtr()] = objectStub;
    auto ret = skeleton->QueryCallbackProxy(objectStub.GetRefPtr());
    EXPECT_NE(ret, nullptr);
    skeleton->noticeStub_.clear();
}

/**
 * @tc.name: QueryCallbackProxyTest002
 * @tc.desc: Verify the QueryCallbackProxy function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, QueryCallbackProxyTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);
    skeleton->noticeStub_.clear();

    sptr<IPCObjectProxy> objectProxy = new IPCObjectProxy(1);
    sptr<IPCObjectStub> objectStub = new IPCObjectStub(u"testObject");
    skeleton->noticeStub_[objectProxy.GetRefPtr()] = objectStub;
    auto ret = skeleton->QueryCallbackProxy(objectStub.GetRefPtr());
    EXPECT_NE(ret, nullptr);
    skeleton->noticeStub_.clear();
}

/**
 * @tc.name: QueryCallbackProxyTest003
 * @tc.desc: Verify the QueryCallbackProxy function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, QueryCallbackProxyTest003, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);
    skeleton->noticeStub_.clear();

    sptr<IPCObjectProxy> objectProxy = new IPCObjectProxy(1);
    sptr<IPCObjectStub> objectStub = new IPCObjectStub(u"testObject");
    skeleton->noticeStub_[objectProxy.GetRefPtr()] = objectStub;
    auto ret = skeleton->QueryCallbackProxy(objectStub.GetRefPtr());
    EXPECT_NE(ret, nullptr);
    skeleton->noticeStub_.clear();
}

/**
 * @tc.name: CreateSoftbusServerTest001
 * @tc.desc: Verify the CreateSoftbusServer function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, CreateSoftbusServerTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    std::string name = "test";
    auto ret = skeleton->CreateSoftbusServer(name);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: CreateSoftbusServerTest002
 * @tc.desc: Verify the CreateSoftbusServer function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, CreateSoftbusServerTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    std::string name = "";
    auto ret = skeleton->CreateSoftbusServer(name);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: QueryRawDataTest001
 * @tc.desc: Verify the QueryRawData function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, QueryRawDataTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    uint32_t fd = 1;
    skeleton->rawData_.clear();
    skeleton->rawData_[fd] = std::make_shared<InvokerRawData>(1);

    auto ret = skeleton->QueryRawData(fd);
    EXPECT_NE(ret, nullptr);
}

/**
 * @tc.name: QueryRawDataTest002
 * @tc.desc: Verify the QueryRawData function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, QueryRawDataTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    uint32_t fd = 1;
    skeleton->rawData_.clear();
    auto ret = skeleton->QueryRawData(fd);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: AttachStubRecvRefInfoTest001
 * @tc.desc: Verify the AttachStubRecvRefInfo function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, AttachStubRecvRefInfoTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    skeleton->stubRecvRefs_.clear();
    sptr<IRemoteObject> stubObject = new IPCObjectStub(u"testObject");
    int pid = 1;
    std::string deviceId = "test";

    std::shared_ptr<StubRefCountObject> countObject =
        std::make_shared<StubRefCountObject>(stubObject.GetRefPtr(), pid, deviceId);
    skeleton->stubRecvRefs_.push_back(countObject);
    auto ret = skeleton->AttachStubRecvRefInfo(stubObject.GetRefPtr(), pid, deviceId);
    EXPECT_EQ(ret, false);
    skeleton->stubRecvRefs_.clear();
    countObject->stub_ = nullptr;
}

/**
 * @tc.name: AttachStubRecvRefInfoTest002
 * @tc.desc: Verify the AttachStubRecvRefInfo function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, AttachStubRecvRefInfoTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    skeleton->stubRecvRefs_.clear();
    sptr<IRemoteObject> stubObject = new IPCObjectStub(u"testObject");
    int pid = 1;
    std::string deviceId = "test";

    auto ret = skeleton->AttachStubRecvRefInfo(stubObject.GetRefPtr(), pid, deviceId);
    EXPECT_EQ(ret, true);
    skeleton->stubRecvRefs_.clear();
}

/**
 * @tc.name: DetachStubRecvRefInfoTest001
 * @tc.desc: Verify the DetachStubRecvRefInfo function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, DetachStubRecvRefInfoTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    skeleton->stubRecvRefs_.clear();
    sptr<IRemoteObject> stubObject = new IPCObjectStub(u"testObject");
    int pid = 1;
    std::string deviceId = "test";

    std::shared_ptr<StubRefCountObject> countObject =
        std::make_shared<StubRefCountObject>(stubObject.GetRefPtr(), pid, deviceId);
    skeleton->stubRecvRefs_.push_back(countObject);
    auto ret = skeleton->DetachStubRecvRefInfo(stubObject.GetRefPtr(), pid, deviceId);
    EXPECT_EQ(ret, true);
    skeleton->stubRecvRefs_.clear();
    countObject->stub_ = nullptr;
}

/**
 * @tc.name: DetachStubRecvRefInfoTest002
 * @tc.desc: Verify the DetachStubRecvRefInfo function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, DetachStubRecvRefInfoTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    skeleton->stubRecvRefs_.clear();
    sptr<IRemoteObject> stubObject = new IPCObjectStub(u"testObject");
    int pid = 1;
    std::string deviceId = "test";

    auto ret = skeleton->DetachStubRecvRefInfo(stubObject.GetRefPtr(), pid, deviceId);
    EXPECT_EQ(ret, false);
    skeleton->stubRecvRefs_.clear();
}

/**
 * @tc.name: QueryStubRecvRefInfoTest001
 * @tc.desc: Verify the QueryStubRecvRefInfo function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, QueryStubRecvRefInfoTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    sptr<IRemoteObject> stubObject = new IPCObjectStub(u"testObject");
    int pid = 1;
    std::string deviceId = "test";

    std::shared_ptr<StubRefCountObject> countObject =
        std::make_shared<StubRefCountObject>(stubObject.GetRefPtr(), pid, deviceId);
    skeleton->stubRecvRefs_.push_back(countObject);
    auto ret = skeleton->QueryStubRecvRefInfo(pid, deviceId);

    ASSERT_TRUE(ret.size() != 0);
    skeleton->stubRecvRefs_.clear();
    countObject->stub_ = nullptr;
}

/**
 * @tc.name: QueryStubRecvRefInfoTest002
 * @tc.desc: Verify the QueryStubRecvRefInfo function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, QueryStubRecvRefInfoTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    sptr<IRemoteObject> stubObject = new IPCObjectStub(u"testObject");
    int pid = 1;
    std::string deviceId = "test";

    std::shared_ptr<StubRefCountObject> countObject =
        std::make_shared<StubRefCountObject>(stubObject.GetRefPtr(), pid, deviceId);
    skeleton->stubRecvRefs_.push_back(countObject);

    skeleton->QueryStubRecvRefInfo(pid, deviceId);
    ASSERT_TRUE(skeleton->stubRecvRefs_.size() != 0);
    skeleton->stubRecvRefs_.clear();
    countObject->stub_ = nullptr;
}

/**
 * @tc.name: DetachStubRefInfoTest001
 * @tc.desc: Verify the DetachStubRefInfo function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, DetachStubRefInfoTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    IPCObjectStub stubObject(u"testObject");
    int pid = 1;
    std::string deviceId = "test";

    std::shared_ptr<StubRefCountObject> countObject =
        std::make_shared<StubRefCountObject>(&stubObject, pid, deviceId);
    skeleton->stubRecvRefs_.push_back(countObject);
    skeleton->stubSendRefs_.push_back(countObject);

    skeleton->DetachStubRefInfo(pid, deviceId);
    ASSERT_TRUE(skeleton->stubRecvRefs_.size() == 0);
    skeleton->stubSendRefs_.clear();
    skeleton->stubRecvRefs_.clear();
    countObject->stub_ = nullptr;
}

/**
 * @tc.name: DetachStubRefInfoTest002
 * @tc.desc: Verify the DetachStubRefInfo function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, DetachStubRefInfoTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    IPCObjectStub stubObject(u"testObject");
    int pid = 1;
    std::string deviceId = "test";

    std::shared_ptr<StubRefCountObject> countObject =
        std::make_shared<StubRefCountObject>(&stubObject, pid, deviceId);
    skeleton->stubRecvRefs_.push_back(countObject);
    skeleton->stubSendRefs_.push_back(countObject);

    skeleton->DetachStubRefInfo(&stubObject, pid, deviceId);
    ASSERT_TRUE(skeleton->stubRecvRefs_.size() == 0);
    skeleton->stubSendRefs_.clear();
    skeleton->stubRecvRefs_.clear();
    countObject->stub_ = nullptr;
}

/**
 * @tc.name: IncStubRefTimesTest001
 * @tc.desc: Verify the IncStubRefTimes function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, IncStubRefTimesTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);
    skeleton->transTimes_.clear();

    sptr<IRemoteObject> object = new IPCObjectStub(u"testObject");
    skeleton->transTimes_[object.GetRefPtr()] = 1;
    bool ret = skeleton->IncStubRefTimes(object.GetRefPtr());
    EXPECT_EQ(ret, true);
    skeleton->transTimes_.clear();
}

/**
 * @tc.name: IncStubRefTimesTest002
 * @tc.desc: Verify the IncStubRefTimes function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, IncStubRefTimesTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);
    skeleton->transTimes_.clear();

    sptr<IRemoteObject> object = new IPCObjectStub(u"testObject");
    bool ret = skeleton->IncStubRefTimes(object.GetRefPtr());
    EXPECT_EQ(ret, true);
    skeleton->transTimes_.clear();
}

/**
 * @tc.name: DecStubRefTimesTest001
 * @tc.desc: Verify the DecStubRefTimes function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, DecStubRefTimesTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);
    skeleton->transTimes_.clear();

    sptr<IRemoteObject> object = new IPCObjectStub(u"testObject");
    skeleton->transTimes_[object.GetRefPtr()] = 1;
    bool ret = skeleton->DecStubRefTimes(object.GetRefPtr());
    EXPECT_EQ(ret, true);
    skeleton->transTimes_.clear();
}

/**
 * @tc.name: DecStubRefTimesTest002
 * @tc.desc: Verify the DecStubRefTimes function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, DecStubRefTimesTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);
    skeleton->transTimes_.clear();

    sptr<IRemoteObject> object = new IPCObjectStub(u"testObject");
    skeleton->transTimes_[object.GetRefPtr()] = 0;
    bool ret = skeleton->DecStubRefTimes(object.GetRefPtr());
    EXPECT_EQ(ret, false);
    skeleton->transTimes_.clear();
}

/**
 * @tc.name: DecStubRefTimesTest003
 * @tc.desc: Verify the DecStubRefTimes function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, DecStubRefTimesTest003, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);
    skeleton->transTimes_.clear();

    sptr<IRemoteObject> object = new IPCObjectStub(u"testObject");
    bool ret = skeleton->DecStubRefTimes(object.GetRefPtr());
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: AttachStubSendRefInfoTest001
 * @tc.desc: Verify the AttachStubSendRefInfo function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, AttachStubSendRefInfoTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    skeleton->stubSendRefs_.clear();
    sptr<IRemoteObject> stubObject = new IPCObjectStub(u"testObject");
    int pid = 1;
    std::string deviceId = "test";

    std::shared_ptr<StubRefCountObject> countObject =
        std::make_shared<StubRefCountObject>(stubObject.GetRefPtr(), pid, deviceId);
    skeleton->stubSendRefs_.push_back(countObject);
    auto ret = skeleton->AttachStubSendRefInfo(stubObject.GetRefPtr(), pid, deviceId);
    EXPECT_EQ(ret, false);
    skeleton->stubRecvRefs_.clear();
    countObject->stub_ = nullptr;
}

/**
 * @tc.name: AttachStubSendRefInfoTest002
 * @tc.desc: Verify the AttachStubSendRefInfo function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, AttachStubSendRefInfoTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    skeleton->stubSendRefs_.clear();
    sptr<IRemoteObject> stubObject = new IPCObjectStub(u"testObject");
    int pid = 1;
    std::string deviceId = "test";

    std::shared_ptr<StubRefCountObject> countObject =
        std::make_shared<StubRefCountObject>(stubObject.GetRefPtr(), pid, deviceId);
    auto ret = skeleton->AttachStubSendRefInfo(stubObject.GetRefPtr(), pid, deviceId);
    EXPECT_EQ(ret, true);
    skeleton->stubRecvRefs_.clear();
    countObject->stub_ = nullptr;
}

/**
 * @tc.name: IsSameRemoteObjectTest001
 * @tc.desc: Verify the IsSameRemoteObject function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, IsSameRemoteObjectTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    std::shared_ptr<FeatureSetData> featureSet = std::make_shared<FeatureSetData>();
    sptr<IRemoteObject> stubObject = new IPCObjectStub(u"testObject");
    int pid = 1;
    int uid = 1;
    std::string deviceId = "test";
    std::shared_ptr<CommAuthInfo> auth =
        std::make_shared<CommAuthInfo>(stubObject.GetRefPtr(), pid, uid, deviceId, featureSet);

    bool ret = skeleton->IsSameRemoteObject(pid, uid, deviceId, auth);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: IsSameRemoteObjectTest002
 * @tc.desc: Verify the IsSameRemoteObject function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, IsSameRemoteObjectTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    std::shared_ptr<FeatureSetData> featureSet = std::make_shared<FeatureSetData>();
    sptr<IRemoteObject> stubObject = new IPCObjectStub(u"testObject");
    int pid = 1;
    int uid = 1;
    std::string deviceId = "test";
    std::shared_ptr<CommAuthInfo> auth =
        std::make_shared<CommAuthInfo>(stubObject.GetRefPtr(), pid, uid, deviceId, featureSet);

    bool ret = skeleton->IsSameRemoteObject(0, uid, deviceId, auth);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: IsSameRemoteObjectTest003
 * @tc.desc: Verify the IsSameRemoteObject function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, IsSameRemoteObjectTest003, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    std::shared_ptr<FeatureSetData> featureSet = std::make_shared<FeatureSetData>();
    sptr<IRemoteObject> stubObject = new IPCObjectStub(u"testObject");
    int pid = 1;
    int uid = 1;
    std::string deviceId = "test";
    std::shared_ptr<CommAuthInfo> auth =
        std::make_shared<CommAuthInfo>(stubObject.GetRefPtr(), pid, uid, deviceId, featureSet);

    bool ret = skeleton->IsSameRemoteObject(pid, 0, deviceId, auth);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: IsSameRemoteObjectTest004
 * @tc.desc: Verify the IsSameRemoteObject function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, IsSameRemoteObjectTest004, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    std::shared_ptr<FeatureSetData> featureSet = std::make_shared<FeatureSetData>();
    sptr<IRemoteObject> stubObject = new IPCObjectStub(u"testObject");
    int pid = 1;
    int uid = 1;
    std::string deviceId = "test";
    std::shared_ptr<CommAuthInfo> auth =
        std::make_shared<CommAuthInfo>(stubObject.GetRefPtr(), pid, uid, deviceId, featureSet);

    bool ret = skeleton->IsSameRemoteObject(stubObject.GetRefPtr(), pid, uid, deviceId, auth);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: IsSameRemoteObjectTest005
 * @tc.desc: Verify the IsSameRemoteObject function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, IsSameRemoteObjectTest005, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    std::shared_ptr<FeatureSetData> featureSet = std::make_shared<FeatureSetData>();
    sptr<IRemoteObject> stubObject = new IPCObjectStub(u"testObject");
    int pid = 1;
    int uid = 1;
    std::string deviceId = "test";
    std::shared_ptr<CommAuthInfo> auth =
        std::make_shared<CommAuthInfo>(stubObject.GetRefPtr(), pid, uid, deviceId, featureSet);
    IRemoteObject *stub = nullptr;

    bool ret = skeleton->IsSameRemoteObject(stub, pid, uid, deviceId, auth);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: IsSameRemoteObjectTest006
 * @tc.desc: Verify the IsSameRemoteObject function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, IsSameRemoteObjectTest006, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    std::shared_ptr<FeatureSetData> featureSet = std::make_shared<FeatureSetData>();
    sptr<IRemoteObject> stubObject = new IPCObjectStub(u"testObject");
    int pid = 1;
    int uid = 1;
    std::string deviceId = "test";
    std::shared_ptr<CommAuthInfo> auth =
        std::make_shared<CommAuthInfo>(stubObject.GetRefPtr(), pid, uid, deviceId, featureSet);

    bool ret = skeleton->IsSameRemoteObject(stubObject.GetRefPtr(), 0, uid, deviceId, auth);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: IsSameRemoteObjectTest007
 * @tc.desc: Verify the IsSameRemoteObject function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, IsSameRemoteObjectTest007, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    std::shared_ptr<FeatureSetData> featureSet = std::make_shared<FeatureSetData>();
    sptr<IRemoteObject> stubObject = new IPCObjectStub(u"testObject");
    int pid = 1;
    int uid = 1;
    std::string deviceId = "test";
    std::shared_ptr<CommAuthInfo> auth =
        std::make_shared<CommAuthInfo>(stubObject.GetRefPtr(), pid, uid, deviceId, featureSet);

    bool ret = skeleton->IsSameRemoteObject(stubObject.GetRefPtr(), pid, 0, deviceId, auth);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: IsSameRemoteObjectTest008
 * @tc.desc: Verify the IsSameRemoteObject function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, IsSameRemoteObjectTest008, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    std::shared_ptr<FeatureSetData> featureSet = std::make_shared<FeatureSetData>();
    sptr<IRemoteObject> stubObject = new IPCObjectStub(u"testObject");
    int pid = 1;
    int uid = 1;
    std::string deviceId = "test";
    std::shared_ptr<CommAuthInfo> auth =
        std::make_shared<CommAuthInfo>(stubObject.GetRefPtr(), pid, uid, deviceId, featureSet);

    bool ret = skeleton->IsSameRemoteObject(stubObject.GetRefPtr(), pid, uid, "deviceId", auth);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: IsSameRemoteObjectTest009
 * @tc.desc: Verify the IsSameRemoteObject function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, IsSameRemoteObjectTest009, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    std::shared_ptr<FeatureSetData> featureSet = std::make_shared<FeatureSetData>();
    sptr<IRemoteObject> stubObject = new IPCObjectStub(u"testObject");
    int pid = 1;
    int uid = 1;
    std::string deviceId = "test";
    std::shared_ptr<CommAuthInfo> auth =
        std::make_shared<CommAuthInfo>(stubObject.GetRefPtr(), pid, uid, deviceId, featureSet);

    bool ret = skeleton->IsSameRemoteObject(pid, uid, "deviceId", auth);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: AttachCommAuthInfoTest001
 * @tc.desc: Verify the AttachCommAuthInfo function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, AttachCommAuthInfoTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    skeleton->commAuth_.clear();
    std::shared_ptr<FeatureSetData> featureSet = std::make_shared<FeatureSetData>();
    sptr<IRemoteObject> stubObject = new IPCObjectStub(u"testObject");
    int pid = 1;
    int uid = 1;
    std::string deviceId = "test";
    std::shared_ptr<CommAuthInfo> auth =
        std::make_shared<CommAuthInfo>(stubObject.GetRefPtr(), pid, uid, deviceId, featureSet);

    skeleton->commAuth_.push_back(auth);
    bool ret = skeleton->AttachCommAuthInfo(stubObject.GetRefPtr(), pid, uid, deviceId, featureSet);
    EXPECT_EQ(ret, true);
    skeleton->commAuth_.clear();
}

/**
 * @tc.name: AttachCommAuthInfoTest002
 * @tc.desc: Verify the AttachCommAuthInfo function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, AttachCommAuthInfoTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    skeleton->commAuth_.clear();
    std::shared_ptr<FeatureSetData> featureSet = std::make_shared<FeatureSetData>();
    sptr<IRemoteObject> stubObject = new IPCObjectStub(u"testObject");
    int pid = 1;
    int uid = 1;
    std::string deviceId = "test";

    bool ret = skeleton->AttachCommAuthInfo(stubObject.GetRefPtr(), pid, uid, deviceId, featureSet);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: QueryIsAuthTest001
 * @tc.desc: Verify the QueryIsAuth function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, QueryIsAuthTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    skeleton->commAuth_.clear();
    std::shared_ptr<FeatureSetData> featureSet = std::make_shared<FeatureSetData>();
    sptr<IRemoteObject> stubObject = new IPCObjectStub(u"testObject");
    int pid = 1;
    int uid = 1;
    std::string deviceId = "test";
    std::shared_ptr<CommAuthInfo> auth =
        std::make_shared<CommAuthInfo>(stubObject.GetRefPtr(), pid, uid, deviceId, featureSet);

    skeleton->commAuth_.push_back(auth);
    auto ret = skeleton->QueryIsAuth(pid, uid, deviceId);
    EXPECT_NE(ret, nullptr);
    skeleton->commAuth_.clear();
    auth->stub_ = nullptr;
}

/**
 * @tc.name: QueryIsAuthTest002
 * @tc.desc: Verify the QueryIsAuth function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, QueryIsAuthTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    skeleton->commAuth_.clear();
    std::shared_ptr<FeatureSetData> featureSet = std::make_shared<FeatureSetData>();
    sptr<IRemoteObject> stubObject = new IPCObjectStub(u"testObject");
    int pid = 1;
    int uid = 1;
    std::string deviceId = "test";
    std::shared_ptr<CommAuthInfo> auth =
        std::make_shared<CommAuthInfo>(stubObject.GetRefPtr(), pid, uid, deviceId, featureSet);

    skeleton->commAuth_.push_back(auth);
    auto ret = skeleton->QueryIsAuth(0, uid, deviceId);
    EXPECT_EQ(ret, nullptr);
    skeleton->commAuth_.clear();
    auth->stub_ = nullptr;
}

/**
 * @tc.name: QueryDBinderCallbackStubTest001
 * @tc.desc: Verify the QueryDBinderCallbackStub function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, QueryDBinderCallbackStubTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);
    skeleton->dbinderSentCallback.clear();

    std::shared_ptr<FeatureSetData> feature = std::make_shared<FeatureSetData>();
    sptr<IPCObjectProxy> objectProxy = new IPCObjectProxy(1);
    sptr<DBinderCallbackStub> stub = new DBinderCallbackStub(
        "serviceName", "peerDeviceID", "localDeviceID", 1, 1, feature);
    skeleton->dbinderSentCallback[objectProxy] = stub;

    auto ret = skeleton->QueryDBinderCallbackStub(objectProxy);
    EXPECT_NE(ret, nullptr);
    skeleton->dbinderSentCallback.clear();
}

/**
 * @tc.name: QueryDBinderCallbackStubTest002
 * @tc.desc: Verify the QueryDBinderCallbackStub function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, QueryDBinderCallbackStubTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);
    skeleton->dbinderSentCallback.clear();

    sptr<IPCObjectProxy> objectProxy = new IPCObjectProxy(1);

    auto ret = skeleton->QueryDBinderCallbackStub(objectProxy);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: QueryDBinderCallbackProxyTest001
 * @tc.desc: Verify the QueryDBinderCallbackProxy function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, QueryDBinderCallbackProxyTest001, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);
    skeleton->dbinderSentCallback.clear();

    std::shared_ptr<FeatureSetData> feature = std::make_shared<FeatureSetData>();
    sptr<IRemoteObject> object = new IPCObjectStub(u"testObject");
    sptr<DBinderCallbackStub> stub = new DBinderCallbackStub(
        "serviceName", "peerDeviceID", "localDeviceID", 1, 1, feature);
    skeleton->dbinderSentCallback[object] = stub;

    auto ret = skeleton->QueryDBinderCallbackStub(object);
    EXPECT_NE(ret, nullptr);
    skeleton->dbinderSentCallback.clear();
}


/**
 * @tc.name: QueryDBinderCallbackProxyTest002
 * @tc.desc: Verify the QueryDBinderCallbackProxy function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, QueryDBinderCallbackProxyTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);
    skeleton->dbinderSentCallback.clear();

    sptr<IRemoteObject> object = new IPCObjectStub(u"testObject");

    auto ret = skeleton->QueryDBinderCallbackProxy(object);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: IsContainsObjectTest002
 * @tc.desc: Verify the IsContainsObject function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, IsContainsObjectTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    sptr<IRemoteObject> object = new IPCObjectStub(u"testObject");
    skeleton->isContainStub_.clear();
    bool ret = skeleton->IsContainsObject(object.GetRefPtr());
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: QueryHandleToIndexTest004
 * @tc.desc: Verify the QueryHandleToIndex function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, QueryHandleToIndexTest004, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    std::list<uint32_t> handleList { 1 };
    uint32_t handler = 1;
    skeleton->handleToStubIndex_.clear();
    uint64_t ret = skeleton->QueryHandleToIndex(handleList, handler);
    uint64_t hd = 0;
    EXPECT_EQ(ret, hd);
}

/**
 * @tc.name: ProxyQueryDBinderSessionTest002
 * @tc.desc: Verify the ProxyQueryDBinderSession function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, ProxyQueryDBinderSessionTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    uint32_t handler = 1;
    skeleton->proxyToSession_.clear();
    auto ret = skeleton->ProxyQueryDBinderSession(handler);

    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: QueryHandleToIndexTest002
 * @tc.desc: Verify the QueryHandleToIndex function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, QueryHandleToIndexTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    uint32_t handler = 1;
    skeleton->handleToStubIndex_.clear();
    uint64_t ret = skeleton->QueryHandleToIndex(handler);

    ASSERT_TRUE(ret == 0);
}

/**
 * @tc.name: QueryProxyBySessionHandleTest002
 * @tc.desc: Verify the QueryProxyBySessionHandle function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, QueryProxyBySessionHandleTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    uint32_t handler = 1;
    std::vector<uint32_t> proxyHandle { 1 };
    skeleton->proxyToSession_.clear();
    auto ret = skeleton->QueryProxyBySessionHandle(handler, proxyHandle);

    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: QueryHandleByDatabusSessionTest002
 * @tc.desc: Verify the QueryHandleByDatabusSession function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, QueryHandleByDatabusSessionTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    skeleton->handleToStubIndex_.clear();
    skeleton->proxyToSession_.clear();
    const std::string name("nameTest");
    const std::string deviceId("deviceIdTest");
    uint64_t index = 1;
    skeleton->handleToStubIndex_.insert(std::pair<uint32_t, uint64_t>(1, 1));

    std::shared_ptr<MockSessionImpl> sessionMock = std::make_shared<MockSessionImpl>();
    auto object = std::make_shared<DBinderSessionObject>(sessionMock, name, deviceId);
    uint32_t handler = 1;
    skeleton->proxyToSession_.insert(
        std::pair<uint32_t, std::shared_ptr<DBinderSessionObject>>(handler, object));
    auto ret = skeleton->QueryHandleByDatabusSession(name, deviceId, index);

    EXPECT_EQ(ret, handler);
}

/**
 * @tc.name: QuerySessionByInfoTest002
 * @tc.desc: Verify the QuerySessionByInfo function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, QuerySessionByInfoTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    const std::string name("name");
    const std::string deviceId("deviceId");
    const std::string deviceIdTest("deviceIdTest");

    skeleton->proxyToSession_.clear();
    std::shared_ptr<MockSessionImpl> sessionMock = std::make_shared<MockSessionImpl>();
    auto object = std::make_shared<DBinderSessionObject>(sessionMock, name, deviceId);
    uint32_t handler = 1;
    skeleton->proxyToSession_.insert(
        std::pair<uint32_t, std::shared_ptr<DBinderSessionObject>>(handler, object));

    auto ret = skeleton->QuerySessionByInfo(name, deviceIdTest);

    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: QuerySessionByInfoTest003
 * @tc.desc: Verify the QuerySessionByInfo function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, QuerySessionByInfoTest003, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    const std::string name("nameTest");
    const std::string deviceId("deviceIdTest");
    skeleton->proxyToSession_.clear();
    auto ret = skeleton->QuerySessionByInfo(name, deviceId);

    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: QueryThreadLockInfoTest002
 * @tc.desc: Verify the QueryThreadLockInfo function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, QueryThreadLockInfoTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    std::thread::id threadId;
    skeleton->threadLockInfo_.clear();

    auto ret = skeleton->QueryThreadLockInfo(threadId);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: DeleteDataThreadFromIdleTest002
 * @tc.desc: Verify the DeleteDataThreadFromIdle function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, DeleteDataThreadFromIdleTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    std::thread::id threadId;
    skeleton->idleDataThreads_.clear();

    bool ret = skeleton->DeleteDataThreadFromIdle(threadId);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: GetIdleDataThreadTest002
 * @tc.desc: Verify the GetIdleDataThread function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, GetIdleDataThreadTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    std::thread::id threadId;
    skeleton->idleDataThreads_.clear();
    auto ret = skeleton->GetIdleDataThread();
    EXPECT_EQ(threadId, ret);
}

/**
 * @tc.name: GetSocketIdleThreadNumTest002
 * @tc.desc: Verify the GetSocketIdleThreadNum function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, GetSocketIdleThreadNumTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);
    delete skeleton->threadPool_;
    skeleton->threadPool_ = nullptr;

    auto ret = skeleton->GetSocketIdleThreadNum();
    EXPECT_EQ(0, ret);
}

/**
 * @tc.name: GetSocketTotalThreadNumTest002
 * @tc.desc: Verify the GetSocketTotalThreadNum function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, GetSocketTotalThreadNumTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);
    delete skeleton->threadPool_;
    skeleton->threadPool_ = nullptr;

    auto ret = skeleton->GetSocketTotalThreadNum();
    EXPECT_EQ(0, ret);
}

/**
 * @tc.name: PopDataInfoFromThreadTest002
 * @tc.desc: Verify the PopDataInfoFromThread function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, PopDataInfoFromThreadTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    std::thread::id threadId;
    std::shared_ptr<ThreadProcessInfo> processInfo =
        std::make_shared<ThreadProcessInfo>();
    skeleton->AddDataInfoToThread(threadId, processInfo);
    ASSERT_TRUE(!(skeleton->dataInfoQueue_[threadId]).empty());

    (skeleton->dataInfoQueue_[threadId]).clear();

    auto ret = skeleton->PopDataInfoFromThread(threadId);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: QueryThreadBySeqNumberTest002
 * @tc.desc: Verify the QueryThreadBySeqNumber function
 * @tc.type: FUNC
 */
HWTEST_F(IPCProcessSkeletonUnitTest, QueryThreadBySeqNumberTest002, TestSize.Level1)
{
    IPCProcessSkeleton *skeleton = IPCProcessSkeleton::GetCurrent();
    ASSERT_TRUE(skeleton != nullptr);

    uint64_t seqNumber = 1;
    skeleton->seqNumberToThread_.clear();
    auto ret = skeleton->QueryThreadBySeqNumber(seqNumber);
    ASSERT_TRUE(ret == nullptr);
}
#endif
