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
#include "hks_mem.h"
#include "hks_test_log.h"
#include "hks_type.h"

#include "hks_import_wrapped_x25519_suite_test.h"

using namespace testing::ext;
namespace Unittest::ImportWrappedKey {
    class HksImportWrappedX25519SuiteTest : public testing::Test {
    public:
        static void SetUpTestCase(void);

        static void TearDownTestCase(void);

        void SetUp();

        void TearDown();
    };

    void HksImportWrappedX25519SuiteTest::SetUpTestCase(void)
    {
    }

    void HksImportWrappedX25519SuiteTest::TearDownTestCase(void)
    {
    }

    void HksImportWrappedX25519SuiteTest::SetUp()
    {
        EXPECT_EQ(HksInitialize(), 0);
    }

    void HksImportWrappedX25519SuiteTest::TearDown() {
    }


    /* -------- Start of x25519 unwrap algorithm suite common import key material and params define -------- */
    static char g_agreeKeyAlgName[] = "X25519";

    static struct HksBlob g_agreeKeyAlgNameBlob = {
        .size = sizeof(g_agreeKeyAlgName),
        .data = (uint8_t *) g_agreeKeyAlgName
    };

    static const uint32_t g_x25519PubKeySize = 32;

    static struct HksParam g_genWrappingKeyParams[] = {
        {.tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_X25519},
        {.tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_UNWRAP},
        {.tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_CURVE25519_KEY_SIZE_256}
    };

    static struct HksParam g_genCallerX25519Params[] = {
        {.tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_X25519},
        {.tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_AGREE},
        {.tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_CURVE25519_KEY_SIZE_256}
    };

    static struct HksParam g_callerAgreeParams[] = {
        {.tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_X25519},
        {.tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_AGREE},
        {.tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_CURVE25519_KEY_SIZE_256}
    };

    static struct HksParam g_importParamsCallerKek[] = {
        {.tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_AES},
        {.tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT},
        {.tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_256},
        {.tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE},
        {.tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_GCM},
        {.tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_NONE},
        {.tag = HKS_TAG_IV, .blob = {.size = Unittest::ImportWrappedKey::IV_SIZE,
                                     .data = (uint8_t *) Unittest::ImportWrappedKey::IV}
        }
    };
    /* -------- End of x25519 unwrap algorithm suite common import key material and params define -------- */

    /* ------------------ Start of AES-256 import key material and params define ------------------ */
    static struct HksBlob g_importedAes256PlainKey = {
        .size = strlen("This is plain key to be imported"),
        .data = (uint8_t *) "This is plain key to be imported"
    };

    static struct HksBlob g_callerAes256Kek = {
        .size = strlen("This is kek to encrypt plain key"),
        .data = (uint8_t *) "This is kek to encrypt plain key"
    };

    static struct HksParam g_importWrappedAes256Params[] = {
        {.tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_AES},
        {.tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT},
        {.tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_256},
        {.tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE},
        {.tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_GCM},
        {.tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_NONE},
        {.tag = HKS_TAG_UNWRAP_ALGORITHM_SUITE, .uint32Param = HKS_UNWRAP_SUITE_X25519_AES_256_GCM_NOPADDING},
        {.tag = HKS_TAG_ASSOCIATED_DATA, .blob = {.size = Unittest::ImportWrappedKey::AAD_SIZE,
                                                  .data = (uint8_t *) Unittest::ImportWrappedKey::AAD}
        },
        {.tag = HKS_TAG_NONCE, .blob = {
            .size = Unittest::ImportWrappedKey::NONCE_SIZE,
            .data = (uint8_t *) Unittest::ImportWrappedKey::NONCE
        }
        }
    };

    static struct HksBlob g_importedKeyAliasAes256 = {
        .size = strlen("test_import_key_x25519_aes256"),
        .data = (uint8_t *) "test_import_key_x25519_aes256"
    };

    static struct HksBlob g_wrappingKeyAliasAes256 = {
        .size = strlen("test_wrappingKey_x25519_aes256"),
        .data = (uint8_t *) "test_wrappingKey_x25519_aes256"
    };

    static struct HksBlob g_callerKeyAliasAes256 = {
        .size = strlen("test_caller_key_x25519_aes256"),
        .data = (uint8_t *) "test_caller_key_x25519_aes256"
    };

    static struct HksBlob g_callerKekAliasAes256 = {
        .size = strlen("test_caller_kek_x25519_aes256"),
        .data = (uint8_t *) "test_caller_kek_x25519_aes256"
    };

    static struct HksBlob g_callerAgreeKeyAliasAes256 = {
        .size = strlen("test_caller_agree_key_x25519_aes256"),
        .data = (uint8_t *) "test_caller_agree_key_x25519_aes256"
    };
    /* ------------------ End of AES-256 import key material and params define ------------------ */

    /* ------------------ Start of RSA-2048 import key material and params define -------------------- */
    static struct HksBlob g_callerRsa2048Kek = {
        .size = strlen("This  is  rsa2048 kek to encrypt"),
        .data = (uint8_t *) "This  is  rsa2048 kek to encrypt"
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

    static const uint8_t g_eData[] = {0x01, 0x00, 0x01};

    static struct HksParam g_importRsa2048KeyParams[] = {
        {.tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_RSA},
        {.tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT},
        {.tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_RSA_KEY_SIZE_2048},
        {.tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_OAEP},
        {.tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA1},
        {.tag = HKS_TAG_IMPORT_KEY_TYPE, .uint32Param = HKS_KEY_TYPE_KEY_PAIR},
        {.tag = HKS_TAG_UNWRAP_ALGORITHM_SUITE, .uint32Param = HKS_UNWRAP_SUITE_X25519_AES_256_GCM_NOPADDING},
    };

    static struct HksBlob g_importedKeyAliasRsa2048 = {
        .size = strlen("test_import_key_x25519_rsa2048"),
        .data = (uint8_t *) "test_import_key_x25519_rsa2048"
    };

    static struct HksBlob g_wrappingKeyAliasRsa2048 = {
        .size = strlen("test_wrappingKey_x25519_rsa2048"),
        .data = (uint8_t *) "test_wrappingKey_x25519_rsa2048"
    };

    static struct HksBlob g_callerKeyAliasRsa2048 = {
        .size = strlen("test_caller_key_x25519_rsa2048"),
        .data = (uint8_t *) "test_caller_key_x25519_rsa2048"
    };

    static struct HksBlob g_callerKekAliasRsa2048 = {
        .size = strlen("test_caller_kek_x25519_rsa2048"),
        .data = (uint8_t *) "test_caller_kek_x25519_rsa2048"
    };

    static struct HksBlob g_callerAgreeKeyAliasRsa2048 = {
        .size = strlen("test_caller_agree_key_x25519_rsa2048"),
        .data = (uint8_t *) "test_caller_agree_key_x25519_rsa2048"
    };

    struct TestImportKeyData {
        struct HksBlob x509PublicKey;
        struct HksBlob publicOrXData;
        struct HksBlob privateOrYData;
        struct HksBlob zData;
    };
    /* ------------------ End of RSA-2048 import key material and params define -------------------- */

    /* ------------------ Start of X25519 pair import key material and params define -------------------- */
    static struct HksBlob g_callerX25519Kek = {
        .size = strlen("This is x25519 pair  kek encrypt"),
        .data = (uint8_t *) "This is x25519 pair  kek encrypt"
    };

    static const uint8_t g_x25519PubDataOne[] = {
        0x9c, 0xf6, 0x7a, 0x8d, 0xce, 0xc2, 0x7f, 0xa7, 0xd9, 0xfd, 0xf1, 0xad, 0xac, 0xf0, 0xb3, 0x8c,
        0xe8, 0x16, 0xa2, 0x65, 0xcc, 0x18, 0x55, 0x60, 0xcd, 0x2f, 0xf5, 0xe5, 0x72, 0xc9, 0x3c, 0x54,
    };

    static const uint8_t g_x25519PriDataOne[] = {
        0x20, 0xd5, 0xbb, 0x54, 0x6f, 0x1f, 0x00, 0x30, 0x4e, 0x33, 0x38, 0xb9, 0x8e, 0x6a, 0xdf, 0xad,
        0x33, 0x6f, 0x51, 0x23, 0xff, 0x4d, 0x95, 0x26, 0xdc, 0xb0, 0x74, 0xb2, 0x5c, 0x7e, 0x85, 0x6c,
    };

    static struct HksParam g_importX25519PairKeyParams[] = {
        {.tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_X25519},
        {.tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_AGREE},
        {.tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_CURVE25519_KEY_SIZE_256},
        {.tag = HKS_TAG_IMPORT_KEY_TYPE, .uint32Param = HKS_KEY_TYPE_KEY_PAIR},
        {.tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA256},
        {.tag = HKS_TAG_UNWRAP_ALGORITHM_SUITE, .uint32Param = HKS_UNWRAP_SUITE_X25519_AES_256_GCM_NOPADDING},
    };

    static struct HksParam g_importX25519PairKeyParamsAnother[] = {
        {.tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_X25519},
        {.tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_AGREE},
        {.tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_CURVE25519_KEY_SIZE_256},
        {.tag = HKS_TAG_IMPORT_KEY_TYPE, .uint32Param = HKS_KEY_TYPE_KEY_PAIR},
        {.tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA256},
        {.tag = HKS_TAG_UNWRAP_ALGORITHM_SUITE, .uint32Param = HKS_UNWRAP_SUITE_X25519_AES_256_GCM_NOPADDING},
    };

    static struct HksBlob g_importedKeyAliasX25519 = {
        .size = strlen("test_import_key_x25519_x25519"),
        .data = (uint8_t *) "test_import_key_x25519_x25519"
    };

    static struct HksBlob g_wrappingKeyAliasX25519 = {
        .size = strlen("test_wrappingKey_x25519_x25519"),
        .data = (uint8_t *) "test_wrappingKey_x25519_x25519"
    };

    static struct HksBlob g_callerKeyAliasX25519 = {
        .size = strlen("test_caller_key_x25519_x25519"),
        .data = (uint8_t *) "test_caller_key_x25519_x25519"
    };

    static struct HksBlob g_callerKekAliasX25519 = {
        .size = strlen("test_caller_kek_x25519_x25519"),
        .data = (uint8_t *) "test_caller_kek_x25519_x25519"
    };

    static struct HksBlob g_callerAgreeKeyAliasX25519 = {
        .size = strlen("test_caller_agree_key_x25519_x25519"),
        .data = (uint8_t *) "test_caller_agree_key_x25519_x25519"
    };

    /* ------------------ End of X25519 pair import key material and params define -------------------- */

    static int32_t ConstructCurve25519Key(struct TestImportKeyData *key,
        uint32_t alg, uint32_t keySize, uint32_t importType, struct HksBlob *outKey)
    {
        struct HksKeyMaterial25519 material;
        material.keyAlg = (enum HksKeyAlg) alg;
        material.keySize = keySize;
        material.pubKeySize = key->publicOrXData.size;
        material.priKeySize = key->privateOrYData.size;
        material.reserved = 0;

        uint32_t size = sizeof(material) + material.pubKeySize + material.priKeySize;
        uint8_t *data = (uint8_t *) HksMalloc(size);
        if (data == nullptr) {
            return HKS_ERROR_MALLOC_FAIL;
        }

        // copy struct material
        if (memcpy_s(data, size, &material, sizeof(material)) != EOK) {
            HksFree(data);
            return HKS_ERROR_BAD_STATE;
        }

        uint32_t offset = sizeof(material);
        // copy publicData
        if (memcpy_s(data + offset, size - offset, key->publicOrXData.data, key->publicOrXData.size) != EOK) {
            HksFree(data);
            return HKS_ERROR_BAD_STATE;
        }
        offset += material.pubKeySize;

        // copy privateData
        if (memcpy_s(data + offset, size - offset, key->privateOrYData.data, key->privateOrYData.size) != EOK) {
            HksFree(data);
            return HKS_ERROR_BAD_STATE;
        }

        outKey->data = data;
        outKey->size = size;
        return HKS_SUCCESS;
    }

    static int32_t ConstructImportedCurve25519Key(uint32_t alg, uint32_t keySize, uint32_t importType,
        struct HksBlob *outKey)
    {
        struct TestImportKeyData key;
        (void) memset_s(&key, sizeof(key), 0, sizeof(key));
        key.privateOrYData.data = (uint8_t *) g_x25519PriDataOne;
        key.privateOrYData.size = sizeof(g_x25519PriDataOne);
        key.publicOrXData.data = (uint8_t *) g_x25519PubDataOne;
        key.publicOrXData.size = sizeof(g_x25519PubDataOne);

        return ConstructCurve25519Key(&key, alg, keySize, importType, outKey);
    }


    static int32_t ConstructRsaKey(const struct HksBlob *nDataBlob, const struct HksBlob *dDataBlob,
        uint32_t keySize, struct HksBlob *outKey, bool isPriKey)
    {
        struct HksKeyMaterialRsa material;
        material.keyAlg = HKS_ALG_RSA;
        material.keySize = keySize;
        material.nSize = nDataBlob->size;
        material.eSize = isPriKey ? 0 : sizeof(g_eData);
        material.dSize = dDataBlob->size;

        uint32_t size = sizeof(material) + material.nSize + material.eSize + material.dSize;
        uint8_t *data = (uint8_t *) HksMalloc(size);
        if (data == nullptr) {
            return HKS_ERROR_MALLOC_FAIL;
        }

        // copy struct material
        if (memcpy_s(data, size, &material, sizeof(material)) != EOK) {
            HksFree(data);
            return HKS_ERROR_BAD_STATE;
        }

        uint32_t offset = sizeof(material);
        // copy nData
        if (memcpy_s(data + offset, size - offset, nDataBlob->data, nDataBlob->size) != EOK) {
            HksFree(data);
            return HKS_ERROR_BAD_STATE;
        }

        offset += material.nSize;
        // copy eData
        if (!isPriKey) {
            if (memcpy_s(data + offset, size - offset, &g_eData, sizeof(g_eData)) != EOK) {
                HksFree(data);
                return HKS_ERROR_BAD_STATE;
            }
            offset += material.eSize;
        }

        // copy dData
        if (memcpy_s(data + offset, size - offset, dDataBlob->data, dDataBlob->size) != EOK) {
            HksFree(data);
            return HKS_ERROR_BAD_STATE;
        }

        outKey->data = data;
        outKey->size = size;
        return HKS_SUCCESS;
    }

    static void InitCommonTestParamsAndDoImport(struct HksImportWrappedKeyTestParams *importWrappedKeyTestParams,
        const struct HksParam *importedKeyParamSetArray, uint32_t arraySize)
    {
        int32_t ret = 0;
        importWrappedKeyTestParams->agreeKeyAlgName = &g_agreeKeyAlgNameBlob;

        struct HksParamSet *genX25519KeyParamSet = nullptr;
        ret = InitParamSet(&genX25519KeyParamSet, g_genWrappingKeyParams,
                           sizeof(g_genWrappingKeyParams) / sizeof(HksParam));
        EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet(gen huks x25519) failed.";
        importWrappedKeyTestParams->genWrappingKeyParamSet = genX25519KeyParamSet;
        importWrappedKeyTestParams->publicKeySize = g_x25519PubKeySize;

        struct HksParamSet *genCallerKeyParamSet = nullptr;
        ret = InitParamSet(&genCallerKeyParamSet, g_genCallerX25519Params,
                           sizeof(g_genCallerX25519Params) / sizeof(HksParam));
        EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet(gen caller x25519) failed.";
        importWrappedKeyTestParams->genCallerKeyParamSet = genCallerKeyParamSet;

        struct HksParamSet *callerImportParamsKek = nullptr;
        ret = InitParamSet(&callerImportParamsKek, g_importParamsCallerKek,
                           sizeof(g_importParamsCallerKek) / sizeof(HksParam));
        EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet(import call kek) failed.";
        importWrappedKeyTestParams->importCallerKekParamSet = callerImportParamsKek;

        struct HksParamSet *agreeParamSet = nullptr;
        ret = InitParamSet(&agreeParamSet, g_callerAgreeParams,
                           sizeof(g_callerAgreeParams) / sizeof(HksParam));
        EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet(agreeParamSet) failed.";
        importWrappedKeyTestParams->agreeParamSet = agreeParamSet;

        struct HksParamSet *importPlainKeyParams = nullptr;
        ret = InitParamSet(&importPlainKeyParams, importedKeyParamSetArray, arraySize);
        EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet(import plain key) failed.";
        importWrappedKeyTestParams->importWrappedKeyParamSet = importPlainKeyParams;

        HksImportWrappedKeyTestCommonCase(importWrappedKeyTestParams);

        HksFreeParamSet(&genX25519KeyParamSet);
        HksFreeParamSet(&genCallerKeyParamSet);
        HksFreeParamSet(&callerImportParamsKek);
        HksFreeParamSet(&importPlainKeyParams);
    }

    /**
     * @tc.name: HksImportWrappedX25519SuiteTest.HksImportWrappedKeyTestX25519Suite001
     * @tc.desc: Test import wrapped aes256-gcm-no_padding key including generate&export x25519 key, generate kek,
     * agree, encrypt, of which generate kek, agree, encrypt should done by caller self.
     * @tc.type: FUNC
     */
    HWTEST_F(HksImportWrappedX25519SuiteTest, HksImportWrappedKeyTestX25519Suite001, TestSize.Level0)
    {
        struct HksImportWrappedKeyTestParams importWrappedKeyTestParams001 = {0};

        importWrappedKeyTestParams001.wrappingKeyAlias = &g_wrappingKeyAliasAes256;
        importWrappedKeyTestParams001.keyMaterialLen = g_importedAes256PlainKey.size;
        importWrappedKeyTestParams001.callerKeyAlias = &g_callerKeyAliasAes256;
        importWrappedKeyTestParams001.callerKekAlias = &g_callerKekAliasAes256;
        importWrappedKeyTestParams001.callerKek = &g_callerAes256Kek;
        importWrappedKeyTestParams001.callerAgreeKeyAlias = &g_callerAgreeKeyAliasAes256;
        importWrappedKeyTestParams001.importedKeyAlias = &g_importedKeyAliasAes256;
        importWrappedKeyTestParams001.importedPlainKey = &g_importedAes256PlainKey;
        InitCommonTestParamsAndDoImport(&importWrappedKeyTestParams001, g_importWrappedAes256Params,
                                        sizeof(g_importWrappedAes256Params) / sizeof(struct HksParam));
        HksClearKeysForWrappedKeyTest(&importWrappedKeyTestParams001);
    }

    /**
     * @tc.name: HksImportWrappedX25519SuiteTest.HksImportWrappedKeyTestX25519Suite002
     * @tc.desc: Test import wrapped rsa key pair including generate&export x25519 key, generate kek, agree, encrypt,
     *           of which generate kek, agree, encrypt should done by caller self.
     * @tc.type: FUNC
     */
    HWTEST_F(HksImportWrappedX25519SuiteTest, HksImportWrappedKeyTestX25519Suite002, TestSize.Level0)
    {
        struct HksBlob nDataBlob = {sizeof(g_nData2048), (uint8_t *) g_nData2048};
        struct HksBlob dDataBlob = {sizeof(g_dData2048), (uint8_t *) g_dData2048};
        struct HksBlob plainKey = {0, nullptr};

        int32_t ret = ConstructRsaKey(&nDataBlob, &dDataBlob, HKS_RSA_KEY_SIZE_2048, &plainKey, false);
        EXPECT_EQ(ret, HKS_SUCCESS) << "construct rsa 2048 failed.";

        struct HksImportWrappedKeyTestParams importWrappedKeyTestParams002 = {0};

        importWrappedKeyTestParams002.wrappingKeyAlias = &g_wrappingKeyAliasRsa2048;
        importWrappedKeyTestParams002.agreeKeyAlgName = &g_agreeKeyAlgNameBlob;
        importWrappedKeyTestParams002.publicKeySize = g_x25519PubKeySize;
        importWrappedKeyTestParams002.keyMaterialLen = plainKey.size;

        importWrappedKeyTestParams002.callerKeyAlias = &g_callerKeyAliasRsa2048;
        importWrappedKeyTestParams002.callerKekAlias = &g_callerKekAliasRsa2048;
        importWrappedKeyTestParams002.callerKek = &g_callerRsa2048Kek;
        importWrappedKeyTestParams002.callerAgreeKeyAlias = &g_callerAgreeKeyAliasRsa2048;
        importWrappedKeyTestParams002.importedKeyAlias = &g_importedKeyAliasRsa2048;
        importWrappedKeyTestParams002.importedPlainKey = &plainKey;
        InitCommonTestParamsAndDoImport(&importWrappedKeyTestParams002, g_importRsa2048KeyParams,
                                        sizeof(g_importRsa2048KeyParams) / sizeof(struct HksParam));
        HksClearKeysForWrappedKeyTest(&importWrappedKeyTestParams002);
    }

    /**
     * @tc.name: HksImportWrappedX25519SuiteTest.HksImportWrappedKeyTestX25519Suite003
     * @tc.desc: Test import wrapped x25519 key pair including generate&export x25519 key, generate kek, agree, encrypt,
     *           of which generate kek, agree, encrypt should done by caller self.
     * @tc.type: FUNC
     */
    HWTEST_F(HksImportWrappedX25519SuiteTest, HksImportWrappedKeyTestX25519Suite003, TestSize.Level0)
    {
        struct HksBlob plainKey = {0, nullptr};
        int32_t ret = ConstructImportedCurve25519Key(HKS_ALG_X25519, HKS_CURVE25519_KEY_SIZE_256, HKS_KEY_TYPE_KEY_PAIR,
                                                     &plainKey);
        EXPECT_EQ(ret, HKS_SUCCESS) << "construct x25519 pair failed.";

        struct HksImportWrappedKeyTestParams importWrappedKeyTestParams003 = {0};

        importWrappedKeyTestParams003.wrappingKeyAlias = &g_wrappingKeyAliasX25519;
        importWrappedKeyTestParams003.keyMaterialLen = plainKey.size;
        importWrappedKeyTestParams003.callerKeyAlias = &g_callerKeyAliasX25519;
        importWrappedKeyTestParams003.callerKekAlias = &g_callerKekAliasX25519;
        importWrappedKeyTestParams003.callerKek = &g_callerX25519Kek;
        importWrappedKeyTestParams003.callerAgreeKeyAlias = &g_callerAgreeKeyAliasX25519;
        importWrappedKeyTestParams003.importedKeyAlias = &g_importedKeyAliasX25519;
        importWrappedKeyTestParams003.importedPlainKey = &plainKey;
        InitCommonTestParamsAndDoImport(&importWrappedKeyTestParams003, g_importX25519PairKeyParams,
                                        sizeof(g_importX25519PairKeyParams) / sizeof(struct HksParam));
        HksClearKeysForWrappedKeyTest(&importWrappedKeyTestParams003);
    }

   /**
     * @tc.name: HksImportWrappedX25519SuiteTest.HksImportWrappedKeyTestX25519Suite004
     * @tc.desc: Test import wrapped x25519 key pair including generate&export x25519 key, generate kek, agree, encrypt,
     *           of which generate kek, agree, encrypt should done by caller self.When importing the key, only the
     *           necessary parameters are passed in.
     * @tc.type: FUNC
     * @tc.require:issueI611S5
     */
    HWTEST_F(HksImportWrappedX25519SuiteTest, HksImportWrappedKeyTestX25519Suite004, TestSize.Level0)
    {
        HKS_LOG_E("Enter HksImportWrappedKeyTestX25519Suite004");
        struct HksBlob plainKey = {0, nullptr};
        int32_t ret = ConstructImportedCurve25519Key(HKS_ALG_X25519, HKS_CURVE25519_KEY_SIZE_256, HKS_KEY_TYPE_KEY_PAIR,
                                                     &plainKey);
        EXPECT_EQ(ret, HKS_SUCCESS) << "construct x25519 pair failed.";

        struct HksImportWrappedKeyTestParams importWrappedKeyTestParams004 = {0};

        importWrappedKeyTestParams004.wrappingKeyAlias = &g_wrappingKeyAliasX25519;
        importWrappedKeyTestParams004.keyMaterialLen = plainKey.size;
        importWrappedKeyTestParams004.callerKeyAlias = &g_callerKeyAliasX25519;
        importWrappedKeyTestParams004.callerKekAlias = &g_callerKekAliasX25519;
        importWrappedKeyTestParams004.callerKek = &g_callerX25519Kek;
        importWrappedKeyTestParams004.callerAgreeKeyAlias = &g_callerAgreeKeyAliasX25519;
        importWrappedKeyTestParams004.importedKeyAlias = &g_importedKeyAliasX25519;
        importWrappedKeyTestParams004.importedPlainKey = &plainKey;
        InitCommonTestParamsAndDoImport(&importWrappedKeyTestParams004, g_importX25519PairKeyParamsAnother,
                                        sizeof(g_importX25519PairKeyParamsAnother) / sizeof(struct HksParam));
        HksClearKeysForWrappedKeyTest(&importWrappedKeyTestParams004);
    }
}
