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

#include "hks_rsa_sign_verify_test_common.h"

#include <gtest/gtest.h>

using namespace testing::ext;
namespace Unittest::RsaSignVerify {
struct RsaSignVerifyParamStruct {
    bool isGenParamSetSameAsImport;
    bool isUseIndataAfterHash;
    uint32_t hashAlgIndex;
};

static int32_t RsaSignVerifyTestNormalCase(struct HksBlob keyAlias, struct HksParamSet *genParamSet,
    struct HksParamSet *signParamSet, struct HksParamSet *verifyParamSet, const struct RsaSignVerifyParamStruct param)
{
    struct HksBlob inData = {
        g_inData.length(),
        const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(g_inData.c_str()))
    };
    uint8_t tmpIn[] = "tempIn";
    struct HksBlob finishInData = { 0, tmpIn };

    if (param.isUseIndataAfterHash) {
        inData.size = g_inDataArrayAfterHashLen[param.hashAlgIndex];
        inData.data = const_cast<uint8_t *>(g_inDataArrayAfterHash[param.hashAlgIndex]);
    }

    /* 1. Generate Key */
    // Generate Key
    int32_t ret = HksGenerateKey(&keyAlias, genParamSet, nullptr);
    EXPECT_EQ(ret, HKS_SUCCESS) << "GenerateKey failed.";

    /* 2. Sign Three Stage */
    // Init
    uint8_t handleS[sizeof(uint64_t)] = {0};
    struct HksBlob handleSign = { sizeof(uint64_t), handleS };
    ret = HksInit(&keyAlias, signParamSet, &handleSign, nullptr);
    EXPECT_EQ(ret, HKS_SUCCESS) << "Init failed.";
    // Update loop
    ret = HksTestUpdate(&handleSign, signParamSet, &inData);
    EXPECT_EQ(ret, HKS_SUCCESS) << "Update failed.";
    // Finish
    uint8_t outDataS[RSA_COMMON_SIZE] = {0};
    struct HksBlob outDataSign = { RSA_COMMON_SIZE, outDataS };
    ret = HksFinish(&handleSign, signParamSet, &finishInData, &outDataSign);
    EXPECT_EQ(ret, HKS_SUCCESS) << "Finish failed.";

    /* 3. Export Public Key */
    uint8_t pubKey[HKS_RSA_KEY_SIZE_1024] = {0};
    struct HksBlob publicKey = { HKS_RSA_KEY_SIZE_1024, pubKey };
    ret = HksExportPublicKey(&keyAlias, genParamSet, &publicKey);
    EXPECT_EQ(ret, HKS_SUCCESS) << "ExportPublicKey failed.";

    /* 4. Import Key */
    char newKey[] = "RSA_Sign_Verify_Import_KeyAlias";
    struct HksBlob newKeyAlias = { .size = strlen(newKey), .data = reinterpret_cast<uint8_t *>(newKey) };
    if (param.isGenParamSetSameAsImport) {
        ret = HksImportKey(&newKeyAlias, genParamSet, &publicKey);
    } else {
        ret = HksImportKey(&newKeyAlias, verifyParamSet, &publicKey);
    }
    EXPECT_EQ(ret, HKS_SUCCESS) << "ImportKey failed";

    /* 5. Verify Three Stage */
    // Init
    uint8_t handleV[sizeof(uint64_t)] = {0};
    struct HksBlob handleVerify = { sizeof(uint64_t), handleV };
    ret = HksInit(&newKeyAlias, verifyParamSet, &handleVerify, nullptr);
    EXPECT_EQ(ret, HKS_SUCCESS) << "Init failed.";
    // Update loop
    ret = HksTestUpdate(&handleVerify, verifyParamSet, &inData);
    EXPECT_EQ(ret, HKS_SUCCESS) << "Update failed.";
    // Finish
    uint8_t temp[] = "out";
    struct HksBlob verifyOut = { sizeof(temp), temp };
    ret = HksFinish(&handleVerify, verifyParamSet, &outDataSign, &verifyOut);
    EXPECT_EQ(ret, HKS_SUCCESS) << "Finish failed.";

    /* 6. Delete Key */
    ret = HksDeleteKey(&newKeyAlias, verifyParamSet);
    EXPECT_EQ(ret, HKS_SUCCESS) << "Delete ImportKey failed.";

    return ret;
}

int32_t HksRsaSignVerifyTestNormalCase(struct HksBlob keyAlias,
    struct HksParamSet *genParamSet, struct HksParamSet *signParamSet, struct HksParamSet *verifyParamSet)
{
    struct RsaSignVerifyParamStruct param = { false, false, 0 };
    struct HksParam *digestAlg = nullptr;
    int32_t ret = HksGetParam(signParamSet, HKS_TAG_DIGEST, &digestAlg);
    EXPECT_EQ(ret, HKS_SUCCESS) << "GetParam failed.";
    if (digestAlg->uint32Param == HKS_DIGEST_NONE) {
        param.isUseIndataAfterHash = true;
    }
    struct HksParam *keySize = nullptr;
    ret = HksGetParam(signParamSet, HKS_TAG_KEY_SIZE, &keySize);
    EXPECT_EQ(ret, HKS_SUCCESS) << "GetParam failed.";

    if (param.isUseIndataAfterHash) {
        uint32_t arraySize = (sizeof(g_inDataArrayAfterHash) / sizeof(uint8_t *));
        for (uint32_t i = 0; i < arraySize; i++) {
            HKS_LOG_E("HksRsaSignVerifyTestNormalCase loop: %" LOG_PUBLIC "d", i);
            if ((keySize->uint32Param == HKS_RSA_KEY_SIZE_512) && (i >= arraySize - 2)) {
                continue;
            }
            param.hashAlgIndex = i;
            ret = RsaSignVerifyTestNormalCase(keyAlias, genParamSet, signParamSet, verifyParamSet, param);
        }
    } else {
        return RsaSignVerifyTestNormalCase(keyAlias, genParamSet, signParamSet, verifyParamSet, param);
    }

    return HKS_SUCCESS;
}

int32_t HksRsaSignVerifyTestNormalAnotherCase(struct HksBlob keyAlias,
    struct HksParamSet *genParamSet, struct HksParamSet *signParamSet, struct HksParamSet *verifyParamSet)
{
    struct RsaSignVerifyParamStruct param = { false, false, 0 };
    struct HksParam *digestAlg = nullptr;
    int32_t ret = HksGetParam(signParamSet, HKS_TAG_DIGEST, &digestAlg);
    EXPECT_EQ(ret, HKS_SUCCESS) << "GetParam failed.";
    if ((ret == HKS_SUCCESS) && (digestAlg->uint32Param == HKS_DIGEST_NONE)) {
        param.isUseIndataAfterHash = true;
    }

    if (param.isUseIndataAfterHash) {
        for (uint32_t i = 0; i < (sizeof(g_inDataArrayAfterHash) / sizeof(uint8_t *)); i++) {
            param.hashAlgIndex = i;
            HKS_LOG_E("HksRsaSignVerifyTestNormalAnotherCase, loop0: %" LOG_PUBLIC "d", i);
            ret = RsaSignVerifyTestNormalCase(keyAlias, genParamSet, signParamSet, verifyParamSet, param);
            if (ret != HKS_SUCCESS) {
                return ret;
            }
        }
    } else {
        return RsaSignVerifyTestNormalCase(keyAlias, genParamSet, signParamSet, verifyParamSet, param);
    }

    return HKS_SUCCESS;
}

int32_t HksRsaSignVerifyTestParamAbsentCase(struct HksBlob keyAlias,
    struct HksParamSet *genParamSet, struct HksParamSet *signParamSet, struct HksParamSet *verifyParamSet)
{
    (void)verifyParamSet;
    struct HksParam *digestAlg = nullptr;
    int32_t ret = HksGetParam(signParamSet, HKS_TAG_DIGEST, &digestAlg);
    EXPECT_NE(ret, HKS_SUCCESS) << "GetParam failed.";

    /* 1. Generate Key */
    // Generate Key
    ret = HksGenerateKey(&keyAlias, genParamSet, nullptr);
    EXPECT_EQ(ret, HKS_SUCCESS) << "GenerateKey failed.";

    /* 2. Sign Three Stage */
    // Init will fail
    uint8_t handleS[sizeof(uint64_t)] = {0};
    struct HksBlob handleSign = { sizeof(uint64_t), handleS };
    ret = HksInit(&keyAlias, signParamSet, &handleSign, nullptr);
    EXPECT_NE(ret, HKS_SUCCESS) << "Init failed.";
    return HKS_SUCCESS;
}

int32_t RSASignVerifyTestAbnormalCase(struct HksBlob keyAlias, struct HksParamSet *genParamSet,
    struct HksParamSet *signParamSet, struct HksParamSet *verifyParamSet, const uint32_t *hashAlgIndex)
{
    struct HksBlob inData = {
        g_inData.length(),
        const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(g_inData.c_str()))
    };
    struct HksBlob finishInData = { 0, NULL };

    if (hashAlgIndex != nullptr) {
        inData.size = g_inDataArrayAfterHashLen[*hashAlgIndex];
        inData.data = const_cast<uint8_t *>(g_inDataArrayAfterHash[*hashAlgIndex]);
    }

    /* 1. Generate Key */
    // Generate Key
    int32_t ret = HksGenerateKey(&keyAlias, genParamSet, nullptr);
    EXPECT_EQ(ret, HKS_SUCCESS) << "GenerateKey failed.";

    /* 2. Sign Three Stage */
    // Init
    uint8_t handleS[sizeof(uint64_t)] = {0};
    struct HksBlob handleSign = { sizeof(uint64_t), handleS };
    ret = HksInit(&keyAlias, signParamSet, &handleSign, nullptr);
    EXPECT_EQ(ret, HKS_SUCCESS) << "Init failed.";
    // Update loop
    ret = HksTestUpdate(&handleSign, signParamSet, &inData);
    EXPECT_EQ(ret, HKS_SUCCESS) << "Update failed.";
    // Finish
    uint8_t outDataS[RSA_COMMON_SIZE] = {0};
    struct HksBlob outDataSign = { RSA_COMMON_SIZE, outDataS };
    ret = HksFinish(&handleSign, signParamSet, &finishInData, &outDataSign);
    EXPECT_NE(ret, HKS_SUCCESS) << "Finish failed.";
    int32_t abortRet = HksAbort(&handleSign, signParamSet);
    EXPECT_EQ(abortRet, HKS_SUCCESS) << "Abort failed.";

    /* 3. Export Public Key */
    uint8_t pubKey[HKS_RSA_KEY_SIZE_1024] = {0};
    struct HksBlob publicKey = { HKS_RSA_KEY_SIZE_1024, pubKey };
    ret = HksExportPublicKey(&keyAlias, genParamSet, &publicKey);
    EXPECT_EQ(ret, HKS_SUCCESS) << "ExportPublicKey failed.";

    /* 4. Import Key */
    char newKey[] = "RSA_Sign_Verify_Import_KeyAlias";
    struct HksBlob newKeyAlias = { .size = strlen(newKey), .data = reinterpret_cast<uint8_t *>(newKey) };
    ret = HksImportKey(&newKeyAlias, verifyParamSet, &publicKey);
    EXPECT_EQ(ret, HKS_SUCCESS) << "ImportKey failed";

    /* 5. Verify Three Stage */
    // Init
    uint8_t handleV[sizeof(uint64_t)] = {0};
    struct HksBlob handleVerify = { sizeof(uint64_t), handleV };
    ret = HksInit(&newKeyAlias, verifyParamSet, &handleVerify, nullptr);
    EXPECT_EQ(ret, HKS_SUCCESS) << "Init failed.";
    // Update loop
    ret = HksTestUpdate(&handleVerify, verifyParamSet, &inData);
    EXPECT_EQ(ret, HKS_SUCCESS) << "Update failed.";
    // Finish
    uint8_t temp[] = "out";
    struct HksBlob verifyOut = { sizeof(temp), temp };
    ret = HksFinish(&handleVerify, verifyParamSet, &outDataSign, &verifyOut);
    EXPECT_NE(ret, HKS_SUCCESS) << "Finish failed.";
    abortRet = HksAbort(&handleVerify, verifyParamSet);
    EXPECT_EQ(abortRet, HKS_SUCCESS) << "Abort failed.";

    /* 6. Delete New Key */
    int32_t deleteRet = HksDeleteKey(&newKeyAlias, verifyParamSet);
    EXPECT_EQ(deleteRet, HKS_SUCCESS) << "Delete ImportKey failed.";

    return ret;
}

int32_t RSASignVerifyTestAbnormalCaseNoPadding(struct HksBlob keyAlias, struct HksParamSet *genParamSet,
    struct HksParamSet *signParamSet, struct HksParamSet *verifyParamSet, const uint32_t *hashAlgIndex)
{
    /* 1. Generate Key */
    // Generate Key
    int32_t ret = HksGenerateKey(&keyAlias, genParamSet, nullptr);
    EXPECT_EQ(ret, HKS_SUCCESS) << "GenerateKey failed.";

    /* 2. Sign Three Stage */
    // Init
    uint8_t handleS[sizeof(uint64_t)] = {0};
    struct HksBlob handleSign = { sizeof(uint64_t), handleS };
    ret = HksInit(&keyAlias, signParamSet, &handleSign, nullptr);
    EXPECT_NE(ret, HKS_SUCCESS) << "Init failed.";
    return ret;
}


int32_t HksRSASignVerifyTestAbnormalCase(struct HksBlob keyAlias,
    struct HksParamSet *genParamSet, struct HksParamSet *signParamSet, struct HksParamSet *verifyParamSet)
{
    uint32_t hashAlgIndex;
    struct HksParam *digestAlg = nullptr;
    int32_t ret = HksGetParam(signParamSet, HKS_TAG_DIGEST, &digestAlg);
    EXPECT_EQ(ret, HKS_SUCCESS) << "GetParam failed.";
    if (digestAlg->uint32Param == HKS_DIGEST_NONE) {
        for (uint32_t i = 0; i < (sizeof(g_inDataArrayAfterHash) / sizeof(uint8_t *)); i++) {
            hashAlgIndex = i;
            ret = RSASignVerifyTestAbnormalCase(keyAlias,
                genParamSet, signParamSet, verifyParamSet, &hashAlgIndex);
            if (ret != HKS_SUCCESS) {
                return ret;
            }
        }
    } else {
        return RSASignVerifyTestAbnormalCase(keyAlias, genParamSet, signParamSet, verifyParamSet, nullptr);
    }

    return HKS_SUCCESS;
}

int32_t HksRSASignVerifyTestAbnormalCaseNoPadding(struct HksBlob keyAlias,
    struct HksParamSet *genParamSet, struct HksParamSet *signParamSet, struct HksParamSet *verifyParamSet)
{
    uint32_t hashAlgIndex;
    struct HksParam *digestAlg = nullptr;
    int32_t ret = HksGetParam(signParamSet, HKS_TAG_DIGEST, &digestAlg);
    EXPECT_EQ(ret, HKS_SUCCESS) << "GetParam failed.";
    if (digestAlg->uint32Param == HKS_DIGEST_NONE) {
        for (uint32_t i = 0; i < (sizeof(g_inDataArrayAfterHash) / sizeof(uint8_t *)); i++) {
            hashAlgIndex = i;
            ret = RSASignVerifyTestAbnormalCaseNoPadding(keyAlias,
                genParamSet, signParamSet, verifyParamSet, &hashAlgIndex);
            if (ret != HKS_SUCCESS) {
                return ret;
            }
        }
    } else {
        return RSASignVerifyTestAbnormalCaseNoPadding(keyAlias, genParamSet, signParamSet, verifyParamSet, nullptr);
    }

    return HKS_SUCCESS;
}
}
