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

#include "gtest/gtest.h"
#include "softbus_app_info.h"
#include "softbus_errcode.h"
#include "softbus_json_utils.h"
#include "softbus_log.h"
#include "softbus_adapter_mem.h"
#include "trans_udp_negotiation_exchange.h"


using namespace testing::ext;

namespace OHOS {

#define TEST_SOCKET_ADDR "192.168.8.119"
#define TEST_ERROR_CODE (-12345)

const char *g_sessionKey = "www.huaweitest.com";
const char *g_pkgName = "dms";
const char *g_sessionName = "ohos.distributedschedule.dms.test";
const char *g_networkid = "ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00";
const char *g_groupid = "TEST_GROUP_ID";

class TransUdpNegoExchange : public testing::Test {
public:
    TransUdpNegoExchange()
    {}
    ~TransUdpNegoExchange()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override
    {}
    void TearDown() override
    {}
};

void TransUdpNegoExchange::SetUpTestCase(void)
{}

void TransUdpNegoExchange::TearDownTestCase(void)
{}

static void GenerateAppInfo(AppInfo *appInfo)
{
    if (appInfo == NULL) {
        appInfo = (AppInfo*)SoftBusMalloc(sizeof(AppInfo));
        EXPECT_TRUE(appInfo != NULL);
        memset_s(appInfo, sizeof(AppInfo), 0, sizeof(AppInfo));
    }
    int res = strcpy_s(appInfo->sessionKey, sizeof(appInfo->sessionKey), g_sessionKey);
    EXPECT_EQ(res, EOK);
    res = strcpy_s(appInfo->myData.addr, sizeof(appInfo->myData.addr), TEST_SOCKET_ADDR);
    EXPECT_EQ(res, EOK);
    res = strcpy_s(appInfo->peerData.addr, sizeof(appInfo->peerData.addr), TEST_SOCKET_ADDR);
    EXPECT_EQ(res, EOK);
    res = strcpy_s(appInfo->myData.sessionName, sizeof(appInfo->myData.sessionName), g_sessionName);
    EXPECT_EQ(res, EOK);
    res = strcpy_s(appInfo->peerData.sessionName, sizeof(appInfo->peerData.sessionName), g_sessionName);
    EXPECT_EQ(res, EOK);
    res = strcpy_s(appInfo->myData.pkgName, sizeof(appInfo->myData.pkgName), g_pkgName);
    EXPECT_EQ(res, EOK);
    res = strcpy_s(appInfo->peerData.pkgName, sizeof(appInfo->peerData.pkgName), g_pkgName);
    EXPECT_EQ(res, EOK);
    res = strcpy_s(appInfo->groupId, sizeof(appInfo->groupId), g_groupid);
    EXPECT_EQ(res, EOK);
}

/**
 * @tc.name: TransUdpNegoExchange01
 * @tc.desc: Transmission udp negotiation pack and unpack request with invalid parameters.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransUdpNegoExchange, TransUdpNegoExchange01, TestSize.Level1)
{
    AppInfo* appInfo = (AppInfo*)SoftBusMalloc(sizeof(AppInfo));
    EXPECT_TRUE(appInfo != NULL);
    memset_s(appInfo, sizeof(AppInfo), 0, sizeof(AppInfo));
    cJSON *msg = cJSON_CreateObject();
    int32_t ret = TransPackRequestUdpInfo(NULL, appInfo);
    EXPECT_EQ(ret, SOFTBUS_ERR);
    ret = TransUnpackRequestUdpInfo(NULL, appInfo);
    EXPECT_EQ(ret, SOFTBUS_ERR);
    ret = TransPackRequestUdpInfo(msg, NULL);
    EXPECT_EQ(ret, SOFTBUS_ERR);
    ret = TransUnpackRequestUdpInfo(msg, NULL);
    EXPECT_EQ(ret, SOFTBUS_ERR);
    cJSON_Delete(msg);
    SoftBusFree(appInfo);
}

/**
 * @tc.name: TransUdpNegoExchange02
 * @tc.desc: Transmission udp negotiation pack and unpack request.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransUdpNegoExchange, TransUdpNegoExchange02, TestSize.Level1)
{
    AppInfo* appInfo = (AppInfo*)SoftBusMalloc(sizeof(AppInfo));
    EXPECT_TRUE(appInfo != NULL);
    memset_s(appInfo, sizeof(AppInfo), 0, sizeof(AppInfo));
    GenerateAppInfo(appInfo);
    appInfo->udpChannelOptType = TYPE_UDP_CHANNEL_OPEN;
    cJSON *msg = cJSON_CreateObject();
    EXPECT_TRUE(msg != NULL);
    int32_t ret = TransPackRequestUdpInfo(msg, appInfo);
    EXPECT_EQ(ret, SOFTBUS_OK);
    memset_s(appInfo, sizeof(AppInfo), 0, sizeof(AppInfo));
    ret = TransUnpackRequestUdpInfo(msg, appInfo);
    EXPECT_EQ(ret, SOFTBUS_OK);
    cJSON_Delete(msg);
    msg = cJSON_CreateObject();
    appInfo->udpChannelOptType = TYPE_UDP_CHANNEL_CLOSE;
    ret = TransPackRequestUdpInfo(msg, appInfo);
    EXPECT_EQ(ret, SOFTBUS_OK);
    memset_s(appInfo, sizeof(AppInfo), 0, sizeof(AppInfo));
    ret = TransUnpackRequestUdpInfo(msg, appInfo);
    EXPECT_EQ(ret, SOFTBUS_OK);
    cJSON_Delete(msg);
    msg = cJSON_CreateObject();
    appInfo->udpChannelOptType = TYPE_INVALID_CHANNEL;
    ret = TransPackRequestUdpInfo(msg, appInfo);
    EXPECT_EQ(ret, SOFTBUS_ERR);
    memset_s(appInfo, sizeof(AppInfo), 0, sizeof(AppInfo));
    cJSON_Delete(msg);
    SoftBusFree(appInfo);
}

/**
 * @tc.name: TransUdpNegoExchange03
 * @tc.desc: Transmission udp negotiation pack and unpack reply with invalid parameters.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransUdpNegoExchange, TransUdpNegoExchange03, TestSize.Level1)
{
    AppInfo* appInfo = (AppInfo*)SoftBusMalloc(sizeof(AppInfo));
    EXPECT_TRUE(appInfo != NULL);
    memset_s(appInfo, sizeof(AppInfo), 0, sizeof(AppInfo));
    cJSON *msg = cJSON_CreateObject();
    int32_t ret = TransPackReplyUdpInfo(NULL, appInfo);
    EXPECT_EQ(ret, SOFTBUS_ERR);
    ret = TransUnpackReplyUdpInfo(NULL, appInfo);
    EXPECT_EQ(ret, SOFTBUS_ERR);
    ret = TransPackReplyUdpInfo(msg, NULL);
    EXPECT_EQ(ret, SOFTBUS_ERR);
    ret = TransUnpackReplyUdpInfo(msg, NULL);
    EXPECT_EQ(ret, SOFTBUS_ERR);
    cJSON_Delete(msg);
    SoftBusFree(appInfo);
}

/**
 * @tc.name: TransUdpNegoExchange04
 * @tc.desc: Transmission udp negotiation pack and unpack reply.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransUdpNegoExchange, TransUdpNegoExchange04, TestSize.Level1)
{
    AppInfo* appInfo = (AppInfo*)SoftBusMalloc(sizeof(AppInfo));
    EXPECT_TRUE(appInfo != NULL);
    memset_s(appInfo, sizeof(AppInfo), 0, sizeof(AppInfo));

    GenerateAppInfo(appInfo);
    appInfo->udpChannelOptType = TYPE_UDP_CHANNEL_OPEN;
    cJSON *msg = cJSON_CreateObject();
    EXPECT_TRUE(msg != NULL);
    int32_t ret = TransPackReplyUdpInfo(msg, appInfo);
    EXPECT_EQ(ret, SOFTBUS_OK);
    memset_s(appInfo, sizeof(AppInfo), 0, sizeof(AppInfo));
    appInfo->udpChannelOptType = TYPE_UDP_CHANNEL_OPEN;
    ret = TransUnpackReplyUdpInfo(msg, appInfo);
    EXPECT_EQ(ret, SOFTBUS_OK);
    cJSON_Delete(msg);

    msg = cJSON_CreateObject();
    appInfo->udpChannelOptType = TYPE_UDP_CHANNEL_CLOSE;
    GenerateAppInfo(appInfo);
    ret = TransPackReplyUdpInfo(msg, appInfo);
    EXPECT_EQ(ret, SOFTBUS_OK);
    memset_s(appInfo, sizeof(AppInfo), 0, sizeof(AppInfo));
    appInfo->udpChannelOptType = TYPE_UDP_CHANNEL_CLOSE;
    ret = TransUnpackReplyUdpInfo(msg, appInfo);
    EXPECT_EQ(ret, SOFTBUS_OK);
    cJSON_Delete(msg);
    SoftBusFree(appInfo);
}

/**
 * @tc.name: TransUdpNegoExchange05
 * @tc.desc: Transmission udp negotiation pack and unpack reply with invalid channel option type.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransUdpNegoExchange, TransUdpNegoExchange05, TestSize.Level1)
{
    AppInfo* appInfo = (AppInfo*)SoftBusMalloc(sizeof(AppInfo));
    EXPECT_TRUE(appInfo != NULL);
    memset_s(appInfo, sizeof(AppInfo), 0, sizeof(AppInfo));
    cJSON *msg = cJSON_CreateObject();
    GenerateAppInfo(appInfo);
    appInfo->udpChannelOptType = TYPE_INVALID_CHANNEL;
    int32_t ret = TransPackReplyUdpInfo(msg, appInfo);
    EXPECT_EQ(ret, SOFTBUS_ERR);
    memset_s(appInfo, sizeof(AppInfo), 0, sizeof(AppInfo));
    cJSON_Delete(msg);

    msg = cJSON_CreateObject();
    GenerateAppInfo(appInfo);
    appInfo->udpChannelOptType = TYPE_UDP_CHANNEL_CLOSE;
    ret = TransPackReplyUdpInfo(msg, appInfo);
    EXPECT_EQ(ret, SOFTBUS_OK);
    memset_s(appInfo, sizeof(AppInfo), 0, sizeof(AppInfo));
    appInfo->udpChannelOptType = TYPE_INVALID_CHANNEL;
    ret = TransUnpackReplyUdpInfo(msg, appInfo);
    EXPECT_EQ(ret, SOFTBUS_ERR);
    cJSON_Delete(msg);
    SoftBusFree(appInfo);
}

/**
 * @tc.name: TransUdpNegoExchange06
 * @tc.desc: Transmission udp negotiation pack and unpack error info with invalid parameters.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransUdpNegoExchange, TransUdpNegoExchange06, TestSize.Level1)
{
    cJSON *msg = cJSON_CreateObject();
    int errCode = TEST_ERROR_CODE;
    int32_t ret = TransPackReplyErrInfo(msg, errCode, NULL);
    EXPECT_EQ(ret, SOFTBUS_ERR);
    ret = TransUnpackReplyErrInfo(NULL, &errCode);
    EXPECT_EQ(ret, SOFTBUS_ERR);
    ret = TransPackReplyErrInfo(NULL, errCode, "error descriptor test");
    EXPECT_EQ(ret, SOFTBUS_ERR);
    ret = TransUnpackReplyErrInfo(msg, NULL);
    EXPECT_EQ(ret, SOFTBUS_ERR);
    cJSON_Delete(msg);
}

/**
 * @tc.name: TransUdpNegoExchange07
 * @tc.desc: Transmission udp negotiation pack and unpack error info.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransUdpNegoExchange, TransUdpNegoExchange07, TestSize.Level1)
{
    cJSON *msg = cJSON_CreateObject();
    EXPECT_TRUE(msg != NULL);
    int errCode = TEST_ERROR_CODE;
    int32_t ret = TransPackReplyErrInfo(msg, errCode, "error descriptor test");
    EXPECT_EQ(ret, SOFTBUS_OK);
    int recvErrcode = 0;
    ret = TransUnpackReplyErrInfo(msg, &recvErrcode);
    EXPECT_EQ(ret, SOFTBUS_OK);
    EXPECT_EQ(errCode, recvErrcode);
    cJSON_Delete(msg);
}

}