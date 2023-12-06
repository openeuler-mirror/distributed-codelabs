/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "hks_core_service_test.h"

#include <gtest/gtest.h>
#include <string>

#include "hks_log.h"
#include "hks_mem.h"
#include "hks_param.h"
#include "hks_type_inner.h"

#include "hks_client_service.h"
#include "hks_core_service.h"
#include "hks_storage.h"

using namespace testing::ext;
namespace Unittest::HksCoreServiceTest {
class HksCoreServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};

void HksCoreServiceTest::SetUpTestCase(void)
{
}

void HksCoreServiceTest::TearDownTestCase(void)
{
}

void HksCoreServiceTest::SetUp()
{
}

void HksCoreServiceTest::TearDown()
{
}

/**
 * @tc.name: HksCoreServiceTest.HksCoreServiceTest001
 * @tc.desc: tdd HksCoreAbort, expect ret == HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HksCoreServiceTest, HksCoreServiceTest001, TestSize.Level0)
{
    HKS_LOG_I("enter HksCoreServiceTest001");
    int32_t ret = HksCoreAbort(nullptr, nullptr);
    ASSERT_TRUE(ret == HKS_ERROR_NULL_POINTER);
}

/**
 * @tc.name: HksCoreServiceTest.HksCoreServiceTest002
 * @tc.desc: tdd HksCoreAbort, expect ret == HKS_ERROR_INSUFFICIENT_MEMORY
 * @tc.type: FUNC
 */
HWTEST_F(HksCoreServiceTest, HksCoreServiceTest002, TestSize.Level0)
{
    HKS_LOG_I("enter HksCoreServiceTest002");
    struct HksBlob handle = { .size = sizeof(uint64_t) * 2, .data = nullptr };
    struct HksParamSet *paramSet = nullptr;
    int32_t ret = HksInitParamSet(&paramSet);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    ret = HksBuildParamSet(&paramSet);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    ret = HksCoreAbort(&handle, paramSet);
    ASSERT_TRUE(ret == HKS_ERROR_INSUFFICIENT_MEMORY);
    HksFreeParamSet(&paramSet);
}

/**
 * @tc.name: HksCoreServiceTest.HksCoreServiceTest003
 * @tc.desc: tdd HksCoreFinish, expect ret == HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HksCoreServiceTest, HksCoreServiceTest003, TestSize.Level0)
{
    HKS_LOG_I("enter HksCoreServiceTest003");
    int32_t ret = HksCoreFinish(nullptr, nullptr, nullptr, nullptr);
    ASSERT_TRUE(ret == HKS_ERROR_NULL_POINTER);
}

/**
 * @tc.name: HksCoreServiceTest.HksCoreServiceTest004
 * @tc.desc: tdd HksCoreFinish, expect ret == HKS_ERROR_INSUFFICIENT_MEMORY
 * @tc.type: FUNC
 */
HWTEST_F(HksCoreServiceTest, HksCoreServiceTest004, TestSize.Level0)
{
    HKS_LOG_I("enter HksCoreServiceTest004");
    struct HksBlob handle = { .size = sizeof(uint64_t) * 2, .data = nullptr };
    struct HksParamSet *paramSet = nullptr;
    int32_t ret = HksInitParamSet(&paramSet);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    ret = HksBuildParamSet(&paramSet);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    struct HksBlob inData = { 0 };
    ret = HksCoreFinish(&handle, paramSet, &inData, nullptr);
    ASSERT_TRUE(ret == HKS_ERROR_INSUFFICIENT_MEMORY);
    HksFreeParamSet(&paramSet);
}

/**
 * @tc.name: HksCoreServiceTest.HksCoreServiceTest005
 * @tc.desc: tdd HksCoreFinish, expect ret == HKS_ERROR_BAD_STATE
 * @tc.type: FUNC
 */
HWTEST_F(HksCoreServiceTest, HksCoreServiceTest005, TestSize.Level0)
{
    HKS_LOG_I("enter HksCoreServiceTest005");
    uint64_t handleData = 0;
    struct HksBlob handle = { .size = sizeof(uint64_t), .data = (uint8_t *)&handleData };
    struct HksParamSet *paramSet = nullptr;
    int32_t ret = HksInitParamSet(&paramSet);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    ret = HksBuildParamSet(&paramSet);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    struct HksBlob inData = { 0 };
    ret = HksCoreFinish(&handle, paramSet, &inData, nullptr);
    ASSERT_TRUE(ret == HKS_ERROR_BAD_STATE);
    HksFreeParamSet(&paramSet);
}

/**
 * @tc.name: HksCoreServiceTest.HksCoreServiceTest006
 * @tc.desc: tdd HksCoreUpdate, expect ret == HKS_ERROR_INSUFFICIENT_MEMORY
 * @tc.type: FUNC
 */
HWTEST_F(HksCoreServiceTest, HksCoreServiceTest006, TestSize.Level0)
{
    HKS_LOG_I("enter HksCoreServiceTest006");
    int32_t ret = HksCoreUpdate(nullptr, nullptr, nullptr, nullptr);
    ASSERT_TRUE(ret == HKS_ERROR_NULL_POINTER);
}

/**
 * @tc.name: HksCoreServiceTest.HksCoreServiceTest007
 * @tc.desc: tdd HksCoreUpdate, expect ret == HKS_ERROR_INSUFFICIENT_MEMORY
 * @tc.type: FUNC
 */
HWTEST_F(HksCoreServiceTest, HksCoreServiceTest007, TestSize.Level0)
{
    HKS_LOG_I("enter HksCoreServiceTest007");
    uint64_t handleData = 0;
    struct HksBlob handle = { .size = sizeof(uint64_t) * 2, .data = (uint8_t *)&handleData };
    struct HksParamSet *paramSet = nullptr;
    int32_t ret = HksInitParamSet(&paramSet);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    ret = HksBuildParamSet(&paramSet);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    struct HksBlob inData = { 0 };
    ret = HksCoreUpdate(&handle, paramSet, &inData, nullptr);
    ASSERT_TRUE(ret == HKS_ERROR_INSUFFICIENT_MEMORY);
    HksFreeParamSet(&paramSet);
}

/**
 * @tc.name: HksCoreServiceTest.HksCoreServiceTest008
 * @tc.desc: tdd HksCoreInit, expect ret == HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HksCoreServiceTest, HksCoreServiceTest008, TestSize.Level0)
{
    HKS_LOG_I("enter HksCoreServiceTest008");
    int32_t ret = HksCoreInit(nullptr, nullptr, nullptr, nullptr);
    ASSERT_TRUE(ret == HKS_ERROR_NULL_POINTER);
}

/**
 * @tc.name: HksCoreServiceTest.HksCoreServiceTest009
 * @tc.desc: tdd HksCoreInit, expect ret == HKS_ERROR_INSUFFICIENT_MEMORY
 * @tc.type: FUNC
 */
HWTEST_F(HksCoreServiceTest, HksCoreServiceTest009, TestSize.Level0)
{
    HKS_LOG_I("enter HksCoreServiceTest009");
    uint64_t handleData = 0;
    struct HksBlob handle = { .size = sizeof(uint32_t), .data = (uint8_t *)&handleData };
    struct HksBlob token = { 0 };
    struct HksBlob key = { 0 };
    struct HksParamSet *paramSet = nullptr;
    int32_t ret = HksInitParamSet(&paramSet);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    ret = HksBuildParamSet(&paramSet);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    ret = HksCoreInit(&key, paramSet, &handle, &token);
    ASSERT_TRUE(ret == HKS_ERROR_INSUFFICIENT_MEMORY);
    HksFreeParamSet(&paramSet);
}

static int32_t TestGenerateKey(const struct HksBlob *keyAlias, const struct HksProcessInfo *processInfo)
{
    struct HksParam tmpParams[] = {
        { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_ECB },
        { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_RSA },
        { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_RSA_KEY_SIZE_2048 },
        { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA256 },
        { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_PSS },
        { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_VERIFY },
        { .tag = HKS_TAG_KEY_STORAGE_FLAG, .uint32Param = HKS_STORAGE_PERSISTENT },
        { .tag = HKS_TAG_KEY_GENERATE_TYPE, .uint32Param = HKS_KEY_GENERATE_TYPE_DEFAULT },
    };
    struct HksParamSet *paramSet = nullptr;
    int32_t ret = HksInitParamSet(&paramSet);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("hks_core_service_test HksInitParamSet failed");
        return ret;
    }

    ret = HksAddParams(paramSet, tmpParams, sizeof(tmpParams) / sizeof(tmpParams[0]));
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("hks_core_service_test HksAddParams failed");
        HksFreeParamSet(&paramSet);
        return ret;
    }

    ret = HksBuildParamSet(&paramSet);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("hks_core_service_test HksBuildParamSet failed");
        HksFreeParamSet(&paramSet);
        return ret;
    }

    ret = HksServiceGenerateKey(processInfo, keyAlias, paramSet, nullptr);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("hks_core_service_test HksGenerateKey failed");
    }
    HksFreeParamSet(&paramSet);
    return ret;
}

static int32_t BuildParamSetWithParam(struct HksParamSet **paramSet, struct HksParam *param)
{
    int32_t ret = HksInitParamSet(paramSet);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("BuildParamSetWithParam HksInitParamSet failed");
        return ret;
    }
    if (param != nullptr) {
        ret = HksAddParams(*paramSet, param, 1);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("BuildParamSetWithParam HksAddParams failed");
            return ret;
        }
    }
    return HksBuildParamSet(paramSet);
}

static const char *g_processNameString = "hks_client";
static const struct HksBlob g_processName = { strlen(g_processNameString), (uint8_t *)g_processNameString };
static const uint32_t USER_ID_INT = 0;
static const struct HksBlob g_userId = { sizeof(USER_ID_INT), (uint8_t *)(&USER_ID_INT)};
static const uint32_t KEY_BLOB_DEFAULT_SIZE = 4096;

/**
 * @tc.name: HksCoreServiceTest.HksCoreServiceTest001
 * @tc.desc: tdd HksCoreExportPublicKey with wrong access token id, expect HKS_ERROR_BAD_STATE
 * @tc.type: FUNC
 */
HWTEST_F(HksCoreServiceTest, HksCoreServiceTest010, TestSize.Level0)
{
    HKS_LOG_I("enter HksCoreServiceTest010");
    const char *alias = "HksCoreServiceTest010";
    const struct HksBlob keyAlias = { strlen(alias), (uint8_t *)alias };
    struct HksProcessInfo processInfo = { g_userId, g_processName, USER_ID_INT, 0 };
    int32_t ret = TestGenerateKey(&keyAlias, &processInfo);
    ASSERT_EQ(ret, HKS_SUCCESS);
    struct HksBlob keyBlob = { .size = KEY_BLOB_DEFAULT_SIZE, .data = (uint8_t *)HksMalloc(KEY_BLOB_DEFAULT_SIZE) };
    ASSERT_NE(keyBlob.data, nullptr);
    ret = HksStoreGetKeyBlob(&processInfo, &keyAlias, HKS_STORAGE_TYPE_KEY, &keyBlob);

    struct HksParamSet *runtimeParamSet = nullptr;
    struct HksParam accessTokenIdRuntime = { .tag = HKS_TAG_ACCESS_TOKEN_ID, .uint64Param = 1 };
    ret = BuildParamSetWithParam(&runtimeParamSet, &accessTokenIdRuntime);
    ASSERT_EQ(ret, HKS_SUCCESS);
    struct HksBlob keyOutBlob = { .size = KEY_BLOB_DEFAULT_SIZE, .data = (uint8_t *)HksMalloc(KEY_BLOB_DEFAULT_SIZE) };

    ret = HksCoreExportPublicKey(&keyBlob, runtimeParamSet, &keyOutBlob);
    ASSERT_EQ(ret, HKS_ERROR_BAD_STATE);

    (void)HksServiceDeleteKey(&processInfo, &keyAlias);
    HksFree(keyOutBlob.data);
    HksFree(keyBlob.data);
    HksFreeParamSet(&runtimeParamSet);
}

/**
 * @tc.name: HksCoreServiceTest.HksCoreServiceTest018
 * @tc.desc: tdd HksCoreExportPublicKey with wrongProcessName, expect HKS_ERROR_BAD_STATE
 * @tc.type: FUNC
 */
HWTEST_F(HksCoreServiceTest, HksCoreServiceTest018, TestSize.Level0)
{
    HKS_LOG_I("enter HksCoreServiceTest018");
    const char *alias = "HksCoreServiceTest018";
    const struct HksBlob keyAlias = { strlen(alias), (uint8_t *)alias };
    struct HksProcessInfo processInfo = { g_userId, g_processName, USER_ID_INT, 0 };
    int32_t ret = TestGenerateKey(&keyAlias, &processInfo);
    ASSERT_EQ(ret, HKS_SUCCESS);
    struct HksBlob keyBlob = { .size = KEY_BLOB_DEFAULT_SIZE, .data = (uint8_t *)HksMalloc(KEY_BLOB_DEFAULT_SIZE) };
    ASSERT_NE(keyBlob.data, nullptr);
    ret = HksStoreGetKeyBlob(&processInfo, &keyAlias, HKS_STORAGE_TYPE_KEY, &keyBlob);
    ASSERT_EQ(ret, HKS_SUCCESS) << "HksCoreServiceTest018 ret is " << ret;

    struct HksParamSet *runtimeParamSet = nullptr;
    struct HksBlob wrongProcessName = { .size = strlen("011"), .data = (uint8_t *)"011"};
    struct HksParam processNameRuntime = { .tag = HKS_TAG_PROCESS_NAME, .blob = wrongProcessName};
    ret = BuildParamSetWithParam(&runtimeParamSet, &processNameRuntime);
    ASSERT_EQ(ret, HKS_SUCCESS);
    struct HksBlob keyOutBlob = { .size = KEY_BLOB_DEFAULT_SIZE, .data = (uint8_t *)HksMalloc(KEY_BLOB_DEFAULT_SIZE) };

    ret = HksCoreExportPublicKey(&keyBlob, runtimeParamSet, &keyOutBlob);
    ASSERT_EQ(ret, HKS_ERROR_BAD_STATE);

    HksFree(keyBlob.data);
    (void)HksServiceDeleteKey(&processInfo, &keyAlias);
    HksFree(keyOutBlob.data);
    HksFreeParamSet(&runtimeParamSet);
}
}
