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

#include <exception>
#include <memory>
#include <unistd.h>

#include "gtest/gtest.h"
#include "device/device_manager_agent.h"
#include "mountpoint/mount_point.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Test {
using namespace testing::ext;
using namespace std;

DistributedHardware::DmDeviceInfo deviceInfo = {
    .deviceId = "testdevid",
    .deviceName = "testdevname",
    .deviceTypeId = 1,
};

class DeviceManagerAgentTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DeviceManagerAgentTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void DeviceManagerAgentTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void DeviceManagerAgentTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void DeviceManagerAgentTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
 * @tc.name: DeviceManagerAgentTest_OnDeviceOnline_0100
 * @tc.desc: Verify the OnDeviceOnline function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_OnDeviceOnline_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceOnline_0100 start";
    bool res = true;

    try {
        DeviceManagerAgent::GetInstance()->OnDeviceOnline(deviceInfo);
    } catch (const exception &e) {
        LOGE("Error:%{public}s", e.what());
        res = false;
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceOnline_0100 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_OnDeviceOffline_0100
 * @tc.desc: Verify the OnDeviceOffline function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_OnDeviceOffline_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceOffline_0100 start";
    bool res = true;

    try {
        DeviceManagerAgent::GetInstance()->OnDeviceOffline(deviceInfo);
    } catch (const exception &e) {
        LOGE("Error:%{public}s", e.what());
        res = false;
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceOffline_0100 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_OnDeviceOffline_0200
 * @tc.desc: Verify the OnDeviceOffline function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_OnDeviceOffline_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceOffline_0200 start";
    bool res = true;

    try {
        DeviceManagerAgent::GetInstance()->OnDeviceOnline(deviceInfo);
        DeviceManagerAgent::GetInstance()->OnDeviceOffline(deviceInfo);
    } catch (const exception &e) {
        LOGE("Error:%{public}s", e.what());
        res = false;
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceOffline_0200 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_OnDeviceChanged_0100
 * @tc.desc: Verify the OnDeviceChanged function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_OnDeviceChanged_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceChanged_0100 start";
    bool res = true;

    try {
        DeviceManagerAgent::GetInstance()->OnDeviceChanged(deviceInfo);
    } catch (const exception &e) {
        LOGE("Error:%{public}s", e.what());
        res = false;
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceChanged_0100 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_OnDeviceReady_0100
 * @tc.desc: Verify the OnDeviceReady function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_OnDeviceReady_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceReady_0100 start";
    bool res = true;

    try {
        DeviceManagerAgent::GetInstance()->OnDeviceReady(deviceInfo);
    } catch (const exception &e) {
        LOGE("Error:%{public}s", e.what());
        res = false;
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceReady_0100 end";
}
} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
