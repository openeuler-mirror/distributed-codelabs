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

#include "hks_useridm_api_wrap_test.h"

#include <gtest/gtest.h>

#include "hks_log.h"
#include "hks_mem.h"
#include "hks_type.h"
#include "hks_useridm_api_wrap.h"
#include "user_idm_client.h"

using namespace testing::ext;
using namespace OHOS::UserIam::UserAuth;
namespace Unittest::HksServiceUseridmWrapTest {
class HksUseridmWrapTest : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};

void HksUseridmWrapTest::SetUpTestCase(void)
{
}

void HksUseridmWrapTest::TearDownTestCase(void)
{
}

void HksUseridmWrapTest::SetUp()
{
}

void HksUseridmWrapTest::TearDown()
{
}

/**
 * @tc.name: HksUseridmWrapTest.HksUseridmWrapTest001
 * @tc.desc: tdd HksUserIdmGetSecInfo, with NULL input, expect HKS_ERROR_INVALID_ARGUMENT
 * @tc.type: FUNC
 */
HWTEST_F(HksUseridmWrapTest, HksUseridmWrapTest001, TestSize.Level0)
{
    HKS_LOG_I("enter HksUseridmWrapTest001");
    int32_t ret = HksUserIdmGetSecInfo(0, NULL);
    EXPECT_EQ(ret, HKS_ERROR_INVALID_ARGUMENT) << "HksUseridmWrapTest001 failed, ret = " << ret;
}

/**
 * @tc.name: HksUseridmWrapTest.HksUseridmWrapTest002
 * @tc.desc: tdd HksUserIdmGetSecInfo, with user id 0, expecting HKS_ERROR_GET_USERIAM_SECINFO_FAILED
 * @tc.type: FUNC
 */
HWTEST_F(HksUseridmWrapTest, HksUseridmWrapTest002, TestSize.Level0)
{
    HKS_LOG_I("enter HksUseridmWrapTest002");
    struct SecInfoWrap *secInfoWrap = NULL;
    int32_t ret = HksUserIdmGetSecInfo(0, &secInfoWrap);
    EXPECT_EQ(ret, HKS_ERROR_GET_USERIAM_SECINFO_FAILED) << "HksUseridmWrapTest002 failed, ret = " << ret;
}

/**
 * @tc.name: HksUseridmWrapTest.HksUseridmWrapTest003
 * @tc.desc: tdd HksUserIdmGetAuthInfoNum, with NULL input, expecting HKS_ERROR_INVALID_ARGUMENT
 * @tc.type: FUNC
 */
HWTEST_F(HksUseridmWrapTest, HksUseridmWrapTest003, TestSize.Level0)
{
    HKS_LOG_I("enter HksUseridmWrapTest003");
    int32_t ret = HksUserIdmGetAuthInfoNum(0, (enum HksUserAuthType)HKS_USER_AUTH_TYPE_FINGERPRINT, NULL);
    EXPECT_EQ(ret, HKS_ERROR_INVALID_ARGUMENT) << "HksUseridmWrapTest003 failed, ret = " << ret;
}

static const uint32_t g_useriamPin = 1;
static const uint32_t g_useriamFace = 2;
static const uint32_t g_useriamFingerPrint = 4;

/**
 * @tc.name: HksUseridmWrapTest.HksUseridmWrapTest004
 * @tc.desc: tdd HksConvertUserIamTypeToHksType, expecting HKS_SUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(HksUseridmWrapTest, HksUseridmWrapTest004, TestSize.Level0)
{
    HKS_LOG_I("enter HksUseridmWrapTest004");
    uint32_t hksValue = 0;
    int32_t ret = HksConvertUserIamTypeToHksType(HKS_AUTH_TYPE, g_useriamPin, &hksValue);
    EXPECT_EQ(ret, HKS_SUCCESS) << "HksUseridmWrapTest004 failed, ret = " << ret;
    EXPECT_EQ(hksValue, HKS_USER_AUTH_TYPE_PIN) << "HksConvertUserIamTypeToHksType failed, hksValue = " << hksValue;
}

/**
 * @tc.name: HksUseridmWrapTest.HksUseridmWrapTest005
 * @tc.desc: tdd HksUserIdmGetAuthInfoNum, expecting HKS_ERROR_GET_USERIAM_AUTHINFO_FAILED
 * @tc.type: FUNC
 */
HWTEST_F(HksUseridmWrapTest, HksUseridmWrapTest005, TestSize.Level0)
{
    HKS_LOG_I("enter HksUseridmWrapTest005");
    uint32_t num = 0;
    int32_t ret = HksUserIdmGetAuthInfoNum(0, HKS_USER_AUTH_TYPE_FINGERPRINT, &num);
    EXPECT_EQ(ret, HKS_ERROR_GET_USERIAM_AUTHINFO_FAILED) << "HksUseridmWrapTest005 failed, ret = " << ret;
}

/**
 * @tc.name: HksUseridmWrapTest.HksUseridmWrapTest006
 * @tc.desc: tdd HksConvertUserIamTypeToHksType, expecting HKS_SUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(HksUseridmWrapTest, HksUseridmWrapTest006, TestSize.Level0)
{
    HKS_LOG_I("enter HksUseridmWrapTest006");
    uint32_t hksValue = 0;
    int32_t ret = HksConvertUserIamTypeToHksType(HKS_AUTH_TYPE, g_useriamFace, &hksValue);
    EXPECT_EQ(ret, HKS_SUCCESS) << "HksUseridmWrapTest006 failed, ret = " << ret;
    EXPECT_EQ(hksValue, HKS_USER_AUTH_TYPE_FACE) << "HksConvertUserIamTypeToHksType failed, hksValue = " << hksValue;
}

/**
 * @tc.name: HksUseridmWrapTest.HksUseridmWrapTest007
 * @tc.desc: tdd HksConvertUserIamTypeToHksType, expecting HKS_SUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(HksUseridmWrapTest, HksUseridmWrapTest007, TestSize.Level0)
{
    HKS_LOG_I("enter HksUseridmWrapTest007");
    uint32_t hksValue = 0;
    int32_t ret = HksConvertUserIamTypeToHksType(HKS_AUTH_TYPE, g_useriamFingerPrint, &hksValue);
    EXPECT_EQ(ret, HKS_SUCCESS) << "HksUseridmWrapTest007 failed, ret = " << ret;
    EXPECT_EQ(hksValue, HKS_USER_AUTH_TYPE_FINGERPRINT) <<
        "HksConvertUserIamTypeToHksType failed, hksValue = " << hksValue;
}

/**
 * @tc.name: HksUseridmWrapTest.HksUseridmWrapTest008
 * @tc.desc: tdd HksUserIdmGetAuthInfoNum, expecting HKS_ERROR_GET_USERIAM_AUTHINFO_FAILED
 * @tc.type: FUNC
 */
HWTEST_F(HksUseridmWrapTest, HksUseridmWrapTest008, TestSize.Level0)
{
    HKS_LOG_I("enter HksUseridmWrapTest008");
    uint32_t num = 0;
    int32_t ret = HksUserIdmGetAuthInfoNum(0, HKS_USER_AUTH_TYPE_PIN, &num);
    EXPECT_EQ(ret, HKS_ERROR_GET_USERIAM_AUTHINFO_FAILED) << "HksUseridmWrapTest008 failed, ret = " << ret;
}

/**
 * @tc.name: HksUseridmWrapTest.HksUseridmWrapTest009
 * @tc.desc: tdd HksUserIdmGetAuthInfoNum, expecting HKS_ERROR_GET_USERIAM_AUTHINFO_FAILED
 * @tc.type: FUNC
 */
HWTEST_F(HksUseridmWrapTest, HksUseridmWrapTest009, TestSize.Level0)
{
    HKS_LOG_I("enter HksUseridmWrapTest009");
    uint32_t num = 0;
    int32_t ret = HksUserIdmGetAuthInfoNum(0, HKS_USER_AUTH_TYPE_FACE, &num);
    EXPECT_EQ(ret, HKS_ERROR_GET_USERIAM_AUTHINFO_FAILED) << "HksUseridmWrapTest009 failed, ret = " << ret;
}

/**
 * @tc.name: HksUseridmWrapTest.HksUseridmWrapTest010
 * @tc.desc: tdd HksUserIdmGetAuthInfoNum, expecting HKS_SUCCESS and num > 0
 * @tc.type: FUNC
 */
HWTEST_F(HksUseridmWrapTest, HksUseridmWrapTest010, TestSize.Level0)
{
    HKS_LOG_I("enter HksUseridmWrapTest010");
    ChangeGetCredentialInfoReturn(true);
    uint32_t num = 0;
    int32_t ret = HksUserIdmGetAuthInfoNum(1, (enum HksUserAuthType)HKS_USER_AUTH_TYPE_FINGERPRINT, &num);
    EXPECT_EQ(ret, HKS_SUCCESS) << "HksUseridmWrapTest010 HksUserIdmGetAuthInfoNum failed, ret = " << ret;
    EXPECT_EQ(num > 0, true) << "HksUseridmWrapTest010 HksUserIdmGetAuthInfoNum failed, num = " << num;
}

/**
 * @tc.name: HksUseridmWrapTest.HksUseridmWrapTest011
 * @tc.desc: tdd HksUserIdmGetAuthInfoNum, expecting HKS_SUCCESS and num == 0
 * @tc.type: FUNC
 */
HWTEST_F(HksUseridmWrapTest, HksUseridmWrapTest011, TestSize.Level0)
{
    HKS_LOG_I("enter HksUseridmWrapTest011");
    ChangeGetCredentialInfoReturn(false);
    uint32_t num = 0;
    int32_t ret = HksUserIdmGetAuthInfoNum(1, (enum HksUserAuthType)HKS_USER_AUTH_TYPE_FINGERPRINT, &num);
    EXPECT_EQ(ret, HKS_SUCCESS) << "HksUseridmWrapTest011 HksUserIdmGetAuthInfoNum failed, ret = " << ret;
    EXPECT_EQ(num == 0, true) << "HksUseridmWrapTest011 HksUserIdmGetAuthInfoNum failed, num = " << num;
}

static const uint32_t g_enrolledIdForPin = 1;

/**
 * @tc.name: HksUseridmWrapTest.HksUseridmWrapTest012
 * @tc.desc: tdd HksUserIdmGetSecInfo, expecting HKS_SUCCESS and num > 0
 * @tc.type: FUNC
 */
HWTEST_F(HksUseridmWrapTest, HksUseridmWrapTest012, TestSize.Level0)
{
    HKS_LOG_I("enter HksUseridmWrapTest012");
    ChangeGetSecUserInfoReturn(true);
    struct SecInfoWrap *secInfo = NULL;
    int32_t ret = HksUserIdmGetSecInfo(1, &secInfo);
    EXPECT_EQ(ret, HKS_SUCCESS) << "HksUseridmWrapTest012 HksUserIdmGetSecInfo failed, ret = " << ret;
    EXPECT_EQ(secInfo->enrolledInfoLen > 0, true);
    EXPECT_EQ(secInfo->enrolledInfo[0].authType, HKS_USER_AUTH_TYPE_PIN);
    EXPECT_EQ(secInfo->enrolledInfo[0].enrolledId, g_enrolledIdForPin);
    HKS_FREE_PTR(secInfo);
}

/**
 * @tc.name: HksUseridmWrapTest.HksUseridmWrapTest013
 * @tc.desc: tdd HksUserIdmGetSecInfo, expecting HKS_SUCCESS and num == 0
 * @tc.type: FUNC
 */
HWTEST_F(HksUseridmWrapTest, HksUseridmWrapTest013, TestSize.Level0)
{
    HKS_LOG_I("enter HksUseridmWrapTest013");
    ChangeGetSecUserInfoReturn(false);
    struct SecInfoWrap *secInfo = NULL;
    int32_t ret = HksUserIdmGetSecInfo(1, &secInfo);
    EXPECT_EQ(ret, HKS_SUCCESS) << "HksUseridmWrapTest013 HksUserIdmGetSecInfo failed, ret = " << ret;
    EXPECT_EQ(secInfo->enrolledInfoLen, 0);
    HKS_FREE_PTR(secInfo);
}
}
