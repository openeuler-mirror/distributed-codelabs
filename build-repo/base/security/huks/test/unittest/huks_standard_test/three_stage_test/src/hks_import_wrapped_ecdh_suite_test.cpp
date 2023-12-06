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

#include "hks_import_wrapped_ecdh_suite_test.h"

using namespace testing::ext;
namespace Unittest::ImportWrappedKey {
    class HksImportWrappedEcdhSuiteTest : public testing::Test {
    public:
        static void SetUpTestCase(void);

        static void TearDownTestCase(void);

        void SetUp();

        void TearDown();
    };

    void HksImportWrappedEcdhSuiteTest::SetUpTestCase(void)
    {
    }

    void HksImportWrappedEcdhSuiteTest::TearDownTestCase(void)
    {
    }

    void HksImportWrappedEcdhSuiteTest::SetUp()
    {
        EXPECT_EQ(HksInitialize(), 0);
    }

    void HksImportWrappedEcdhSuiteTest::TearDown()
    {
    }


    /* -------- Start of Ecdh unwrap algorithm suite common import key material and params define -------- */
    static char g_agreeKeyAlgName[] = "ECDH";

    static struct HksBlob g_agreeKeyAlgNameBlob = {
        .size = sizeof(g_agreeKeyAlgName),
        .data = (uint8_t *) g_agreeKeyAlgName
    };

    static const uint32_t g_ecdhPubKeySize = HKS_ECC_KEY_SIZE_256;

    static struct HksParam g_genWrappingKeyParams[] = {
        {.tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_ECC},
        {.tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_UNWRAP},
        {.tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_ECC_KEY_SIZE_256},
        {.tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_NONE},
        {.tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE},
        {.tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CBC}
    };

    static struct HksParam g_genCallerEcdhParams[] = {
        {.tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_ECC},
        {.tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_AGREE},
        {.tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_ECC_KEY_SIZE_256},
        {.tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_NONE},
        {.tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE},
        {.tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CBC}
    };

    static struct HksParam g_callerAgreeParams[] = {
        {.tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_ECDH},
        {.tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_AGREE},
        {.tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_ECC_KEY_SIZE_256}
    };

    static struct HksParam g_importParamsCallerKek[] = {
        {.tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_AES},
        {.tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT},
        {.tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_256},
        {.tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE},
        {.tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_GCM},
        {.tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_NONE},
        {.tag = HKS_TAG_IV, .blob =
            {.size = Unittest::ImportWrappedKey::IV_SIZE, .data = (uint8_t *) Unittest::ImportWrappedKey::IV}
        }
    };
    /* -------- End of x25519 unwrap algorithm suite common import key material and params define -------- */

    /* ------------------ Start of AES-256 import key material and params define ------------------ */
    static struct HksBlob g_importedAes192PlainKey = {
        .size = strlen("The aes192 key to import"),
        .data = (uint8_t *) "The aes192 key to import"
    };

    static struct HksBlob g_callerAes256Kek = {
        .size = strlen("The is kek to encrypt aes192 key"),
        .data = (uint8_t *) "The is kek to encrypt aes192 key"
    };

    static struct HksParam g_importWrappedAes256Params[] = {
        {.tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_AES},
        {.tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT},
        {.tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_192},
        {.tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE},
        {.tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CBC},
        {.tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_NONE},
        {.tag = HKS_TAG_UNWRAP_ALGORITHM_SUITE, .uint32Param = HKS_UNWRAP_SUITE_ECDH_AES_256_GCM_NOPADDING},
        {.tag = HKS_TAG_IV, .blob = { .size = IV_SIZE, .data = (uint8_t*)IV} },
    };

    static struct HksBlob g_importedKeyAliasAes256 = {
        .size = strlen("test_import_key_ecdh_aes256"),
        .data = (uint8_t *) "test_import_key_ecdh_aes256"
    };

    static struct HksBlob g_wrappingKeyAliasAes256 = {
        .size = strlen("test_wrappingKey_ecdh_aes256"),
        .data = (uint8_t *) "test_wrappingKey_ecdh_aes256"
    };

    static struct HksBlob g_callerKeyAliasAes256 = {
        .size = strlen("test_caller_key_ecdh_aes256"),
        .data = (uint8_t *) "test_caller_key_ecdh_aes256"
    };

    static struct HksBlob g_callerKekAliasAes256 = {
        .size = strlen("test_caller_kek_ecdh_aes256"),
        .data = (uint8_t *) "test_caller_kek_ecdh_aes256"
    };

    static struct HksBlob g_callerAgreeKeyAliasAes256 = {
        .size = strlen("test_caller_agree_key_ecdh_aes256"),
        .data = (uint8_t *) "test_caller_agree_key_ecdh_aes256"
    };
    /* ------------------ End of AES-256 import key material and params define ------------------ */

    /* ------------------ Start of RSA-4096 import key material and params define -------------------- */
    static struct HksBlob g_callerRsa4096Kek = {
        .size = strlen("This  is  Rsa4096 kek to encrypt"),
        .data = (uint8_t *) "This  is  Rsa4096 kek to encrypt"
    };


    static const uint8_t g_nData4096[] = {
        0xcd, 0x1f, 0x40, 0xcd, 0x37, 0x56, 0x2f, 0x78, 0xbb, 0x27, 0x1e, 0xba, 0x66, 0xba, 0x84, 0xc6,
        0xdb, 0xa0, 0x72, 0xf3, 0x1b, 0x53, 0x8c, 0x46, 0x99, 0x65, 0x0d, 0xf7, 0xb4, 0xae, 0x19, 0x46,
        0x1e, 0x05, 0xb0, 0xbc, 0xe1, 0x84, 0x9b, 0x44, 0xc5, 0x06, 0x18, 0xef, 0x68, 0x4b, 0x80, 0xf4,
        0xfc, 0x00, 0x79, 0x8e, 0x21, 0x5e, 0x8f, 0x26, 0x65, 0x69, 0x09, 0x31, 0x4b, 0xa0, 0x95, 0x37,
        0x53, 0xa2, 0xf5, 0x52, 0x78, 0xb5, 0x3c, 0xec, 0x58, 0x25, 0x1f, 0x23, 0x99, 0x89, 0xc1, 0x46,
        0x29, 0x65, 0xbb, 0xbe, 0x59, 0x4c, 0xe4, 0x6f, 0xfc, 0xeb, 0x53, 0xe5, 0x33, 0x6e, 0x9a, 0x14,
        0x57, 0x2a, 0x39, 0xc5, 0xec, 0x4e, 0xc3, 0x3e, 0xcf, 0x76, 0xa3, 0xe9, 0xf8, 0xe3, 0x8a, 0x40,
        0x1b, 0x3d, 0x07, 0x0e, 0xe4, 0x8e, 0xc7, 0x03, 0xbe, 0xf0, 0x3b, 0xb1, 0x25, 0x4d, 0x20, 0x07,
        0x9d, 0x2a, 0x18, 0x92, 0x4b, 0x62, 0x34, 0x73, 0xbc, 0x60, 0x3c, 0x18, 0xa2, 0xf8, 0x23, 0x3f,
        0x43, 0xa5, 0xd8, 0x0c, 0x3f, 0x36, 0xd3, 0x4a, 0x83, 0xe8, 0x93, 0xce, 0x55, 0x6b, 0x31, 0xeb,
        0x8e, 0x8d, 0xc0, 0x01, 0x96, 0xee, 0x4c, 0x15, 0xf1, 0x21, 0x5b, 0x34, 0xe9, 0x42, 0x1a, 0x4a,
        0x7b, 0x6f, 0x58, 0x0b, 0x44, 0x4b, 0xc6, 0x9e, 0x15, 0x1c, 0xb9, 0x49, 0x97, 0x99, 0xfc, 0x4b,
        0xac, 0x9e, 0xc8, 0xfe, 0x97, 0xf7, 0x56, 0x62, 0x7e, 0x8f, 0x0e, 0xd2, 0x8a, 0xc3, 0x65, 0x72,
        0xe8, 0xcd, 0xb0, 0xbb, 0x21, 0x95, 0xb5, 0x4a, 0x6e, 0x57, 0x13, 0xf9, 0x68, 0x9f, 0xab, 0x53,
        0xe8, 0xab, 0x10, 0x36, 0x6f, 0x0f, 0x53, 0x53, 0x0b, 0xcd, 0x9b, 0x16, 0x7f, 0x33, 0xfa, 0x45,
        0x2d, 0x52, 0x4d, 0x67, 0x5d, 0x4e, 0xcd, 0x67, 0xef, 0x32, 0x09, 0x09, 0xe6, 0x26, 0x37, 0x63,
        0x4e, 0x43, 0x4e, 0x70, 0xc1, 0xc8, 0xc7, 0xf1, 0x98, 0x81, 0xa3, 0x94, 0xa3, 0x3f, 0xe5, 0x40,
        0x00, 0x26, 0xf9, 0xef, 0x37, 0x57, 0xdc, 0xf4, 0x14, 0x52, 0x8e, 0x24, 0xf2, 0x80, 0xf7, 0x09,
        0x94, 0x29, 0x04, 0xc0, 0x93, 0xdc, 0xc4, 0xfb, 0x67, 0xa3, 0x93, 0x48, 0xfe, 0x3e, 0x50, 0x3e,
        0x40, 0xbf, 0xa5, 0xfb, 0x73, 0xf7, 0x72, 0xad, 0x1e, 0x79, 0x95, 0x3d, 0x52, 0x06, 0x6c, 0xc1,
        0xfa, 0x63, 0x92, 0xcd, 0xfc, 0xc8, 0x61, 0x9f, 0x8d, 0xc1, 0xc3, 0xce, 0x5d, 0x88, 0xcd, 0xf0,
        0xce, 0x82, 0xb4, 0x2d, 0x17, 0x01, 0x03, 0x74, 0xb1, 0x4c, 0x7c, 0xeb, 0x53, 0xd6, 0x2a, 0x4c,
        0x99, 0xb1, 0x17, 0x89, 0xb6, 0x50, 0x37, 0x51, 0xb4, 0xea, 0x54, 0x4f, 0x34, 0x5c, 0xc2, 0xa6,
        0xe7, 0x15, 0x6c, 0x35, 0xa4, 0x18, 0x76, 0x44, 0x52, 0xda, 0x66, 0x0c, 0x18, 0x49, 0x2f, 0x7e,
        0x54, 0x59, 0xfa, 0x9c, 0xbb, 0xc3, 0xf4, 0x57, 0x59, 0x53, 0x90, 0xca, 0x73, 0x62, 0xc6, 0xbd,
        0xd4, 0x37, 0x0f, 0x6b, 0xb8, 0x6f, 0xbf, 0x24, 0xd4, 0xef, 0xde, 0x98, 0x9c, 0x05, 0x30, 0xc5,
        0xa6, 0xb9, 0x39, 0xe8, 0x5b, 0xc5, 0x30, 0xb8, 0x18, 0xb0, 0x86, 0x33, 0x1f, 0x36, 0x64, 0xe7,
        0x18, 0xd5, 0x05, 0xf5, 0x7d, 0x23, 0xe7, 0xdd, 0x96, 0x1a, 0x82, 0xf4, 0xfd, 0x90, 0x86, 0x69,
        0xb5, 0x4b, 0x7b, 0xec, 0x4d, 0x1a, 0x78, 0xad, 0x6f, 0x55, 0x69, 0xc7, 0x8d, 0x69, 0x00, 0x8b,
        0x56, 0x8e, 0x4c, 0xef, 0x1d, 0xe3, 0xa8, 0xbd, 0xa8, 0x93, 0xfc, 0xc8, 0xc9, 0x1f, 0x4c, 0x0c,
        0x64, 0xc4, 0x42, 0x15, 0xdc, 0xfd, 0x57, 0x44, 0xe6, 0xf8, 0x83, 0xc5, 0xb5, 0x97, 0x2a, 0xd1,
        0x0d, 0x30, 0xb4, 0xef, 0xac, 0x7f, 0xc8, 0xcf, 0x90, 0x83, 0x89, 0xb3, 0x2c, 0xc7, 0xb0, 0x5b,
    };

    static const uint8_t g_dData4096[] = {
        0x80, 0x03, 0xc2, 0x48, 0x91, 0x01, 0x8b, 0xcc, 0xf8, 0x58, 0x58, 0xd8, 0x12, 0x66, 0xd3, 0x98,
        0xdb, 0xd6, 0xce, 0x06, 0xa5, 0x06, 0x46, 0x64, 0x85, 0x35, 0x49, 0x3b, 0x4f, 0x9a, 0xdc, 0x11,
        0x23, 0x89, 0x7f, 0x9c, 0xd1, 0xce, 0x15, 0xad, 0x1c, 0x9d, 0x4a, 0x90, 0x97, 0x71, 0x8f, 0xc0,
        0xd5, 0x49, 0x62, 0x93, 0x6e, 0x85, 0xfa, 0x27, 0x07, 0x61, 0x3f, 0x28, 0x71, 0xbf, 0x7d, 0x80,
        0xb3, 0x04, 0xdf, 0xc0, 0x21, 0xaf, 0x52, 0x9f, 0x59, 0xff, 0x88, 0xe8, 0x89, 0xd9, 0x2c, 0x33,
        0x22, 0x80, 0xe8, 0x63, 0xca, 0x8d, 0xaf, 0x7f, 0xa8, 0xc4, 0x21, 0x6e, 0xc8, 0xe3, 0x7c, 0xeb,
        0xb6, 0xde, 0x96, 0x81, 0xcc, 0x91, 0xf7, 0x4e, 0x4a, 0xe2, 0x0b, 0x38, 0x69, 0x69, 0x70, 0x8d,
        0xc3, 0xb2, 0x19, 0xa6, 0x94, 0xc3, 0xfa, 0xb6, 0x7e, 0xc2, 0xa5, 0x2e, 0x50, 0x4e, 0x4a, 0xf0,
        0x15, 0x89, 0x01, 0x1e, 0xc4, 0x47, 0x3c, 0xdf, 0x02, 0x30, 0xb9, 0x30, 0xd2, 0xfc, 0xb3, 0xda,
        0xb6, 0xd8, 0x58, 0xf6, 0x83, 0xb9, 0x2e, 0xb2, 0xb1, 0x92, 0x44, 0x34, 0xa0, 0xa0, 0xae, 0xde,
        0x08, 0xf0, 0xb4, 0x65, 0x09, 0x92, 0x1d, 0x10, 0x10, 0x96, 0x35, 0xba, 0xd6, 0xdc, 0xb7, 0x55,
        0xc6, 0xa5, 0x5d, 0xb8, 0x62, 0x5e, 0x3c, 0x4d, 0xca, 0x75, 0x4b, 0xb9, 0xff, 0x10, 0x4e, 0x21,
        0xd8, 0x50, 0x9c, 0xb6, 0x2f, 0x97, 0x60, 0x65, 0x46, 0x00, 0xf8, 0x4a, 0x33, 0xe9, 0xf0, 0x75,
        0x7f, 0x8f, 0x67, 0x7d, 0xd2, 0x5f, 0x01, 0x6e, 0xbb, 0x60, 0x43, 0x0a, 0x03, 0xde, 0xaa, 0x69,
        0x4a, 0xab, 0x00, 0x0a, 0x48, 0xd3, 0xb2, 0xb8, 0x00, 0x84, 0xc8, 0x06, 0x89, 0x4a, 0xa2, 0x25,
        0xec, 0x3c, 0x12, 0x2c, 0xb1, 0x52, 0x0d, 0xe5, 0x90, 0xa5, 0x94, 0x78, 0x86, 0x17, 0x51, 0xb6,
        0xdc, 0x1a, 0xd4, 0xc2, 0xf3, 0x4e, 0xa9, 0xf8, 0x90, 0x86, 0xd0, 0x34, 0xa6, 0x80, 0x8a, 0x40,
        0x13, 0xd2, 0xa1, 0x68, 0x27, 0xca, 0xfe, 0x66, 0xc5, 0x2c, 0x7f, 0x79, 0x60, 0x8d, 0x27, 0xac,
        0xbb, 0xa8, 0x61, 0xe0, 0xa0, 0x10, 0x64, 0x97, 0xb0, 0x0f, 0xa7, 0xef, 0xe8, 0x88, 0x09, 0xac,
        0x46, 0x80, 0xbe, 0x90, 0xa5, 0xe6, 0xc9, 0xba, 0x4b, 0x10, 0x91, 0x24, 0xa7, 0x02, 0x00, 0x5c,
        0x49, 0x16, 0x1d, 0xa1, 0x1f, 0xbf, 0x71, 0x36, 0x9e, 0xfd, 0x4b, 0xc7, 0x7e, 0x98, 0xee, 0x7b,
        0x97, 0xf3, 0xa5, 0x3f, 0x52, 0xc4, 0x87, 0x20, 0xdc, 0x14, 0xfa, 0xae, 0x34, 0xd2, 0xf5, 0x7a,
        0x26, 0x3a, 0x01, 0xba, 0x15, 0x52, 0xd0, 0x3d, 0xe4, 0x42, 0x2b, 0x5b, 0x96, 0x54, 0x41, 0xd6,
        0x29, 0x3f, 0x06, 0xf0, 0xb2, 0x6a, 0x59, 0x9a, 0x99, 0xa3, 0xa8, 0x40, 0x67, 0xba, 0x41, 0x42,
        0xce, 0x02, 0x1d, 0x48, 0xda, 0x91, 0x85, 0x5c, 0x87, 0x5a, 0xe7, 0xa4, 0x35, 0x26, 0xaa, 0x4f,
        0x04, 0xd4, 0x30, 0x18, 0x8c, 0xb1, 0x2f, 0x89, 0x5f, 0x3e, 0x49, 0x0c, 0x4d, 0x21, 0xe0, 0xd9,
        0x10, 0xb2, 0x5e, 0x66, 0x8f, 0x33, 0x96, 0xaf, 0x61, 0xff, 0xc6, 0x95, 0xd3, 0xb1, 0x8c, 0x71,
        0x1f, 0xc4, 0x95, 0x8f, 0xb8, 0x32, 0x0a, 0x71, 0x3f, 0xc5, 0xe8, 0xca, 0x88, 0xc7, 0xd3, 0xa1,
        0x25, 0xcb, 0xf8, 0x04, 0x04, 0x53, 0x28, 0xf2, 0xf0, 0xb5, 0xf5, 0x8f, 0xf8, 0xdc, 0x57, 0xb4,
        0xc8, 0x13, 0x80, 0x55, 0x33, 0xc1, 0xdd, 0x88, 0xcc, 0x37, 0xc9, 0xcd, 0xa3, 0x2b, 0x2b, 0x2c,
        0xe2, 0xdb, 0xd7, 0xca, 0x34, 0xe0, 0x0c, 0xb5, 0x3c, 0xa3, 0x12, 0xa6, 0x94, 0x68, 0xaa, 0x66,
        0xc7, 0x92, 0xe2, 0xde, 0x06, 0xbb, 0x48, 0xee, 0x27, 0x03, 0x7c, 0x0e, 0xf5, 0x51, 0xca, 0xd9,
    };

    static const uint8_t g_eData[] = {0x01, 0x00, 0x01};

    static struct HksParam g_importRsa4096KeyParams[] = {
        {.tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_RSA},
        {.tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT},
        {.tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_RSA_KEY_SIZE_4096},
        {.tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_OAEP},
        {.tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA256},
        {.tag = HKS_TAG_IMPORT_KEY_TYPE, .uint32Param = HKS_KEY_TYPE_KEY_PAIR},
        {.tag = HKS_TAG_UNWRAP_ALGORITHM_SUITE, .uint32Param = HKS_UNWRAP_SUITE_ECDH_AES_256_GCM_NOPADDING},
    };

    static struct HksBlob g_importedKeyAliasRsa4096 = {
        .size = strlen("test_import_key_ecdh_Rsa4096"),
        .data = (uint8_t *) "test_import_key_ecdh_Rsa4096"
    };

    static struct HksBlob g_wrappingKeyAliasRsa4096 = {
        .size = strlen("test_wrappingKey_ecdh_Rsa4096"),
        .data = (uint8_t *) "test_wrappingKey_ecdh_Rsa4096"
    };

    static struct HksBlob g_callerKeyAliasRsa4096 = {
        .size = strlen("test_caller_key_ecdh_Rsa4096"),
        .data = (uint8_t *) "test_caller_key_ecdh_Rsa4096"
    };

    static struct HksBlob g_callerKekAliasRsa4096 = {
        .size = strlen("test_caller_kek_ecdh_Rsa4096"),
        .data = (uint8_t *) "test_caller_kek_ecdh_Rsa4096"
    };

    static struct HksBlob g_callerAgreeKeyAliasRsa4096 = {
        .size = strlen("test_caller_agree_key_ecdh_Rsa4096"),
        .data = (uint8_t *) "test_caller_agree_key_ecdh_Rsa4096"
    };

    struct TestImportKeyData {
        struct HksBlob x509PublicKey;
        struct HksBlob publicOrXData;
        struct HksBlob privateOrYData;
        struct HksBlob zData;
    };
    /* ------------------ End of RSA-4096 import key material and params define -------------------- */

    /* ------------------ Start of hmac256 pair import key material and params define -------------------- */
    static struct HksBlob g_importHmac256Key = {
        .size = strlen("This is hmac256 key to be import"),
        .data = (uint8_t *) "This is hmac256 key to be import"
    };

    static struct HksBlob g_callerHmac256Kek = {
        .size = strlen("This is hmac256 pair kek encrypt"),
        .data = (uint8_t *) "This is hmac256 pair kek encrypt"
    };

    static struct HksParam g_importHmac256KeyParams[] = {
        {.tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_HMAC},
        {.tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_MAC},
        {.tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_256},
        {.tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA384},
        {.tag = HKS_TAG_UNWRAP_ALGORITHM_SUITE, .uint32Param = HKS_UNWRAP_SUITE_ECDH_AES_256_GCM_NOPADDING},
    };

    static struct HksParam g_importHmac256KeyAtherParams[] = {
        {.tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_HMAC},
        {.tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_MAC},
        {.tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_256},
        {.tag = HKS_TAG_UNWRAP_ALGORITHM_SUITE, .uint32Param = HKS_UNWRAP_SUITE_ECDH_AES_256_GCM_NOPADDING},
    };

    static struct HksBlob g_importedKeyAliasHmac256 = {
        .size = strlen("test_import_key_ecdh_hmac256"),
        .data = (uint8_t *) "test_import_key_ecdh_hmac256"
    };

    static struct HksBlob g_wrappingKeyAliasHmac256 = {
        .size = strlen("test_wrappingKey_ecdh_hmac256"),
        .data = (uint8_t *) "test_wrappingKey_ecdh_hmac256"
    };

    static struct HksBlob g_callerKeyAliasHmac256 = {
        .size = strlen("test_caller_key_ecdh_hmac256"),
        .data = (uint8_t *) "test_caller_key_ecdh_hmac256"
    };

    static struct HksBlob g_callerKekAliasHmac256 = {
        .size = strlen("test_caller_kek_ecdh_hmac256"),
        .data = (uint8_t *) "test_caller_kek_ecdh_hmac256"
    };

    static struct HksBlob g_callerAgreeKeyAliasHmac256 = {
        .size = strlen("test_caller_agree_key_ecdh_hmac256"),
        .data = (uint8_t *) "test_caller_agree_key_ecdh_hmac256"
    };
    /* ------------------ End of hmac256 pair import key material and params define -------------------- */

    static int32_t ConstructKey(const struct HksBlob *nDataBlob, const struct HksBlob *dDataBlob,
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

        struct HksParamSet *genEcdhKeyParamSet = nullptr;
        ret = InitParamSet(&genEcdhKeyParamSet, g_genWrappingKeyParams,
                           sizeof(g_genWrappingKeyParams) / sizeof(HksParam));
        EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet(gen huks ecdh) failed.";
        importWrappedKeyTestParams->genWrappingKeyParamSet = genEcdhKeyParamSet;
        importWrappedKeyTestParams->publicKeySize = g_ecdhPubKeySize;

        struct HksParamSet *genCallerKeyParamSet = nullptr;
        ret = InitParamSet(&genCallerKeyParamSet, g_genCallerEcdhParams,
                           sizeof(g_genCallerEcdhParams) / sizeof(HksParam));
        EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet(gen caller Ecdh) failed.";
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

        HksFreeParamSet(&genEcdhKeyParamSet);
        HksFreeParamSet(&genCallerKeyParamSet);
        HksFreeParamSet(&callerImportParamsKek);
        HksFreeParamSet(&importPlainKeyParams);
    }

    /**
     * @tc.name: HksImportWrappedEcdhSuiteTest.HksImportWrappedKeyTestEcdhSuite001
     * @tc.desc: Test import wrapped aes256-gcm-no_padding key including generate&export ecdh p256 key, generate kek,
     * agree, encrypt, of which generate kek, agree, encrypt should done by caller self.
     * @tc.type: FUNC
     */
    HWTEST_F(HksImportWrappedEcdhSuiteTest, HksImportWrappedKeyTestEcdhSuite001, TestSize.Level0)
    {
        struct HksImportWrappedKeyTestParams importWrappedKeyTestParams001 = {0};

        importWrappedKeyTestParams001.wrappingKeyAlias = &g_wrappingKeyAliasAes256;
        importWrappedKeyTestParams001.keyMaterialLen = g_importedAes192PlainKey.size;
        importWrappedKeyTestParams001.callerKeyAlias = &g_callerKeyAliasAes256;
        importWrappedKeyTestParams001.callerKekAlias = &g_callerKekAliasAes256;
        importWrappedKeyTestParams001.callerKek = &g_callerAes256Kek;
        importWrappedKeyTestParams001.callerAgreeKeyAlias = &g_callerAgreeKeyAliasAes256;
        importWrappedKeyTestParams001.importedKeyAlias = &g_importedKeyAliasAes256;
        importWrappedKeyTestParams001.importedPlainKey = &g_importedAes192PlainKey;
        InitCommonTestParamsAndDoImport(&importWrappedKeyTestParams001, g_importWrappedAes256Params,
                                        sizeof(g_importWrappedAes256Params) / sizeof(struct HksParam));
        HksClearKeysForWrappedKeyTest(&importWrappedKeyTestParams001);
    }

    /**
     * @tc.name: HksImportWrappedEcdhSuiteTest.HksImportWrappedKeyTestEcdhSuite002
     * @tc.desc: Test import wrapped rsa key pair including generate&export ecdh p256 key, generate kek, agree, encrypt,
     *           of which generate kek, agree, encrypt should done by caller self.
     * @tc.type: FUNC
     */
    HWTEST_F(HksImportWrappedEcdhSuiteTest, HksImportWrappedKeyTestEcdhSuite002, TestSize.Level0)
    {
        struct HksBlob nDataBlob = {sizeof(g_nData4096), (uint8_t *) g_nData4096};
        struct HksBlob dDataBlob = {sizeof(g_dData4096), (uint8_t *) g_dData4096};
        struct HksBlob plainKey = {0, nullptr};

        int32_t ret = ConstructKey(&nDataBlob, &dDataBlob, HKS_RSA_KEY_SIZE_4096, &plainKey, false);
        EXPECT_EQ(ret, HKS_SUCCESS) << "construct rsa 2048 failed.";

        struct HksImportWrappedKeyTestParams importWrappedKeyTestParams002 = {0};

        importWrappedKeyTestParams002.wrappingKeyAlias = &g_wrappingKeyAliasRsa4096;
        importWrappedKeyTestParams002.keyMaterialLen = plainKey.size;
        importWrappedKeyTestParams002.callerKeyAlias = &g_callerKeyAliasRsa4096;
        importWrappedKeyTestParams002.callerKekAlias = &g_callerKekAliasRsa4096;
        importWrappedKeyTestParams002.callerKek = &g_callerRsa4096Kek;
        importWrappedKeyTestParams002.callerAgreeKeyAlias = &g_callerAgreeKeyAliasRsa4096;
        importWrappedKeyTestParams002.importedKeyAlias = &g_importedKeyAliasRsa4096;
        importWrappedKeyTestParams002.importedPlainKey = &plainKey;
        InitCommonTestParamsAndDoImport(&importWrappedKeyTestParams002, g_importRsa4096KeyParams,
                                        sizeof(g_importRsa4096KeyParams) / sizeof(struct HksParam));
        HksClearKeysForWrappedKeyTest(&importWrappedKeyTestParams002);
    }


    /**
     * @tc.name: HksImportWrappedEcdhSuiteTest.HksImportWrappedKeyTestEcdhSuite003
     * @tc.desc: Test import wrapped hmac256 key pair including generate&export ecdh p256 key, generate kek, agree,
     *           encrypt, of which generate kek, agree, encrypt should done by caller self.
     * @tc.type: FUNC
     */
    HWTEST_F(HksImportWrappedEcdhSuiteTest, HksImportWrappedKeyTestEcdhSuite003, TestSize.Level0)
    {
        struct HksImportWrappedKeyTestParams importWrappedKeyTestParams003 = {0};

        importWrappedKeyTestParams003.wrappingKeyAlias = &g_wrappingKeyAliasHmac256;
        importWrappedKeyTestParams003.keyMaterialLen = g_importHmac256Key.size;
        importWrappedKeyTestParams003.callerKeyAlias = &g_callerKeyAliasHmac256;
        importWrappedKeyTestParams003.callerKekAlias = &g_callerKekAliasHmac256;
        importWrappedKeyTestParams003.callerKek = &g_callerHmac256Kek;
        importWrappedKeyTestParams003.callerAgreeKeyAlias = &g_callerAgreeKeyAliasHmac256;
        importWrappedKeyTestParams003.importedKeyAlias = &g_importedKeyAliasHmac256;
        importWrappedKeyTestParams003.importedPlainKey = &g_importHmac256Key;
        InitCommonTestParamsAndDoImport(&importWrappedKeyTestParams003, g_importHmac256KeyParams,
                                        sizeof(g_importHmac256KeyParams) / sizeof(struct HksParam));
        HksClearKeysForWrappedKeyTest(&importWrappedKeyTestParams003);
    }

    /**
     * @tc.name: HksImportWrappedEcdhSuiteTest.HksImportWrappedKeyTestEcdhSuite004
     * @tc.desc: Test import wrapped hmac256 key pair including generate&export ecdh p256 key, generate kek, agree,
     *           encrypt, of which generate kek, agree, encrypt should done by caller self. When importing the key,
     *           only the necessary parameters are passed in.
     * @tc.type: FUNC
     * @tc.require:issueI611S5
     */
    HWTEST_F(HksImportWrappedEcdhSuiteTest, HksImportWrappedKeyTestEcdhSuite004, TestSize.Level0)
    {
        HKS_LOG_E("Enter HksImportWrappedKeyTestEcdhSuite004");
        struct HksImportWrappedKeyTestParams importWrappedKeyTestParams004 = {0};

        importWrappedKeyTestParams004.wrappingKeyAlias = &g_wrappingKeyAliasHmac256;
        importWrappedKeyTestParams004.keyMaterialLen = g_importHmac256Key.size;
        importWrappedKeyTestParams004.callerKeyAlias = &g_callerKeyAliasHmac256;
        importWrappedKeyTestParams004.callerKekAlias = &g_callerKekAliasHmac256;
        importWrappedKeyTestParams004.callerKek = &g_callerHmac256Kek;
        importWrappedKeyTestParams004.callerAgreeKeyAlias = &g_callerAgreeKeyAliasHmac256;
        importWrappedKeyTestParams004.importedKeyAlias = &g_importedKeyAliasHmac256;
        importWrappedKeyTestParams004.importedPlainKey = &g_importHmac256Key;
        InitCommonTestParamsAndDoImport(&importWrappedKeyTestParams004, g_importHmac256KeyAtherParams,
                                        sizeof(g_importHmac256KeyAtherParams) / sizeof(struct HksParam));
        HksClearKeysForWrappedKeyTest(&importWrappedKeyTestParams004);
    }
}
