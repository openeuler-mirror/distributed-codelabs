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

#include "process_communicator_impl.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <string>

#include "app_types.h"
#include "auto_launch_export.h"
#include "ipc_skeleton.h"
#include "objectstore_errors.h"

namespace {
using namespace testing::ext;
using namespace OHOS::ObjectStore;

static constexpr uint32_t MTU_SIZE = 4096 * 1024;

class NativeProcessCommunicatorImplTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void NativeProcessCommunicatorImplTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void NativeProcessCommunicatorImplTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void NativeProcessCommunicatorImplTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void NativeProcessCommunicatorImplTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
 * @tc.name: ProcessCommunicatorImpl_Start_Stop_001
 * @tc.desc: test ProcessCommunicatorImpl Start and Stop.
 * @tc.type: FUNC
 */
HWTEST_F(NativeProcessCommunicatorImplTest, ProcessCommunicatorImpl_Start_Stop_001, TestSize.Level1)
{
    std::string processLabel = "INVALID_SESSION_NAME";
    ProcessCommunicatorImpl *processCommunicator = new ProcessCommunicatorImpl();
    auto ret = processCommunicator->Start(processLabel);
    EXPECT_EQ(DistributedDB::DBStatus::DB_ERROR, ret);
    ret = processCommunicator->Stop();
    EXPECT_EQ(DistributedDB::DBStatus::DB_ERROR, ret);
    delete processCommunicator;
}

/**
 * @tc.name: ProcessCommunicatorImpl_RegOnDeviceChange_001
 * @tc.desc: test ProcessCommunicatorImpl RegOnDeviceChange.
 * @tc.type: FUNC
 */
HWTEST_F(NativeProcessCommunicatorImplTest, ProcessCommunicatorImpl_RegOnDeviceChange_001, TestSize.Level1)
{
    ProcessCommunicatorImpl *processCommunicator = new ProcessCommunicatorImpl();
    auto ret =
        processCommunicator->RegOnDeviceChange([](const DistributedDB::DeviceInfos &devInfo, bool isOnline) -> void {
            return;
        });
    EXPECT_EQ(DistributedDB::DBStatus::OK, ret);
    ret = processCommunicator->RegOnDeviceChange(nullptr);
    EXPECT_EQ(DistributedDB::DBStatus::OK, ret);
    delete processCommunicator;
}

/**
 * @tc.name: ProcessCommunicatorImpl_RegOnDeviceChange_002
 * @tc.desc: test ProcessCommunicatorImpl RegOnDeviceChange.
 * @tc.type: FUNC
 */
HWTEST_F(NativeProcessCommunicatorImplTest, ProcessCommunicatorImpl_RegOnDeviceChange_002, TestSize.Level1)
{
    ProcessCommunicatorImpl *processCommunicator = new ProcessCommunicatorImpl();
    auto ret =
        processCommunicator->RegOnDeviceChange([](const DistributedDB::DeviceInfos &devInfo, bool isOnline) -> void {
            return;
        });
    EXPECT_EQ(DistributedDB::DBStatus::OK, ret);
    ret = processCommunicator->RegOnDeviceChange([](const DistributedDB::DeviceInfos &devInfo, bool isOnline) -> void {
        return;
    });
    EXPECT_EQ(DistributedDB::DBStatus::DB_ERROR, ret);
    ret = processCommunicator->RegOnDeviceChange(nullptr);
    EXPECT_EQ(DistributedDB::DBStatus::OK, ret);
    delete processCommunicator;
}

/**
 * @tc.name: ProcessCommunicatorImpl_RegOnDeviceChange_003
 * @tc.desc: test ProcessCommunicatorImpl RegOnDeviceChange.
 * @tc.type: FUNC
 */
HWTEST_F(NativeProcessCommunicatorImplTest, ProcessCommunicatorImpl_RegOnDeviceChange_003, TestSize.Level1)
{
    ProcessCommunicatorImpl *processCommunicator = new ProcessCommunicatorImpl();
    auto ret =
        processCommunicator->RegOnDeviceChange([](const DistributedDB::DeviceInfos &devInfo, bool isOnline) -> void {
            return;
        });
    EXPECT_EQ(DistributedDB::DBStatus::OK, ret);
    ret = processCommunicator->RegOnDeviceChange(nullptr);
    EXPECT_EQ(DistributedDB::DBStatus::OK, ret);
    ret = processCommunicator->RegOnDeviceChange(nullptr);
    EXPECT_EQ(DistributedDB::DBStatus::DB_ERROR, ret);
    delete processCommunicator;
}

/**
 * @tc.name: ProcessCommunicatorImpl_RegOnDataReceive_001
 * @tc.desc: test ProcessCommunicatorImpl RegOnDataReceive.
 * @tc.type: FUNC
 */
HWTEST_F(NativeProcessCommunicatorImplTest, ProcessCommunicatorImpl_RegOnDataReceive_001, TestSize.Level1)
{
    ProcessCommunicatorImpl *processCommunicator = new ProcessCommunicatorImpl();
    auto ret = processCommunicator->RegOnDataReceive(
        [](const DistributedDB::DeviceInfos &srcDevInfo, const uint8_t *data, uint32_t length) -> void {
            return;
        });
    EXPECT_EQ(DistributedDB::DBStatus::DB_ERROR, ret);
    delete processCommunicator;
}

/**
 * @tc.name: ProcessCommunicatorImpl_RegOnDataReceive_002
 * @tc.desc: test ProcessCommunicatorImpl RegOnDataReceive.
 * @tc.type: FUNC
 */
HWTEST_F(NativeProcessCommunicatorImplTest, ProcessCommunicatorImpl_RegOnDataReceive_002, TestSize.Level1)
{
    ProcessCommunicatorImpl *processCommunicator = new ProcessCommunicatorImpl();
    auto ret = processCommunicator->RegOnDataReceive(nullptr);
    EXPECT_EQ(DistributedDB::DBStatus::DB_ERROR, ret);
    delete processCommunicator;
}

/**
 * @tc.name: ProcessCommunicatorImpl_SendData_001
 * @tc.desc: test ProcessCommunicatorImpl SendData.
 * @tc.type: FUNC
 */
HWTEST_F(NativeProcessCommunicatorImplTest, ProcessCommunicatorImpl_SendData_001, TestSize.Level1)
{
    std::string processLabel = "processLabel01";
    DistributedDB::DeviceInfos deviceInfos = { "identifier" };
    uint8_t data = 1;
    uint32_t length = 1;
    ProcessCommunicatorImpl *processCommunicator = new ProcessCommunicatorImpl();
    auto ret = processCommunicator->SendData(deviceInfos, &data, length);
    EXPECT_EQ(DistributedDB::DBStatus::DB_ERROR, ret);
    delete processCommunicator;
}

/**
 * @tc.name: ProcessCommunicatorImpl_GetMtuSize_001
 * @tc.desc: test ProcessCommunicatorImpl GetMtuSize.
 * @tc.type: FUNC
 */
HWTEST_F(NativeProcessCommunicatorImplTest, ProcessCommunicatorImpl_GetMtuSize_001, TestSize.Level1)
{
    ProcessCommunicatorImpl *processCommunicator = new ProcessCommunicatorImpl();
    DistributedDB::DeviceInfos deviceInfos = { "identifier" };
    auto ret = processCommunicator->GetMtuSize(deviceInfos);
    EXPECT_EQ(MTU_SIZE, ret);
    ret = processCommunicator->GetMtuSize();
    EXPECT_EQ(MTU_SIZE, ret);
    delete processCommunicator;
}

/**
 * @tc.name: ProcessCommunicatorImpl_IsSameProcessLabelStartedOnPeerDevice_001
 * @tc.desc: test ProcessCommunicatorImpl IsSameProcessLabelStartedOnPeerDevice.
 * @tc.type: FUNC
 */
HWTEST_F(NativeProcessCommunicatorImplTest, IsSameProcessLabelStartedOnPeerDevice_001, TestSize.Level1)
{
    ProcessCommunicatorImpl *processCommunicator = new ProcessCommunicatorImpl();
    DistributedDB::DeviceInfos deviceInfos = { "identifier" };
    auto ret = processCommunicator->IsSameProcessLabelStartedOnPeerDevice(deviceInfos);
    EXPECT_EQ(false, ret);
    delete processCommunicator;
}
}
