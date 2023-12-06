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
#include <unistd.h>

#include "gtest/gtest.h"
#include "network/kernel_talker.h"
#include "network/softbus/softbus_session.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Test {
using namespace testing::ext;
using namespace std;

constexpr int TEST_SESSION_ID = 10;
constexpr int USER_ID = 100;
static const string SESSION_CID = "testSession";
static const string SAME_ACCOUNT = "account";
shared_ptr<MountPoint> g_smp;
weak_ptr<MountPoint> g_wmp;
shared_ptr<KernelTalker> g_talker;

class KernelTalkerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() {};
    void TearDown() {};
};

void KernelTalkerTest::SetUpTestCase(void)
{
    g_smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, SAME_ACCOUNT));
    g_wmp = g_smp;
    g_talker = std::make_shared<KernelTalker>(g_wmp, [](NotifyParam &param) {}, [](const std::string &cid) {});
};

void KernelTalkerTest::TearDownTestCase(void)
{
    g_talker = nullptr;
    g_smp = nullptr;
};

/**
 * @tc.name: KernelTalkerTest_SinkSessionTokernel_0100
 * @tc.desc: Verify the SinkSessionTokernel function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(KernelTalkerTest, KernelTalkerTest_SinkSessionTokernel_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KernelTalkerTest_SinkSessionTokernel_0100 start";
    bool res = true;
    std::shared_ptr<SoftbusSession> session = make_shared<SoftbusSession>(TEST_SESSION_ID);
    try {
        g_talker->SinkSessionTokernel(session);
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "KernelTalkerTest_SinkSessionTokernel_0100 end";
}

/**
 * @tc.name: KernelTalkerTest_SinkDevslTokernel_0100
 * @tc.desc: Verify the SinkDevslTokernel function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(KernelTalkerTest, KernelTalkerTest_SinkDevslTokernel_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KernelTalkerTest_SinkDevslTokernel_0100 start";
    bool res = true;
    try {
        g_talker->SinkDevslTokernel(SESSION_CID, 1);
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "KernelTalkerTest_SinkDevslTokernel_0100 end";
}

/**
 * @tc.name: KernelTalkerTest_SinkOfflineCmdToKernel_0100
 * @tc.desc: Verify the SinkOfflineCmdToKernel function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(KernelTalkerTest, KernelTalkerTest_SinkOfflineCmdToKernel_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KernelTalkerTest_SinkOfflineCmdToKernel_0100 start";
    bool res = true;
    try {
        g_talker->SinkOfflineCmdToKernel("testSession");
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "KernelTalkerTest_SinkOfflineCmdToKernel_0100 end";
}

/**
 * @tc.name: KernelTalkerTest_CreatePollThread_0100
 * @tc.desc: Verify the CreatePollThread function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(KernelTalkerTest, KernelTalkerTest_CreatePollThread_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KernelTalkerTest_CreatePollThread_0100 start";
    bool res = true;
    try {
        g_talker->CreatePollThread();
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "KernelTalkerTest_CreatePollThread_0100 end";
}

/**
 * @tc.name: KernelTalkerTest_WaitForPollThreadExited_0100
 * @tc.desc: Verify the WaitForPollThreadExited function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(KernelTalkerTest, KernelTalkerTest_WaitForPollThreadExited_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KernelTalkerTest_WaitForPollThreadExited_0100 start";
    int res = true;
    try {
        g_talker->WaitForPollThreadExited();
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "KernelTalkerTest_WaitForPollThreadExited_0100 end";
}
} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
