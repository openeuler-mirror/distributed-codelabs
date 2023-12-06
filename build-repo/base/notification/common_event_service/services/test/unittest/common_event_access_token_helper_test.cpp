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

#include <gtest/gtest.h>
#include <numeric>

#include "access_token_helper.h"
#include "mock_constant.h"

namespace OHOS {
namespace EventFwk {
using namespace testing::ext;
using namespace OHOS::Security::AccessToken;

class CommonEventAccessTokenHelperTest : public testing::Test {
public:
    CommonEventAccessTokenHelperTest()
    {}
    ~CommonEventAccessTokenHelperTest()
    {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CommonEventAccessTokenHelperTest::SetUpTestCase(void)
{}

void CommonEventAccessTokenHelperTest::TearDownTestCase(void)
{}

void CommonEventAccessTokenHelperTest::SetUp(void)
{}

void CommonEventAccessTokenHelperTest::TearDown(void)
{}

/**
 * @tc.name: IsDlpHap_0100
 * @tc.desc: Judge DLP accessToken
 * @tc.type: FUNC
 * @tc.require: I582VA
 */
HWTEST_F(CommonEventAccessTokenHelperTest, IsDlpHap_0100, Level1)
{
    GTEST_LOG_(INFO) << "IsDlpHap_0100 start";

    AccessTokenID callerToken = DLP_PERMISSION_GRANTED;
    EXPECT_TRUE(AccessTokenHelper::IsDlpHap(callerToken));

    callerToken = PERMISSION_GRANTED;
    EXPECT_FALSE(AccessTokenHelper::IsDlpHap(callerToken));

    GTEST_LOG_(INFO) << "IsDlpHap_0100 end";
}

/**
 * @tc.name: VerifyNativeToken_0100
 * @tc.desc: Judge DLP accessToken
 * @tc.type: FUNC
 * @tc.require: I5R11Y
 */
HWTEST_F(CommonEventAccessTokenHelperTest, VerifyNativeToken_0100, Level1)
{
    AccessTokenID callerToken = DLP_PERMISSION_GRANTED;
    EXPECT_FALSE(AccessTokenHelper::VerifyNativeToken(callerToken));

    callerToken = PERMISSION_GRANTED;
    EXPECT_FALSE(AccessTokenHelper::VerifyNativeToken(callerToken));
}

/**
 * @tc.name: VerifyAccessToken_0100
 * @tc.desc: Judge DLP accessToken
 * @tc.type: FUNC
 * @tc.require: I5R11Y
 */
HWTEST_F(CommonEventAccessTokenHelperTest, VerifyAccessToken_0100, Level1)
{
    std::string permission = "PERMISSION";
    AccessTokenID callerToken = DLP_PERMISSION_GRANTED;
    EXPECT_FALSE(AccessTokenHelper::VerifyAccessToken(callerToken, permission));

    callerToken = PERMISSION_GRANTED;
    EXPECT_FALSE(AccessTokenHelper::VerifyAccessToken(callerToken, permission));
}
}
}