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

#include "hks_base_check_test.h"

#include <gtest/gtest.h>

#include "hks_base_check.h"

#include "hks_log.h"
#include "hks_mem.h"
#include "hks_cmd_id.h"
#include "hks_type.h"

#include <cstring>

using namespace testing::ext;
namespace Unittest::HksFrameworkCommonBaseCheckTest {
class HksBaseCheckTest : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};

void HksBaseCheckTest::SetUpTestCase(void)
{
}

void HksBaseCheckTest::TearDownTestCase(void)
{
}

void HksBaseCheckTest::SetUp()
{
}

void HksBaseCheckTest::TearDown()
{
}

/**
 * @tc.name: HksBaseCheckTest.HksBaseCheckTest001
 * @tc.desc: tdd HksCheckCipherMaterialParams, expecting HKS_ERROR_CHECK_GET_IV_FAIL
 * @tc.type: FUNC
 */
HWTEST_F(HksBaseCheckTest, HksBaseCheckTest001, TestSize.Level0)
{
    HKS_LOG_I("enter HksBaseCheckTest001");
    struct ParamsValues values;
    struct Params param = { true, HKS_MODE_CBC };
    values.mode = param;
    int32_t ret = HksCheckCipherMaterialParams(HKS_ALG_SM4, &values, nullptr);
    ASSERT_EQ(ret, HKS_ERROR_CHECK_GET_IV_FAIL) << "HksCheckCipherMaterialParams failed, ret = " << ret;
}

/**
 * @tc.name: HksBaseCheckTest.HksBaseCheckTest002
 * @tc.desc: tdd HksCheckCihperData, expecting HKS_ERROR_INVALID_MODE
 * @tc.type: FUNC
 */
HWTEST_F(HksBaseCheckTest, HksBaseCheckTest002, TestSize.Level0)
{
    HKS_LOG_I("enter HksBaseCheckTest002");
    struct ParamsValues values;
    struct Params param = { true, HKS_MODE_OFB };
    values.mode = param;
    int32_t ret = HksCheckCihperData(0, HKS_ALG_SM4, &values, nullptr, nullptr);
    ASSERT_EQ(ret, HKS_ERROR_INVALID_MODE) << "HksCheckCihperData failed, ret = " << ret;
}

/**
 * @tc.name: HksBaseCheckTest.HksBaseCheckTest003
 * @tc.desc: tdd HksCheckCihperData, expecting HKS_ERROR_INVALID_ALGORITHM
 * @tc.type: FUNC
 */
HWTEST_F(HksBaseCheckTest, HksBaseCheckTest003, TestSize.Level0)
{
    HKS_LOG_I("enter HksBaseCheckTest003");
    int32_t ret = HksCheckCihperData(0, HKS_ALG_PBKDF2, nullptr, nullptr, nullptr);
    ASSERT_EQ(ret, HKS_ERROR_INVALID_ALGORITHM) << "HksCheckCihperData failed, ret = " << ret;
}

/**
 * @tc.name: HksBaseCheckTest.HksBaseCheckTest004
 * @tc.desc: tdd HksCheckCipherMutableParams, expecting HKS_ERROR_INVALID_ARGUMENT
 * @tc.type: FUNC
 */
HWTEST_F(HksBaseCheckTest, HksBaseCheckTest004, TestSize.Level0)
{
    HKS_LOG_I("enter HksBaseCheckTest004");
    int32_t ret = HksCheckCipherMutableParams(0, HKS_ALG_SM4, nullptr);
    ASSERT_EQ(ret, HKS_ERROR_INVALID_ARGUMENT) << "HksCheckCipherMutableParams failed, ret = " << ret;
}

/**
 * @tc.name: HksBaseCheckTest.HksBaseCheckTest005
 * @tc.desc: tdd HksCheckCipherMutableParams, expecting HKS_ERROR_INVALID_ARGUMENT
 * @tc.type: FUNC
 */
HWTEST_F(HksBaseCheckTest, HksBaseCheckTest005, TestSize.Level0)
{
    HKS_LOG_I("enter HksBaseCheckTest005");
    struct ParamsValues values;
    struct Params purParam = { true, HKS_KEY_PURPOSE_ENCRYPT };
    values.purpose = purParam;
    struct Params modeParam = { true, HKS_MODE_OFB };
    values.mode = modeParam;
    struct Params paddingParam = { true, HKS_PADDING_NONE };
    values.padding = paddingParam;
    int32_t ret = HksCheckCipherMutableParams(HKS_CMD_ID_ENCRYPT, HKS_ALG_SM4, &values);
    ASSERT_EQ(ret, HKS_ERROR_INVALID_PADDING) << "HksCheckCipherMutableParams failed, ret = " << ret;
}

/**
 * @tc.name: HksBaseCheckTest.HksBaseCheckTest006
 * @tc.desc: tdd HksCheckSignature, expecting HKS_ERROR_INVALID_ARGUMENT
 * @tc.type: FUNC
 */
HWTEST_F(HksBaseCheckTest, HksBaseCheckTest006, TestSize.Level0)
{
    HKS_LOG_I("enter HksBaseCheckTest006");
    
    int32_t ret = HksCheckSignature(0, HKS_ALG_RSA, HKS_ECC_KEY_SIZE_256, nullptr);
    ASSERT_EQ(ret, HKS_ERROR_INVALID_ARGUMENT) << "HksCheckSignature failed, ret = " << ret;
}

/**
 * @tc.name: HksBaseCheckTest.HksBaseCheckTest007
 * @tc.desc: tdd HksCheckSignature, expecting HKS_ERROR_INVALID_ARGUMENT
 * @tc.type: FUNC
 */
HWTEST_F(HksBaseCheckTest, HksBaseCheckTest007, TestSize.Level0)
{
    HKS_LOG_I("enter HksBaseCheckTest007");
    
    int32_t ret = HksCheckSignature(0, HKS_ALG_ECC, HKS_RSA_KEY_SIZE_1024, nullptr);
    ASSERT_EQ(ret, HKS_ERROR_INVALID_ARGUMENT) << "HksCheckSignature failed, ret = " << ret;
}

/**
 * @tc.name: HksBaseCheckTest.HksBaseCheckTest008
 * @tc.desc: tdd HksCheckSignature, expecting HKS_ERROR_INVALID_ARGUMENT
 * @tc.type: FUNC
 */
HWTEST_F(HksBaseCheckTest, HksBaseCheckTest008, TestSize.Level0)
{
    HKS_LOG_I("enter HksBaseCheckTest008");
    int32_t ret = HksCheckSignature(0, HKS_ALG_SM2, HKS_RSA_KEY_SIZE_1024, nullptr);
    ASSERT_EQ(ret, HKS_ERROR_INVALID_ARGUMENT) << "HksCheckSignature failed, ret = " << ret;
}

/**
 * @tc.name: HksBaseCheckTest.HksBaseCheckTest009
 * @tc.desc: tdd CheckImportMutableParams, expecting HKS_ERROR_INVALID_PURPOSE
 * @tc.type: FUNC
 */
HWTEST_F(HksBaseCheckTest, HksBaseCheckTest009, TestSize.Level0)
{
    HKS_LOG_I("enter HksBaseCheckTest009");
    struct ParamsValues values;
    struct Params param = { true, HKS_KEY_PURPOSE_ENCRYPT };
    values.purpose = param;
    int32_t ret = CheckImportMutableParams(HKS_ALG_SM2, &values);
    ASSERT_EQ(ret, HKS_ERROR_INVALID_PURPOSE) << "CheckImportMutableParams failed, ret = " << ret;
}

/**
 * @tc.name: HksBaseCheckTest.HksBaseCheckTest010
 * @tc.desc: tdd CheckImportMutableParams, expecting HKS_ERROR_INVALID_PURPOSE
 * @tc.type: FUNC
 */
HWTEST_F(HksBaseCheckTest, HksBaseCheckTest010, TestSize.Level0)
{
    HKS_LOG_I("enter HksBaseCheckTest010");
    struct ParamsValues values;
    struct Params param = { true, HKS_KEY_PURPOSE_ENCRYPT };
    values.purpose = param;
    int32_t ret = CheckImportMutableParams(HKS_ALG_ECC, &values);
    ASSERT_EQ(ret, HKS_ERROR_INVALID_PURPOSE) << "CheckImportMutableParams failed, ret = " << ret;
}

/**
 * @tc.name: HksBaseCheckTest.HksBaseCheckTest011
 * @tc.desc: tdd CheckImportMutableParams, expecting HKS_ERROR_INVALID_PURPOSE
 * @tc.type: FUNC
 */
HWTEST_F(HksBaseCheckTest, HksBaseCheckTest011, TestSize.Level0)
{
    HKS_LOG_I("enter HksBaseCheckTest011");
    struct ParamsValues values;
    struct Params param = { true, HKS_KEY_PURPOSE_DERIVE };
    values.purpose = param;
    int32_t ret = CheckImportMutableParams(HKS_ALG_RSA, &values);
    ASSERT_EQ(ret, HKS_ERROR_INVALID_PURPOSE) << "CheckImportMutableParams failed, ret = " << ret;
}

/**
 * @tc.name: HksBaseCheckTest.HksBaseCheckTest012
 * @tc.desc: tdd HksCheckGenKeyMutableParams, expecting HKS_ERROR_INVALID_PADDING
 * @tc.type: FUNC
 */
HWTEST_F(HksBaseCheckTest, HksBaseCheckTest012, TestSize.Level0)
{
    HKS_LOG_I("enter HksBaseCheckTest012");
    struct ParamsValues values;
    struct Params purParam = { true, HKS_KEY_PURPOSE_ENCRYPT };
    values.purpose = purParam;
    struct Params paddingParam = { true, HKS_PADDING_PSS };
    values.padding = paddingParam;
    int32_t ret = HksCheckGenKeyMutableParams(HKS_ALG_RSA, &values);
    ASSERT_EQ(ret, HKS_ERROR_INVALID_PADDING) << "CheckImportMutableParams failed, ret = " << ret;
}

/**
 * @tc.name: HksBaseCheckTest.HksBaseCheckTest013
 * @tc.desc: tdd HksCheckGenKeyMutableParams, expecting HKS_ERROR_INVALID_PADDING
 * @tc.type: FUNC
 */
HWTEST_F(HksBaseCheckTest, HksBaseCheckTest013, TestSize.Level0)
{
    HKS_LOG_I("enter HksBaseCheckTest013");
    struct ParamsValues values;
    struct Params modeParam = { true, HKS_MODE_CBC };
    values.mode = modeParam;
    struct Params purParam = { true, HKS_KEY_PURPOSE_ENCRYPT };
    values.purpose = purParam;
    struct Params paddingParam = { true, HKS_PADDING_PSS };
    values.padding = paddingParam;
    int32_t ret = HksCheckGenKeyMutableParams(HKS_ALG_AES, &values);
    ASSERT_EQ(ret, HKS_ERROR_INVALID_PADDING) << "CheckImportMutableParams failed, ret = " << ret;
}

/**
 * @tc.name: HksBaseCheckTest.HksBaseCheckTest014
 * @tc.desc: tdd HksGetKeySize, expecting HKS_ERROR_INVALID_KEY_FILE
 * @tc.type: FUNC
 */
HWTEST_F(HksBaseCheckTest, HksBaseCheckTest014, TestSize.Level0)
{
    HKS_LOG_I("enter HksBaseCheckTest014");
    struct HksBlob key = { .size = 0, .data = nullptr};
    uint32_t keySize = 0;
    int32_t ret = HksGetKeySize(HKS_ALG_RSA, &key, &keySize);
    ASSERT_EQ(ret, HKS_ERROR_INVALID_KEY_FILE) << "HksGetKeySize failed, ret = " << ret;
}

/**
 * @tc.name: HksBaseCheckTest.HksBaseCheckTest015
 * @tc.desc: tdd HksGetKeySize, expecting HKS_ERROR_INVALID_KEY_FILE
 * @tc.type: FUNC
 */
HWTEST_F(HksBaseCheckTest, HksBaseCheckTest015, TestSize.Level0)
{
    HKS_LOG_I("enter HksBaseCheckTest015");
    struct HksBlob key = { .size = sizeof(struct HksParamSet), .data = nullptr};
    uint32_t keySize = 0;
    int32_t ret = HksGetKeySize(HKS_ALG_RSA, &key, &keySize);
    ASSERT_EQ(ret, HKS_ERROR_INVALID_KEY_FILE) << "HksGetKeySize failed, ret = " << ret;
}

/**
 * @tc.name: HksBaseCheckTest.HksBaseCheckTest016
 * @tc.desc: tdd HksCheckCipherMaterialParams, expecting HKS_ERROR_INVALID_AAD
 * @tc.type: FUNC
 */
HWTEST_F(HksBaseCheckTest, HksBaseCheckTest016, TestSize.Level0)
{
    HKS_LOG_I("enter HksBaseCheckTest016");
    struct ParamsValues values;
    struct Params modeParam = { true, HKS_MODE_CCM };
    values.mode = modeParam;
    struct HksParamSet *paramSet = nullptr;
    int32_t ret = HksInitParamSet(&paramSet);
    ASSERT_EQ(ret, HKS_SUCCESS);
    const char* aadData = "0";
    struct HksBlob aad = { .size = strlen(aadData), .data = (uint8_t *)aadData };
    struct HksParam aadParam = { .tag = HKS_TAG_ASSOCIATED_DATA, .blob = aad };
    ret = HksAddParams(paramSet, &aadParam, 1);
    ASSERT_EQ(ret, HKS_SUCCESS);
    ret = HksBuildParamSet(&paramSet);
    ASSERT_EQ(ret, HKS_SUCCESS);
    ret = HksCheckCipherMaterialParams(HKS_ALG_AES, &values, paramSet);
    ASSERT_EQ(ret, HKS_ERROR_INVALID_AAD) << "HksCheckCipherMaterialParams failed, ret = " << ret;
    HksFreeParamSet(&paramSet);
}

/**
 * @tc.name: HksBaseCheckTest.HksBaseCheckTest017
 * @tc.desc: tdd HksCheckCipherMaterialParams, expecting HKS_ERROR_CHECK_GET_IV_FAIL
 * @tc.type: FUNC
 */
HWTEST_F(HksBaseCheckTest, HksBaseCheckTest017, TestSize.Level0)
{
    HKS_LOG_I("enter HksBaseCheckTest017");
    struct ParamsValues values;
    struct Params modeParam = { true, HKS_MODE_CCM };
    values.mode = modeParam;
    struct HksParamSet *paramSet = nullptr;
    int32_t ret = HksInitParamSet(&paramSet);
    ASSERT_EQ(ret, HKS_SUCCESS);
    const char* aadData = "00000";
    struct HksBlob aad = { .size = strlen(aadData), .data = (uint8_t *)aadData };
    struct HksParam aadParam = { .tag = HKS_TAG_ASSOCIATED_DATA, .blob = aad };
    ret = HksAddParams(paramSet, &aadParam, 1);
    ASSERT_EQ(ret, HKS_SUCCESS);
    const char* nonceData = "000";
    struct HksBlob nonce = { .size = strlen(aadData), .data = (uint8_t *)nonceData };
    struct HksParam param = { .tag = HKS_TAG_NONCE, .blob = nonce };
    ret = HksAddParams(paramSet, &param, 1);
    ASSERT_EQ(ret, HKS_SUCCESS);
    ret = HksBuildParamSet(&paramSet);
    ASSERT_EQ(ret, HKS_SUCCESS);
    ret = HksCheckCipherMaterialParams(HKS_ALG_AES, &values, paramSet);
    ASSERT_EQ(ret, HKS_ERROR_INVALID_NONCE) << "HksCheckCipherMaterialParams failed, ret = " << ret;
    HksFreeParamSet(&paramSet);
}

/**
 * @tc.name: HksBaseCheckTest.HksBaseCheckTest018
 * @tc.desc: tdd HksGetKeySize, expecting HKS_ERROR_INVALID_KEY_FILE
 * @tc.type: FUNC
 */
HWTEST_F(HksBaseCheckTest, HksBaseCheckTest018, TestSize.Level0)
{
    HKS_LOG_I("enter HksBaseCheckTest018");
    struct HksParamSet *paramSet = nullptr;
    int32_t ret = HksInitParamSet(&paramSet);
    ASSERT_EQ(ret, HKS_SUCCESS);
    struct HksParam param = { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_ECC_KEY_SIZE_224 };
    ret = HksAddParams(paramSet, &param, 1);
    ASSERT_EQ(ret, HKS_SUCCESS);
    ret = HksBuildParamSet(&paramSet);
    ASSERT_EQ(ret, HKS_SUCCESS);
    struct HksBlob key = { .size = sizeof(paramSet), .data = (uint8_t *)paramSet};
    uint32_t keySize = 0;
    ret = HksGetKeySize(HKS_ALG_RSA, &key, &keySize);
    ASSERT_EQ(ret, HKS_ERROR_INVALID_KEY_FILE) << "HksGetKeySize failed, ret = " << ret;
    HksFreeParamSet(&paramSet);
}
}
