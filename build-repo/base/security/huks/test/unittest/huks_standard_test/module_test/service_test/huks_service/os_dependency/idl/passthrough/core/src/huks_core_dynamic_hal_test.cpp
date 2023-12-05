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

#include "huks_core_dynamic_hal_test.h"

#include <gtest/gtest.h>
#include <string>

#include "hks_api.h"
#include "hks_log.h"
#include "hks_mem.h"
#include "hks_param.h"
#include "huks_core_hal.h"

using namespace testing::ext;
namespace Unittest::HksIpcCoreDynamicHalTest {
class HksIpcCoreDynamicHalTest : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};

void HksIpcCoreDynamicHalTest::SetUpTestCase(void)
{
}

void HksIpcCoreDynamicHalTest::TearDownTestCase(void)
{
}

void HksIpcCoreDynamicHalTest::SetUp()
{
    HksInitialize();
}

void HksIpcCoreDynamicHalTest::TearDown()
{
}

/**
 * @tc.name: HksIpcCoreDynamicHalTest.HksIpcCoreDynamicHalTest001
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HksIpcCoreDynamicHalTest, HksIpcCoreDynamicHalTest001, TestSize.Level0)
{
    HKS_LOG_I("enter HksIpcCoreDynamicHalTest001");
    int32_t ret = HksCreateHuksHdiDevice(NULL);
    EXPECT_EQ(ret, HKS_ERROR_NULL_POINTER) << "HksIpcCoreDynamicHalTest001 failed, ret = " << ret;
}

/**
 * @tc.name: HksIpcCoreDynamicHalTest.HksIpcCoreDynamicHalTest002
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_SUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(HksIpcCoreDynamicHalTest, HksIpcCoreDynamicHalTest002, TestSize.Level0)
{
    HKS_LOG_I("enter HksIpcCoreDynamicHalTest002");
    struct HuksHdi *hdi = nullptr;
    int32_t ret = HksCreateHuksHdiDevice(&hdi);
    EXPECT_EQ(ret, HKS_SUCCESS) << "HksCreateHuksHdiDevice failed, ret = " << ret;
    ret = HksCreateHuksHdiDevice(&hdi);
    EXPECT_EQ(ret, HKS_SUCCESS) << "redo HksCreateHuksHdiDevice failed, ret = " << ret;
    (void)HksDestroyHuksHdiDevice(&hdi);
    (void)HksDestroyHuksHdiDevice(&hdi);
}

/**
 * @tc.name: HksIpcCoreDynamicHalTest.HksIpcCoreDynamicHalTest003
 * @tc.desc: tdd HksDestroyHuksHdiDevice, expect HKS_SUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(HksIpcCoreDynamicHalTest, HksIpcCoreDynamicHalTest003, TestSize.Level0)
{
    HKS_LOG_I("enter HksIpcCoreDynamicHalTest003");
    struct HuksHdi *hdi = nullptr;
    int32_t ret = HksCreateHuksHdiDevice(&hdi);
    EXPECT_EQ(ret, HKS_SUCCESS) << "HksCreateHuksHdiDevice failed, ret = " << ret;
    ret = HksDestroyHuksHdiDevice(&hdi);
    EXPECT_EQ(ret, HKS_SUCCESS) << "HksDestroyHuksHdiDevice failed, ret = " << ret;
}

/**
 * @tc.name: HksIpcCoreDynamicHalTest.HksIpcCoreDynamicHalTest004
 * @tc.desc: tdd HksDestroyHuksHdiDevice, expect HKS_SUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(HksIpcCoreDynamicHalTest, HksIpcCoreDynamicHalTest004, TestSize.Level0)
{
    HKS_LOG_I("enter HksIpcCoreDynamicHalTest004");
    struct HuksHdi *hdi = nullptr;
    int32_t ret = HksDestroyHuksHdiDevice(&hdi);
    EXPECT_EQ(ret, HKS_SUCCESS) << "HksDestroyHuksHdiDevice failed, ret = " << ret;
}
}
