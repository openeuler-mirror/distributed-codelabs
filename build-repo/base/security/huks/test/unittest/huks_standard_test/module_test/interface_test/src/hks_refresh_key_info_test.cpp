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

#include "hks_refresh_key_info_test.h"

#include <gtest/gtest.h>
#include <thread>
#include <unistd.h>

#include "hks_api.h"
#include "hks_log.h"
#include "hks_type.h"

using namespace testing::ext;
namespace Unittest::HksRefreshKeyInfoTest {
class HksRefreshKeyInfoTest : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};

void HksRefreshKeyInfoTest::SetUpTestCase(void)
{
}

void HksRefreshKeyInfoTest::TearDownTestCase(void)
{
}

void HksRefreshKeyInfoTest::SetUp()
{
    EXPECT_EQ(HksInitialize(), 0);
}

void HksRefreshKeyInfoTest::TearDown()
{
}

/**
 * @tc.name: HksRefreshKeyInfoTest.HksRefreshKeyInfoTest001
 * @tc.desc: tdd HksRefreshKeyInfo, expect HKS_SUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(HksRefreshKeyInfoTest, HksRefreshKeyInfoTest001, TestSize.Level0)
{
    HKS_LOG_I("enter HksRefreshKeyInfoTest001");
    int32_t ret = HksRefreshKeyInfo();
    EXPECT_EQ(ret, HKS_SUCCESS) << "HksRefreshKeyInfoTest001 failed, ret = " << ret;
}
}
