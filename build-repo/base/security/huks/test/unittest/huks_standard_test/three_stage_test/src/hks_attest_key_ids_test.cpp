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

#include "hks_attest_key_ids_test.h"

#include <gtest/gtest.h>

#include "hks_attest_key_test_common.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

using namespace testing::ext;

namespace Unittest::AttestKey {
static struct HksBlob g_secInfo = { sizeof(SEC_INFO_DATA), (uint8_t *)SEC_INFO_DATA };
static struct HksBlob g_challenge = { sizeof(CHALLENGE_DATA), (uint8_t *)CHALLENGE_DATA };
static struct HksBlob g_version = { sizeof(VERSION_DATA), (uint8_t *)VERSION_DATA };
static struct HksBlob g_udid = { sizeof(UDID_DATA), (uint8_t *)UDID_DATA };
static struct HksBlob g_sn = { sizeof(SN_DATA), (uint8_t *)SN_DATA };
static struct HksBlob g_dId = { sizeof(DEVICE_ID), (uint8_t *)DEVICE_ID };

class HksAttestKeyIdsTest : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};

void HksAttestKeyIdsTest::SetUpTestCase(void)
{
}

void HksAttestKeyIdsTest::TearDownTestCase(void)
{
}

void HksAttestKeyIdsTest::SetUp()
{
}

void HksAttestKeyIdsTest::TearDown()
{
}

static int32_t SetIdsToken()
{
    uint64_t tokenId;
    const char **acls = new const char *[1];
    acls[0] = "ohos.permission.ACCESS_IDS"; // system_core
    const char **perms = new const char *[2];
    perms[0] = "ohos.permission.PLACE_CALL"; // system_basic
    perms[1] = "ohos.permission.ACCESS_IDS"; // system_core
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 2,
        .dcaps = nullptr,
        .perms = perms,
        .aplStr = "system_basic",
    };
    infoInstance.acls = acls;
    infoInstance.aclsNum = 1;
    infoInstance.processName = "test_attest";
    tokenId = GetAccessTokenId(&infoInstance);
    int32_t ret = SetSelfTokenID(tokenId);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_I("SetSelfTokenID fail, ret is %" LOG_PUBLIC "x!", ret);
    }
    return ret;
}

static const struct HksBlob g_keyAlias = { sizeof(ALIAS), (uint8_t *)ALIAS };

static const struct HksParam g_idsParams[] = {
    { .tag = HKS_TAG_ATTESTATION_ID_SEC_LEVEL_INFO, .blob = g_secInfo },
    { .tag = HKS_TAG_ATTESTATION_CHALLENGE, .blob = g_challenge },
    { .tag = HKS_TAG_ATTESTATION_ID_VERSION_INFO, .blob = g_version },
    { .tag = HKS_TAG_ATTESTATION_ID_ALIAS, .blob = g_keyAlias },
    { .tag = HKS_TAG_ATTESTATION_ID_UDID, .blob = g_udid },
    { .tag = HKS_TAG_ATTESTATION_ID_SERIAL, .blob = g_sn },
    { .tag = HKS_TAG_ATTESTATION_ID_DEVICE, .blob = g_dId },
};

/**
 * @tc.name: HksAttestKeyIdsTest.HksAttestKeyIdsTest001
 * @tc.desc: attest without permission and fail.
 * @tc.type: FUNC
 * @tc.require: issueI5NY0L
 */
HWTEST_F(HksAttestKeyIdsTest, HksAttestKeyIdsTest001, TestSize.Level0)
{
    int32_t ret = TestGenerateKey(&g_keyAlias);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    struct HksParamSet *paramSet = NULL;
    ret = GenerateParamSet(&paramSet, g_idsParams, sizeof(g_idsParams) / sizeof(g_idsParams[0]));
    ASSERT_TRUE(ret == HKS_SUCCESS);
    HksCertChain *certChain = NULL;
    const struct HksTestCertChain certParam = { true, true, true, g_size };
    ret = ConstructDataToCertChain(&certChain, &certParam);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    ret = HksAttestKey(&g_keyAlias, paramSet, certChain);

    ASSERT_TRUE(ret == HKS_ERROR_NO_PERMISSION);
    ret = ValidateCertChainTest(certChain, g_idsParams, IDS_PARAM);
    ASSERT_TRUE(ret != HKS_SUCCESS);
    FreeCertChain(&certChain, certChain->certsCount);
    certChain = NULL;

    HksFreeParamSet(&paramSet);

    ret = HksDeleteKey(&g_keyAlias, NULL);
    ASSERT_TRUE(ret == HKS_SUCCESS);
}

/**
 * @tc.name: HksAttestKeyIdsTest.HksAttestKeyIdsTest002
 * @tc.desc: attest with permission and right param then validate suc.
 * @tc.type: FUNC
 * @tc.require: issueI5NY0L
 */
HWTEST_F(HksAttestKeyIdsTest, HksAttestKeyIdsTest002, TestSize.Level0)
{
    int32_t ret = SetIdsToken();
    if (ret != HKS_SUCCESS) {
        HKS_LOG_I("SetSelfTokenID fail, ret is %" LOG_PUBLIC "x!", ret);
    }
    ASSERT_TRUE(ret == HKS_SUCCESS);
    ret = TestGenerateKey(&g_keyAlias);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    struct HksParamSet *paramSet = NULL;
    ret = GenerateParamSet(&paramSet, g_idsParams, sizeof(g_idsParams) / sizeof(g_idsParams[0]));
    HksCertChain *certChain = NULL;
    ASSERT_TRUE(ret == HKS_SUCCESS);
    const struct HksTestCertChain certParam = { true, true, true, g_size };
    ret = ConstructDataToCertChain(&certChain, &certParam);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    ret = HksAttestKey(&g_keyAlias, paramSet, certChain);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_I("HksAttestKey fail, ret is %" LOG_PUBLIC "d!", ret);
    }
    ASSERT_TRUE(ret == HKS_SUCCESS);
    ret = ValidateCertChainTest(certChain, g_idsParams, IDS_PARAM);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    FreeCertChain(&certChain, certChain->certsCount);
    certChain = NULL;

    HksFreeParamSet(&paramSet);

    ret = HksDeleteKey(&g_keyAlias, NULL);
    ASSERT_TRUE(ret == HKS_SUCCESS);
}

/**
 * @tc.name: HksAttestKeyIdsTest.HksAttestKeyIdsTest003
 * @tc.desc: attest without cert data and fail.
 * @tc.type: FUNC
 * @tc.require: issueI5NY0L
 */
HWTEST_F(HksAttestKeyIdsTest, HksAttestKeyIdsTest003, TestSize.Level0)
{
    int32_t ret = SetIdsToken();
    if (ret != HKS_SUCCESS) {
        HKS_LOG_I("SetSelfTokenID fail, ret is %" LOG_PUBLIC "x!", ret);
    }
    ASSERT_TRUE(ret == HKS_SUCCESS);
    struct HksParamSet *paramSet = NULL;
    GenerateParamSet(&paramSet, g_idsParams, sizeof(g_idsParams) / sizeof(g_idsParams[0]));
    ret = TestGenerateKey(&g_keyAlias);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    HksCertChain *certChain = NULL;
    const struct HksTestCertChain certParam = { true, true, false, g_size };
    (void)ConstructDataToCertChain(&certChain, &certParam);
    ret = HksAttestKey(&g_keyAlias, paramSet, certChain);
    ASSERT_TRUE(ret == HKS_ERROR_INVALID_ARGUMENT);
    FreeCertChain(&certChain, certChain->certsCount);
    certChain = NULL;

    HksFreeParamSet(&paramSet);

    ret = HksDeleteKey(&g_keyAlias, NULL);
    ASSERT_TRUE(ret == HKS_SUCCESS);
}

/**
 * @tc.name: HksAttestKeyIdsTest.HksAttestKeyIdsTest004
 * @tc.desc: attest without cert count and fail.
 * @tc.type: FUNC
 * @tc.require: issueI5NY0L
 */
HWTEST_F(HksAttestKeyIdsTest, HksAttestKeyIdsTest004, TestSize.Level0)
{
    int32_t ret = SetIdsToken();
    if (ret != HKS_SUCCESS) {
        HKS_LOG_I("SetSelfTokenID fail, ret is %" LOG_PUBLIC "x!", ret);
    }
    ASSERT_TRUE(ret == HKS_SUCCESS);
    ret = TestGenerateKey(&g_keyAlias);
    ASSERT_TRUE(ret == HKS_SUCCESS);
    HksCertChain *certChain = NULL;
    struct HksParamSet *paramSet = NULL;
    GenerateParamSet(&paramSet, g_idsParams, sizeof(g_idsParams) / sizeof(g_idsParams[0]));
    const struct HksTestCertChain certParam = { true, false, true, g_size };
    (void)ConstructDataToCertChain(&certChain, &certParam);
    ret = HksAttestKey(&g_keyAlias, paramSet, certChain);
    ASSERT_TRUE(ret == HKS_ERROR_INVALID_ARGUMENT);
    FreeCertChain(&certChain, certChain->certsCount);
    certChain = NULL;

    HksFreeParamSet(&paramSet);

    ret = HksDeleteKey(&g_keyAlias, NULL);
    ASSERT_TRUE(ret == HKS_SUCCESS);
}

/**
 * @tc.name: HksAttestKeyIdsTest.HksAttestKeyIdsTest005
 * @tc.desc: attest without cert chain and fail.
 * @tc.type: FUNC
 * @tc.require: issueI5NY0L
 */
HWTEST_F(HksAttestKeyIdsTest, HksAttestKeyIdsTest005, TestSize.Level0)
{
    int32_t ret = SetIdsToken();
    if (ret != HKS_SUCCESS) {
        HKS_LOG_I("SetSelfTokenID fail, ret is %" LOG_PUBLIC "x!", ret);
    }
    ASSERT_TRUE(ret == HKS_SUCCESS);
    ret = TestGenerateKey(&g_keyAlias);
    ASSERT_TRUE(ret == HKS_SUCCESS);

    struct HksParamSet *paramSet = NULL;
    GenerateParamSet(&paramSet, g_idsParams, sizeof(g_idsParams) / sizeof(g_idsParams[0]));
    HksCertChain *certChain = NULL;
    const struct HksTestCertChain certParam = { false, true, true, g_size };
    (void)ConstructDataToCertChain(&certChain, &certParam);
    ret = HksAttestKey(&g_keyAlias, paramSet, certChain);
    ASSERT_TRUE(ret == HKS_ERROR_NULL_POINTER);
    if (certChain != NULL) {
        FreeCertChain(&certChain, certChain->certsCount);
    }
    certChain = NULL;

    HksFreeParamSet(&paramSet);

    ret = HksDeleteKey(&g_keyAlias, NULL);
    ASSERT_TRUE(ret == HKS_SUCCESS);
}
}
