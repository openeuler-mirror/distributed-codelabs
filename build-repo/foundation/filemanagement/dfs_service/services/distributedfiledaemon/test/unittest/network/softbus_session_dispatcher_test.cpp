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
#include "network/softbus/softbus_agent.h"
#include "network/softbus/softbus_session_dispatcher.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Test {
using namespace testing::ext;
using namespace std;

constexpr int TEST_SESSION_ID = 10;
constexpr int E_OK = 0;
constexpr int E_UNKNOWN = -1;
static const string SAME_ACCOUNT = "account";

class SoftbusSessionDispatcherTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: SoftbusSessionDispatcherTest_RegisterSessionListener_0100
 * @tc.desc: Verify the RegisterSessionListener/UnregisterSessionListener function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(SoftbusSessionDispatcherTest, SoftbusSessionDispatcherTest_RegisterSessionListener_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusSessionDispatcherTest_RegisterSessionListener_0100 start";
    constexpr int userId = 100;
    auto mp = make_unique<MountPoint>(
            OHOS::Storage::DistributedFile::Utils::DfsuMountArgumentDescriptors::Alpha(userId, SAME_ACCOUNT));
    shared_ptr<MountPoint> smp = move(mp);
    weak_ptr<MountPoint> wmp(smp);
    std::shared_ptr<SoftbusAgent> agent = std::make_shared<SoftbusAgent>(wmp);
    weak_ptr<SoftbusAgent> wsba(agent);
    const string busName = "testBus";
    bool res = true;

    try {
        SoftbusSessionDispatcher::RegisterSessionListener(busName, wsba);
        SoftbusSessionDispatcher::UnregisterSessionListener(busName);
    } catch (const exception &e) {
        LOGE("%{public}s", e.what());
        res = false;
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "SoftbusSessionDispatcherTest_RegisterSessionListener_0100 end";
}

/**
 * @tc.name: SoftbusSessionDispatcherTest_GetAgent_0100
 * @tc.desc: Verify the GetAgent function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(SoftbusSessionDispatcherTest, SoftbusSessionDispatcherTest_GetAgent_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusSessionDispatcherTest_GetAgent_0100 start";
    try {
        weak_ptr<SoftbusAgent> wp = SoftbusSessionDispatcher::GetAgent(TEST_SESSION_ID);
        EXPECT_TRUE(wp.expired() == true);
    } catch (const exception &e) {
        LOGE("%{public}s", e.what());
    }
    GTEST_LOG_(INFO) << "SoftbusSessionDispatcherTest_GetAgent_0100 end";
}

/**
 * @tc.name: SoftbusSessionDispatcherTest_OnSessionOpened_0100
 * @tc.desc: Verify the OnSessionOpened function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(SoftbusSessionDispatcherTest, SoftbusSessionDispatcherTest_OnSessionOpened_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusSessionDispatcherTest_OnSessionOpened_0100 start";
    bool res = true;

    try {
        SoftbusSessionDispatcher::OnSessionOpened(TEST_SESSION_ID, E_OK);
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "SoftbusSessionDispatcherTest_OnSessionOpened_0100 end";
}

/**
 * @tc.name: SoftbusSessionDispatcherTest_OnSessionOpened_0200
 * @tc.desc: Verify the OnSessionOpened function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(SoftbusSessionDispatcherTest, SoftbusSessionDispatcherTest_OnSessionOpened_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusSessionDispatcherTest_OnSessionOpened_0200 start";
    bool res = true;

    try {
        SoftbusSessionDispatcher::OnSessionOpened(TEST_SESSION_ID, E_UNKNOWN);
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "SoftbusSessionDispatcherTest_OnSessionOpened_0200 end";
}

/**
 * @tc.name: SoftbusSessionDispatcherTest_OnSessionClosed_0100
 * @tc.desc: Verify the OnSessionClosed function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(SoftbusSessionDispatcherTest, SoftbusSessionDispatcherTest_OnSessionClosed_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusSessionDispatcherTest_OnSessionClosed_0100 start";
    bool res = true;

    try {
        SoftbusSessionDispatcher::OnSessionClosed(TEST_SESSION_ID);
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "SoftbusSessionDispatcherTest_OnSessionClosed_0100 end";
}
} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
