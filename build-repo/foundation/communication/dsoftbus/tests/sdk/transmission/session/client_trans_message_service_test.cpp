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

#include <securec.h>

#include <gtest/gtest.h>
#include "softbus_def.h"
#include "softbus_errcode.h"
#include "softbus_log.h"
#include "softbus_trans_def.h"
#include "softbus_json_utils.h"
#include "softbus_app_info.h"
#include "softbus_config_type.h"
#include "softbus_server_frame.h"
#include "softbus_adapter_mem.h"
#include "client_trans_message_service.h"
#include "client_trans_message_service.c"
#include "client_trans_session_manager.h"
#include "softbus_common.h"


#define TRANS_TEST_SESSION_ID 10
#define TRANS_TEST_CHANNEL_ID 1000
#define TRANS_TEST_FILE_ENCRYPT 10
#define TRANS_TEST_ALGORITHM 1
#define TRANS_TEST_CRC 1
#define TRANS_TEST_AUTH_DATA "test auth message data"
#define TRANS_TEST_MAX_MSG_LEN (1 * 1024)
#define TRANS_TEST_MAX_BYTES_LEN (2 * 1024)
#define TRANS_TEST_SEND_LEN 123
#define TRANS_TEST_FILE_COUNT 2

#define TRANS_TEST_INVALID_SEND_LEN (1024 * 1024)

using namespace testing::ext;

namespace OHOS {

const char *g_pkgName = "dms";
const char *g_sessionName = "ohos.distributedschedule.dms.test";
const char *g_sessionKey = "www.huaweitest.com";
const char *g_networkId = "ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF0";
const char *g_deviceId = "ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF0";
const char *g_groupId = "TEST_GROUP_ID";
static SessionAttribute g_sessionAttr = {
    .dataType = TYPE_BYTES,
};
class TransClientMsgServiceTest : public testing::Test {
public:
    TransClientMsgServiceTest()
    {}
    ~TransClientMsgServiceTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override
    {}
    void TearDown() override
    {}
};

void TransClientMsgServiceTest::SetUpTestCase(void)
{
    InitSoftBusServer();
    int32_t ret = TransClientInit();
    ASSERT_EQ(ret,  SOFTBUS_OK);
}

void TransClientMsgServiceTest::TearDownTestCase(void)
{
}

static int OnSessionOpened(int sessionId, int result)
{
    LOG_INFO("session opened,sesison id = %d\r\n", sessionId);
    return SOFTBUS_OK;
}

static void OnSessionClosed(int sessionId)
{
    LOG_INFO("session closed, session id = %d\r\n", sessionId);
}

static void OnBytesReceived(int sessionId, const void *data, unsigned int len)
{
    LOG_INFO("session bytes received, session id = %d\r\n", sessionId);
}

static void OnMessageReceived(int sessionId, const void *data, unsigned int len)
{
    LOG_INFO("session msg received, session id = %d\r\n", sessionId);
}

static void OnStreamReceived(int sessionId, const StreamData *data, const StreamData *ext, const StreamFrameInfo *param)
{
    LOG_INFO("session stream received, session id = %d\r\n", sessionId);
}

static void OnQosEvent(int sessionId, int eventId, int tvCount, const QosTv *tvList)
{
    LOG_INFO("session Qos event emit, session id = %d\r\n", sessionId);
}

static ISessionListener g_sessionlistener = {
    .OnSessionOpened = OnSessionOpened,
    .OnSessionClosed = OnSessionClosed,
    .OnBytesReceived = OnBytesReceived,
    .OnMessageReceived = OnMessageReceived,
    .OnStreamReceived = OnStreamReceived,
    .OnQosEvent = OnQosEvent,
};

static void TestGenerateCommParam(SessionParam *sessionParam)
{
    sessionParam->sessionName = g_sessionName;
    sessionParam->peerSessionName = g_sessionName;
    sessionParam->peerDeviceId = g_deviceId;
    sessionParam->groupId = g_groupId;
    sessionParam->attr = &g_sessionAttr;
}

static SessionInfo *TestGenerateSession(const SessionParam *param)
{
    SessionInfo *session = (SessionInfo*)SoftBusCalloc(sizeof(SessionInfo));
    if (session == NULL) {
        return NULL;
    }

    if (strcpy_s(session->info.peerSessionName, SESSION_NAME_SIZE_MAX, param->peerSessionName) != EOK ||
        strcpy_s(session->info.peerDeviceId, DEVICE_ID_SIZE_MAX, param->peerDeviceId) != EOK ||
        strcpy_s(session->info.groupId, GROUP_ID_SIZE_MAX, param->groupId) != EOK) {
        SoftBusFree(session);
        return NULL;
    }

    session->sessionId = TRANS_TEST_SESSION_ID;
    session->channelId = TRANS_TEST_CHANNEL_ID;
    session->channelType = CHANNEL_TYPE_BUTT;
    session->isServer = false;
    session->isEnable = false;
    session->routeType = ROUTE_TYPE_ALL;
    session->info.flag = TYPE_BYTES;
    session->isEncrypt = true;
    session->algorithm = TRANS_TEST_ALGORITHM;
    session->fileEncrypt = TRANS_TEST_FILE_ENCRYPT;
    session->crc = TRANS_TEST_CRC;

    return session;
}

static int32_t AddSessionServerAndSession(const char *sessionName, int32_t channelType, int32_t businessType,
                                          bool isServer, bool isEnable)
{
    SessionParam *sessionParam = (SessionParam*)SoftBusCalloc(sizeof(SessionParam));
    if (sessionParam == NULL) {
        return SOFTBUS_ERR;
    }

    TestGenerateCommParam(sessionParam);
    sessionParam->sessionName = sessionName;
    int32_t ret = ClientAddSessionServer(SEC_TYPE_PLAINTEXT, g_pkgName, sessionName, &g_sessionlistener);
    if (ret != SOFTBUS_OK) {
        return SOFTBUS_ERR;
    }

    SessionInfo *session = TestGenerateSession(sessionParam);
    if (session == NULL) {
        return SOFTBUS_ERR;
    }

    session->channelType = (ChannelType)channelType;
    session->businessType = (BusinessType)businessType;
    session->isServer = isServer;
    session->isEnable = isEnable;
    ret = ClientAddNewSession(sessionName, session);
    if (ret != SOFTBUS_OK) {
        return SOFTBUS_ERR;
    }

    int32_t sessionId = 0;
    ret = ClientGetSessionIdByChannelId(TRANS_TEST_CHANNEL_ID, channelType, &sessionId);
    if (ret != SOFTBUS_OK) {
        return SOFTBUS_ERR;
    }
    SoftBusFree(sessionParam);
    return sessionId;
}

static void DeleteSessionServerAndSession(const char *sessionName, int32_t sessionId)
{
    int32_t ret = ClientDeleteSession(sessionId);
    EXPECT_EQ(ret, SOFTBUS_OK);
    ret = ClientDeleteSessionServer(SEC_TYPE_PLAINTEXT, sessionName);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/**
 * @tc.name: TransClientMsgServiceTest01
 * @tc.desc: Transmission sdk message service check send length with different parameters.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransClientMsgServiceTest, TransClientMsgServiceTest01, TestSize.Level1)
{
    int ret = CheckSendLen(CHANNEL_TYPE_BUTT, BUSINESS_TYPE_MESSAGE, TRANS_TEST_SEND_LEN);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
    ret = CheckSendLen(CHANNEL_TYPE_AUTH, BUSINESS_TYPE_MESSAGE, TRANS_TEST_INVALID_SEND_LEN);
    EXPECT_EQ(ret, SOFTBUS_TRANS_SEND_LEN_BEYOND_LIMIT);
    ret = CheckSendLen(CHANNEL_TYPE_AUTH, BUSINESS_TYPE_MESSAGE, TRANS_TEST_SEND_LEN);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/**
 * @tc.name: TransClientMsgServiceTest02
 * @tc.desc: Transmission sdk message service send bytes with different parameters.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransClientMsgServiceTest, TransClientMsgServiceTest02, TestSize.Level1)
{
    int32_t sessionId = AddSessionServerAndSession(g_sessionName, CHANNEL_TYPE_PROXY, BUSINESS_TYPE_BUTT, false, true);
    ASSERT_GT(sessionId, 0);
    int ret = SendBytes(sessionId, TRANS_TEST_AUTH_DATA, (unsigned int)strlen(TRANS_TEST_AUTH_DATA));
    EXPECT_EQ(ret, SOFTBUS_TRANS_BUSINESS_TYPE_NOT_MATCH);
    DeleteSessionServerAndSession(g_sessionName, sessionId);
    sessionId = AddSessionServerAndSession(g_sessionName, CHANNEL_TYPE_AUTH, BUSINESS_TYPE_NOT_CARE, false, true);
    ASSERT_GT(sessionId, 0);
    ret = SendBytes(sessionId, TRANS_TEST_AUTH_DATA, (unsigned int)strlen(TRANS_TEST_AUTH_DATA));
    EXPECT_EQ(ret, SOFTBUS_ERR);
    DeleteSessionServerAndSession(g_sessionName, sessionId);
}

/**
 * @tc.name: TransClientMsgServiceTest03
 * @tc.desc: Transmission sdk message service send message with different parameters.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransClientMsgServiceTest, TransClientMsgServiceTest03, TestSize.Level1)
{
    int32_t sessionId = AddSessionServerAndSession(g_sessionName, CHANNEL_TYPE_UDP, BUSINESS_TYPE_BUTT, false, true);
    ASSERT_GT(sessionId, 0);
    int ret = SendMessage(sessionId, TRANS_TEST_AUTH_DATA, (unsigned int)strlen(TRANS_TEST_AUTH_DATA));
    EXPECT_EQ(ret, SOFTBUS_TRANS_SEND_LEN_BEYOND_LIMIT);
    DeleteSessionServerAndSession(g_sessionName, sessionId);
    sessionId = AddSessionServerAndSession(g_sessionName, CHANNEL_TYPE_PROXY, BUSINESS_TYPE_BUTT, false, true);
    ASSERT_GT(sessionId, 0);
    ret = SendMessage(sessionId, TRANS_TEST_AUTH_DATA, (unsigned int)strlen(TRANS_TEST_AUTH_DATA));
    EXPECT_EQ(ret, SOFTBUS_TRANS_BUSINESS_TYPE_NOT_MATCH);
    DeleteSessionServerAndSession(g_sessionName, sessionId);
    sessionId = AddSessionServerAndSession(g_sessionName, CHANNEL_TYPE_AUTH, BUSINESS_TYPE_NOT_CARE, false, true);
    ASSERT_GT(sessionId, 0);
    ret = SendMessage(sessionId, TRANS_TEST_AUTH_DATA, (unsigned int)strlen(TRANS_TEST_AUTH_DATA));
    EXPECT_EQ(ret, SOFTBUS_ERR);
    DeleteSessionServerAndSession(g_sessionName, sessionId);
}

/**
 * @tc.name: TransClientMsgServiceTest04
 * @tc.desc: Transmission sdk message service send stream with different parameters.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransClientMsgServiceTest, TransClientMsgServiceTest04, TestSize.Level1)
{
    char dataBuf[TRANS_TEST_MAX_MSG_LEN] = {0};
    StreamData data = {
        .buf = dataBuf,
        .bufLen = TRANS_TEST_MAX_MSG_LEN
    };
    char extBuf[TRANS_TEST_MAX_MSG_LEN] = {0};
    StreamData ext = {
        .buf = extBuf,
        .bufLen = TRANS_TEST_MAX_MSG_LEN
    };
    StreamFrameInfo streamFrameInfo = {0};
    int32_t sessionId = AddSessionServerAndSession(g_sessionName, CHANNEL_TYPE_BUTT, BUSINESS_TYPE_BUTT, false, true);
    ASSERT_GT(sessionId, 0);
    int ret = SendStream(sessionId, &data, &ext, &streamFrameInfo);
    EXPECT_EQ(ret, SOFTBUS_TRANS_STREAM_ONLY_UDP_CHANNEL);
    DeleteSessionServerAndSession(g_sessionName, sessionId);
    sessionId = AddSessionServerAndSession(g_sessionName, CHANNEL_TYPE_UDP, BUSINESS_TYPE_BUTT, false, false);
    ASSERT_GT(sessionId, 0);
    ret = SendStream(sessionId, &data, &ext, &streamFrameInfo);
    EXPECT_EQ(ret, SOFTBUS_TRANS_SESSION_NO_ENABLE);
    DeleteSessionServerAndSession(g_sessionName, sessionId);
    sessionId = AddSessionServerAndSession(g_sessionName, CHANNEL_TYPE_UDP, BUSINESS_TYPE_BUTT, false, true);
    ASSERT_GT(sessionId, 0);
    ret = SendStream(sessionId, &data, &ext, &streamFrameInfo);
    EXPECT_EQ(ret, SOFTBUS_TRANS_BUSINESS_TYPE_NOT_MATCH);
    DeleteSessionServerAndSession(g_sessionName, sessionId);
    sessionId = AddSessionServerAndSession(g_sessionName, CHANNEL_TYPE_UDP, BUSINESS_TYPE_STREAM, false, true);
    ASSERT_GT(sessionId, 0);
    ret = SendStream(sessionId, &data, &ext, &streamFrameInfo);
    EXPECT_NE(ret, SOFTBUS_OK);
    DeleteSessionServerAndSession(g_sessionName, sessionId);
}

/**
 * @tc.name: TransClientMsgServiceTest05
 * @tc.desc: Transmission sdk message service send file with different parameters.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransClientMsgServiceTest, TransClientMsgServiceTest05, TestSize.Level1)
{
    const char *sFileList[TRANS_TEST_FILE_COUNT] = {
        "/DATA/LOCAL/X",
        "/DATA/LOCAL/Y"
    };
    const char *dFileList[TRANS_TEST_FILE_COUNT] = {
        "/DATA/LOCAL/X",
        "/DATA/LOCAL/Y"
    };
    int32_t sessionId = AddSessionServerAndSession(g_sessionName, CHANNEL_TYPE_UDP, BUSINESS_TYPE_BUTT, false, true);
    ASSERT_GT(sessionId, 0);
    int ret = SendFile(sessionId, sFileList, dFileList, TRANS_TEST_FILE_COUNT);
    EXPECT_EQ(ret, SOFTBUS_TRANS_BUSINESS_TYPE_NOT_MATCH);
    DeleteSessionServerAndSession(g_sessionName, sessionId);
    sessionId = AddSessionServerAndSession(g_sessionName, CHANNEL_TYPE_AUTH, BUSINESS_TYPE_NOT_CARE, false, false);
    ASSERT_GT(sessionId, 0);
    ret = SendFile(sessionId, sFileList, dFileList, TRANS_TEST_FILE_COUNT);
    EXPECT_EQ(ret, SOFTBUS_TRANS_SESSION_NO_ENABLE);
    DeleteSessionServerAndSession(g_sessionName, sessionId);
    sessionId = AddSessionServerAndSession(g_sessionName, CHANNEL_TYPE_AUTH, BUSINESS_TYPE_NOT_CARE, false, true);
    ASSERT_GT(sessionId, 0);
    ret = SendFile(sessionId, sFileList, dFileList, TRANS_TEST_FILE_COUNT);
    EXPECT_EQ(ret, SOFTBUS_TRANS_CHANNEL_TYPE_INVALID);
    DeleteSessionServerAndSession(g_sessionName, sessionId);
}
}