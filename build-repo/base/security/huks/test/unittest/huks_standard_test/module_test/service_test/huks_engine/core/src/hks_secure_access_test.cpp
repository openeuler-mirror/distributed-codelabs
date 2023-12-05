/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "hks_secure_access_test.h"

#include <gtest/gtest.h>

#include "hks_keynode.h"
#include "hks_log.h"
#include "hks_mem.h"
#include "hks_param.h"
#include "hks_type_inner.h"

#include "hks_secure_access.h"

#include "base/security/huks/services/huks_standard/huks_engine/main/core/src/hks_secure_access.c"

using namespace testing::ext;
namespace Unittest::HksSecureAccessTest {
class HksSecureAccessTest : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};

void HksSecureAccessTest::SetUpTestCase(void)
{
}

void HksSecureAccessTest::TearDownTestCase(void)
{
}

void HksSecureAccessTest::SetUp()
{
}

void HksSecureAccessTest::TearDown()
{
}

static int32_t BuildParamSetWithParam(struct HksParamSet **paramSet, const struct HksParam *params, uint32_t paramCnt,
    bool isWithMandataryParams)
{
    int32_t ret = HksInitParamSet(paramSet);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("HksInitParamSet failed");
        return ret;
    }

    if (isWithMandataryParams) {
        struct HksParam processNameBlob = {
            .tag = HKS_TAG_PROCESS_NAME,
            .blob = {
                .size = strlen("0"),
                .data = (uint8_t *)"0"
            }
        };
        ret = HksAddParams(*paramSet, &processNameBlob, 1);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksAddParams failed");
            return ret;
        }
    }

    if (params != nullptr) {
        ret = HksAddParams(*paramSet, params, paramCnt);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksAddParams failed");
            return ret;
        }
    }

    return HksBuildParamSet(paramSet);
}

/**
 * @tc.name: HksSecureAccessTest.HksSecureAccessTest001
 * @tc.desc: tdd HksProcessIdentityVerify, expect HKS_SUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(HksSecureAccessTest, HksSecureAccessTest001, TestSize.Level0)
{
    HKS_LOG_I("enter HksSecureAccessTest001");

    struct HksParamSet *blobParamSet = nullptr;
    struct HksParam accessTokenIdBlob = { .tag = HKS_TAG_ACCESS_TOKEN_ID, .uint64Param = 0 };
    int32_t ret = BuildParamSetWithParam(&blobParamSet, &accessTokenIdBlob, 1, true);
    EXPECT_EQ(ret, HKS_SUCCESS);

    struct HksParamSet *runtimeParamSet = nullptr;
    struct HksParam accessTokenIdRuntime = { .tag = HKS_TAG_ACCESS_TOKEN_ID, .uint64Param = 0 };

    ret = BuildParamSetWithParam(&runtimeParamSet, &accessTokenIdRuntime, 1, true);
    EXPECT_EQ(ret, HKS_SUCCESS);

    ret = HksProcessIdentityVerify(blobParamSet, runtimeParamSet);
    EXPECT_EQ(ret, HKS_SUCCESS);
    HksFreeParamSet(&blobParamSet);
    HksFreeParamSet(&runtimeParamSet);
}

/**
 * @tc.name: HksSecureAccessTest.HksSecureAccessTest002
 * @tc.desc: tdd HksProcessIdentityVerify, expect HKS_ERROR_BAD_STATE
 * @tc.type: FUNC
 */
HWTEST_F(HksSecureAccessTest, HksSecureAccessTest002, TestSize.Level0)
{
    HKS_LOG_I("enter HksSecureAccessTest002");

    struct HksParamSet *blobParamSet = nullptr;
    struct HksParam accessTokenIdBlob = { .tag = HKS_TAG_ACCESS_TOKEN_ID, .uint64Param = 1 };
    int32_t ret = BuildParamSetWithParam(&blobParamSet, &accessTokenIdBlob, 1, true);
    EXPECT_EQ(ret, HKS_SUCCESS);

    struct HksParamSet *runtimeParamSet = nullptr;
    struct HksParam accessTokenIdRuntime = { .tag = HKS_TAG_ACCESS_TOKEN_ID, .uint64Param = 0 };
    ret = BuildParamSetWithParam(&runtimeParamSet, &accessTokenIdRuntime, 1, true);
    EXPECT_EQ(ret, HKS_SUCCESS);

    ret = HksProcessIdentityVerify(blobParamSet, runtimeParamSet);
    EXPECT_EQ(ret, HKS_ERROR_BAD_STATE);
    HksFreeParamSet(&blobParamSet);
    HksFreeParamSet(&runtimeParamSet);
}

/**
 * @tc.name: HksSecureAccessTest.HksSecureAccessTest003
 * @tc.desc: tdd HksProcessIdentityVerify, expect HKS_SUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(HksSecureAccessTest, HksSecureAccessTest003, TestSize.Level0)
{
    HKS_LOG_I("enter HksSecureAccessTest003");

    struct HksParamSet *blobParamSet = nullptr;
    int32_t ret = BuildParamSetWithParam(&blobParamSet, nullptr, 0, true);
    EXPECT_EQ(ret, HKS_SUCCESS);

    struct HksParamSet *runtimeParamSet = nullptr;
    struct HksParam accessTokenIdRuntime = { .tag = HKS_TAG_ACCESS_TOKEN_ID, .uint64Param = 0 };

    ret = BuildParamSetWithParam(&runtimeParamSet, &accessTokenIdRuntime, 1, true);
    EXPECT_EQ(ret, HKS_SUCCESS);

    ret = HksProcessIdentityVerify(blobParamSet, runtimeParamSet);
    EXPECT_EQ(ret, HKS_SUCCESS);
    HksFreeParamSet(&blobParamSet);
    HksFreeParamSet(&runtimeParamSet);
}

/**
 * @tc.name: HksSecureAccessTest.HksSecureAccessTest004
 * @tc.desc: tdd HksProcessIdentityVerify, expect HKS_ERROR_BAD_STATE
 * @tc.type: FUNC
 */
HWTEST_F(HksSecureAccessTest, HksSecureAccessTest004, TestSize.Level0)
{
    HKS_LOG_I("enter HksSecureAccessTest004");

    struct HksParamSet *blobParamSet = nullptr;
    struct HksParam accessTokenIdBlob = { .tag = HKS_TAG_ACCESS_TOKEN_ID, .uint64Param = 1 };
    int32_t ret = BuildParamSetWithParam(&blobParamSet, &accessTokenIdBlob, 1, true);
    EXPECT_EQ(ret, HKS_SUCCESS);

    struct HksParamSet *runtimeParamSet = nullptr;

    ret = BuildParamSetWithParam(&runtimeParamSet, nullptr, 0, true);
    EXPECT_EQ(ret, HKS_SUCCESS);

    ret = HksProcessIdentityVerify(blobParamSet, runtimeParamSet);
    EXPECT_EQ(ret, HKS_ERROR_BAD_STATE);
    HksFreeParamSet(&blobParamSet);
    HksFreeParamSet(&runtimeParamSet);
}

/**
 * @tc.name: HksSecureAccessTest.HksSecureAccessTest009
 * @tc.desc: tdd HksCheckCompareProcessName, expect HKS_ERROR_BAD_STATE
 * @tc.type: FUNC
 */
HWTEST_F(HksSecureAccessTest, HksSecureAccessTest009, TestSize.Level0)
{
    HKS_LOG_I("enter HksSecureAccessTest009");

    int32_t ret = HksCheckCompareProcessName(NULL, NULL);
    ASSERT_EQ(ret, HKS_ERROR_BAD_STATE);
}

/**
 * @tc.name: HksSecureAccessTest.HksSecureAccessTest010
 * @tc.desc: tdd HksCheckCompareProcessName, expect HKS_ERROR_BAD_STATE
 * @tc.type: FUNC
 */
HWTEST_F(HksSecureAccessTest, HksSecureAccessTest010, TestSize.Level0)
{
    HKS_LOG_I("enter HksSecureAccessTest010");
    struct HksParamSet *blobParamSet = nullptr;
    struct HksBlob processName = { .size = strlen("011"), .data = (uint8_t *)"011"};
    struct HksParam processNameBlob = { .tag = HKS_TAG_PROCESS_NAME, .blob = processName};
    int32_t ret = BuildParamSetWithParam(&blobParamSet, &processNameBlob, 1, false);
    ASSERT_EQ(ret, HKS_SUCCESS);

    ret = HksCheckCompareProcessName(blobParamSet, NULL);
    ASSERT_EQ(ret, HKS_ERROR_BAD_STATE);
    HksFreeParamSet(&blobParamSet);
}

/**
 * @tc.name: HksSecureAccessTest.HksSecureAccessTest011
 * @tc.desc: tdd HksCheckCompareProcessName, expect HKS_ERROR_BAD_STATE
 * @tc.type: FUNC
 */
HWTEST_F(HksSecureAccessTest, HksSecureAccessTest011, TestSize.Level0)
{
    HKS_LOG_I("enter HksSecureAccessTest011");
    struct HksParamSet *blobParamSet = nullptr;
    struct HksBlob processName = { .size = strlen("011"), .data = (uint8_t *)"011"};
    struct HksParam processNameBlob = { .tag = HKS_TAG_PROCESS_NAME, .blob = processName};
    int32_t ret = BuildParamSetWithParam(&blobParamSet, &processNameBlob, 1, false);
    ASSERT_EQ(ret, HKS_SUCCESS);

    struct HksParamSet *runtimeParamSet = nullptr;
    struct HksBlob processName2 = { .size = strlen("012"), .data = (uint8_t *)"012"};
    struct HksParam processNameRuntime = { .tag = HKS_TAG_PROCESS_NAME, .blob = processName2};
    ret = BuildParamSetWithParam(&runtimeParamSet, &processNameRuntime, 1, false);
    ASSERT_EQ(ret, HKS_SUCCESS);

    ret = HksCheckCompareProcessName(blobParamSet, runtimeParamSet);
    ASSERT_EQ(ret, HKS_ERROR_BAD_STATE);

    HksFreeParamSet(&blobParamSet);
    HksFreeParamSet(&runtimeParamSet);
}

/**
 * @tc.name: HksSecureAccessTest.HksSecureAccessTest012
 * @tc.desc: tdd HksCheckCompareProcessName, expect HKS_ERROR_BAD_STATE
 * @tc.type: FUNC
 */
HWTEST_F(HksSecureAccessTest, HksSecureAccessTest012, TestSize.Level0)
{
    HKS_LOG_I("enter HksSecureAccessTest012");
    struct HksParamSet *blobParamSet = nullptr;
    struct HksBlob processName = { .size = strlen("011"), .data = (uint8_t *)"011"};
    struct HksParam processNameBlob = { .tag = HKS_TAG_PROCESS_NAME, .blob = processName};
    int32_t ret = BuildParamSetWithParam(&blobParamSet, &processNameBlob, 1, false);
    ASSERT_EQ(ret, HKS_SUCCESS);

    struct HksParamSet *runtimeParamSet = nullptr;
    struct HksBlob processName2 = { .size = strlen("0121"), .data = (uint8_t *)"0121"};
    struct HksParam processNameRuntime = { .tag = HKS_TAG_PROCESS_NAME, .blob = processName2};
    ret = BuildParamSetWithParam(&runtimeParamSet, &processNameRuntime, 1, false);
    ASSERT_EQ(ret, HKS_SUCCESS);

    ret = HksCheckCompareProcessName(blobParamSet, runtimeParamSet);
    ASSERT_EQ(ret, HKS_ERROR_BAD_STATE);

    HksFreeParamSet(&blobParamSet);
    HksFreeParamSet(&runtimeParamSet);
}
}
