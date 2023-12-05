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

#include <string>
#include <vector>

#include <gtest/gtest.h>
#include <securec.h>

#include "hks_api.h"
#include "hks_mem.h"
#include "hks_param.h"
#include "hks_three_stage_test_common.h"
#include "hks_type.h"

using namespace testing::ext;
namespace Unittest::ExportKey {
const int SET_SIZE_4096 = 4096;
#ifdef _USE_OPENSSL_
const uint32_t DSA_COMMON_SIZE = 1024;
#endif

struct TestCaseParams {
    std::vector<HksParam> params;
};

class HksExportTest : public testing::Test {
public:
    int32_t RunTestCase(const TestCaseParams &testCaseParams)
    {
        const char *keyAliasString = "This is for export normal key";
        struct HksBlob authId = { strlen(keyAliasString), (uint8_t *)keyAliasString };

        /* 1. Generate Key */
        struct HksParamSet *paramInSet = nullptr;
        int32_t ret = InitParamSet(&paramInSet, testCaseParams.params.data(), testCaseParams.params.size());
        if (ret != HKS_SUCCESS) {
            return ret;
        }

        ret = HksGenerateKey(&authId, paramInSet, nullptr);
        HksFreeParamSet(&paramInSet);
        EXPECT_EQ(ret, HKS_SUCCESS) << "GenerateKey failed.";

        /* 2. Export Key */
        HksBlob publicKey = { .size = SET_SIZE_4096, .data = (uint8_t *)HksMalloc(SET_SIZE_4096) };
        if (publicKey.data == nullptr) {
            return HKS_ERROR_MALLOC_FAIL;
        }
        ret = HksExportPublicKey(&authId, nullptr, &publicKey);
        EXPECT_EQ(ret, HKS_SUCCESS) << "ExportPublicKey failed.";

        /* 3. Delete Key */
        (void)HksDeleteKey(&authId, nullptr);
        HksFree(publicKey.data);

        return ret;
    }
};

const TestCaseParams g_huksExportKey00100Params = {
    .params = {
        { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_RSA },
        { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_RSA_KEY_SIZE_512 },
        { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT },
        { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA512 },
        { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_PKCS1_V1_5 },
    },
};

const TestCaseParams g_huksExportKey00200Params = {
    .params = {
        { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_RSA },
        { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_RSA_KEY_SIZE_768 },
        { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT },
        { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA512 },
        { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_OAEP },
    },
};

const TestCaseParams g_huksExportKey00300Params = {
    .params = {
        { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_RSA },
        { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_RSA_KEY_SIZE_1024 },
        { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT },
        { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA512 },
        { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_PKCS1_V1_5 },
    },
};

const TestCaseParams g_huksExportKey00400Params = {
    .params = {
        { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_RSA },
        { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_RSA_KEY_SIZE_2048 },
        { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT },
        { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA256 },
        { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_PKCS1_V1_5 },
    },
};

const TestCaseParams g_huksExportKey00500Params = {
    .params = {
        { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_RSA },
        { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_RSA_KEY_SIZE_3072 },
        { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT },
        { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA512 },
        { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_PKCS1_V1_5 },
    },
};

const TestCaseParams g_huksExportKey00600Params = {
    .params = {
        { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_RSA },
        { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_RSA_KEY_SIZE_4096 },
        { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT },
        { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA224 },
        { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_PKCS1_V1_5 },
    },
};

const TestCaseParams g_huksExportKey00700Params = {
    .params = {
        { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_ECC },
        { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_ECC_KEY_SIZE_224 },
        { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_SIGN | HKS_KEY_PURPOSE_VERIFY },
        { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA224 },
    },
};

const TestCaseParams g_huksExportKey00800Params = {
    .params = {
        { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_ECC },
        { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_ECC_KEY_SIZE_256 },
        { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_SIGN | HKS_KEY_PURPOSE_VERIFY },
        { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA256 },
    },
};

const TestCaseParams g_huksExportKey00900Params = {
    .params = {
        { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_ECC },
        { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_ECC_KEY_SIZE_384 },
        { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_SIGN | HKS_KEY_PURPOSE_VERIFY },
        { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA384 },
    },
};

const TestCaseParams g_huksExportKey01000Params = {
    .params = {
        { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_ECC },
        { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_ECC_KEY_SIZE_521 },
        { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_SIGN | HKS_KEY_PURPOSE_VERIFY },
        { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA512 },
    },
};

const TestCaseParams g_huksExportKey01100Params = {
    .params = {
        { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_X25519 },
        { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_CURVE25519_KEY_SIZE_256 },
        { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_AGREE },
    },
};

const TestCaseParams g_huksExportKey01200Params = {
    .params = {
        { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_ED25519 },
        { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_CURVE25519_KEY_SIZE_256 },
        { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_SIGN | HKS_KEY_PURPOSE_VERIFY },
    },
};

#ifdef _USE_OPENSSL_
// mbedtls engine don't support DSA alg
const TestCaseParams g_huksExportKey01300Params = {
    .params = {
        { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_DSA },
        { .tag = HKS_TAG_KEY_SIZE, .uint32Param = DSA_COMMON_SIZE },
        { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_SIGN | HKS_KEY_PURPOSE_VERIFY },
        { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA512 },
    },
};
#endif

const TestCaseParams g_huksExportKey01400Params = {
    .params = {
        { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_DH },
        { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_DH_KEY_SIZE_2048 },
        { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_AGREE },
    },
};

const TestCaseParams g_huksExportKey01500Params = {
    .params = {
        { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_DH },
        { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_DH_KEY_SIZE_3072 },
        { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_AGREE },
    },
};

const TestCaseParams g_huksExportKey01600Params = {
    .params = {
        { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_DH },
        { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_DH_KEY_SIZE_4096 },
        { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_AGREE },
    },
};

#ifdef _USE_OPENSSL_
/* mbedtls engine don't support SM2 alg */
const TestCaseParams g_huksExportKey01700Params = {
    .params = {
        { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_SM2 },
        { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_SM2_KEY_SIZE_256 },
        { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_SIGN | HKS_KEY_PURPOSE_VERIFY },
        { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SM3 },
    },
};
#endif

/**
 * @tc.number    : HksExportTest00100
 * @tc.name      : HksExportTest00100
 * @tc.desc      : Test huks exportkey (RSA512)
 */
HWTEST_F(HksExportTest, HksExportTest00100, TestSize.Level0)
{
    EXPECT_EQ(RunTestCase(g_huksExportKey00100Params), HKS_SUCCESS);
}

/**
 * @tc.number    : HksExportTest00200
 * @tc.name      : HksExportTest00200
 * @tc.desc      : Test huks exportkey (RSA768)
 */
HWTEST_F(HksExportTest, HksExportTest00200, TestSize.Level0)
{
    EXPECT_EQ(RunTestCase(g_huksExportKey00200Params), HKS_SUCCESS);
}

/**
 * @tc.number    : HksExportTest00300
 * @tc.name      : HksExportTest00300
 * @tc.desc      : Test huks exportkey (RSA1024)
 */
HWTEST_F(HksExportTest, HksExportTest00300, TestSize.Level0)
{
    EXPECT_EQ(RunTestCase(g_huksExportKey00300Params), HKS_SUCCESS);
}

/**
 * @tc.number    : HksExportTest00400
 * @tc.name      : HksExportTest00400
 * @tc.desc      : Test huks exportkey (RSA2048)
 */
HWTEST_F(HksExportTest, HksExportTest00400, TestSize.Level0)
{
    EXPECT_EQ(RunTestCase(g_huksExportKey00400Params), HKS_SUCCESS);
}

/**
 * @tc.number    : HksExportTest00500
 * @tc.name      : HksExportTest00500
 * @tc.desc      : Test huks exportkey (RSA3072)
 */
HWTEST_F(HksExportTest, HksExportTest00500, TestSize.Level1)
{
    EXPECT_EQ(RunTestCase(g_huksExportKey00500Params), HKS_SUCCESS);
}

/**
 * @tc.number    : HksExportTest00600
 * @tc.name      : HksExportTest00600
 * @tc.desc      : Test huks exportkey (RSA4096)
 */
HWTEST_F(HksExportTest, HksExportTest00600, TestSize.Level1)
{
    EXPECT_EQ(RunTestCase(g_huksExportKey00600Params), HKS_SUCCESS);
}
/**
 * @tc.number    : HksExportTest00700
 * @tc.name      : HksExportTest00700
 * @tc.desc      : Test huks exportkey (ECC224)
 */
HWTEST_F(HksExportTest, HksExportTest00700, TestSize.Level0)
{
    EXPECT_EQ(RunTestCase(g_huksExportKey00700Params), HKS_SUCCESS);
}
/**
 * @tc.number    : HksExportTest00800
 * @tc.name      : HksExportTest00800
 * @tc.desc      : Test huks exportkey (ECC256)
 */
HWTEST_F(HksExportTest, HksExportTest00800, TestSize.Level0)
{
    EXPECT_EQ(RunTestCase(g_huksExportKey00800Params), HKS_SUCCESS);
}
/**
 * @tc.number    : HksExportTest00900
 * @tc.name      : HksExportTest00900
 * @tc.desc      : Test huks exportkey (ECC384)
 */
HWTEST_F(HksExportTest, HksExportTest00900, TestSize.Level0)
{
    EXPECT_EQ(RunTestCase(g_huksExportKey00900Params), HKS_SUCCESS);
}

/**
 * @tc.number    : HksExportTest01000
 * @tc.name      : HksExportTest01000
 * @tc.desc      : Test huks exportkey (ECC521)
 */
HWTEST_F(HksExportTest, HksExportTest01000, TestSize.Level0)
{
    EXPECT_EQ(RunTestCase(g_huksExportKey01000Params), HKS_SUCCESS);
}

/**
 * @tc.number    : HksExportTest01100
 * @tc.name      : HksExportTest01100
 * @tc.desc      : Test huks exportkey (ED25519)
 */
HWTEST_F(HksExportTest, HksExportTest01100, TestSize.Level0)
{
    EXPECT_EQ(RunTestCase(g_huksExportKey01100Params), HKS_SUCCESS);
}

/**
 * @tc.number    : HksExportTest01200
 * @tc.name      : HksExportTest01200
 * @tc.desc      : Test huks exportkey (X25519)
 */
HWTEST_F(HksExportTest, HksExportTest01200, TestSize.Level0)
{
    EXPECT_EQ(RunTestCase(g_huksExportKey01200Params), HKS_SUCCESS);
}

/**
 * @tc.number    : HksExportTest01300
 * @tc.name      : HksExportTest01300
 * @tc.desc      : Test huks exportkey (DSA)
 */
HWTEST_F(HksExportTest, HksExportTest01300, TestSize.Level0)
{
#ifdef _USE_OPENSSL_
    // mbedtls engine don't support DSA alg
    EXPECT_EQ(RunTestCase(g_huksExportKey01300Params), HKS_SUCCESS);
#endif
}

/**
 * @tc.number    : HksExportTest01400
 * @tc.name      : HksExportTest01400
 * @tc.desc      : Test huks exportkey (DH2048)
 */
HWTEST_F(HksExportTest, HksExportTest01400, TestSize.Level0)
{
    EXPECT_EQ(RunTestCase(g_huksExportKey01400Params), HKS_SUCCESS);
}

/**
 * @tc.number    : HksExportTest01500
 * @tc.name      : HksExportTest01500
 * @tc.desc      : Test huks exportkey (DH3072)
 */
HWTEST_F(HksExportTest, HksExportTest01500, TestSize.Level0)
{
    EXPECT_EQ(RunTestCase(g_huksExportKey01500Params), HKS_SUCCESS);
}

/**
 * @tc.number    : HksExportTest01600
 * @tc.name      : HksExportTest01600
 * @tc.desc      : Test huks exportkey (DH4096)
 */
HWTEST_F(HksExportTest, HksExportTest01600, TestSize.Level0)
{
    EXPECT_EQ(RunTestCase(g_huksExportKey01600Params), HKS_SUCCESS);
}

/**
 * @tc.number    : HksExportTest01700
 * @tc.name      : HksExportTest01700
 * @tc.desc      : Test huks exportkey (SM2)
 * @tc.require: issueI5NY2Y
 */
HWTEST_F(HksExportTest, HksExportTest01700, TestSize.Level0)
{
#ifdef _USE_OPENSSL_
    // mbedtls engine don't support SM2 alg
    EXPECT_EQ(RunTestCase(g_huksExportKey01700Params), HKS_SUCCESS);
#endif
}
}
