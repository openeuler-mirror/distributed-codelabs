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

#include "hks_access_control_agree_test.h"
#include "hks_access_control_test_common.h"
#include "hks_api.h"

#include <gtest/gtest.h>
#include <vector>

using namespace testing::ext;
namespace Unittest::HksAccessControlPartTest {
class HksAccessControlAgreeTest : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};

void HksAccessControlAgreeTest::SetUpTestCase(void)
{
}

void HksAccessControlAgreeTest::TearDownTestCase(void)
{
}

void HksAccessControlAgreeTest::SetUp()
{
    ASSERT_EQ(HksInitialize(), 0);
}

void HksAccessControlAgreeTest::TearDown()
{
}

/* 001: gen ecdh for agree; init for agree */
const TestAccessCaseParams HKS_ACCESS_TEST_001_PARAMS = {
    .genParams =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_ECC },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_AGREE },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_ECC_KEY_SIZE_256 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_NONE },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CBC },
            { .tag = HKS_TAG_USER_AUTH_TYPE, .uint32Param = HKS_USER_AUTH_TYPE_PIN },
            { .tag = HKS_TAG_KEY_AUTH_ACCESS_TYPE, .uint32Param = HKS_AUTH_ACCESS_INVALID_CLEAR_PASSWORD },
            { .tag = HKS_TAG_CHALLENGE_TYPE, .uint32Param = HKS_CHALLENGE_TYPE_NORMAL },
        },
    .initParams =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_ECDH },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_AGREE },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_ECC_KEY_SIZE_256 },
            { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
            { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CBC },
            { .tag = HKS_TAG_USER_AUTH_TYPE, .uint32Param = HKS_USER_AUTH_TYPE_PIN },
            { .tag = HKS_TAG_KEY_AUTH_ACCESS_TYPE, .uint32Param = HKS_AUTH_ACCESS_INVALID_CLEAR_PASSWORD },
            { .tag = HKS_TAG_CHALLENGE_TYPE, .uint32Param = HKS_CHALLENGE_TYPE_NORMAL },
        },
    .initResult = HKS_SUCCESS
};
static struct HksBlob g_keyAliasFinal001 = {
    strlen("HksECDHAgreeKeyAliasTest001_final"),
    (uint8_t *)"HksECDHAgreeKeyAliasTest001_final"
};

static struct HksParam g_agreeAccessFinish001[] = {
    {
        .tag = HKS_TAG_KEY_STORAGE_FLAG,
        .uint32Param = HKS_STORAGE_PERSISTENT
    }, {
        .tag = HKS_TAG_IS_KEY_ALIAS,
        .boolParam = true
    }, {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_AES
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_AES_KEY_SIZE_256
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_DERIVE
    }, {
        .tag = HKS_TAG_DIGEST,
        .uint32Param = HKS_DIGEST_SHA256
    }, {
        .tag = HKS_TAG_KEY_ALIAS,
        .blob = g_keyAliasFinal001
    }, {
        .tag = HKS_TAG_PADDING,
        .uint32Param = HKS_PADDING_NONE
    }, {
        .tag = HKS_TAG_BLOCK_MODE,
        .uint32Param = HKS_MODE_CBC
    }
};

/* 002: gen x25519 for agree; init for agree */
const TestAccessCaseParams HKS_ACCESS_TEST_002_PARAMS = {
    .genParams =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_X25519 },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_AGREE },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_CURVE25519_KEY_SIZE_256 },
            { .tag = HKS_TAG_USER_AUTH_TYPE, .uint32Param = HKS_USER_AUTH_TYPE_FACE },
            { .tag = HKS_TAG_KEY_AUTH_ACCESS_TYPE, .uint32Param = HKS_AUTH_ACCESS_INVALID_NEW_BIO_ENROLL },
            { .tag = HKS_TAG_CHALLENGE_TYPE, .uint32Param = HKS_CHALLENGE_TYPE_NORMAL },
        },
    .initParams =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_X25519 },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_AGREE },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_CURVE25519_KEY_SIZE_256 },
            { .tag = HKS_TAG_USER_AUTH_TYPE, .uint32Param = HKS_USER_AUTH_TYPE_FACE },
            { .tag = HKS_TAG_KEY_AUTH_ACCESS_TYPE, .uint32Param = HKS_AUTH_ACCESS_INVALID_NEW_BIO_ENROLL },
            { .tag = HKS_TAG_CHALLENGE_TYPE, .uint32Param = HKS_CHALLENGE_TYPE_NORMAL },
        },
    .initResult = HKS_SUCCESS
};
static struct HksBlob g_keyAliasFinal002 = {
    strlen("HksX25519AgreeKeyAliasTest002_final"),
    (uint8_t *)"HksX25519AgreeKeyAliasTest002_final"
};
static struct HksParam g_agreeAccessFinish002[] = {
    {
        .tag = HKS_TAG_KEY_STORAGE_FLAG,
        .uint32Param = HKS_STORAGE_PERSISTENT
    }, {
        .tag = HKS_TAG_IS_KEY_ALIAS,
        .boolParam = true
    }, {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_AES
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_AES_KEY_SIZE_256
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_DERIVE
    }, {
        .tag = HKS_TAG_DIGEST,
        .uint32Param = HKS_DIGEST_SHA256
    }, {
        .tag = HKS_TAG_KEY_ALIAS,
        .blob = g_keyAliasFinal002
    }, {
        .tag = HKS_TAG_PADDING,
        .uint32Param = HKS_PADDING_NONE
    }, {
        .tag = HKS_TAG_BLOCK_MODE,
        .uint32Param = HKS_MODE_CBC
    }
};

/* 003: gen dh for agree; init for agree */
const TestAccessCaseParams HKS_ACCESS_TEST_003_PARAMS = {
    .genParams =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_DH },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_AGREE },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_DH_KEY_SIZE_2048 },
            { .tag = HKS_TAG_USER_AUTH_TYPE, .uint32Param = HKS_USER_AUTH_TYPE_FINGERPRINT },
            { .tag = HKS_TAG_KEY_AUTH_ACCESS_TYPE, .uint32Param = HKS_AUTH_ACCESS_INVALID_NEW_BIO_ENROLL },
            { .tag = HKS_TAG_CHALLENGE_TYPE, .uint32Param = HKS_CHALLENGE_TYPE_NORMAL },
        },
    .initParams =
        {
            { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_DH },
            { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_AGREE },
            { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_DH_KEY_SIZE_2048 },
            { .tag = HKS_TAG_USER_AUTH_TYPE, .uint32Param = HKS_USER_AUTH_TYPE_FINGERPRINT },
            { .tag = HKS_TAG_KEY_AUTH_ACCESS_TYPE, .uint32Param = HKS_AUTH_ACCESS_INVALID_NEW_BIO_ENROLL },
            { .tag = HKS_TAG_CHALLENGE_TYPE, .uint32Param = HKS_CHALLENGE_TYPE_NORMAL },
        },
    .initResult = HKS_SUCCESS
};
static struct HksBlob g_keyAliasFinal003 = {
    strlen("HksDHAgreeKeyAliasTest003_final"),
    (uint8_t *)"HksDHAgreeKeyAliasTest003_final"
};
static struct HksParam g_agreeAccessFinish003[] = {
    {
        .tag = HKS_TAG_KEY_STORAGE_FLAG,
        .uint32Param = HKS_STORAGE_PERSISTENT
    }, {
        .tag = HKS_TAG_IS_KEY_ALIAS,
        .boolParam = true
    }, {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_AES
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_AES_KEY_SIZE_256
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_DERIVE
    }, {
        .tag = HKS_TAG_DIGEST,
        .uint32Param = HKS_DIGEST_SHA256
    }, {
        .tag = HKS_TAG_KEY_ALIAS,
        .blob = g_keyAliasFinal003
    }, {
        .tag = HKS_TAG_PADDING,
        .uint32Param = HKS_PADDING_NONE
    }, {
        .tag = HKS_TAG_BLOCK_MODE,
        .uint32Param = HKS_MODE_CBC
    }
};

/**
 * @tc.name: HksAccessControlAgreeTest.HksAccessAgreePartTest001
 * @tc.desc: alg-ECDH gen-pur-Agree.
 * @tc.type: FUNC
 * @tc.auth_type: PIN
 * @tc.result:HKS_SUCCESS
 * @tc.require: issueI5NY0M
 */
HWTEST_F(HksAccessControlAgreeTest, HksAccessAgreePartTest001, TestSize.Level0)
{
    HKS_LOG_I("Enter HksAccessAgreePartTest001");
    const IDMParams testIDMParams = {
        .secureUid = 1,
        .enrolledId = 1,
        .time = 0,
        .authType = 1
    };
    struct HksParamSet *finishParamSet = nullptr;
    int ret = InitParamSet(&finishParamSet, g_agreeAccessFinish001,
        sizeof(g_agreeAccessFinish001) / sizeof(HksParam));
    ASSERT_EQ(ret, HKS_SUCCESS);
    ASSERT_EQ(CheckAccessAgreeTest(HKS_ACCESS_TEST_001_PARAMS, finishParamSet,
        testIDMParams), HKS_SUCCESS);
    ASSERT_EQ(HksDeleteKey(&g_keyAliasFinal001, nullptr), HKS_SUCCESS);
}

/**
 * @tc.name: HksAccessControlAgreeTest.HksAccessAgreePartTest002
 * @tc.desc: alg-DH gen-pur-Agree.
 * @tc.type: FUNC
 * @tc.auth_type: FACE
 * @tc.result:HKS_SUCCESS
 * @tc.require: issueI5NY0M
 */
HWTEST_F(HksAccessControlAgreeTest, HksAccessAgreePartTest002, TestSize.Level0)
{
    HKS_LOG_I("Enter HksAccessAgreePartTest002");
    const IDMParams testIDMParams = {
        .secureUid = 1,
        .enrolledId = 3,
        .time = 0,
        .authType = 2
    };
    struct HksParamSet *finishParamSet = nullptr;
    int ret = InitParamSet(&finishParamSet, g_agreeAccessFinish002,
        sizeof(g_agreeAccessFinish002) / sizeof(HksParam));
    ASSERT_EQ(ret, HKS_SUCCESS);
    ASSERT_EQ(CheckAccessAgreeTest(HKS_ACCESS_TEST_002_PARAMS, finishParamSet,
        testIDMParams), HKS_SUCCESS);
    ASSERT_EQ(HksDeleteKey(&g_keyAliasFinal002, nullptr), HKS_SUCCESS);
}

/**
 * @tc.name: HksAccessControlAgreeTest.HksAccessAgreePartTest003
 * @tc.desc: alg-X25519 gen-pur-Agree.
 * @tc.type: FUNC
 * @tc.auth_type: FINGERPRINT
 * @tc.result:HKS_SUCCESS
 * @tc.require: issueI5NY0M
 */
HWTEST_F(HksAccessControlAgreeTest, HksAccessAgreePartTest003, TestSize.Level0)
{
    HKS_LOG_I("Enter HksAccessAgreePartTest003");
    const IDMParams testIDMParams = {
        .secureUid = 1,
        .enrolledId = 2,
        .time = 0,
        .authType = 4
    };
    struct HksParamSet *finishParamSet = nullptr;
    int ret = InitParamSet(&finishParamSet, g_agreeAccessFinish003,
        sizeof(g_agreeAccessFinish003) / sizeof(HksParam));
    ASSERT_EQ(ret, HKS_SUCCESS);
    ASSERT_EQ(CheckAccessAgreeTest(HKS_ACCESS_TEST_003_PARAMS, finishParamSet,
        testIDMParams), HKS_SUCCESS);
    ASSERT_EQ(HksDeleteKey(&g_keyAliasFinal003, nullptr), HKS_SUCCESS);
}
}
