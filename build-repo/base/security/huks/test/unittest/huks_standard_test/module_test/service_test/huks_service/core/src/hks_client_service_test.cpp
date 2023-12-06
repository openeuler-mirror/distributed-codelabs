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

#include "hks_client_service_test.h"

#include <gtest/gtest.h>
#include <cstring>

#include "hks_api.h"
#include "hks_attest_key_test_common.h"
#include "hks_client_service.h"
#include "hks_log.h"
#include "hks_mem.h"
#include "hks_param.h"
#include "hks_session_manager.h"
#include "hks_type_inner.h"

using namespace testing::ext;
using namespace Unittest::AttestKey;
namespace Unittest::HksClientServiceTest {
class HksClientServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};

void HksClientServiceTest::SetUpTestCase(void)
{
}

void HksClientServiceTest::TearDownTestCase(void)
{
}

void HksClientServiceTest::SetUp()
{
    HksServiceInitialize();
}

void HksClientServiceTest::TearDown()
{
}

static int32_t TestGenerateKeyWithProcessInfo(const struct HksBlob *keyAlias, const struct HksProcessInfo *processInfo)
{
    struct HksParam tmpParams[] = {
        { .tag = HKS_TAG_KEY_STORAGE_FLAG, .uint32Param = HKS_STORAGE_PERSISTENT },
        { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_RSA },
        { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_RSA_KEY_SIZE_2048 },
        { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA256 },
        { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_PSS },
        { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_VERIFY },
        { .tag = HKS_TAG_KEY_GENERATE_TYPE, .uint32Param = HKS_KEY_GENERATE_TYPE_DEFAULT },
        { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_ECB },
    };
    struct HksParamSet *paramSet = nullptr;
    int32_t ret = HksInitParamSet(&paramSet);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("HksServiceDeleteProcessInfo HksInitParamSet failed");
        return ret;
    }

    ret = HksAddParams(paramSet, tmpParams, sizeof(tmpParams) / sizeof(tmpParams[0]));
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("HksServiceDeleteProcessInfo HksAddParams failed");
        HksFreeParamSet(&paramSet);
        return ret;
    }

    ret = HksBuildParamSet(&paramSet);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("HksServiceDeleteProcessInfo HksBuildParamSet failed");
        HksFreeParamSet(&paramSet);
        return ret;
    }

    ret = HksServiceGenerateKey(processInfo, keyAlias, paramSet, nullptr);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("HksServiceDeleteProcessInfo HksGenerateKey failed");
    }
    HksFreeParamSet(&paramSet);
    return ret;
}

static const char *g_processNameString = "hks_client";
static const struct HksBlob g_processName = { strlen(g_processNameString), (uint8_t *)g_processNameString };
static uint32_t g_userIdInt = 0;
static const struct HksBlob g_userId = { sizeof(g_userIdInt), (uint8_t *)(&g_userIdInt)};

/**
 * @tc.name: HksClientServiceTest.HksClientServiceTest001
 * @tc.desc: tdd HksServiceDeleteProcessInfo, expect HKS_ERROR_INVALID_ARGUMENT
 * @tc.type: FUNC
 */
HWTEST_F(HksClientServiceTest, HksClientServiceTest001, TestSize.Level0)
{
    HKS_LOG_I("enter HksClientServiceTest001");
    const char *alias = "HksClientServiceTest001";
    const struct HksBlob keyAlias = { strlen(alias), (uint8_t *)alias };
    struct HksProcessInfo processInfo = { g_userId, g_processName, g_userIdInt, 0 };
    int32_t ret = TestGenerateKeyWithProcessInfo(&keyAlias, &processInfo);
    ASSERT_EQ(ret, HKS_SUCCESS) << "HksClientServiceTest001 TestGenerateKey failed, ret = " << ret;
    ret = HksServiceKeyExist(&processInfo, &keyAlias);
    EXPECT_EQ(ret, HKS_SUCCESS) << "HksClientServiceTest001 HksServiceDeleteProcessInfo failed, ret = " << ret;
    uint64_t handle = 111;
    struct HksBlob operationHandle = { .size = sizeof(uint64_t), .data = (uint8_t *)&handle };
    CreateOperation(&processInfo, &operationHandle, true);
    HksServiceDeleteProcessInfo(&processInfo);
    ret = HksServiceKeyExist(&processInfo, &keyAlias);
    EXPECT_NE(ret, HKS_SUCCESS) << "HksClientServiceTest001 HksServiceDeleteProcessInfo failed, ret = " << ret;
}

/**
 * @tc.name: HksClientServiceTest.HksClientServiceTest002
 * @tc.desc: tdd HksServiceDeleteProcessInfo, expect HKS_ERROR_INVALID_ARGUMENT
 * @tc.type: FUNC
 */
HWTEST_F(HksClientServiceTest, HksClientServiceTest002, TestSize.Level0)
{
    HKS_LOG_I("enter HksClientServiceTest002");
    const char *alias = "HksClientServiceTest002";
    const struct HksBlob keyAlias = { strlen(alias), (uint8_t *)alias };
    struct HksProcessInfo processInfo = { g_userId, g_processName, g_userIdInt, 0 };
    int32_t ret = TestGenerateKeyWithProcessInfo(&keyAlias, &processInfo);
    ASSERT_EQ(ret, HKS_SUCCESS) << "HksClientServiceTest002 TestGenerateKey failed, ret = " << ret;
    ret = HksServiceKeyExist(&processInfo, &keyAlias);
    EXPECT_EQ(ret, HKS_SUCCESS) << "HksClientServiceTest002 HksServiceDeleteProcessInfo failed, ret = " << ret;

    struct HksBlob processName2 = { 0, nullptr };
    struct HksProcessInfo processInfo2 = { g_userId, processName2, g_userIdInt, 0 };
    HksServiceDeleteProcessInfo(&processInfo2);
    ret = HksServiceKeyExist(&processInfo, &keyAlias);
    EXPECT_NE(ret, HKS_SUCCESS) << "HksClientServiceTest002 HksServiceDeleteProcessInfo failed, ret = " << ret;
}

static const uint32_t g_defaultCertSize = 10240;

static void FreeCertChainBlob(struct HksBlob *certChain)
{
    HKS_FREE_PTR(certChain->data);
    certChain->size = 0;
    HKS_FREE_PTR(certChain);
}

static int32_t ConstructCertChainBlob(struct HksBlob **outCertChain)
{
    struct HksBlob *certChain = (struct HksBlob *)HksMalloc(sizeof(struct HksBlob) * HKS_CERT_COUNT);
    if (certChain == nullptr) {
        return HKS_ERROR_MALLOC_FAIL;
    }
    certChain->data = (uint8_t *)HksMalloc(g_defaultCertSize);
    if (certChain->data == nullptr) {
        FreeCertChainBlob(certChain);
        return HKS_ERROR_MALLOC_FAIL;
    }
    certChain->size = g_defaultCertSize;

    *outCertChain = certChain;
    return HKS_SUCCESS;
}

static struct HksBlob g_secInfo = { sizeof(SEC_INFO_DATA), (uint8_t *)SEC_INFO_DATA };
static struct HksBlob g_challenge = { sizeof(CHALLENGE_DATA), (uint8_t *)CHALLENGE_DATA };
static struct HksBlob g_version = { sizeof(VERSION_DATA), (uint8_t *)VERSION_DATA };
static struct HksBlob g_udid = { sizeof(UDID_DATA), (uint8_t *)UDID_DATA };
static struct HksBlob g_sn = { sizeof(SN_DATA), (uint8_t *)SN_DATA };
static struct HksBlob g_dId = { sizeof(DEVICE_ID), (uint8_t *)DEVICE_ID };

static struct HksBlob g_appIdBlob = { sizeof(APP_ID), (uint8_t *)APP_ID };
static struct HksBlob g_brandBlob = { sizeof(ATTEST_BRAND), (uint8_t *)ATTEST_BRAND };

/**
 * @tc.name: HksClientServiceTest.HksClientServiceTest003
 * @tc.desc: tdd HksServiceAttestKey, expect HKS_SUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(HksClientServiceTest, HksClientServiceTest003, TestSize.Level0)
{
    HKS_LOG_I("enter HksClientServiceTest003");
    const char *alias = "HksClientServiceTest003_alias";
    const char *product = "product";
    const char *imei = "imei";
    const char *meid = "meid";
    const char *manufacturer = "manufacturer";
    const char *model = "model";
    const char *socid = "socid";
    const struct HksBlob keyAliasBlob = { strlen(alias), (uint8_t *)alias };
    const struct HksBlob productBlob = { strlen(product), (uint8_t *)product };
    const struct HksBlob imeiBlob = { strlen(imei), (uint8_t *)imei };
    const struct HksBlob meidBlob = { strlen(meid), (uint8_t *)meid };
    const struct HksBlob manufacturerBlob = { strlen(manufacturer), (uint8_t *)manufacturer };
    const struct HksBlob modelBlob = { strlen(model), (uint8_t *)model };
    const struct HksBlob socidBlob = { strlen(socid), (uint8_t *)socid };
    const struct HksParam commonParams[] = {
        { .tag = HKS_TAG_ATTESTATION_ID_SEC_LEVEL_INFO, .blob = g_secInfo },
        { .tag = HKS_TAG_ATTESTATION_CHALLENGE, .blob = g_challenge },
        { .tag = HKS_TAG_ATTESTATION_ID_VERSION_INFO, .blob = g_version },
        { .tag = HKS_TAG_ATTESTATION_ID_ALIAS, .blob = keyAliasBlob },
        { .tag = HKS_TAG_ATTESTATION_ID_BRAND, .blob = g_brandBlob },
        { .tag = HKS_TAG_ATTESTATION_ID_PRODUCT, .blob = productBlob },
        { .tag = HKS_TAG_ATTESTATION_ID_IMEI, .blob = imeiBlob },
        { .tag = HKS_TAG_ATTESTATION_ID_MEID, .blob = meidBlob },
        { .tag = HKS_TAG_ATTESTATION_ID_MANUFACTURER, .blob = manufacturerBlob },
        { .tag = HKS_TAG_ATTESTATION_ID_MODEL, .blob = modelBlob },
        { .tag = HKS_TAG_ATTESTATION_ID_SOCID, .blob = socidBlob },
        { .tag = HKS_TAG_ATTESTATION_APPLICATION_ID, .blob = g_appIdBlob },
    };
    uint32_t userIdInt = 0;
    struct HksBlob userId = { sizeof(userIdInt), (uint8_t *)(&userIdInt)};
    const char *processNameString = "hks_client";
    struct HksBlob processName = { strlen(processNameString), (uint8_t *)processNameString };
    struct HksProcessInfo processInfo = { userId, processName, userIdInt };
    int32_t ret = TestGenerateKeyWithProcessInfo(&keyAliasBlob, &processInfo);
    ASSERT_EQ(ret, HKS_SUCCESS) << "TestGenerateKey failed, ret = " << ret;
    struct HksParamSet *paramSet = nullptr;
    GenerateParamSet(&paramSet, commonParams, sizeof(commonParams) / sizeof(commonParams[0]));
    struct HksBlob *certChain = nullptr;
    ret = ConstructCertChainBlob(&certChain);
    ASSERT_TRUE(ret == HKS_SUCCESS) << "ConstructCertChainBlob failed, ret = " << ret;
    ret = HksServiceAttestKey(&processInfo, &keyAliasBlob, paramSet, certChain);
    ASSERT_TRUE(ret == HKS_SUCCESS) << "HksServiceAttestKey failed, ret = " << ret;
    FreeCertChainBlob(certChain);
    HksFreeParamSet(&paramSet);
    ret = HksServiceDeleteKey(&processInfo, &keyAliasBlob);
    ASSERT_TRUE(ret == HKS_SUCCESS);
}

static const struct HksParam g_generateX25519Params[] = {
    { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_X25519 },
    { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_AGREE },
    { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_CURVE25519_KEY_SIZE_256 },
};

static int32_t GenerateX25519(const struct HksBlob *keyAlias, const struct HksProcessInfo *processInfo)
{
    struct HksParamSet *paramSet = nullptr;
    int32_t ret = GenerateParamSet(&paramSet, g_generateX25519Params, sizeof(g_generateX25519Params) /
        sizeof(g_generateX25519Params[0]));
    EXPECT_EQ(ret, HKS_SUCCESS) << "GenerateParamSet failed, ret = " << ret;

    ret = HksServiceGenerateKey(processInfo, keyAlias, paramSet, nullptr);
    EXPECT_EQ(ret, HKS_SUCCESS) << "HksGenerateKey failed, ret = " << ret;
    return ret;
}

/**
 * @tc.name: HksClientServiceTest.HksClientServiceTest004
 * @tc.desc: tdd HksServiceAttestKey with x25519, expect HKS_SUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(HksClientServiceTest, HksClientServiceTest004, TestSize.Level0)
{
    HKS_LOG_I("enter HksClientServiceTest004");
    const char *alias = "HksClientServiceTest004_alias";
    const struct HksBlob keyAlias = { strlen(alias), (uint8_t *)alias };
    const struct HksParam attestParams[] = {
    { .tag = HKS_TAG_ATTESTATION_ID_SEC_LEVEL_INFO, .blob = g_secInfo },
    { .tag = HKS_TAG_ATTESTATION_CHALLENGE, .blob = g_challenge },
    { .tag = HKS_TAG_ATTESTATION_ID_VERSION_INFO, .blob = g_version },
    { .tag = HKS_TAG_ATTESTATION_ID_ALIAS, .blob = keyAlias },
    { .tag = HKS_TAG_ATTESTATION_ID_UDID, .blob = g_udid },
    { .tag = HKS_TAG_ATTESTATION_ID_SERIAL, .blob = g_sn },
    { .tag = HKS_TAG_ATTESTATION_ID_DEVICE, .blob = g_dId },
    };
    struct HksProcessInfo processInfo = { g_userId, g_processName, g_userIdInt, 0 };
    int32_t ret = GenerateX25519(&keyAlias, &processInfo);
    EXPECT_EQ(ret, HKS_SUCCESS) << "GenerateX25519 failed, ret = " << ret;
    struct HksParamSet *paramSet = nullptr;
    GenerateParamSet(&paramSet, attestParams, sizeof(attestParams) / sizeof(attestParams[0]));
    struct HksBlob *certChain = nullptr;
    ret = ConstructCertChainBlob(&certChain);
    ASSERT_TRUE(ret == HKS_SUCCESS) << "ConstructCertChainBlob failed, ret = " << ret;
    ret = HksServiceAttestKey(&processInfo, &keyAlias, paramSet, certChain);
    ASSERT_TRUE(ret == HKS_SUCCESS) << "HksServiceAttestKey failed, ret = " << ret;
    HKS_LOG_I("Attest key success!");
    FreeCertChainBlob(certChain);

    HksFreeParamSet(&paramSet);

    ret = HksServiceDeleteKey(&processInfo, &keyAlias);
    ASSERT_TRUE(ret == HKS_SUCCESS);
}

static const struct HksParam g_generateECCParams[] = {
    { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_ECC },
    { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_ECC_KEY_SIZE_256 },
    { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_SIGN | HKS_KEY_PURPOSE_VERIFY },
    { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA384 },
    { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
};

static int32_t GenerateECC(const struct HksBlob *keyAlias, const struct HksProcessInfo *processInfo)
{
    struct HksParamSet *paramSet = nullptr;
    int32_t ret = GenerateParamSet(&paramSet, g_generateECCParams, sizeof(g_generateECCParams) /
        sizeof(g_generateECCParams[0]));
    EXPECT_EQ(ret, HKS_SUCCESS) << "GenerateParamSet failed, ret = " << ret;

    ret = HksServiceGenerateKey(processInfo, keyAlias, paramSet, nullptr);
    EXPECT_EQ(ret, HKS_SUCCESS) << "HksGenerateKey failed, ret = " << ret;
    return ret;
}

/**
 * @tc.name: HksClientServiceTest.HksClientServiceTest005
 * @tc.desc: tdd HksServiceAttestKey with ecc, expect HKS_SUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(HksClientServiceTest, HksClientServiceTest005, TestSize.Level0)
{
    HKS_LOG_I("enter HksClientServiceTest005");
    const char *alias = "HksClientServiceTest005_alias";
    const struct HksBlob keyAlias = { strlen(alias), (uint8_t *)alias };
    const struct HksParam attestParams[] = {
    { .tag = HKS_TAG_ATTESTATION_ID_SEC_LEVEL_INFO, .blob = g_secInfo },
    { .tag = HKS_TAG_ATTESTATION_CHALLENGE, .blob = g_challenge },
    { .tag = HKS_TAG_ATTESTATION_ID_DEVICE, .blob = g_dId },
    { .tag = HKS_TAG_ATTESTATION_ID_VERSION_INFO, .blob = g_version },
    { .tag = HKS_TAG_ATTESTATION_ID_ALIAS, .blob = keyAlias },
    { .tag = HKS_TAG_ATTESTATION_ID_UDID, .blob = g_udid },
    { .tag = HKS_TAG_ATTESTATION_ID_SERIAL, .blob = g_sn },
    };
    struct HksProcessInfo processInfo = { g_userId, g_processName, g_userIdInt, 0 };
    int32_t ret = GenerateECC(&keyAlias, &processInfo);
    EXPECT_EQ(ret, HKS_SUCCESS) << "HksClientServiceTest005 GenerateECC failed, ret = " << ret;
    struct HksParamSet *paramSet = nullptr;
    ret = GenerateParamSet(&paramSet, attestParams, sizeof(attestParams) / sizeof(attestParams[0]));
    EXPECT_EQ(ret, HKS_SUCCESS);
    struct HksBlob *certChain = nullptr;
    ret = ConstructCertChainBlob(&certChain);
    ASSERT_TRUE(ret == HKS_SUCCESS) << "HksClientServiceTest005 ConstructCertChainBlob failed, ret = " << ret;
    ret = HksServiceAttestKey(&processInfo, &keyAlias, paramSet, certChain);
    ASSERT_TRUE(ret == HKS_SUCCESS) << "HksClientServiceTest005 HksServiceAttestKey failed, ret = " << ret;
    HKS_LOG_I("Attest key success!");
    FreeCertChainBlob(certChain);

    HksFreeParamSet(&paramSet);

    ret = HksServiceDeleteKey(&processInfo, &keyAlias);
    ASSERT_TRUE(ret == HKS_SUCCESS);
}

/**
 * @tc.name: HksClientServiceTest.HksClientServiceTest006
 * @tc.desc: tdd HksServiceGetKeyParamSet with ecc, expect HKS_ERROR_BAD_STATE
 * @tc.type: FUNC
 */
HWTEST_F(HksClientServiceTest, HksClientServiceTest006, TestSize.Level0)
{
    HKS_LOG_I("enter HksClientServiceTest006");
    const char *alias = "HksClientServiceTest006_alias";
    const struct HksBlob keyAlias = { strlen(alias), (uint8_t *)alias };
    
    struct HksProcessInfo processInfo = { g_userId, g_processName, g_userIdInt, 0 };
    int32_t ret = GenerateECC(&keyAlias, &processInfo);
    EXPECT_EQ(ret, HKS_SUCCESS) << "HksClientServiceTest006 GenerateECC failed, ret = " << ret;
    
    struct HksParamSet *paramSet = nullptr;
    ret = GenerateParamSet(&paramSet, nullptr, 0);
    ASSERT_TRUE(ret == HKS_SUCCESS);

    struct HksProcessInfo processInfo2 = { g_userId, g_processName, g_userIdInt, 1 };

    ret = HksServiceGetKeyParamSet(&processInfo2, &keyAlias, paramSet);
    ASSERT_TRUE(ret == HKS_ERROR_BAD_STATE);

    HksFreeParamSet(&paramSet);
    (void)HksServiceDeleteKey(&processInfo, &keyAlias);
}

static uint32_t g_paramSetDefaultSize = 4096;

/**
 * @tc.name: HksClientServiceTest.HksClientServiceTest007
 * @tc.desc: tdd HksServiceGetKeyParamSet with ecc, expect HKS_SUCCESS and no accessTokenId in paramSet
 * @tc.type: FUNC
 */
HWTEST_F(HksClientServiceTest, HksClientServiceTest007, TestSize.Level0)
{
    HKS_LOG_I("enter HksClientServiceTest007");
    const char *alias = "HksClientServiceTest007_alias";
    const struct HksBlob keyAlias = { strlen(alias), (uint8_t *)alias };

    struct HksProcessInfo processInfo = { g_userId, g_processName, g_userIdInt, 0 };
    int32_t ret = GenerateECC(&keyAlias, &processInfo);
    EXPECT_EQ(ret, HKS_SUCCESS) << "HksClientServiceTest007 GenerateECC failed, ret = " << ret;

    struct HksParamSet *paramSet = (struct HksParamSet *)HksMalloc(g_paramSetDefaultSize);

    ASSERT_TRUE(paramSet != nullptr);

    paramSet->paramSetSize = g_paramSetDefaultSize;

    ret = HksServiceGetKeyParamSet(&processInfo, &keyAlias, paramSet);
    ASSERT_TRUE(ret == HKS_SUCCESS);

    struct HksParam *accessTokenId = nullptr;
    ret = HksGetParam(paramSet, HKS_TAG_ACCESS_TOKEN_ID, &accessTokenId);
    ASSERT_TRUE(ret == HKS_ERROR_PARAM_NOT_EXIST);

    HksFreeParamSet(&paramSet);
    (void)HksServiceDeleteKey(&processInfo, &keyAlias);
}

static struct HksParam g_aesGenParams[] = {
    {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_AES_KEY_SIZE_256
    }, {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_AES
    }, {
        .tag = HKS_TAG_BLOCK_MODE,
        .uint32Param = HKS_MODE_CBC
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT
    }, {
        .tag = HKS_TAG_PADDING,
        .uint32Param = HKS_PADDING_NONE
    }
};

static const uint32_t g_ivSize = 16;
static uint8_t g_iv[g_ivSize] = { 0 };

static struct HksParam g_aesEncryptParams[] = {
    {
        .tag = HKS_TAG_IV,
        .blob = {
            .size = g_ivSize,
            .data = (uint8_t *)g_iv
        }
    }, {
        .tag = HKS_TAG_PADDING,
        .uint32Param = HKS_PADDING_NONE
    }, {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_AES
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_ENCRYPT
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_AES_KEY_SIZE_256
    }, {
        .tag = HKS_TAG_BLOCK_MODE,
        .uint32Param = HKS_MODE_CBC
    }, {
        .tag = HKS_TAG_DIGEST,
        .uint32Param = HKS_DIGEST_NONE
    },
};

/**
 * @tc.name: HksClientServiceTest.HksClientServiceTest008
 * @tc.desc: tdd three stage with access token id, expect HKS_SHKS_ERROR_BAD_STATEUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(HksClientServiceTest, HksClientServiceTest008, TestSize.Level0)
{
    HKS_LOG_I("enter HksClientServiceTest008");
    const char *alias = "alias";
    const struct HksBlob keyAlias = { strlen(alias), (uint8_t *)alias };

    struct HksProcessInfo processInfo = { g_userId, g_processName, g_userIdInt, 0 };
    struct HksParamSet *genParamSet = nullptr;
    int32_t ret = GenerateParamSet(&genParamSet, g_aesGenParams, sizeof(g_aesGenParams) / sizeof(g_aesGenParams[0]));
    ASSERT_TRUE(ret == HKS_SUCCESS);
    ret = HksServiceGenerateKey(&processInfo, &keyAlias, genParamSet, nullptr);
    ASSERT_TRUE(ret == HKS_SUCCESS);

    struct HksParamSet *encParamSet = nullptr;
    ret = GenerateParamSet(&encParamSet, g_aesEncryptParams,
        sizeof(g_aesEncryptParams) / sizeof(g_aesEncryptParams[0]));
    ASSERT_TRUE(ret == HKS_SUCCESS);
    
    struct HksBlob handle = { .size = sizeof(uint64_t), .data = (uint8_t *)HksMalloc(sizeof(uint64_t)) };
    struct HksBlob token = { 0 };

    ret = HksServiceInit(&processInfo, &keyAlias, encParamSet, &handle, &token);
    ASSERT_TRUE(ret == HKS_SUCCESS);

    struct HksProcessInfo processInfo2 = { g_userId, g_processName, g_userIdInt, 1 };

    struct HksBlob inDataBlob = { 0 };
    struct HksBlob outDataBlob = { 0 };

    ret = HksServiceUpdate(&handle, &processInfo2, encParamSet, &inDataBlob, &outDataBlob);
    ASSERT_TRUE(ret == HKS_ERROR_BAD_STATE);

    (void)HksServiceAbort(&handle, &processInfo, encParamSet);

    HksFreeParamSet(&genParamSet);
    HksFreeParamSet(&encParamSet);
    (void)HksServiceDeleteKey(&processInfo, &keyAlias);
}
}
