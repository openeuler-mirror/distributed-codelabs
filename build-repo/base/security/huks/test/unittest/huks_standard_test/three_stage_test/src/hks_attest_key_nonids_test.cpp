/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "hks_attest_key_nonids_test.h"

#include <gtest/gtest.h>

#include "hks_attest_key_test_common.h"

using namespace testing::ext;
namespace Unittest::AttestKey {
static struct HksBlob g_secInfo = { sizeof(SEC_INFO_DATA), (uint8_t *)SEC_INFO_DATA };
static struct HksBlob g_challenge = { sizeof(CHALLENGE_DATA), (uint8_t *)CHALLENGE_DATA };
static struct HksBlob g_version = { sizeof(VERSION_DATA), (uint8_t *)VERSION_DATA };

class HksAttestKeyNonIdsTest : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};

void HksAttestKeyNonIdsTest::SetUpTestCase(void)
{
}

void HksAttestKeyNonIdsTest::TearDownTestCase(void)
{
}

void HksAttestKeyNonIdsTest::SetUp()
{
}

void HksAttestKeyNonIdsTest::TearDown()
{
}

static const struct HksBlob g_keyAlias = { sizeof(ALIAS), (uint8_t *)ALIAS };

static const struct HksParam g_commonParams[] = {
    { .tag = HKS_TAG_ATTESTATION_ID_SEC_LEVEL_INFO, .blob = g_secInfo },
    { .tag = HKS_TAG_ATTESTATION_CHALLENGE, .blob = g_challenge },
    { .tag = HKS_TAG_ATTESTATION_ID_VERSION_INFO, .blob = g_version },
    { .tag = HKS_TAG_ATTESTATION_ID_ALIAS, .blob = g_keyAlias },
};

static const uint32_t g_keyParamsetSize = 1024;

/**
 * @tc.name: HksAttestKeyNonIdsTest.HksAttestKeyNonIdsTest001
 * @tc.desc: attest with right params and validate success.
 * @tc.type: FUNC
 * @tc.require: issueI5NY0L
 */
HWTEST_F(HksAttestKeyNonIdsTest, HksAttestKeyNonIdsTest001, TestSize.Level0)
{
    HKS_LOG_I("enter HksAttestKeyNonIdsTest001");
    int32_t ret = TestGenerateKey(&g_keyAlias);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    struct HksParamSet *paramSet = nullptr;
    GenerateParamSet(&paramSet, g_commonParams, sizeof(g_commonParams) / sizeof(g_commonParams[0]));
    HksCertChain *certChain = nullptr;
    const struct HksTestCertChain certParam = { true, true, true, g_size };
    (void)ConstructDataToCertChain(&certChain, &certParam);
    ret = HksAttestKey(&g_keyAlias, paramSet, certChain);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_I("HksAttestKey fail, ret is %" LOG_PUBLIC "d!", ret);
    }
    ASSERT_TRUE(ret == HKS_SUCCESS);
    HKS_LOG_I("Attest key success!");

    struct HksParam g_getParam = {
        .tag = HKS_TAG_ASYMMETRIC_PUBLIC_KEY_DATA,
        .blob = { .size = g_keyParamsetSize, .data = (uint8_t *)HksMalloc(g_keyParamsetSize) }
    };
    ASSERT_TRUE(g_getParam.blob.data != nullptr);

    struct HksParamSet *paramOutSet = nullptr;
    HksInitParamSet(&paramOutSet);
    HksAddParams(paramOutSet, &g_getParam, 1);
    HksBuildParamSet(&paramOutSet);
    HksFree(g_getParam.blob.data);
    ret = HksGetKeyParamSet(&g_keyAlias, nullptr, paramOutSet);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    struct HksParam *keySizeParam = nullptr;
    ret = HksGetParam(paramOutSet, HKS_TAG_KEY_SIZE, &keySizeParam);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    ASSERT_TRUE(keySizeParam->uint32Param == HKS_RSA_KEY_SIZE_2048);
    HksFreeParamSet(&paramOutSet);

    ret = ValidateCertChainTest(certChain, g_commonParams, NON_IDS_PARAM);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    HKS_LOG_I("Validate key success!");
    FreeCertChain(&certChain, certChain->certsCount);
    certChain = nullptr;

    HksFreeParamSet(&paramSet);

    ret = HksDeleteKey(&g_keyAlias, nullptr);
    ASSERT_TRUE(ret == HKS_SUCCESS);
}

/**
 * @tc.name: HksAttestKeyNonIdsTest.HksAttestKeyNonIdsTest002
 * @tc.desc: attest without cert data and fail.
 * @tc.type: FUNC
 * @tc.require: issueI5NY0L
 */
HWTEST_F(HksAttestKeyNonIdsTest, HksAttestKeyNonIdsTest002, TestSize.Level0)
{
    HKS_LOG_I("enter HksAttestKeyNonIdsTest002");
    int32_t ret = TestGenerateKey(&g_keyAlias);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    struct HksParamSet *paramSet = NULL;
    HksCertChain *certChain = NULL;
    GenerateParamSet(&paramSet, g_commonParams, sizeof(g_commonParams) / sizeof(g_commonParams[0]));
    const struct HksTestCertChain certParam = { true, true, false, g_size };
    (void)ConstructDataToCertChain(&certChain, &certParam);
    ret = HksAttestKey(&g_keyAlias, paramSet, certChain);
    ASSERT_TRUE(ret == HKS_ERROR_INVALID_ARGUMENT);

    HksFreeParamSet(&paramSet);
    FreeCertChain(&certChain, certChain->certsCount);
    certChain = NULL;

    ret = HksDeleteKey(&g_keyAlias, NULL);
    ASSERT_TRUE(ret == HKS_SUCCESS);
}

/**
 * @tc.name: HksAttestKeyNonIdsTest.HksAttestKeyNonIdsTest003
 * @tc.desc: attest without cert count and fail.
 * @tc.type: FUNC
 * @tc.require: issueI5NY0L
 */
HWTEST_F(HksAttestKeyNonIdsTest, HksAttestKeyNonIdsTest003, TestSize.Level0)
{
    HKS_LOG_I("enter HksAttestKeyNonIdsTest003");
    int32_t ret = TestGenerateKey(&g_keyAlias);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    struct HksParamSet *paramSet = NULL;
    GenerateParamSet(&paramSet, g_commonParams, sizeof(g_commonParams) / sizeof(g_commonParams[0]));
    HksCertChain *certChain = NULL;
    const struct HksTestCertChain certParam = { true, false, true, g_size };
    (void)ConstructDataToCertChain(&certChain, &certParam);
    ret = HksAttestKey(&g_keyAlias, paramSet, certChain);
    ASSERT_TRUE(ret == HKS_ERROR_INVALID_ARGUMENT);

    FreeCertChain(&certChain, certChain->certsCount);

    HksFreeParamSet(&paramSet);

    ret = HksDeleteKey(&g_keyAlias, NULL);
    ASSERT_TRUE(ret == HKS_SUCCESS);
}

/**
 * @tc.name: HksAttestKeyNonIdsTest.HksAttestKeyNonIdsTest004
 * @tc.desc: attest without cert chain and fail.
 * @tc.type: FUNC
 * @tc.require: issueI5NY0L
 */
HWTEST_F(HksAttestKeyNonIdsTest, HksAttestKeyNonIdsTest004, TestSize.Level0)
{
    HKS_LOG_I("enter HksAttestKeyNonIdsTest004");
    int32_t ret = TestGenerateKey(&g_keyAlias);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    struct HksParamSet *paramSet = NULL;
    GenerateParamSet(&paramSet, g_commonParams, sizeof(g_commonParams) / sizeof(g_commonParams[0]));
    HksCertChain *certChain = NULL;
    const struct HksTestCertChain certParam = { false, true, true, g_size };
    (void)ConstructDataToCertChain(&certChain, &certParam);
    ret = HksAttestKey(&g_keyAlias, paramSet, certChain);
    ASSERT_TRUE(ret == HKS_ERROR_NULL_POINTER);
    if (certChain != NULL) {
        FreeCertChain(&certChain, certChain->certsCount);
    }

    HksFreeParamSet(&paramSet);

    ret = HksDeleteKey(&g_keyAlias, NULL);
    ASSERT_TRUE(ret == HKS_SUCCESS);
}

/**
 * @tc.name: HksAttestKeyNonIdsTest.HksAttestKeyNonIdsTest005
 * @tc.desc: attest with base64 and validate success.
 * @tc.type: FUNC
 * @tc.require: issueI5NY0L
 */
HWTEST_F(HksAttestKeyNonIdsTest, HksAttestKeyNonIdsTest005, TestSize.Level0)
{
    HKS_LOG_I("enter HksAttestKeyNonIdsTest005");
    int32_t ret = TestGenerateKey(&g_keyAlias);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    struct HksParam g_commonParams[] = {
        { .tag = HKS_TAG_ATTESTATION_ID_SEC_LEVEL_INFO, .blob = g_secInfo },
        { .tag = HKS_TAG_ATTESTATION_CHALLENGE, .blob = g_challenge },
        { .tag = HKS_TAG_ATTESTATION_ID_VERSION_INFO, .blob = g_version },
        { .tag = HKS_TAG_ATTESTATION_ID_ALIAS, .blob = g_keyAlias },
        { .tag = HKS_TAG_ATTESTATION_BASE64, .boolParam = true },
    };
    struct HksParamSet *paramSet = NULL;
    GenerateParamSet(&paramSet, g_commonParams, sizeof(g_commonParams) / sizeof(g_commonParams[0]));
    HksCertChain *certChain = NULL;
    const struct HksTestCertChain certParam = { true, true, true, g_size };
    (void)ConstructDataToCertChain(&certChain, &certParam);
    ret = HksAttestKey(&g_keyAlias, paramSet, certChain);

    ASSERT_TRUE(ret == HKS_SUCCESS);
    ret = ValidateCertChainTest(certChain, g_commonParams, NON_IDS_PARAM);
    ASSERT_TRUE(ret == HKS_SUCCESS);

    FreeCertChain(&certChain, certChain->certsCount);
    certChain = NULL;

    HksFreeParamSet(&paramSet);

    ret = HksDeleteKey(&g_keyAlias, NULL);
    ASSERT_TRUE(ret == HKS_SUCCESS);
}

/**
 * @tc.name: HksAttestKeyNonIdsTest.HksAttestKeyNonIdsTest006
 * @tc.desc: attest with right params and validate success.
 * @tc.type: FUNC
 * @tc.require: issueI5NY0L
 */
HWTEST_F(HksAttestKeyNonIdsTest, HksAttestKeyNonIdsTest006, TestSize.Level0)
{
    HKS_LOG_I("enter HksAttestKeyNonIdsTest006");
    int32_t ret = TestGenerateKey(&g_keyAlias);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    struct HksParamSet *paramSet = nullptr;
    GenerateParamSet(&paramSet, g_commonParams, sizeof(g_commonParams) / sizeof(g_commonParams[0]));
    HksCertChain *certChain = nullptr;
    const struct HksTestCertChain certParam = { true, true, true, g_size };
    (void)ConstructDataToCertChain(&certChain, &certParam);
    ret = HksAttestKey(&g_keyAlias, paramSet, certChain);

    ASSERT_TRUE(ret == HKS_SUCCESS);
    
    struct HksParamSet *keyFlagparamSet = nullptr;
    ret = HksInitParamSet(&keyFlagparamSet);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    struct HksBlob keyFlagBlob = { .size = sizeof(uint32_t),
        .data = reinterpret_cast<uint8_t *>(HksMalloc(sizeof(uint32_t))) };

    struct HksParam keyFlagParam = { .tag = HKS_TAG_KEY_FLAG, .blob = keyFlagBlob };
    ret = HksAddParams(keyFlagparamSet, &keyFlagParam, 1);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    ret = HksBuildParamSet(&keyFlagparamSet);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    ret = HksValidateCertChain(certChain, keyFlagparamSet);
    ASSERT_TRUE(ret == HKS_SUCCESS) << "HksValidateCertChain and get key flag failed";

    ASSERT_EQ(keyFlagparamSet->params[0].blob.size, sizeof(uint32_t));
    uint32_t keyflag = *(reinterpret_cast<uint32_t *>(keyFlagparamSet->params[0].blob.data));
    ASSERT_EQ(HKS_KEY_FLAG_GENERATE_KEY, keyflag) << "fail compare key flag, " << keyflag;
    
    HKS_FREE_BLOB(keyFlagBlob);
    HksFreeParamSet(&keyFlagparamSet);

    FreeCertChain(&certChain, certChain->certsCount);
    certChain = nullptr;

    HksFreeParamSet(&paramSet);

    ret = HksDeleteKey(&g_keyAlias, nullptr);
    ASSERT_TRUE(ret == HKS_SUCCESS);
}

/**
 * @tc.name: HksAttestKeyNonIdsTest.HksAttestKeyNonIdsTest007
 * @tc.desc: attest with device id and expect HKS_ERROR_NO_PERMISSION
 * @tc.type: FUNC
 * @tc.require: issueI5NY0L
 */
HWTEST_F(HksAttestKeyNonIdsTest, HksAttestKeyNonIdsTest007, TestSize.Level0)
{
    HKS_LOG_I("enter HksAttestKeyNonIdsTest007");
    int32_t ret = TestGenerateKey(&g_keyAlias);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    static struct HksBlob dId = { sizeof(DEVICE_ID), (uint8_t *)DEVICE_ID };
    struct HksParam g_commonParams[] = {
        { .tag = HKS_TAG_ATTESTATION_ID_SEC_LEVEL_INFO, .blob = g_secInfo },
        { .tag = HKS_TAG_ATTESTATION_CHALLENGE, .blob = g_challenge },
        { .tag = HKS_TAG_ATTESTATION_ID_VERSION_INFO, .blob = g_version },
        { .tag = HKS_TAG_ATTESTATION_ID_ALIAS, .blob = g_keyAlias },
        { .tag = HKS_TAG_ATTESTATION_ID_DEVICE, .blob = dId },
        { .tag = HKS_TAG_ATTESTATION_BASE64, .boolParam = true },
    };
    struct HksParamSet *paramSet = NULL;
    GenerateParamSet(&paramSet, g_commonParams, sizeof(g_commonParams) / sizeof(g_commonParams[0]));
    HksCertChain *certChain = NULL;
    const struct HksTestCertChain certParam = { true, true, true, g_size };
    (void)ConstructDataToCertChain(&certChain, &certParam);
    ret = HksAttestKey(&g_keyAlias, paramSet, certChain);

    ASSERT_TRUE(ret == HKS_ERROR_NO_PERMISSION);

    FreeCertChain(&certChain, certChain->certsCount);
    certChain = NULL;

    HksFreeParamSet(&paramSet);

    ret = HksDeleteKey(&g_keyAlias, NULL);
    ASSERT_TRUE(ret == HKS_SUCCESS);
}
}
