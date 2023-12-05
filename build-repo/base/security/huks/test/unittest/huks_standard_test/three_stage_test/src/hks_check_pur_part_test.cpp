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

#include "hks_check_pur_part_test.h"

#include <gtest/gtest.h>

using namespace testing::ext;
namespace Unittest::CheckPurposeTest {
class HksCheckPurPartTest : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};

void HksCheckPurPartTest::SetUpTestCase(void)
{
}

void HksCheckPurPartTest::TearDownTestCase(void)
{
}

void HksCheckPurPartTest::SetUp()
{
    EXPECT_EQ(HksInitialize(), 0);
}

void HksCheckPurPartTest::TearDown()
{
}

/* 001: gen rsa for cipher; init for sign */
const TestPurposeCaseParams HKS_PURPOE_TEST_001_PARAMS = {
    .genParams =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_RSA },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_RSA_KEY_SIZE_1024 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA256 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_OAEP },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_ECB },
        },
    .initParams =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_RSA },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_SIGN },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_RSA_KEY_SIZE_1024 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA256 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_OAEP },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_ECB },
        },
    .initResult = HKS_ERROR_INVALID_ARGUMENT
};

/* 002: gen hmac for hmac; init for sign */
const TestPurposeCaseParams HKS_PURPOE_TEST_002_PARAMS = {
    .genParams =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_HMAC },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_MAC },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_CURVE25519_KEY_SIZE_256 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA1 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CBC },
        },
    .initParams =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_HMAC },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_SIGN },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_CURVE25519_KEY_SIZE_256 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA1 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CBC },
        },
    .initResult = HKS_ERROR_INVALID_ALGORITHM
};

/* 003: gen aes for derive; init for cipher */
const TestPurposeCaseParams HKS_PURPOE_TEST_003_PARAMS = {
    .genParams =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_AES },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_DERIVE },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_128 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA512 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CBC },
        },
    .initParams =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_AES },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_128 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA512 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CBC },
        },
    .initResult = HKS_ERROR_INVALID_ARGUMENT
};

#ifdef _USE_OPENSSL_
/* mbedtls engine don't support DSA alg */
/* 004: gen hmac for hmac; init for cipher */
const TestPurposeCaseParams HKS_PURPOE_TEST_004_PARAMS = {
    .genParams =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_DSA },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_SIGN | HKS_KEY_PURPOSE_VERIFY },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_RSA_KEY_SIZE_1024 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA1 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_PKCS1_V1_5 },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_ECB },
        },
    .initParams =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_DSA },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_RSA_KEY_SIZE_1024 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA1 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_PKCS1_V1_5 },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_ECB },
        },
    .initResult = HKS_ERROR_INVALID_ALGORITHM
};
#endif

/* 005: gen ecc for sign; init for agree */
const TestPurposeCaseParams HKS_PURPOE_TEST_005_PARAMS = {
    .genParams =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_ECC },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_SIGN | HKS_KEY_PURPOSE_VERIFY },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_ECC_KEY_SIZE_224 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CBC },
        },
    .initParams =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_ECDH },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_AGREE },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_ECC_KEY_SIZE_224 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA512 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CBC },
        },
    .initResult = HKS_ERROR_INVALID_ARGUMENT
};

#ifdef _USE_OPENSSL_
/* mbedtls engine don't support DSA alg */
/* 006: gen dsa for sign; init for agree */
const TestPurposeCaseParams HKS_PURPOE_TEST_006_PARAMS = {
    .genParams =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_DSA },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_SIGN | HKS_KEY_PURPOSE_VERIFY },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_RSA_KEY_SIZE_1024 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA1 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_PKCS1_V1_5 },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_ECB },
        },
    .initParams =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_DSA },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_AGREE },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_RSA_KEY_SIZE_1024 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA1 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_PKCS1_V1_5 },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_ECB },
        },
    .initResult = HKS_ERROR_INVALID_ALGORITHM
};
#endif

/* 007: gen hmac for mac; init for mac */
const TestPurposeCaseParams HKS_PURPOE_TEST_007_PARAMS = {
    .genParams =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_HMAC },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_MAC },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_CURVE25519_KEY_SIZE_256 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA1 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CBC },
        },
    .initParams =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_HMAC },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_MAC },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_CURVE25519_KEY_SIZE_256 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_MD5 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CBC },
        },
    .initResult = HKS_ERROR_INVALID_ARGUMENT
};

/* 008: gen aes for cipher; init for mac */
const TestPurposeCaseParams HKS_PURPOE_TEST_008_PARAMS = {
    .genParams =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_AES },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_256 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA1 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CBC },
        },
    .initParams =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_DSA },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_MAC },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_256 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA1 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CBC },
        },
    .initResult = HKS_ERROR_INVALID_ALGORITHM
};

static int32_t CheckPurposeTest(const TestPurposeCaseParams &testCaseParams)
{
    struct HksParamSet *genParamSet = nullptr;
    int32_t ret = InitParamSet(&genParamSet, testCaseParams.genParams.data(), testCaseParams.genParams.size());
    if (ret != HKS_SUCCESS) {
        HKS_LOG_I("InitParamSet(gen) failed, ret : %" LOG_PUBLIC "d", ret);
        return ret;
    }

    uint8_t alias[] = "testCheckPurpose";
    struct HksBlob keyAlias = { sizeof(alias), alias };
    ret = HksGenerateKey(&keyAlias, genParamSet, nullptr);
    if (ret != HKS_SUCCESS) {
        HksFreeParamSet(&genParamSet);
        HKS_LOG_I("HksGenerateKey failed, ret : %" LOG_PUBLIC "d", ret);
        return ret;
    }

    struct HksParamSet *initParamSet = nullptr;
    ret = InitParamSet(&initParamSet, testCaseParams.initParams.data(), testCaseParams.initParams.size());
    if (ret != HKS_SUCCESS) {
        HksFreeParamSet(&genParamSet);
        HKS_LOG_I("InitParamSet(init) failed, ret : %" LOG_PUBLIC "d", ret);
        return ret;
    }

    uint64_t handle = 0;
    struct HksBlob handleBlob = { sizeof(handle), (uint8_t *)&handle };
    ret = HksInit(&keyAlias, initParamSet, &handleBlob, nullptr);
    EXPECT_EQ(ret, testCaseParams.initResult);

    HksFreeParamSet(&genParamSet);
    HksFreeParamSet(&initParamSet);
    (void)HksDeleteKey(&keyAlias, nullptr);

    return (ret == testCaseParams.initResult) ? HKS_SUCCESS : HKS_FAILURE;
}

/**
 * @tc.name: HksCheckPurposeTest.HksCheckPurposeTest001
 * @tc.desc: alg-RSA gen-pur-Encrypt init-pur-Sign.
 * @tc.type: FUNC
 * @tc.result:HKS_ERROR_INVALID_ARGUMENT
 */
HWTEST_F(HksCheckPurPartTest, HksCheckPurTest001, TestSize.Level0)
{
    HKS_LOG_I("Enter HksCheckPurposeTest001");
    EXPECT_EQ(CheckPurposeTest(HKS_PURPOE_TEST_001_PARAMS), HKS_SUCCESS);
}

/**
 * @tc.name: HksAuthPartTest.HksCheckPurposeTest002
 * @tc.desc: alg-HMAC gen-pur-Mac init-pur-Sign.
 * @tc.type: FUNC
 * @tc.result:HKS_ERROR_INVALID_ALGORITHM
 */
HWTEST_F(HksCheckPurPartTest, HksCheckPurTest002, TestSize.Level0)
{
    HKS_LOG_I("Enter HksCheckPurposeTest002");
    EXPECT_EQ(CheckPurposeTest(HKS_PURPOE_TEST_002_PARAMS), HKS_SUCCESS);
}

/**
 * @tc.name: HksCheckPurposeTest.HksCheckPurposeTest003
 * @tc.desc: alg-AES gen-pur-Derive init-pur-Encrypt.
 * @tc.type: FUNC
 * @tc.result:HKS_ERROR_INVALID_ARGUMENT
 */
HWTEST_F(HksCheckPurPartTest, HksCheckPurTest003, TestSize.Level0)
{
    HKS_LOG_I("Enter HksCheckPurposeTest003");
    EXPECT_EQ(CheckPurposeTest(HKS_PURPOE_TEST_003_PARAMS), HKS_SUCCESS);
}

/**
 * @tc.name: HksAuthPartTest.HksCheckPurposeTest004
 * @tc.desc: alg-DSA gen-pur-Sign init-pur-Encrypt.
 * @tc.type: FUNC
 * @tc.result:HKS_ERROR_INVALID_ALGORITHM
 */
HWTEST_F(HksCheckPurPartTest, HksCheckPurTest004, TestSize.Level0)
{
#ifdef _USE_OPENSSL_
    /* mbedtls engine don't support DSA alg */
    HKS_LOG_I("Enter HksCheckPurposeTest004");
    EXPECT_EQ(CheckPurposeTest(HKS_PURPOE_TEST_004_PARAMS), HKS_SUCCESS);
#endif
}

/**
 * @tc.name: HksCheckPurposeTest.HksCheckPurposeTest005
 * @tc.desc: alg-ECC gen-pur-Sign init-pur-Agree.
 * @tc.type: FUNC
 * @tc.result:HKS_ERROR_INVALID_ARGUMENT
 */
HWTEST_F(HksCheckPurPartTest, HksCheckPurTest005, TestSize.Level0)
{
    HKS_LOG_I("Enter HksCheckPurposeTest005");
    EXPECT_EQ(CheckPurposeTest(HKS_PURPOE_TEST_005_PARAMS), HKS_SUCCESS);
}

/**
 * @tc.name: HksAuthPartTest.HksCheckPurposeTest006
 * @tc.desc: alg-DSA gen-pur-Sign init-pur-Agree.
 * @tc.type: FUNC
 * @tc.result:HKS_ERROR_INVALID_ALGORITHM
 */
HWTEST_F(HksCheckPurPartTest, HksCheckPurTest006, TestSize.Level0)
{
#ifdef _USE_OPENSSL_
    /* mbedtls engine don't support DSA alg */
    HKS_LOG_I("Enter HksCheckPurposeTest006");
    EXPECT_EQ(CheckPurposeTest(HKS_PURPOE_TEST_006_PARAMS), HKS_SUCCESS);
#endif
}

/**
 * @tc.name: HksCheckPurposeTest.HksCheckPurposeTest007
 * @tc.desc: alg-ECC gen-pur-Sign init-pur-Agree.
 * @tc.type: FUNC
 * @tc.result:HKS_ERROR_INVALID_ARGUMENT
 */
HWTEST_F(HksCheckPurPartTest, HksCheckPurTest007, TestSize.Level0)
{
    HKS_LOG_I("Enter HksCheckPurposeTest007");
    EXPECT_EQ(CheckPurposeTest(HKS_PURPOE_TEST_007_PARAMS), HKS_SUCCESS);
}

/**
 * @tc.name: HksAuthPartTest.HksCheckPurposeTest008
 * @tc.desc: alg-DSA gen-pur-Sign init-pur-Agree.
 * @tc.type: FUNC
 * @tc.result:HKS_ERROR_INVALID_ALGORITHM
 */
HWTEST_F(HksCheckPurPartTest, HksCheckPurTest008, TestSize.Level0)
{
    HKS_LOG_I("Enter HksCheckPurposeTest008");
    EXPECT_EQ(CheckPurposeTest(HKS_PURPOE_TEST_008_PARAMS), HKS_SUCCESS);
}
}

