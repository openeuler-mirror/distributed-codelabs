/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iostream>
#include <mutex>
#include <thread>
#include <condition_variable>
#include "gtest/gtest.h"
#include "file_ex.h"
#include "securec.h"
#include "softbus_bus_center.h"
#include "dev_slinfo_adpt.h"
#include "DevSLMgrTest.h"
#include "DevslinfoListTest.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "accesstoken_kit.h"

using namespace testing::ext;
class DevSLMgrTest : public testing::Test {
protected:
    DevSLMgrTest();
    ~DevSLMgrTest();
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
private:
};

static const int32_t DEV_SEC_LEVEL_ERR = 100;

struct DeviceSecurityInfo {
    uint32_t magicNum {0};
    uint32_t result {0};
    uint32_t level {0};
};

extern "C" {
    extern void OnApiDeviceSecInfoCallback(const DeviceIdentify *identify, struct DeviceSecurityInfo *info);
}

static void NativeTokenGet()
{
    uint64_t tokenId;
    const char **perms = new const char *[1];
    perms[0] = "ohos.permission.DISTRIBUTED_DATASYNC";
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 1,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .aplStr = "system_basic",
    };

    infoInstance.processName = "DevSLMgrTest";
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
    delete[] perms;
}

DevSLMgrTest::DevSLMgrTest() {}
DevSLMgrTest::~DevSLMgrTest() {}
void DevSLMgrTest::SetUpTestCase()
{
    OHOS::SaveStringToFile("/sys/fs/selinux/enforce", "0");
    NativeTokenGet();
}
void DevSLMgrTest::TearDownTestCase()
{
    OHOS::SaveStringToFile("/sys/fs/selinux/enforce", "1");
}
void DevSLMgrTest::SetUp() {}
void DevSLMgrTest::TearDown() {}

static void DATASL_GetUdidByOpp(DEVSLQueryParams *queryParams)
{
    char udid[65] = "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
    (void)memcpy_s(queryParams->udid, MAX_UDID_LENGTH, udid, MAX_UDID_LENGTH);
    queryParams->udidLen = MAX_UDID_LENGTH;
}

static void DATASL_GetUdidByExcept(DEVSLQueryParams *queryParams)
{
    char udid[65] = "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
    (void)memcpy_s(queryParams->udid, MAX_UDID_LENGTH, udid, MAX_UDID_LENGTH);
    queryParams->udidLen = MAX_UDID_LENGTH + 1;
}

static int32_t GetLocalUdid(DEVSLQueryParams *queryParams)
{
    const char *pkgName = "ohos.dslm";
    NodeBasicInfo info;
    int32_t ret = GetLocalNodeDeviceInfo(pkgName, &info);
    if (GetNodeKeyInfo(pkgName, info.networkId, NODE_KEY_UDID, (uint8_t *)(queryParams->udid), UDID_BUF_LEN) != 0) {
        return ret;
    }
    queryParams->udidLen = MAX_UDID_LENGTH;
    return DEVSL_SUCCESS;
}

static HWTEST_F(DevSLMgrTest, TestOnstart, TestSize.Level1)
{
    int32_t ret;

    ret = DATASL_OnStart();
    EXPECT_EQ(DEVSL_SUCCESS, ret);
    DATASL_OnStop();
}

static HWTEST_F(DevSLMgrTest, TestGetHighestSecLevel001, TestSize.Level1)
{
    int32_t ret;
    uint32_t levelInfo = 0;

    ret = DATASL_OnStart();
    EXPECT_EQ(DEVSL_SUCCESS, ret);
    ret = DATASL_GetHighestSecLevel(nullptr, &levelInfo);
    EXPECT_EQ(DEVSL_ERR_BAD_PARAMETERS, ret);
    DATASL_OnStop();
}

static HWTEST_F(DevSLMgrTest, TestGetHighestSecLevel002, TestSize.Level1)
{
    int32_t ret;

    DEVSLQueryParams queryParams;
    (void)memset_s(&queryParams, sizeof(queryParams), 0, sizeof(queryParams));
    DATASL_GetUdidByOpp(&queryParams);

    ret = DATASL_OnStart();
    EXPECT_EQ(DEVSL_SUCCESS, ret);
    ret = DATASL_GetHighestSecLevel(&queryParams, nullptr);
    EXPECT_EQ(DEVSL_ERR_BAD_PARAMETERS, ret);
    DATASL_OnStop();
}

static HWTEST_F(DevSLMgrTest, TestGetHighestSecLevel003, TestSize.Level1)
{
    int32_t ret;
    uint32_t levelInfo = 0;

    DEVSLQueryParams queryParams;
    (void)memset_s(&queryParams, sizeof(queryParams), 0, sizeof(queryParams));
    DATASL_GetUdidByOpp(&queryParams);

    ret = DATASL_OnStart();
    EXPECT_EQ(DEVSL_SUCCESS, ret);
    ret = DATASL_GetHighestSecLevel(&queryParams, &levelInfo);
    EXPECT_EQ(DATA_SEC_LEVEL0, static_cast<int32_t>(levelInfo));
    EXPECT_EQ(ERR_NOEXIST_DEVICE, ret);
    DATASL_OnStop();
}

static HWTEST_F(DevSLMgrTest, TestGetHighestSecLevel004, TestSize.Level1)
{
    int32_t ret;
    uint32_t levelInfo = 0;

    DEVSLQueryParams queryParams;
    (void)memset_s(&queryParams, sizeof(queryParams), 0, sizeof(queryParams));
    ret = GetLocalUdid(&queryParams);
    EXPECT_EQ(DEVSL_SUCCESS, ret);

    ret = DATASL_OnStart();
    EXPECT_EQ(DEVSL_SUCCESS, ret);
    ret = DATASL_GetHighestSecLevel(&queryParams, &levelInfo);
    EXPECT_EQ(DEVSL_SUCCESS, ret);
    DATASL_OnStop();
}

static void tmpCallback000(DEVSLQueryParams *queryParams, int32_t result, uint32_t levelInfo)
{
    EXPECT_EQ(DEVSL_ERR_BAD_PARAMETERS, result);
}

static HWTEST_F(DevSLMgrTest, TestGetHighestSecLevelAsync001, TestSize.Level1)
{
    int32_t ret;

    ret = DATASL_OnStart();
    EXPECT_EQ(DEVSL_SUCCESS, ret);
    ret = DATASL_GetHighestSecLevelAsync(nullptr, &tmpCallback000);
    EXPECT_EQ(DEVSL_ERR_BAD_PARAMETERS, ret);
    DATASL_OnStop();
}

static HWTEST_F(DevSLMgrTest, TestGetHighestSecLevelAsync002, TestSize.Level1)
{
    int32_t ret;
    DEVSLQueryParams queryParams;
    (void)memset_s(&queryParams, sizeof(queryParams), 0, sizeof(queryParams));
    DATASL_GetUdidByOpp(&queryParams);

    ret = DATASL_OnStart();
    EXPECT_EQ(DEVSL_SUCCESS, ret);
    ret = DATASL_GetHighestSecLevelAsync(&queryParams, nullptr);
    EXPECT_EQ(DEVSL_ERR_BAD_PARAMETERS, ret);
    DATASL_OnStop();
}

static void tmpCallback(DEVSLQueryParams *queryParams, int32_t result, uint32_t levelInfo)
{
    EXPECT_EQ(ERR_NOEXIST_DEVICE, result);
    EXPECT_EQ(DATA_SEC_LEVEL0, static_cast<int32_t>(levelInfo));
}

static HWTEST_F(DevSLMgrTest, TestGetHighestSecLevelAsync003, TestSize.Level1)
{
    int32_t ret;
    DEVSLQueryParams queryParams;
    (void)memset_s(&queryParams, sizeof(queryParams), 0, sizeof(queryParams));
    DATASL_GetUdidByOpp(&queryParams);

    ret = DATASL_OnStart();
    EXPECT_EQ(DEVSL_SUCCESS, ret);
    ret = DATASL_GetHighestSecLevelAsync(&queryParams, &tmpCallback);
    EXPECT_EQ(ERR_NOEXIST_DEVICE, ret);
    DATASL_OnStop();
}

static int32_t g_cnt = 0;
static std::mutex g_mtx;
static std::condition_variable g_cv;

static void tmpCallbackLocal(DEVSLQueryParams *queryParams, int32_t result, uint32_t levelInfo)
{
    g_cnt++;
    EXPECT_EQ(DEVSL_SUCCESS, result);
}

static HWTEST_F(DevSLMgrTest, TestGetHighestSecLevelAsync004, TestSize.Level1)
{
    int32_t ret;
    DEVSLQueryParams queryParams;
    (void)memset_s(&queryParams, sizeof(queryParams), 0, sizeof(queryParams));
    ret = GetLocalUdid(&queryParams);
    EXPECT_EQ(DEVSL_SUCCESS, ret);

    ret = DATASL_OnStart();
    EXPECT_EQ(DEVSL_SUCCESS, ret);
    ret = DATASL_GetHighestSecLevelAsync(&queryParams, &tmpCallbackLocal);
    EXPECT_EQ(DEVSL_SUCCESS, ret);

    std::unique_lock<std::mutex> lck(g_mtx);
    g_cv.wait_for(lck, std::chrono::milliseconds(2000), []() { return (g_cnt == 1); });
    EXPECT_EQ(g_cnt, 1);

    DATASL_OnStop();
}

static HWTEST_F(DevSLMgrTest, TestGetHighestSecLevelExcept001, TestSize.Level1)
{
    OnApiDeviceSecInfoCallback(nullptr, nullptr);
    GetDeviceSecLevelByUdidAsync(nullptr, 0);
    int32_t ret;
    uint32_t levelInfo = 0;
    int32_t devLevel = 0;
    DEVSLQueryParams queryParams;
    (void)memset_s(&queryParams, sizeof(queryParams), 0, sizeof(queryParams));
    DATASL_GetUdidByExcept(&queryParams);
    ret = GetDeviceSecLevelByUdid(static_cast<const uint8_t *>(queryParams.udid), queryParams.udidLen, &devLevel);
    EXPECT_EQ(DEVSL_ERROR, ret);
    ret = DATASL_OnStart();
    EXPECT_EQ(DEVSL_SUCCESS, ret);
    ret = StartDevslEnv();
    EXPECT_EQ(DEVSL_SUCCESS, ret);

    ret = DATASL_GetHighestSecLevel(&queryParams, &levelInfo);
    EXPECT_EQ(DEVSL_ERR_BAD_PARAMETERS, ret);

    DeviceIdentify devId;
    (void)memset_s(&devId, sizeof(devId), 0, sizeof(devId));
    (void)memcpy_s(devId.identity, MAX_UDID_LENGTH, queryParams.udid, queryParams.udidLen);
    devId.length = queryParams.udidLen;
    DeviceSecurityInfo devInfo;

    OnApiDeviceSecInfoCallback(&devId, nullptr);
    OnApiDeviceSecInfoCallback(&devId, &devInfo);

    DATASL_OnStop();
}

static HWTEST_F(DevSLMgrTest, TestGetHighestSecLevelExcept002, TestSize.Level1)
{
    int32_t ret;
    int32_t devLevel = 0;
    ret = DATASL_OnStart();
    EXPECT_EQ(DEVSL_SUCCESS, ret);

    DEVSLQueryParams queryParamsLocal;
    (void)memset_s(&queryParamsLocal, sizeof(queryParamsLocal), 0, sizeof(queryParamsLocal));
    ret = GetLocalUdid(&queryParamsLocal);
    EXPECT_EQ(DEVSL_SUCCESS, ret);

    DeviceSecurityInfo devInfo;
    DeviceIdentify devIdLocal;
    (void)memset_s(&devIdLocal, sizeof(devIdLocal), 0, sizeof(devIdLocal));
    (void)memcpy_s(devIdLocal.identity, MAX_UDID_LENGTH, queryParamsLocal.udid, queryParamsLocal.udidLen);
    devIdLocal.length = queryParamsLocal.udidLen;
    OnApiDeviceSecInfoCallback(&devIdLocal, &devInfo);

    DEVSLQueryParams queryParams;
    (void)memset_s(&queryParams, sizeof(queryParams), 0, sizeof(queryParams));
    DATASL_GetUdidByExcept(&queryParams);
    DEVSLQueryParams queryParamsOpp;
    (void)memset_s(&queryParamsOpp, sizeof(queryParamsOpp), 0, sizeof(queryParamsOpp));
    DATASL_GetUdidByOpp(&queryParamsOpp);
    ret = GetDeviceSecLevelByUdid(static_cast<const uint8_t *>(queryParamsOpp.udid), queryParamsOpp.udidLen, &devLevel);
    EXPECT_EQ(ERR_NOEXIST_DEVICE, ret);

    DeviceIdentify devIdOpp;
    OnApiDeviceSecInfoCallback(&devIdOpp, &devInfo);

    ret = CompareUdid(&queryParamsLocal, &queryParams);
    EXPECT_EQ(DEVSL_ERROR, ret);

    ret = CompareUdid(&queryParamsLocal, &queryParamsOpp);
    EXPECT_EQ(DEVSL_ERROR, ret);

    ret = CompareUdid(&queryParamsLocal, &queryParamsLocal);

    uint32_t result = GetDataSecLevelByDevSecLevel(DEV_SEC_LEVEL_ERR);
    EXPECT_EQ(DATA_SEC_LEVEL0, result);

    EXPECT_EQ(DEVSL_SUCCESS, ret);
    DATASL_OnStop();
}

static struct DATASLListParams *g_tmpList = nullptr;

static void ListCallback(DEVSLQueryParams *queryParams, int32_t result, uint32_t levelInfo)
{
    EXPECT_EQ(DEVSL_SUCCESS, DEVSL_SUCCESS);
}

static HWTEST_F(DevSLMgrTest, TestGetHighestSecLevelExcept003, TestSize.Level1)
{
    int32_t ret = InitPthreadMutex();
    EXPECT_EQ(DEVSL_SUCCESS, ret);

    ClearList(g_tmpList);
    DEVSLQueryParams queryParamsOpp;
    (void)memset_s(&queryParamsOpp, sizeof(queryParamsOpp), 0, sizeof(queryParamsOpp));
    DATASL_GetUdidByOpp(&queryParamsOpp);

    DeviceIdentify devIdOpp;
    (void)memset_s(&devIdOpp, sizeof(devIdOpp), 0, sizeof(devIdOpp));
    (void)memcpy_s(devIdOpp.identity, MAX_UDID_LENGTH, queryParamsOpp.udid, queryParamsOpp.udidLen);
    devIdOpp.length = queryParamsOpp.udidLen;
    DeviceSecurityInfo devInfo;

    OnApiDeviceSecInfoCallback(&devIdOpp, &devInfo);

    g_tmpList = InitList();
    if (g_tmpList != nullptr) {
        EXPECT_EQ(DEVSL_SUCCESS, DEVSL_SUCCESS);
    } else {
        EXPECT_EQ(DEVSL_SUCCESS, DEVSL_ERROR);
    }

    struct DATASLCallbackParams *newListNode =
        (struct DATASLCallbackParams*)malloc(sizeof(struct DATASLCallbackParams));
    if (newListNode == nullptr) {
        EXPECT_EQ(DEVSL_SUCCESS, DEVSL_SUCCESS);
    }
    (void)memcpy_s(newListNode->queryParams.udid, MAX_UDID_LENGTH, queryParamsOpp.udid, queryParamsOpp.udidLen);
    newListNode->queryParams.udidLen = queryParamsOpp.udidLen;
    newListNode->callback = &ListCallback;

    PushListNode(g_tmpList, newListNode);
    RemoveListNode(g_tmpList, newListNode);
    ClearList(g_tmpList);
    g_tmpList = nullptr;
    DestroyPthreadMutex();
}