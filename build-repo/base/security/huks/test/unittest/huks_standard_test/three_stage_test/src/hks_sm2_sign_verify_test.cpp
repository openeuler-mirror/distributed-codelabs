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

#include "hks_sm2_sign_verify_test.h"

#include <gtest/gtest.h>

using namespace testing::ext;
namespace Unittest::Sm2SignVerify {
class HksSm2SignVerifyTest : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};

void HksSm2SignVerifyTest::SetUpTestCase(void)
{
}

void HksSm2SignVerifyTest::TearDownTestCase(void)
{
}

void HksSm2SignVerifyTest::SetUp()
{
    EXPECT_EQ(HksInitialize(), 0);
}

void HksSm2SignVerifyTest::TearDown()
{
}

#ifdef _USE_OPENSSL_
const uint32_t SUCCESS_RETURN_INDEX = 0;
const uint32_t FAILURE_RETURN_INDEX = 1;

const uint32_t NECESSARY_PARAMS_SUCCESS_RETURN_INDEX = 4;

static const struct GenerateKeyCaseParam g_genParamsTest[] = {
    {   0,
        HKS_SUCCESS,
        {
            {
                .tag = HKS_TAG_ALGORITHM,
                .uint32Param = HKS_ALG_SM2
            }, {
                .tag = HKS_TAG_KEY_SIZE,
                .uint32Param = HKS_SM2_KEY_SIZE_256
            }, {
                .tag = HKS_TAG_PURPOSE,
                .uint32Param = HKS_KEY_PURPOSE_SIGN | HKS_KEY_PURPOSE_VERIFY
            }, {
                .tag = HKS_TAG_DIGEST,
                .uint32Param = HKS_DIGEST_SM3
            },
        },
    },

    {   1,
        HKS_ERROR_INVALID_KEY_SIZE,
        {
            {
                .tag = HKS_TAG_ALGORITHM,
                .uint32Param = HKS_ALG_SM2
            }, {
                .tag = HKS_TAG_KEY_SIZE,
                .uint32Param = HKS_AES_KEY_SIZE_512
            }, {
                .tag = HKS_TAG_PURPOSE,
                .uint32Param = HKS_KEY_PURPOSE_SIGN | HKS_KEY_PURPOSE_VERIFY
            }, {
                .tag = HKS_TAG_DIGEST,
                .uint32Param = HKS_DIGEST_SM3
            },
        },
    },

    {   2,
        HKS_ERROR_INVALID_DIGEST,
        {
            {
                .tag = HKS_TAG_ALGORITHM,
                .uint32Param = HKS_ALG_SM2
            }, {
                .tag = HKS_TAG_KEY_SIZE,
                .uint32Param = HKS_SM2_KEY_SIZE_256
            }, {
                .tag = HKS_TAG_PURPOSE,
                .uint32Param = HKS_KEY_PURPOSE_SIGN | HKS_KEY_PURPOSE_VERIFY
            }, {
                .tag = HKS_TAG_DIGEST,
                .uint32Param = HKS_DIGEST_SHA1
            },
        },
    },

    {   3,
        HKS_ERROR_INVALID_PURPOSE,
        {
            {
                .tag = HKS_TAG_ALGORITHM,
                .uint32Param = HKS_ALG_SM2
            }, {
                .tag = HKS_TAG_KEY_SIZE,
                .uint32Param = HKS_SM2_KEY_SIZE_256
            }, {
                .tag = HKS_TAG_PURPOSE,
                .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT
            }, {
                .tag = HKS_TAG_DIGEST,
                .uint32Param = HKS_DIGEST_SM3
            },
        },
    },

    {   4,
        HKS_SUCCESS,
        {
            {
                .tag = HKS_TAG_ALGORITHM,
                .uint32Param = HKS_ALG_SM2
            }, {
                .tag = HKS_TAG_KEY_SIZE,
                .uint32Param = HKS_SM2_KEY_SIZE_256
            }, {
                .tag = HKS_TAG_PURPOSE,
                .uint32Param = HKS_KEY_PURPOSE_SIGN | HKS_KEY_PURPOSE_VERIFY
            }
        },
    },
};
static const struct GenerateKeyCaseParam g_signParamsTest[] = {
    {   0,
        HKS_SUCCESS,
        {
            {
                .tag = HKS_TAG_ALGORITHM,
                .uint32Param = HKS_ALG_SM2
            }, {
                .tag = HKS_TAG_KEY_SIZE,
                .uint32Param = HKS_SM2_KEY_SIZE_256
            }, {
                .tag = HKS_TAG_PURPOSE,
                .uint32Param = HKS_KEY_PURPOSE_SIGN
            }, {
                .tag = HKS_TAG_DIGEST,
                .uint32Param = HKS_DIGEST_SM3
            },
        },
    },

    {   1,
        HKS_ERROR_INVALID_ARGUMENT,
        {
            {
                .tag = HKS_TAG_ALGORITHM,
                .uint32Param = HKS_ALG_SM2
            }, {
                .tag = HKS_TAG_KEY_SIZE,
                .uint32Param = HKS_AES_KEY_SIZE_512
            }, {
                .tag = HKS_TAG_PURPOSE,
                .uint32Param = HKS_KEY_PURPOSE_SIGN
            }, {
                .tag = HKS_TAG_DIGEST,
                .uint32Param = HKS_DIGEST_SM3
            },
        },
    },

    {   2,
        HKS_ERROR_INVALID_ARGUMENT,
        {
            {
                .tag = HKS_TAG_ALGORITHM,
                .uint32Param = HKS_ALG_SM2
            }, {
                .tag = HKS_TAG_KEY_SIZE,
                .uint32Param = HKS_SM2_KEY_SIZE_256
            }, {
                .tag = HKS_TAG_PURPOSE,
                .uint32Param = HKS_KEY_PURPOSE_SIGN
            }, {
                .tag = HKS_TAG_DIGEST,
                .uint32Param = HKS_DIGEST_SHA1
            },
        },
    },

    {   3,
        HKS_ERROR_INVALID_ALGORITHM,
        {
            {
                .tag = HKS_TAG_ALGORITHM,
                .uint32Param = HKS_ALG_SM2
            }, {
                .tag = HKS_TAG_KEY_SIZE,
                .uint32Param = HKS_SM2_KEY_SIZE_256
            }, {
                .tag = HKS_TAG_PURPOSE,
                .uint32Param = HKS_KEY_PURPOSE_ENCRYPT
            }, {
                .tag = HKS_TAG_DIGEST,
                .uint32Param = HKS_DIGEST_SM3
            },
        },
    },

    {   4,
        HKS_SUCCESS,
        {
            {
                .tag = HKS_TAG_ALGORITHM,
                .uint32Param = HKS_ALG_SM2
            }, {
                .tag = HKS_TAG_KEY_SIZE,
                .uint32Param = HKS_SM2_KEY_SIZE_256
            }, {
                .tag = HKS_TAG_PURPOSE,
                .uint32Param = HKS_KEY_PURPOSE_SIGN
            }, {
                .tag = HKS_TAG_DIGEST,
                .uint32Param = HKS_DIGEST_NONE
            },
        },
    },
};
static const struct GenerateKeyCaseParam g_verifyParamsTest[] = {
    {   0,
        HKS_SUCCESS,
        {
            {
                .tag = HKS_TAG_ALGORITHM,
                .uint32Param = HKS_ALG_SM2
            }, {
                .tag = HKS_TAG_KEY_SIZE,
                .uint32Param = HKS_SM2_KEY_SIZE_256
            }, {
                .tag = HKS_TAG_PURPOSE,
                .uint32Param = HKS_KEY_PURPOSE_VERIFY
            }, {
                .tag = HKS_TAG_DIGEST,
                .uint32Param = HKS_DIGEST_SM3
            },
        },
    },

    {   1,
        HKS_ERROR_INVALID_KEY_SIZE,
        {
            {
                .tag = HKS_TAG_ALGORITHM,
                .uint32Param = HKS_ALG_SM2
            }, {
                .tag = HKS_TAG_KEY_SIZE,
                .uint32Param = HKS_AES_KEY_SIZE_512
            }, {
                .tag = HKS_TAG_PURPOSE,
                .uint32Param = HKS_KEY_PURPOSE_VERIFY
            }, {
                .tag = HKS_TAG_DIGEST,
                .uint32Param = HKS_DIGEST_SM3
            },
        },
    },

    {   2,
        HKS_ERROR_INVALID_DIGEST,
        {
            {
                .tag = HKS_TAG_ALGORITHM,
                .uint32Param = HKS_ALG_SM2
            }, {
                .tag = HKS_TAG_KEY_SIZE,
                .uint32Param = HKS_SM2_KEY_SIZE_256
            }, {
                .tag = HKS_TAG_PURPOSE,
                .uint32Param = HKS_KEY_PURPOSE_VERIFY
            }, {
                .tag = HKS_TAG_DIGEST,
                .uint32Param = HKS_DIGEST_SHA1
            },
        },
    },

    {   3,
        HKS_ERROR_INVALID_PURPOSE,
        {
            {
                .tag = HKS_TAG_ALGORITHM,
                .uint32Param = HKS_ALG_SM2
            }, {
                .tag = HKS_TAG_KEY_SIZE,
                .uint32Param = HKS_SM2_KEY_SIZE_256
            }, {
                .tag = HKS_TAG_PURPOSE,
                .uint32Param = HKS_KEY_PURPOSE_DECRYPT
            }, {
                .tag = HKS_TAG_DIGEST,
                .uint32Param = HKS_DIGEST_SM3
            },
        },
    },

    {   4,
        HKS_SUCCESS,
        {
            {
                .tag = HKS_TAG_ALGORITHM,
                .uint32Param = HKS_ALG_SM2
            }, {
                .tag = HKS_TAG_KEY_SIZE,
                .uint32Param = HKS_SM2_KEY_SIZE_256
            }, {
                .tag = HKS_TAG_PURPOSE,
                .uint32Param = HKS_KEY_PURPOSE_VERIFY
            }, {
                .tag = HKS_TAG_DIGEST,
                .uint32Param = HKS_DIGEST_NONE
            },
        },
    },
};

static int32_t HksTestSignVerify(const struct HksBlob *keyAlias, const struct HksParamSet *paramSet,
    struct HksBlob *inData, struct HksBlob *outData)
{
    uint8_t tmpHandle[sizeof(uint64_t)] = {0};
    struct HksBlob handle = { sizeof(uint64_t), tmpHandle };
    int32_t ret;

    struct HksParam *digestAlg = nullptr;
    ret = HksGetParam(paramSet, HKS_TAG_DIGEST, &digestAlg);
    EXPECT_EQ(ret, HKS_SUCCESS) << "GetParam failed.";
    if (digestAlg->uint32Param == HKS_DIGEST_NONE) {
        inData->size = g_inDataArrayAfterHashLen[0];
        inData->data = const_cast<uint8_t *>(g_inDataArrayAfterHash[0]);
    }

    do {
        ret = HksInit(keyAlias, paramSet, &handle, nullptr);
        if (ret != HKS_SUCCESS) {
            break;
        }

        struct HksParam *tmpParam = NULL;
        ret = HksGetParam(paramSet, HKS_TAG_PURPOSE, &tmpParam);
        if (ret != HKS_SUCCESS) {
            break;
        }

        ret = TestUpdateFinish(&handle, paramSet, tmpParam->uint32Param, inData, outData);
        if (ret != HKS_SUCCESS) {
            break;
        }
        ret = HKS_SUCCESS;
    } while (0);

    (void)HksAbort(&handle, paramSet);
    return ret;
}

static int CreateImportKeyAlias(struct HksBlob *importKeyAlias, const struct HksBlob *keyAlias)
{
    const char *tmp = "new";
    importKeyAlias->size = keyAlias->size + strlen(tmp);
    importKeyAlias->data = (uint8_t *)HksMalloc(importKeyAlias->size);
    if (importKeyAlias->data == nullptr) {
        return HKS_ERROR_MALLOC_FAIL;
    }

    if (memcpy_s(importKeyAlias->data, importKeyAlias->size, keyAlias->data, keyAlias->size) != EOK) {
        HKS_FREE_PTR(importKeyAlias->data);
        return HKS_ERROR_BAD_STATE;
    }

    if (memcpy_s(importKeyAlias->data + keyAlias->size,
        importKeyAlias->size - keyAlias->size, tmp, strlen(tmp)) != EOK) {
        HKS_FREE_PTR(importKeyAlias->data);
        return HKS_ERROR_BAD_STATE;
    }

    return HKS_SUCCESS;
}

void static FreeBuffAndDeleteKey(struct HksParamSet **paramSet1, struct HksParamSet **paramSet2,
    struct HksParamSet **paramSet3, const struct HksBlob *keyAlias1, const struct HksBlob *keyAlias2)
{
    (void)HksDeleteKey(keyAlias1, *paramSet1);
    (void)HksDeleteKey(keyAlias2, *paramSet1);
    HksFreeParamSet(paramSet1);
    HksFreeParamSet(paramSet2);
    HksFreeParamSet(paramSet3);
}

static int32_t HksSm2SignVerifyTestRun(const struct HksBlob *keyAlias, const uint32_t genIndex,
    const uint32_t signIndex, const uint32_t verifyIndex, const bool isTestSignData)
{
    struct HksParamSet *genParamSet = nullptr;
    struct HksParamSet *signParamSet = nullptr;
    struct HksParamSet *verifyParamSet = nullptr;

    struct HksBlob inData = { g_inData.length(), (uint8_t *)g_inData.c_str() };
    uint8_t outDataS[SM2_COMMON_SIZE] = {0};
    struct HksBlob outDataSign = { SM2_COMMON_SIZE, outDataS };

    uint8_t pubKey[HKS_MAX_KEY_LEN] = {0};
    struct HksBlob publicKey = { HKS_MAX_KEY_LEN, pubKey };

    struct HksBlob importKeyAlias;
    int32_t ret = CreateImportKeyAlias(&importKeyAlias, keyAlias);
    EXPECT_EQ(ret, HKS_SUCCESS) << "createImportKeyAlias failed.";
    do {
        /* 1. Generate Key */
        uint32_t cnt = sizeof(g_genParamsTest[genIndex].params) / sizeof(HksParam);
        cnt = (genIndex == NECESSARY_PARAMS_SUCCESS_RETURN_INDEX) ? (cnt - 1) : cnt;
        ret = InitParamSet(&genParamSet, g_genParamsTest[genIndex].params, cnt);
        EXPECT_EQ(ret, HKS_SUCCESS) << "InitGenParamSet failed.";
        ret = HksGenerateKey(keyAlias, genParamSet, nullptr);
        if (ret != HKS_SUCCESS) {
            ret = ((ret == g_genParamsTest[genIndex].result) ? HKS_SUCCESS : ret);
            break;
        }

        /* 2. Sign Three Stage */
        ret = InitParamSet(&signParamSet, g_signParamsTest[signIndex].params,
            sizeof(g_signParamsTest[signIndex].params) / sizeof(HksParam));
        EXPECT_EQ(ret, HKS_SUCCESS) << "InitSignParamSet failed.";
        ret = HksTestSignVerify(keyAlias, signParamSet, &inData, &outDataSign);
        if (ret != HKS_SUCCESS) {
            ret = ((ret == g_signParamsTest[signIndex].result) ? HKS_SUCCESS : ret);
            break;
        }

        /* 3. Export Public Key */
        EXPECT_EQ(HksExportPublicKey(keyAlias, genParamSet, &publicKey), HKS_SUCCESS) << "ExportPublicKey failed.";

        /* 4. Import Key */
        ret = InitParamSet(&verifyParamSet, g_verifyParamsTest[verifyIndex].params,
            sizeof(g_verifyParamsTest[verifyIndex].params) / sizeof(HksParam));
        EXPECT_EQ(ret, HKS_SUCCESS) << "InitVerifyParamSet failed.";
        ret = HksImportKey(&importKeyAlias, verifyParamSet, &publicKey);
        if (ret != HKS_SUCCESS) {
            ret = ((ret == g_verifyParamsTest[verifyIndex].result) ? HKS_SUCCESS : ret);
            break;
        }

        /* 5. Verify Three Stage */
        if (isTestSignData) {
            (void)memset_s(outDataSign.data, outDataSign.size, 0, outDataSign.size);
        }
        ret = HksTestSignVerify(&importKeyAlias, verifyParamSet, &inData, &outDataSign);
        if (ret != HKS_SUCCESS) {
            ret = ((ret == g_verifyParamsTest[verifyIndex].result) ? HKS_SUCCESS : ret);
        }
    } while (0);

    FreeBuffAndDeleteKey(&genParamSet, &signParamSet, &verifyParamSet, keyAlias, &importKeyAlias);
    HKS_FREE_PTR(importKeyAlias.data);
    return ret;
}

/**
 * @tc.name: HksSm2SignVerifyTest.HksSm2SignVerifyTest001
 * @tc.desc: normal parameter test case : alg-SM2, pur-Sign/Verify, keySize-256 and dig-SM3.
 * @tc.type: FUNC
 */
HWTEST_F(HksSm2SignVerifyTest, HksSm2SignVerifyTest001, TestSize.Level0)
{
    HKS_LOG_E("Enter HksSm2SignVerifyTest001");
    const char *keyAliasString = "HksSM2SignVerifyKeyAliasTest001";
    struct HksBlob keyAlias = { strlen(keyAliasString), (uint8_t *)keyAliasString };
    int ret = HksSm2SignVerifyTestRun(&keyAlias, SUCCESS_RETURN_INDEX,
            SUCCESS_RETURN_INDEX, SUCCESS_RETURN_INDEX, false);
    EXPECT_EQ(ret, HKS_SUCCESS) << "sm2SignVerifyTest001 failed.";
}

/**
 * @tc.name: HksSm2SignVerifyTest.HksSm2SignVerifyTest002
 * @tc.desc: abnormal parameter test cases : the abnormal parameter is tag
 * @tc.type: FUNC
 */
HWTEST_F(HksSm2SignVerifyTest, HksSm2SignVerifyTest002, TestSize.Level0)
{
    HKS_LOG_E("Enter HksSm2SignVerifyTest002");
    const char *keyAliasString = "HksSM2SignVerifyKeyAliasTest002";
    struct HksBlob keyAlias = { strlen(keyAliasString), (uint8_t *)keyAliasString };

    uint32_t genPramCnt = sizeof(g_genParamsTest) / sizeof(GenerateKeyCaseParam) - 1;
    uint32_t signPramCnt = sizeof(g_signParamsTest) / sizeof(GenerateKeyCaseParam) - 1;
    uint32_t verifyPramCnt = sizeof(g_verifyParamsTest) / sizeof(GenerateKeyCaseParam) - 1;

    int ret;
    for (uint32_t i = FAILURE_RETURN_INDEX; i < genPramCnt; i++) {
        ret = HksSm2SignVerifyTestRun(&keyAlias, i, SUCCESS_RETURN_INDEX, SUCCESS_RETURN_INDEX, false);
        EXPECT_EQ(ret, HKS_SUCCESS) << "sm2SignVerifyTest002 gen abnormal test failed.";
    }

    for (uint32_t j = FAILURE_RETURN_INDEX; j < signPramCnt; j++) {
        ret = HksSm2SignVerifyTestRun(&keyAlias, SUCCESS_RETURN_INDEX, j, SUCCESS_RETURN_INDEX, false);
        EXPECT_EQ(ret, HKS_SUCCESS) << "sm2SignVerifyTest002 sign abnormal test failed.";
    }

    for (uint32_t k = FAILURE_RETURN_INDEX; k < verifyPramCnt; k++) {
        ret = HksSm2SignVerifyTestRun(&keyAlias, SUCCESS_RETURN_INDEX, SUCCESS_RETURN_INDEX, k, false);
        EXPECT_EQ(ret, HKS_SUCCESS) << "sm2SignVerifyTest002 verify abnormal test failed.";
    }
}

/**
 * @tc.name: HksSm2SignVerifyTest.HksSm2SignVerifyTest003
 * @tc.desc: abnormal parameter test case : the abnormal parameter is verifyData
 * @tc.type: FUNC
 */
HWTEST_F(HksSm2SignVerifyTest, HksSm2SignVerifyTest003, TestSize.Level0)
{
    HKS_LOG_E("Enter HksSm2SignVerifyTest003");
    const char *keyAliasString = "HksSM2SignVerifyKeyAliasTest003";
    struct HksBlob keyAlias = { strlen(keyAliasString), (uint8_t *)keyAliasString };

    int ret = HksSm2SignVerifyTestRun(&keyAlias, SUCCESS_RETURN_INDEX,
            SUCCESS_RETURN_INDEX, SUCCESS_RETURN_INDEX, true);
    EXPECT_EQ(ret, HKS_FAILURE) << "sm2SignVerifyTest003 failed.";
}

/**
 * @tc.name: HksSm2SignVerifyTest.HksSm2SignVerifyTest004
 * @tc.desc: normal parameter test case : alg-SM2, pur-Sign/Verify, keySize-256 and dig-NONE.
 * @tc.type: FUNC
 * @tc.require:issueI611S5
 */
HWTEST_F(HksSm2SignVerifyTest, HksSm2SignVerifyTest004, TestSize.Level0)
{
    HKS_LOG_E("Enter HksSm2SignVerifyTest004");
    const char *keyAliasString = "HksSM2SignVerifyKeyAliasTest004";
    struct HksBlob keyAlias = { strlen(keyAliasString), (uint8_t *)keyAliasString };
    int ret = HksSm2SignVerifyTestRun(&keyAlias, NECESSARY_PARAMS_SUCCESS_RETURN_INDEX,
    NECESSARY_PARAMS_SUCCESS_RETURN_INDEX, NECESSARY_PARAMS_SUCCESS_RETURN_INDEX, false);
    EXPECT_EQ(ret, HKS_SUCCESS) << "sm2SignVerifyTest004 failed.";
}
#endif
}
