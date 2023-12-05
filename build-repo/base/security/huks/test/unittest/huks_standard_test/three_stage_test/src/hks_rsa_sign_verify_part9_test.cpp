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

#include "hks_rsa_sign_verify_part9_test.h"
#include "hks_log.h"
#include "hks_rsa_sign_verify_test_common.h"

#include <gtest/gtest.h>

using namespace testing::ext;
namespace Unittest::RsaSignVerify {
class HksRsaSignVerifyPart9Test : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};

void HksRsaSignVerifyPart9Test::SetUpTestCase(void)
{
}

void HksRsaSignVerifyPart9Test::TearDownTestCase(void)
{
}

void HksRsaSignVerifyPart9Test::SetUp()
{
    EXPECT_EQ(HksInitialize(), 0);
}

void HksRsaSignVerifyPart9Test::TearDown()
{
}

static struct HksParam g_genParamsTest081[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_RSA
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_SIGN | HKS_KEY_PURPOSE_VERIFY
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_RSA_KEY_SIZE_512
    }
};
static struct HksParam g_signParamsTest081[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_RSA
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_SIGN
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_RSA_KEY_SIZE_512
    }, {
        .tag = HKS_TAG_PADDING,
        .uint32Param = HKS_PADDING_PSS
    }, {
        .tag = HKS_TAG_DIGEST,
        .uint32Param = HKS_DIGEST_MD5
    }
};
static struct HksParam g_verifyParamsTest081[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_RSA
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_VERIFY
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_RSA_KEY_SIZE_512
    }, {
        .tag = HKS_TAG_PADDING,
        .uint32Param = HKS_PADDING_PSS
    }, {
        .tag = HKS_TAG_DIGEST,
        .uint32Param = HKS_DIGEST_MD5
    }
};

static struct HksParam g_genParamsTest082[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_RSA
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_SIGN | HKS_KEY_PURPOSE_VERIFY
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_RSA_KEY_SIZE_768
    }
};
static struct HksParam g_signParamsTest082[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_RSA
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_SIGN
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_RSA_KEY_SIZE_768
    }, {
        .tag = HKS_TAG_PADDING,
        .uint32Param = HKS_PADDING_PSS
    }, {
        .tag = HKS_TAG_DIGEST,
        .uint32Param = HKS_DIGEST_SHA1
    }
};
static struct HksParam g_verifyParamsTest082[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_RSA
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_VERIFY
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_RSA_KEY_SIZE_768
    }, {
        .tag = HKS_TAG_PADDING,
        .uint32Param = HKS_PADDING_PSS
    }, {
        .tag = HKS_TAG_DIGEST,
        .uint32Param = HKS_DIGEST_SHA1
    }
};

static struct HksParam g_genParamsTest083[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_RSA
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_SIGN | HKS_KEY_PURPOSE_VERIFY
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_RSA_KEY_SIZE_1024
    }
};
static struct HksParam g_signParamsTest083[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_RSA
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_SIGN
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_RSA_KEY_SIZE_1024
    }, {
        .tag = HKS_TAG_PADDING,
        .uint32Param = HKS_PADDING_PSS
    }, {
        .tag = HKS_TAG_DIGEST,
        .uint32Param = HKS_DIGEST_SHA224
    }
};
static struct HksParam g_verifyParamsTest083[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_RSA
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_VERIFY
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_RSA_KEY_SIZE_1024
    }, {
        .tag = HKS_TAG_PADDING,
        .uint32Param = HKS_PADDING_PSS
    }, {
        .tag = HKS_TAG_DIGEST,
        .uint32Param = HKS_DIGEST_SHA224
    }
};

static struct HksParam g_genParamsTest084[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_RSA
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_SIGN | HKS_KEY_PURPOSE_VERIFY
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_RSA_KEY_SIZE_2048
    }
};
static struct HksParam g_signParamsTest084[] = {
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
static struct HksParam g_verifyParamsTest084[] = {
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

static struct HksParam g_genParamsTest085[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_RSA
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_SIGN | HKS_KEY_PURPOSE_VERIFY
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_RSA_KEY_SIZE_3072
    }
};
static struct HksParam g_signParamsTest085[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_RSA
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_SIGN
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_RSA_KEY_SIZE_3072
    }, {
        .tag = HKS_TAG_PADDING,
        .uint32Param = HKS_PADDING_PKCS1_V1_5
    }, {
        .tag = HKS_TAG_DIGEST,
        .uint32Param = HKS_DIGEST_SHA384
    }
};
static struct HksParam g_verifyParamsTest085[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_RSA
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_VERIFY
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_RSA_KEY_SIZE_3072
    }, {
        .tag = HKS_TAG_PADDING,
        .uint32Param = HKS_PADDING_PKCS1_V1_5
    }, {
        .tag = HKS_TAG_DIGEST,
        .uint32Param = HKS_DIGEST_SHA384
    }
};

static struct HksParam g_genParamsTest086[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_RSA
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_SIGN | HKS_KEY_PURPOSE_VERIFY
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_RSA_KEY_SIZE_4096
    }
};
static struct HksParam g_signParamsTest086[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_RSA
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_SIGN
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_RSA_KEY_SIZE_4096
    }, {
        .tag = HKS_TAG_PADDING,
        .uint32Param = HKS_PADDING_PKCS1_V1_5
    }, {
        .tag = HKS_TAG_DIGEST,
        .uint32Param = HKS_DIGEST_SHA512
    }
};
static struct HksParam g_verifyParamsTest086[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_RSA
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_VERIFY
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_RSA_KEY_SIZE_4096
    }, {
        .tag = HKS_TAG_PADDING,
        .uint32Param = HKS_PADDING_PKCS1_V1_5
    }, {
        .tag = HKS_TAG_DIGEST,
        .uint32Param = HKS_DIGEST_SHA512
    }
};

static struct HksParam g_genParamsTest087[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_RSA
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_SIGN | HKS_KEY_PURPOSE_VERIFY
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_RSA_KEY_SIZE_1024
    }
};

static struct HksParam g_signParamsTest087[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_RSA
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_SIGN
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_RSA_KEY_SIZE_1024
    }, {
        .tag = HKS_TAG_PADDING,
        .uint32Param = HKS_PADDING_PKCS1_V1_5
    }, {
        .tag = HKS_TAG_DIGEST,
        .uint32Param = HKS_DIGEST_NONE
    }
};
static struct HksParam g_verifyParamsTest087[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_RSA
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_VERIFY
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_RSA_KEY_SIZE_1024
    }, {
        .tag = HKS_TAG_PADDING,
        .uint32Param = HKS_PADDING_PKCS1_V1_5
    }, {
        .tag = HKS_TAG_DIGEST,
        .uint32Param = HKS_DIGEST_NONE
    }
};

static struct HksParam g_genParamsTest088[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_RSA
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_SIGN | HKS_KEY_PURPOSE_VERIFY
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_RSA_KEY_SIZE_1024
    }
};

static struct HksParam g_signParamsTest088[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_RSA
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_SIGN
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_RSA_KEY_SIZE_1024
    }
};
static struct HksParam g_verifyParamsTest088[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_RSA
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_VERIFY
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_RSA_KEY_SIZE_1024
    }
};

static struct HksParam g_genParamsTest089[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_RSA
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_SIGN | HKS_KEY_PURPOSE_VERIFY
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_RSA_KEY_SIZE_1024
    }
};

static struct HksParam g_signParamsTest089[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_RSA
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_SIGN
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_RSA_KEY_SIZE_1024
    }, {
        .tag = HKS_TAG_PADDING,
        .uint32Param = HKS_PADDING_PSS
    }, {
        .tag = HKS_TAG_DIGEST,
        .uint32Param = HKS_DIGEST_NONE
    }
};
static struct HksParam g_verifyParamsTest089[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_RSA
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_VERIFY
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_RSA_KEY_SIZE_1024
    }, {
        .tag = HKS_TAG_PADDING,
        .uint32Param = HKS_PADDING_PSS
    }, {
        .tag = HKS_TAG_DIGEST,
        .uint32Param = HKS_DIGEST_NONE
    }
};

static struct HksParam g_genParamsTest090[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_RSA
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_SIGN | HKS_KEY_PURPOSE_VERIFY
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_RSA_KEY_SIZE_1024
    }
};

static struct HksParam g_signParamsTest090[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_RSA
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_SIGN
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_RSA_KEY_SIZE_1024
    }, {
        .tag = HKS_TAG_PADDING,
        .uint32Param = HKS_PADDING_NONE
    }, {
        .tag = HKS_TAG_DIGEST,
        .uint32Param = HKS_DIGEST_NONE
    }
};
static struct HksParam g_verifyParamsTest090[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_RSA
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_VERIFY
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_RSA_KEY_SIZE_1024
    }, {
        .tag = HKS_TAG_PADDING,
        .uint32Param = HKS_PADDING_NONE
    }, {
        .tag = HKS_TAG_DIGEST,
        .uint32Param = HKS_DIGEST_NONE
    }
};

/**
 * @tc.name: HksRsaSignVerifyPart9Test.HksRsaSignVerifyPart9Test081
 * @tc.desc: rsa sign and verify; alg-RSA, pad-PSS and digest-MD5.
 * @tc.type: FUNC
 * @tc.require:issueI611S5
 */
HWTEST_F(HksRsaSignVerifyPart9Test, HksRsaSignVerifyPart9Test081, TestSize.Level1)
{
    HKS_LOG_E(" Enter HksRsaSignVerifyPart9Test081");
    int32_t ret = HKS_FAILURE;
    const char *keyAliasString = "HksRSASignVerifyKeyAliasTest081";
    struct HksParamSet *genParamSet = nullptr;
    struct HksParamSet *signParamSet = nullptr;
    struct HksParamSet *verifyParamSet = nullptr;

    ret = InitParamSet(&genParamSet, g_genParamsTest081, sizeof(g_genParamsTest081) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet failed.";
    ret = InitParamSet(&signParamSet, g_signParamsTest081, sizeof(g_signParamsTest081) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet failed.";
    ret = InitParamSet(&verifyParamSet, g_verifyParamsTest081, sizeof(g_verifyParamsTest081) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet failed.";

    /* Generate Key */
    struct HksBlob keyAlias = {strlen(keyAliasString), (uint8_t *)keyAliasString};

    if ((genParamSet != nullptr) || (signParamSet != nullptr) || (verifyParamSet != nullptr)) {
        ret = HksRsaSignVerifyTestNormalAnotherCase(keyAlias, genParamSet, signParamSet, verifyParamSet);
    }

    /* Delete Key */
    ret = HksDeleteKey(&keyAlias, genParamSet);
    EXPECT_EQ(ret, HKS_SUCCESS) << "DeleteKey failed.";

    HksFreeParamSet(&genParamSet);
    HksFreeParamSet(&signParamSet);
    HksFreeParamSet(&verifyParamSet);
}

/**
 * @tc.name: HksRsaSignVerifyPart9Test.HksRsaSignVerifyPart9Test082
 * @tc.desc: rsa sign and verify; alg-RSA, pad-PSS and digest-SHA1.
 * @tc.type: FUNC
 * @tc.require:issueI611S5
 */
HWTEST_F(HksRsaSignVerifyPart9Test, HksRsaSignVerifyPart9Test082, TestSize.Level1)
{
    HKS_LOG_E(" Enter HksRsaSignVerifyPart9Test082");
    int32_t ret = HKS_FAILURE;
    const char *keyAliasString = "HksRSASignVerifyKeyAliasTest082";
    struct HksParamSet *genParamSet = nullptr;
    struct HksParamSet *signParamSet = nullptr;
    struct HksParamSet *verifyParamSet = nullptr;

    ret = InitParamSet(&genParamSet, g_genParamsTest082, sizeof(g_genParamsTest082) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet failed.";
    ret = InitParamSet(&signParamSet, g_signParamsTest082, sizeof(g_signParamsTest082) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet failed.";
    ret = InitParamSet(&verifyParamSet, g_verifyParamsTest082, sizeof(g_verifyParamsTest082) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet failed.";

    /* Generate Key */
    struct HksBlob keyAlias = {strlen(keyAliasString), (uint8_t *)keyAliasString};

    if ((genParamSet != nullptr) || (signParamSet != nullptr) || (verifyParamSet != nullptr)) {
        ret = HksRsaSignVerifyTestNormalAnotherCase(keyAlias, genParamSet, signParamSet, verifyParamSet);
    }

    /* Delete Key */
    ret = HksDeleteKey(&keyAlias, genParamSet);
    EXPECT_EQ(ret, HKS_SUCCESS) << "DeleteKey failed.";

    HksFreeParamSet(&genParamSet);
    HksFreeParamSet(&signParamSet);
    HksFreeParamSet(&verifyParamSet);
}

/**
 * @tc.name: HksRsaSignVerifyPart9Test.HksRsaSignVerifyPart9Test083
 * @tc.desc: rsa sign and verify; alg-RSA, pad-PSS and digest-SHA224.
 * @tc.type: FUNC
 * @tc.require:issueI611S5
 */
HWTEST_F(HksRsaSignVerifyPart9Test, HksRsaSignVerifyPart9Test083, TestSize.Level1)
{
    HKS_LOG_E(" Enter HksRsaSignVerifyPart9Test083");
    int32_t ret = HKS_FAILURE;
    const char *keyAliasString = "HksRSASignVerifyKeyAliasTest083";
    struct HksParamSet *genParamSet = nullptr;
    struct HksParamSet *signParamSet = nullptr;
    struct HksParamSet *verifyParamSet = nullptr;

    ret = InitParamSet(&genParamSet, g_genParamsTest083, sizeof(g_genParamsTest083) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet failed.";
    ret = InitParamSet(&signParamSet, g_signParamsTest083, sizeof(g_signParamsTest083) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet failed.";
    ret = InitParamSet(&verifyParamSet, g_verifyParamsTest083, sizeof(g_verifyParamsTest083) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet failed.";

    /* Generate Key */
    struct HksBlob keyAlias = {strlen(keyAliasString), (uint8_t *)keyAliasString};

    if ((genParamSet != nullptr) || (signParamSet != nullptr) || (verifyParamSet != nullptr)) {
        ret = HksRsaSignVerifyTestNormalAnotherCase(keyAlias, genParamSet, signParamSet, verifyParamSet);
    }

    /* Delete Key */
    ret = HksDeleteKey(&keyAlias, genParamSet);
    EXPECT_EQ(ret, HKS_SUCCESS) << "DeleteKey failed.";

    HksFreeParamSet(&genParamSet);
    HksFreeParamSet(&signParamSet);
    HksFreeParamSet(&verifyParamSet);
}

/**
 * @tc.name: HksRsaSignVerifyPart9Test.HksRsaSignVerifyPart9Test084
 * @tc.desc: rsa sign and verify; alg-RSA, pad-PSS and digest-SHA256.
 * @tc.type: FUNC
 * @tc.require:issueI611S5
 */
HWTEST_F(HksRsaSignVerifyPart9Test, HksRsaSignVerifyPart9Test084, TestSize.Level1)
{
    HKS_LOG_E(" Enter HksRsaSignVerifyPart9Test084");
    int32_t ret = HKS_FAILURE;
    const char *keyAliasString = "HksRSASignVerifyKeyAliasTest084";
    struct HksParamSet *genParamSet = nullptr;
    struct HksParamSet *signParamSet = nullptr;
    struct HksParamSet *verifyParamSet = nullptr;

    ret = InitParamSet(&genParamSet, g_genParamsTest084, sizeof(g_genParamsTest084) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet failed.";
    ret = InitParamSet(&signParamSet, g_signParamsTest084, sizeof(g_signParamsTest084) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet failed.";
    ret = InitParamSet(&verifyParamSet, g_verifyParamsTest084, sizeof(g_verifyParamsTest084) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet failed.";

    /* Generate Key */
    struct HksBlob keyAlias = {strlen(keyAliasString), (uint8_t *)keyAliasString};

    if ((genParamSet != nullptr) || (signParamSet != nullptr) || (verifyParamSet != nullptr)) {
        ret = HksRsaSignVerifyTestNormalAnotherCase(keyAlias, genParamSet, signParamSet, verifyParamSet);
    }

    /* Delete Key */
    ret = HksDeleteKey(&keyAlias, genParamSet);
    EXPECT_EQ(ret, HKS_SUCCESS) << "DeleteKey failed.";

    HksFreeParamSet(&genParamSet);
    HksFreeParamSet(&signParamSet);
    HksFreeParamSet(&verifyParamSet);
}

/**
 * @tc.name: HksRsaSignVerifyPart9Test.HksRsaSignVerifyPart9Test085
 * @tc.desc: rsa sign and verify; alg-RSA, pad-PKCS1_V1_5 and digest-SHA384.
 * @tc.type: FUNC
 * @tc.require:issueI611S5
 */
HWTEST_F(HksRsaSignVerifyPart9Test, HksRsaSignVerifyPart9Test085, TestSize.Level1)
{
    HKS_LOG_E(" Enter HksRsaSignVerifyPart9Test085");
    int32_t ret = HKS_FAILURE;
    const char *keyAliasString = "HksRSASignVerifyKeyAliasTest085";
    struct HksParamSet *genParamSet = nullptr;
    struct HksParamSet *signParamSet = nullptr;
    struct HksParamSet *verifyParamSet = nullptr;

    ret = InitParamSet(&genParamSet, g_genParamsTest085, sizeof(g_genParamsTest085) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet failed.";
    ret = InitParamSet(&signParamSet, g_signParamsTest085, sizeof(g_signParamsTest085) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet failed.";
    ret = InitParamSet(&verifyParamSet, g_verifyParamsTest085, sizeof(g_verifyParamsTest085) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet failed.";

    /* Generate Key */
    struct HksBlob keyAlias = {strlen(keyAliasString), (uint8_t *)keyAliasString};

    if ((genParamSet != nullptr) || (signParamSet != nullptr) || (verifyParamSet != nullptr)) {
        ret = HksRsaSignVerifyTestNormalAnotherCase(keyAlias, genParamSet, signParamSet, verifyParamSet);
    }

    /* Delete Key */
    ret = HksDeleteKey(&keyAlias, genParamSet);
    EXPECT_EQ(ret, HKS_SUCCESS) << "DeleteKey failed.";

    HksFreeParamSet(&genParamSet);
    HksFreeParamSet(&signParamSet);
    HksFreeParamSet(&verifyParamSet);
}

/**
 * @tc.name: HksRsaSignVerifyPart9Test.HksRsaSignVerifyPart9Test086
 * @tc.desc: rsa sign and verify; alg-RSA, pad-PKCS1_V1_5 and digest-SHA512.
 * @tc.type: FUNC
 * @tc.require:issueI611S5
 */
HWTEST_F(HksRsaSignVerifyPart9Test, HksRsaSignVerifyPart9Test086, TestSize.Level1)
{
    HKS_LOG_E(" Enter HksRsaSignVerifyPart9Test086");
    int32_t ret = HKS_FAILURE;
    const char *keyAliasString = "HksRSASignVerifyKeyAliasTest086";
    struct HksParamSet *genParamSet = nullptr;
    struct HksParamSet *signParamSet = nullptr;
    struct HksParamSet *verifyParamSet = nullptr;

    ret = InitParamSet(&genParamSet, g_genParamsTest086, sizeof(g_genParamsTest086) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet failed.";
    ret = InitParamSet(&signParamSet, g_signParamsTest086, sizeof(g_signParamsTest086) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet failed.";
    ret = InitParamSet(&verifyParamSet, g_verifyParamsTest086, sizeof(g_verifyParamsTest086) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet failed.";

    /* Generate Key */
    struct HksBlob keyAlias = {strlen(keyAliasString), (uint8_t *)keyAliasString};

    if ((genParamSet != nullptr) || (signParamSet != nullptr) || (verifyParamSet != nullptr)) {
        ret = HksRsaSignVerifyTestNormalAnotherCase(keyAlias, genParamSet, signParamSet, verifyParamSet);
    }

    /* Delete Key */
    ret = HksDeleteKey(&keyAlias, genParamSet);
    EXPECT_EQ(ret, HKS_SUCCESS) << "DeleteKey failed.";

    HksFreeParamSet(&genParamSet);
    HksFreeParamSet(&signParamSet);
    HksFreeParamSet(&verifyParamSet);
}

/**
 * @tc.name: HksRsaSignVerifyPart9Test.HksRsaSignVerifyPart9Test087
 * @tc.desc: rsa sign and verify; alg-RSA, pad-PKCS1_V1_5 and digest-MD5.
 * @tc.type: FUNC
 * @tc.require:issueI611S5
 */
HWTEST_F(HksRsaSignVerifyPart9Test, HksRsaSignVerifyPart9Test087, TestSize.Level1)
{
    HKS_LOG_E(" Enter HksRsaSignVerifyPart9Test087");
    int32_t ret = HKS_FAILURE;
    const char *keyAliasString = "HksRSASignVerifyKeyAliasTest087";
    struct HksParamSet *genParamSet = nullptr;
    struct HksParamSet *signParamSet = nullptr;
    struct HksParamSet *verifyParamSet = nullptr;

    ret = InitParamSet(&genParamSet, g_genParamsTest087, sizeof(g_genParamsTest087) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet failed.";
    ret = InitParamSet(&signParamSet, g_signParamsTest087, sizeof(g_signParamsTest087) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet failed.";
    ret = InitParamSet(&verifyParamSet, g_verifyParamsTest087, sizeof(g_verifyParamsTest087) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet failed.";

    /* Generate Key */
    struct HksBlob keyAlias = {strlen(keyAliasString), (uint8_t *)keyAliasString};

    if ((genParamSet != nullptr) || (signParamSet != nullptr) || (verifyParamSet != nullptr)) {
        ret = HksRsaSignVerifyTestNormalAnotherCase(keyAlias, genParamSet, signParamSet, verifyParamSet);
    }

    /* Delete Key */
    ret = HksDeleteKey(&keyAlias, genParamSet);
    EXPECT_EQ(ret, HKS_SUCCESS) << "DeleteKey failed.";

    HksFreeParamSet(&genParamSet);
    HksFreeParamSet(&signParamSet);
    HksFreeParamSet(&verifyParamSet);
}

/**
 * @tc.name: HksRsaSignVerifyPart9Test.HksRsaSignVerifyPart9Test087
 * @tc.desc: rsa sign and verify; alg-RSA.
 * @tc.type: FUNC
 * @tc.require:issueI611S5
 */
HWTEST_F(HksRsaSignVerifyPart9Test, HksRsaSignVerifyPart9Test088, TestSize.Level1)
{
    HKS_LOG_E(" Enter HksRsaSignVerifyPart9Test088");
    int32_t ret = HKS_FAILURE;
    const char *keyAliasString = "HksRSASignVerifyKeyAliasTest088";
    struct HksParamSet *genParamSet = nullptr;
    struct HksParamSet *signParamSet = nullptr;
    struct HksParamSet *verifyParamSet = nullptr;

    ret = InitParamSet(&genParamSet, g_genParamsTest088, sizeof(g_genParamsTest088) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet failed.";
    ret = InitParamSet(&signParamSet, g_signParamsTest088, sizeof(g_signParamsTest088) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet failed.";
    ret = InitParamSet(&verifyParamSet, g_verifyParamsTest088, sizeof(g_verifyParamsTest088) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet failed.";

    /* Generate Key */
    struct HksBlob keyAlias = {strlen(keyAliasString), (uint8_t *)keyAliasString};

    if ((genParamSet != nullptr) || (signParamSet != nullptr) || (verifyParamSet != nullptr)) {
        ret = HksRsaSignVerifyTestParamAbsentCase(keyAlias, genParamSet, signParamSet, verifyParamSet);
    }

    /* Delete Key */
    ret = HksDeleteKey(&keyAlias, genParamSet);
    EXPECT_EQ(ret, HKS_SUCCESS) << "DeleteKey failed.";

    HksFreeParamSet(&genParamSet);
    HksFreeParamSet(&signParamSet);
    HksFreeParamSet(&verifyParamSet);
}

/**
 * @tc.name: HksRsaSignVerifyPart9Test.HksRsaSignVerifyPart9Test089
 * @tc.desc: rsa sign and verify; alg-RSA, pad-PKCS1_V1_5 and digest-MD5.
 * @tc.type: FUNC
 * @tc.require:issueI611S5
 */
HWTEST_F(HksRsaSignVerifyPart9Test, HksRsaSignVerifyPart9Test089, TestSize.Level1)
{
    HKS_LOG_E(" Enter HksRsaSignVerifyPart9Test089");
    int32_t ret = HKS_FAILURE;
    const char *keyAliasString = "HksRSASignVerifyKeyAliasTest089";
    struct HksParamSet *genParamSet = nullptr;
    struct HksParamSet *signParamSet = nullptr;
    struct HksParamSet *verifyParamSet = nullptr;

    ret = InitParamSet(&genParamSet, g_genParamsTest089, sizeof(g_genParamsTest089) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet failed.";
    ret = InitParamSet(&signParamSet, g_signParamsTest089, sizeof(g_signParamsTest089) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet failed.";
    ret = InitParamSet(&verifyParamSet, g_verifyParamsTest089, sizeof(g_verifyParamsTest089) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet failed.";

    /* Generate Key */
    struct HksBlob keyAlias = {strlen(keyAliasString), (uint8_t *)keyAliasString};

    if ((genParamSet != nullptr) || (signParamSet != nullptr) || (verifyParamSet != nullptr)) {
        ret = HksRsaSignVerifyTestNormalAnotherCase(keyAlias, genParamSet, signParamSet, verifyParamSet);
    }

    /* Delete Key */
    ret = HksDeleteKey(&keyAlias, genParamSet);
    EXPECT_EQ(ret, HKS_SUCCESS) << "DeleteKey failed.";

    HksFreeParamSet(&genParamSet);
    HksFreeParamSet(&signParamSet);
    HksFreeParamSet(&verifyParamSet);
}

/**
 * @tc.name: HksRsaSignVerifyPart9Test.HksRsaSignVerifyPart9Test090
 * @tc.desc: rsa sign and verify; alg-RSA, pad-PKCS1_V1_5 and digest-MD5.
 * @tc.type: FUNC
 * @tc.require:issueI611S5
 */
HWTEST_F(HksRsaSignVerifyPart9Test, HksRsaSignVerifyPart9Test090, TestSize.Level1)
{
    HKS_LOG_E(" Enter HksRsaSignVerifyPart9Test090");
    int32_t ret = HKS_FAILURE;
    const char *keyAliasString = "HksRSASignVerifyKeyAliasTest090";
    struct HksParamSet *genParamSet = nullptr;
    struct HksParamSet *signParamSet = nullptr;
    struct HksParamSet *verifyParamSet = nullptr;

    ret = InitParamSet(&genParamSet, g_genParamsTest090, sizeof(g_genParamsTest090) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet failed.";
    ret = InitParamSet(&signParamSet, g_signParamsTest090, sizeof(g_signParamsTest090) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet failed.";
    ret = InitParamSet(&verifyParamSet, g_verifyParamsTest090, sizeof(g_verifyParamsTest090) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet failed.";

    /* Generate Key */
    struct HksBlob keyAlias = {strlen(keyAliasString), (uint8_t *)keyAliasString};

    if ((genParamSet != nullptr) || (signParamSet != nullptr) || (verifyParamSet != nullptr)) {
        ret = HksRSASignVerifyTestAbnormalCaseNoPadding(keyAlias, genParamSet, signParamSet, verifyParamSet);
    }

    /* Delete Key */
    ret = HksDeleteKey(&keyAlias, genParamSet);
    EXPECT_EQ(ret, HKS_SUCCESS) << "DeleteKey failed.";

    HksFreeParamSet(&genParamSet);
    HksFreeParamSet(&signParamSet);
    HksFreeParamSet(&verifyParamSet);
}
} // namespace Unittest::RsaSignVerify
