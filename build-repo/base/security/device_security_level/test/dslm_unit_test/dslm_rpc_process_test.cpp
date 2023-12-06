/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "dslm_rpc_process_test.h"

#include "device_security_defines.h"
#include "device_security_info.h"
#include "dslm_msg_interface_mock.h"
#include "dslm_rpc_process.h"

using namespace testing;
using namespace testing::ext;

extern "C" int32_t OnPeerMsgReceived(const DeviceIdentify *devId, const uint8_t *msg, uint32_t len);
extern "C" int32_t OnSendResultNotifier(const DeviceIdentify *devId, uint64_t transNo, uint32_t result);

namespace OHOS {
namespace Security {
namespace DslmUnitTest {
void DslmRpcProcessTest::SetUpTestCase()
{
}

void DslmRpcProcessTest::TearDownTestCase()
{
}

void DslmRpcProcessTest::SetUp()
{
}

void DslmRpcProcessTest::TearDown()
{
}

/**
 * @tc.name: OnPeerMsgReceived_InvalidInputPara_001
 * @tc.desc: function OnPeerMsgReceived with null input
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmRpcProcessTest, OnPeerMsgReceived_InvalidInputPara_001, TestSize.Level0)
{
    int32_t ret;

    ret = OnPeerMsgReceived(NULL, NULL, 0);
    EXPECT_EQ(ret, ERR_INVALID_PARA);

    const DeviceIdentify identify = {DEVICE_ID_MAX_LEN, {0}};
    ret = OnPeerMsgReceived(&identify, NULL, 0);
    EXPECT_EQ(ret, ERR_INVALID_PARA);

    const uint8_t message[] = {'1', '2'};
    ret = OnPeerMsgReceived(&identify, message, 0);
    EXPECT_EQ(ret, ERR_INVALID_PARA);
}

/**
 * @tc.name: OnPeerMsgReceived_InvalidInputPara_002
 * @tc.desc: function OnPeerMsgReceived with invalid inputs
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmRpcProcessTest, OnPeerMsgReceived_InvalidInputPara_002, TestSize.Level0)
{
    int32_t ret;
    const DeviceIdentify identify = {DEVICE_ID_MAX_LEN, {0}};
    const uint8_t message1[] = {'1', '2', 3};
    ret = OnPeerMsgReceived(&identify, message1, 3);
    EXPECT_EQ(ret, ERR_INVALID_PARA);

    const uint8_t message2[] = {'1', 0xff, '2', '3'};
    ret = OnPeerMsgReceived(&identify, message2, 4);
    EXPECT_EQ(ret, ERR_INVALID_PARA);
}

/**
 * @tc.name: OnPeerMsgReceived_InvalidInputPara_003
 * @tc.desc: function OnPeerMsgReceived with invalid inputs
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmRpcProcessTest, OnPeerMsgReceived_InvalidInputPara_003, TestSize.Level0)
{
    const DeviceIdentify identify = {DEVICE_ID_MAX_LEN, {0}};
    const char *message = R"({"message":1,"payload":{"version":131072,"challenge111":"0102030405060708"}})";
    uint32_t messageLen = strlen(message) + 1;
    int32_t ret = OnPeerMsgReceived(&identify, (const uint8_t *)message, messageLen);
    EXPECT_EQ(ret, ERR_NO_CHALLENGE);
}

/**
 * @tc.name: OnPeerMsgReceived_InvalidInputPara_004
 * @tc.desc: function OnPeerMsgReceived with no valid device
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmRpcProcessTest, OnPeerMsgReceived_InvalidInputPara_004, TestSize.Level0)
{
    const DeviceIdentify identify = {DEVICE_ID_MAX_LEN, {0}};
    const char *message = R"({"message":2,"payload":{"version":131072,"challenge111":"0102030405060708"}})";
    uint32_t messageLen = strlen(message) + 1;
    int32_t ret = OnPeerMsgReceived(&identify, (const uint8_t *)message, messageLen);
    EXPECT_EQ(ret, ERR_NOEXIST_DEVICE);
}

/**
 * @tc.name: OnPeerMsgReceived_InvalidInputPara_005
 * @tc.desc: function OnPeerMsgReceived with invalid msg type
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmRpcProcessTest, OnPeerMsgReceived_InvalidInputPara_005, TestSize.Level0)
{
    const DeviceIdentify identify = {DEVICE_ID_MAX_LEN, {0}};
    const char *message = R"({"message":3,"payload":{"version":131072,"challenge111":"0102030405060708"}})";
    uint32_t messageLen = strlen(message) + 1;
    int32_t ret = OnPeerMsgReceived(&identify, (const uint8_t *)message, messageLen);
    EXPECT_EQ(ret, ERR_INVALID_PARA);
}

/**
 * @tc.name: OnSendResultNotifier_InvalidInputPara_001
 * @tc.desc: function OnSendResultNotifier test
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmRpcProcessTest, OnSendResultNotifier_InvalidInputPara_001, TestSize.Level0)
{
    const DeviceIdentify identify = {DEVICE_ID_MAX_LEN, {0}};
    int32_t ret = OnSendResultNotifier(&identify, 0, SUCCESS);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name: InitService_InvalidInputPara_001
 * @tc.desc: function InitService test with messenger not init
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmRpcProcessTest, InitService_InvalidInputPara_001, TestSize.Level0)
{
    uint32_t ret = InitService();
    EXPECT_EQ(ret, ERR_MSG_NOT_INIT);
}

/**
 * @tc.name: InitService_InvalidInputPara_002
 * @tc.desc: function InitService test with messenger init success
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmRpcProcessTest, InitService_InvalidInputPara_002, TestSize.Level0)
{
    DslmMsgInterfaceMock mock;

    auto fillUpDeviceIdentify = [](DeviceIdentify *identify) { identify->length = DEVICE_ID_MAX_LEN; };

    EXPECT_CALL(mock, IsMessengerReady(_)).Times(AtLeast(1)).WillRepeatedly(Return(true));
    EXPECT_CALL(mock, GetSelfDeviceIdentify(_, _, _))
        .WillOnce(Return(false))
        .WillRepeatedly(DoAll(WithArg<1>(fillUpDeviceIdentify), Return(true)));

    uint32_t ret = InitService();
    EXPECT_EQ(ret, SUCCESS);
}
} // namespace DslmUnitTest
} // namespace Security
} // namespace OHOS
