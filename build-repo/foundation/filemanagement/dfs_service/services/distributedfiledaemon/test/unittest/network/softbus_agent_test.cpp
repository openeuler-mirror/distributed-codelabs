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
#include "network/softbus/softbus_session.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Test {
using namespace testing::ext;
using namespace std;

constexpr int E_OK = 0;
constexpr int USER_ID = 100;
static const string SAME_ACCOUNT = "account";

class SoftbusAgentTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: SoftbusAgentTest_OnSessionOpened_0100
 * @tc.desc: Verify the OnSessionOpened function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(SoftbusAgentTest, SoftbusAgentTest_OnSessionOpened_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusAgentTest_OnSessionOpened_0100 start";
    auto mp = make_unique<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, SAME_ACCOUNT));
    shared_ptr<MountPoint> smp = move(mp);
    weak_ptr<MountPoint> wmp(smp);
    std::shared_ptr<SoftbusAgent> agent = std::make_shared<SoftbusAgent>(wmp);

    auto execFun = [](std::shared_ptr<SoftbusAgent> ag) {
        const int sessionId = 1;
        const int result = E_OK;
        int ret = ag->OnSessionOpened(sessionId, result);
        EXPECT_TRUE(ret == E_OK);
    };

    std::thread execThread(execFun, agent);
    sleep(1);
    execThread.join();
    GTEST_LOG_(INFO) << "SoftbusAgentTest_OnSessionOpened_0100 end";
}

/**
 * @tc.name: SoftbusAgentTest_OnSessionOpened_0200
 * @tc.desc: Verify the OnSessionOpened function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(SoftbusAgentTest, SoftbusAgentTest_OnSessionOpened_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusAgentTest_OnSessionOpened_0200 start";
    auto mp = make_unique<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, SAME_ACCOUNT));
    shared_ptr<MountPoint> smp = move(mp);
    weak_ptr<MountPoint> wmp(smp);
    std::shared_ptr<SoftbusAgent> agent = std::make_shared<SoftbusAgent>(wmp);

    auto execFun = [](std::shared_ptr<SoftbusAgent> ag) {
        const int sessionId = 1;
        const int result = -1;
        int ret = ag->OnSessionOpened(sessionId, result);
        EXPECT_TRUE(ret == result);
    };

    std::thread execThread(execFun, agent);
    sleep(1);
    execThread.join();
    GTEST_LOG_(INFO) << "SoftbusAgentTest_OnSessionOpened_0200 end";
}

/**
 * @tc.name: SoftbusAgentTest_OnSessionClosed_0100
 * @tc.desc: Verify the OnSessionClosed function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(SoftbusAgentTest, SoftbusAgentTest_OnSessionClosed_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusAgentTest_OnSessionClosed_0100 start";
    auto mp = make_unique<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, SAME_ACCOUNT));
    shared_ptr<MountPoint> smp = move(mp);
    weak_ptr<MountPoint> wmp(smp);
    std::shared_ptr<SoftbusAgent> agent = std::make_shared<SoftbusAgent>(wmp);
    const int sessionId = 1;
    bool res = true;
    try {
        agent->OnSessionClosed(sessionId);
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "SoftbusAgentTest_OnSessionClosed_0100 end";
}

/**
 * @tc.name: SoftbusAgentTest_Start_0100
 * @tc.desc: Verify the Start function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(SoftbusAgentTest, SoftbusAgentTest_Start_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusAgentTest_Start_0100 start";
    auto mp = make_unique<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, SAME_ACCOUNT));
    shared_ptr<MountPoint> smp = move(mp);
    weak_ptr<MountPoint> wmp(smp);
    std::shared_ptr<SoftbusAgent> agent = std::make_shared<SoftbusAgent>(wmp);
    bool res = true;
    try {
        agent->Start();
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == false);
    GTEST_LOG_(INFO) << "SoftbusAgentTest_Start_0100 end";
}

/**
 * @tc.name: SoftbusAgentTest_Stop_0100
 * @tc.desc: Verify the Stop function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(SoftbusAgentTest, SoftbusAgentTest_Stop_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusAgentTest_Stop_0100 start";
    auto mp = make_unique<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, SAME_ACCOUNT));
    shared_ptr<MountPoint> smp = move(mp);
    weak_ptr<MountPoint> wmp(smp);
    std::shared_ptr<SoftbusAgent> agent = std::make_shared<SoftbusAgent>(wmp);
    bool res = true;
    try {
        agent->Stop();
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == false);
    GTEST_LOG_(INFO) << "SoftbusAgentTest_Stop_0100 end";
}

/**
 * @tc.name: SoftbusAgentTest_ConnectOnlineDevices_0100
 * @tc.desc: Verify the ConnectOnlineDevices function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(SoftbusAgentTest, SoftbusAgentTest_ConnectOnlineDevices_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusAgentTest_ConnectOnlineDevices_0100 start";
    auto mp = make_unique<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, SAME_ACCOUNT));
    shared_ptr<MountPoint> smp = move(mp);
    weak_ptr<MountPoint> wmp(smp);
    std::shared_ptr<SoftbusAgent> agent = std::make_shared<SoftbusAgent>(wmp);
    bool res = true;
    try {
        agent->ConnectOnlineDevices();
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == false);
    GTEST_LOG_(INFO) << "SoftbusAgentTest_ConnectOnlineDevices_0100 end";
}

/**
 * @tc.name: SoftbusAgentTest_DisconnectAllDevices_0100
 * @tc.desc: Verify the DisconnectAllDevices function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(SoftbusAgentTest, SoftbusAgentTest_DisconnectAllDevices_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusAgentTest_DisconnectAllDevices_0100 start";
    auto mp = make_unique<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, SAME_ACCOUNT));
    shared_ptr<MountPoint> smp = move(mp);
    weak_ptr<MountPoint> wmp(smp);
    std::shared_ptr<SoftbusAgent> agent = std::make_shared<SoftbusAgent>(wmp);
    bool res = true;
    try {
        agent->DisconnectAllDevices();
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "SoftbusAgentTest_DisconnectAllDevices_0100 end";
}

/**
 * @tc.name: SoftbusAgentTest_ConnectDeviceAsync_0100
 * @tc.desc: Verify the ConnectDeviceAsync function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(SoftbusAgentTest, SoftbusAgentTest_ConnectDeviceAsync_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusAgentTest_ConnectDeviceAsync_0100 start";
    auto mp = make_unique<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, SAME_ACCOUNT));
    shared_ptr<MountPoint> smp = move(mp);
    weak_ptr<MountPoint> wmp(smp);
    std::shared_ptr<SoftbusAgent> agent = std::make_shared<SoftbusAgent>(wmp);
    DistributedHardware::DmDeviceInfo info = {
        .deviceId = "testdevid",
        .deviceName = "testdevname",
        .deviceTypeId = 1,
    };
    DeviceInfo devInfo(info);

    bool res = true;
    try {
        agent->ConnectDeviceAsync(devInfo);
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == false);
    GTEST_LOG_(INFO) << "SoftbusAgentTest_ConnectDeviceAsync_0100 end";
}

/**
 * @tc.name: SoftbusAgentTest_DisconnectDevice_0100
 * @tc.desc: Verify the DisconnectDevice function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(SoftbusAgentTest, SoftbusAgentTest_DisconnectDevice_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusAgentTest_DisconnectDevice_0100 start";
    auto mp = make_unique<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, SAME_ACCOUNT));
    shared_ptr<MountPoint> smp = move(mp);
    weak_ptr<MountPoint> wmp(smp);
    std::shared_ptr<SoftbusAgent> agent = std::make_shared<SoftbusAgent>(wmp);
    DistributedHardware::DmDeviceInfo info = {
        .deviceId = "testdevid",
        .deviceName = "testdevname",
        .deviceTypeId = 1,
    };
    DeviceInfo devInfo(info);

    bool res = true;
    try {
        agent->DisconnectDevice(devInfo);
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "SoftbusAgentTest_DisconnectDevice_0100 end";
}

/**
 * @tc.name: SoftbusAgentTest_AcceptSession_0100
 * @tc.desc: Verify the AcceptSession function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(SoftbusAgentTest, SoftbusAgentTest_AcceptSession_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusAgentTest_AcceptSession_0100 start";
    auto mp = make_unique<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, SAME_ACCOUNT));
    shared_ptr<MountPoint> smp = move(mp);
    weak_ptr<MountPoint> wmp(smp);
    std::shared_ptr<SoftbusAgent> agent = std::make_shared<SoftbusAgent>(wmp);
    const int testSessionId = 99;
    auto session = make_shared<SoftbusSession>(testSessionId);
    bool res = true;
    try {
        agent->AcceptSession(session);
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "SoftbusAgentTest_AcceptSession_0100 end";
}

/**
 * @tc.name: SoftbusAgentTest_GetMountPoint_0100
 * @tc.desc: Verify the GetMountPoint function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(SoftbusAgentTest, SoftbusAgentTest_GetMountPoint_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusAgentTest_GetMountPoint_0100 start";
    auto mp = make_unique<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, SAME_ACCOUNT));
    shared_ptr<MountPoint> smp = move(mp);
    weak_ptr<MountPoint> wmp(smp);
    std::shared_ptr<SoftbusAgent> agent = std::make_shared<SoftbusAgent>(wmp);
    bool res = true;
    try {
        agent->GetMountPoint();
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "SoftbusAgentTest_GetMountPoint_0100 end";
}
} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
