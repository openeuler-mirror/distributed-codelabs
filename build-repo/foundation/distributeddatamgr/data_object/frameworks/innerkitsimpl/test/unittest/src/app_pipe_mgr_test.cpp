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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <string>

#include "app_pipe_mgr.h"
#include "app_types.h"
#include "auto_launch_export.h"
#include "ipc_skeleton.h"
#include "objectstore_errors.h"
#include "mock_app_data_change_listener.h"

namespace {
using namespace testing::ext;
using namespace OHOS::ObjectStore;

class NativeAppPipeMgrTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void NativeAppPipeMgrTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void NativeAppPipeMgrTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void NativeAppPipeMgrTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void NativeAppPipeMgrTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
 * @tc.name: NativeAppPipeMgrTest_StartWatchDataChange_001
 * @tc.desc: test NativeAppPipeMgrTest StartWatchDataChange. argument invalid
 * @tc.type: FUNC
 */
HWTEST_F(NativeAppPipeMgrTest, NativeAppPipeMgrTest_StartWatchDataChange_001, TestSize.Level1)
{
    PipeInfo pipeInfo = {""};
    AppDataChangeListener *observer = new MockAppDataChangeListener();
    AppPipeMgr *appPipeMgr = new AppPipeMgr();
    auto ret = appPipeMgr->StartWatchDataChange(observer, pipeInfo);
    EXPECT_EQ(Status::INVALID_ARGUMENT, ret);
}

/**
 * @tc.name: NativeAppPipeMgrTest_StartWatchDataChange_002
 * @tc.desc: test NativeAppPipeMgrTest StartWatchDataChange. pipId not found.
 * @tc.type: FUNC
 */
HWTEST_F(NativeAppPipeMgrTest, NativeAppPipeMgrTest_StartWatchDataChange_002, TestSize.Level1)
{
    PipeInfo pipeInfo = { "pipId01" };
    AppDataChangeListener *observer = new MockAppDataChangeListener();
    AppPipeMgr *appPipeMgr = new AppPipeMgr();
    auto ret = appPipeMgr->StartWatchDataChange(observer, pipeInfo);
    EXPECT_EQ(Status::ERROR, ret);
}

/**
 * @tc.name: NativeAppPipeMgrTest_StopWatchDataChange_001
 * @tc.desc: test NativeAppPipeMgrTest StopWatchDataChange. argument invalid
 * @tc.type: FUNC
 */
HWTEST_F(NativeAppPipeMgrTest, NativeAppPipeMgrTest_StopWatchDataChange_001, TestSize.Level1)
{
    PipeInfo pipeInfo = { "pipId01" };
    AppPipeMgr *appPipeMgr = new AppPipeMgr();
    auto ret = appPipeMgr->StopWatchDataChange(nullptr, pipeInfo);
    EXPECT_EQ(Status::INVALID_ARGUMENT, ret);
}

/**
 * @tc.name: NativeAppPipeMgrTest_StopWatchDataChange_002
 * @tc.desc: test NativeAppPipeMgrTest StopWatchDataChange.
 * @tc.type: FUNC
 */
HWTEST_F(NativeAppPipeMgrTest, NativeAppPipeMgrTest_StopWatchDataChange_002, TestSize.Level1)
{
    PipeInfo pipeInfo = { "pipId01" };
    AppPipeMgr *appPipeMgr = new AppPipeMgr();
    AppDataChangeListener *observer = new MockAppDataChangeListener();
    auto ret = appPipeMgr->StopWatchDataChange(observer, pipeInfo);
    EXPECT_EQ(Status::ERROR, ret);
}

/**
 * @tc.name: NativeAppPipeMgrTest_StopWatchDataChange_003
 * @tc.desc: test NativeAppPipeMgrTest StopWatchDataChange.
 * @tc.type: FUNC
 */
HWTEST_F(NativeAppPipeMgrTest, NativeAppPipeMgrTest_StopWatchDataChange_003, TestSize.Level1)
{
    PipeInfo pipeInfo = { "pipId01" };
    AppPipeMgr *appPipeMgr = new AppPipeMgr();
    AppDataChangeListener *observer = new MockAppDataChangeListener();
    auto ret = appPipeMgr->StopWatchDataChange(observer, pipeInfo);
    EXPECT_EQ(Status::ERROR, ret);
}

/**
 * @tc.name: NativeAppPipeMgrTest_Start_001
 * @tc.desc: test NativeAppPipeMgrTest Start. pipInfo is empty
 * @tc.type: FUNC
 */
HWTEST_F(NativeAppPipeMgrTest, NativeAppPipeMgrTest_Start_001, TestSize.Level1)
{
    PipeInfo pipeInfo = {};
    AppPipeMgr *appPipeMgr = new AppPipeMgr();
    auto ret = appPipeMgr->Start(pipeInfo);
    EXPECT_EQ(Status::INVALID_ARGUMENT, ret);
}

/**
 * @tc.name: NativeAppPipeMgrTest_Start_002
 * @tc.desc: test NativeAppPipeMgrTest Start. invalid pipInfo
 * @tc.type: FUNC
 */
HWTEST_F(NativeAppPipeMgrTest, NativeAppPipeMgrTest_Start_002, TestSize.Level1)
{
    PipeInfo pipeInfo = {"INVALID_SESSION_NAME"};
    AppPipeMgr *appPipeMgr = new AppPipeMgr();
    auto ret = appPipeMgr->Start(pipeInfo);
    EXPECT_EQ(Status::ILLEGAL_STATE, ret);
}

/**
 * @tc.name: NativeAppPipeMgrTest_Start_003
 * @tc.desc: test NativeAppPipeMgrTest Start.
 * @tc.type: FUNC
 */
HWTEST_F(NativeAppPipeMgrTest, NativeAppPipeMgrTest_Start_003, TestSize.Level1)
{
    PipeInfo pipeInfo = { "pipInfo01" };
    AppPipeMgr *appPipeMgr = new AppPipeMgr();
    auto ret = appPipeMgr->Start(pipeInfo);
    EXPECT_EQ(Status::SUCCESS, ret);
    ret = appPipeMgr->Stop(pipeInfo);
    EXPECT_EQ(Status::SUCCESS, ret);
}

/**
 * @tc.name: NativeAppPipeMgrTest_Start_004
 * @tc.desc: test NativeAppPipeMgrTest Start. repeat start
 * @tc.type: FUNC
 */
HWTEST_F(NativeAppPipeMgrTest, NativeAppPipeMgrTest_Start_004, TestSize.Level1)
{
    PipeInfo pipeInfo = { "pipInfo01" };
    AppPipeMgr *appPipeMgr = new AppPipeMgr();
    auto ret = appPipeMgr->Start(pipeInfo);
    EXPECT_EQ(Status::SUCCESS, ret);
    ret = appPipeMgr->Start(pipeInfo);
    EXPECT_EQ(Status::REPEATED_REGISTER, ret);
    ret = appPipeMgr->Stop(pipeInfo);
    EXPECT_EQ(Status::SUCCESS, ret);
}

/**
 * @tc.name: NativeAppPipeMgrTest_Stop_001
 * @tc.desc: test NativeAppPipeMgrTest Stop. pipInfo not found.
 * @tc.type: FUNC
 */
HWTEST_F(NativeAppPipeMgrTest, NativeAppPipeMgrTest_Stop_001, TestSize.Level1)
{
    PipeInfo pipeInfo = { "pipInfo01" };
    AppPipeMgr *appPipeMgr = new AppPipeMgr();
    auto ret = appPipeMgr->Stop(pipeInfo);
    EXPECT_EQ(Status::KEY_NOT_FOUND, ret);
}

/**
 * @tc.name: NativeAppPipeMgrTest_Stop_002
 * @tc.desc: test NativeAppPipeMgrTest Stop. RemoveSessionServer failed
 * @tc.type: FUNC
 */
HWTEST_F(NativeAppPipeMgrTest, NativeAppPipeMgrTest_Stop_002, TestSize.Level1)
{
    PipeInfo pipeInfo = { "REMOVE_FAILED_SESSION_NAME" };
    AppPipeMgr *appPipeMgr = new AppPipeMgr();
    auto ret = appPipeMgr->Start(pipeInfo);
    EXPECT_EQ(Status::SUCCESS, ret);
    ret = appPipeMgr->Stop(pipeInfo);
    EXPECT_EQ(Status::ERROR, ret);
}

/**
 * @tc.name: NativeAppPipeMgrTest_SendData_001
 * @tc.desc: test NativeAppPipeMgrTest SendData. input is invalid
 * @tc.type: FUNC
 */
HWTEST_F(NativeAppPipeMgrTest, NativeAppPipeMgrTest_SendData_001, TestSize.Level1)
{
    PipeInfo pipeInfo = {};
    DeviceId deviceId = {"devideId01"};
    int size = 1;
    uint8_t tmpNum = 1;
    uint8_t *ptr = &tmpNum;
    MessageInfo messageInfo = {MessageType::DEFAULT};
    AppPipeMgr *appPipeMgr = new AppPipeMgr();
    auto ret = appPipeMgr->SendData(pipeInfo, deviceId, ptr, size, messageInfo);
    EXPECT_EQ(Status::ERROR, ret);
}

/**
 * @tc.name: NativeAppPipeMgrTest_SendData_002
 * @tc.desc: test NativeAppPipeMgrTest SendData. pipInfo not found
 * @tc.type: FUNC
 */
HWTEST_F(NativeAppPipeMgrTest, NativeAppPipeMgrTest_SendData_002, TestSize.Level1)
{
    PipeInfo pipeInfo = {"pipInfo02"};
    DeviceId deviceId = {"devideId02"};
    int size = 1;
    uint8_t tmpNum = 1;
    uint8_t *ptr = &tmpNum;
    MessageInfo messageInfo = { MessageType::DEFAULT };
    AppPipeMgr *appPipeMgr = new AppPipeMgr();
    auto ret = appPipeMgr->SendData(pipeInfo, deviceId, ptr, size, messageInfo);
    EXPECT_EQ(Status::KEY_NOT_FOUND, ret);
}

/**
 * @tc.name: NativeAppPipeMgrTest_IsSameStartedOnPeer_001
 * @tc.desc: test NativeAppPipeMgrTest IsSameStartedOnPeer. pipInfo or deviceId is empty
 * @tc.type: FUNC
 */
HWTEST_F(NativeAppPipeMgrTest, NativeAppPipeMgrTest_IsSameStartedOnPeer_001, TestSize.Level1)
{
    PipeInfo pipeInfo = { "pipInfo01" };
    DeviceId deviceId = {};
    AppPipeMgr *appPipeMgr = new AppPipeMgr();
    auto ret = appPipeMgr->IsSameStartedOnPeer(pipeInfo, deviceId);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: NativeAppPipeMgrTest_IsSameStartedOnPeer_002
 * @tc.desc: test NativeAppPipeMgrTest IsSameStartedOnPeer. pipInfo not found
 * @tc.type: FUNC
 */
HWTEST_F(NativeAppPipeMgrTest, NativeAppPipeMgrTest_IsSameStartedOnPeer_002, TestSize.Level1)
{
    PipeInfo pipeInfo = { "pipInfo02" };
    DeviceId deviceId = {"deviceId02"};
    AppPipeMgr *appPipeMgr = new AppPipeMgr();
    auto ret = appPipeMgr->IsSameStartedOnPeer(pipeInfo, deviceId);
    EXPECT_EQ(false, ret);
}

}
