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

#include "hks_common_check_test.h"

#include <gtest/gtest.h>

#include "hks_common_check.h"

#include "hks_log.h"
#include "hks_mem.h"
#include "hks_param.h"
#include "hks_type.h"

#include <cstring>

using namespace testing::ext;
namespace Unittest::HksFrameworkCommonCheckTest {
class HksCommonCheckTest : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};

void HksCommonCheckTest::SetUpTestCase(void)
{
}

void HksCommonCheckTest::TearDownTestCase(void)
{
}

void HksCommonCheckTest::SetUp()
{
}

void HksCommonCheckTest::TearDown()
{
}

/**
 * @tc.name: HksCommonCheckTest.HksCommonCheckTest001
 * @tc.desc: tdd HksGetBlobFromWrappedData, expecting HKS_ERROR_INVALID_ARGUMENT
 * @tc.type: FUNC
 */
HWTEST_F(HksCommonCheckTest, HksCommonCheckTest001, TestSize.Level0)
{
    HKS_LOG_I("enter HksCommonCheckTest001");
    int32_t ret = HksGetBlobFromWrappedData(nullptr, 0, 0, nullptr);
    ASSERT_TRUE(ret == HKS_ERROR_INVALID_ARGUMENT);
}

/**
 * @tc.name: HksCommonCheckTest.HksCommonCheckTest002
 * @tc.desc: tdd HksCheckWrappedDataFormatValidity, expecting HKS_ERROR_INVALID_ARGUMENT
 * @tc.type: FUNC
 */
HWTEST_F(HksCommonCheckTest, HksCommonCheckTest002, TestSize.Level0)
{
    HKS_LOG_I("enter HksCommonCheckTest002");
    int32_t ret = HksCheckWrappedDataFormatValidity(nullptr, 0, nullptr);
    ASSERT_TRUE(ret == HKS_ERROR_INVALID_ARGUMENT);
}

/**
 * @tc.name: HksCommonCheckTest.HksCommonCheckTest003
 * @tc.desc: tdd HksCheckAesAeMode, expecting HKS_ERROR_CHECK_GET_ALG_FAIL
 * @tc.type: FUNC
 */
HWTEST_F(HksCommonCheckTest, HksCommonCheckTest003, TestSize.Level0)
{
    HKS_LOG_I("enter HksCommonCheckTest003");
    struct HksParamSet *paramSet = nullptr;
    int32_t ret = HksInitParamSet(&paramSet);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    ret = HksBuildParamSet(&paramSet);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    ret = HksCheckAesAeMode(paramSet, nullptr, nullptr);
    ASSERT_TRUE(ret == HKS_ERROR_CHECK_GET_ALG_FAIL);
    HksFreeParamSet(&paramSet);
}

/**
 * @tc.name: HksCommonCheckTest.HksCommonCheckTest004
 * @tc.desc: tdd HksCheckAesAeMode, expecting HKS_ERROR_CHECK_GET_MODE_FAIL
 * @tc.type: FUNC
 */
HWTEST_F(HksCommonCheckTest, HksCommonCheckTest004, TestSize.Level0)
{
    HKS_LOG_I("enter HksCommonCheckTest004");
    struct HksParamSet *paramSet = nullptr;
    int32_t ret = HksInitParamSet(&paramSet);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    struct HksParam param = { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_AES};
    ret = HksAddParams(paramSet, &param, 1);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    ret = HksBuildParamSet(&paramSet);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    bool isAes = true;
    ret = HksCheckAesAeMode(paramSet, &isAes, nullptr);
    ASSERT_TRUE(ret == HKS_ERROR_CHECK_GET_MODE_FAIL);
    HksFreeParamSet(&paramSet);
}

/**
 * @tc.name: HksCommonCheckTest.HksCommonCheckTest005
 * @tc.desc: tdd HksCheckParamSetValidity, expecting HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HksCommonCheckTest, HksCommonCheckTest005, TestSize.Level0)
{
    HKS_LOG_I("enter HksCommonCheckTest005");
    int32_t ret = HksCheckParamSetValidity(nullptr);
    ASSERT_TRUE(ret == HKS_ERROR_NULL_POINTER);
}

/**
 * @tc.name: HksCommonCheckTest.HksCommonCheckTest006
 * @tc.desc: tdd HksCheckParamSetValidity, expecting HKS_ERROR_INVALID_ARGUMENT
 * @tc.type: FUNC
 */
HWTEST_F(HksCommonCheckTest, HksCommonCheckTest006, TestSize.Level0)
{
    HKS_LOG_I("enter HksCommonCheckTest006");
    struct HksParamSet *paramSet = nullptr;
    int32_t ret = HksInitParamSet(&paramSet);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    ret = HksBuildParamSet(&paramSet);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    paramSet->paramSetSize = 0;
    ret = HksCheckParamSetValidity(paramSet);
    ASSERT_TRUE(ret == HKS_ERROR_INVALID_ARGUMENT);
    HksFreeParamSet(&paramSet);
}

/**
 * @tc.name: HksCommonCheckTest.HksCommonCheckTest007
 * @tc.desc: tdd HksCheckBlob2, expecting HKS_ERROR_INVALID_ARGUMENT
 * @tc.type: FUNC
 */
HWTEST_F(HksCommonCheckTest, HksCommonCheckTest007, TestSize.Level0)
{
    HKS_LOG_I("enter HksCommonCheckTest007");
    int32_t ret = HksCheckBlob2(nullptr, nullptr);
    ASSERT_TRUE(ret == HKS_ERROR_INVALID_ARGUMENT);
}

/**
 * @tc.name: HksCommonCheckTest.HksCommonCheckTest008
 * @tc.desc: tdd HksCheckBlob2, expecting HKS_ERROR_INVALID_ARGUMENT
 * @tc.type: FUNC
 */
HWTEST_F(HksCommonCheckTest, HksCommonCheckTest008, TestSize.Level0)
{
    HKS_LOG_I("enter HksCommonCheckTest008");
    const char *blobData = "test";
    struct HksBlob blob1 = { .size = strlen(blobData), .data = (uint8_t *)blobData };
    int32_t ret = HksCheckBlob2(&blob1, nullptr);
    ASSERT_TRUE(ret == HKS_ERROR_INVALID_ARGUMENT);
}

/**
 * @tc.name: HksCommonCheckTest.HksCommonCheckTest009
 * @tc.desc: tdd HksCheckBlob3, expecting HKS_ERROR_INVALID_ARGUMENT
 * @tc.type: FUNC
 */
HWTEST_F(HksCommonCheckTest, HksCommonCheckTest009, TestSize.Level0)
{
    HKS_LOG_I("enter HksCommonCheckTest009");
    int32_t ret = HksCheckBlob3(nullptr, nullptr, nullptr);
    ASSERT_TRUE(ret == HKS_ERROR_INVALID_ARGUMENT);
}

/**
 * @tc.name: HksCommonCheckTest.HksCommonCheckTest010
 * @tc.desc: tdd HksCheckBlob3, expecting HKS_ERROR_INVALID_ARGUMENT
 * @tc.type: FUNC
 */
HWTEST_F(HksCommonCheckTest, HksCommonCheckTest010, TestSize.Level0)
{
    HKS_LOG_I("enter HksCommonCheckTest010");
    const char *blobData = "test";
    struct HksBlob blob1 = { .size = strlen(blobData), .data = (uint8_t *)blobData };
    struct HksBlob blob2 = { .size = strlen(blobData), .data = (uint8_t *)blobData };
    int32_t ret = HksCheckBlob3(&blob1, &blob2, nullptr);
    ASSERT_TRUE(ret == HKS_ERROR_INVALID_ARGUMENT);
}

/**
 * @tc.name: HksCommonCheckTest.HksCommonCheckTest011
 * @tc.desc: tdd HksCheckBlob4, expecting HKS_ERROR_INVALID_ARGUMENT
 * @tc.type: FUNC
 */
HWTEST_F(HksCommonCheckTest, HksCommonCheckTest011, TestSize.Level0)
{
    HKS_LOG_I("enter HksCommonCheckTest011");
    int32_t ret = HksCheckBlob4(nullptr, nullptr, nullptr, nullptr);
    ASSERT_TRUE(ret == HKS_ERROR_INVALID_ARGUMENT);
}

/**
 * @tc.name: HksCommonCheckTest.HksCommonCheckTest012
 * @tc.desc: tdd HksCheckBlob4, expecting HKS_ERROR_INVALID_ARGUMENT
 * @tc.type: FUNC
 */
HWTEST_F(HksCommonCheckTest, HksCommonCheckTest012, TestSize.Level0)
{
    HKS_LOG_I("enter HksCommonCheckTest012");
    const char *blobData = "test";
    struct HksBlob blob1 = { .size = strlen(blobData), .data = (uint8_t *)blobData };
    int32_t ret = HksCheckBlob4(&blob1, nullptr, nullptr, nullptr);
    ASSERT_TRUE(ret == HKS_ERROR_INVALID_ARGUMENT);
}

/**
 * @tc.name: HksCommonCheckTest.HksCommonCheckTest013
 * @tc.desc: tdd HksCheckBlob4, expecting HKS_ERROR_INVALID_ARGUMENT
 * @tc.type: FUNC
 */
HWTEST_F(HksCommonCheckTest, HksCommonCheckTest013, TestSize.Level0)
{
    HKS_LOG_I("enter HksCommonCheckTest013");
    const char *blobData = "test";
    struct HksBlob blob1 = { .size = strlen(blobData), .data = (uint8_t *)blobData };
    struct HksBlob blob2 = { .size = strlen(blobData), .data = (uint8_t *)blobData };
    struct HksBlob blob3 = { .size = strlen(blobData), .data = (uint8_t *)blobData };
    int32_t ret = HksCheckBlob4(&blob1, &blob2, &blob3, nullptr);
    ASSERT_TRUE(ret == HKS_ERROR_INVALID_ARGUMENT);
}
}
