/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include "publish_manager.h"
#include "system_time.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::EventFwk;

namespace {
constexpr int32_t FLOOD_ATTACK_MAX = 20;
constexpr int32_t NOT_ATTACK_TIME = 10 + FLOOD_ATTACK_MAX;
constexpr int32_t TEST_TIMES = 100;
constexpr int32_t FLOOD_ATTACK_INTERVAL_MAX = 5;
constexpr int32_t SLEEP_TIME = 1000 * FLOOD_ATTACK_INTERVAL_MAX;
constexpr pid_t APPUID1 = 50;
constexpr pid_t APPUID2 = 51;
constexpr pid_t APPUID3 = 52;

class CommonEventPublishManagerEventUnitTest : public testing::Test {
public:
    CommonEventPublishManagerEventUnitTest()
    {}
    ~CommonEventPublishManagerEventUnitTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CommonEventPublishManagerEventUnitTest::SetUpTestCase(void)
{}

void CommonEventPublishManagerEventUnitTest::TearDownTestCase(void)
{}

void CommonEventPublishManagerEventUnitTest::SetUp(void)
{}

void CommonEventPublishManagerEventUnitTest::TearDown(void)
{}

/*
 * @tc.number: CommonEventPublishManagerEventUnitTest_0100
 * @tc.name: test effectinve event floodAttack
 */
HWTEST_F(CommonEventPublishManagerEventUnitTest, CommonEventPublishManagerEventUnitTestt_0100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "CommonEventPublishManagerEventUnitTest, CommonEventPublishManagerEventUnitTestt_0100, TestSize.Level1";

    bool result = false;

    for (int i = 1; i <= TEST_TIMES; ++i) {
        result = DelayedSingleton<PublishManager>::GetInstance()->CheckIsFloodAttack(APPUID1);
        if (result) {
            EXPECT_GT(i, FLOOD_ATTACK_MAX);
        }
    }
    GTEST_LOG_(INFO)
        << "CommonEventPublishManagerEventUnitTest, CommonEventPublishManagerEventUnitTestt_0100, TestSize.Level1 end";
}

/*
 * @tc.number: CommonEventPublishManagerEventUnitTest_0200
 * @tc.name: test not event floodAttack
 */
HWTEST_F(CommonEventPublishManagerEventUnitTest, CommonEventPublishManagerEventUnitTestt_0200,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "CommonEventPublishManagerEventUnitTest, CommonEventPublishManagerEventUnitTestt_0200, TestSize.Level1";

    for (int i = 1; i <= TEST_TIMES; ++i) {
        usleep(SLEEP_TIME);
        EXPECT_FALSE(DelayedSingleton<PublishManager>::GetInstance()->CheckIsFloodAttack(APPUID2));
    }
    GTEST_LOG_(INFO)
        << "CommonEventPublishManagerEventUnitTest, CommonEventPublishManagerEventUnitTestt_0200, TestSize.Level1 end";
}

/*
 * @tc.number: CommonEventPublishManagerEventUnitTest_0300
 * @tc.name: test first some times not event floodAttack then effectinve event floodAttack
 */
HWTEST_F(CommonEventPublishManagerEventUnitTest, CommonEventPublishManagerEventUnitTestt_0300,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "CommonEventPublishManagerEventUnitTest, CommonEventPublishManagerEventUnitTestt_0300, TestSize.Level1";

    bool result = false;

    for (int i = 1; i <= TEST_TIMES; ++i) {
        if (i <= NOT_ATTACK_TIME) {
            EXPECT_FALSE(DelayedSingleton<PublishManager>::GetInstance()->CheckIsFloodAttack(APPUID3));
            usleep(SLEEP_TIME);
        } else {
            result = DelayedSingleton<PublishManager>::GetInstance()->CheckIsFloodAttack(APPUID3);
            if (result) {
                EXPECT_GE(i, NOT_ATTACK_TIME + FLOOD_ATTACK_MAX);
                break;
            }
        }
    }
    GTEST_LOG_(INFO)
        << "CommonEventPublishManagerEventUnitTest, CommonEventPublishManagerEventUnitTestt_0300, TestSize.Level1 end";
}
}  // namespace
