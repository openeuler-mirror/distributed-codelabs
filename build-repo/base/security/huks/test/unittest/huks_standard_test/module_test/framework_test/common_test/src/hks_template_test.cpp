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

#include "hks_template_test.h"

#include <gtest/gtest.h>

#include "hks_log.h"
#include "hks_template.h"
#include "hks_type.h"

#include <cstring>

using namespace testing::ext;
namespace Unittest::HksFrameworkCommonTemplateTest {
class HksTemplateTest : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};

void HksTemplateTest::SetUpTestCase(void)
{
}

void HksTemplateTest::TearDownTestCase(void)
{
}

void HksTemplateTest::SetUp()
{
}

void HksTemplateTest::TearDown()
{
}

static int32_t TestLogReturn()
{
    int32_t ret = HKS_ERROR_NOT_EXIST;
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_BAD_STATE, "test result error level")
    return HKS_SUCCESS;
}

/**
 * @tc.name: HksTemplateTest.HksTemplateTest001
 * @tc.desc: tdd HKS_IF_NOT_SUCC_LOGE_RETURN, expecting HKS_ERROR_BAD_STATE
 * @tc.type: FUNC
 */
HWTEST_F(HksTemplateTest, HksTemplateTest001, TestSize.Level0)
{
    HKS_LOG_I("enter HksTemplateTest001");
    int32_t ret = TestLogReturn();
    ASSERT_TRUE(ret == HKS_ERROR_BAD_STATE);
}

static int32_t TestVariadicLogReturn()
{
    int32_t ret = HKS_ERROR_NOT_EXIST;
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_BAD_STATE, "expect: %d", HKS_ERROR_BAD_STATE)
    return HKS_SUCCESS;
}

/**
 * @tc.name: HksTemplateTest.HksTemplateTest002
 * @tc.desc: tdd HKS_IF_NOT_SUCC_LOGE_RETURN, expecting HKS_ERROR_BAD_STATE
 * @tc.type: FUNC
 */
HWTEST_F(HksTemplateTest, HksTemplateTest002, TestSize.Level0)
{
    HKS_LOG_I("enter HksTemplateTest002");
    int32_t ret = TestVariadicLogReturn();
    ASSERT_TRUE(ret == HKS_ERROR_BAD_STATE);
}

static int32_t TestMultiVariadicLogReturn()
{
    int32_t ret = HKS_ERROR_NOT_EXIST;
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_BAD_STATE, "expect: %d, %s", HKS_ERROR_BAD_STATE, "xxxxxxx")
    return HKS_SUCCESS;
}

/**
 * @tc.name: HksTemplateTest.HksTemplateTest003
 * @tc.desc: tdd HKS_IF_NOT_SUCC_LOGE_RETURN, expecting HKS_ERROR_BAD_STATE
 * @tc.type: FUNC
 */
HWTEST_F(HksTemplateTest, HksTemplateTest003, TestSize.Level0)
{
    HKS_LOG_I("enter HksTemplateTest003");
    int32_t ret = TestMultiVariadicLogReturn();
    ASSERT_TRUE(ret == HKS_ERROR_BAD_STATE);
}

/**
 * @tc.name: HksTemplateTest.HksTemplateTest004
 * @tc.desc: tdd HKS_EXCEPRTION_HALDER_LOGE, expecting HKS_ERROR_BAD_STATE
 * @tc.type: FUNC
 */
HWTEST_F(HksTemplateTest, HksTemplateTest004, TestSize.Level0)
{
    HKS_LOG_I("enter HksTemplateTest004");
    int32_t ret = HKS_ERROR_API_NOT_SUPPORTED;
    HKS_IF_NOT_SUCC_LOGE(ret, "HksTemplateTest004")
    HKS_IF_NOT_SUCC_LOGE(ret, "HksTemplateTest004, %d, %s", 0, "HksTemplateTest004")
}

static int32_t TestReturn()
{
    int32_t ret = HKS_ERROR_NOT_EXIST;
    HKS_IF_NOT_SUCC_RETURN(ret, HKS_ERROR_BAD_STATE)
    return HKS_SUCCESS;
}

/**
 * @tc.name: HksTemplateTest.HksTemplateTest005
 * @tc.desc: tdd HKS_IF_NOT_SUCC_RETURN, expecting HKS_ERROR_BAD_STATE
 * @tc.type: FUNC
 */
HWTEST_F(HksTemplateTest, HksTemplateTest005, TestSize.Level0)
{
    HKS_LOG_I("enter HksTemplateTest005");
    int32_t ret = TestReturn();
    ASSERT_TRUE(ret == HKS_ERROR_BAD_STATE);
}

static int32_t TestLogBreak()
{
    uint32_t i = 0;
    const uint32_t max = 10;
    const uint32_t target = 3;
    int32_t ret = HKS_FAILURE;
    for (; i < max; ++i) {
        if (i == target) {
            HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksTemplateTest006, %d, %s", 0, "HksTemplateTest006")
        }
    }
    if (i == target) {
        return HKS_SUCCESS;
    }
    return HKS_FAILURE;
}

/**
 * @tc.name: HksTemplateTest.HksTemplateTest006
 * @tc.desc: tdd HKS_IF_NOT_SUCC_LOGE_BREAK, expecting HKS_SUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(HksTemplateTest, HksTemplateTest006, TestSize.Level0)
{
    HKS_LOG_I("enter HksTemplateTest006");
    int32_t ret = TestLogBreak();
    ASSERT_TRUE(ret == HKS_SUCCESS);
}

static int32_t TestNullLogeReturn()
{
    HKS_IF_NULL_LOGE_RETURN(nullptr, HKS_ERROR_NULL_POINTER, "is nullptr: %d", HKS_ERROR_NULL_POINTER)
    return HKS_SUCCESS;
}

/**
 * @tc.name: HksTemplateTest.HksTemplateTest007
 * @tc.desc: tdd HKS_IF_NULL_LOGE_RETURN, expecting HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HksTemplateTest, HksTemplateTest007, TestSize.Level0)
{
    HKS_LOG_I("enter HksTemplateTest007");
    int32_t ret = TestNullLogeReturn();
    ASSERT_TRUE(ret == HKS_ERROR_NULL_POINTER);
}

static int32_t TestBreak()
{
    uint32_t i = 0;
    const uint32_t max = 10;
    const uint32_t target = 3;
    int32_t ret = HKS_FAILURE;
    for (; i < max; ++i) {
        if (i == target) {
            HKS_IF_NOT_SUCC_BREAK(ret)
        }
    }
    if (i == target) {
        return HKS_SUCCESS;
    }
    return HKS_FAILURE;
}

/**
 * @tc.name: HksTemplateTest.HksTemplateTest008
 * @tc.desc: tdd HKS_IF_NOT_SUCC_BREAK, expecting HKS_SUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(HksTemplateTest, HksTemplateTest008, TestSize.Level0)
{
    HKS_LOG_I("enter HksTemplateTest008");
    int32_t ret = TestBreak();
    ASSERT_TRUE(ret == HKS_SUCCESS);
}
}
