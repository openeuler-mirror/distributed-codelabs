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

#include "useridm_mock_test.h"

#include <gtest/gtest.h>

#include "hks_log.h"
#include "hks_type.h"
#include "hks_useridm_api_wrap.h"

using namespace testing::ext;
namespace Unittest::HksServiceUseridmMockTest {
class HksServiceUseridmMockTest : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};

void HksServiceUseridmMockTest::SetUpTestCase(void)
{
}

void HksServiceUseridmMockTest::TearDownTestCase(void)
{
}

void HksServiceUseridmMockTest::SetUp()
{
}

void HksServiceUseridmMockTest::TearDown()
{
}

/**
 * @tc.name: HksAccessControlRsaSignVerifyTest.HksServiceUseridmMockTest001
 * @tc.desc: try convert between the huks auth type and iam auth type NULL
 * @tc.type: FUNC
 * @tc.result:HKS_SUCCESS
 * @tc.require: issueI5NY0M
 */
HWTEST_F(HksServiceUseridmMockTest, HksServiceUseridmMockTest001, TestSize.Level0)
{
    HKS_LOG_I("Enter HksServiceUseridmMockTest001");
    uint32_t userIamPin = 1;
    int32_t ret = HksConvertUserIamTypeToHksType(HKS_AUTH_TYPE, userIamPin, NULL);
    EXPECT_EQ(ret, HKS_ERROR_NULL_POINTER);
}

/**
 * @tc.name: HksAccessControlRsaSignVerifyTest.HksServiceUseridmMockTest002
 * @tc.desc: try convert between the huks auth type and iam auth type pin
 * @tc.type: FUNC
 * @tc.result:HKS_SUCCESS
 * @tc.require: issueI5NY0M
 */
HWTEST_F(HksServiceUseridmMockTest, HksServiceUseridmMockTest002, TestSize.Level0)
{
    HKS_LOG_I("Enter HksServiceUseridmMockTest002");
    uint32_t userIamPin = 1;
    uint32_t hksValue = 0;
    (void)HksConvertUserIamTypeToHksType(HKS_AUTH_TYPE, userIamPin, &hksValue);
    EXPECT_EQ(hksValue, HKS_USER_AUTH_TYPE_PIN);
}

/**
 * @tc.name: HksAccessControlRsaSignVerifyTest.HksServiceUseridmMockTest003
 * @tc.desc: try convert between the huks auth type and iam auth type face
 * @tc.type: FUNC
 * @tc.result:HKS_SUCCESS
 * @tc.require: issueI5NY0M
 */
HWTEST_F(HksServiceUseridmMockTest, HksServiceUseridmMockTest003, TestSize.Level0)
{
    HKS_LOG_I("Enter HksServiceUseridmMockTest003");
    uint32_t userIamFace = 2;
    uint32_t hksValue = 0;
    (void)HksConvertUserIamTypeToHksType(HKS_AUTH_TYPE, userIamFace, &hksValue);
    EXPECT_EQ(hksValue, HKS_USER_AUTH_TYPE_FACE);
}

/**
 * @tc.name: HksAccessControlRsaSignVerifyTest.HksServiceUseridmMockTest004
 * @tc.desc: try convert between the huks auth type and iam auth type fingerprint
 * @tc.type: FUNC
 * @tc.result:HKS_SUCCESS
 * @tc.require: issueI5NY0M
 */
HWTEST_F(HksServiceUseridmMockTest, HksServiceUseridmMockTest004, TestSize.Level0)
{
    HKS_LOG_I("Enter HksServiceUseridmMockTest004");
    uint32_t userIamFinger = 4;
    uint32_t hksValue = 0;
    (void)HksConvertUserIamTypeToHksType(HKS_AUTH_TYPE, userIamFinger, &hksValue);
    EXPECT_EQ(hksValue, HKS_USER_AUTH_TYPE_FINGERPRINT);
}
}
