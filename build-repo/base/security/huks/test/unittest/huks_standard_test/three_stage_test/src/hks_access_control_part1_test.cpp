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

#include "hks_access_control_part1_test.h"

#include <gtest/gtest.h>
#include <vector>

#include "hks_access_control_test_common.h"

using namespace testing::ext;
namespace Unittest::AccessControl {
class HksAccessControlPart1Test : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};

void HksAccessControlPart1Test::SetUpTestCase(void)
{
}

void HksAccessControlPart1Test::TearDownTestCase(void)
{
}

void HksAccessControlPart1Test::SetUp()
{
    EXPECT_EQ(HksInitialize(), 0);
}

void HksAccessControlPart1Test::TearDown()
{
}

/**
 * @tc.name: HksAccessControlPart1Test.HksAccessControlPart1Test001
 * @tc.desc: alg-AES gen-pur-Encrypt.
 * @tc.type: FUNC
 * @tc.auth_type: FACE
 * @tc.result:HKS_SUCCESS
 * @tc.require: issueI5NY0M
 */
HWTEST_F(HksAccessControlPart1Test, HksAccessControlPart1Test001, TestSize.Level0)
{
    std::vector<uint8_t> token;
    int32_t ret = HksAccessControlPartTest::AuthTokenImportKey();
    EXPECT_EQ(ret, HKS_SUCCESS) << "ImportKey failed.";
    const char *challenge = "AuthToken_Sign_Verify_KeyAliass";
    struct HksBlob challengeBlob = { TOKEN_CHALLENGE_LEN, (uint8_t *)challenge };
    ret = HksAccessControlPartTest::AuthTokenSign(&challengeBlob, 1, 1, 0, token);
    EXPECT_EQ(ret, HKS_SUCCESS) << "Authtoken Sign failed.";
}
}
