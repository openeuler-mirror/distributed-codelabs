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

#include "hks_ecc_sign_verify_test_common.h"

#include <gtest/gtest.h>

using namespace testing::ext;
namespace Unittest::EccSifnVerify {
int32_t HksTestSignVerify(struct HksBlob *keyAlias, struct HksParamSet *paramSet,
    const struct HksBlob *inData, struct HksBlob *outData, bool isSign)
{
    uint8_t tmpHandle[sizeof(uint64_t)] = {0};
    struct HksBlob handle = { sizeof(uint64_t), tmpHandle };
    int32_t ret = HksInit(keyAlias, paramSet, &handle, nullptr);
    EXPECT_EQ(ret, HKS_SUCCESS) << "Init failed.";
    if (ret != HKS_SUCCESS) {
        return HKS_FAILURE;
    }

    struct HksParam *tmpParam = NULL;
    ret = HksGetParam(paramSet, HKS_TAG_PURPOSE, &tmpParam);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("get tag purpose failed.");
        return HKS_FAILURE;
    }

    ret = TestUpdateFinish(&handle, paramSet, tmpParam->uint32Param, inData, outData);
    EXPECT_EQ(ret, HKS_SUCCESS) << "TestUpdateFinish failed.";
    if (ret != HKS_SUCCESS) {
        return HKS_FAILURE;
    }

    if (isSign) {
        uint8_t tmpOut[ECC_COMMON_SIZE] = {0};
        struct HksBlob outData1 = { ECC_COMMON_SIZE, tmpOut };
        ret = HksSign(keyAlias, paramSet, inData, &outData1);
        EXPECT_EQ(ret, HKS_SUCCESS) << "HksSign failed.";
    } else {
        ret = HksVerify(keyAlias, paramSet, inData, outData);
        EXPECT_EQ(ret, HKS_SUCCESS) << "HksVerify failed.";
    }

    return ret;
}

int32_t HksEccSignVerifyTestNormalCase(struct HksBlob keyAlias, struct HksParamSet *genParamSet,
    struct HksParamSet *signParamSet, struct HksParamSet *verifyParamSet, uint32_t loopIndex)
{
    struct HksBlob inData = {
        g_inData.length(),
        const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(g_inData.c_str()))
    };
    int32_t ret = HKS_FAILURE;

    struct HksParam *digestAlg = nullptr;
    ret = HksGetParam(signParamSet, HKS_TAG_DIGEST, &digestAlg);
    EXPECT_EQ(ret, HKS_SUCCESS) << "GetParam failed.";
    if (digestAlg->uint32Param == HKS_DIGEST_NONE) {
        inData.size = g_inDataArrayAfterHashLen[loopIndex];
        inData.data = const_cast<uint8_t *>(g_inDataArrayAfterHash[loopIndex]);
    }

    /* 1. Generate Key */
    // Generate Key
    ret = HksGenerateKey(&keyAlias, genParamSet, nullptr);
    EXPECT_EQ(ret, HKS_SUCCESS) << "GenerateKey failed.";

    /* 2. Sign Three Stage */
    uint8_t outDataS[ECC_COMMON_SIZE] = {0};
    struct HksBlob outDataSign = { ECC_COMMON_SIZE, outDataS };
    ret = HksTestSignVerify(&keyAlias, signParamSet, &inData, &outDataSign, true);
    EXPECT_EQ(ret, HKS_SUCCESS) << "Sign failed.";

    /* 3. Export Public Key */
    uint8_t pubKey[HKS_ECC_KEY_SIZE_521] = {0};
    struct HksBlob publicKey = { HKS_ECC_KEY_SIZE_521, pubKey };
    ret = HksExportPublicKey(&keyAlias, genParamSet, &publicKey);
    EXPECT_EQ(ret, HKS_SUCCESS) << "ExportPublicKey failed.";

    /* 4. Import Key */
    char newKey[] = "ECC_Sign_Verify_Import_KeyAlias";
    struct HksBlob newKeyAlias = { .size = strlen(newKey), .data = reinterpret_cast<uint8_t *>(newKey) };
    ret = HksImportKey(&newKeyAlias, verifyParamSet, &publicKey);
    EXPECT_EQ(ret, HKS_SUCCESS) << "ImportKey failed";

    /* 5. Verify Three Stage */
    ret = HksTestSignVerify(&newKeyAlias, verifyParamSet, &inData, &outDataSign, false);
    EXPECT_EQ(ret, HKS_SUCCESS) << "Verify failed.";

    /* 6. Delete New Key */
    int32_t deleteRet = HksDeleteKey(&newKeyAlias, verifyParamSet);
    EXPECT_EQ(deleteRet, HKS_SUCCESS) << "Delete ImportKey failed.";

    return ret;
}

int32_t HksTestSignVerifyParamAbsent(struct HksBlob keyAlias, struct HksParamSet *genParamSet,
    struct HksParamSet *signParamSet)
{
    /* 1. Generate Key */
    uint32_t ret = HksGenerateKey(&keyAlias, genParamSet, nullptr);
    EXPECT_EQ(ret, HKS_SUCCESS) << "GenerateKey failed.";

    uint8_t tmpHandle[sizeof(uint64_t)] = {0};
    struct HksBlob handle = { sizeof(uint64_t), tmpHandle };
    ret = HksInit(&keyAlias, signParamSet, &handle, nullptr);
    EXPECT_NE(ret, HKS_SUCCESS) << "Init failed.";
    return ret;
}
}