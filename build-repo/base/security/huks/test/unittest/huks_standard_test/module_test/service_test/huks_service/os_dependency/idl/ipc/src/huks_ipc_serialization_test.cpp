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

#include "huks_ipc_serialization_test.h"

#include <gtest/gtest.h>
#include <cstring>

#include "hks_log.h"
#include "hks_ipc_serialization.h"
#include "hks_mem.h"
#include "hks_param.h"
#include "hks_type_inner.h"

using namespace testing::ext;
namespace Unittest::HksIpcSerializationTest {
class HksIpcSerializationTest : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};

void HksIpcSerializationTest::SetUpTestCase(void)
{
}

void HksIpcSerializationTest::TearDownTestCase(void)
{
}

void HksIpcSerializationTest::SetUp()
{
}

void HksIpcSerializationTest::TearDown()
{
}

/**
 * @tc.name: HksIpcSerializationTest.HksIpcSerializationTest001
 * @tc.desc: tdd HksParamSetToParams, expect HKS_SUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(HksIpcSerializationTest, HksIpcSerializationTest001, TestSize.Level0)
{
    HKS_LOG_I("enter HksIpcSerializationTest001");
    struct HksParamSet *paramSet = nullptr;
    int32_t ret = HksInitParamSet(&paramSet);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    const char *alias = "alias";
    struct HksBlob aliasBlob = { .size = strlen(alias), .data = (uint8_t *)alias };
    struct HksParam aliasParam = { .tag = HKS_TAG_ATTESTATION_ID_ALIAS, .blob = aliasBlob };
    ret = HksAddParams(paramSet, &aliasParam, 1);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    ret = HksBuildParamSet(&paramSet);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    struct HksParamOut aliasOutParam = { .tag = HKS_TAG_ATTESTATION_ID_ALIAS, .blob = &aliasBlob };
    ret = HksParamSetToParams(paramSet, &aliasOutParam, 1);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    HksFreeParamSet(&paramSet);
}

/**
 * @tc.name: HksIpcSerializationTest.HksIpcSerializationTest002
 * @tc.desc: tdd HksParamSetToParams, expect HKS_ERROR_PARAM_NOT_EXIST
 * @tc.type: FUNC
 */
HWTEST_F(HksIpcSerializationTest, HksIpcSerializationTest002, TestSize.Level0)
{
    HKS_LOG_I("enter HksIpcSerializationTest002");
    struct HksParamSet *paramSet = nullptr;
    int32_t ret = HksInitParamSet(&paramSet);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    const char *alias = "alias";
    struct HksBlob aliasBlob = { .size = strlen(alias), .data = (uint8_t *)alias };
    struct HksParamOut aliasOutParam = { .tag = HKS_TAG_ATTESTATION_ID_ALIAS, .blob = &aliasBlob };
    ret = HksBuildParamSet(&paramSet);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    ret = HksParamSetToParams(paramSet, &aliasOutParam, 1);
    ASSERT_TRUE(ret == HKS_ERROR_PARAM_NOT_EXIST);
    HksFreeParamSet(&paramSet);
}

/**
 * @tc.name: HksIpcSerializationTest.HksIpcSerializationTest003
 * @tc.desc: tdd HksParamSetToParams, expect HKS_SUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(HksIpcSerializationTest, HksIpcSerializationTest003, TestSize.Level0)
{
    HKS_LOG_I("enter HksIpcSerializationTest003");
    struct HksParamSet *paramSet = nullptr;
    int32_t ret = HksInitParamSet(&paramSet);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    const char *alias = "alias";
    struct HksBlob aliasBlob = { .size = strlen(alias), .data = (uint8_t *)alias };
    struct HksParamOut aliasOutParam = { .tag = HKS_TAG_ATTESTATION_ID_ALIAS, .blob = &aliasBlob };
    struct HksParam aliasNullParam = { .tag = HKS_TAG_ATTESTATION_ID_ALIAS + HKS_PARAM_BUFFER_NULL_INTERVAL,
        .blob = aliasBlob };
    ret = HksAddParams(paramSet, &aliasNullParam, 1);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    ret = HksBuildParamSet(&paramSet);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    ret = HksParamSetToParams(paramSet, &aliasOutParam, 1);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    ASSERT_TRUE(aliasOutParam.blob->data == nullptr && aliasOutParam.blob->size == 0);
    HksFreeParamSet(&paramSet);
}

/**
 * @tc.name: HksIpcSerializationTest.HksIpcSerializationTest004
 * @tc.desc: tdd HksParamSetToParams, expect HKS_SUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(HksIpcSerializationTest, HksIpcSerializationTest004, TestSize.Level0)
{
    HKS_LOG_I("enter HksIpcSerializationTest004");
    struct HksParamSet *paramSet = nullptr;
    int32_t ret = HksInitParamSet(&paramSet);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    struct HksParam param = { .tag = HKS_TAG_KEY_AUTH_RESULT, .int32Param = 0 };
    int32_t outParamInt = 1;
    struct HksParamOut outParam = { .tag = HKS_TAG_KEY_AUTH_RESULT, .int32Param = &outParamInt };
    ret = HksAddParams(paramSet, &param, 1);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    ret = HksBuildParamSet(&paramSet);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    ret = HksParamSetToParams(paramSet, &outParam, 1);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    ASSERT_TRUE(*(outParam.int32Param) == 0);
    HksFreeParamSet(&paramSet);
}

/**
 * @tc.name: HksIpcSerializationTest.HksIpcSerializationTest005
 * @tc.desc: tdd HksParamSetToParams, expect HKS_SUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(HksIpcSerializationTest, HksIpcSerializationTest005, TestSize.Level0)
{
    HKS_LOG_I("enter HksIpcSerializationTest005");
    struct HksParamSet *paramSet = nullptr;
    int32_t ret = HksInitParamSet(&paramSet);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    struct HksParam param = { .tag = HKS_TAG_ACCESS_TIME, .uint32Param = 0 };
    uint32_t outParamUint = 1;
    struct HksParamOut outParam = { .tag = HKS_TAG_ACCESS_TIME, .uint32Param = &outParamUint };
    ret = HksAddParams(paramSet, &param, 1);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    ret = HksBuildParamSet(&paramSet);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    ret = HksParamSetToParams(paramSet, &outParam, 1);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    ASSERT_TRUE(*(outParam.uint32Param) == 0);
    HksFreeParamSet(&paramSet);
}

/**
 * @tc.name: HksIpcSerializationTest.HksIpcSerializationTest006
 * @tc.desc: tdd HksParamSetToParams, expect HKS_SUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(HksIpcSerializationTest, HksIpcSerializationTest006, TestSize.Level0)
{
    HKS_LOG_I("enter HksIpcSerializationTest006");
    struct HksParamSet *paramSet = nullptr;
    int32_t ret = HksInitParamSet(&paramSet);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    struct HksParam param = { .tag = HKS_TAG_IF_NEED_APPEND_AUTH_INFO, .boolParam = true };
    bool outParamBool = false;
    struct HksParamOut outParam = { .tag = HKS_TAG_IF_NEED_APPEND_AUTH_INFO, .boolParam = &outParamBool };
    ret = HksAddParams(paramSet, &param, 1);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    ret = HksBuildParamSet(&paramSet);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    ret = HksParamSetToParams(paramSet, &outParam, 1);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    ASSERT_TRUE(*(outParam.boolParam) == true);
    HksFreeParamSet(&paramSet);
}

/**
 * @tc.name: HksIpcSerializationTest.HksIpcSerializationTest007
 * @tc.desc: tdd HksParamSetToParams, expect HKS_SUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(HksIpcSerializationTest, HksIpcSerializationTest007, TestSize.Level0)
{
    HKS_LOG_I("enter HksIpcSerializationTest007");
    struct HksParamSet *paramSet = nullptr;
    int32_t ret = HksInitParamSet(&paramSet);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    struct HksParam param = { .tag = HKS_TAG_KEY_ACCESS_TIME, .uint64Param = 0 };
    uint64_t outParamUint = 1;
    struct HksParamOut outParam = { .tag = HKS_TAG_KEY_ACCESS_TIME, .uint64Param = &outParamUint };
    ret = HksAddParams(paramSet, &param, 1);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    ret = HksBuildParamSet(&paramSet);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    ret = HksParamSetToParams(paramSet, &outParam, 1);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    ASSERT_TRUE(*(outParam.uint64Param) == 0);
    HksFreeParamSet(&paramSet);
}

/**
 * @tc.name: HksIpcSerializationTest.HksIpcSerializationTest008
 * @tc.desc: tdd HksParamSetToParams, expect HKS_ERROR_INVALID_ARGUMENT
 * @tc.type: FUNC
 */
HWTEST_F(HksIpcSerializationTest, HksIpcSerializationTest008, TestSize.Level0)
{
    HKS_LOG_I("enter HksIpcSerializationTest008");
    struct HksParamSet *paramSet = nullptr;
    int32_t ret = HksInitParamSet(&paramSet);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    struct HksParam param = { .tag = HKS_TAG_KEY_ACCESS_TIME ^ HKS_TAG_TYPE_ULONG, .uint64Param = 0 };
    uint64_t outParamUint = 1;
    struct HksParamOut outParam = { .tag = HKS_TAG_KEY_ACCESS_TIME ^ HKS_TAG_TYPE_ULONG, .uint64Param = &outParamUint };
    ret = HksAddParams(paramSet, &param, 1);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    ret = HksBuildParamSet(&paramSet);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    ret = HksParamSetToParams(paramSet, &outParam, 1);
    ASSERT_TRUE(ret == HKS_ERROR_INVALID_ARGUMENT);
    HksFreeParamSet(&paramSet);
}

/**
 * @tc.name: HksIpcSerializationTest.HksIpcSerializationTest009
 * @tc.desc: tdd HksParamSetToParams, expect HKS_ERROR_PARAM_NOT_EXIST
 * @tc.type: FUNC
 */
HWTEST_F(HksIpcSerializationTest, HksIpcSerializationTest009, TestSize.Level0)
{
    HKS_LOG_I("enter HksIpcSerializationTest009");
    struct HksParamSet *paramSet = nullptr;
    int32_t ret = HksInitParamSet(&paramSet);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    const char *alias = "alias";
    struct HksBlob aliasBlob = { .size = strlen(alias), .data = (uint8_t *)alias };
    struct HksParamOut aliasOutParam = { .tag = HKS_TAG_ATTESTATION_ID_ALIAS, .blob = &aliasBlob };
    ret = HksBuildParamSet(&paramSet);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    ret = HksParamSetToParams(paramSet, &aliasOutParam, 1);
    ASSERT_TRUE(ret == HKS_ERROR_PARAM_NOT_EXIST);
    HksFreeParamSet(&paramSet);
}
}
