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

#include "hks_param_test.h"

#include <gtest/gtest.h>

#include "hks_param.h"

#include "hks_log.h"
#include "hks_mem.h"
#include "hks_type.h"

#include <cstring>

using namespace testing::ext;
namespace Unittest::HksFrameworkCommonParamTest {
class HksParamTest : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};

void HksParamTest::SetUpTestCase(void)
{
}

void HksParamTest::TearDownTestCase(void)
{
}

void HksParamTest::SetUp()
{
}

void HksParamTest::TearDown()
{
}

/**
 * @tc.name: HksParamTest.HksParamTest001
 * @tc.desc: tdd HksCheckParamMatch, expecting HKS_ERROR_INVALID_ARGUMENT
 * @tc.type: FUNC
 */
HWTEST_F(HksParamTest, HksParamTest001, TestSize.Level0)
{
    HKS_LOG_I("enter HksParamTest001");
    struct HksParam baseParam = { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_RSA };
    struct HksParam otherParam = { .tag = HKS_TAG_CHALLENGE_POS, .uint32Param = HKS_CHALLENGE_POS_0 };
    int32_t ret = HksCheckParamMatch(&baseParam, &otherParam);
    EXPECT_EQ(ret, HKS_ERROR_INVALID_ARGUMENT) << "HksCheckParamMatch failed, ret = " << ret;
}

/**
 * @tc.name: HksParamTest.HksParamTest002
 * @tc.desc: tdd HksCheckParamMatch, expecting HKS_ERROR_INVALID_ARGUMENT
 * @tc.type: FUNC
 */
HWTEST_F(HksParamTest, HksParamTest002, TestSize.Level0)
{
    HKS_LOG_I("enter HksParamTest002");
    struct HksParam baseParam = { .tag = HKS_TAG_KEY_ACCESS_TIME, .uint64Param = (uint64_t)1 };
    struct HksParam otherParam = { .tag = HKS_TAG_KEY_ACCESS_TIME, .uint64Param = (uint64_t)2 };
    int32_t ret = HksCheckParamMatch(&baseParam, &otherParam);
    EXPECT_EQ(ret, HKS_ERROR_INVALID_ARGUMENT) << "HksCheckParamMatch failed, ret = " << ret;
}

/**
 * @tc.name: HksParamTest.HksParamTest003
 * @tc.desc: tdd HksCheckParamMatch, expecting HKS_ERROR_INVALID_ARGUMENT
 * @tc.type: FUNC
 */
HWTEST_F(HksParamTest, HksParamTest003, TestSize.Level0)
{
    HKS_LOG_I("enter HksParamTest003");
    struct HksParam baseParam = { .tag = HKS_TAG_IS_APPEND_UPDATE_DATA, .boolParam = true };
    struct HksParam otherParam = { .tag = HKS_TAG_IS_APPEND_UPDATE_DATA, .boolParam = false };
    int32_t ret = HksCheckParamMatch(&baseParam, &otherParam);
    EXPECT_EQ(ret, HKS_ERROR_INVALID_ARGUMENT) << "HksCheckParamMatch failed, ret = " << ret;
}

/**
 * @tc.name: HksParamTest.HksParamTest004
 * @tc.desc: tdd HksCheckParamMatch, expecting HKS_ERROR_INVALID_ARGUMENT
 * @tc.type: FUNC
 */
HWTEST_F(HksParamTest, HksParamTest004, TestSize.Level0)
{
    HKS_LOG_I("enter HksParamTest004");
    const char *baseData = "1";
    struct HksBlob baseBlob = { strlen(baseData), (uint8_t *)baseData};
    struct HksParam baseParam = { .tag = HKS_TAG_USER_AUTH_SECURE_UID, .blob = baseBlob};
    const char *otherData = "21";
    struct HksBlob otherBlob = { strlen(otherData), (uint8_t *)otherData};
    struct HksParam otherParam = { .tag = HKS_TAG_USER_AUTH_SECURE_UID, .blob = otherBlob };
    int32_t ret = HksCheckParamMatch(&baseParam, &otherParam);
    EXPECT_EQ(ret, HKS_ERROR_INVALID_ARGUMENT) << "HksCheckParamMatch failed, ret = " << ret;
}

/**
 * @tc.name: HksParamTest.HksParamTest005
 * @tc.desc: tdd HksCheckParamMatch, expecting HKS_ERROR_INVALID_ARGUMENT
 * @tc.type: FUNC
 */
HWTEST_F(HksParamTest, HksParamTest005, TestSize.Level0)
{
    HKS_LOG_I("enter HksParamTest005");
    const char *baseData = "1";
    struct HksBlob baseBlob = { strlen(baseData), (uint8_t *)baseData};
    struct HksParam baseParam = { .tag = HKS_TAG_USER_AUTH_SECURE_UID, .blob = baseBlob};
    const char *otherData = "2";
    struct HksBlob otherBlob = { strlen(otherData), (uint8_t *)otherData};
    struct HksParam otherParam = { .tag = HKS_TAG_USER_AUTH_SECURE_UID, .blob = otherBlob };
    int32_t ret = HksCheckParamMatch(&baseParam, &otherParam);
    EXPECT_EQ(ret, HKS_ERROR_INVALID_ARGUMENT) << "HksCheckParamMatch failed, ret = " << ret;
}

/**
 * @tc.name: HksParamTest.HksParamTest006
 * @tc.desc: tdd HksCheckParamMatch, expecting HKS_SUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(HksParamTest, HksParamTest006, TestSize.Level0)
{
    HKS_LOG_I("enter HksParamTest006");
    const char *baseData = "1";
    struct HksBlob baseBlob = { strlen(baseData), (uint8_t *)baseData};
    struct HksParam baseParam = { .tag = HKS_TAG_USER_AUTH_SECURE_UID, .blob = baseBlob};
    int32_t ret = HksCheckParamMatch(&baseParam, &baseParam);
    EXPECT_EQ(ret, HKS_SUCCESS) << "HksCheckParamMatch failed, ret = " << ret;
}

/**
 * @tc.name: HksParamTest.HksParamTest007
 * @tc.desc: tdd HksAddParams, expecting HKS_ERROR_INVALID_ARGUMENT
 * @tc.type: FUNC
 */
HWTEST_F(HksParamTest, HksParamTest007, TestSize.Level0)
{
    HKS_LOG_I("enter HksParamTest007");
    const char *baseData = "1";
    struct HksBlob baseBlob = { .size = UINT32_MAX, .data = (uint8_t *)baseData};
    struct HksParam baseParam = { .tag = HKS_TAG_USER_AUTH_SECURE_UID, .blob = baseBlob};
    struct HksParamSet *paramSet = nullptr;
    int32_t ret = HksInitParamSet(&paramSet);
    ASSERT_EQ(ret, HKS_SUCCESS)<< "HksInitParamSet failed, ret = " << ret;
    ret = HksAddParams(paramSet, &baseParam, 1);
    ASSERT_EQ(ret, HKS_ERROR_INVALID_ARGUMENT)<< "HksAddParams failed, ret = " << ret;
    HksFreeParamSet(&paramSet);
}

/**
 * @tc.name: HksParamTest.HksParamTest008
 * @tc.desc: tdd HksGetParamSet, expecting HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HksParamTest, HksParamTest008, TestSize.Level0)
{
    HKS_LOG_I("enter HksParamTest008");
    int32_t ret = HksGetParamSet(nullptr, 0, nullptr);
    ASSERT_EQ(ret, HKS_ERROR_NULL_POINTER)<< "HksGetParamSet failed, ret = " << ret;
}

/**
 * @tc.name: HksParamTest.HksParamTest009
 * @tc.desc: tdd HksGetParam, expecting HKS_ERROR_INVALID_ARGUMENT
 * @tc.type: FUNC
 */
HWTEST_F(HksParamTest, HksParamTest009, TestSize.Level0)
{
    HKS_LOG_I("enter HksParamTest009");
    struct HksParamSet *paramSet = nullptr;
    int32_t ret = HksInitParamSet(&paramSet);
    ASSERT_EQ(ret, HKS_SUCCESS)<< "HksInitParamSet failed, ret = " << ret;
    ret = HksBuildParamSet(&paramSet);
    ASSERT_EQ(ret, HKS_SUCCESS)<< "HksBuildParamSet failed, ret = " << ret;
    paramSet->paramSetSize = 0;
    struct HksParam *param = nullptr;
    ret = HksGetParam(paramSet, HKS_TAG_ACCESS_TIME, &param);
    ASSERT_EQ(ret, HKS_ERROR_INVALID_ARGUMENT)<< "HksGetParam failed, ret = " << ret;
    HksFreeParamSet(&paramSet);
}

/**
 * @tc.name: HksParamTest.HksParamTest010
 * @tc.desc: tdd HksGetParamSet, expecting HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HksParamTest, HksParamTest010, TestSize.Level0)
{
    HKS_LOG_I("enter HksParamTest010");
    struct HksParamSet *paramSet = nullptr;
    int32_t ret = HksInitParamSet(&paramSet);
    ASSERT_EQ(ret, HKS_SUCCESS)<< "HksInitParamSet failed, ret = " << ret;
    ret = HksBuildParamSet(&paramSet);
    ASSERT_EQ(ret, HKS_SUCCESS)<< "HksBuildParamSet failed, ret = " << ret;
    ret = HksGetParamSet(paramSet, paramSet->paramSetSize, nullptr);
    ASSERT_EQ(ret, HKS_ERROR_NULL_POINTER)<< "HksGetParamSet failed, ret = " << ret;
}

/**
 * @tc.name: HksParamTest.HksParamTest011
 * @tc.desc: tdd HksFreeParamSet
 * @tc.type: FUNC
 */
HWTEST_F(HksParamTest, HksParamTest011, TestSize.Level0)
{
    HKS_LOG_I("enter HksParamTest011");
    HksFreeParamSet(nullptr);
}


/**
 * @tc.name: HksParamTest.HksParamTest012
 * @tc.desc: tdd HksBuildParamSet, expecting HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HksParamTest, HksParamTest012, TestSize.Level0)
{
    HKS_LOG_I("enter HksParamTest012");
    int32_t ret = HksBuildParamSet(nullptr);
    ASSERT_EQ(ret, HKS_ERROR_NULL_POINTER)<< "HksBuildParamSet failed, ret = " << ret;
}

/**
 * @tc.name: HksParamTest.HksParamTest013
 * @tc.desc: tdd HksBuildParamSet, expecting HKS_ERROR_INVALID_ARGUMENT
 * @tc.type: FUNC
 */
HWTEST_F(HksParamTest, HksParamTest013, TestSize.Level0)
{
    HKS_LOG_I("enter HksParamTest013");
    struct HksParamSet *paramSet = nullptr;
    int32_t ret = HksInitParamSet(&paramSet);
    ASSERT_EQ(ret, HKS_SUCCESS)<< "HksInitParamSet failed, ret = " << ret;
    paramSet->paramSetSize = 0;
    ret = HksBuildParamSet(&paramSet);
    ASSERT_EQ(ret, HKS_ERROR_INVALID_ARGUMENT)<< "HksBuildParamSet failed, ret = " << ret;
    HksFreeParamSet(&paramSet);
}

/**
 * @tc.name: HksParamTest.HksParamTest014
 * @tc.desc: tdd HksInitParamSet, expecting HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HksParamTest, HksParamTest014, TestSize.Level0)
{
    HKS_LOG_I("enter HksParamTest014");
    int32_t ret = HksInitParamSet(nullptr);
    ASSERT_EQ(ret, HKS_ERROR_NULL_POINTER)<< "HksInitParamSet failed, ret = " << ret;
}

/**
 * @tc.name: HksParamTest.HksParamTest015
 * @tc.desc: tdd HksCheckParamSet, expecting HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HksParamTest, HksParamTest015, TestSize.Level0)
{
    HKS_LOG_I("enter HksParamTest015");
    int32_t ret = HksCheckParamSet(nullptr, 0);
    ASSERT_EQ(ret, HKS_ERROR_NULL_POINTER)<< "HksCheckParamSet failed, ret = " << ret;
}

/**
 * @tc.name: HksParamTest.HksParamTest016
 * @tc.desc: tdd HksFreshParamSet, expecting HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HksParamTest, HksParamTest016, TestSize.Level0)
{
    HKS_LOG_I("enter HksParamTest016");
    int32_t ret = HksFreshParamSet(nullptr, true);
    ASSERT_EQ(ret, HKS_ERROR_NULL_POINTER)<< "HksFreshParamSet failed, ret = " << ret;
}

/**
 * @tc.name: HksParamTest.HksParamTest017
 * @tc.desc: tdd HksFreshParamSet, expecting HKS_ERROR_INVALID_ARGUMENT
 * @tc.type: FUNC
 */
HWTEST_F(HksParamTest, HksParamTest017, TestSize.Level0)
{
    HKS_LOG_I("enter HksParamTest017");
    struct HksParamSet *paramSet = nullptr;
    int32_t ret = HksInitParamSet(&paramSet);
    ASSERT_EQ(ret, HKS_SUCCESS)<< "HksInitParamSet failed, ret = " << ret;
    ret = HksBuildParamSet(&paramSet);
    ASSERT_EQ(ret, HKS_SUCCESS)<< "HksBuildParamSet failed, ret = " << ret;
    paramSet->paramSetSize = 0;
    ret = HksFreshParamSet(nullptr, true);
    ASSERT_EQ(ret, HKS_ERROR_NULL_POINTER)<< "HksFreshParamSet failed, ret = " << ret;
}

/**
 * @tc.name: HksParamTest.HksParamTest018
 * @tc.desc: tdd HksCheckIsTagAlreadyExist, expecting HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HksParamTest, HksParamTest018, TestSize.Level0)
{
    HKS_LOG_I("enter HksParamTest018");
    int32_t ret = HksCheckIsTagAlreadyExist(nullptr, 0, nullptr);
    ASSERT_EQ(ret, HKS_ERROR_NULL_POINTER)<< "HksCheckIsTagAlreadyExist failed, ret = " << ret;
}

/**
 * @tc.name: HksParamTest.HksParamTest019
 * @tc.desc: tdd HksCheckIsTagAlreadyExist, expecting HKS_ERROR_INVALID_ARGUMENT
 * @tc.type: FUNC
 */
HWTEST_F(HksParamTest, HksParamTest019, TestSize.Level0)
{
    HKS_LOG_I("enter HksParamTest019");
    struct HksParamSet *paramSet = nullptr;
    int32_t ret = HksInitParamSet(&paramSet);
    ASSERT_EQ(ret, HKS_SUCCESS)<< "HksInitParamSet failed, ret = " << ret;
    ret = HksBuildParamSet(&paramSet);
    ASSERT_EQ(ret, HKS_SUCCESS)<< "HksBuildParamSet failed, ret = " << ret;
    paramSet->paramSetSize = 0;
    struct HksParam param = { 0 };
    ret = HksCheckIsTagAlreadyExist(&param, 0, paramSet);
    ASSERT_EQ(ret, HKS_ERROR_INVALID_ARGUMENT)<< "HksCheckIsTagAlreadyExist failed, ret = " << ret;
}

/**
 * @tc.name: HksParamTest.HksParamTest020
 * @tc.desc: tdd HksCheckParamMatch, expecting HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HksParamTest, HksParamTest020, TestSize.Level0)
{
    HKS_LOG_I("enter HksParamTest020");
    int32_t ret = HksCheckParamMatch(nullptr, nullptr);
    ASSERT_EQ(ret, HKS_ERROR_NULL_POINTER)<< "HksCheckParamMatch failed, ret = " << ret;
}
}
