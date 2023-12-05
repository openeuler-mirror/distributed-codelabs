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

#include "hks_condition_test.h"

#include <gtest/gtest.h>
#include <thread>
#include <unistd.h>

#include "hks_condition.h"
#include "hks_log.h"
#include "hks_type.h"

struct HksCondition {
    bool notified;
    bool waited;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

using namespace testing::ext;
namespace Unittest::HksUtilsConditionTest {
class HksConditionTest : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};

void HksConditionTest::SetUpTestCase(void)
{
}

void HksConditionTest::TearDownTestCase(void)
{
}

void HksConditionTest::SetUp()
{
}

void HksConditionTest::TearDown()
{
}

/**
 * @tc.name: HksConditionTest.HksConditionTest001
 * @tc.desc: tdd HksConditionWait, with nullptr input, expect -1
 * @tc.type: FUNC
 */
HWTEST_F(HksConditionTest, HksConditionTest001, TestSize.Level0)
{
    HKS_LOG_I("enter HksConditionTest001");
    int32_t ret = HksConditionWait(nullptr);
    EXPECT_EQ(ret, -1) << "HksConditionTest001 failed, ret = " << ret;
}

void NotifyCondition(HksCondition *condition)
{
    sleep(1);
    (void)condition;
    int32_t ret = HksConditionNotify(condition);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("HksConditionNotify failed, ret = %" LOG_PUBLIC "d", ret);
    }
}

/**
 * @tc.name: HksConditionTest.HksConditionTest002
 * @tc.desc: tdd HksConditionWait, with notified false, expecting HKS_SUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(HksConditionTest, HksConditionTest002, TestSize.Level0)
{
    HKS_LOG_I("enter HksConditionTest002");
    HksCondition *condition = HksConditionCreate();
    EXPECT_EQ(condition == nullptr, false) << "HksConditionCreate failed";
    std::thread thObj(NotifyCondition, condition);
    int32_t ret = HksConditionWait(condition);
    EXPECT_EQ(ret, HKS_SUCCESS) << "HksConditionTest002 failed, ret = " << ret;
    HksConditionDestroy(condition);
    thObj.join();
}

/**
 * @tc.name: HksConditionTest.HksConditionTest003
 * @tc.desc: tdd HksConditionWait, with notified true, expecting HKS_SUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(HksConditionTest, HksConditionTest003, TestSize.Level0)
{
    HKS_LOG_I("enter HksConditionTest003");
    HksCondition *condition = HksConditionCreate();
    condition->notified = true;
    int32_t ret = HksConditionWait(condition);
    EXPECT_EQ(ret, HKS_SUCCESS) << "HksConditionTest003 failed, ret = " << ret;
    HksConditionDestroy(condition);
}

/**
 * @tc.name: HksConditionTest.HksConditionTest004
 * @tc.desc: tdd HksConditionNotify, with waited false, expecting HKS_SUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(HksConditionTest, HksConditionTest004, TestSize.Level0)
{
    HKS_LOG_I("enter HksConditionTest004");
    HksCondition *condition = HksConditionCreate();
    condition->notified = true;
    int32_t ret = HksConditionNotify(condition);
    EXPECT_EQ(ret, HKS_SUCCESS) << "HksConditionTest004 failed, ret = " << ret;
    HksConditionDestroy(condition);
}

/**
 * @tc.name: HksConditionTest.HksConditionTest005
 * @tc.desc: tdd HksConditionDestroy, with nullptr input
 * @tc.type: FUNC
 */
HWTEST_F(HksConditionTest, HksConditionTest005, TestSize.Level0)
{
    HKS_LOG_I("enter HksConditionTest005");
    HksConditionDestroy(nullptr);
}
}
