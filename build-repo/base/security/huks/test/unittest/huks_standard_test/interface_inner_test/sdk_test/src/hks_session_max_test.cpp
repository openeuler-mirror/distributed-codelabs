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

#include <gtest/gtest.h>

#include "hks_api.h"
#include "hks_param.h"
#include "hks_test_log.h"
#include "hks_type.h"

using namespace testing::ext;
namespace {
static const uint32_t MAX_SESSION_NUM_MORE_1 = 16;
static const uint32_t HMAC_OUTPUT_SIZE = 32;
class HksSessionMaxTest : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};

void HksSessionMaxTest::SetUpTestCase(void)
{
}

void HksSessionMaxTest::TearDownTestCase(void)
{
}

void HksSessionMaxTest::SetUp()
{
}

void HksSessionMaxTest::TearDown()
{
}

static void GenerateBaseKey(const struct HksBlob *alias)
{
    HKS_TEST_LOG_I("Generate Base Key");
    struct HksParamSet *paramSet = NULL;
    ASSERT_TRUE(HksInitParamSet(&paramSet) == 0);

    struct HksParam tmpParams[] = {
        { .tag = HKS_TAG_ALGORITHM,
            .uint32Param = HKS_ALG_HMAC },
        { .tag = HKS_TAG_KEY_SIZE,
            .uint32Param = HKS_AES_KEY_SIZE_256 },
        { .tag = HKS_TAG_PURPOSE,
            .uint32Param = HKS_KEY_PURPOSE_MAC },
        { .tag = HKS_TAG_DIGEST,
            .uint32Param = HKS_DIGEST_SHA256 },
    };

    ASSERT_TRUE(HksAddParams(paramSet, tmpParams, sizeof(tmpParams) / sizeof(tmpParams[0])) == 0);
    ASSERT_TRUE(HksBuildParamSet(&paramSet) == 0);
    ASSERT_TRUE(HksGenerateKey(alias, paramSet, NULL) == 0);

    HksFreeParamSet(&paramSet);
}

static void ConstructInitParamSet(struct HksParamSet **outParamSet)
{
    struct HksParamSet *paramSet = NULL;
    ASSERT_TRUE(HksInitParamSet(&paramSet) == 0);

    struct HksParam tmpParams[] = {
        { .tag = HKS_TAG_ALGORITHM,
            .uint32Param = HKS_ALG_HMAC },
        { .tag = HKS_TAG_PURPOSE,
            .uint32Param = HKS_KEY_PURPOSE_MAC },
        { .tag = HKS_TAG_DIGEST,
            .uint32Param = HKS_DIGEST_SHA256 },
    };

    ASSERT_TRUE(HksAddParams(paramSet, tmpParams, sizeof(tmpParams) / sizeof(tmpParams[0])) == 0);
    ASSERT_TRUE(HksBuildParamSet(&paramSet) == 0);

    *outParamSet = paramSet;
}

static void SessionMaxTest(const struct HksBlob *alias)
{
    uint64_t handle[MAX_SESSION_NUM_MORE_1];
    for (uint32_t i = 0; i < MAX_SESSION_NUM_MORE_1; ++i) {
        struct HksBlob handleBlob = { sizeof(uint64_t), (uint8_t *)&handle[i] };
        struct HksParamSet *paramSet = NULL;
        ConstructInitParamSet(&paramSet);
        uint8_t tokenData[HMAC_OUTPUT_SIZE] = {0};
        struct HksBlob token = { sizeof(tokenData), tokenData };
        EXPECT_EQ(HksInit(alias, paramSet, &handleBlob, &token), HKS_SUCCESS);
        HksFreeParamSet(&paramSet);
    }

    for (uint32_t i = 0; i < MAX_SESSION_NUM_MORE_1; ++i) {
        struct HksParamSet *paramSet = NULL;
        ConstructInitParamSet(&paramSet);

        uint8_t tmpInput[] = "testForSessionMaxTest";
        uint8_t tmpOutput[HMAC_OUTPUT_SIZE] = {0};
        struct HksBlob input = { sizeof(tmpInput), tmpInput };
        struct HksBlob output = { sizeof(tmpOutput), tmpOutput };
        struct HksBlob handleBlob = { sizeof(uint64_t), (uint8_t *)&handle[i] };

        if (i == 0) {
            EXPECT_EQ(HksUpdate(&handleBlob, paramSet, &input, &output), HKS_ERROR_NOT_EXIST);
            EXPECT_EQ(HksFinish(&handleBlob, paramSet, &input, &output), HKS_ERROR_NOT_EXIST);
        } else {
            EXPECT_EQ(HksUpdate(&handleBlob, paramSet, &input, &output), HKS_SUCCESS) << "i:" << i;
            EXPECT_EQ(HksFinish(&handleBlob, paramSet, &input, &output), HKS_SUCCESS) << "i:" << i;
        }

        HksFreeParamSet(&paramSet);
    }

    for (uint32_t i = 0; i < MAX_SESSION_NUM_MORE_1; ++i) {
        struct HksBlob handleBlob = { sizeof(uint64_t), (uint8_t *)&handle[i] };
        struct HksParamSet *paramSet = NULL;
        ConstructInitParamSet(&paramSet);
        EXPECT_EQ(HksAbort(&handleBlob, paramSet), HKS_SUCCESS) << "i:" << i;
        HksFreeParamSet(&paramSet);
    }
}

/**
 * @tc.name: HksSessionMaxTest.HksSessionMaxTest001
 * @tc.desc: The static function will return true;
 * @tc.type: FUNC
 */
HWTEST_F(HksSessionMaxTest, HksSessionMaxTest001, TestSize.Level0)
{
    uint8_t alias[] = "test_max_session_key_alias";
    struct HksBlob aliasBlob = { sizeof(alias), alias };
    GenerateBaseKey(&aliasBlob);

    SessionMaxTest(&aliasBlob);

    EXPECT_EQ(HksDeleteKey(&aliasBlob, NULL), 0);
}

/**
 * @tc.name: HksSessionMaxTest.HksSessionMaxTest002
 * @tc.desc: The static function will return true;
 * @tc.type: FUNC
 */
HWTEST_F(HksSessionMaxTest, HksSessionMaxTest002, TestSize.Level0)
{
    struct HksParamSet *paramSet = NULL;
    ConstructInitParamSet(&paramSet);

    uint8_t tmpInput[] = "testForSessionMaxTest";
    uint8_t tmpOutput[HMAC_OUTPUT_SIZE] = {0};
    struct HksBlob input = { sizeof(tmpInput), tmpInput };
    struct HksBlob output = { sizeof(tmpOutput), tmpOutput };
    uint64_t temp = HMAC_OUTPUT_SIZE;
    struct HksBlob handleBlob = { sizeof(uint64_t), (uint8_t *)&temp };

    EXPECT_EQ(HksUpdate(&handleBlob, paramSet, &input, &output), HKS_ERROR_NOT_EXIST);
    EXPECT_EQ(HksFinish(&handleBlob, paramSet, &input, &output), HKS_ERROR_NOT_EXIST);
    EXPECT_EQ(HksAbort(&handleBlob, paramSet), HKS_SUCCESS);

    HksFreeParamSet(&paramSet);
}
}

