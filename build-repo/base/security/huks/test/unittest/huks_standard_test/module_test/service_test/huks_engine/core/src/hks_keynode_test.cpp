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

#include "hks_keynode_test.h"

#include <gtest/gtest.h>
#include <string>

#include "hks_keynode.h"
#include "hks_log.h"
#include "hks_mem.h"
#include "hks_param.h"
#include "hks_type_inner.h"

using namespace testing::ext;
namespace Unittest::HksKeyNodeTest {
class HksKeyNodeTest : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};

void HksKeyNodeTest::SetUpTestCase(void)
{
}

void HksKeyNodeTest::TearDownTestCase(void)
{
}

void HksKeyNodeTest::SetUp()
{
}

void HksKeyNodeTest::TearDown()
{
}

static const struct HksParam g_params[] = {
    {
        .tag = HKS_TAG_CRYPTO_CTX,
        .uint64Param = 0
    },
};

/**
 * @tc.name: HksKeyNodeTest.HksKeyNodeTest001
 * @tc.desc: tdd HksCreateKeyNode, expect keyNode == NULL
 * @tc.type: FUNC
 */
HWTEST_F(HksKeyNodeTest, HksKeyNodeTest001, TestSize.Level0)
{
    HKS_LOG_I("enter HksKeyNodeTest001");
    struct HksParamSet *paramSet = nullptr;
    int32_t ret = HksInitParamSet(&paramSet);
    EXPECT_EQ(ret, HKS_SUCCESS) << "HksKeyNodeTest001 HksInitParamSet failed";
    ret = HksAddParams(paramSet, g_params, sizeof(g_params) / sizeof(g_params[0]));
    EXPECT_EQ(ret, HKS_SUCCESS) << "HksKeyNodeTest001 HksAddParams failed";
    ret = HksBuildParamSet(&paramSet);
    EXPECT_EQ(ret, HKS_SUCCESS) << "HksKeyNodeTest001 HksBuildParamSet failed";
    struct HuksKeyNode *keyNode = HksCreateKeyNode(nullptr, paramSet);
    EXPECT_EQ(keyNode == nullptr, true) << "HksKeyNodeTest001 HksCreateKeyNode not failed";
    HksFreeParamSet(&paramSet);
}
}
