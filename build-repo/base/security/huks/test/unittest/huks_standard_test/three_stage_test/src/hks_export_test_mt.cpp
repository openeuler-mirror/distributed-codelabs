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
namespace Unittest::ExportKeyMt {
struct TestCaseParams {
    std::vector<HksParam> params;
};

const char *keyAliasString = "This is for export normal key";
struct HksBlob authId = { strlen(keyAliasString), (uint8_t *)keyAliasString };
class HksExportTestMt : public testing::Test {
public:
    int32_t RunTestCase(const TestCaseParams &testCaseParams)
    {
        /* Generate Key */
        struct HksParamSet *paramInSet = nullptr;
        int32_t ret = InitParamSet(&paramInSet, testCaseParams.params.data(), testCaseParams.params.size());
        if (paramInSet == nullptr) {
            return HKS_ERROR_MALLOC_FAIL;
        }
        ret = HksGenerateKey(&authId, paramInSet, nullptr);
        EXPECT_EQ(ret, HKS_SUCCESS) << "GenerateKey failed.";
        HksFreeParamSet(&paramInSet);

        return ret;
    }
};

const TestCaseParams g_huksExportKeyMt100Params = {
    .params = {
        { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_ECC },
        { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_ECC_KEY_SIZE_224 },
        { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_SIGN | HKS_KEY_PURPOSE_VERIFY },
        { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA224 },
    },
};

/**
 * @tc.number    : HksExportTestMt00100
 * @tc.name      : HksExportTestMt00100
 * @tc.desc      : Test huks normal exportkey
 */
HWTEST_F(HksExportTestMt, HksExportTestMt00100, TestSize.Level0)
{
    EXPECT_EQ(RunTestCase(g_huksExportKeyMt100Params), HKS_SUCCESS);
    struct HksBlob testAlias = { strlen(keyAliasString), (uint8_t *)keyAliasString };
    HksBlob publicKey = { .size = HKS_ECC_KEY_SIZE_224, .data = (uint8_t *)HksMalloc(HKS_ECC_KEY_SIZE_224) };
    if (publicKey.data != nullptr) {
        EXPECT_EQ(HksExportPublicKey(&testAlias, nullptr, &publicKey), HKS_SUCCESS);
    }
    EXPECT_EQ(HksDeleteKey(&authId, nullptr), HKS_SUCCESS);
    HksFree(publicKey.data);
}
/**
 * @tc.number    : HksExportTestMt00200
 * @tc.name      : HksExportTestMt00200
 * @tc.desc      : Test huks error exportkey (ECC224/keyalias = NULL)
 */
HWTEST_F(HksExportTestMt, HksExportTestMt0020, TestSize.Level0)
{
    EXPECT_EQ(RunTestCase(g_huksExportKeyMt100Params), HKS_SUCCESS);
    HksBlob publicKey = { .size = HKS_ECC_KEY_SIZE_224, .data = (uint8_t *)HksMalloc(HKS_ECC_KEY_SIZE_224) };
    if (publicKey.data != nullptr) {
        EXPECT_EQ(HksExportPublicKey(nullptr, nullptr, &publicKey), HKS_ERROR_NULL_POINTER);
    }
    EXPECT_EQ(HksDeleteKey(&authId, nullptr), HKS_SUCCESS);
    HksFree(publicKey.data);
}
/**
 * @tc.number    : HksExportTestMt00300
 * @tc.name      : HksExportTestMt00300
 * @tc.desc      : Test huks error exportkey (ECC224/key = NULL)
 */
HWTEST_F(HksExportTestMt, HksExportTestMt0030, TestSize.Level0)
{
    EXPECT_EQ(RunTestCase(g_huksExportKeyMt100Params), HKS_SUCCESS);
    struct HksBlob testAlias = { strlen(keyAliasString), (uint8_t *)keyAliasString };
    EXPECT_EQ(HksExportPublicKey(&testAlias, nullptr, nullptr), HKS_ERROR_NULL_POINTER);
    EXPECT_EQ(HksDeleteKey(&authId, nullptr), HKS_SUCCESS);
}
/**
 * @tc.number    : HksExportTestMt00400
 * @tc.name      : HksExportTestMt00400
 * @tc.desc      : Test huks error exportkey (ECC224/keyalias.size = 0)
 */
HWTEST_F(HksExportTestMt, HksExportTestMt0040, TestSize.Level0)
{
    EXPECT_EQ(RunTestCase(g_huksExportKeyMt100Params), HKS_SUCCESS);
    struct HksBlob testAlias = { 0, (uint8_t *)keyAliasString };
    HksBlob publicKey = { .size = HKS_ECC_KEY_SIZE_224, .data = (uint8_t *)HksMalloc(HKS_ECC_KEY_SIZE_224) };
    if (publicKey.data != nullptr) {
        EXPECT_EQ(HksExportPublicKey(&testAlias, nullptr, &publicKey), HKS_ERROR_INVALID_ARGUMENT);
    }
    EXPECT_EQ(HksDeleteKey(&authId, nullptr), HKS_SUCCESS);
    HksFree(publicKey.data);
}
/**
 * @tc.number    : HksExportTestMt00500
 * @tc.name      : HksExportTestMt00500
 * @tc.desc      : Test huks error exportkey (ECC224/keyalias.size = 4096)
 */
HWTEST_F(HksExportTestMt, HksExportTestMt0050, TestSize.Level0)
{
    const int SET_SIZE_4096 = 4096;
    EXPECT_EQ(RunTestCase(g_huksExportKeyMt100Params), HKS_SUCCESS);
    uint8_t testAliasString[SET_SIZE_4096];
    struct HksBlob testAlias = { SET_SIZE_4096, testAliasString };
    HksBlob publicKey = { .size = HKS_ECC_KEY_SIZE_224, .data = (uint8_t *)HksMalloc(HKS_ECC_KEY_SIZE_224) };
    if (publicKey.data != nullptr) {
        EXPECT_EQ(HksExportPublicKey(&testAlias, nullptr, &publicKey), HKS_ERROR_INVALID_ARGUMENT);
    }
    EXPECT_EQ(HksDeleteKey(&authId, nullptr), HKS_SUCCESS);
    HksFree(publicKey.data);
}
/**
 * @tc.number    : HksExportTestMt00600
 * @tc.name      : HksExportTestMt00600
 * @tc.desc      : Test huks error exportkey (ECC224/keyalias.data = nullptr)
 */
HWTEST_F(HksExportTestMt, HksExportTestMt0060, TestSize.Level0)
{
    EXPECT_EQ(RunTestCase(g_huksExportKeyMt100Params), HKS_SUCCESS);
    struct HksBlob testAlias = { strlen(keyAliasString), nullptr };
    HksBlob publicKey = { .size = HKS_ECC_KEY_SIZE_224, .data = (uint8_t *)HksMalloc(HKS_ECC_KEY_SIZE_224) };
    if (publicKey.data != nullptr) {
        EXPECT_EQ(HksExportPublicKey(&testAlias, nullptr, &publicKey), HKS_ERROR_INVALID_ARGUMENT);
    }
    EXPECT_EQ(HksDeleteKey(&authId, nullptr), HKS_SUCCESS);
    HksFree(publicKey.data);
}
/**
 * @tc.number    : HksExportTestMt00700
 * @tc.name      : HksExportTestMt00700
 * @tc.desc      : Test huks error exportkey (ECC224/key.size = 0)
 */
HWTEST_F(HksExportTestMt, HksExportTestMt0070, TestSize.Level0)
{
    EXPECT_EQ(RunTestCase(g_huksExportKeyMt100Params), HKS_SUCCESS);
    HksBlob publicKey = { 0, .data = (uint8_t *)HksMalloc(HKS_ECC_KEY_SIZE_224) };
    if (publicKey.data != nullptr) {
        EXPECT_EQ(HksExportPublicKey(&authId, nullptr, &publicKey), HKS_ERROR_INSUFFICIENT_DATA);
    }
    EXPECT_EQ(HksDeleteKey(&authId, nullptr), HKS_SUCCESS);
    HksFree(publicKey.data);
}
/**
 * @tc.number    : HksExportTestMt00800
 * @tc.name      : HksExportTestMt00800
 * @tc.desc      : Test huks error exportkey (ECC224/key.data = nullptr)
 */
HWTEST_F(HksExportTestMt, HksExportTestMt0080, TestSize.Level0)
{
    EXPECT_EQ(RunTestCase(g_huksExportKeyMt100Params), HKS_SUCCESS);
    HksBlob publicKey = { .size = HKS_ECC_KEY_SIZE_224, nullptr };
    if (publicKey.data != nullptr) {
        EXPECT_EQ(HksExportPublicKey(&authId, nullptr, &publicKey), HKS_ERROR_BAD_STATE);
    }
    EXPECT_EQ(HksDeleteKey(&authId, nullptr), HKS_SUCCESS);
    HksFree(publicKey.data);
}
/**
 * @tc.number    : HksExportTestMt00900
 * @tc.name      : HksExportTestMt00900
 * @tc.desc      : Test huks error exportkey (generatekey no exist)
 */
HWTEST_F(HksExportTestMt, HksExportTestMt0090, TestSize.Level0)
{
    struct HksBlob testAlias = { strlen(keyAliasString), (uint8_t *)keyAliasString };
    HksBlob publicKey = { .size = HKS_ECC_KEY_SIZE_224, .data = (uint8_t *)HksMalloc(HKS_ECC_KEY_SIZE_224) };
    if (publicKey.data != nullptr) {
        EXPECT_EQ(HksExportPublicKey(&testAlias, nullptr, &publicKey), HKS_ERROR_NOT_EXIST);
    }
    EXPECT_EQ(HksDeleteKey(&authId, nullptr), HKS_ERROR_NOT_EXIST);
    HksFree(publicKey.data);
}
}
