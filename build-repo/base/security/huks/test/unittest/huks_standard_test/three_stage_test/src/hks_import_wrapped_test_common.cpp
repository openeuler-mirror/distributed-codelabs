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

#include "hks_mem.h"
#include "hks_test_log.h"
#include "hks_type.h"
#include "hks_param.h"
#include "hks_three_stage_test_common.h"

#include "hks_import_wrapped_test_common.h"

using namespace testing::ext;
namespace Unittest::ImportWrappedKey {
    static struct HksParam g_aesKekEncryptParams[] = {
        {.tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_AES},
        {.tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT},
        {.tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_256},
        {.tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE},
        {.tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_GCM},
        {.tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_NONE},
        {.tag = HKS_TAG_ASSOCIATED_DATA, .blob = {.size = Unittest::ImportWrappedKey::AAD_SIZE,
                                                  .data = (uint8_t *) Unittest::ImportWrappedKey::AAD}
        },
        { .tag = HKS_TAG_NONCE, .blob = {.size = Unittest::ImportWrappedKey::NONCE_SIZE,
                                         .data = (uint8_t *) Unittest::ImportWrappedKey::NONCE}
        }
    };

    static struct HksParam g_importAgreeKeyParams[] = {
        {.tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_AES},
        {.tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT},
        {.tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_256},
        {.tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE},
        {.tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_GCM},
        {.tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_NONE},
        {.tag = HKS_TAG_IV, .blob = {.size = Unittest::ImportWrappedKey::IV_SIZE,
                                     .data = (uint8_t *) Unittest::ImportWrappedKey::IV }
        }
    };

    static int32_t BuildWrappedKeyData(struct HksBlob **blobArray, uint32_t size, struct HksBlob *outData)
    {
        uint32_t totalLength = size * sizeof(uint32_t);

        /* counter size */
        for (uint32_t i = 0; i < size; ++i) {
            totalLength += blobArray[i]->size;
        }

        struct HksBlob outBlob = { 0, nullptr };
        outBlob.size = totalLength;
        (void)MallocAndCheckBlobData(&outBlob, outBlob.size);

        uint32_t offset = 0;

        /* copy data */
        for (uint32_t i = 0; i < size; ++i) {
            if (memcpy_s(outBlob.data + offset, totalLength - offset, reinterpret_cast<uint8_t *>(&blobArray[i]->size),
                sizeof(blobArray[i]->size)) != EOK) {
                return HKS_ERROR_BUFFER_TOO_SMALL;
            }
            offset += sizeof(blobArray[i]->size);

            if (memcpy_s(outBlob.data + offset, totalLength - offset, blobArray[i]->data,
                blobArray[i]->size) != EOK) {
                return HKS_ERROR_BUFFER_TOO_SMALL;
            }
            offset += blobArray[i]->size;
        }

        outData->size = outBlob.size;
        outData->data = outBlob.data;
        return HKS_SUCCESS;
    }

    static int32_t CheckParamsValid(const struct HksImportWrappedKeyTestParams *params)
    {
        if (params == nullptr) {
            return HKS_ERROR_INVALID_ARGUMENT;
        }

        if (params->wrappingKeyAlias == nullptr || params->genWrappingKeyParamSet == nullptr ||
            params->agreeKeyAlgName == nullptr || params->callerKeyAlias == nullptr ||
            params->genCallerKeyParamSet == nullptr || params->callerKekAlias == nullptr ||
            params->callerKek == nullptr || params->importCallerKekParamSet == nullptr ||
            params->callerAgreeKeyAlias == nullptr || params->agreeParamSet == nullptr ||
            params->importWrappedKeyParamSet == nullptr || params->importedKeyAlias == nullptr ||
            params->importedPlainKey == nullptr) {
            return HKS_ERROR_INVALID_ARGUMENT;
        }
        return HKS_SUCCESS;
    }

    static void GenerateAndExportHuksPublicKey(const struct HksImportWrappedKeyTestParams *params,
        struct HksBlob *huksPublicKey)
    {
        HKS_TEST_LOG_I("-------------1. Test_GenerateHuks[%s]Key1\n",
            reinterpret_cast<char *>(params->agreeKeyAlgName->data));
        int32_t ret = HksGenerateKey(params->wrappingKeyAlias, params->genWrappingKeyParamSet, nullptr);
        EXPECT_EQ(ret, HKS_SUCCESS) << "Generate huks key failed.";

        HKS_TEST_LOG_I("-------------2. Test_ExportHuks[%s]Key1\n",
            reinterpret_cast<char *>(params->agreeKeyAlgName->data));
        huksPublicKey->size = params->publicKeySize;

        EXPECT_EQ(MallocAndCheckBlobData(huksPublicKey, huksPublicKey->size),
            HKS_SUCCESS) << "Malloc pub key failed.";
        ret = HksExportPublicKey(params->wrappingKeyAlias, nullptr, huksPublicKey);
        EXPECT_EQ(ret, HKS_SUCCESS) << "Export huks public key failed.";
    }

    static void GenerateAndExportCallerPublicKey(const struct HksImportWrappedKeyTestParams *params,
        struct HksBlob *callerSelfPublicKey)
    {
        HKS_TEST_LOG_I("-------------3. Test_GenerateCallerSelf[%s]Key!\n",
            reinterpret_cast<char *>(params->agreeKeyAlgName->data));
        int32_t ret = HksGenerateKey(params->callerKeyAlias, params->genCallerKeyParamSet, nullptr);
        EXPECT_EQ(ret, HKS_SUCCESS) << "Generate caller key failed.";

        callerSelfPublicKey->size = params->publicKeySize;
        EXPECT_EQ(MallocAndCheckBlobData(callerSelfPublicKey,
            callerSelfPublicKey->size), HKS_SUCCESS) << "malloc fail";
        ret = HksExportPublicKey(params->callerKeyAlias, params->genWrappingKeyParamSet, callerSelfPublicKey);
        EXPECT_EQ(ret, HKS_SUCCESS) << "Export caller public key failed.";
    }

    static void ImportKekAndAgreeSharedSecret(const struct HksImportWrappedKeyTestParams *params,
        const struct HksBlob *huksPublicKey, struct HksBlob *outSharedKey)
    {
        HKS_TEST_LOG_I("-------------4. Test_ImportCallerSelfKek!\n");
        int32_t ret = HksImportKey(params->callerKekAlias, params->importCallerKekParamSet, params->callerKek);
        EXPECT_EQ(ret, HKS_SUCCESS) << "ImportCallerSelfKek failed.";

        HKS_TEST_LOG_I("-------------5. Test_CallerAgree[%s]Key and Import To Huks!\n",
                       reinterpret_cast<char *>(params->agreeKeyAlgName->data));
        EXPECT_EQ(MallocAndCheckBlobData(outSharedKey, outSharedKey->size), HKS_SUCCESS) << "Malloc sharedKey failed.";

        ret = HksAgreeKey(params->agreeParamSet, params->callerKeyAlias, huksPublicKey, outSharedKey);
        EXPECT_EQ(ret, HKS_SUCCESS) << "HksAgreeKey with huks public key and caller private key failed.";

        struct HksParamSet *importAgreeKeyParams = nullptr;
        ret = InitParamSet(&importAgreeKeyParams, g_importAgreeKeyParams,
                           sizeof(g_importAgreeKeyParams) / sizeof(HksParam));
        EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet(importAgreeKey) failed.";

        ret = HksImportKey(params->callerAgreeKeyAlias, importAgreeKeyParams, outSharedKey);
        EXPECT_EQ(ret, HKS_SUCCESS) << "import agree shared key failed.";
        HksFreeParamSet(&importAgreeKeyParams);
    }

    static void EncryptImportedPlainKeyAndKek(const struct HksImportWrappedKeyTestParams *params,
        struct HksBlob *plainCipherText, struct HksBlob *kekCipherText)
    {
        HKS_TEST_LOG_I("-------------6. Test_CallerEncryptPlainKeyUseKek!\n");
        struct HksParamSet *encryptParamSet = nullptr;
        int32_t ret = InitParamSet(&encryptParamSet, g_aesKekEncryptParams,
                                   sizeof(g_aesKekEncryptParams) / sizeof(HksParam));
        EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet(aesKekEnc) failed.";
        ret = HksEncrypt(params->callerKekAlias, encryptParamSet, params->importedPlainKey, plainCipherText);
        EXPECT_EQ(ret, HKS_SUCCESS) << "HksEncrypt plain key to be imported failed.";

        HKS_TEST_LOG_I("-------------7. Test_CallerEncryptKekUseAgreeKey!\n");
        ret = HksEncrypt(params->callerAgreeKeyAlias, encryptParamSet, params->callerKek, kekCipherText);
        EXPECT_EQ(ret, HKS_SUCCESS) << "Kek encrypt failed.";
        HksFreeParamSet(&encryptParamSet);
    }

    static void ImportWrappedKey(const struct HksImportWrappedKeyTestParams *params, struct HksBlob *plainCipher,
        struct HksBlob *kekCipherText, struct HksBlob *peerPublicKey, struct HksBlob *wrappedKeyData)
    {
        HKS_TEST_LOG_I("-------------8. Test_ImportWrappedKey!\n");
        struct HksBlob commonAad = {.size = Unittest::ImportWrappedKey::AAD_SIZE,
                                    .data = reinterpret_cast<uint8_t *>(Unittest::ImportWrappedKey::AAD)};
        struct HksBlob commonNonce = {.size = Unittest::ImportWrappedKey::NONCE_SIZE,
                                      .data = reinterpret_cast<uint8_t *>(Unittest::ImportWrappedKey::NONCE)};
        struct HksBlob keyMaterialLen = {.size = sizeof(uint32_t), .data = (uint8_t *)&params->keyMaterialLen };

        /* copy AEAD tag from cipher text and decrease its size */
        const uint32_t tagSize = Unittest::ImportWrappedKey::AEAD_TAG_SIZE;
        uint8_t kekTagBuf[tagSize] = {0};
        struct HksBlob kekTag = { .size = tagSize, .data = kekTagBuf };
        if (memcpy_s(kekTag.data, tagSize, plainCipher->data + (plainCipher->size - tagSize), tagSize) != EOK) {
            EXPECT_EQ(HKS_ERROR_BUFFER_TOO_SMALL, EOK) << "memcpy kek tag failed.";
        }
        plainCipher->size -= tagSize;

        /* copy AEAD tag from kek cipher text and decrease its size */
        uint8_t agreeKeyTagBuf[tagSize] = {0};
        struct HksBlob agreeKeyTag = { .size = tagSize, .data = agreeKeyTagBuf };
        if (memcpy_s(agreeKeyTagBuf, tagSize, kekCipherText->data + (kekCipherText->size - tagSize), tagSize) != EOK) {
            EXPECT_EQ(HKS_ERROR_BUFFER_TOO_SMALL, EOK) << "memcpy agreekey tag failed.";
        }
        kekCipherText->size -= tagSize;

        struct HksBlob *blobArray[] = { peerPublicKey, &commonAad, &commonNonce, &agreeKeyTag, kekCipherText,
                                        &commonAad, &commonNonce, &kekTag, &keyMaterialLen, plainCipher };
        int32_t ret = BuildWrappedKeyData(blobArray, HKS_IMPORT_WRAPPED_KEY_TOTAL_BLOBS, wrappedKeyData);
        EXPECT_EQ(ret, HKS_SUCCESS) << "BuildWrappedKeyData failed.";
        
        struct HksParam *purpose = nullptr;
        ret = HksGetParam(params->importWrappedKeyParamSet, HKS_TAG_PURPOSE, &purpose);
        EXPECT_EQ(ret, HKS_SUCCESS) << "Get wrapped purpose param failed.";

        ret = HksImportWrappedKey(params->importedKeyAlias, params->wrappingKeyAlias,
                                  params->importWrappedKeyParamSet, wrappedKeyData);

        if (purpose->uint32Param == (uint32_t)HKS_KEY_PURPOSE_UNWRAP) {
            EXPECT_EQ(ret, HKS_ERROR_INVALID_PURPOSE) << "Import unwrap purpose wrapped key shouldn't be success.";
        } else {
            EXPECT_EQ(ret, HKS_SUCCESS) << "HksImportWrappedKey failed.";
        }
    }

    void HksImportWrappedKeyTestCommonCase(const struct HksImportWrappedKeyTestParams *params)
    {
        int32_t ret = CheckParamsValid(params);
        EXPECT_EQ(ret, HKS_SUCCESS) << "CheckParamsValid failed.";
        if (ret != HKS_SUCCESS) {
            return;
        }

        struct HksBlob huksPublicKey = { 0, nullptr };
        struct HksBlob callerSelfPublicKey = { 0, nullptr };
        struct HksBlob outSharedKey = { .size = HKS_KEY_BYTES(HKS_AES_KEY_SIZE_256), .data = nullptr };
        struct HksBlob wrappedKeyData = { 0, nullptr };
        uint8_t plainKeyCipherBuffer[MAX_KEY_SIZE] = {0};
        struct HksBlob plainCipherText = { MAX_KEY_SIZE, plainKeyCipherBuffer };
        uint8_t kekCipherTextBuffer[MAX_KEY_SIZE] = {0};
        struct HksBlob kekCipherText = { MAX_KEY_SIZE, kekCipherTextBuffer };

        GenerateAndExportHuksPublicKey(params, &huksPublicKey);
        GenerateAndExportCallerPublicKey(params, &callerSelfPublicKey);
        ImportKekAndAgreeSharedSecret(params, &huksPublicKey, &outSharedKey);
        EncryptImportedPlainKeyAndKek(params, &plainCipherText, &kekCipherText);
        ImportWrappedKey(params, &plainCipherText, &kekCipherText, &callerSelfPublicKey, &wrappedKeyData);

        HKS_FREE_BLOB(huksPublicKey);
        HKS_FREE_BLOB(callerSelfPublicKey);
        HKS_FREE_BLOB(outSharedKey);
        HKS_FREE_BLOB(wrappedKeyData);
    }

    void HksClearKeysForWrappedKeyTest(const struct HksImportWrappedKeyTestParams *params)
    {
        int32_t ret = CheckParamsValid(params);
        EXPECT_EQ(ret, HKS_SUCCESS) << "CheckParamsValid failed.";
        if (ret != HKS_SUCCESS) {
            return;
        }
        (void)HksDeleteKey(params->wrappingKeyAlias, nullptr);
        (void)HksDeleteKey(params->callerKeyAlias, nullptr);
        (void)HksDeleteKey(params->callerKekAlias, nullptr);
        (void)HksDeleteKey(params->callerAgreeKeyAlias, nullptr);
        (void)HksDeleteKey(params->importedKeyAlias, nullptr);
    }
}
