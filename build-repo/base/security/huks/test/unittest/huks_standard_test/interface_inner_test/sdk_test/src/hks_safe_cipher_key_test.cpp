/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#include "hks_safe_cipher_key_test.h"

#include "hks_api.h"
#include "hks_param.h"
#include "hks_test_api_performance.h"
#include "hks_test_curve25519.h"
#include "hks_test_file_operator.h"
#include "hks_test_log.h"
#include "hks_test_mem.h"

using namespace testing::ext;
namespace {
class HksSafeCipherKeyTest : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};

void HksSafeCipherKeyTest::SetUpTestCase(void)
{
}

void HksSafeCipherKeyTest::TearDownTestCase(void)
{
}

void HksSafeCipherKeyTest::SetUp()
{
}

void HksSafeCipherKeyTest::TearDown()
{
}

const char *g_storePath = "/data/service/el1/public/huks_service/maindata/+0+0+0+0/key";
const char *g_testEd25519 = "test_ed25519";

static int32_t CompareTwoKey(const struct HksBlob *keyAliasOne, const struct HksBlob *keyAliasTwo)
{
    uint32_t sizeOne = HksFileSize(g_storePath, (char *)keyAliasOne->data);
    uint8_t *bufOne = (uint8_t *)HksTestMalloc(sizeOne);
    if (bufOne == nullptr) {
        return HKS_ERROR_MALLOC_FAIL;
    }
    uint32_t sizeRead = HksFileRead(g_storePath, (char *)keyAliasOne->data, 0, bufOne, sizeOne);

    uint32_t sizeTwo = HksFileSize(g_storePath, (char *)keyAliasTwo->data);
    uint8_t *bufTwo = (uint8_t *)HksTestMalloc(sizeTwo);
    if (bufTwo == nullptr) {
        HksTestFree(bufOne);
        return HKS_ERROR_MALLOC_FAIL;
    }
    sizeRead = HksFileRead(g_storePath, (char *)keyAliasTwo->data, 0, bufTwo, sizeOne);
    int32_t ret = memcmp(bufOne, bufTwo, sizeRead);
    HksTestFree(bufOne);
    HksTestFree(bufTwo);
    return ret;
}

/**
 * @tc.name: HksSafeCipherKeyTest.HksSafeCipherKeyTest001
 * @tc.desc: The static function will return true;
 * @tc.type: FUNC
 */
HWTEST_F(HksSafeCipherKeyTest, HksSafeCipherKeyTest001, TestSize.Level0)
{
    struct HksBlob ed25519Alias = { strlen(g_testEd25519), (uint8_t *)g_testEd25519 };
    int32_t ret = TestGenerateEd25519Key(ed25519Alias);
    uint8_t pubKey[32] = {0};
    uint32_t pubKeyLen = 32;
    struct HksBlob pubKeyInfo = { pubKeyLen, pubKey };
    ret = HksExportPublicKey(&ed25519Alias, NULL, &pubKeyInfo);
    HKS_TEST_ASSERT(ret == 0);
    ret = HksDeleteKey(&ed25519Alias, NULL);
    HKS_TEST_ASSERT(ret == 0);

    struct HksBlob newAliasOne = { strlen("test_ed25519_1"), (uint8_t *)"test_ed25519_1" };
    ret = TestImportEd25519(newAliasOne, &pubKeyInfo);
    HKS_TEST_ASSERT(ret == 0);

    struct HksBlob newAliasTwo = { strlen("test_ed25519_2"), (uint8_t *)"test_ed25519_2" };
    ret = TestImportEd25519(newAliasTwo, &pubKeyInfo);
    HKS_TEST_ASSERT(ret == 0);

    ret = CompareTwoKey(&newAliasOne, &newAliasTwo);
    HKS_TEST_ASSERT(ret != 0);
    ASSERT_TRUE(ret != 0);

    ret = HksDeleteKey(&newAliasOne, NULL);
    HKS_TEST_ASSERT(ret == 0);
    ASSERT_TRUE(ret == 0);
    ret = HksDeleteKey(&newAliasTwo, NULL);
    HKS_TEST_ASSERT(ret == 0);
    ASSERT_TRUE(ret == 0);
}
}