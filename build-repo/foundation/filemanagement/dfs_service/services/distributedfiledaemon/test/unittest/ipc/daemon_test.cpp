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

#include <memory>

#include "gtest/gtest.h"
#include "ipc/daemon.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Test {
using namespace testing::ext;

std::shared_ptr<Daemon> g_daemon = nullptr;

class DaemonTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DaemonTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
    if (g_daemon == nullptr) {
        int32_t saId = 10;
        g_daemon = std::make_shared<Daemon>(saId);
        ASSERT_TRUE(g_daemon != nullptr) << "SystemAbility failed";
    }
}

void DaemonTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
    g_daemon = nullptr;
}

void DaemonTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void DaemonTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
 * @tc.name: DaemonTest_OnStart_0100
 * @tc.desc: Verify the OnStart function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DaemonTest, DaemonTest_OnStart_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_OnStart_0100 start";
    bool res = true;
    try {
        g_daemon->OnStart();
    }
    catch(const std::exception& e) {
        res = false;
        LOGE("DaemonTest_OnStart_0100 : %{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DaemonTest_OnStart_0100 end";
}

/**
 * @tc.name: DaemonTest_OnAddSystemAbility_0200
 * @tc.desc: Verify the OnAddSystemAbility function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DaemonTest, DaemonTest_OnAddSystemAbility_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_OnAddSystemAbility_0200 start";
    bool res = true;
    try {
        int32_t saId = 10;
        g_daemon->OnAddSystemAbility(saId, "");
    }
    catch(const std::exception& e) {
        res = false;
        LOGE("DaemonTest_OnAddSystemAbility_0200 : %{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DaemonTest_OnAddSystemAbility_0200 end";
}

/**
 * @tc.name: DaemonTest_OnRemoveSystemAbility_0300
 * @tc.desc: Verify the OnRemoveSystemAbility function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DaemonTest, DaemonTest_OnRemoveSystemAbility_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_OnRemoveSystemAbility_0300 start";
    bool res = true;
    try {
        int32_t saId = 10;
        g_daemon->OnRemoveSystemAbility(saId, "");
    }
    catch(const std::exception& e) {
        res = false;
        LOGE("DaemonTest_OnRemoveSystemAbility_0300 : %{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DaemonTest_OnRemoveSystemAbility_0300 end";
}

/**
 * @tc.name: DaemonTest_QueryServiceState_0400
 * @tc.desc: Verify the QueryServiceState function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DaemonTest, DaemonTest_QueryServiceState_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_QueryServiceState_0400 start";
    ServiceRunningState state { ServiceRunningState::STATE_NOT_START };
    state = g_daemon->QueryServiceState();
    EXPECT_EQ(state, ServiceRunningState::STATE_RUNNING);
    GTEST_LOG_(INFO) << "DaemonTest_QueryServiceState_0400 end";
}

/**
 * @tc.name: DaemonTest_EchoServerDemo_0500
 * @tc.desc: Verify the EchoServerDemo function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DaemonTest, DaemonTest_EchoServerDemo_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_EchoServerDemo_0500 start";
    int32_t res = 1;
    try {
        res = g_daemon->EchoServerDemo("");
    }
    catch(const std::exception& e) {
        LOGE("DaemonTest_EchoServerDemo_0500 : %{public}s", e.what());
    }
    EXPECT_EQ(res, 0);
    GTEST_LOG_(INFO) << "DaemonTest_EchoServerDemo_0500 end";
}

/**
 * @tc.name: DaemonTest_OnStop_0600
 * @tc.desc: Verify the OnStop function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DaemonTest, DaemonTest_OnStop_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_OnStop_0600 start";
    bool res = true;
    try {
        g_daemon->OnStop();
    }
    catch(const std::exception& e) {
        res = false;
        LOGE("DaemonTest_OnStop_0600 : %{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DaemonTest_OnStop_0600 end";
}
} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS