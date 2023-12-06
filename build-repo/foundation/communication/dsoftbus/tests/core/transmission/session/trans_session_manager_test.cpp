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
#include "softbus_app_info.h"
#include "softbus_server_frame.h"
#include "softbus_adapter_mem.h"
#include "trans_session_manager.h"

#define TRANS_TEST_INVALID_PID (-1)
#define TRANS_TEST_INVALID_UID (-1)

#define MAX_SESSION_SERVER_NUM 32

using namespace testing::ext;

namespace OHOS {

const char *g_pkgName = "dms";
const char *g_sessionName = "ohos.distributedschedule.dms.test";
const char *g_networkid = "ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00";
class TransSessionManagerTest : public testing::Test {
public:
    TransSessionManagerTest()
    {}
    ~TransSessionManagerTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override
    {}
    void TearDown() override
    {}
};

void TransSessionManagerTest::SetUpTestCase(void)
{
    InitSoftBusServer();
}

void TransSessionManagerTest::TearDownTestCase(void)
{
}

/**
 * @tc.name: TransSessionManagerTest01
 * @tc.desc: Transmission session manager initialize.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransSessionManagerTest, TransSessionManagerTest01, TestSize.Level1)
{
    int32_t ret = TransSessionMgrInit();
    EXPECT_EQ(ret,  SOFTBUS_OK);
    TransSessionMgrDeinit();
    ret = TransSessionMgrInit();
    EXPECT_EQ(ret,  SOFTBUS_OK);
    TransSessionMgrDeinit();
}

/**
 * @tc.name: TransSessionManagerTest02
 * @tc.desc: Transmission session manager judge whether session exists with invalid parameters.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransSessionManagerTest, TransSessionManagerTest02, TestSize.Level1)
{
    bool res = TransSessionServerIsExist(NULL);
    EXPECT_FALSE(res);
    res = TransSessionServerIsExist(g_sessionName);
    EXPECT_FALSE(res);
}

/**
 * @tc.name: TransSessionManagerTest03
 * @tc.desc: Transmission session manager add item with invalid parameters.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransSessionManagerTest, TransSessionManagerTest03, TestSize.Level1)
{
    int32_t ret = TransSessionServerAddItem(NULL);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
    SessionServer *sessionServer = (SessionServer*)SoftBusMalloc(sizeof(SessionServer));
    memset_s(sessionServer, sizeof(SessionServer), 0, sizeof(SessionServer));
    ret = TransSessionServerAddItem(sessionServer);
    EXPECT_EQ(ret, SOFTBUS_NO_INIT);
}

/**
 * @tc.name: TransSessionManagerTest04
 * @tc.desc: Transmission session manager del item with invalid parameters.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransSessionManagerTest, TransSessionManagerTest04, TestSize.Level1)
{
    int32_t ret = TransSessionServerDelItem(NULL);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
    ret = TransSessionServerDelItem(g_sessionName);
    EXPECT_EQ(ret, SOFTBUS_ERR);
}

/**
 * @tc.name: TransSessionManagerTest05
 * @tc.desc: Transmission session manager del item with not existed item.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransSessionManagerTest, TransSessionManagerTest05, TestSize.Level1)
{
    int32_t ret = TransSessionMgrInit();
    EXPECT_EQ(ret,  SOFTBUS_OK);
    TransSessionServerDelItem(g_sessionName);
    TransSessionMgrDeinit();
}

/**
 * @tc.name: TransSessionManagerTest06
 * @tc.desc: Transmission session manager get package name by session name with invalid parameters.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransSessionManagerTest, TransSessionManagerTest06, TestSize.Level1)
{
    int32_t ret = TransSessionMgrInit();
    EXPECT_EQ(ret,  SOFTBUS_OK);
    char pkgName[PKG_NAME_SIZE_MAX] = {0};
    ret = TransGetPkgNameBySessionName(NULL, pkgName, PKG_NAME_SIZE_MAX);
    EXPECT_EQ(ret,  SOFTBUS_ERR);
    ret = TransGetPkgNameBySessionName(g_sessionName, NULL, PKG_NAME_SIZE_MAX);
    EXPECT_EQ(ret,  SOFTBUS_ERR);
    ret = TransGetPkgNameBySessionName(g_sessionName, pkgName, 0);
    EXPECT_EQ(ret,  SOFTBUS_ERR);
    TransSessionMgrDeinit();
}

/**
 * @tc.name: TransSessionManagerTest07
 * @tc.desc: Transmission session manager delete item by package name with invalid parameters.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransSessionManagerTest, TransSessionManagerTest07, TestSize.Level1)
{
    int32_t ret = TransSessionMgrInit();
    EXPECT_EQ(ret, SOFTBUS_OK);
    SessionServer *sessionServer = (SessionServer*)SoftBusMalloc(sizeof(SessionServer));
    EXPECT_TRUE(sessionServer != NULL);
    memset_s(sessionServer, sizeof(SessionServer), 0, sizeof(SessionServer));
    ret = strcpy_s(sessionServer->sessionName, sizeof(sessionServer->sessionName), g_sessionName);
    EXPECT_EQ(ret, EOK);
    ret = strcpy_s(sessionServer->pkgName, sizeof(sessionServer->pkgName), g_pkgName);
    EXPECT_EQ(ret, EOK);
    sessionServer->pid = TRANS_TEST_INVALID_PID;
    ret = TransSessionServerAddItem(sessionServer);
    EXPECT_EQ(ret, SOFTBUS_OK);
    bool res = TransSessionServerIsExist(g_sessionName);
    EXPECT_TRUE(res);
    TransDelItemByPackageName(NULL);
    res = TransSessionServerIsExist(g_sessionName);
    EXPECT_TRUE(res);
    TransSessionMgrDeinit();
    TransDelItemByPackageName(g_pkgName);
}

/**
 * @tc.name: TransSessionManagerTest08
 * @tc.desc: Transmission session manager get uid and pid by session name with invalid parameters.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransSessionManagerTest, TransSessionManagerTest08, TestSize.Level1)
{
    int32_t pid = 0;
    int32_t uid = 0;
    int32_t ret = TransGetUidAndPid(NULL, &uid, &pid);
    EXPECT_EQ(ret,  SOFTBUS_INVALID_PARAM);
    ret = TransGetUidAndPid(g_sessionName, NULL, &pid);
    EXPECT_EQ(ret,  SOFTBUS_INVALID_PARAM);
    ret = TransGetUidAndPid(g_sessionName, &uid, NULL);
    EXPECT_EQ(ret,  SOFTBUS_INVALID_PARAM);
    ret = TransGetUidAndPid(g_sessionName, &uid, &pid);
    EXPECT_EQ(ret,  SOFTBUS_ERR);
}

/**
 * @tc.name: TransSessionManagerTest09
 * @tc.desc: Transmission session manager get package name by session name.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransSessionManagerTest, TransSessionManagerTest09, TestSize.Level1)
{
    int32_t ret = TransSessionMgrInit();
    EXPECT_EQ(ret,  SOFTBUS_OK);
    SessionServer *sessionServer = (SessionServer*)SoftBusMalloc(sizeof(SessionServer));
    EXPECT_TRUE(sessionServer != NULL);
    memset_s(sessionServer, sizeof(SessionServer), 0, sizeof(SessionServer));
    ret = strcpy_s(sessionServer->sessionName, sizeof(sessionServer->sessionName), g_sessionName);
    EXPECT_EQ(ret, EOK);
    ret = strcpy_s(sessionServer->pkgName, sizeof(sessionServer->pkgName), g_pkgName);
    EXPECT_EQ(ret, EOK);
    ret = TransSessionServerAddItem(sessionServer);
    EXPECT_EQ(ret, SOFTBUS_OK);
    char pkgName[PKG_NAME_SIZE_MAX] = {0};
    ret = TransGetPkgNameBySessionName(g_sessionName, pkgName, PKG_NAME_SIZE_MAX);
    EXPECT_EQ(ret,  SOFTBUS_OK);
    ret = strncmp(pkgName, g_pkgName, strlen(g_pkgName));
    EXPECT_EQ(ret,  EOK);
    ret = TransSessionServerDelItem(g_sessionName);
    EXPECT_EQ(ret, SOFTBUS_OK);
    TransSessionMgrDeinit();
}

/**
 * @tc.name: TransSessionManagerTest10
 * @tc.desc: Transmission session manager delete item by package name.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransSessionManagerTest, TransSessionManagerTest10, TestSize.Level1)
{
    int32_t ret = TransSessionMgrInit();
    EXPECT_EQ(ret,  SOFTBUS_OK);
    SessionServer *sessionServer = (SessionServer*)SoftBusMalloc(sizeof(SessionServer));
    EXPECT_TRUE(sessionServer != NULL);
    memset_s(sessionServer, sizeof(SessionServer), 0, sizeof(SessionServer));
    ret = strcpy_s(sessionServer->sessionName, sizeof(sessionServer->sessionName), g_sessionName);
    EXPECT_EQ(ret, EOK);
    ret = strcpy_s(sessionServer->pkgName, sizeof(sessionServer->pkgName), g_pkgName);
    EXPECT_EQ(ret, EOK);
    sessionServer->pid = TRANS_TEST_INVALID_PID;
    ret = TransSessionServerAddItem(sessionServer);
    EXPECT_EQ(ret, SOFTBUS_OK);
    TransDelItemByPackageName(g_pkgName);
    bool res = TransSessionServerIsExist(g_sessionName);
    EXPECT_FALSE(res);
    TransSessionMgrDeinit();
}

/**
 * @tc.name: TransSessionManagerTest11
 * @tc.desc: Transmission session manager judge whether session exists.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransSessionManagerTest, TransSessionManagerTest11, TestSize.Level1)
{
    int32_t ret = TransSessionMgrInit();
    EXPECT_EQ(ret,  SOFTBUS_OK);
    SessionServer *sessionServer = (SessionServer*)SoftBusMalloc(sizeof(SessionServer));
    EXPECT_TRUE(sessionServer != NULL);
    memset_s(sessionServer, sizeof(SessionServer), 0, sizeof(SessionServer));
    ret = strcpy_s(sessionServer->sessionName, sizeof(sessionServer->sessionName), g_sessionName);
    EXPECT_EQ(ret, EOK);
    ret = strcpy_s(sessionServer->pkgName, sizeof(sessionServer->pkgName), g_pkgName);
    EXPECT_EQ(ret, EOK);
    sessionServer->pid = TRANS_TEST_INVALID_PID;
    ret = TransSessionServerAddItem(sessionServer);
    EXPECT_EQ(ret, SOFTBUS_OK);
    bool res = TransSessionServerIsExist(g_sessionName);
    EXPECT_TRUE(res);
    SessionServer *newSessionServer = (SessionServer*)SoftBusMalloc(sizeof(SessionServer));
    EXPECT_TRUE(newSessionServer != NULL);
    memset_s(newSessionServer, sizeof(SessionServer), 0, sizeof(SessionServer));
    ret = strcpy_s(newSessionServer->sessionName, sizeof(newSessionServer->sessionName), g_sessionName);
    EXPECT_EQ(ret, EOK);
    ret = strcpy_s(newSessionServer->pkgName, sizeof(newSessionServer->pkgName), g_pkgName);
    EXPECT_EQ(ret, EOK);
    newSessionServer->pid = TRANS_TEST_INVALID_PID;
    ret = TransSessionServerAddItem(newSessionServer);
    EXPECT_EQ(ret, SOFTBUS_SERVER_NAME_REPEATED);
    ret = TransSessionServerDelItem(g_sessionName);
    EXPECT_EQ(ret, SOFTBUS_OK);
    SoftBusFree(newSessionServer);
    TransSessionMgrDeinit();
}
/**
 * @tc.name: TransSessionManagerTest12
 * @tc.desc: Transmission session manager get pid and uid by session name.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransSessionManagerTest, TransSessionManagerTest12, TestSize.Level1)
{
    int32_t ret = TransSessionMgrInit();
    EXPECT_EQ(ret,  SOFTBUS_OK);
    SessionServer *sessionServer = (SessionServer*)SoftBusMalloc(sizeof(SessionServer));
    EXPECT_TRUE(sessionServer != NULL);
    memset_s(sessionServer, sizeof(SessionServer), 0, sizeof(SessionServer));
    ret = strcpy_s(sessionServer->sessionName, sizeof(sessionServer->sessionName), g_sessionName);
    EXPECT_EQ(ret, EOK);
    ret = strcpy_s(sessionServer->pkgName, sizeof(sessionServer->pkgName), g_pkgName);
    EXPECT_EQ(ret, EOK);
    sessionServer->pid = TRANS_TEST_INVALID_PID;
    sessionServer->uid = TRANS_TEST_INVALID_UID;
    ret = TransSessionServerAddItem(sessionServer);
    EXPECT_EQ(ret, SOFTBUS_OK);
    int32_t pid = 0;
    int32_t uid = 0;
    ret = TransGetUidAndPid(g_sessionName, &uid, &pid);
    EXPECT_EQ(uid,  TRANS_TEST_INVALID_UID);
    EXPECT_EQ(pid, TRANS_TEST_INVALID_PID);
    ret = TransSessionServerDelItem(g_sessionName);
    EXPECT_EQ(ret, SOFTBUS_OK);
    TransSessionMgrDeinit();
}

/**
 * @tc.name: TransSessionManagerTest13
 * @tc.desc: Transmission session manager onLink down with invalid parameters.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransSessionManagerTest, TransSessionManagerTest13, TestSize.Level1)
{
    int32_t ret = TransSessionMgrInit();
    EXPECT_EQ(ret,  SOFTBUS_OK);
    TransOnLinkDown(NULL, WIFI_P2P);
    TransSessionMgrDeinit();
    TransOnLinkDown(g_networkid, WIFI_P2P);
}
/**
 * @tc.name: TransSessionManagerTest14
 * @tc.desc: Transmission session manager onLink down with wrong parameters.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransSessionManagerTest, TransSessionManagerTest14, TestSize.Level1)
{
    int32_t ret = TransSessionMgrInit();
    EXPECT_EQ(ret,  SOFTBUS_OK);
    SessionServer *sessionServer = (SessionServer*)SoftBusMalloc(sizeof(SessionServer));
    EXPECT_TRUE(sessionServer != NULL);
    memset_s(sessionServer, sizeof(SessionServer), 0, sizeof(SessionServer));
    ret = strcpy_s(sessionServer->sessionName, sizeof(sessionServer->sessionName), g_sessionName);
    EXPECT_EQ(ret, EOK);
    ret = strcpy_s(sessionServer->pkgName, sizeof(sessionServer->pkgName), g_pkgName);
    EXPECT_EQ(ret, EOK);
    sessionServer->pid = TRANS_TEST_INVALID_PID;
    sessionServer->uid = TRANS_TEST_INVALID_UID;
    ret = TransSessionServerAddItem(sessionServer);
    EXPECT_EQ(ret,  SOFTBUS_OK);
    TransOnLinkDown(g_networkid, WIFI_P2P);
}

/**
 * @tc.name: TransSessionManagerTest15
 * @tc.desc: Transmission session manager add item to maxmun.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransSessionManagerTest, TransSessionManagerTest15, TestSize.Level1)
{
    int32_t ret = TransSessionMgrInit();
    EXPECT_EQ(ret,  SOFTBUS_OK);
    for (int i = 0; i < MAX_SESSION_SERVER_NUM - 1; ++i) {
        char sessionNme[SESSION_NAME_SIZE_MAX] = {0};
        char pkgName[PKG_NAME_SIZE_MAX] = {0};
        ret = sprintf_s(sessionNme, SESSION_NAME_SIZE_MAX, "%s%d", g_sessionName, i);
        EXPECT_GT(ret, 0);
        ret = sprintf_s(pkgName, PKG_NAME_SIZE_MAX, "%s%d", g_pkgName, i);
        EXPECT_GT(ret, 0);
        SessionServer *sessionServer = (SessionServer*)SoftBusMalloc(sizeof(SessionServer));
        EXPECT_TRUE(sessionServer != NULL);
        memset_s(sessionServer, sizeof(SessionServer), 0, sizeof(SessionServer));
        ret = strcpy_s(sessionServer->sessionName, sizeof(sessionServer->sessionName), sessionNme);
        EXPECT_EQ(ret, EOK);
        ret = strcpy_s(sessionServer->pkgName, sizeof(sessionServer->pkgName), pkgName);
        EXPECT_EQ(ret, EOK);
        ret = TransSessionServerAddItem(sessionServer);
        EXPECT_EQ(ret, SOFTBUS_OK);
    }

    SessionServer *sessionServer = (SessionServer*)SoftBusMalloc(sizeof(SessionServer));
    EXPECT_TRUE(sessionServer != NULL);
    memset_s(sessionServer, sizeof(SessionServer), 0, sizeof(SessionServer));
    ret = strcpy_s(sessionServer->sessionName, sizeof(sessionServer->sessionName), g_sessionName);
    EXPECT_EQ(ret, EOK);
    ret = strcpy_s(sessionServer->pkgName, sizeof(sessionServer->pkgName), g_pkgName);
    EXPECT_EQ(ret, EOK);
    ret = TransSessionServerAddItem(sessionServer);
    EXPECT_EQ(ret, SOFTBUS_INVALID_NUM);
    SoftBusFree(sessionServer);

    for (int i = 0; i < MAX_SESSION_SERVER_NUM - 1; ++i) {
        char sessionNme[SESSION_NAME_SIZE_MAX] = {0};
        ret = sprintf_s(sessionNme, SESSION_NAME_SIZE_MAX, "%s%d", g_sessionName, i);
        EXPECT_GT(ret, 0);
        ret = TransSessionServerDelItem(sessionNme);
        EXPECT_EQ(ret, SOFTBUS_OK);
    }
    TransSessionMgrDeinit();
}
}