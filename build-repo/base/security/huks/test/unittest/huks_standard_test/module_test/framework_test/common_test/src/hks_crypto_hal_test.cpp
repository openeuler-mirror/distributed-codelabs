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

#include "hks_crypto_hal_test.h"

#include <gtest/gtest.h>

#include "hks_base_check.h"

#include "hks_crypto_hal.h"
#include "hks_log.h"
#include "hks_mem.h"
#include "hks_ability.h"
#include "hks_type.h"

#include <cstring>
#include "securec.h"

using namespace testing::ext;
namespace Unittest::HksFrameworkCommonCryptoHalTest {
class HksCryptoHalTest : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};

void HksCryptoHalTest::SetUpTestCase(void)
{
    (void)HksCryptoAbilityInit();
}

void HksCryptoHalTest::TearDownTestCase(void)
{
}

void HksCryptoHalTest::SetUp()
{
}

void HksCryptoHalTest::TearDown()
{
}

/**
 * @tc.name: HksCryptoHalTest.HksCryptoHalTest001
 * @tc.desc: tdd HksCryptoHalFillPrivRandom, expecting non-all-0 data
 * @tc.type: FUNC
 */
HWTEST_F(HksCryptoHalTest, HksCryptoHalTest001, TestSize.Level0)
{
    HKS_LOG_I("enter HksCryptoHalTest001");
    const uint32_t randomSize = 32;
    struct HksBlob random = { .size = randomSize, .data = reinterpret_cast<uint8_t *>(HksMalloc(randomSize)) };
    (void)memset_s(random.data, random.size, 0, random.size);
    int32_t ret = HksCryptoHalFillPrivRandom(&random);
    ASSERT_EQ(ret, HKS_SUCCESS);
    uint32_t i = 0;
    bool isAllZero = true;
    for (; i < random.size; ++i) {
        if (random.data[i] != 0) {
            isAllZero = false;
            break;
        }
    }
    HKS_FREE_BLOB(random);
    ASSERT_EQ(isAllZero, false);
}
}
