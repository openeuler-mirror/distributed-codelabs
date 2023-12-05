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

#include "hks_import_key_test.h"

#include <gtest/gtest.h>

using namespace testing::ext;
namespace Unittest::ImportKeyTest {
class HksImportKeyTest : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};

void HksImportKeyTest::SetUpTestCase(void)
{
}

void HksImportKeyTest::TearDownTestCase(void)
{
}

void HksImportKeyTest::SetUp()
{
    EXPECT_EQ(HksInitialize(), 0);
}

void HksImportKeyTest::TearDown()
{
}

struct ImportKeyCaseParams {
    std::vector<HksParam> params;
    uint32_t keySize = 0;
    HksErrorCode importKeyResult = HksErrorCode::HKS_SUCCESS;
};

/* 001: ase-128-encrypt-decrypt-gcm-none */
const ImportKeyCaseParams HKS_IMPORT_TEST_001_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_AES },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_128 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_GCM },
        },
    .keySize = HKS_AES_KEY_SIZE_128 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_SUCCESS,
};

/* 002: ase-192-encrypt-decrypt-cbc-none */
const ImportKeyCaseParams HKS_IMPORT_TEST_002_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_AES },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_192 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CBC },
        },
    .keySize = HKS_AES_KEY_SIZE_192 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_SUCCESS,
};

/* 003: ase-256-encrypt-decrypt-cbc-pkcs7 */
const ImportKeyCaseParams HKS_IMPORT_TEST_003_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_AES },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_256 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_PKCS7 },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CBC },
        },
    .keySize = HKS_AES_KEY_SIZE_256 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_SUCCESS,
};

/* 004: ase-256-encrypt-decrypt-ecb-none */
const ImportKeyCaseParams HKS_IMPORT_TEST_004_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_AES },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_256 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_ECB },
        },
    .keySize = HKS_AES_KEY_SIZE_256 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_SUCCESS,
};

/* 005: ase-256-encrypt-decrypt-ecb-pkcs7 */
const ImportKeyCaseParams HKS_IMPORT_TEST_005_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_AES },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_256 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_PKCS7 },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_ECB },
        },
    .keySize = HKS_AES_KEY_SIZE_256 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_SUCCESS,
};

/* 006: ase-256-encrypt-decrypt-ctr-none */
const ImportKeyCaseParams HKS_IMPORT_TEST_006_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_AES },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_256 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CTR },
        },
    .keySize = HKS_AES_KEY_SIZE_256 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_SUCCESS,
};

/* 007: ase-256-derive-sha256 */
const ImportKeyCaseParams HKS_IMPORT_TEST_007_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_AES },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_DERIVE },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_256 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA256 },
        },
    .keySize = HKS_AES_KEY_SIZE_256 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_SUCCESS,
};

/* 008: hmac-256-sha1 */
const ImportKeyCaseParams HKS_IMPORT_TEST_008_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_HMAC },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_MAC },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_256 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA1 },
        },
    .keySize = HKS_AES_KEY_SIZE_256 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_SUCCESS,
};

/* 009: hmac-256-sha224 */
const ImportKeyCaseParams HKS_IMPORT_TEST_009_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_HMAC },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_MAC },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_256 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA224 },
        },
    .keySize = HKS_AES_KEY_SIZE_256 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_SUCCESS,
};

/* 010: hmac-256-sha256 */
const ImportKeyCaseParams HKS_IMPORT_TEST_010_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_HMAC },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_MAC },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_256 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA256 },
        },
    .keySize = HKS_AES_KEY_SIZE_256 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_SUCCESS,
};

/* 011: hmac-256-sha384 */
const ImportKeyCaseParams HKS_IMPORT_TEST_011_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_HMAC },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_MAC },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_256 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA384 },
        },
    .keySize = HKS_AES_KEY_SIZE_256 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_SUCCESS,
};

/* 012: hmac-128-sha512 */
const ImportKeyCaseParams HKS_IMPORT_TEST_012_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_HMAC },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_MAC },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_128 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA512 },
        },
    .keySize = HKS_AES_KEY_SIZE_128 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_SUCCESS,
};

/* 013: sm3-128-sm3 */
const ImportKeyCaseParams HKS_IMPORT_TEST_013_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_SM3 },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_MAC },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_128 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SM3 },
        },
    .keySize = HKS_AES_KEY_SIZE_128 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_SUCCESS,
};

/* 014: sm3-256-sm3 */
const ImportKeyCaseParams HKS_IMPORT_TEST_014_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_SM3 },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_MAC },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_256 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SM3 },
        },
    .keySize = HKS_AES_KEY_SIZE_256 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_SUCCESS,
};

/* 015: sm4-128-cbc-none */
const ImportKeyCaseParams HKS_IMPORT_TEST_015_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_SM4 },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_SM4_KEY_SIZE_128 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CBC },
        },
    .keySize = HKS_SM4_KEY_SIZE_128 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_SUCCESS,
};

/* 016: sm4-128-cbc-pkcs7 */
const ImportKeyCaseParams HKS_IMPORT_TEST_016_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_SM4 },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_SM4_KEY_SIZE_128 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_PKCS7 },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CBC },
        },
    .keySize = HKS_SM4_KEY_SIZE_128 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_SUCCESS,
};

/* 017: sm4-128-ecb-none */
const ImportKeyCaseParams HKS_IMPORT_TEST_017_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_SM4 },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_SM4_KEY_SIZE_128 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_ECB },
        },
    .keySize = HKS_SM4_KEY_SIZE_128 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_SUCCESS,
};

/* 018: sm4-128-ecb-pkcs7 */
const ImportKeyCaseParams HKS_IMPORT_TEST_018_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_SM4 },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_SM4_KEY_SIZE_128 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_PKCS7 },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_ECB },
        },
    .keySize = HKS_SM4_KEY_SIZE_128 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_SUCCESS,
};

/* 019: sm4-128-ctr-none */
const ImportKeyCaseParams HKS_IMPORT_TEST_019_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_SM4 },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_SM4_KEY_SIZE_128 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CTR },
        },
    .keySize = HKS_SM4_KEY_SIZE_128 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_SUCCESS,
};

/* 020: sm4-128-ctr-none */
const ImportKeyCaseParams HKS_IMPORT_TEST_020_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_SM4 },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_SM4_KEY_SIZE_128 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CTR },
        },
    .keySize = HKS_SM4_KEY_SIZE_128 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_SUCCESS,
};

/* 021: aes-invalid-keysize */
const ImportKeyCaseParams HKS_IMPORT_TEST_021_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_AES },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_256 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CBC },
        },
    .keySize = HKS_AES_KEY_SIZE_256,
    .importKeyResult = HKS_ERROR_INVALID_KEY_INFO,
};

/* 022: hmac-invalid-keysize */
const ImportKeyCaseParams HKS_IMPORT_TEST_022_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_HMAC },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_MAC },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_128 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA256 },
        },
    .keySize = HKS_AES_KEY_SIZE_256 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_ERROR_INVALID_KEY_INFO,
};

/* 023: sm3-invalid-keysize */
const ImportKeyCaseParams HKS_IMPORT_TEST_023_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_SM3 },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_MAC },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_128 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SM3 },
        },
    .keySize = HKS_AES_KEY_SIZE_256 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_ERROR_INVALID_KEY_INFO,
};

/* 024: sm4-invalid-keysize */
const ImportKeyCaseParams HKS_IMPORT_TEST_024_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_SM4 },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_SM4_KEY_SIZE_128 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CBC },
        },
    .keySize = HKS_AES_KEY_SIZE_256 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_ERROR_INVALID_KEY_INFO,
};

/* 025: sm3-invalid-digest */
const ImportKeyCaseParams HKS_IMPORT_TEST_025_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_SM3 },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_MAC },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_128 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA256 },
        },
    .keySize = HKS_AES_KEY_SIZE_128 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_ERROR_INVALID_DIGEST,
};

/* 026: sm4-invalid-param-keysize */
const ImportKeyCaseParams HKS_IMPORT_TEST_026_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_SM4 },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_256 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CBC },
        },
    .keySize = HKS_SM4_KEY_SIZE_128 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_ERROR_INVALID_KEY_SIZE,
};

/* 027: sm4-invalid-mode-gcm */
const ImportKeyCaseParams HKS_IMPORT_TEST_027_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_SM4 },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_SM4_KEY_SIZE_128 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_GCM },
        },
    .keySize = HKS_SM4_KEY_SIZE_128 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_ERROR_INVALID_MODE,
};

/* 028: sm4-invalid-padding-ctr-pcks7 */
const ImportKeyCaseParams HKS_IMPORT_TEST_028_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_SM4 },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_SM4_KEY_SIZE_128 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_PKCS7 },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CTR },
        },
    .keySize = HKS_SM4_KEY_SIZE_128 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_ERROR_INVALID_PADDING,
};

/* 029: sm4-invalid-purpose */
const ImportKeyCaseParams HKS_IMPORT_TEST_029_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_SM4 },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_MAC },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_SM4_KEY_SIZE_128 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CTR },
        },
    .keySize = HKS_SM4_KEY_SIZE_128 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_ERROR_INVALID_PURPOSE,
};

/* 030: sm3-invalid-purpose */
const ImportKeyCaseParams HKS_IMPORT_TEST_030_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_SM3 },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_AGREE },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_SM4_KEY_SIZE_128 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SM3 },
        },
    .keySize = HKS_SM4_KEY_SIZE_128 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_ERROR_INVALID_PURPOSE,
};

/* 031: rsa-invalid-keysize */
const ImportKeyCaseParams HKS_IMPORT_TEST_031_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_RSA },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_SIGN },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_RSA_KEY_SIZE_2048 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_PSS },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA256 },
            { .tag = HKS_TAG_IMPORT_KEY_TYPE, .uint32Param = HKS_KEY_TYPE_PRIVATE_KEY },
        },
    .keySize = HKS_RSA_KEY_SIZE_2048 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_ERROR_INVALID_KEY_INFO,
};

/* 032: rsa-invalid-keysize2 */
const ImportKeyCaseParams HKS_IMPORT_TEST_032_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_RSA },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_SIGN },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_RSA_KEY_SIZE_2048 },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_PSS },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA256 },
            { .tag = HKS_TAG_IMPORT_KEY_TYPE, .uint32Param = HKS_KEY_TYPE_KEY_PAIR },
        },
    .keySize = HKS_AES_KEY_SIZE_128 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_ERROR_INVALID_KEY_INFO,
};

/* 033: ecc-invalid-keysize */
const ImportKeyCaseParams HKS_IMPORT_TEST_033_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_ECC },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_SIGN },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_ECC_KEY_SIZE_256 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA256 },
            { .tag = HKS_TAG_IMPORT_KEY_TYPE, .uint32Param = HKS_KEY_TYPE_PRIVATE_KEY },
        },
    .keySize = HKS_ECC_KEY_SIZE_256 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_ERROR_INVALID_KEY_INFO,
};

/* 034: ecc-invalid-keysize2 */
const ImportKeyCaseParams HKS_IMPORT_TEST_034_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_ECC },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_SIGN },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_ECC_KEY_SIZE_256 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA256 },
            { .tag = HKS_TAG_IMPORT_KEY_TYPE, .uint32Param = HKS_KEY_TYPE_KEY_PAIR },
        },
    .keySize = HKS_AES_KEY_SIZE_128 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_ERROR_INVALID_KEY_INFO,
};

#ifdef _USE_OPENSSL_
/* mbedtls engine don't support DSA alg */
/* 035: dsa-invalid-keysize */
const ImportKeyCaseParams HKS_IMPORT_TEST_035_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_DSA },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_SIGN },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_RSA_KEY_SIZE_2048 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA256 },
            { .tag = HKS_TAG_IMPORT_KEY_TYPE, .uint32Param = HKS_KEY_TYPE_PRIVATE_KEY },
        },
    .keySize = HKS_RSA_KEY_SIZE_2048 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_ERROR_INVALID_KEY_INFO,
};

/* mbedtls engine don't support DSA alg */
/* 036: dsa-invalid-keysize2 */
const ImportKeyCaseParams HKS_IMPORT_TEST_036_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_DSA },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_SIGN },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_RSA_KEY_SIZE_2048 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA256 },
            { .tag = HKS_TAG_IMPORT_KEY_TYPE, .uint32Param = HKS_KEY_TYPE_KEY_PAIR },
        },
    .keySize = HKS_AES_KEY_SIZE_128 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_ERROR_INVALID_KEY_INFO,
};
#endif

/* 037: x25519-invalid-keysize2 */
const ImportKeyCaseParams HKS_IMPORT_TEST_037_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_X25519 },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_AGREE },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_CURVE25519_KEY_SIZE_256 },
            { .tag = HKS_TAG_IMPORT_KEY_TYPE, .uint32Param = HKS_KEY_TYPE_PRIVATE_KEY },
        },
    .keySize = HKS_AES_KEY_SIZE_128 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_ERROR_INVALID_KEY_INFO,
};

/* 038: x25519-invalid-keysize2 */
const ImportKeyCaseParams HKS_IMPORT_TEST_038_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_X25519 },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_AGREE },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_CURVE25519_KEY_SIZE_256 },
            { .tag = HKS_TAG_IMPORT_KEY_TYPE, .uint32Param = HKS_KEY_TYPE_KEY_PAIR },
        },
    .keySize = HKS_CURVE25519_KEY_SIZE_256 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_ERROR_INVALID_KEY_INFO,
};

/* 039: ed25519-invalid-keysize2 */
const ImportKeyCaseParams HKS_IMPORT_TEST_039_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_ED25519 },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_SIGN },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_CURVE25519_KEY_SIZE_256 },
            { .tag = HKS_TAG_IMPORT_KEY_TYPE, .uint32Param = HKS_KEY_TYPE_PRIVATE_KEY },
        },
    .keySize = HKS_AES_KEY_SIZE_128 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_ERROR_INVALID_KEY_INFO,
};

/* 040: ed25519-invalid-keysize2 */
const ImportKeyCaseParams HKS_IMPORT_TEST_040_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_ED25519 },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_SIGN },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_CURVE25519_KEY_SIZE_256 },
            { .tag = HKS_TAG_IMPORT_KEY_TYPE, .uint32Param = HKS_KEY_TYPE_KEY_PAIR },
        },
    .keySize = HKS_CURVE25519_KEY_SIZE_256 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_ERROR_INVALID_KEY_INFO,
};

/* 041: dh-invalid-keysize2 */
const ImportKeyCaseParams HKS_IMPORT_TEST_041_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_DH },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_AGREE },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_DH_KEY_SIZE_2048 },
            { .tag = HKS_TAG_IMPORT_KEY_TYPE, .uint32Param = HKS_KEY_TYPE_PRIVATE_KEY },
        },
    .keySize = HKS_DH_KEY_SIZE_2048 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_ERROR_INVALID_KEY_INFO,
};

/* 042: dh-invalid-keysize2 */
const ImportKeyCaseParams HKS_IMPORT_TEST_042_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_DH },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_AGREE },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_DH_KEY_SIZE_2048 },
            { .tag = HKS_TAG_IMPORT_KEY_TYPE, .uint32Param = HKS_KEY_TYPE_KEY_PAIR },
        },
    .keySize = HKS_DH_KEY_SIZE_2048 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_ERROR_INVALID_KEY_INFO,
};

/* 043: sm2-invalid-keysize2 */
const ImportKeyCaseParams HKS_IMPORT_TEST_043_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_SM2 },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_SIGN },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_SM2_KEY_SIZE_256 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SM3 },
            { .tag = HKS_TAG_IMPORT_KEY_TYPE, .uint32Param = HKS_KEY_TYPE_PRIVATE_KEY },
        },
    .keySize = HKS_SM2_KEY_SIZE_256 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_ERROR_INVALID_KEY_INFO,
};

/* 044: sm2-invalid-keysize2 */
const ImportKeyCaseParams HKS_IMPORT_TEST_044_PARAMS = {
    .params =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_SM2 },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_SIGN },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_SM2_KEY_SIZE_256 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SM3 },
            { .tag = HKS_TAG_IMPORT_KEY_TYPE, .uint32Param = HKS_KEY_TYPE_KEY_PAIR },
        },
    .keySize = HKS_SM2_KEY_SIZE_256 / HKS_BITS_PER_BYTE,
    .importKeyResult = HKS_ERROR_INVALID_KEY_INFO,
};

static int32_t ImportTest(const ImportKeyCaseParams &testCaseParams)
{
    struct HksParamSet *importParamSet = nullptr;
    int32_t ret = InitParamSet(&importParamSet, testCaseParams.params.data(), testCaseParams.params.size());
    if (ret != HKS_SUCCESS) {
        return ret;
    }

    uint8_t *keyData = (uint8_t *)HksMalloc(testCaseParams.keySize);
    if (keyData == nullptr) {
        HksFreeParamSet(&importParamSet);
        return HKS_ERROR_MALLOC_FAIL;
    }
    struct HksBlob key = { testCaseParams.keySize, keyData };
    (void)HksGenerateRandom(nullptr, &key);

    uint8_t alias[] = "test_import_key";
    struct HksBlob keyAlias = { sizeof(alias), alias };
    ret = HksImportKey(&keyAlias, importParamSet, &key);
    HksFreeParamSet(&importParamSet);
    HksFree(key.data);
    EXPECT_EQ(ret, testCaseParams.importKeyResult);
    if (ret == HKS_SUCCESS) {
        (void)HksDeleteKey(&keyAlias, nullptr);
    }

    return (ret == testCaseParams.importKeyResult) ? HKS_SUCCESS : HKS_FAILURE;
}

HWTEST_F(HksImportKeyTest, HksImportKeyTest001, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_001_PARAMS), HKS_SUCCESS);
}

HWTEST_F(HksImportKeyTest, HksImportKeyTest002, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_002_PARAMS), HKS_SUCCESS);
}

HWTEST_F(HksImportKeyTest, HksImportKeyTest003, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_003_PARAMS), HKS_SUCCESS);
}

HWTEST_F(HksImportKeyTest, HksImportKeyTest004, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_004_PARAMS), HKS_SUCCESS);
}

HWTEST_F(HksImportKeyTest, HksImportKeyTest005, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_005_PARAMS), HKS_SUCCESS);
}

HWTEST_F(HksImportKeyTest, HksImportKeyTest006, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_006_PARAMS), HKS_SUCCESS);
}

HWTEST_F(HksImportKeyTest, HksImportKeyTest007, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_007_PARAMS), HKS_SUCCESS);
}

HWTEST_F(HksImportKeyTest, HksImportKeyTest008, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_008_PARAMS), HKS_SUCCESS);
}

HWTEST_F(HksImportKeyTest, HksImportKeyTest009, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_009_PARAMS), HKS_SUCCESS);
}

HWTEST_F(HksImportKeyTest, HksImportKeyTest010, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_010_PARAMS), HKS_SUCCESS);
}

HWTEST_F(HksImportKeyTest, HksImportKeyTest011, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_011_PARAMS), HKS_SUCCESS);
}

HWTEST_F(HksImportKeyTest, HksImportKeyTest012, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_012_PARAMS), HKS_SUCCESS);
}

HWTEST_F(HksImportKeyTest, HksImportKeyTest013, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_013_PARAMS), HKS_SUCCESS);
}

HWTEST_F(HksImportKeyTest, HksImportKeyTest014, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_014_PARAMS), HKS_SUCCESS);
}

HWTEST_F(HksImportKeyTest, HksImportKeyTest015, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_015_PARAMS), HKS_SUCCESS);
}

HWTEST_F(HksImportKeyTest, HksImportKeyTest016, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_016_PARAMS), HKS_SUCCESS);
}

HWTEST_F(HksImportKeyTest, HksImportKeyTest017, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_017_PARAMS), HKS_SUCCESS);
}

HWTEST_F(HksImportKeyTest, HksImportKeyTest018, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_018_PARAMS), HKS_SUCCESS);
}

HWTEST_F(HksImportKeyTest, HksImportKeyTest019, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_019_PARAMS), HKS_SUCCESS);
}

HWTEST_F(HksImportKeyTest, HksImportKeyTest020, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_020_PARAMS), HKS_SUCCESS);
}

HWTEST_F(HksImportKeyTest, HksImportKeyTest021, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_021_PARAMS), HKS_SUCCESS);
}

HWTEST_F(HksImportKeyTest, HksImportKeyTest022, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_022_PARAMS), HKS_SUCCESS);
}

HWTEST_F(HksImportKeyTest, HksImportKeyTest023, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_023_PARAMS), HKS_SUCCESS);
}

HWTEST_F(HksImportKeyTest, HksImportKeyTest024, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_024_PARAMS), HKS_SUCCESS);
}

HWTEST_F(HksImportKeyTest, HksImportKeyTest025, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_025_PARAMS), HKS_SUCCESS);
}

HWTEST_F(HksImportKeyTest, HksImportKeyTest026, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_026_PARAMS), HKS_SUCCESS);
}

HWTEST_F(HksImportKeyTest, HksImportKeyTest027, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_027_PARAMS), HKS_SUCCESS);
}

HWTEST_F(HksImportKeyTest, HksImportKeyTest028, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_028_PARAMS), HKS_SUCCESS);
}

HWTEST_F(HksImportKeyTest, HksImportKeyTest029, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_029_PARAMS), HKS_SUCCESS);
}

HWTEST_F(HksImportKeyTest, HksImportKeyTest030, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_030_PARAMS), HKS_SUCCESS);
}

HWTEST_F(HksImportKeyTest, HksImportKeyTest031, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_031_PARAMS), HKS_SUCCESS);
}

HWTEST_F(HksImportKeyTest, HksImportKeyTest032, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_032_PARAMS), HKS_SUCCESS);
}

HWTEST_F(HksImportKeyTest, HksImportKeyTest033, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_023_PARAMS), HKS_SUCCESS);
}

HWTEST_F(HksImportKeyTest, HksImportKeyTest034, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_034_PARAMS), HKS_SUCCESS);
}

#ifdef _USE_OPENSSL_
HWTEST_F(HksImportKeyTest, HksImportKeyTest035, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_035_PARAMS), HKS_SUCCESS);
}

HWTEST_F(HksImportKeyTest, HksImportKeyTest036, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_036_PARAMS), HKS_SUCCESS);
}

HWTEST_F(HksImportKeyTest, HksImportKeyTest037, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_037_PARAMS), HKS_SUCCESS);
}
#endif

HWTEST_F(HksImportKeyTest, HksImportKeyTest038, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_038_PARAMS), HKS_SUCCESS);
}

HWTEST_F(HksImportKeyTest, HksImportKeyTest039, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_029_PARAMS), HKS_SUCCESS);
}

HWTEST_F(HksImportKeyTest, HksImportKeyTest040, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_040_PARAMS), HKS_SUCCESS);
}

HWTEST_F(HksImportKeyTest, HksImportKeyTest041, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_041_PARAMS), HKS_SUCCESS);
}

HWTEST_F(HksImportKeyTest, HksImportKeyTest042, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_042_PARAMS), HKS_SUCCESS);
}

HWTEST_F(HksImportKeyTest, HksImportKeyTest043, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_043_PARAMS), HKS_SUCCESS);
}

HWTEST_F(HksImportKeyTest, HksImportKeyTest044, TestSize.Level0)
{
    EXPECT_EQ(ImportTest(HKS_IMPORT_TEST_044_PARAMS), HKS_SUCCESS);
}
} // namespace Unittest::ImportKeyTest
