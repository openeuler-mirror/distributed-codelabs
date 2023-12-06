/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <cstring>
#include <gtest/gtest.h>
#include <securec.h>

#include "lnn_huks_utils.h"
#include "softbus_adapter_mem.h"
#include "softbus_errcode.h"

namespace OHOS {
using namespace testing::ext;

static constexpr char KEY_ALIAS[] = "dsoftbus_test_key_alias";
static constexpr char RANDOM_KEY[] = "b0d8bfed90d1e018c84f0a1abd4cbcc7f33481b42476719b401b1d70d3998a7c";

static struct HksBlob g_keyAlias = {0};

class LnnHuksUtilsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void LnnHuksUtilsTest::SetUpTestCase()
{
    g_keyAlias.size = strlen(KEY_ALIAS);
    g_keyAlias.data = (uint8_t *)KEY_ALIAS;

    EXPECT_EQ(LnnInitHuksInterface(), SOFTBUS_OK);
}

void LnnHuksUtilsTest::TearDownTestCase()
{
    LnnDeinitHuksInterface();
}

void LnnHuksUtilsTest::SetUp()
{
}

void LnnHuksUtilsTest::TearDown()
{
    (void)LnnDeleteKeyByHuks(&g_keyAlias);
}

/*
* @tc.name: Generate_Key_Test_001
* @tc.desc: generate key test
* @tc.type: FUNC
* @tc.require: I5RHYE
*/
HWTEST_F(LnnHuksUtilsTest, Generate_Key_Test_01, TestSize.Level0)
{
    struct HksBlob keyAlias = {0};
    keyAlias.size = strlen(KEY_ALIAS);
    keyAlias.data = (uint8_t *)KEY_ALIAS;
    
    EXPECT_EQ(LnnGenerateKeyByHuks(&keyAlias), SOFTBUS_OK);
}

/*
* @tc.name: Generate_Key_Test_002
* @tc.desc: generate key twice test
* @tc.type: FUNC
* @tc.require: I5RHYE
*/
HWTEST_F(LnnHuksUtilsTest, Generate_Key_Test_02, TestSize.Level0)
{
    struct HksBlob keyAlias = {0};
    keyAlias.size = strlen(KEY_ALIAS);
    keyAlias.data = (uint8_t *)KEY_ALIAS;
    
    EXPECT_EQ(LnnGenerateKeyByHuks(&keyAlias), SOFTBUS_OK);
    EXPECT_EQ(LnnGenerateKeyByHuks(&keyAlias), SOFTBUS_OK);
}

/*
* @tc.name: Generate_Random_Test_001
* @tc.desc: generate randowm key test
* @tc.type: FUNC
* @tc.require: I5RHYE
*/
HWTEST_F(LnnHuksUtilsTest, Generate_Random_Test_01, TestSize.Level0)
{
    uint8_t randomKey[LNN_HUKS_AES_COMMON_SIZE] = {0};

    EXPECT_EQ(LnnGenerateRandomByHuks(randomKey, LNN_HUKS_AES_COMMON_SIZE), SOFTBUS_OK);
}

/*
* @tc.name: Encrypt_Data_Test_001
* @tc.desc: encrypt data test
* @tc.type: FUNC
* @tc.require: I5RHYE
*/
HWTEST_F(LnnHuksUtilsTest, Encrypt_Data_Test_01, TestSize.Level0)
{
    struct HksBlob inData = {0};
    inData.size = strlen(RANDOM_KEY);
    inData.data = (uint8_t *)RANDOM_KEY;

    struct HksBlob outData = {0};
    outData.data = (uint8_t *)SoftBusCalloc(LNN_HUKS_AES_COMMON_SIZE);
    ASSERT_NE(outData.data, nullptr);

    EXPECT_EQ(LnnGenerateKeyByHuks(&g_keyAlias), SOFTBUS_OK);
    EXPECT_EQ(LnnEncryptDataByHuks(&g_keyAlias, &inData, &outData), SOFTBUS_OK);
    EXPECT_NE(memcmp(inData.data, outData.data, inData.size), 0);
    SoftBusFree(outData.data);
}

/*
* @tc.name: Decrypt_Data_Test_001
* @tc.desc: decrypt data test
* @tc.type: FUNC
* @tc.require: I5RHYE
*/
HWTEST_F(LnnHuksUtilsTest, Decrypt_Data_Test_01, TestSize.Level0)
{
    struct HksBlob plainData = {0};
    plainData.size = strlen(RANDOM_KEY);
    plainData.data = (uint8_t *)RANDOM_KEY;

    struct HksBlob encryptData = {0};
    encryptData.data = (uint8_t *)SoftBusCalloc(LNN_HUKS_AES_COMMON_SIZE);
    ASSERT_NE(encryptData.data, nullptr);

    struct HksBlob decryptData = {0};
    decryptData.data = (uint8_t *)SoftBusCalloc(LNN_HUKS_AES_COMMON_SIZE);
    ASSERT_NE(decryptData.data, nullptr);

    EXPECT_EQ(LnnGenerateKeyByHuks(&g_keyAlias), SOFTBUS_OK);
    EXPECT_EQ(LnnEncryptDataByHuks(&g_keyAlias, &plainData, &encryptData), SOFTBUS_OK);
    EXPECT_NE(memcmp(plainData.data, encryptData.data, plainData.size), 0);

    EXPECT_EQ(LnnDecryptDataByHuks(&g_keyAlias, &encryptData, &decryptData), SOFTBUS_OK);
    EXPECT_EQ(memcmp(decryptData.data, plainData.data, decryptData.size), 0);
    SoftBusFree(encryptData.data);
    SoftBusFree(decryptData.data);
}
} // namespace OHOS
