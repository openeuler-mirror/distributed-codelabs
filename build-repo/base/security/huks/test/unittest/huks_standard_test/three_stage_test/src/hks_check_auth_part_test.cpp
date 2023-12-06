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

#include "hks_check_auth_part_test.h"

#include <gtest/gtest.h>

using namespace testing::ext;
namespace Unittest::CheckAuthTest {
class HksCheckAuthPartTest : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};

void HksCheckAuthPartTest::SetUpTestCase(void)
{
}

void HksCheckAuthPartTest::TearDownTestCase(void)
{
}

void HksCheckAuthPartTest::SetUp()
{
    EXPECT_EQ(HksInitialize(), 0);
}

void HksCheckAuthPartTest::TearDown()
{
}

const uint32_t KEY_PARAMSET_SIZE = 1024;

/* 001: gen aes for cipher; init for cipher */
const TestAuthCaseParams HKS_AUTH_TEST_001_PARAMS = {
    .genParams =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_AES },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_128 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CBC },
            { .tag = HKS_TAG_USER_AUTH_TYPE, .uint32Param = HKS_USER_AUTH_TYPE_PIN },
            { .tag = HKS_TAG_KEY_AUTH_ACCESS_TYPE, .uint32Param = HKS_AUTH_ACCESS_INVALID_CLEAR_PASSWORD },
            { .tag = HKS_TAG_CHALLENGE_TYPE, .uint32Param = HKS_CHALLENGE_TYPE_NORMAL },
        },
    .initParams =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_AES },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_128 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CBC },
            { .tag = HKS_TAG_USER_AUTH_TYPE, .uint32Param = HKS_USER_AUTH_TYPE_PIN },
            { .tag = HKS_TAG_KEY_AUTH_ACCESS_TYPE, .uint32Param = HKS_AUTH_ACCESS_INVALID_CLEAR_PASSWORD },
            { .tag = HKS_TAG_CHALLENGE_TYPE, .uint32Param = HKS_CHALLENGE_TYPE_NORMAL },
        },
    .initResult = HKS_SUCCESS
};

#ifdef _USE_OPENSSL_
/* mbedtls engine don't support SM4 alg */
/* 002: gen sm4 for cipher; init for cipher */
const TestAuthCaseParams HKS_AUTH_TEST_002_PARAMS = {
    .genParams =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_SM4 },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_SM4_KEY_SIZE_128 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_PKCS7 },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CBC },
            { .tag = HKS_TAG_USER_AUTH_TYPE, .uint32Param = HKS_USER_AUTH_TYPE_FINGERPRINT },
            { .tag = HKS_TAG_KEY_AUTH_ACCESS_TYPE, .uint32Param = HKS_AUTH_ACCESS_INVALID_NEW_BIO_ENROLL },
            { .tag = HKS_TAG_CHALLENGE_TYPE, .uint32Param = HKS_CHALLENGE_TYPE_NORMAL },
        },
    .initParams =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_SM4 },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_SM4_KEY_SIZE_128 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_PKCS7 },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CBC },
            { .tag = HKS_TAG_USER_AUTH_TYPE, .uint32Param = HKS_USER_AUTH_TYPE_FINGERPRINT },
            { .tag = HKS_TAG_KEY_AUTH_ACCESS_TYPE, .uint32Param = HKS_AUTH_ACCESS_INVALID_NEW_BIO_ENROLL },
            { .tag = HKS_TAG_CHALLENGE_TYPE, .uint32Param = HKS_CHALLENGE_TYPE_NORMAL },
        },
    .initResult = HKS_SUCCESS
};
#endif

/* 003: gen hmac for mac; init for mac */
const TestAuthCaseParams HKS_AUTH_TEST_003_PARAMS = {
    .genParams =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_HMAC },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_MAC },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_256 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA1 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CBC },
            { .tag = HKS_TAG_USER_AUTH_TYPE, .uint32Param = HKS_USER_AUTH_TYPE_FACE },
            { .tag = HKS_TAG_KEY_AUTH_ACCESS_TYPE, .uint32Param = HKS_AUTH_ACCESS_INVALID_NEW_BIO_ENROLL },
            { .tag = HKS_TAG_CHALLENGE_TYPE, .uint32Param = HKS_CHALLENGE_TYPE_NORMAL },
        },
    .initParams =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_HMAC },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_MAC },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_256 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA1  },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CBC },
            { .tag = HKS_TAG_USER_AUTH_TYPE, .uint32Param = HKS_USER_AUTH_TYPE_FACE },
            { .tag = HKS_TAG_KEY_AUTH_ACCESS_TYPE, .uint32Param = HKS_AUTH_ACCESS_INVALID_NEW_BIO_ENROLL },
            { .tag = HKS_TAG_CHALLENGE_TYPE, .uint32Param = HKS_CHALLENGE_TYPE_NORMAL },
        },
    .initResult = HKS_SUCCESS
};

/* 004: gen sm3 for mac; init for mac */
const TestAuthCaseParams HKS_AUTH_TEST_004_PARAMS = {
    .genParams =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_SM3 },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_MAC },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_SM2_KEY_SIZE_256 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SM3 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CBC },
            { .tag = HKS_TAG_USER_AUTH_TYPE, .uint32Param = HKS_USER_AUTH_TYPE_FACE },
            { .tag = HKS_TAG_CHALLENGE_TYPE, .uint32Param = HKS_CHALLENGE_TYPE_NORMAL },
        },
    .initParams =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_SM3 },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_MAC },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_SM2_KEY_SIZE_256 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SM3 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CBC },
            { .tag = HKS_TAG_USER_AUTH_TYPE, .uint32Param = HKS_USER_AUTH_TYPE_FACE },
            { .tag = HKS_TAG_CHALLENGE_TYPE, .uint32Param = HKS_CHALLENGE_TYPE_NORMAL },
        },
    .initResult = HKS_ERROR_INVALID_ARGUMENT
};

/* 005: gen x25519 for agree; init for agree */
const TestAuthCaseParams HKS_AUTH_TEST_005_PARAMS = {
    .genParams =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_X25519 },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_AGREE },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_CURVE25519_KEY_SIZE_256 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CBC },
            { .tag = HKS_TAG_USER_AUTH_TYPE, .uint32Param = HKS_USER_AUTH_TYPE_FINGERPRINT },
            { .tag = HKS_TAG_KEY_AUTH_ACCESS_TYPE, .uint32Param = HKS_AUTH_ACCESS_INVALID_CLEAR_PASSWORD },
            { .tag = HKS_TAG_CHALLENGE_TYPE, .uint32Param = HKS_CHALLENGE_TYPE_NORMAL },
        },
    .initParams =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_X25519 },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_AGREE },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_CURVE25519_KEY_SIZE_256 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CBC },
            { .tag = HKS_TAG_USER_AUTH_TYPE, .uint32Param = HKS_USER_AUTH_TYPE_FINGERPRINT },
            { .tag = HKS_TAG_KEY_AUTH_ACCESS_TYPE, .uint32Param = HKS_AUTH_ACCESS_INVALID_CLEAR_PASSWORD },
            { .tag = HKS_TAG_CHALLENGE_TYPE, .uint32Param = HKS_CHALLENGE_TYPE_NORMAL },
        },
    .initResult = HKS_SUCCESS
};

/* 006: gen ecc for sign; init for sign */
const TestAuthCaseParams HKS_AUTH_TEST_006_PARAMS = {
    .genParams =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_ECC },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_SIGN | HKS_KEY_PURPOSE_VERIFY },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_ECC_KEY_SIZE_224 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CBC },
            { .tag = HKS_TAG_USER_AUTH_TYPE, .uint32Param = HKS_USER_AUTH_TYPE_PIN },
            { .tag = HKS_TAG_KEY_AUTH_ACCESS_TYPE, .uint32Param = HKS_AUTH_ACCESS_INVALID_NEW_BIO_ENROLL },
            { .tag = HKS_TAG_CHALLENGE_TYPE, .uint32Param = HKS_CHALLENGE_TYPE_NORMAL },
        },
    .initParams =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_ECC },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_SIGN },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_KEY_PURPOSE_SIGN },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CBC },
            { .tag = HKS_TAG_USER_AUTH_TYPE, .uint32Param = HKS_USER_AUTH_TYPE_PIN },
            { .tag = HKS_TAG_KEY_AUTH_ACCESS_TYPE, .uint32Param = HKS_AUTH_ACCESS_INVALID_NEW_BIO_ENROLL },
            { .tag = HKS_TAG_CHALLENGE_TYPE, .uint32Param = HKS_CHALLENGE_TYPE_NORMAL },
        },
    .initResult = HKS_ERROR_INVALID_ARGUMENT
};

/* 007: gen ed25519 for sign; init for sign */
const TestAuthCaseParams HKS_AUTH_TEST_007_PARAMS = {
    .genParams =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_ED25519 },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_SIGN | HKS_KEY_PURPOSE_VERIFY },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_CURVE25519_KEY_SIZE_256 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA1 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CBC },
            { .tag = HKS_TAG_USER_AUTH_TYPE, .uint32Param = HKS_USER_AUTH_TYPE_PIN },
            { .tag = HKS_TAG_CHALLENGE_TYPE, .uint32Param = HKS_CHALLENGE_TYPE_NORMAL },
        },
    .initParams =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_ED25519 },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_VERIFY },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_CURVE25519_KEY_SIZE_256 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA1 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CBC },
            { .tag = HKS_TAG_USER_AUTH_TYPE, .uint32Param = HKS_USER_AUTH_TYPE_PIN },
            { .tag = HKS_TAG_CHALLENGE_TYPE, .uint32Param = HKS_CHALLENGE_TYPE_NORMAL },
        },
    .initResult = HKS_ERROR_INVALID_ARGUMENT
};

/* 008: gen hkdf for derive; init for derive */
const TestAuthCaseParams HKS_AUTH_TEST_008_PARAMS = {
    .genParams =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_AES },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_DERIVE },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_128 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA256 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CBC },
            { .tag = HKS_TAG_USER_AUTH_TYPE, .uint32Param = HKS_USER_AUTH_TYPE_PIN | HKS_USER_AUTH_TYPE_FINGERPRINT },
            { .tag = HKS_TAG_KEY_AUTH_ACCESS_TYPE, .uint32Param = HKS_AUTH_ACCESS_INVALID_CLEAR_PASSWORD },
            { .tag = HKS_TAG_CHALLENGE_TYPE, .uint32Param = HKS_CHALLENGE_TYPE_NORMAL },
        },
    .initParams =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_HKDF },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_AGREE },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_256 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA256 },
            { .tag = HKS_TAG_DERIVE_KEY_SIZE, .uint32Param = 32 },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CBC },
            { .tag = HKS_TAG_USER_AUTH_TYPE, .uint32Param = HKS_USER_AUTH_TYPE_PIN | HKS_USER_AUTH_TYPE_FINGERPRINT },
            { .tag = HKS_TAG_KEY_AUTH_ACCESS_TYPE, .uint32Param = HKS_AUTH_ACCESS_INVALID_CLEAR_PASSWORD },
            { .tag = HKS_TAG_CHALLENGE_TYPE, .uint32Param = HKS_CHALLENGE_TYPE_NORMAL },
        },
    .initResult = HKS_SUCCESS
};

static int32_t getParamSetAuthTest(struct HksParamSet **paramOutSet, const struct HksParamSet *genParamSet)
{
    struct HksParam localSecureKey = {
        .tag = HKS_TAG_SYMMETRIC_KEY_DATA,
        .blob = { .size = KEY_PARAMSET_SIZE, .data = (uint8_t *)HksMalloc(KEY_PARAMSET_SIZE) }
    };
    if (localSecureKey.blob.data == nullptr) {
        return HKS_ERROR_MALLOC_FAIL;
    }

    HksInitParamSet(paramOutSet);
    struct HksParam *algParam = nullptr;
    HksGetParam(genParamSet, HKS_TAG_ALGORITHM, &algParam);
    if (algParam->uint32Param == HKS_ALG_AES || algParam->uint32Param == HKS_ALG_SM4) {
        localSecureKey.tag = HKS_TAG_SYMMETRIC_KEY_DATA;
    } else if (algParam->uint32Param == HKS_ALG_RSA || algParam->uint32Param == HKS_ALG_SM2 ||
    algParam->uint32Param == HKS_ALG_ECC || algParam->uint32Param == HKS_ALG_DSA ||
    algParam->uint32Param == HKS_ALG_X25519 || algParam->uint32Param == HKS_ALG_ED25519 ||
    algParam->uint32Param == HKS_ALG_DH) {
        localSecureKey.tag = HKS_TAG_ASYMMETRIC_PUBLIC_KEY_DATA;
    } else if (algParam->uint32Param == HKS_ALG_HMAC || algParam->uint32Param == HKS_ALG_SM3 ||
     algParam->uint32Param == HKS_ALG_HKDF || algParam->uint32Param == HKS_ALG_PBKDF2) {
        localSecureKey.tag = HKS_TAG_ASYMMETRIC_PRIVATE_KEY_DATA;
    }
    HksAddParams(*paramOutSet, &localSecureKey, 1);
    HksBuildParamSet(paramOutSet);
 
    return HKS_SUCCESS;
}

static int32_t CheckGenAuthTest(const TestAuthCaseParams &testCaseParams)
{
    struct HksParamSet *genParamSet = nullptr;
    int32_t ret = InitParamSet(&genParamSet, testCaseParams.genParams.data(), testCaseParams.genParams.size());
    if (ret != HKS_SUCCESS) {
        HKS_LOG_I("InitParamSet(gen) failed, ret : %" LOG_PUBLIC "d", ret);
        return ret;
    }
    uint8_t alias[] = "testCheckAuth";
    struct HksBlob keyAlias = { sizeof(alias), alias };
    ret = HksGenerateKey(&keyAlias, genParamSet, nullptr);
    if (ret != HKS_SUCCESS) {
        HksFreeParamSet(&genParamSet);
        HKS_LOG_I("HksGenerateKey failed, ret : %" LOG_PUBLIC "d", ret);
        return ret;
    }

    struct HksParamSet *paramOutSet = nullptr;
    getParamSetAuthTest(&paramOutSet, genParamSet);

    ret = HksGetKeyParamSet(&keyAlias, genParamSet, paramOutSet);
    if (ret != HKS_SUCCESS) {
        return ret;
    }

    struct HksParam *secureParam = nullptr;
    ret = HksGetParam(paramOutSet, HKS_TAG_KEY_AUTH_ACCESS_TYPE, &secureParam);

    struct HksParam *userParam = nullptr;
    if (ret != HksGetParam(paramOutSet, HKS_TAG_USER_AUTH_TYPE, &userParam)) {
        return HKS_FAILURE;
    }

    HksFreeParamSet(&paramOutSet);
    HksFreeParamSet(&genParamSet);
    (void)HksDeleteKey(&keyAlias, nullptr);

    return (ret == testCaseParams.initResult) ? HKS_SUCCESS : HKS_FAILURE;
}

static int32_t CheckImportAuthTest(const TestAuthCaseParams &testCaseParams)
{
    struct HksParamSet *genParamSet = nullptr;
    struct HksParamSet *initParamSet = nullptr;
    int32_t ret = InitParamSet(&genParamSet, testCaseParams.genParams.data(), testCaseParams.genParams.size());
    if (ret != HKS_SUCCESS) {
        HKS_LOG_I("InitParamSet(gen) failed, ret : %" LOG_PUBLIC "d", ret);
        return ret;
    }
    
    uint8_t alias[] = "testCheckAuth";
    struct HksBlob keyAlias = { sizeof(alias), alias };
    ret = HksGenerateKey(&keyAlias, genParamSet, nullptr);
    if (ret != HKS_SUCCESS) {
        HksFreeParamSet(&genParamSet);
        HKS_LOG_I("HksGenerateKey failed, ret : %" LOG_PUBLIC "d", ret);
        return ret;
    }

    struct HksParamSet *paramOutSet = nullptr;
    getParamSetAuthTest(&paramOutSet, genParamSet);
    ret = InitParamSet(&initParamSet, testCaseParams.initParams.data(), testCaseParams.initParams.size());
    if (ret != HKS_SUCCESS) {
        HKS_LOG_I("InitParamSet(gen) failed, ret : %" LOG_PUBLIC "d", ret);
        return ret;
    }
    char tmpKey[] = "NewKeyAlias";
    struct HksBlob newKeyAlias = { .size = strlen(tmpKey), .data = (uint8_t *)tmpKey };
    uint8_t tmpPublicKey[HKS_RSA_KEY_SIZE_1024] = {0};
    struct HksBlob publicKey = { HKS_RSA_KEY_SIZE_1024, (uint8_t *)tmpPublicKey };
    ret = HksExportPublicKey(&keyAlias, genParamSet, &publicKey);
    ret = HksImportKey(&newKeyAlias, initParamSet, &publicKey);

    ret = HksGetKeyParamSet(&newKeyAlias, initParamSet, paramOutSet);
    if (ret != HKS_SUCCESS) {
        return ret;
    }
    struct HksParam *secureParam = nullptr;
    ret = HksGetParam(paramOutSet, HKS_TAG_KEY_AUTH_ACCESS_TYPE, &secureParam);
    struct HksParam *userParam = nullptr;
    if (ret != HksGetParam(paramOutSet, HKS_TAG_USER_AUTH_TYPE, &userParam)) {
        return HKS_FAILURE;
    }

    HksFreeParamSet(&paramOutSet);
    HksFreeParamSet(&genParamSet);
    (void)HksDeleteKey(&keyAlias, nullptr);
    return (ret == testCaseParams.initResult) ? HKS_SUCCESS : HKS_FAILURE;
}


/**
 * @tc.name: HksCheckAuthTest.HksCheckPurposeTest001
 * @tc.desc: alg-AES gen-pur-Encrypt.
 * @tc.type: FUNC
 * @tc.result:HKS_SUCCESS
 * @tc.require: issueI5NY0M
 */
HWTEST_F(HksCheckAuthPartTest, HksCheckAuthTest001, TestSize.Level0)
{
    HKS_LOG_I("Enter HksCheckAuthTest001");
    EXPECT_EQ(CheckGenAuthTest(HKS_AUTH_TEST_001_PARAMS), HKS_SUCCESS);
}

/**
 * @tc.name: HksCheckAuthTest.HksCheckPurposeTest002
 * @tc.desc: alg-SM4 gen-pur-Encrypt.
 * @tc.type: FUNC
 * @tc.result:HKS_SUCCESS
 * @tc.require: issueI5NY0M
 */
HWTEST_F(HksCheckAuthPartTest, HksCheckAuthTest002, TestSize.Level0)
{
#ifdef _USE_OPENSSL_
    /* mbedtls engine don't support SM4 alg */
    HKS_LOG_I("Enter HksCheckAuthTest002");
    EXPECT_EQ(CheckGenAuthTest(HKS_AUTH_TEST_002_PARAMS), HKS_SUCCESS);
#endif
}

/**
 * @tc.name: HksCheckAuthTest.HksCheckPurposeTest003
 * @tc.desc: alg-HMAC gen-pur-Mac.
 * @tc.type: FUNC
 * @tc.result:HKS_SUCCESS
 * @tc.require: issueI5NY0M
 */
HWTEST_F(HksCheckAuthPartTest, HksCheckAuthTest003, TestSize.Level0)
{
    HKS_LOG_I("Enter HksCheckAuthTest003");
    EXPECT_EQ(CheckGenAuthTest(HKS_AUTH_TEST_003_PARAMS), HKS_SUCCESS);
}

/**
 * @tc.name: HksCheckAuthTest.HksCheckPurposeTest004
 * @tc.desc: alg-SM3 gen-pur-mac.
 * @tc.type: FUNC
 * @tc.result:HKS_ERROR_INVALID_ARGUMENT
 * @tc.require: issueI5NY0M
 */
HWTEST_F(HksCheckAuthPartTest, HksCheckAuthTest004, TestSize.Level0)
{
    HKS_LOG_I("Enter HksCheckAuthTest004");
    EXPECT_EQ(CheckGenAuthTest(HKS_AUTH_TEST_004_PARAMS), HKS_ERROR_CHECK_GET_ACCESS_TYPE_FAILED);
}

/**
 * @tc.name: HksCheckAuthTest.HksCheckPurposeTest005
 * @tc.desc: alg-X25519 gen-pur-Agree with fingerprint and INVALID_CLEAR_PASSWORD.
 * @tc.type: FUNC
 * @tc.result:HKS_SUCCESS
 * @tc.require: issueI5NY0M
 */
HWTEST_F(HksCheckAuthPartTest, HksCheckAuthTest005, TestSize.Level0)
{
    HKS_LOG_I("Enter HksCheckAuthTest005");
    EXPECT_EQ(CheckImportAuthTest(HKS_AUTH_TEST_005_PARAMS), HKS_SUCCESS);
}

/**
 * @tc.name: HksCheckAuthTest.HksCheckPurposeTest006
 * @tc.desc: alg-ECC gen-pur-Sign.
 * @tc.type: FUNC
 * @tc.result:HKS_ERROR_INVALID_ARGUMENT
 * @tc.require: issueI5NY0M
 */
HWTEST_F(HksCheckAuthPartTest, HksCheckAuthTest006, TestSize.Level0)
{
    HKS_LOG_I("Enter HksCheckAuthTest006");
    EXPECT_EQ(CheckImportAuthTest(HKS_AUTH_TEST_006_PARAMS), HKS_ERROR_INVALID_ARGUMENT);
}

/**
 * @tc.name: HksCheckAuthTest.HksCheckPurposeTest009
 * @tc.desc: alg-ED25519 gen-pur-Sign.
 * @tc.type: FUNC
 * @tc.result:HKS_ERROR_INVALID_ARGUMENT
 * @tc.require: issueI5NY0M
 */
HWTEST_F(HksCheckAuthPartTest, HksCheckAuthTest007, TestSize.Level0)
{
    HKS_LOG_I("Enter HksCheckAuthTest007");
    EXPECT_EQ(CheckImportAuthTest(HKS_AUTH_TEST_007_PARAMS), HKS_ERROR_CHECK_GET_ACCESS_TYPE_FAILED);
}

/**
 * @tc.name: HksCheckAuthTest.HksCheckPurposeTest008
 * @tc.desc: alg-HKDF gen-pur-Derive.
 * @tc.type: FUNC
 * @tc.result:HKS_SUCCESS
 * @tc.require: issueI5NY0M
 */
HWTEST_F(HksCheckAuthPartTest, HksCheckAuthTest008, TestSize.Level0)
{
    HKS_LOG_I("Enter HksCheckAuthTest008");
    EXPECT_EQ(CheckGenAuthTest(HKS_AUTH_TEST_008_PARAMS), HKS_SUCCESS);
}
}
