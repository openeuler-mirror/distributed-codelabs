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

#include "hks_core_hal_api_test.h"

#include <gtest/gtest.h>

#include "hks_core_hal_api.h"
#include "hks_log.h"
#include "hks_type_inner.h"

using namespace testing::ext;
namespace Unittest::HksCoreHalApiTest {
class HksCoreHalApiTest : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};

void HksCoreHalApiTest::SetUpTestCase(void)
{
}

void HksCoreHalApiTest::TearDownTestCase(void)
{
}

void HksCoreHalApiTest::SetUp()
{
}

void HksCoreHalApiTest::TearDown()
{
}

/**
 * @tc.name: HksCoreHalApiTest.HksCoreHalApiTest001
 * @tc.desc: tdd HksCoreHalElapsedRealTime, expect HKS_SUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(HksCoreHalApiTest, HksCoreHalApiTest001, TestSize.Level0)
{
    HKS_LOG_I("enter HksCoreHalApiTest001");
    uint64_t curTime = 0;
    int32_t ret = HksCoreHalElapsedRealTime(&curTime);
    EXPECT_EQ(ret, HKS_SUCCESS) << "HksCoreHalElapsedRealTime failed";
    EXPECT_EQ(curTime > 0, true) << "HksCoreHalElapsedRealTime failed, curTime = " << curTime;
}
}
