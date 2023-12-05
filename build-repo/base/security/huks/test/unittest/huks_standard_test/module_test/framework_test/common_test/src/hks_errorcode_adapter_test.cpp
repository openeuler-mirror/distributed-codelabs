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

#include "hks_errorcode_adapter_test.h"

#include <gtest/gtest.h>

#include "hks_errcode_adapter.h"

#include "hks_log.h"
#include "hks_type.h"

using namespace testing::ext;
namespace Unittest::HksErrorCodeAdapterTest {
class HksErrorCodeAdapterTest : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};

void HksErrorCodeAdapterTest::SetUpTestCase(void)
{
}

void HksErrorCodeAdapterTest::TearDownTestCase(void)
{
}

void HksErrorCodeAdapterTest::SetUp()
{
}

void HksErrorCodeAdapterTest::TearDown()
{
}

/**
 * @tc.name: HksErrorCodeAdapterTest.HksErrorCodeAdapterTest001
 * @tc.desc: tdd HksConvertErrCode, expecting HKS_SUCCESS
 * @tc.type: FUNC
 * @tc.require: issueI5UOK7
 */
HWTEST_F(HksErrorCodeAdapterTest, HksErrorCodeAdapterTest001, TestSize.Level0)
{
    HKS_LOG_I("enter HksErrorCodeAdapterTest001");
    uint32_t ret = HKS_SUCCESS;
    struct HksResult result = HksConvertErrCode(ret);
    ASSERT_EQ(result.errorCode, HKS_SUCCESS) << "HksConvertErrCode failed, ret = " << ret;
}

/**
 * @tc.name: HksErrorCodeAdapterTest.HksErrorCodeAdapterTest002
 * @tc.desc: tdd HksConvertErrCode, expecting HUKS_ERR_CODE_EXTERNAL_ERROR
 * @tc.type: FUNC
 * @tc.require: issueI5UOK7
 */
HWTEST_F(HksErrorCodeAdapterTest, HksErrorCodeAdapterTest002, TestSize.Level0)
{
    HKS_LOG_I("enter HksErrorCodeAdapterTest002");
    uint32_t ret = 1;
    struct HksResult result = HksConvertErrCode(ret);
    ASSERT_EQ(result.errorCode, HUKS_ERR_CODE_EXTERNAL_ERROR) << "HksConvertErrCode failed, ret = " << ret;
}
}
