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

#include "hks_refresh_key_info_test.h"

#include <gtest/gtest.h>
#include <thread>
#include <unistd.h>

#include "hks_api.h"
#include "hks_log.h"
#include "hks_mem.h"
#include "hks_param.h"
#include "hks_type.h"

#include "securec.h"

using namespace testing::ext;
namespace Unittest::HksGetKeyInfoListTest {
class HksGetKeyInfoListTest : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};

void HksGetKeyInfoListTest::SetUpTestCase(void)
{
}

void HksGetKeyInfoListTest::TearDownTestCase(void)
{
}

void HksGetKeyInfoListTest::SetUp()
{
    EXPECT_EQ(HksInitialize(), 0);
}

void HksGetKeyInfoListTest::TearDown()
{
}

/**
 * @tc.name: HksGetKeyInfoListTest.HksGetKeyInfoListTest001
 * @tc.desc: tdd HksGetKeyInfoList, with NULL paramset, expecting HKS_ERROR_INVALID_ARGUMENT
 * @tc.type: FUNC
 */
HWTEST_F(HksGetKeyInfoListTest, HksGetKeyInfoListTest001, TestSize.Level0)
{
    HKS_LOG_I("enter HksGetKeyInfoListTest001");
    struct HksKeyInfo keyInfoList = { { 0 }, nullptr };
    uint32_t listCount = 1;
    int32_t ret = HksGetKeyInfoList(nullptr, &keyInfoList, &listCount);
    EXPECT_EQ(ret, HKS_ERROR_INVALID_ARGUMENT) << "HksGetKeyInfoListTest001 failed, ret = " << ret;
}

/**
 * @tc.name: HksGetKeyInfoListTest.HksGetKeyInfoListTest002
 * @tc.desc: tdd HksGetKeyInfoList, with NULL input, expecting HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HksGetKeyInfoListTest, HksGetKeyInfoListTest002, TestSize.Level0)
{
    HKS_LOG_I("enter HksGetKeyInfoListTest002");
    uint32_t listCount = 0;
    int32_t ret = HksGetKeyInfoList(nullptr, nullptr, &listCount);
    EXPECT_EQ(ret, HKS_ERROR_NULL_POINTER) << "HksGetKeyInfoListTest002 failed, ret = " << ret;
}

static const char *g_alias = "key_alias";

static int32_t BuildParamSetForGenerateKey(struct HksParamSet **outParamSet)
{
    struct HksParam tmpParams[] = {
        { .tag = HKS_TAG_KEY_STORAGE_FLAG, .uint32Param = HKS_STORAGE_PERSISTENT },
        { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_RSA },
        { .tag = HKS_TAG_KEY_GENERATE_TYPE, .uint32Param = HKS_KEY_GENERATE_TYPE_DEFAULT },
        { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_RSA_KEY_SIZE_2048 },
        { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_VERIFY },
        { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA256 },
        { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_PSS },
        { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_ECB },
    };
    struct HksParamSet *paramSet = nullptr;
    int32_t ret = HksInitParamSet(&paramSet);
    if (ret != HKS_SUCCESS) {
        return ret;
    }

    ret = HksAddParams(paramSet, tmpParams, sizeof(tmpParams) / sizeof(tmpParams[0]));
    if (ret != HKS_SUCCESS) {
        HksFreeParamSet(&paramSet);
        return ret;
    }

    ret = HksBuildParamSet(&paramSet);
    if (ret != HKS_SUCCESS) {
        HksFreeParamSet(&paramSet);
        return ret;
    }
    *outParamSet = paramSet;
    return HKS_SUCCESS;
}

static void FreeKeyInfoList(struct HksKeyInfo **keyInfoList, uint32_t listCount)
{
    for (uint32_t i = 0; i < listCount; ++i) {
        if ((*keyInfoList)[i].alias.data == nullptr) {
            break;
        }
        HKS_FREE_PTR((*keyInfoList)[i].alias.data);
        if ((*keyInfoList)[i].paramSet == nullptr) {
            break;
        }
        HksFreeParamSet(&((*keyInfoList)[i].paramSet));
    }
    HKS_FREE_PTR(*keyInfoList);
}

static int32_t BuildKeyInfoList(struct HksKeyInfo **outKeyInfoList, uint32_t listCount)
{
    struct HksKeyInfo *keyInfoList = (struct HksKeyInfo *)HksMalloc(sizeof(struct HksKeyInfo) * listCount);
    if (keyInfoList == nullptr) {
        return HKS_ERROR_MALLOC_FAIL;
    }
    (void)memset_s(keyInfoList, sizeof(struct HksKeyInfo) * listCount, 0, sizeof(struct HksKeyInfo) * listCount);
    int32_t ret = HKS_SUCCESS;
    for (uint32_t i = 0; i < listCount; ++i) {
        keyInfoList[i].alias.data = (uint8_t *)HksMalloc(HKS_MAX_KEY_ALIAS_LEN);
        if (keyInfoList[i].alias.data == nullptr) {
            FreeKeyInfoList(&keyInfoList, listCount);
            return HKS_ERROR_MALLOC_FAIL;
        }
        keyInfoList[i].alias.size = HKS_MAX_KEY_ALIAS_LEN;
        ret = HksInitParamSet(&(keyInfoList[i].paramSet));
        if (ret != HKS_SUCCESS) {
            FreeKeyInfoList(&keyInfoList, listCount);
            return ret;
        }
        keyInfoList[i].paramSet->paramSetSize = HKS_DEFAULT_PARAM_SET_SIZE;
    }
    *outKeyInfoList = keyInfoList;
    return ret;
}

static const uint32_t g_initKeyInfoListNum = 64;

/**
 * @tc.name: HksGetKeyInfoListTest.HksGetKeyInfoListTest003
 * @tc.desc: tdd HksGetKeyInfoList, with NULL input, expecting HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HksGetKeyInfoListTest, HksGetKeyInfoListTest003, TestSize.Level0)
{
    HKS_LOG_I("enter HksGetKeyInfoListTest003");
    struct HksBlob keyAlias = { sizeof(g_alias), (uint8_t *)g_alias };
    struct HksParamSet *paramSet = nullptr;
    int32_t ret = BuildParamSetForGenerateKey(&paramSet);
    EXPECT_EQ(ret, HKS_SUCCESS) << "BuildParamSetForGenerateKey failed, ret = " << ret;
    ret = HksGenerateKey(&keyAlias, paramSet, nullptr);
    EXPECT_EQ(ret, HKS_SUCCESS) << "HksGenerateKey failed, ret = " << ret;
    HksFreeParamSet(&paramSet);
    uint32_t listCount = g_initKeyInfoListNum;
    struct HksKeyInfo *keyInfoList = nullptr;
    ret = BuildKeyInfoList(&keyInfoList, g_initKeyInfoListNum);
    ASSERT_EQ(ret, HKS_SUCCESS) << "BuildKeyInfoList failed, ret = " << ret;
    ret = HksGetKeyInfoList(nullptr, keyInfoList, &listCount);
    FreeKeyInfoList(&keyInfoList, g_initKeyInfoListNum);
    EXPECT_EQ(ret, HKS_SUCCESS) << "HksGetKeyInfoList failed, ret = " << ret;
    EXPECT_EQ(listCount >= 1, true) << "HksGetKeyInfoList failed, listCount = " << listCount;
}
}
