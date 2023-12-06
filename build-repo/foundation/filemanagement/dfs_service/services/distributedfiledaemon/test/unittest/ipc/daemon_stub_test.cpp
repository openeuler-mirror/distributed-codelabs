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
#include "daemon_stub_mock.h"
#include "ipc/daemon_stub.h"
#include "ipc/i_daemon.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Test {
namespace {
    const int ERROR_CODE = 99999;
}
using namespace testing::ext;

class DaemonStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DaemonStubTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void DaemonStubTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void DaemonStubTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void DaemonStubTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
 * @tc.name: DaemonStubTest_OnRemoteRequest_0100
 * @tc.desc: Verify the OnRemoteRequest function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DaemonStubTest, DaemonStubTest_OnRemoteRequest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonStubTest_OnRemoteRequest_0100 start";
    DaemonStubMock mock;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    bool bRet = data.WriteInterfaceToken(u"error descriptor");
    EXPECT_TRUE(bRet) << "write token error";

    int32_t ret = mock.OnRemoteRequest(IDaemon::DFS_DAEMON_CMD_ECHO, data, reply, option);
    EXPECT_TRUE(ret != ERR_NONE) << "descriptor error";
    GTEST_LOG_(INFO) << "DaemonStubTest_OnRemoteRequest_0100 end";
}

/**
 * @tc.name: DaemonStubTest_OnRemoteRequest_0200
 * @tc.desc: Verify the OnRemoteRequest function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DaemonStubTest, DaemonStubTest_OnRemoteRequest_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonStubTest_OnRemoteRequest_0200 start";
    DaemonStubMock mock;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    bool bRet = data.WriteInterfaceToken(IDaemon::GetDescriptor());
    EXPECT_TRUE(bRet) << "write token error";

    int32_t ret = mock.OnRemoteRequest(ERROR_CODE, data, reply, option);
    EXPECT_TRUE(ret != IDaemon::DFS_DAEMON_SUCCESS) << "request code error";
    GTEST_LOG_(INFO) << "DaemonStubTest_OnRemoteRequest_0200 end";
}

/**
 * @tc.name: DaemonStubTest_OnRemoteRequest_0300
 * @tc.desc: Verify the OnRemoteRequest function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(DaemonStubTest, DaemonStubTest_OnRemoteRequest_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonStubTest_OnRemoteRequest_0300 start";
    DaemonStubMock mock;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    bool bRet = data.WriteInterfaceToken(IDaemon::GetDescriptor());
    EXPECT_TRUE(bRet) << "write token error";

    int32_t ret = mock.OnRemoteRequest(IDaemon::DFS_DAEMON_CMD_ECHO, data, reply, option);
    EXPECT_TRUE(ret == IDaemon::DFS_DAEMON_SUCCESS);
    GTEST_LOG_(INFO) << "DaemonStubTest_OnRemoteRequest_0300 end";
}
} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS