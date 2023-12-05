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

#include "hks_refresh_key_info_test.h"

#include <gtest/gtest.h>
#include <unistd.h>

#include "hks_api.h"
#include "hks_log.h"
#include "hks_type.h"
#include "hks_param.h"
#include "native_huks_type.h"
#include "hks_errcode_adapter.h"

using namespace testing::ext;
namespace Unittest::HksNativeApiCompatibilityTest {
class HksNativeApiCompatibilityTest : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};

void HksNativeApiCompatibilityTest::SetUpTestCase(void)
{
}

void HksNativeApiCompatibilityTest::TearDownTestCase(void)
{
}

void HksNativeApiCompatibilityTest::SetUp()
{
}

void HksNativeApiCompatibilityTest::TearDown()
{
}

/**
 * @tc.name: HksNativeApiCompatibilityTest001
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:OH_Huks_KeyDigest equal to HksKeyDigest
 * @tc.type: FUNC
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest001, TestSize.Level0)
{
    ASSERT_EQ((uint32_t)OH_HUKS_DIGEST_NONE == (uint32_t)HKS_DIGEST_NONE, true);
    ASSERT_EQ((uint32_t)OH_HUKS_DIGEST_MD5 == (uint32_t)HKS_DIGEST_MD5, true);
    ASSERT_EQ((uint32_t)OH_HUKS_DIGEST_SM3 == (uint32_t)HKS_DIGEST_SM3, true);
    ASSERT_EQ((uint32_t)OH_HUKS_DIGEST_SHA1 == (uint32_t)HKS_DIGEST_SHA1, true);
    ASSERT_EQ((uint32_t)OH_HUKS_DIGEST_SHA224 == (uint32_t)HKS_DIGEST_SHA224, true);
    ASSERT_EQ((uint32_t)OH_HUKS_DIGEST_SHA256 == (uint32_t)HKS_DIGEST_SHA256, true);
    ASSERT_EQ((uint32_t)OH_HUKS_DIGEST_SHA384 == (uint32_t)HKS_DIGEST_SHA384, true);
    ASSERT_EQ((uint32_t)OH_HUKS_DIGEST_SHA512 == (uint32_t)HKS_DIGEST_SHA512, true);
}

/**
 * @tc.name: HksNativeApiCompatibilityTest002
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:OH_Huks_KeyDigest value compatibility
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest002, TestSize.Level0)
{
    ASSERT_EQ((uint32_t)OH_HUKS_DIGEST_NONE == 0, true);
    ASSERT_EQ((uint32_t)OH_HUKS_DIGEST_MD5 == 1, true);
    ASSERT_EQ((uint32_t)OH_HUKS_DIGEST_SM3 == 2, true);
    ASSERT_EQ((uint32_t)OH_HUKS_DIGEST_SHA1 == 10, true);
    ASSERT_EQ((uint32_t)OH_HUKS_DIGEST_SHA224 == 11, true);
    ASSERT_EQ((uint32_t)OH_HUKS_DIGEST_SHA256 == 12, true);
    ASSERT_EQ((uint32_t)OH_HUKS_DIGEST_SHA384 == 13, true);
    ASSERT_EQ((uint32_t)OH_HUKS_DIGEST_SHA512 == 14, true);
}

/**
 * @tc.name: HksNativeApiCompatibilityTest003
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:OH_Huks_KeyPadding equal to HksKeyPadding
 * @tc.type: FUNC
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest003, TestSize.Level0)
{
    ASSERT_EQ((uint32_t)OH_HUKS_PADDING_NONE == (uint32_t)HKS_PADDING_NONE, true);
    ASSERT_EQ((uint32_t)OH_HUKS_PADDING_OAEP == (uint32_t)HKS_PADDING_OAEP, true);
    ASSERT_EQ((uint32_t)OH_HUKS_PADDING_PSS == (uint32_t)HKS_PADDING_PSS, true);
    ASSERT_EQ((uint32_t)OH_HUKS_PADDING_PKCS1_V1_5 == (uint32_t)HKS_PADDING_PKCS1_V1_5, true);
    ASSERT_EQ((uint32_t)OH_HUKS_PADDING_PKCS5 == (uint32_t)HKS_PADDING_PKCS5, true);
    ASSERT_EQ((uint32_t)OH_HUKS_PADDING_PKCS7 == (uint32_t)HKS_PADDING_PKCS7, true);
}

/**
 * @tc.name: HksNativeApiCompatibilityTest004
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:OH_Huks_KeyPadding value compatibility
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest004, TestSize.Level0)
{
    ASSERT_EQ((uint32_t)OH_HUKS_PADDING_NONE == 0, true);
    ASSERT_EQ((uint32_t)OH_HUKS_PADDING_OAEP == 1, true);
    ASSERT_EQ((uint32_t)OH_HUKS_PADDING_PSS == 2, true);
    ASSERT_EQ((uint32_t)OH_HUKS_PADDING_PKCS1_V1_5 == 3, true);
    ASSERT_EQ((uint32_t)OH_HUKS_PADDING_PKCS5 == 4, true);
    ASSERT_EQ((uint32_t)OH_HUKS_PADDING_PKCS7 == 5, true);
}


/**
 * @tc.name: HksNativeApiCompatibilityTest005
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:OH_Huks_CipherMode equal to HksCipherMode
 * @tc.type: FUNC
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest005, TestSize.Level0)
{
    ASSERT_EQ((uint32_t)OH_HUKS_MODE_ECB == (uint32_t)HKS_MODE_ECB, true);
    ASSERT_EQ((uint32_t)OH_HUKS_MODE_CBC == (uint32_t)HKS_MODE_CBC, true);
    ASSERT_EQ((uint32_t)OH_HUKS_MODE_CTR == (uint32_t)HKS_MODE_CTR, true);
    ASSERT_EQ((uint32_t)OH_HUKS_MODE_OFB == (uint32_t)HKS_MODE_OFB, true);
    ASSERT_EQ((uint32_t)OH_HUKS_MODE_CCM == (uint32_t)HKS_MODE_CCM, true);
    ASSERT_EQ((uint32_t)OH_HUKS_MODE_GCM == (uint32_t)HKS_MODE_GCM, true);
}

/**
 * @tc.name: HksNativeApiCompatibilityTest006
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:OH_Huks_CipherMode value compatibility
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest006, TestSize.Level0)
{
    ASSERT_EQ((uint32_t)OH_HUKS_MODE_ECB == 1, true);
    ASSERT_EQ((uint32_t)OH_HUKS_MODE_CBC == 2, true);
    ASSERT_EQ((uint32_t)OH_HUKS_MODE_CTR == 3, true);
    ASSERT_EQ((uint32_t)OH_HUKS_MODE_OFB == 4, true);
    ASSERT_EQ((uint32_t)OH_HUKS_MODE_CCM == 31, true);
    ASSERT_EQ((uint32_t)OH_HUKS_MODE_GCM == 32, true);
}

/**
 * @tc.name: HksNativeApiCompatibilityTest007
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:OH_Huks_KeyPurpose equal to HksKeyPurpose
 * @tc.type: FUNC
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest007, TestSize.Level0)
{
    ASSERT_EQ((uint32_t)OH_HUKS_KEY_PURPOSE_ENCRYPT == (uint32_t)HKS_KEY_PURPOSE_ENCRYPT, true);
    ASSERT_EQ((uint32_t)OH_HUKS_KEY_PURPOSE_DECRYPT == (uint32_t)HKS_KEY_PURPOSE_DECRYPT, true);
    ASSERT_EQ((uint32_t)OH_HUKS_KEY_PURPOSE_SIGN == (uint32_t)HKS_KEY_PURPOSE_SIGN, true);
    ASSERT_EQ((uint32_t)OH_HUKS_KEY_PURPOSE_VERIFY == (uint32_t)HKS_KEY_PURPOSE_VERIFY, true);
    ASSERT_EQ((uint32_t)OH_HUKS_KEY_PURPOSE_DERIVE == (uint32_t)HKS_KEY_PURPOSE_DERIVE, true);
    ASSERT_EQ((uint32_t)OH_HUKS_KEY_PURPOSE_WRAP == (uint32_t)HKS_KEY_PURPOSE_WRAP, true);
    ASSERT_EQ((uint32_t)OH_HUKS_KEY_PURPOSE_UNWRAP == (uint32_t)HKS_KEY_PURPOSE_UNWRAP, true);
    ASSERT_EQ((uint32_t)OH_HUKS_KEY_PURPOSE_MAC == (uint32_t)HKS_KEY_PURPOSE_MAC, true);
    ASSERT_EQ((uint32_t)OH_HUKS_KEY_PURPOSE_AGREE == (uint32_t)HKS_KEY_PURPOSE_AGREE, true);
}

/**
 * @tc.name: HksNativeApiCompatibilityTest008
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:OH_Huks_KeyPurpose value compatibility
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest008, TestSize.Level0)
{
    ASSERT_EQ((uint32_t)OH_HUKS_KEY_PURPOSE_ENCRYPT == 1, true);
    ASSERT_EQ((uint32_t)OH_HUKS_KEY_PURPOSE_DECRYPT == 2, true);
    ASSERT_EQ((uint32_t)OH_HUKS_KEY_PURPOSE_SIGN == 4, true);
    ASSERT_EQ((uint32_t)OH_HUKS_KEY_PURPOSE_VERIFY == 8, true);
    ASSERT_EQ((uint32_t)OH_HUKS_KEY_PURPOSE_DERIVE == 16, true);
    ASSERT_EQ((uint32_t)OH_HUKS_KEY_PURPOSE_WRAP == 32, true);
    ASSERT_EQ((uint32_t)OH_HUKS_KEY_PURPOSE_UNWRAP == 64, true);
    ASSERT_EQ((uint32_t)OH_HUKS_KEY_PURPOSE_MAC == 128, true);
    ASSERT_EQ((uint32_t)OH_HUKS_KEY_PURPOSE_AGREE == 256, true);
}

/**
 * @tc.name: HksNativeApiCompatibilityTest009
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:OH_Huks_KeySize equal to HksKeySize
 * @tc.type: FUNC
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest009, TestSize.Level0)
{
    ASSERT_EQ((uint32_t)OH_HUKS_RSA_KEY_SIZE_512 == (uint32_t)HKS_RSA_KEY_SIZE_512, true);
    ASSERT_EQ((uint32_t)OH_HUKS_RSA_KEY_SIZE_768 == (uint32_t)HKS_RSA_KEY_SIZE_768, true);
    ASSERT_EQ((uint32_t)OH_HUKS_RSA_KEY_SIZE_1024 == (uint32_t)HKS_RSA_KEY_SIZE_1024, true);
    ASSERT_EQ((uint32_t)OH_HUKS_RSA_KEY_SIZE_2048 == (uint32_t)HKS_RSA_KEY_SIZE_2048, true);
    ASSERT_EQ((uint32_t)OH_HUKS_RSA_KEY_SIZE_3072 == (uint32_t)HKS_RSA_KEY_SIZE_3072, true);
    ASSERT_EQ((uint32_t)OH_HUKS_RSA_KEY_SIZE_4096 == (uint32_t)HKS_RSA_KEY_SIZE_4096, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ECC_KEY_SIZE_224 == (uint32_t)HKS_ECC_KEY_SIZE_224, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ECC_KEY_SIZE_256 == (uint32_t)HKS_ECC_KEY_SIZE_256, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ECC_KEY_SIZE_384 == (uint32_t)HKS_ECC_KEY_SIZE_384, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ECC_KEY_SIZE_521 == (uint32_t)HKS_ECC_KEY_SIZE_521, true);
    ASSERT_EQ((uint32_t)OH_HUKS_AES_KEY_SIZE_128 == (uint32_t)HKS_AES_KEY_SIZE_128, true);
    ASSERT_EQ((uint32_t)OH_HUKS_AES_KEY_SIZE_192 == (uint32_t)HKS_AES_KEY_SIZE_192, true);
    ASSERT_EQ((uint32_t)OH_HUKS_AES_KEY_SIZE_256 == (uint32_t)HKS_AES_KEY_SIZE_256, true);
    ASSERT_EQ((uint32_t)OH_HUKS_AES_KEY_SIZE_512 == (uint32_t)HKS_AES_KEY_SIZE_512, true);
    ASSERT_EQ((uint32_t)OH_HUKS_DH_KEY_SIZE_2048 == (uint32_t)HKS_DH_KEY_SIZE_2048, true);
    ASSERT_EQ((uint32_t)OH_HUKS_DH_KEY_SIZE_3072 == (uint32_t)HKS_DH_KEY_SIZE_3072, true);
    ASSERT_EQ((uint32_t)OH_HUKS_DH_KEY_SIZE_4096 == (uint32_t)HKS_DH_KEY_SIZE_4096, true);
    ASSERT_EQ((uint32_t)OH_HUKS_SM2_KEY_SIZE_256 == (uint32_t)HKS_SM2_KEY_SIZE_256, true);
    ASSERT_EQ((uint32_t)OH_HUKS_SM4_KEY_SIZE_128 == (uint32_t)HKS_SM4_KEY_SIZE_128, true);
    ASSERT_EQ((uint32_t)OH_HUKS_CURVE25519_KEY_SIZE_256 == (uint32_t)HKS_CURVE25519_KEY_SIZE_256, true);
}

/**
 * @tc.name: HksNativeApiCompatibilityTest010
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:OH_Huks_KeyPurpose value compatibility
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest010, TestSize.Level0)
{
    ASSERT_EQ((uint32_t)OH_HUKS_RSA_KEY_SIZE_512 == 512, true);
    ASSERT_EQ((uint32_t)OH_HUKS_RSA_KEY_SIZE_768 == 768, true);
    ASSERT_EQ((uint32_t)OH_HUKS_RSA_KEY_SIZE_1024 == 1024, true);
    ASSERT_EQ((uint32_t)OH_HUKS_RSA_KEY_SIZE_2048 == 2048, true);
    ASSERT_EQ((uint32_t)OH_HUKS_RSA_KEY_SIZE_3072 == 3072, true);
    ASSERT_EQ((uint32_t)OH_HUKS_RSA_KEY_SIZE_4096 == 4096, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ECC_KEY_SIZE_224 == 224, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ECC_KEY_SIZE_256 == 256, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ECC_KEY_SIZE_384 == 384, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ECC_KEY_SIZE_521 == 521, true);
    ASSERT_EQ((uint32_t)OH_HUKS_AES_KEY_SIZE_128 == 128, true);
    ASSERT_EQ((uint32_t)OH_HUKS_AES_KEY_SIZE_192 == 192, true);
    ASSERT_EQ((uint32_t)OH_HUKS_AES_KEY_SIZE_256 == 256, true);
    ASSERT_EQ((uint32_t)OH_HUKS_AES_KEY_SIZE_512 == 512, true);
    ASSERT_EQ((uint32_t)OH_HUKS_DH_KEY_SIZE_2048 == 2048, true);
    ASSERT_EQ((uint32_t)OH_HUKS_DH_KEY_SIZE_3072 == 3072, true);
    ASSERT_EQ((uint32_t)OH_HUKS_DH_KEY_SIZE_4096 == 4096, true);
    ASSERT_EQ((uint32_t)OH_HUKS_SM2_KEY_SIZE_256 == 256, true);
    ASSERT_EQ((uint32_t)OH_HUKS_SM4_KEY_SIZE_128 == 128, true);
    ASSERT_EQ((uint32_t)OH_HUKS_CURVE25519_KEY_SIZE_256 == 256, true);
}

/**
 * @tc.name: HksNativeApiCompatibilityTest011
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:OH_Huks_KeyAlg equal to HksKeyAlg
 * @tc.type: FUNC
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest011, TestSize.Level0)
{
    ASSERT_EQ((uint32_t)OH_HUKS_ALG_RSA == (uint32_t)HKS_ALG_RSA, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ALG_ECC == (uint32_t)HKS_ALG_ECC, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ALG_DSA == (uint32_t)HKS_ALG_DSA, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ALG_AES == (uint32_t)HKS_ALG_AES, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ALG_HMAC == (uint32_t)HKS_ALG_HMAC, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ALG_HKDF == (uint32_t)HKS_ALG_HKDF, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ALG_PBKDF2 == (uint32_t)HKS_ALG_PBKDF2, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ALG_ECDH == (uint32_t)HKS_ALG_ECDH, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ALG_X25519 == (uint32_t)HKS_ALG_X25519, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ALG_ED25519 == (uint32_t)HKS_ALG_ED25519, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ALG_DH == (uint32_t)HKS_ALG_DH, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ALG_SM2 == (uint32_t)HKS_ALG_SM2, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ALG_SM3 == (uint32_t)HKS_ALG_SM3, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ALG_SM4 == (uint32_t)HKS_ALG_SM4, true);
}

/**
 * @tc.name: HksNativeApiCompatibilityTest012
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:OH_Huks_KeyAlg value compatibility
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest012, TestSize.Level0)
{
    ASSERT_EQ((uint32_t)OH_HUKS_ALG_RSA == 1, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ALG_ECC == 2, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ALG_DSA == 3, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ALG_AES == 20, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ALG_HMAC == 50, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ALG_HKDF == 51, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ALG_PBKDF2 == 52, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ALG_ECDH == 100, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ALG_X25519 == 101, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ALG_ED25519 == 102, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ALG_DH == 103, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ALG_SM2 == 150, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ALG_SM3 == 151, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ALG_SM4 == 152, true);
}

/**
 * @tc.name: HksNativeApiCompatibilityTest013
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:OH_Huks_AlgSuite equal to HksAlgSuite
 * @tc.type: FUNC
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest013, TestSize.Level0)
{
    ASSERT_EQ((uint32_t)OH_HUKS_UNWRAP_SUITE_X25519_AES_256_GCM_NOPADDING ==
        HKS_UNWRAP_SUITE_X25519_AES_256_GCM_NOPADDING, true);
    ASSERT_EQ((uint32_t)OH_HUKS_UNWRAP_SUITE_ECDH_AES_256_GCM_NOPADDING ==
        HKS_UNWRAP_SUITE_ECDH_AES_256_GCM_NOPADDING, true);
}

/**
 * @tc.name: HksNativeApiCompatibilityTest014
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:OH_Huks_AlgSuite value compatibility
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest014, TestSize.Level0)
{
    ASSERT_EQ((uint32_t)OH_HUKS_UNWRAP_SUITE_X25519_AES_256_GCM_NOPADDING == 1, true);
    ASSERT_EQ((uint32_t)OH_HUKS_UNWRAP_SUITE_ECDH_AES_256_GCM_NOPADDING == 2, true);
}

/**
 * @tc.name: HksNativeApiCompatibilityTest015
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:OH_Huks_KeyGenerateType equal to HksKeyGenerateType
 * @tc.type: FUNC
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest015, TestSize.Level0)
{
    ASSERT_EQ((uint32_t)OH_HUKS_KEY_GENERATE_TYPE_DEFAULT == (uint32_t)HKS_KEY_GENERATE_TYPE_DEFAULT, true);
    ASSERT_EQ((uint32_t)OH_HUKS_KEY_GENERATE_TYPE_DERIVE == (uint32_t)HKS_KEY_GENERATE_TYPE_DERIVE, true);
    ASSERT_EQ((uint32_t)OH_HUKS_KEY_GENERATE_TYPE_AGREE == (uint32_t)HKS_KEY_GENERATE_TYPE_AGREE, true);
}

/**
 * @tc.name: HksNativeApiCompatibilityTest016
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:OH_Huks_KeyGenerateType value compatibility
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest016, TestSize.Level0)
{
    ASSERT_EQ((uint32_t)OH_HUKS_KEY_GENERATE_TYPE_DEFAULT == 0, true);
    ASSERT_EQ((uint32_t)OH_HUKS_KEY_GENERATE_TYPE_DERIVE == 1, true);
    ASSERT_EQ((uint32_t)OH_HUKS_KEY_GENERATE_TYPE_AGREE == 2, true);
}

/**
 * @tc.name: HksNativeApiCompatibilityTest017
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:OH_Huks_KeyFlag equal to HksKeyFlag
 * @tc.type: FUNC
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest017, TestSize.Level0)
{
    ASSERT_EQ((uint32_t)OH_HUKS_KEY_FLAG_IMPORT_KEY == (uint32_t)HKS_KEY_FLAG_IMPORT_KEY, true);
    ASSERT_EQ((uint32_t)OH_HUKS_KEY_FLAG_GENERATE_KEY == (uint32_t)HKS_KEY_FLAG_GENERATE_KEY, true);
    ASSERT_EQ((uint32_t)OH_HUKS_KEY_FLAG_AGREE_KEY == (uint32_t)HKS_KEY_FLAG_AGREE_KEY, true);
    ASSERT_EQ((uint32_t)OH_HUKS_KEY_FLAG_DERIVE_KEY == (uint32_t)HKS_KEY_FLAG_DERIVE_KEY, true);
}

/**
 * @tc.name: HksNativeApiCompatibilityTest018
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:OH_Huks_KeyFlag value compatibility
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest018, TestSize.Level0)
{
    ASSERT_EQ((uint32_t)OH_HUKS_KEY_FLAG_IMPORT_KEY == 1, true);
    ASSERT_EQ((uint32_t)OH_HUKS_KEY_FLAG_GENERATE_KEY == 2, true);
    ASSERT_EQ((uint32_t)OH_HUKS_KEY_FLAG_AGREE_KEY == 3, true);
    ASSERT_EQ((uint32_t)OH_HUKS_KEY_FLAG_DERIVE_KEY == 4, true);
}

/**
 * @tc.name: HksNativeApiCompatibilityTest019
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:OH_Huks_KeyStorageType equal to HksKeyStorageType
 * @tc.type: FUNC
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest019, TestSize.Level0)
{
    ASSERT_EQ((uint32_t)OH_HUKS_STORAGE_TEMP == (uint32_t)HKS_STORAGE_TEMP, true);
    ASSERT_EQ((uint32_t)OH_HUKS_STORAGE_PERSISTENT == (uint32_t)HKS_STORAGE_PERSISTENT, true);
}

/**
 * @tc.name: HksNativeApiCompatibilityTest020
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:OH_Huks_KeyStorageType value compatibility
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest020, TestSize.Level0)
{
    ASSERT_EQ((uint32_t)OH_HUKS_STORAGE_TEMP == 0, true);
    ASSERT_EQ((uint32_t)OH_HUKS_STORAGE_PERSISTENT == 1, true);
}

/**
 * @tc.name: HksNativeApiCompatibilityTest021
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:OH_Huks_ImportKeyType equal to HksImportKeyType
 * @tc.type: FUNC
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest021, TestSize.Level0)
{
    ASSERT_EQ((uint32_t)OH_HUKS_KEY_TYPE_PUBLIC_KEY == (uint32_t)HKS_KEY_TYPE_PUBLIC_KEY, true);
    ASSERT_EQ((uint32_t)OH_HUKS_KEY_TYPE_PRIVATE_KEY == (uint32_t)HKS_KEY_TYPE_PRIVATE_KEY, true);
    ASSERT_EQ((uint32_t)OH_HUKS_KEY_TYPE_KEY_PAIR == (uint32_t)HKS_KEY_TYPE_KEY_PAIR, true);
}

/**
 * @tc.name: HksNativeApiCompatibilityTest022
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:OH_Huks_ErrCode value compatibility
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest022, TestSize.Level0)
{
    ASSERT_EQ((uint32_t)OH_HUKS_SUCCESS == 0, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ERR_CODE_PERMISSION_FAIL == 201, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ERR_CODE_ILLEGAL_ARGUMENT == 401, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ERR_CODE_NOT_SUPPORTED_API == 801, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ERR_CODE_FEATURE_NOT_SUPPORTED == 12000001, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ERR_CODE_MISSING_CRYPTO_ALG_ARGUMENT == 12000002, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ERR_CODE_INVALID_CRYPTO_ALG_ARGUMENT == 12000003, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ERR_CODE_FILE_OPERATION_FAIL == 12000004, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ERR_CODE_COMMUNICATION_FAIL == 12000005, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ERR_CODE_CRYPTO_FAIL == 12000006, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ERR_CODE_KEY_AUTH_PERMANENTLY_INVALIDATED == 12000007, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ERR_CODE_KEY_AUTH_VERIFY_FAILED == 12000008, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ERR_CODE_KEY_AUTH_TIME_OUT == 12000009, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ERR_CODE_SESSION_LIMIT == 12000010, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ERR_CODE_ITEM_NOT_EXIST == 12000011, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ERR_CODE_INTERNAL_ERROR == 12000012, true);
    ASSERT_EQ((uint32_t)OH_HUKS_ERR_CODE_CREDENTIAL_NOT_EXIST == 12000013, true);
}

/**
 * @tc.name: HksNativeApiCompatibilityTest023
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:OH_Huks_TagType equal to HksTagType
 * @tc.type: FUNC
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest023, TestSize.Level0)
{
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_TYPE_INVALID == (uint32_t)HKS_TAG_TYPE_INVALID, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_TYPE_INT == (uint32_t)HKS_TAG_TYPE_INT, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_TYPE_UINT == (uint32_t)HKS_TAG_TYPE_UINT, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_TYPE_ULONG == (uint32_t)HKS_TAG_TYPE_ULONG, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_TYPE_BOOL == (uint32_t)HKS_TAG_TYPE_BOOL, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_TYPE_BYTES == (uint32_t)HKS_TAG_TYPE_BYTES, true);
}

/**
 * @tc.name: HksNativeApiCompatibilityTest024
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:OH_Huks_TagType value compatibility
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest024, TestSize.Level0)
{
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_TYPE_INVALID == 0 << 28, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_TYPE_INT == 1 << 28, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_TYPE_UINT == 2 << 28, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_TYPE_ULONG == 3 << 28, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_TYPE_BOOL == 4 << 28, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_TYPE_BYTES == 5 << 28, true);
}
/**
 * @tc.name: HksNativeApiCompatibilityTest025
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:OH_Huks_UserAuthType equal to HksUserAuthType
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest025, TestSize.Level0)
{
    ASSERT_EQ((uint32_t)OH_HUKS_USER_AUTH_TYPE_FINGERPRINT == (uint32_t)HKS_USER_AUTH_TYPE_FINGERPRINT, true);
    ASSERT_EQ((uint32_t)OH_HUKS_USER_AUTH_TYPE_FACE == (uint32_t)HKS_USER_AUTH_TYPE_FACE, true);
    ASSERT_EQ((uint32_t)OH_HUKS_USER_AUTH_TYPE_PIN == (uint32_t)HKS_USER_AUTH_TYPE_PIN, true);
}


/**
 * @tc.name: HksNativeApiCompatibilityTest026
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:OH_Huks_UserAuthType value compatibility
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest026, TestSize.Level0)
{
    ASSERT_EQ((uint32_t)OH_HUKS_USER_AUTH_TYPE_FINGERPRINT == 1 << 0, true);
    ASSERT_EQ((uint32_t)OH_HUKS_USER_AUTH_TYPE_FACE == 1 << 1, true);
    ASSERT_EQ((uint32_t)OH_HUKS_USER_AUTH_TYPE_PIN == 1 << 2, true);
}

/**
 * @tc.name: HksNativeApiCompatibilityTest027
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:OH_Huks_AuthAccessType equal to HksAuthAccessType
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest027, TestSize.Level0)
{
    ASSERT_EQ((uint32_t)OH_HUKS_AUTH_ACCESS_INVALID_CLEAR_PASSWORD ==
        (uint32_t)HKS_AUTH_ACCESS_INVALID_CLEAR_PASSWORD, true);
    ASSERT_EQ((uint32_t)OH_HUKS_AUTH_ACCESS_INVALID_NEW_BIO_ENROLL ==
        (uint32_t)HKS_AUTH_ACCESS_INVALID_NEW_BIO_ENROLL, true);
}


/**
 * @tc.name: HksNativeApiCompatibilityTest028
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:OH_Huks_AuthAccessType value compatibility
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest028, TestSize.Level0)
{
    ASSERT_EQ((uint32_t)OH_HUKS_AUTH_ACCESS_INVALID_CLEAR_PASSWORD == 1 << 0, true);
    ASSERT_EQ((uint32_t)OH_HUKS_AUTH_ACCESS_INVALID_NEW_BIO_ENROLL == 1 << 1, true);
}


/**
 * @tc.name: HksNativeApiCompatibilityTest029
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:OH_Huks_ChallengeType equal to HksChallengeType
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest029, TestSize.Level0)
{
    ASSERT_EQ((uint32_t)OH_HUKS_CHALLENGE_TYPE_NORMAL == (uint32_t)HKS_CHALLENGE_TYPE_NORMAL, true);
    ASSERT_EQ((uint32_t)OH_HUKS_CHALLENGE_TYPE_CUSTOM == (uint32_t)HKS_CHALLENGE_TYPE_CUSTOM, true);
    ASSERT_EQ((uint32_t)OH_HUKS_CHALLENGE_TYPE_NONE == (uint32_t)HKS_CHALLENGE_TYPE_NONE, true);
}


/**
 * @tc.name: HksNativeApiCompatibilityTest030
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:OH_Huks_ChallengeType value compatibility
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest030, TestSize.Level0)
{
    ASSERT_EQ((uint32_t)OH_HUKS_CHALLENGE_TYPE_NORMAL == 0, true);
    ASSERT_EQ((uint32_t)OH_HUKS_CHALLENGE_TYPE_CUSTOM == 1, true);
    ASSERT_EQ((uint32_t)OH_HUKS_CHALLENGE_TYPE_NONE == 2, true);
}

/**
 * @tc.name: HksNativeApiCompatibilityTest031
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:OH_Huks_ChallengePosition equal to HksChallengeType
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest031, TestSize.Level0)
{
    ASSERT_EQ((uint32_t)OH_HUKS_CHALLENGE_POS_0 == (uint32_t)HKS_CHALLENGE_POS_0, true);
    ASSERT_EQ((uint32_t)OH_HUKS_CHALLENGE_POS_1 == (uint32_t)HKS_CHALLENGE_POS_1, true);
    ASSERT_EQ((uint32_t)OH_HUKS_CHALLENGE_POS_2 == (uint32_t)HKS_CHALLENGE_POS_2, true);
    ASSERT_EQ((uint32_t)OH_HUKS_CHALLENGE_POS_3 == (uint32_t)HKS_CHALLENGE_POS_3, true);
}


/**
 * @tc.name: HksNativeApiCompatibilityTest032
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:OH_Huks_ChallengePosition value compatibility
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest032, TestSize.Level0)
{
    ASSERT_EQ((uint32_t)OH_HUKS_CHALLENGE_POS_0 == 0, true);
    ASSERT_EQ((uint32_t)OH_HUKS_CHALLENGE_POS_1 == 1, true);
    ASSERT_EQ((uint32_t)OH_HUKS_CHALLENGE_POS_2 == 2, true);
    ASSERT_EQ((uint32_t)OH_HUKS_CHALLENGE_POS_3 == 3, true);
}

/**
 * @tc.name: HksNativeApiCompatibilityTest033
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:OH_Huks_SecureSignType equal to HksSecureSignType
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest033, TestSize.Level0)
{
    ASSERT_EQ((uint32_t)OH_HUKS_SECURE_SIGN_WITH_AUTHINFO == (uint32_t)HKS_SECURE_SIGN_WITH_AUTHINFO, true);
}


/**
 * @tc.name: HksNativeApiCompatibilityTest034
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:OH_Huks_SecureSignType value compatibility
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest034, TestSize.Level0)
{
    ASSERT_EQ((uint32_t)OH_HUKS_SECURE_SIGN_WITH_AUTHINFO == 1, true);
}

/**
 * @tc.name: HksNativeApiCompatibilityTest035
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:OH_Huks_Tag equal to HksTag
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest035, TestSize.Level0)
{
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_ALGORITHM == (uint32_t)HKS_TAG_ALGORITHM, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_PURPOSE == (uint32_t)HKS_TAG_PURPOSE, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_KEY_SIZE == (uint32_t)HKS_TAG_KEY_SIZE, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_DIGEST == (uint32_t)HKS_TAG_DIGEST, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_PADDING == (uint32_t)HKS_TAG_PADDING, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_BLOCK_MODE == (uint32_t)HKS_TAG_BLOCK_MODE, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_KEY_TYPE == (uint32_t)HKS_TAG_KEY_TYPE, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_ASSOCIATED_DATA == (uint32_t)HKS_TAG_ASSOCIATED_DATA, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_NONCE == (uint32_t)HKS_TAG_NONCE, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_IV == (uint32_t)HKS_TAG_IV, true);
    
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_INFO == (uint32_t)HKS_TAG_INFO, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_SALT == (uint32_t)HKS_TAG_SALT, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_ITERATION == (uint32_t)HKS_TAG_ITERATION, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_KEY_GENERATE_TYPE == (uint32_t)HKS_TAG_KEY_GENERATE_TYPE, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_AGREE_ALG == (uint32_t)HKS_TAG_AGREE_ALG, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_AGREE_PUBLIC_KEY_IS_KEY_ALIAS ==
        (uint32_t)HKS_TAG_AGREE_PUBLIC_KEY_IS_KEY_ALIAS, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_AGREE_PRIVATE_KEY_ALIAS == (uint32_t)HKS_TAG_AGREE_PRIVATE_KEY_ALIAS, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_AGREE_PUBLIC_KEY == (uint32_t)HKS_TAG_AGREE_PUBLIC_KEY, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_KEY_ALIAS == (uint32_t)HKS_TAG_KEY_ALIAS, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_DERIVE_KEY_SIZE == (uint32_t)HKS_TAG_DERIVE_KEY_SIZE, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_IMPORT_KEY_TYPE == (uint32_t)HKS_TAG_IMPORT_KEY_TYPE, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_UNWRAP_ALGORITHM_SUITE == (uint32_t)HKS_TAG_UNWRAP_ALGORITHM_SUITE, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_ALL_USERS == (uint32_t)HKS_TAG_ALL_USERS, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_USER_ID == (uint32_t)HKS_TAG_USER_ID, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_NO_AUTH_REQUIRED == (uint32_t)HKS_TAG_NO_AUTH_REQUIRED, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_USER_AUTH_TYPE == (uint32_t)HKS_TAG_USER_AUTH_TYPE, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_AUTH_TIMEOUT == (uint32_t)HKS_TAG_AUTH_TIMEOUT, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_AUTH_TOKEN == (uint32_t)HKS_TAG_AUTH_TOKEN, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_KEY_AUTH_ACCESS_TYPE == (uint32_t)HKS_TAG_KEY_AUTH_ACCESS_TYPE, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_KEY_SECURE_SIGN_TYPE == (uint32_t)HKS_TAG_KEY_SECURE_SIGN_TYPE, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_CHALLENGE_TYPE == (uint32_t)HKS_TAG_CHALLENGE_TYPE, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_CHALLENGE_POS == (uint32_t)HKS_TAG_CHALLENGE_POS, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_ATTESTATION_CHALLENGE == (uint32_t)HKS_TAG_ATTESTATION_CHALLENGE, true);

    ASSERT_EQ((uint32_t)OH_HUKS_TAG_ATTESTATION_APPLICATION_ID == (uint32_t)HKS_TAG_ATTESTATION_APPLICATION_ID, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_ATTESTATION_ID_ALIAS == (uint32_t)HKS_TAG_ATTESTATION_ID_ALIAS, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_ATTESTATION_ID_SEC_LEVEL_INFO ==
        (uint32_t)HKS_TAG_ATTESTATION_ID_SEC_LEVEL_INFO, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_ATTESTATION_ID_VERSION_INFO ==
        (uint32_t)HKS_TAG_ATTESTATION_ID_VERSION_INFO, true);

    ASSERT_EQ((uint32_t)OH_HUKS_TAG_IS_KEY_ALIAS == (uint32_t)HKS_TAG_IS_KEY_ALIAS, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_KEY_STORAGE_FLAG == (uint32_t)HKS_TAG_KEY_STORAGE_FLAG, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_IS_ALLOWED_WRAP == (uint32_t)HKS_TAG_IS_ALLOWED_WRAP, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_KEY_WRAP_TYPE == (uint32_t)HKS_TAG_KEY_WRAP_TYPE, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_KEY_AUTH_ID == (uint32_t)HKS_TAG_KEY_AUTH_ID, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_KEY_ROLE == (uint32_t)HKS_TAG_KEY_ROLE, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_KEY_FLAG == (uint32_t)HKS_TAG_KEY_FLAG, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_IS_ASYNCHRONIZED == (uint32_t)HKS_TAG_IS_ASYNCHRONIZED, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_KEY_DOMAIN == (uint32_t)HKS_TAG_KEY_DOMAIN, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_AE_TAG == (uint32_t)HKS_TAG_AE_TAG, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_SYMMETRIC_KEY_DATA == (uint32_t)HKS_TAG_SYMMETRIC_KEY_DATA, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_ASYMMETRIC_PUBLIC_KEY_DATA == (uint32_t)HKS_TAG_ASYMMETRIC_PUBLIC_KEY_DATA, true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_ASYMMETRIC_PRIVATE_KEY_DATA ==
        (uint32_t)HKS_TAG_ASYMMETRIC_PRIVATE_KEY_DATA, true);
}

/**
 * @tc.name: HksNativeApiCompatibilityTest036
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:OH_Huks_Tag value compatibility
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest036, TestSize.Level0)
{
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_ALGORITHM == (OH_HUKS_TAG_TYPE_UINT | 1), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_PURPOSE == (OH_HUKS_TAG_TYPE_UINT | 2), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_KEY_SIZE == (OH_HUKS_TAG_TYPE_UINT | 3), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_DIGEST == (OH_HUKS_TAG_TYPE_UINT | 4), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_PADDING == (OH_HUKS_TAG_TYPE_UINT | 5), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_BLOCK_MODE == (OH_HUKS_TAG_TYPE_UINT | 6), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_KEY_TYPE == (OH_HUKS_TAG_TYPE_UINT | 7), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_ASSOCIATED_DATA == (OH_HUKS_TAG_TYPE_BYTES | 8), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_NONCE == (OH_HUKS_TAG_TYPE_BYTES | 9), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_IV == (OH_HUKS_TAG_TYPE_BYTES | 10), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_INFO == (OH_HUKS_TAG_TYPE_BYTES | 11), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_SALT == (OH_HUKS_TAG_TYPE_BYTES | 12), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_ITERATION == (OH_HUKS_TAG_TYPE_UINT | 14), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_KEY_GENERATE_TYPE == (OH_HUKS_TAG_TYPE_UINT | 15), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_AGREE_ALG == (OH_HUKS_TAG_TYPE_UINT | 19), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_AGREE_PUBLIC_KEY_IS_KEY_ALIAS == (OH_HUKS_TAG_TYPE_BOOL | 20), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_AGREE_PRIVATE_KEY_ALIAS == (OH_HUKS_TAG_TYPE_BYTES | 21), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_AGREE_PUBLIC_KEY == (OH_HUKS_TAG_TYPE_BYTES | 22), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_KEY_ALIAS == (OH_HUKS_TAG_TYPE_BYTES | 23), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_DERIVE_KEY_SIZE == (OH_HUKS_TAG_TYPE_UINT | 24), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_IMPORT_KEY_TYPE == (OH_HUKS_TAG_TYPE_UINT | 25), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_UNWRAP_ALGORITHM_SUITE == (OH_HUKS_TAG_TYPE_UINT | 26), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_ALL_USERS == (OH_HUKS_TAG_TYPE_BOOL | 301), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_USER_ID == (OH_HUKS_TAG_TYPE_UINT | 302), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_NO_AUTH_REQUIRED == (OH_HUKS_TAG_TYPE_BOOL | 303), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_USER_AUTH_TYPE == (OH_HUKS_TAG_TYPE_UINT | 304), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_AUTH_TIMEOUT == (OH_HUKS_TAG_TYPE_UINT | 305), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_AUTH_TOKEN == (OH_HUKS_TAG_TYPE_BYTES | 306), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_KEY_AUTH_ACCESS_TYPE == (OH_HUKS_TAG_TYPE_UINT | 307), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_KEY_SECURE_SIGN_TYPE == (OH_HUKS_TAG_TYPE_UINT | 308), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_CHALLENGE_TYPE == (OH_HUKS_TAG_TYPE_UINT | 309), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_CHALLENGE_POS == (OH_HUKS_TAG_TYPE_UINT | 310), true);
    
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_ATTESTATION_CHALLENGE == (OH_HUKS_TAG_TYPE_BYTES | 501), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_ATTESTATION_APPLICATION_ID == (OH_HUKS_TAG_TYPE_BYTES | 502), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_ATTESTATION_ID_ALIAS == (OH_HUKS_TAG_TYPE_BYTES | 511), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_ATTESTATION_ID_SEC_LEVEL_INFO == (OH_HUKS_TAG_TYPE_BYTES | 514), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_ATTESTATION_ID_VERSION_INFO == (OH_HUKS_TAG_TYPE_BYTES | 515), true);

    ASSERT_EQ((uint32_t)OH_HUKS_TAG_IS_KEY_ALIAS == (OH_HUKS_TAG_TYPE_BOOL | 1001), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_KEY_STORAGE_FLAG == (OH_HUKS_TAG_TYPE_UINT | 1002), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_IS_ALLOWED_WRAP == (OH_HUKS_TAG_TYPE_BOOL | 1003), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_KEY_WRAP_TYPE == (OH_HUKS_TAG_TYPE_UINT | 1004), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_KEY_AUTH_ID == (OH_HUKS_TAG_TYPE_BYTES | 1005), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_KEY_ROLE == (OH_HUKS_TAG_TYPE_UINT | 1006), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_KEY_FLAG == (OH_HUKS_TAG_TYPE_UINT | 1007), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_IS_ASYNCHRONIZED == (OH_HUKS_TAG_TYPE_UINT | 1008), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_KEY_DOMAIN == (OH_HUKS_TAG_TYPE_UINT | 1011), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_AE_TAG == (OH_HUKS_TAG_TYPE_BYTES | 10009), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_SYMMETRIC_KEY_DATA == (OH_HUKS_TAG_TYPE_BYTES | 20001), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_ASYMMETRIC_PUBLIC_KEY_DATA == (OH_HUKS_TAG_TYPE_BYTES | 20002), true);
    ASSERT_EQ((uint32_t)OH_HUKS_TAG_ASYMMETRIC_PRIVATE_KEY_DATA == (OH_HUKS_TAG_TYPE_BYTES | 20003), true);
}


/**
 * @tc.name: HksNativeApiCompatibilityTest060
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:struct OH_Huks_Result member and size
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest060, TestSize.Level0)
{
    ASSERT_EQ(sizeof(struct OH_Huks_Result) == sizeof(struct HksResult), true);
    struct OH_Huks_Result result;
    
    ASSERT_EQ(sizeof(result.errorCode) == sizeof(int32_t), true);
    ASSERT_EQ(sizeof(result.errorMsg) == sizeof(const char *), true);
    ASSERT_EQ(sizeof(result.data) == sizeof(uint8_t *), true);
    struct HksResult hksResult;
    ASSERT_EQ(sizeof(hksResult.errorCode) == sizeof(int32_t), true);
    ASSERT_EQ(sizeof(hksResult.errorMsg) == sizeof(const char *), true);
    ASSERT_EQ(sizeof(hksResult.data) == sizeof(uint8_t *), true);
}

/**
 * @tc.name: HksNativeApiCompatibilityTest061
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:struct OH_Huks_Blob member and size
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest061, TestSize.Level0)
{
    ASSERT_EQ(sizeof(struct OH_Huks_Blob) == sizeof(struct HksBlob), true);
    struct OH_Huks_Blob blob;
    ASSERT_EQ(sizeof(blob.size) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(blob.data) == sizeof(uint8_t *), true);
    struct HksBlob hksBlob;
    ASSERT_EQ(sizeof(hksBlob.size) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(hksBlob.data) == sizeof(uint8_t *), true);
}

/**
 * @tc.name: HksNativeApiCompatibilityTest062
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:struct OH_Huks_Param member and size
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest062, TestSize.Level0)
{
    ASSERT_EQ(sizeof(struct OH_Huks_Param) == sizeof(struct HksParam), true);
    struct OH_Huks_Param param;
    ASSERT_EQ(sizeof(param.tag) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(param.boolParam) == sizeof(bool), true);
    ASSERT_EQ(sizeof(param.int32Param) == sizeof(int32_t), true);
    ASSERT_EQ(sizeof(param.uint32Param) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(param.uint64Param) == sizeof(uint64_t), true);
    ASSERT_EQ(sizeof(param.blob) == sizeof(struct OH_Huks_Blob), true);
    struct HksParam hksParam;
    ASSERT_EQ(sizeof(hksParam.tag) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(hksParam.boolParam) == sizeof(bool), true);
    ASSERT_EQ(sizeof(hksParam.int32Param) == sizeof(int32_t), true);
    ASSERT_EQ(sizeof(hksParam.uint32Param) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(hksParam.uint64Param) == sizeof(uint64_t), true);
    ASSERT_EQ(sizeof(hksParam.blob) == sizeof(struct HksBlob), true);
}

/**
 * @tc.name: HksNativeApiCompatibilityTest063
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:struct OH_Huks_ParamSet member and size
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest063, TestSize.Level0)
{
    ASSERT_EQ(sizeof(struct OH_Huks_ParamSet) == sizeof(struct HksParamSet), true);
    struct OH_Huks_ParamSet paramset;
    ASSERT_EQ(sizeof(paramset.paramSetSize) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(paramset.paramsCnt) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(paramset.params[0]) == sizeof(struct OH_Huks_Param), true);
    struct HksParamSet hksParamSet;
    ASSERT_EQ(sizeof(hksParamSet.paramSetSize) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(hksParamSet.paramsCnt) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(hksParamSet.params[0]) == sizeof(struct HksParam), true);
}

/**
 * @tc.name: HksNativeApiCompatibilityTest064
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:struct OH_Huks_CertChain member and size
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest064, TestSize.Level0)
{
    ASSERT_EQ(sizeof(struct OH_Huks_CertChain) == sizeof(struct HksCertChain), true);
    struct OH_Huks_CertChain certChain;
    ASSERT_EQ(sizeof(certChain.certs) == sizeof(struct OH_Huks_Blob *), true);
    ASSERT_EQ(sizeof(certChain.certsCount) == sizeof(uint32_t), true);
    struct HksCertChain hksCertChain;
    ASSERT_EQ(sizeof(hksCertChain.certs) == sizeof(struct HksBlob *), true);
    ASSERT_EQ(sizeof(hksCertChain.certsCount) == sizeof(uint32_t), true);
}

/**
 * @tc.name: HksNativeApiCompatibilityTest065
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:struct OH_Huks_KeyInfo member and size
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest065, TestSize.Level0)
{
    ASSERT_EQ(sizeof(struct OH_Huks_KeyInfo) == sizeof(struct HksKeyInfo), true);
    struct OH_Huks_KeyInfo keyInfo;
    ASSERT_EQ(sizeof(keyInfo.alias) == sizeof(struct OH_Huks_Blob), true);
    ASSERT_EQ(sizeof(keyInfo.paramSet) == sizeof(struct OH_Huks_ParamSet *), true);
    struct HksKeyInfo hksKeyInfo;
    ASSERT_EQ(sizeof(hksKeyInfo.alias) == sizeof(struct HksBlob), true);
    ASSERT_EQ(sizeof(hksKeyInfo.paramSet) == sizeof(struct HksParamSet *), true);
}

/**
 * @tc.name: HksNativeApiCompatibilityTest066
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:struct OH_Huks_PubKeyInfo member and size
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest066, TestSize.Level0)
{
    ASSERT_EQ(sizeof(struct OH_Huks_PubKeyInfo) == sizeof(struct HksPubKeyInfo), true);
    struct OH_Huks_PubKeyInfo pubKeyInfo;
    ASSERT_EQ(sizeof(pubKeyInfo.keyAlg) == sizeof(OH_Huks_KeyAlg), true);
    ASSERT_EQ(sizeof(pubKeyInfo.keySize) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(pubKeyInfo.nOrXSize) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(pubKeyInfo.eOrYSize) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(pubKeyInfo.placeHolder) == sizeof(uint32_t), true);
    struct HksPubKeyInfo hksPubKeyInfo;
    ASSERT_EQ(sizeof(hksPubKeyInfo.keyAlg) == sizeof(HksKeyAlg), true);
    ASSERT_EQ(sizeof(hksPubKeyInfo.keySize) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(hksPubKeyInfo.nOrXSize) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(hksPubKeyInfo.eOrYSize) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(hksPubKeyInfo.placeHolder) == sizeof(uint32_t), true);
}

/**
 * @tc.name: HksNativeApiCompatibilityTest067
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:struct OH_Huks_KeyMaterialRsa member and size
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest067, TestSize.Level0)
{
    ASSERT_EQ(sizeof(struct OH_Huks_KeyMaterialRsa) == sizeof(struct HksKeyMaterialRsa), true);
    struct OH_Huks_KeyMaterialRsa keyMaterialRsa;
    ASSERT_EQ(sizeof(keyMaterialRsa.keyAlg) == sizeof(OH_Huks_KeyAlg), true);
    ASSERT_EQ(sizeof(keyMaterialRsa.keySize) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(keyMaterialRsa.nSize) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(keyMaterialRsa.eSize) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(keyMaterialRsa.dSize) == sizeof(uint32_t), true);
    struct HksKeyMaterialRsa hksKeyMaterialRsa;
    ASSERT_EQ(sizeof(hksKeyMaterialRsa.keyAlg) == sizeof(HksKeyAlg), true);
    ASSERT_EQ(sizeof(hksKeyMaterialRsa.keySize) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(hksKeyMaterialRsa.nSize) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(hksKeyMaterialRsa.eSize) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(hksKeyMaterialRsa.dSize) == sizeof(uint32_t), true);
}

/**
 * @tc.name: HksNativeApiCompatibilityTest068
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:struct OH_Huks_KeyMaterialEcc member and size
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest068, TestSize.Level0)
{
    ASSERT_EQ(sizeof(struct OH_Huks_KeyMaterialEcc) == sizeof(struct HksKeyMaterialEcc), true);
    struct OH_Huks_KeyMaterialEcc keyMaterialEcc;
    ASSERT_EQ(sizeof(keyMaterialEcc.keyAlg) == sizeof(OH_Huks_KeyAlg), true);
    ASSERT_EQ(sizeof(keyMaterialEcc.keySize) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(keyMaterialEcc.xSize) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(keyMaterialEcc.ySize) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(keyMaterialEcc.zSize) == sizeof(uint32_t), true);
    struct HksKeyMaterialEcc hksKeyMaterialEcc;
    ASSERT_EQ(sizeof(hksKeyMaterialEcc.keyAlg) == sizeof(HksKeyAlg), true);
    ASSERT_EQ(sizeof(hksKeyMaterialEcc.keySize) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(hksKeyMaterialEcc.xSize) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(hksKeyMaterialEcc.ySize) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(hksKeyMaterialEcc.zSize) == sizeof(uint32_t), true);
}

/**
 * @tc.name: HksNativeApiCompatibilityTest069
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:struct OH_Huks_KeyMaterialDsa member and size
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest069, TestSize.Level0)
{
    ASSERT_EQ(sizeof(struct OH_Huks_KeyMaterialDsa) == sizeof(struct HksKeyMaterialDsa), true);
    struct OH_Huks_KeyMaterialDsa keyMaterialDsa;
    ASSERT_EQ(sizeof(keyMaterialDsa.keyAlg) == sizeof(OH_Huks_KeyAlg), true);
    ASSERT_EQ(sizeof(keyMaterialDsa.keySize) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(keyMaterialDsa.xSize) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(keyMaterialDsa.ySize) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(keyMaterialDsa.pSize) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(keyMaterialDsa.qSize) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(keyMaterialDsa.gSize) == sizeof(uint32_t), true);
    struct HksKeyMaterialDsa hksKeyMaterialDsa;
    ASSERT_EQ(sizeof(hksKeyMaterialDsa.keyAlg) == sizeof(HksKeyAlg), true);
    ASSERT_EQ(sizeof(hksKeyMaterialDsa.keySize) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(hksKeyMaterialDsa.xSize) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(hksKeyMaterialDsa.ySize) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(hksKeyMaterialDsa.pSize) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(hksKeyMaterialDsa.qSize) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(hksKeyMaterialDsa.gSize) == sizeof(uint32_t), true);
}

/**
 * @tc.name: HksNativeApiCompatibilityTest070
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:struct OH_Huks_KeyMaterialDh member and size
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest070, TestSize.Level0)
{
    ASSERT_EQ(sizeof(struct OH_Huks_KeyMaterialDh) == sizeof(struct HksKeyMaterialDh), true);
    struct OH_Huks_KeyMaterialDh keyMaterialDh;
    ASSERT_EQ(sizeof(keyMaterialDh.keyAlg) == sizeof(OH_Huks_KeyAlg), true);
    ASSERT_EQ(sizeof(keyMaterialDh.keySize) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(keyMaterialDh.pubKeySize) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(keyMaterialDh.priKeySize) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(keyMaterialDh.reserved) == sizeof(uint32_t), true);
    struct HksKeyMaterialDh hksKeyMaterialDh;
    ASSERT_EQ(sizeof(hksKeyMaterialDh.keyAlg) == sizeof(HksKeyAlg), true);
    ASSERT_EQ(sizeof(hksKeyMaterialDh.keySize) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(hksKeyMaterialDh.pubKeySize) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(hksKeyMaterialDh.priKeySize) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(hksKeyMaterialDh.reserved) == sizeof(uint32_t), true);
}

/**
 * @tc.name: HksNativeApiCompatibilityTest071
 * @tc.desc: normal case to test native api(OHOS-NDK) compatibility:struct OH_Huks_KeyMaterial25519 member and size
 * @tc.require: issueI5UONG
 */
HWTEST_F(HksNativeApiCompatibilityTest, HksNativeApiCompatibilityTest071, TestSize.Level0)
{
    ASSERT_EQ(sizeof(struct OH_Huks_KeyMaterial25519) == sizeof(struct HksKeyMaterial25519), true);
    struct OH_Huks_KeyMaterial25519 keyMaterial25519;
    ASSERT_EQ(sizeof(keyMaterial25519.keyAlg) == sizeof(OH_Huks_KeyAlg), true);
    ASSERT_EQ(sizeof(keyMaterial25519.keySize) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(keyMaterial25519.pubKeySize) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(keyMaterial25519.priKeySize) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(keyMaterial25519.reserved) == sizeof(uint32_t), true);
    struct HksKeyMaterial25519 hksKeyMaterial25519;
    ASSERT_EQ(sizeof(hksKeyMaterial25519.keyAlg) == sizeof(HksKeyAlg), true);
    ASSERT_EQ(sizeof(hksKeyMaterial25519.keySize) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(hksKeyMaterial25519.pubKeySize) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(hksKeyMaterial25519.priKeySize) == sizeof(uint32_t), true);
    ASSERT_EQ(sizeof(hksKeyMaterial25519.reserved) == sizeof(uint32_t), true);
}

}
