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

#include <gtest/gtest.h>

#include "hks_import_wrapped_test_common.h"
#include "hks_three_stage_test_common.h"
#include "hks_access_control_test_common.h"
#include "hks_mem.h"
#include "hks_test_log.h"
#include "hks_type.h"
#include "hks_api.h"
#include "hks_access_control_secure_sign_test.h"

using namespace testing::ext;
using namespace Unittest::HksAccessControlPartTest;
namespace Unittest::AccessControlSecureSignTest {
class HksAccessControlSecureSignTest : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};

void HksAccessControlSecureSignTest::SetUpTestCase(void)
{
}

void HksAccessControlSecureSignTest::TearDownTestCase(void)
{
}

void HksAccessControlSecureSignTest::SetUp()
{
    EXPECT_EQ(HksInitialize(), 0);
}

void HksAccessControlSecureSignTest::TearDown()
{
}

static const std::string g_inData = "Hks_SM4_Cipher_Test_000000000000000000000000000000000000000000000000000000000000"
    "00000000000000000000000000000000000000000000000000000000000000000000000000000000"
    "0000000000000000000000000000000000000000000000000000000000000000000000000_string";
    
static const std::string g_inDataLess64 = "Hks_SM4_Cipher_Test_000000000000000000000000000000000000";

static const uint32_t g_authHeadSize = 24;

static const uint32_t g_secureUid = 1;

static const uint32_t g_enrolledIdPin = 1;

static const uint32_t g_enrolledIdFinger = 2;

static const uint32_t g_credentialId = 0;

static const uint32_t g_time = 0;

static struct HksBlob g_genKeyAlias = {
    .size = strlen("TestGenKeyForSignWithInfo"),
    .data = (uint8_t *)"TestGenKeyForSignWithInfo"
};

static struct HksBlob g_importKeyAlias = {
    .size = strlen("TestImportKeyForSignWithInfo"),
    .data = (uint8_t *)"TestImportKeyForSignWithInfo"
};

static struct HksBlob g_importKeyNoAuthAlias = {
    .size = strlen("TestImportKeyNoSignWithInfo"),
    .data = (uint8_t *)"TestImportKeyNoSignWithInfo"
};

static const uint32_t g_outDataSize = 2048;

static uint8_t g_outBuffer[g_outDataSize] = {0};

static struct HksBlob g_outDataBlob = {
    .size = g_outDataSize,
    .data = g_outBuffer
};

static struct HksBlob g_inDataBlob = { g_inData.length(), (uint8_t *)g_inData.c_str() };

static struct HksBlob g_inDataBlobTwoStage = { g_inDataLess64.length(), (uint8_t *)g_inDataLess64.c_str() };

struct HksTestSecureSignGenParams {
    struct HksBlob *keyAlias;
    struct HksParam *inputParams;
    uint32_t inputParamSize;
    int32_t expectResult;
};

struct HksTestSecureSignImportParams {
    struct HksBlob *keyAlias;
    struct HksParam *inputParams;
    struct HksBlob importKey;
    uint32_t inputParamSize;
    int32_t expectResult;
};

struct HksTestSecureSignVerifyUpdateFinishParams {
    struct HksBlob *keyAlias;
    struct HksBlob *keyAliasNoAuth;
    struct HksParam *updateParams;
    struct HksBlob *outBuffer;
    struct HksBlob *signature;
    struct HksBlob *inData;
    uint32_t inputParamSize;
    int32_t expectResult;
    bool isThreeStageUse;
};

static struct HksParam g_genRsaWithSignAuthParams[] = {
{
    .tag = HKS_TAG_ALGORITHM,
    .uint32Param = HKS_ALG_RSA
}, {
    .tag = HKS_TAG_PURPOSE,
    .uint32Param = HKS_KEY_PURPOSE_SIGN | HKS_KEY_PURPOSE_VERIFY
}, {
    .tag = HKS_TAG_KEY_SIZE,
    .uint32Param = HKS_RSA_KEY_SIZE_4096
}, {
    .tag = HKS_TAG_PADDING,
    .uint32Param = HKS_PADDING_PSS
}, {
    .tag = HKS_TAG_DIGEST,
    .uint32Param = HKS_DIGEST_SHA512
}, {
    .tag = HKS_TAG_USER_AUTH_TYPE,
    .uint32Param = HKS_USER_AUTH_TYPE_PIN
}, {
    .tag = HKS_TAG_KEY_AUTH_ACCESS_TYPE,
    .uint32Param = HKS_AUTH_ACCESS_INVALID_CLEAR_PASSWORD
}, {
    .tag = HKS_TAG_CHALLENGE_TYPE,
    .uint32Param = HKS_CHALLENGE_TYPE_NORMAL
}, {
    .tag = HKS_TAG_KEY_SECURE_SIGN_TYPE,
    .uint32Param = HKS_SECURE_SIGN_WITH_AUTHINFO
}
};

static struct HksParam g_genEd25519WithSignAuthParams[] = {
{
    .tag = HKS_TAG_ALGORITHM,
    .uint32Param = HKS_ALG_ED25519
}, {
    .tag = HKS_TAG_PURPOSE,
    .uint32Param = HKS_KEY_PURPOSE_SIGN | HKS_KEY_PURPOSE_VERIFY
}, {
    .tag = HKS_TAG_KEY_SIZE,
    .uint32Param = HKS_CURVE25519_KEY_SIZE_256
}, {
    .tag = HKS_TAG_DIGEST,
    .uint32Param = HKS_DIGEST_SHA1
}, {
    .tag = HKS_TAG_USER_AUTH_TYPE,
    .uint32Param = HKS_USER_AUTH_TYPE_FACE
}, {
    .tag = HKS_TAG_KEY_AUTH_ACCESS_TYPE,
    .uint32Param = HKS_AUTH_ACCESS_INVALID_NEW_BIO_ENROLL
}, {
    .tag = HKS_TAG_CHALLENGE_TYPE,
    .uint32Param = HKS_CHALLENGE_TYPE_NORMAL
}, {
    .tag = HKS_TAG_KEY_SECURE_SIGN_TYPE,
    .uint32Param = HKS_SECURE_SIGN_WITH_AUTHINFO
}
};

#ifdef _USE_OPENSSL_
// mbedtls engine don't support DSA alg
static struct HksParam g_genDsaWithSignAuthParams[] = {
{
    .tag = HKS_TAG_ALGORITHM,
    .uint32Param = HKS_ALG_DSA
}, {
    .tag = HKS_TAG_PURPOSE,
    .uint32Param = HKS_KEY_PURPOSE_SIGN | HKS_KEY_PURPOSE_VERIFY
}, {
    .tag = HKS_TAG_KEY_SIZE,
    .uint32Param = 1024
}, {
    .tag = HKS_TAG_DIGEST,
    .uint32Param = HKS_DIGEST_SHA1
}, {
    .tag = HKS_TAG_USER_AUTH_TYPE,
    .uint32Param = HKS_USER_AUTH_TYPE_PIN
}, {
    .tag = HKS_TAG_KEY_AUTH_ACCESS_TYPE,
    .uint32Param = HKS_AUTH_ACCESS_INVALID_CLEAR_PASSWORD
}, {
    .tag = HKS_TAG_CHALLENGE_TYPE,
    .uint32Param = HKS_CHALLENGE_TYPE_NORMAL
}, {
    .tag = HKS_TAG_KEY_SECURE_SIGN_TYPE,
    .uint32Param = HKS_SECURE_SIGN_WITH_AUTHINFO
}
};
#endif

static struct HksTestSecureSignGenParams g_testRsaGenParams = {
    .keyAlias = &g_genKeyAlias,
    .inputParams = g_genRsaWithSignAuthParams,
    .inputParamSize = HKS_ARRAY_SIZE(g_genRsaWithSignAuthParams),
    .expectResult = HKS_SUCCESS
};

static struct HksTestSecureSignGenParams g_testEd25519GenParams = {
    .keyAlias = &g_genKeyAlias,
    .inputParams = g_genEd25519WithSignAuthParams,
    .inputParamSize = HKS_ARRAY_SIZE(g_genEd25519WithSignAuthParams),
    .expectResult = HKS_SUCCESS
};

#ifdef _USE_OPENSSL_
static struct HksTestSecureSignGenParams g_testDsaGenParams = {
    .keyAlias = &g_genKeyAlias,
    .inputParams = g_genDsaWithSignAuthParams,
    .inputParamSize = HKS_ARRAY_SIZE(g_genDsaWithSignAuthParams),
    .expectResult = HKS_SUCCESS
};
#endif

static struct HksParam g_importDsaKeyParams[] = {
    { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_DSA },
    { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_SIGN | HKS_KEY_PURPOSE_VERIFY },
    { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_RSA_KEY_SIZE_2048 },
    { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA256 },
    { .tag = HKS_TAG_IMPORT_KEY_TYPE, .uint32Param = HKS_KEY_TYPE_KEY_PAIR },
    {
        .tag = HKS_TAG_USER_AUTH_TYPE,
        .uint32Param = HKS_USER_AUTH_TYPE_PIN
    }, {
        .tag = HKS_TAG_KEY_AUTH_ACCESS_TYPE,
        .uint32Param = HKS_AUTH_ACCESS_INVALID_CLEAR_PASSWORD
    }, {
        .tag = HKS_TAG_CHALLENGE_TYPE,
        .uint32Param = HKS_CHALLENGE_TYPE_NORMAL
    }, {
        .tag = HKS_TAG_KEY_SECURE_SIGN_TYPE,
        .uint32Param = HKS_SECURE_SIGN_WITH_AUTHINFO
    }
};

    
static struct HksParam g_importDsaKeyParamsNoAuthInfo[] = {
    { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_DSA },
    { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_SIGN | HKS_KEY_PURPOSE_VERIFY },
    { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_RSA_KEY_SIZE_2048 },
    { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA256 },
    { .tag = HKS_TAG_IMPORT_KEY_TYPE, .uint32Param = HKS_KEY_TYPE_KEY_PAIR }
};

static struct HksParam g_importRsaKeyParams[] = {
    { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_RSA },
    { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_SIGN | HKS_KEY_PURPOSE_VERIFY },
    { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_RSA_KEY_SIZE_2048 },
    { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_PSS },
    { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA256 },
    { .tag = HKS_TAG_IMPORT_KEY_TYPE, .uint32Param = HKS_KEY_TYPE_KEY_PAIR },
    {
        .tag = HKS_TAG_USER_AUTH_TYPE,
        .uint32Param = HKS_USER_AUTH_TYPE_PIN
    }, {
        .tag = HKS_TAG_KEY_AUTH_ACCESS_TYPE,
        .uint32Param = HKS_AUTH_ACCESS_INVALID_CLEAR_PASSWORD
    }, {
        .tag = HKS_TAG_CHALLENGE_TYPE,
        .uint32Param = HKS_CHALLENGE_TYPE_NORMAL
    }, {
        .tag = HKS_TAG_KEY_SECURE_SIGN_TYPE,
        .uint32Param = HKS_SECURE_SIGN_WITH_AUTHINFO
    }
};

static struct HksParam g_importRsaKeyParamsWithBioAndClearPassword[] = {
    { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_RSA },
    { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_SIGN | HKS_KEY_PURPOSE_VERIFY },
    { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_RSA_KEY_SIZE_2048 },
    { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_PSS },
    { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA256 },
    { .tag = HKS_TAG_IMPORT_KEY_TYPE, .uint32Param = HKS_KEY_TYPE_KEY_PAIR },
    {
        .tag = HKS_TAG_USER_AUTH_TYPE,
        .uint32Param = HKS_USER_AUTH_TYPE_FINGERPRINT
    }, {
        .tag = HKS_TAG_KEY_AUTH_ACCESS_TYPE,
        .uint32Param = HKS_AUTH_ACCESS_INVALID_CLEAR_PASSWORD
    }, {
        .tag = HKS_TAG_CHALLENGE_TYPE,
        .uint32Param = HKS_CHALLENGE_TYPE_NORMAL
    }, {
        .tag = HKS_TAG_KEY_SECURE_SIGN_TYPE,
        .uint32Param = HKS_SECURE_SIGN_WITH_AUTHINFO
    }
};

static struct HksParam g_importRsaKeyParamsNoAuth[] = {
    { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_RSA },
    { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_SIGN | HKS_KEY_PURPOSE_VERIFY },
    { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_RSA_KEY_SIZE_2048 },
    { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_PSS },
    { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA256 },
    { .tag = HKS_TAG_IMPORT_KEY_TYPE, .uint32Param = HKS_KEY_TYPE_KEY_PAIR }
};

static struct HksParam g_importKeyEd25519Params[] = {
    { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_ED25519 },
    { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_SIGN | HKS_KEY_PURPOSE_VERIFY },
    { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_CURVE25519_KEY_SIZE_256 },
    { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA256 },
    { .tag = HKS_TAG_IMPORT_KEY_TYPE, .uint32Param = HKS_KEY_TYPE_KEY_PAIR },
    {
        .tag = HKS_TAG_USER_AUTH_TYPE,
        .uint32Param = HKS_USER_AUTH_TYPE_PIN
    }, {
        .tag = HKS_TAG_KEY_AUTH_ACCESS_TYPE,
        .uint32Param = HKS_AUTH_ACCESS_INVALID_CLEAR_PASSWORD
    }, {
        .tag = HKS_TAG_CHALLENGE_TYPE,
        .uint32Param = HKS_CHALLENGE_TYPE_NORMAL
    }, {
        .tag = HKS_TAG_KEY_SECURE_SIGN_TYPE,
        .uint32Param = HKS_SECURE_SIGN_WITH_AUTHINFO
    }
};

static struct HksParam g_importKeyEd25519ParamsNoAuth[] = {
    { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_ED25519 },
    { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_SIGN | HKS_KEY_PURPOSE_VERIFY },
    { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_CURVE25519_KEY_SIZE_256 },
    { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA256 },
    { .tag = HKS_TAG_IMPORT_KEY_TYPE, .uint32Param = HKS_KEY_TYPE_KEY_PAIR }
};

static struct HksParam g_signParamsTestRsa[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_RSA
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_SIGN
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_RSA_KEY_SIZE_2048
    }, {
        .tag = HKS_TAG_PADDING,
        .uint32Param = HKS_PADDING_PSS
    }, {
        .tag = HKS_TAG_DIGEST,
        .uint32Param = HKS_DIGEST_SHA256
    }
};

static struct HksParam g_verifyParamsTestRsa[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_RSA
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_VERIFY
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_RSA_KEY_SIZE_2048
    }, {
        .tag = HKS_TAG_PADDING,
        .uint32Param = HKS_PADDING_PSS
    }, {
        .tag = HKS_TAG_DIGEST,
        .uint32Param = HKS_DIGEST_SHA256
    }
};

static struct HksParam g_signParamsTestDsa[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_DSA
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_SIGN
    }, {
        .tag = HKS_TAG_DIGEST,
        .uint32Param = HKS_DIGEST_SHA256
    }
};

static struct HksParam g_verifyParamsTestDsa[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_DSA
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_VERIFY
    }, {
        .tag = HKS_TAG_DIGEST,
        .uint32Param = HKS_DIGEST_SHA256
    }
};

static struct HksParam g_signParamsTestEd25519[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_ED25519
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_SIGN
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_CURVE25519_KEY_SIZE_256
    }, {
        .tag = HKS_TAG_DIGEST,
        .uint32Param = HKS_DIGEST_SHA256
    }
};

static struct HksParam g_verifyParamsTestEd25519[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_ED25519
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_VERIFY
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_CURVE25519_KEY_SIZE_256
    }, {
        .tag = HKS_TAG_DIGEST,
        .uint32Param = HKS_DIGEST_SHA256
    }
};

static const uint8_t g_nData2048[] = {
    0xc5, 0x35, 0x62, 0x48, 0xc4, 0x92, 0x87, 0x73, 0x0d, 0x42, 0x96, 0xfc, 0x7b, 0x11, 0x05, 0x06,
    0x0f, 0x8d, 0x66, 0xc1, 0x0e, 0xad, 0x37, 0x44, 0x92, 0x95, 0x2f, 0x6a, 0x55, 0xba, 0xec, 0x1d,
    0x54, 0x62, 0x0a, 0x4b, 0xd3, 0xc7, 0x05, 0xe4, 0x07, 0x40, 0xd9, 0xb7, 0xc2, 0x12, 0xcb, 0x9a,
    0x90, 0xad, 0xe3, 0x24, 0xe8, 0x5e, 0xa6, 0xf8, 0xd0, 0x6e, 0xbc, 0xd1, 0x69, 0x7f, 0x6b, 0xe4,
    0x2b, 0x4e, 0x1a, 0x65, 0xbb, 0x73, 0x88, 0x6b, 0x7c, 0xaf, 0x7e, 0xd0, 0x47, 0x26, 0xeb, 0xa5,
    0xbe, 0xd6, 0xe8, 0xee, 0x9c, 0xa5, 0x66, 0xa5, 0xc9, 0xd3, 0x25, 0x13, 0xc4, 0x0e, 0x6c, 0xab,
    0x50, 0xb6, 0x50, 0xc9, 0xce, 0x8f, 0x0a, 0x0b, 0xc6, 0x28, 0x69, 0xe9, 0x83, 0x69, 0xde, 0x42,
    0x56, 0x79, 0x7f, 0xde, 0x86, 0x24, 0xca, 0xfc, 0xaa, 0xc0, 0xf3, 0xf3, 0x7f, 0x92, 0x8e, 0x8a,
    0x12, 0x52, 0xfe, 0x50, 0xb1, 0x5e, 0x8c, 0x01, 0xce, 0xfc, 0x7e, 0xf2, 0x4f, 0x5f, 0x03, 0xfe,
    0xa7, 0xcd, 0xa1, 0xfc, 0x94, 0x52, 0x00, 0x8b, 0x9b, 0x7f, 0x09, 0xab, 0xa8, 0xa4, 0xf5, 0xb4,
    0xa5, 0xaa, 0xfc, 0x72, 0xeb, 0x17, 0x40, 0xa9, 0xee, 0xbe, 0x8f, 0xc2, 0xd1, 0x80, 0xc2, 0x0d,
    0x44, 0xa9, 0x59, 0x44, 0x59, 0x81, 0x3b, 0x5d, 0x4a, 0xde, 0xfb, 0xae, 0x24, 0xfc, 0xa3, 0xd9,
    0xbc, 0x57, 0x55, 0xc2, 0x26, 0xbc, 0x19, 0xa7, 0x9a, 0xc5, 0x59, 0xa3, 0xee, 0x5a, 0xef, 0x41,
    0x80, 0x7d, 0xf8, 0x5e, 0xc1, 0x1d, 0x32, 0x38, 0x41, 0x5b, 0xb6, 0x92, 0xb8, 0xb7, 0x03, 0x0d,
    0x3e, 0x59, 0x0f, 0x1c, 0xb3, 0xe1, 0x2a, 0x95, 0x1a, 0x3b, 0x50, 0x4f, 0xc4, 0x1d, 0xcf, 0x73,
    0x7c, 0x14, 0xca, 0xe3, 0x0b, 0xa7, 0xc7, 0x1a, 0x41, 0x4a, 0xee, 0xbe, 0x1f, 0x43, 0xdd, 0xf9,
};

static const uint8_t g_dData2048[] = {
    0x88, 0x4b, 0x82, 0xe7, 0xe3, 0xe3, 0x99, 0x75, 0x6c, 0x9e, 0xaf, 0x17, 0x44, 0x3e, 0xd9, 0x07,
    0xfd, 0x4b, 0xae, 0xce, 0x92, 0xc4, 0x28, 0x44, 0x5e, 0x42, 0x79, 0x08, 0xb6, 0xc3, 0x7f, 0x58,
    0x2d, 0xef, 0xac, 0x4a, 0x07, 0xcd, 0xaf, 0x46, 0x8f, 0xb4, 0xc4, 0x43, 0xf9, 0xff, 0x5f, 0x74,
    0x2d, 0xb5, 0xe0, 0x1c, 0xab, 0xf4, 0x6e, 0xd5, 0xdb, 0xc8, 0x0c, 0xfb, 0x76, 0x3c, 0x38, 0x66,
    0xf3, 0x7f, 0x01, 0x43, 0x7a, 0x30, 0x39, 0x02, 0x80, 0xa4, 0x11, 0xb3, 0x04, 0xd9, 0xe3, 0x57,
    0x23, 0xf4, 0x07, 0xfc, 0x91, 0x8a, 0xc6, 0xcc, 0xa2, 0x16, 0x29, 0xb3, 0xe5, 0x76, 0x4a, 0xa8,
    0x84, 0x19, 0xdc, 0xef, 0xfc, 0xb0, 0x63, 0x33, 0x0b, 0xfa, 0xf6, 0x68, 0x0b, 0x08, 0xea, 0x31,
    0x52, 0xee, 0x99, 0xef, 0x43, 0x2a, 0xbe, 0x97, 0xad, 0xb3, 0xb9, 0x66, 0x7a, 0xae, 0xe1, 0x8f,
    0x57, 0x86, 0xe5, 0xfe, 0x14, 0x3c, 0x81, 0xd0, 0x64, 0xf8, 0x86, 0x1a, 0x0b, 0x40, 0x58, 0xc9,
    0x33, 0x49, 0xb8, 0x99, 0xc6, 0x2e, 0x94, 0x70, 0xee, 0x09, 0x88, 0xe1, 0x5c, 0x4e, 0x6c, 0x22,
    0x72, 0xa7, 0x2a, 0x21, 0xdd, 0xd7, 0x1d, 0xfc, 0x63, 0x15, 0x0b, 0xde, 0x06, 0x9c, 0xf3, 0x28,
    0xf3, 0xac, 0x4a, 0xa8, 0xb5, 0x50, 0xca, 0x9b, 0xcc, 0x0a, 0x04, 0xfe, 0x3f, 0x98, 0x68, 0x81,
    0xac, 0x24, 0x53, 0xea, 0x1f, 0x1c, 0x6e, 0x5e, 0xca, 0xe8, 0x31, 0x0d, 0x08, 0x12, 0xf3, 0x26,
    0xf8, 0x5e, 0xeb, 0x10, 0x27, 0xae, 0xaa, 0xc3, 0xad, 0x6c, 0xc1, 0x89, 0xdb, 0x7d, 0x5a, 0x12,
    0x55, 0xad, 0x11, 0x19, 0xa1, 0xa9, 0x8f, 0x0b, 0x6d, 0x78, 0x8d, 0x1c, 0xdf, 0xe5, 0x63, 0x82,
    0x0b, 0x7d, 0x23, 0x04, 0xb4, 0x75, 0x8c, 0xed, 0x77, 0xfc, 0x1a, 0x85, 0x29, 0x11, 0xe0, 0x61,
};

static const uint8_t g_eData[] = { 0x01, 0x00, 0x01 };


static const uint8_t g_ed25519PriData[] = {
    0x61, 0xd3, 0xe7, 0x53, 0x6d, 0x79, 0x5d, 0x71, 0xc2, 0x2a, 0x51, 0x2d, 0x5e, 0xcb, 0x67, 0x3d,
    0xdd, 0xde, 0xf0, 0xac, 0xdb, 0xba, 0x24, 0xfd, 0xf8, 0x3a, 0x7b, 0x32, 0x6e, 0x05, 0xe6, 0x37,
};

static const uint8_t g_ed25519PubData[] = {
    0xab, 0xc7, 0x0f, 0x99, 0x4f, 0x6a, 0x08, 0xd0, 0x9c, 0x5d, 0x10, 0x60, 0xf8, 0x93, 0xd2, 0x8e,
    0xe0, 0x63, 0x0e, 0x70, 0xbf, 0xad, 0x30, 0x41, 0x43, 0x09, 0x27, 0x2d, 0xb3, 0x30, 0x95, 0xa7,
};

static const uint8_t g_xData[] = {
    0x99, 0x97, 0x76, 0x67, 0x4a, 0xd1, 0x21, 0xe2, 0xbd, 0x75, 0xf3, 0x05, 0x34, 0xe6, 0xc2, 0x27,
    0xf9, 0x4b, 0xb0, 0x47, 0xb4, 0x7c, 0xc0, 0x16, 0x05, 0x2c, 0x93, 0xd2, 0xdf, 0xe6, 0x13, 0xa8,
};

static const uint8_t g_yData[] = {
    0x9b, 0xf4, 0x99, 0x33, 0x05, 0xf0, 0x0c, 0xa9, 0x88, 0xcc, 0xb0, 0x3e, 0x17, 0x6b, 0x67, 0xc7,
    0x8e, 0x31, 0x6d, 0xf7, 0x34, 0xfe, 0xdd, 0x76, 0x8e, 0x3d, 0xfa, 0x8c, 0x39, 0xdd, 0xce, 0xfa,
    0xa9, 0xe2, 0x7b, 0xc0, 0x01, 0x23, 0xf6, 0x62, 0x93, 0x77, 0x69, 0x91, 0x6e, 0xa5, 0x32, 0x46,
    0xbb, 0x95, 0x67, 0x81, 0xbd, 0xe8, 0xb1, 0xe4, 0xdf, 0xc8, 0x75, 0x99, 0x23, 0xc0, 0x29, 0x1b,
    0xc7, 0x23, 0x0b, 0xf2, 0x11, 0xb8, 0x0e, 0x59, 0x25, 0xe3, 0xa2, 0x3f, 0x22, 0xe4, 0xf6, 0x20,
    0xd2, 0xdf, 0x58, 0xd2, 0x99, 0xd8, 0x62, 0x9a, 0x24, 0x2e, 0x13, 0x4b, 0xea, 0xc9, 0x8c, 0xf9,
    0x90, 0x6d, 0xa4, 0x34, 0x27, 0x67, 0xee, 0x5c, 0x10, 0x63, 0x58, 0xc0, 0x96, 0x7b, 0x2d, 0x5b,
    0x6c, 0xcd, 0x0c, 0x7b, 0x18, 0xcc, 0x0d, 0x7d, 0x3a, 0xa7, 0x57, 0x9c, 0x12, 0x10, 0x8a, 0x3a,
    0xf4, 0x6b, 0xe4, 0x38, 0xba, 0x9c, 0xf4, 0x7c, 0x27, 0xeb, 0x06, 0x88, 0x35, 0x6b, 0xd7, 0x01,
    0xe3, 0x71, 0x48, 0x41, 0xa4, 0x1d, 0x9b, 0x1d, 0x06, 0x8a, 0x1f, 0x2c, 0x9d, 0xac, 0x81, 0x50,
    0x4d, 0x9b, 0x05, 0xcf, 0xa2, 0x3f, 0x5c, 0x84, 0x97, 0x82, 0xa6, 0x27, 0x77, 0x88, 0xfa, 0x8a,
    0x34, 0x8f, 0x23, 0xcb, 0x79, 0x51, 0x07, 0xfb, 0x43, 0xab, 0x75, 0xee, 0xd4, 0x0c, 0x6a, 0x4c,
    0x5e, 0x1a, 0xc1, 0xa1, 0xd7, 0x32, 0xf8, 0xbe, 0x2b, 0x82, 0xa1, 0x69, 0x4f, 0x24, 0x25, 0x24,
    0x35, 0x5d, 0xf1, 0x4e, 0x07, 0x0b, 0x0b, 0xb5, 0x79, 0x44, 0xcf, 0xea, 0xb6, 0xfa, 0x99, 0x60,
    0x34, 0x5d, 0xad, 0xca, 0x3a, 0x6c, 0x9e, 0xa1, 0x81, 0xe0, 0x51, 0xc2, 0x10, 0x0e, 0x6c, 0x61,
    0x78, 0x07, 0x94, 0x2f, 0xca, 0x0e, 0x04, 0x48, 0x01, 0xaf, 0x29, 0x17, 0x3e, 0x95, 0x99, 0x2d,
};

static const uint8_t g_pData[] = {
    0xa3, 0x45, 0xc9, 0x21, 0x8a, 0xce, 0x76, 0xf9, 0x52, 0x6e, 0x49, 0x2f, 0x25, 0x05, 0xa9, 0xde,
    0xdb, 0xd7, 0x6b, 0x39, 0x59, 0x9c, 0x73, 0xe4, 0xf0, 0xe9, 0x39, 0xd0, 0xb5, 0x12, 0x3d, 0xc0,
    0xb5, 0x2a, 0x08, 0xf3, 0x0a, 0xa4, 0xfc, 0x4a, 0xdb, 0x00, 0x83, 0x85, 0x05, 0xd2, 0xea, 0x9e,
    0x0b, 0xa9, 0x1f, 0x68, 0x77, 0xd6, 0x13, 0x82, 0xdb, 0xf9, 0x52, 0xe5, 0xae, 0x26, 0x37, 0x2d,
    0x8b, 0xbd, 0x2e, 0x1d, 0x9e, 0x0c, 0xf2, 0xab, 0x1c, 0xe9, 0x78, 0xad, 0x76, 0x16, 0x55, 0x12,
    0x81, 0xe0, 0xc9, 0x99, 0x43, 0x9e, 0xf9, 0x78, 0x31, 0x27, 0x7d, 0x13, 0xd7, 0x05, 0x8c, 0xd7,
    0xef, 0x47, 0xc4, 0x9a, 0xc5, 0xa2, 0x99, 0x2a, 0x6a, 0xa0, 0xd8, 0xc7, 0x26, 0xde, 0xf6, 0xa2,
    0x24, 0x12, 0x66, 0x4e, 0x90, 0xa1, 0xe2, 0x9e, 0xaa, 0xb3, 0x40, 0x93, 0x2a, 0x9d, 0x23, 0x85,
    0xec, 0x64, 0xa6, 0x20, 0xdb, 0xe4, 0xbe, 0x8e, 0xe5, 0x4e, 0xec, 0xf6, 0xa8, 0xca, 0x9b, 0x0d,
    0x3e, 0x34, 0x5c, 0x7a, 0xb7, 0x38, 0x2d, 0x41, 0x48, 0x0c, 0x03, 0xc3, 0x55, 0xfa, 0x10, 0x81,
    0x62, 0xc4, 0x08, 0x30, 0xf5, 0x39, 0x4b, 0x5f, 0x32, 0x22, 0x50, 0x6f, 0x9f, 0xcb, 0xb2, 0x8a,
    0x9e, 0x45, 0x71, 0xe2, 0xec, 0xc5, 0x67, 0xeb, 0x3c, 0xe5, 0x8f, 0x16, 0x44, 0x19, 0xe3, 0x2d,
    0xa5, 0x8f, 0xd7, 0xdb, 0x40, 0x3a, 0x17, 0xa9, 0x0c, 0x19, 0x8b, 0x00, 0x69, 0x22, 0x8d, 0x3f,
    0x52, 0x9f, 0x43, 0x06, 0xd5, 0x5d, 0x79, 0x60, 0xa9, 0xc2, 0xd4, 0x3c, 0x1e, 0x81, 0x05, 0x6e,
    0x37, 0x77, 0x82, 0x51, 0x25, 0x74, 0x6a, 0x99, 0xaa, 0xb0, 0xf2, 0x4f, 0x40, 0x2b, 0x29, 0x3f,
    0x34, 0x9c, 0x97, 0x42, 0x63, 0x9c, 0x49, 0xe8, 0x09, 0x3a, 0xbf, 0x26, 0x8c, 0xcb, 0x7e, 0x11,
};

static const uint8_t g_qData[] = {
    0xc4, 0x59, 0x2b, 0xa5, 0xe1, 0x19, 0x89, 0xf2, 0x2c, 0xde, 0x54, 0x08, 0xd3, 0xfa, 0xd3, 0x37,
    0x40, 0xd3, 0xc0, 0x88, 0xe1, 0x08, 0xf0, 0x06, 0xd4, 0x65, 0x82, 0x46, 0xbe, 0xa1, 0x82, 0xdd,
};

static const uint8_t g_gData[] = {
    0x1c, 0x2d, 0x79, 0x2c, 0x1e, 0x9d, 0x9a, 0x69, 0xe3, 0x59, 0xa6, 0xc9, 0xbe, 0xb3, 0x55, 0x2e,
    0x9d, 0xc6, 0xfd, 0x77, 0x33, 0xfe, 0x61, 0x31, 0x6a, 0x2d, 0xcf, 0x12, 0xb9, 0x1f, 0x58, 0x4a,
    0xac, 0xd4, 0xbc, 0xd0, 0xf7, 0xb7, 0x16, 0x3a, 0x6b, 0x80, 0x3e, 0x22, 0x74, 0x81, 0xde, 0x51,
    0x29, 0x95, 0xe6, 0x28, 0x0c, 0xaf, 0x64, 0xba, 0x11, 0x0e, 0x1a, 0xae, 0xb5, 0xb8, 0x40, 0x59,
    0xed, 0x75, 0x5a, 0x5e, 0x54, 0x0b, 0xba, 0x8a, 0xb2, 0x14, 0x23, 0xb0, 0xec, 0x68, 0x18, 0xa1,
    0xd2, 0xfa, 0x2d, 0x16, 0x15, 0xab, 0x1f, 0x0b, 0x18, 0x32, 0x93, 0xc2, 0xd0, 0x54, 0xe0, 0x37,
    0x6f, 0xff, 0x0d, 0x67, 0x4a, 0x90, 0x41, 0x06, 0x33, 0xfc, 0xab, 0xf8, 0xdc, 0x1e, 0x16, 0xf2,
    0x06, 0x93, 0xe3, 0x52, 0x18, 0x46, 0x5e, 0xe3, 0x7f, 0xba, 0x98, 0x56, 0x89, 0x0b, 0xce, 0x0b,
    0xba, 0x01, 0xe2, 0x66, 0x71, 0x85, 0x2a, 0x32, 0x43, 0x9d, 0x48, 0xaf, 0xb9, 0xe4, 0xd3, 0xc8,
    0xdc, 0x3a, 0x8e, 0xb4, 0xf0, 0xa5, 0x11, 0xd4, 0x5f, 0xbf, 0x65, 0x62, 0x76, 0x4c, 0x30, 0xfb,
    0x29, 0x1c, 0x15, 0xa6, 0x16, 0x8a, 0x7f, 0x17, 0x56, 0x40, 0x79, 0x33, 0xd4, 0x91, 0x29, 0xf1,
    0x39, 0x8d, 0xfd, 0x48, 0x97, 0x84, 0xc6, 0x42, 0x1e, 0x83, 0xd2, 0xe5, 0xf9, 0xa4, 0x26, 0x3c,
    0xb8, 0x6a, 0xce, 0x8b, 0xb5, 0x0d, 0xd8, 0x72, 0x38, 0x3b, 0x65, 0xc8, 0x7c, 0x01, 0xf7, 0x6d,
    0x8d, 0x50, 0x87, 0xc2, 0xce, 0x55, 0xfb, 0xe4, 0xf9, 0xe2, 0x98, 0x28, 0x9e, 0x05, 0xdf, 0x28,
    0xcc, 0x0f, 0xe3, 0x54, 0x64, 0x36, 0x2d, 0xa4, 0x7c, 0x5a, 0x0c, 0xcd, 0xe0, 0x51, 0x8f, 0x38,
    0x3a, 0xe7, 0x82, 0x3a, 0x62, 0x69, 0xce, 0xee, 0x53, 0x2a, 0x7c, 0xec, 0x46, 0x5c, 0x51, 0x33,
};

static int32_t CheckSignWithInfoTag(const struct HksBlob *alias, const struct HksParamSet *paramSet)
{
    struct HksParamSet *keyParamSet = NULL;
    int32_t ret = GenParamSetAuthTest(&keyParamSet, paramSet);
    EXPECT_EQ(ret, HKS_SUCCESS) << "GenParamSetAuthTest failed.";

    ret = HksGetKeyParamSet(alias, paramSet, keyParamSet);
    EXPECT_EQ(ret, HKS_SUCCESS) << "HksGetKeyParamSet failed.";

    struct HksParam *secureParam = nullptr;
    ret = HksGetParam(keyParamSet, HKS_TAG_KEY_AUTH_ACCESS_TYPE, &secureParam);
    EXPECT_EQ(ret, HKS_SUCCESS) << "HksGetParam auth access failed.";

    struct HksParam *userParam = nullptr;
    ret = HksGetParam(keyParamSet, HKS_TAG_USER_AUTH_TYPE, &userParam);
    EXPECT_EQ(ret, HKS_SUCCESS) << "HksGetParam user auth failed.";

    struct HksParam *secSignType = nullptr;
    ret = HksGetParam(keyParamSet, HKS_TAG_KEY_SECURE_SIGN_TYPE, &secSignType);
    EXPECT_EQ(ret, HKS_SUCCESS) << "HksGetParam secure sign type failed.";
    EXPECT_EQ(secSignType->uint32Param, HKS_SECURE_SIGN_WITH_AUTHINFO) << "HksGetParam secure sign type failed.";

    HksFreeParamSet(&keyParamSet);
    return ret;
}

static int32_t BuildImportKeyParamsForRsa(struct HksTestSecureSignImportParams *importParams, bool isAuth,
    bool isClearPasswordInvalid)
{
    if (isClearPasswordInvalid) {
        importParams->inputParams = g_importRsaKeyParamsWithBioAndClearPassword;
        importParams->inputParamSize = sizeof(g_importRsaKeyParamsWithBioAndClearPassword) /
            sizeof(g_importRsaKeyParamsWithBioAndClearPassword[0]);
    } else {
        importParams->inputParams = isAuth ? g_importRsaKeyParams : g_importRsaKeyParamsNoAuth;
        importParams->inputParamSize = isAuth ? sizeof(g_importRsaKeyParams)/sizeof(g_importRsaKeyParams[0]) :
            sizeof(g_importRsaKeyParamsNoAuth)/sizeof(g_importRsaKeyParamsNoAuth[0]);
    }
    
    importParams->expectResult = HKS_SUCCESS;
    uint8_t *keyBuffer = (uint8_t *)HksMalloc(MAX_KEY_SIZE);
    if (keyBuffer == nullptr) {
        return HKS_ERROR_MALLOC_FAIL;
    }
    importParams->importKey.data = keyBuffer;
    importParams->importKey.size = MAX_KEY_SIZE;
    struct HksBlob nDataBlob = { sizeof(g_nData2048), (uint8_t *)g_nData2048 };
    struct HksBlob dData2048 = { sizeof(g_dData2048), (uint8_t *)g_dData2048 };
    struct HksBlob eData = { sizeof(g_eData), (uint8_t *)g_eData };
    int32_t ret = Unittest::HksAccessControlPartTest::ConstructRsaKeyPair(&nDataBlob, &dData2048, &eData,
        HKS_RSA_KEY_SIZE_2048, &importParams->importKey);
    if (ret != HKS_SUCCESS) {
        HksFree(keyBuffer);
    }
    return ret;
}

static int32_t BuildImportKeyParamsForDSA(struct HksTestSecureSignImportParams *importParams, bool isAuth)
{
    importParams->inputParams = isAuth ? g_importDsaKeyParams : g_importDsaKeyParamsNoAuthInfo;
    importParams->inputParamSize = isAuth ? sizeof(g_importDsaKeyParams)/sizeof(g_importDsaKeyParams[0]) :
        sizeof(g_importDsaKeyParamsNoAuthInfo)/sizeof(g_importDsaKeyParamsNoAuthInfo[0]);
    importParams->expectResult = HKS_SUCCESS;
    uint8_t *keyBuffer = (uint8_t *)HksMalloc(MAX_KEY_SIZE);
    if (keyBuffer == nullptr) {
        return HKS_ERROR_MALLOC_FAIL;
    }
    importParams->importKey.data = keyBuffer;
    importParams->importKey.size = MAX_KEY_SIZE;
    struct HksBlob xData = { sizeof(g_xData), (uint8_t *)g_xData };
    struct HksBlob yData = { sizeof(g_yData), (uint8_t *)g_yData };
    struct HksBlob pData = { sizeof(g_pData), (uint8_t *)g_pData };
    struct HksBlob qData = { sizeof(g_qData), (uint8_t *)g_qData };
    struct HksBlob gData = { sizeof(g_gData), (uint8_t *)g_gData };
    struct TestDsaKeyParams dsaKeyParams = {
        .xData = &xData,
        .yData = &yData,
        .pData = &pData,
        .qData = &qData,
        .gData = &gData
    };
    int32_t ret = Unittest::HksAccessControlPartTest::ConstructDsaKeyPair(HKS_RSA_KEY_SIZE_2048, &dsaKeyParams,
        &importParams->importKey);
    if (ret != HKS_SUCCESS) {
        HksFree(keyBuffer);
    }
    return HKS_SUCCESS;
}

static int32_t BuildImportKeyTestParams(struct HksTestSecureSignImportParams *importParams, uint32_t alg,
    bool isAuth, bool isClearPasswordInvalid)
{
    importParams->keyAlias = isAuth ? &g_importKeyAlias : &g_importKeyNoAuthAlias;
    int32_t ret;
    switch (alg) {
        case HKS_ALG_RSA: {
            ret = BuildImportKeyParamsForRsa(importParams, isAuth, isClearPasswordInvalid);
            return ret;
        }
        case HKS_ALG_DSA: {
            ret = BuildImportKeyParamsForDSA(importParams, isAuth);
            return ret;
        }
            break;
        case HKS_ALG_ED25519: {
            importParams->inputParams = isAuth ? g_importKeyEd25519Params : g_importKeyEd25519ParamsNoAuth;
            importParams->inputParamSize = isAuth ? sizeof(g_importKeyEd25519Params)/sizeof(g_importKeyEd25519Params[0])
                : sizeof(g_importKeyEd25519ParamsNoAuth) / sizeof(g_importKeyEd25519ParamsNoAuth[0]);
            importParams->expectResult = HKS_SUCCESS;
            uint8_t *keyBuffer = (uint8_t *)HksMalloc(MAX_KEY_SIZE);
            if (keyBuffer == nullptr) {
                return HKS_ERROR_MALLOC_FAIL;
            }
            importParams->importKey.data = keyBuffer;
            importParams->importKey.size = MAX_KEY_SIZE;
            struct HksBlob ed25519PubData = { sizeof(g_ed25519PubData), (uint8_t *)g_ed25519PubData };
            struct HksBlob ed25519PriData = { sizeof(g_ed25519PriData), (uint8_t *)g_ed25519PriData };
            ret = ConstructEd25519KeyPair(HKS_CURVE25519_KEY_SIZE_256, HKS_ALG_ED25519, &ed25519PubData,
                &ed25519PriData, &importParams->importKey);
            if (ret != HKS_SUCCESS) {
                HksFree(keyBuffer);
            }
            return ret;
        }
            break;
        default:
                break;
    }
    return HKS_FAILURE;
}

static int32_t BuildUpdateFinishParams(struct HksTestSecureSignVerifyUpdateFinishParams *updateFinishParams,
    uint32_t alg, bool isThreeStage)
{
    updateFinishParams->keyAlias = &g_importKeyAlias;
    updateFinishParams->keyAliasNoAuth = &g_importKeyNoAuthAlias;
    updateFinishParams->isThreeStageUse = isThreeStage;
    g_outDataBlob.data = g_outBuffer;
    g_outDataBlob.size = sizeof(g_outBuffer);
    updateFinishParams->outBuffer = &g_outDataBlob;
    updateFinishParams->inData = isThreeStage ? &g_inDataBlob : &g_inDataBlobTwoStage;
    switch (alg) {
        case HKS_ALG_RSA: {
            updateFinishParams->updateParams = g_signParamsTestRsa;
            updateFinishParams->inputParamSize = HKS_ARRAY_SIZE(g_signParamsTestRsa);
            updateFinishParams->expectResult = HKS_SUCCESS;
            return HKS_SUCCESS;
        }
        case HKS_ALG_ED25519: {
            updateFinishParams->updateParams = g_signParamsTestEd25519;
            updateFinishParams->inputParamSize = HKS_ARRAY_SIZE(g_signParamsTestEd25519);
            updateFinishParams->expectResult = HKS_SUCCESS;
            return HKS_SUCCESS;
        }
        case HKS_ALG_DSA: {
            updateFinishParams->updateParams = g_signParamsTestDsa;
            updateFinishParams->inputParamSize = HKS_ARRAY_SIZE(g_signParamsTestDsa);
            updateFinishParams->expectResult = HKS_SUCCESS;
            return HKS_SUCCESS;
        }
        default:
            break;
    }
    return HKS_FAILURE;
}

static int32_t BuildUpdateFinishVerifyParams(struct HksTestSecureSignVerifyUpdateFinishParams *updateFinishParams,
    uint32_t alg, bool isThreeStage, struct HksBlob *inData, struct HksBlob *signature)
{
    updateFinishParams->keyAlias = &g_importKeyAlias;
    updateFinishParams->keyAliasNoAuth = &g_importKeyNoAuthAlias;
    updateFinishParams->isThreeStageUse = isThreeStage;
    g_outDataBlob.data = g_outBuffer;
    g_outDataBlob.size = sizeof(g_outBuffer);
    updateFinishParams->outBuffer = &g_outDataBlob;
    updateFinishParams->inData = inData;
    updateFinishParams->signature = signature;
    switch (alg) {
        case HKS_ALG_RSA: {
            updateFinishParams->updateParams = g_verifyParamsTestRsa;
            updateFinishParams->inputParamSize = HKS_ARRAY_SIZE(g_verifyParamsTestRsa);
            updateFinishParams->expectResult = HKS_SUCCESS;
            return HKS_SUCCESS;
        }
        case HKS_ALG_ED25519: {
            updateFinishParams->updateParams = g_verifyParamsTestEd25519;
            updateFinishParams->inputParamSize = HKS_ARRAY_SIZE(g_verifyParamsTestEd25519);
            updateFinishParams->expectResult = HKS_SUCCESS;
            return HKS_SUCCESS;
        }
        case HKS_ALG_DSA: {
            updateFinishParams->updateParams = g_verifyParamsTestDsa;
            updateFinishParams->inputParamSize = HKS_ARRAY_SIZE(g_verifyParamsTestDsa);
            updateFinishParams->expectResult = HKS_SUCCESS;
            return HKS_SUCCESS;
        }
        default:
            break;
    }
    return HKS_FAILURE;
}

static void TestGenerateKeyWithSecureSignTag(struct HksTestSecureSignGenParams *params)
{
    struct HksParamSet *genParamSet = NULL;
    int32_t ret = InitParamSet(&genParamSet, params->inputParams, params->inputParamSize);
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet failed.";

    /**
     * @tc.steps:step1. Generate a key with user_auth_type and sign_with_info tag
     */
    ret = HksGenerateKey(params->keyAlias, genParamSet, nullptr);
    EXPECT_EQ(ret, HKS_SUCCESS) << "HksGenerateKey rsa key failed.";

    /**
     * @tc.steps:step2. Get key paramSet check if related key tag exist
     */
    ret = CheckSignWithInfoTag(params->keyAlias, genParamSet);
    EXPECT_EQ(ret, HKS_SUCCESS) << "CheckSignWithInfoTag rsa key failed.";

    /**
     * @tc.steps:step3. Delete key and free paramSet
     */
    HksDeleteKey(params->keyAlias, nullptr);
    HksFreeParamSet(&genParamSet);
}

int32_t TestImportKeyWithSecureSignTag(struct HksTestSecureSignImportParams *params, bool ifCheckTag)
{
    struct HksParamSet *importParams = nullptr;
    int32_t ret = InitParamSet(&importParams, params->inputParams, params->inputParamSize);
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet failed.";

    /**
     * @tc.steps:step1. Import a key with user_auth_type and sign_with_info tag
     */
    ret = HksImportKey(params->keyAlias, importParams, &params->importKey);
    EXPECT_EQ(ret, HKS_SUCCESS) << "HksImportKey key failed.";

    if (ifCheckTag) {
        /**
         * @tc.steps:step2. Get key paramSet check if related key tag exist
         */
        ret = CheckSignWithInfoTag(params->keyAlias, importParams);
        EXPECT_EQ(ret, HKS_SUCCESS) << "CheckSignWithInfoTag rsa key failed.";
    }

    /**
     * @tc.steps:step3. Free paramSet
     */
    HksFreeParamSet(&importParams);
    return ret;
}

int32_t HksTestUpdateFinishSignAuthInfo(struct HksTestSecureSignVerifyUpdateFinishParams *updateFinishParams,
    struct HksTestGenAuthTokenParams *genAuthTokenParams)
{
    uint8_t tmpHandle[sizeof(uint64_t)] = {0};
    struct HksBlob handle = { sizeof(uint64_t), tmpHandle };
    uint8_t tmpChallenge[TOKEN_SIZE] = {0};
    struct HksBlob challenge = { sizeof(tmpChallenge), tmpChallenge };

    struct HksParamSet *paramSet = nullptr;
    int32_t ret = InitParamSet(&paramSet, updateFinishParams->updateParams, updateFinishParams->inputParamSize);
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet failed.";
    if (ret != HKS_SUCCESS) {
        return HKS_FAILURE;
    }

    ret = HksInit(updateFinishParams->keyAlias, paramSet, &handle, &challenge);
    EXPECT_EQ(ret, HKS_SUCCESS) << "Init failed.";
    if (ret != HKS_SUCCESS) {
        HksFreeParamSet(&paramSet);
        return HKS_FAILURE;
    }

    genAuthTokenParams->authChallenge = &challenge;
    
    struct HksParamSet *newParamSet = nullptr;
    ret = HksBuildAuthTokenSecure(paramSet, genAuthTokenParams, &newParamSet);
    EXPECT_EQ(ret, HKS_SUCCESS) << "HksBuildAuthTokenSecure failed.";
    if (ret != HKS_SUCCESS) {
        HksFreeParamSet(&paramSet);
        return HKS_FAILURE;
    }

    struct HksParam *tmpParam = NULL;
    ret = HksGetParam(newParamSet, HKS_TAG_PURPOSE, &tmpParam);
    if (ret != HKS_SUCCESS) {
        HksFreeParamSet(&paramSet);
        HksFreeParamSet(&newParamSet);
        HKS_LOG_E("get tag purpose failed.");
        return HKS_FAILURE;
    }

    if (updateFinishParams->isThreeStageUse) {
        ret = TestUpdateFinish(&handle, newParamSet, tmpParam->uint32Param, updateFinishParams->inData,
            updateFinishParams->outBuffer);
    } else {
        ret = HksFinish(&handle, newParamSet, updateFinishParams->inData, updateFinishParams->outBuffer);
    }
    EXPECT_EQ(ret, HKS_SUCCESS) << "TestUpdateFinish failed.";
    if (ret != HKS_SUCCESS) {
        HksFreeParamSet(&paramSet);
        HksFreeParamSet(&newParamSet);
        return HKS_FAILURE;
    }
    
    HksFreeParamSet(&paramSet);
    HksFreeParamSet(&newParamSet);
    return ret;
}

int32_t VerifyUpdateFinish(struct HksBlob *handle, struct HksParamSet *newParamSet, struct HksParam *purposeParam,
    struct HksTestSecureSignVerifyUpdateFinishParams *updateFinishParams, bool isSign)
{
    int32_t ret;
    if (isSign) {
        if (updateFinishParams->isThreeStageUse) {
            ret = TestUpdateFinish(handle, newParamSet, purposeParam->uint32Param, updateFinishParams->inData,
                updateFinishParams->outBuffer);
        } else {
            ret = HksFinish(handle, newParamSet, updateFinishParams->inData, updateFinishParams->outBuffer);
        }
    } else {
        if (updateFinishParams->isThreeStageUse) {
            ret = TestUpdateFinish(handle, newParamSet, purposeParam->uint32Param, updateFinishParams->inData,
                updateFinishParams->signature);
        } else {
            ret = HksFinish(handle, newParamSet, updateFinishParams->inData, updateFinishParams->signature);
        }
    }
    return ret;
}

int32_t HksTestUpdateFinishVerifySignAuthInfo(struct HksTestSecureSignVerifyUpdateFinishParams *updateFinishParams,
    struct HksTestGenAuthTokenParams *genAuthTokenParams, bool isSign)
{
    uint8_t tmpHandle[sizeof(uint64_t)] = {0};
    struct HksBlob handle = { sizeof(uint64_t), tmpHandle };
    uint8_t tmpChallenge[TOKEN_SIZE] = {0};
    struct HksBlob challenge = { sizeof(tmpChallenge), tmpChallenge };

    struct HksParamSet *paramSet = nullptr;
    int32_t ret = InitParamSet(&paramSet, updateFinishParams->updateParams, updateFinishParams->inputParamSize);
    if (ret != HKS_SUCCESS) {
        return HKS_FAILURE;
    }

    ret = HksInit(updateFinishParams->keyAlias, paramSet, &handle, &challenge);
    if (ret != HKS_SUCCESS) {
        HksFreeParamSet(&paramSet);
        return HKS_FAILURE;
    }

    genAuthTokenParams->authChallenge = &challenge;

    struct HksParamSet *newParamSet = nullptr;
    ret = HksBuildAuthTokenSecure(paramSet, genAuthTokenParams, &newParamSet);
    if (ret != HKS_SUCCESS) {
        HksFreeParamSet(&paramSet);
        return HKS_FAILURE;
    }

    struct HksParam *purposeParam = NULL;
    ret = HksGetParam(newParamSet, HKS_TAG_PURPOSE, &purposeParam);
    if (ret != HKS_SUCCESS) {
        HksFreeParamSet(&paramSet);
        HksFreeParamSet(&newParamSet);
        HKS_LOG_E("get tag purpose failed.");
        return HKS_FAILURE;
    }

    ret = VerifyUpdateFinish(&handle, newParamSet, purposeParam, updateFinishParams, isSign);

    EXPECT_EQ(ret, HKS_SUCCESS) << "TestUpdateFinish failed.";

    if (ret != HKS_SUCCESS) {
        HksFreeParamSet(&paramSet);
        HksFreeParamSet(&newParamSet);
        return HKS_FAILURE;
    }
    
    return ret;
}

static const uint32_t g_fingerPrintInUserIam = 4;
static const uint32_t g_pinInUserIam = 1;

static void BuildAuthTokenParams(struct HksTestGenAuthTokenParams *authTokenParams, bool isClearPasswordInvalid)
{
    if (isClearPasswordInvalid) {
        authTokenParams->secureUid = g_secureUid;
        authTokenParams->enrolledId = g_enrolledIdFinger;
        authTokenParams->credentialId = g_credentialId;
        authTokenParams->time = g_time;
        authTokenParams->authType = g_fingerPrintInUserIam;
        return;
    }
    authTokenParams->secureUid = g_secureUid;
    authTokenParams->enrolledId = g_enrolledIdPin;
    authTokenParams->credentialId = g_credentialId;
    authTokenParams->time = g_time;
    authTokenParams->authType = g_pinInUserIam;
}

static int32_t BuildSigAndIndataBlob(struct HksBlob *sigBlob, struct HksBlob *inDataBlob,
    struct HksTestSecureSignVerifyUpdateFinishParams *secureSignUpdateFinish)
{
    int32_t ret = memcpy_s(sigBlob->data, sigBlob->size,
        secureSignUpdateFinish->outBuffer->data + g_authHeadSize,
        secureSignUpdateFinish->outBuffer->size - g_authHeadSize);
    if (ret != EOK) {
        return ret;
    }
    ret = memcpy_s(inDataBlob->data, inDataBlob->size,
        secureSignUpdateFinish->outBuffer->data, g_authHeadSize);
    if (ret != EOK) {
        return ret;
    }
    ret = memcpy_s(inDataBlob->data + g_authHeadSize, inDataBlob->size - g_authHeadSize,
        secureSignUpdateFinish->inData->data, secureSignUpdateFinish->inData->size);
    return ret;
}

static void TestImportKeyWithSignTagAndTestUseKeyCommonCase(uint32_t alg, bool isThreeStage,
    bool isClearPasswordInvalid)
{
    /**
     * @tc.steps:step1. import a key with user_auth_type and sign_with_info tag
     */
    struct HksTestSecureSignImportParams importParams;
    (void)memset_s((uint8_t *)&importParams, sizeof(struct HksTestSecureSignImportParams), 0,
        sizeof(struct HksTestSecureSignImportParams));
    int32_t ret = BuildImportKeyTestParams(&importParams, alg, true, isClearPasswordInvalid);
    EXPECT_EQ(ret, HKS_SUCCESS) << "BuildImportKeyTestParams failed.";
    if (ret != HKS_SUCCESS) {
        return;
    }

    ret = TestImportKeyWithSecureSignTag(&importParams, true);
    EXPECT_EQ(ret, HKS_SUCCESS) << "TestImportKeyWithSecureSignTag failed.";
    if (ret != HKS_SUCCESS) {
        return;
    }

    /**
     * @tc.steps:step2. Import a key without user_auth_type and sign_with_info tag
     */
    struct HksTestSecureSignImportParams importParamsWithoutSignAuth;
    (void)memset_s((uint8_t *)&importParamsWithoutSignAuth, sizeof(struct HksTestSecureSignImportParams), 0,
        sizeof(struct HksTestSecureSignImportParams));
    ret = BuildImportKeyTestParams(&importParamsWithoutSignAuth, alg, false, isClearPasswordInvalid);
    EXPECT_EQ(ret, HKS_SUCCESS) << "BuildImportKeyTestParams without sign auth failed.";
    if (ret != HKS_SUCCESS) {
        return;
    }

    /**
     * @tc.steps:step3. use the key to sign:init update finish. check the sign data whether equals the expected data
     */
    ret = TestImportKeyWithSecureSignTag(&importParamsWithoutSignAuth, false);
    EXPECT_EQ(ret, HKS_SUCCESS) << "TestImportKeyWithSecureSignTag2 failed.";
    struct HksTestSecureSignVerifyUpdateFinishParams secureSignUpdateFinish;
    ret = BuildUpdateFinishParams(&secureSignUpdateFinish, alg, isThreeStage);
    EXPECT_EQ(ret, HKS_SUCCESS) << "BuildUpdateFinishParams failed.";

    struct HksTestGenAuthTokenParams genAuthTokenParams = { 0 };
    BuildAuthTokenParams(&genAuthTokenParams, isClearPasswordInvalid);

    ret = HksTestUpdateFinishVerifySignAuthInfo(&secureSignUpdateFinish, &genAuthTokenParams, true);
    EXPECT_EQ(ret, secureSignUpdateFinish.expectResult) << "HksTestUpdateFinishSignAuthInfo failed.";

    uint8_t sigature[secureSignUpdateFinish.outBuffer->size - g_authHeadSize];
    struct HksBlob sigBlob = {secureSignUpdateFinish.outBuffer->size - g_authHeadSize, sigature};
    uint8_t inData[secureSignUpdateFinish.inData->size + g_authHeadSize];
    struct HksBlob inDataBlob = {secureSignUpdateFinish.inData->size + g_authHeadSize, inData};
    ret = BuildSigAndIndataBlob(&sigBlob, &inDataBlob, &secureSignUpdateFinish);
    EXPECT_EQ(ret, HKS_SUCCESS) << "BuildSigAndIndataBlob failed.";

    struct HksTestSecureSignVerifyUpdateFinishParams secureSignUpdateFinishVerify;
    ret = BuildUpdateFinishVerifyParams(&secureSignUpdateFinishVerify, alg, true, &inDataBlob, &sigBlob);
    EXPECT_EQ(ret, HKS_SUCCESS) << "BuildUpdateFinishVerifyParams failed.";
    ret = HksTestUpdateFinishVerifySignAuthInfo(&secureSignUpdateFinishVerify, &genAuthTokenParams, false);

    HKS_FREE_BLOB(importParams.importKey);
    HKS_FREE_BLOB(importParamsWithoutSignAuth.importKey);
    HksDeleteKey(secureSignUpdateFinish.keyAlias, nullptr);
    HksDeleteKey(secureSignUpdateFinish.keyAliasNoAuth, nullptr);
}

/**
 * @tc.name: HksAccessControlSecureSignTest001
 * @tc.desc: normal case to test generate a rsa key with sign_with_info tag and check paramSet
 *           has the tag
 * @tc.type: FUNC
 * @tc.require: issueI5NY0M
 */
HWTEST_F(HksAccessControlSecureSignTest, HksAccessControlSecureSignTest001, TestSize.Level0)
{
    HKS_LOG_E("enter HksAccessControlSecureSignTest001");
    TestGenerateKeyWithSecureSignTag(&g_testRsaGenParams);
}

/**
 * @tc.name: HksAccessControlSecureSignTest002
 * @tc.desc: normal case to test generate a ed25519 key with sign_with_info tag and check paramSet
 *           has the tag
 * @tc.type: FUNC
 * @tc.require: issueI5NY0M
 */
HWTEST_F(HksAccessControlSecureSignTest, HksAccessControlSecureSignTest002, TestSize.Level0)
{
    HKS_LOG_E("enter HksAccessControlSecureSignTest002");
    TestGenerateKeyWithSecureSignTag(&g_testEd25519GenParams);
}

/**
 * @tc.name: HksAccessControlSecureSignTest003
 * @tc.desc: normal case to test generate a dsa key with sign_with_info tag and check paramSet
 *           has the tag
 * @tc.type: FUNC
 * @tc.require: issueI5NY0M
 */
HWTEST_F(HksAccessControlSecureSignTest, HksAccessControlSecureSignTest003, TestSize.Level0)
{
#ifdef _USE_OPENSSL_
    HKS_LOG_E("enter HksAccessControlSecureSignTest003");
    TestGenerateKeyWithSecureSignTag(&g_testDsaGenParams);
#endif
}

/**
 * @tc.name: HksAccessControlSecureSignTest004
 * @tc.desc: normal case to test import a rsa key with user auth type and use the key
 *           to sign data, check the signature whether equals the expected
 * @tc.type: FUNC
 * @tc.require: issueI5NY0M
 */
HWTEST_F(HksAccessControlSecureSignTest, HksAccessControlSecureSignTest004, TestSize.Level0)
{
    HKS_LOG_E("enter HksAccessControlSecureSignTest004");
    TestImportKeyWithSignTagAndTestUseKeyCommonCase(HKS_ALG_RSA, true, false);
}

/**
 * @tc.name: HksAccessControlSecureSignTest005
 * @tc.desc: normal case to test import a dsa key with user auth type and use the key
 *           to sign data, check the signature whether equals the expected
 * @tc.type: FUNC
 * @tc.require: issueI5NY0M
 */
HWTEST_F(HksAccessControlSecureSignTest, HksAccessControlSecureSignTest005, TestSize.Level0)
{
#ifdef _USE_OPENSSL_
    HKS_LOG_E("enter HksAccessControlSecureSignTest005");
    TestImportKeyWithSignTagAndTestUseKeyCommonCase(HKS_ALG_DSA, true, false);
#endif
}

/**
 * @tc.name: HksAccessControlSecureSignTest006
 * @tc.desc: normal case to test import a rsa key with user auth type and use the key:init&finish
 *           to sign data, check the signature whether equals the expected
 * @tc.type: FUNC
 * @tc.require: issueI5NY0M
 */
HWTEST_F(HksAccessControlSecureSignTest, HksAccessControlSecureSignTest006, TestSize.Level0)
{
    HKS_LOG_E("enter HksAccessControlSecureSignTest006");
    TestImportKeyWithSignTagAndTestUseKeyCommonCase(HKS_ALG_RSA, false, false);
}

/**
 * @tc.name: HksAccessControlSecureSignTest007
 * @tc.desc: normal case to test import a rsa key with auth type as fingerprint and access type as invalid clear
 *           password, check the signature whether equals the expected
 * @tc.type: FUNC
 * @tc.require: issueI5NY0M
 */
HWTEST_F(HksAccessControlSecureSignTest, HksAccessControlSecureSignTest007, TestSize.Level0)
{
    HKS_LOG_E("enter HksAccessControlSecureSignTest007");
    TestImportKeyWithSignTagAndTestUseKeyCommonCase(HKS_ALG_RSA, true, true);
}
}
