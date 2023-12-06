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

#include <gtest/gtest.h>
#include "securec.h"

#include "client_trans_udp_manager.h"
#include "softbus_def.h"
#include "softbus_errcode.h"
#include "session.h"
#include "trans_udp_channel_manager.h"
#include "client_trans_session_callback.h"

using namespace std;
using namespace testing::ext;

const char *g_sessionName = "ohos.distributedschedule.dms.test";
const char *g_networkid = "ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00";
const char *g_groupid = "TEST_GROUP_ID";

namespace OHOS {
#define TEST_ERR_PID (-1)
#define TEST_LEN 10
#define TEST_DATA_TYPE 2
#define TEST_PID 2
#define TEST_STATE 1
#define TEST_ERR_CODE 1
#define TEST_CHANNELID 5
#define TEST_CHANNELTYPE 2
#define TEST_REMOTE_TYPE 0
#define TEST_EVENT_ID 2
#define TEST_COUNT 2
#define TEST_ERR_COUNT (-2)
#define TEST_ERRCODE 0
#define TEST_FILE_NAME "test.filename.01"
#define STREAM_DATA_LENGTH 10

class ClientTransUdpManagerTest : public testing::Test {
public:
    ClientTransUdpManagerTest() {}
    ~ClientTransUdpManagerTest() {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override {}
    void TearDown() override {}
};

void ClientTransUdpManagerTest::SetUpTestCase(void) {}

void ClientTransUdpManagerTest::TearDownTestCase(void) {}

/**
 * @tc.name: TransOnUdpChannelOpenedTest001
 * @tc.desc: trans on udp channel opened test, use the wrong parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransUdpManagerTest, TransOnUdpChannelOpenedTest001, TestSize.Level0)
{
    int32_t ret;
    ChannelInfo channel;
    int32_t udpPort;

    ret = TransOnUdpChannelOpened(NULL, &channel, &udpPort);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    ret = TransOnUdpChannelOpened(g_sessionName, NULL, &udpPort);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    ret = TransOnUdpChannelOpened(g_sessionName, &channel, NULL);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/**
 * @tc.name: TransOnUdpChannelOpenedTest002
 * @tc.desc: trans on udp channel opened test, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransUdpManagerTest, TransOnUdpChannelOpenedTest002, TestSize.Level0)
{
    int32_t ret;
    ChannelInfo channel;
    char strTmp[] = "ABCDEFG";
    char strSessionName[] = "ohos.distributedschedule.dms.test";
    int32_t udpPort;
    channel.channelId = TEST_CHANNELID;
    channel.businessType = BUSINESS_TYPE_STREAM;
    channel.channelType = TEST_CHANNELTYPE;
    channel.fd = TEST_DATA_TYPE;
    channel.isServer = true;
    channel.isEnabled = true;
    channel.peerUid = TEST_CHANNELID;
    channel.peerPid = TEST_CHANNELID;
    channel.groupId = strTmp;
    channel.sessionKey = strTmp;
    channel.keyLen = sizeof(channel.sessionKey);
    channel.peerSessionName = strSessionName;
    channel.peerDeviceId = strTmp;
    channel.myIp = strTmp;
    channel.streamType = TEST_COUNT;
    channel.isUdpFile = true;
    channel.peerPort = TEST_COUNT;
    channel.peerIp = strTmp;
    channel.routeType = TEST_DATA_TYPE;

    ret = TransOnUdpChannelOpened(g_sessionName, &channel, &udpPort);
    EXPECT_EQ(SOFTBUS_OK, ret);

    ret = TransOnUdpChannelClosed(channel.channelId);
    EXPECT_EQ(SOFTBUS_OK, ret);

    channel.businessType = BUSINESS_TYPE_FILE;
    ret = TransOnUdpChannelOpened(strSessionName, &channel, &udpPort);
    EXPECT_EQ(SOFTBUS_ERR, ret);

    channel.businessType = BUSINESS_TYPE_FILE;
    channel.channelId = TEST_CHANNELID + 1;
    ret = TransOnUdpChannelOpened(g_sessionName, &channel, &udpPort);
    EXPECT_EQ(SOFTBUS_ERR, ret);

    channel.businessType = TEST_COUNT;
    ret = TransOnUdpChannelOpened(g_sessionName, &channel, &udpPort);
    EXPECT_EQ(SOFTBUS_ERR, ret);
}

/**
 * @tc.name: TransOnUdpChannelOpenedTest003
 * @tc.desc: trans on udp channel opened test, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransUdpManagerTest, TransOnUdpChannelOpenedTest003, TestSize.Level0)
{
    int32_t ret;
    ChannelInfo channel;
    char strTmp[] = "ABCDEFG";
    char strSessionName[] = "ohos.distributedschedule.dms.test";
    int32_t udpPort;
    QosTv tvList;
    channel.channelId = TEST_CHANNELID;
    channel.businessType = BUSINESS_TYPE_STREAM;
    channel.channelType = TEST_CHANNELTYPE;
    channel.fd = TEST_DATA_TYPE;
    channel.isServer = true;
    channel.isEnabled = true;
    channel.peerUid = TEST_CHANNELID;
    channel.peerPid = TEST_CHANNELID;
    channel.groupId = strTmp;
    channel.sessionKey = strTmp;
    channel.keyLen = sizeof(channel.sessionKey);
    channel.peerSessionName = strSessionName;
    channel.peerDeviceId = strTmp;
    channel.myIp = strTmp;
    channel.streamType = TEST_COUNT;
    channel.isUdpFile = true;
    channel.peerPort = TEST_COUNT;
    channel.peerIp = strTmp;
    channel.routeType = TEST_DATA_TYPE;

    ret = TransOnUdpChannelOpened(g_sessionName, &channel, &udpPort);
    EXPECT_EQ(SOFTBUS_OK, ret);

    ret = TransOnUdpChannelQosEvent(TEST_CHANNELID, TEST_EVENT_ID, TEST_COUNT, &tvList);
    EXPECT_EQ(SOFTBUS_OK, ret);

    channel.businessType = BUSINESS_TYPE_BUTT;
    ret = TransOnUdpChannelOpenFailed(TEST_CHANNELID, TEST_ERRCODE);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/**
 * @tc.name: TransOnUdpChannelOpenFailedTest001
 * @tc.desc: trans on udp channel opened test, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransUdpManagerTest, TransOnUdpChannelOpenFailedTest001, TestSize.Level0)
{
    int32_t ret;
    ret = TransOnUdpChannelOpenFailed(TEST_CHANNELID, TEST_ERRCODE);
    EXPECT_EQ(SOFTBUS_OK, ret);

    ret = TransOnUdpChannelOpenFailed(0, TEST_ERRCODE);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/**
 * @tc.name: TransOnUdpChannelClosedTest001
 * @tc.desc: trans on udp channel closed test, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransUdpManagerTest, TransOnUdpChannelClosedTest001, TestSize.Level0)
{
    int32_t ret;
    ret = TransOnUdpChannelClosed(TEST_CHANNELID);
    EXPECT_EQ(SOFTBUS_ERR, ret);
}

/**
 * @tc.name: TransOnUdpChannelQosEventTest001
 * @tc.desc: trans on udp channel qos event test, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransUdpManagerTest, TransOnUdpChannelQosEventTest001, TestSize.Level0)
{
    int32_t ret;
    QosTv tvList;
    ret = TransOnUdpChannelQosEvent(TEST_CHANNELID, TEST_EVENT_ID, TEST_COUNT, &tvList);
    EXPECT_EQ(SOFTBUS_ERR, ret);
}

/**
 * @tc.name: ClientTransCloseUdpChannelTest001
 * @tc.desc: client trans close udp channel test, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransUdpManagerTest, ClientTransCloseUdpChannelTest001, TestSize.Level0)
{
    int32_t ret;
    ret = ClientTransCloseUdpChannel(TEST_CHANNELID);
    EXPECT_EQ(SOFTBUS_ERR, ret);
}

/**
 * @tc.name: TransUdpChannelSendStreamTest001
 * @tc.desc: trans udp channel send stream test, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransUdpManagerTest, TransUdpChannelSendStreamTest001, TestSize.Level0)
{
    int32_t ret;
    ChannelInfo channel;
    char strTmp[] = "ABCDEFG";
    char strSessionName[] = "ohos.distributedschedule.dms.test";
    int32_t udpPort;
    channel.channelId = TEST_CHANNELID;
    channel.businessType = BUSINESS_TYPE_STREAM;
    channel.channelType = TEST_CHANNELTYPE;
    channel.fd = TEST_DATA_TYPE;
    channel.isServer = true;
    channel.isEnabled = true;
    channel.peerUid = TEST_CHANNELID;
    channel.peerPid = TEST_CHANNELID;
    channel.groupId = strTmp;
    channel.sessionKey = strTmp;
    channel.keyLen = sizeof(channel.sessionKey);
    channel.peerSessionName = strSessionName;
    channel.peerDeviceId = strTmp;
    channel.myIp = strTmp;
    channel.streamType = TEST_COUNT;
    channel.isUdpFile = true;
    channel.peerPort = TEST_COUNT;
    channel.peerIp = strTmp;
    channel.routeType = TEST_DATA_TYPE;

    char sendStringData[STREAM_DATA_LENGTH] = "diudiudiu";
    StreamData tmpData = {
        sendStringData,
        STREAM_DATA_LENGTH,
    };
    char str[STREAM_DATA_LENGTH] = "oohoohooh";
    StreamData tmpData2 = {
        str,
        STREAM_DATA_LENGTH,
    };

    StreamFrameInfo tmpf = {};
    ret = TransUdpChannelSendStream(TEST_CHANNELID, &tmpData, &tmpData2, &tmpf);
    EXPECT_EQ(SOFTBUS_TRANS_UDP_GET_CHANNEL_FAILED, ret);

    ret = TransOnUdpChannelOpened(g_sessionName, &channel, &udpPort);
    EXPECT_EQ(SOFTBUS_TRANS_UDP_START_STREAM_SERVER_FAILED, ret);

    ret = TransUdpChannelSendStream(TEST_CHANNELID, &tmpData, &tmpData2, &tmpf);
    EXPECT_EQ(SOFTBUS_TRANS_UDP_GET_CHANNEL_FAILED, ret);
}

/**
 * @tc.name: TransUdpChannelSendFileTest001
 * @tc.desc: trans udp channel send file test, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransUdpManagerTest, TransUdpChannelSendFileTest001, TestSize.Level0)
{
    int32_t ret;
    const char *sFileList[] = {
        "/data/big.tar",
        "/data/richu.jpg",
        "/data/richu-002.jpg",
        "/data/richu-003.jpg",
    };
    ret = TransUdpChannelSendFile(TEST_CHANNELID, NULL, NULL, 1);
    EXPECT_EQ(SOFTBUS_TRANS_UDP_GET_CHANNEL_FAILED, ret);

    ret = TransUdpChannelSendFile(TEST_CHANNELID, sFileList, NULL, 0);
    EXPECT_EQ(SOFTBUS_TRANS_UDP_GET_CHANNEL_FAILED, ret);

    ret = TransUdpChannelSendFile(TEST_CHANNELID, sFileList, NULL, 1);
    EXPECT_EQ(SOFTBUS_TRANS_UDP_GET_CHANNEL_FAILED, ret);
}

/**
 * @tc.name: TransGetUdpChannelByFileIdTest001
 * @tc.desc: trans get udp channel by fileid test, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransUdpManagerTest, TransGetUdpChannelByFileIdTest001, TestSize.Level0)
{
    int32_t ret;
    UdpChannel udpChannel;
    (void)memset_s(&udpChannel, sizeof(UdpChannel), 0, sizeof(UdpChannel));
    ret = TransGetUdpChannelByFileId(TEST_DATA_TYPE, &udpChannel);
    EXPECT_EQ(SOFTBUS_ERR, ret);
}

/**
 * @tc.name: ClientTransAddUdpChannelTest001
 * @tc.desc: client trans add udp channel test, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransUdpManagerTest, ClientTransAddUdpChannelTest001, TestSize.Level0)
{
    int32_t ret;
    ChannelInfo channel;
    char strTmp[] = "ABCDEFG";
    char strSessionName[] = "ohos.distributedschedule.dms.test";
    int32_t udpPort;
    channel.channelId = TEST_CHANNELID;
    channel.businessType = BUSINESS_TYPE_STREAM;
    channel.channelType = TEST_CHANNELTYPE;
    channel.fd = TEST_DATA_TYPE;
    channel.isServer = true;
    channel.isEnabled = true;
    channel.peerUid = TEST_CHANNELID;
    channel.peerPid = TEST_CHANNELID;
    channel.groupId = strTmp;
    channel.sessionKey = strTmp;
    channel.keyLen = sizeof(channel.sessionKey);
    channel.peerSessionName = strSessionName;
    channel.peerDeviceId = strTmp;
    channel.myIp = strTmp;
    channel.streamType = TEST_COUNT;
    channel.isUdpFile = true;
    channel.peerPort = TEST_COUNT;
    channel.peerIp = strTmp;
    channel.routeType = TEST_DATA_TYPE;

    ret = TransOnUdpChannelOpened(g_sessionName, &channel, &udpPort);
    EXPECT_EQ(SOFTBUS_TRANS_UDP_START_STREAM_SERVER_FAILED, ret);

    ret = TransOnUdpChannelOpened(g_sessionName, &channel, &udpPort);
    EXPECT_EQ(SOFTBUS_TRANS_UDP_START_STREAM_SERVER_FAILED, ret);

    ClientTransUdpMgrDeinit();
    ret = TransOnUdpChannelOpened(g_sessionName, &channel, &udpPort);
    EXPECT_EQ(SOFTBUS_TRANS_UDP_CLIENT_ADD_CHANNEL_FAILED, ret);
}

/**
 * @tc.name: ClientTransUdpManagerTest001
 * @tc.desc: client trans udp manager test, use the normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransUdpManagerTest, ClientTransUdpManagerTest001, TestSize.Level0)
{
    int32_t ret;

    IClientSessionCallBack *cb = GetClientSessionCb();
    ret = ClientTransUdpMgrInit(cb);
    EXPECT_TRUE(ret == SOFTBUS_OK);
}
} // namespace OHOS