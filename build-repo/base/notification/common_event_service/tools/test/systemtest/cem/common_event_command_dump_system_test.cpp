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

#include <thread>
#include "common_event_manager.h"
#include "common_event_subscriber.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::EventFwk;

namespace {
const std::string STRING_EVENT = "com.ces.event";
const std::string STRING_NO_SUBSCRIBERS =
    "Subscribers:\tNo information\n"
    "Sticky Events:\tNo information\n"
    "Pending Events:\tNo information\n"
    "History Events:\tNo information\n";
const int32_t TIME_DELAY_FOR_SERVICES = 2;

std::string ExecuteCommand(const std::string &command)
{
    std::string result = "";
    FILE *file = popen(command.c_str(), "r");

    // wait for services
    std::this_thread::sleep_for(std::chrono::seconds(TIME_DELAY_FOR_SERVICES));

    if (file != nullptr) {
        char commandResult[1024] = {0};
        while ((fgets(commandResult, sizeof(commandResult), file)) != nullptr) {
            result.append(commandResult);
        }
        pclose(file);
        file = nullptr;
    }

    return result;
}
}  // namespace

class CemCommandDumpSystemTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void CemCommandDumpSystemTest::SetUpTestCase()
{}

void CemCommandDumpSystemTest::TearDownTestCase()
{}

void CemCommandDumpSystemTest::SetUp()
{
    // reset optind to 0
    optind = 0;
}

void CemCommandDumpSystemTest::TearDown()
{}

class CommonEventSubscriberTest : public CommonEventSubscriber {
public:
    explicit CommonEventSubscriberTest(const CommonEventSubscribeInfo &subscribeInfo)
        : CommonEventSubscriber(subscribeInfo)
    {}

    ~CommonEventSubscriberTest()
    {}

    void OnReceiveEvent(const CommonEventData &data)
    {}
};

/**
 * @tc.number: Cem_Command_Dump_SystemTest_0100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem dump -a" command with a subscriber.
 */
HWTEST_F(CemCommandDumpSystemTest, Cem_Command_Dump_SystemTest_0100, Function | MediumTest | Level1)
{
    /* Subscribe */

    // make matching skills
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(STRING_EVENT);

    // make subscriber info
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    // make a subscriber object
    auto subscriberTestPtr = std::make_shared<CommonEventSubscriberTest>(subscribeInfo);
    // subscribe a common event
    CommonEventManager::SubscribeCommonEvent(subscriberTestPtr);

    // dump all subscribers
    std::string command = "cem dump -a";
    std::string commandResult = ExecuteCommand(command);

    EXPECT_NE(commandResult, "");

    // unsubscribe a common event
    CommonEventManager::UnSubscribeCommonEvent(subscriberTestPtr);
}

/**
 * @tc.number: Cem_Command_Dump_SystemTest_0200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem dump -e <name>" command with no subscriber.
 */
HWTEST_F(CemCommandDumpSystemTest, Cem_Command_Dump_SystemTest_0200, Function | MediumTest | Level1)
{
    // dump all subscribers for an event
    std::string command = "cem dump -e " + STRING_EVENT;
    std::string commandResult = ExecuteCommand(command);

    EXPECT_EQ(commandResult, STRING_NO_SUBSCRIBERS);
}

/**
 * @tc.number: Cem_Command_Dump_SystemTest_0300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem dump -e <name>" command with a subscriber.
 */
HWTEST_F(CemCommandDumpSystemTest, Cem_Command_Dump_SystemTest_0300, Function | MediumTest | Level1)
{
    /* Subscribe */

    // make matching skills
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(STRING_EVENT);

    // make subscriber info
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    // make a subscriber object
    auto subscriberTestPtr = std::make_shared<CommonEventSubscriberTest>(subscribeInfo);
    // subscribe a common event
    CommonEventManager::SubscribeCommonEvent(subscriberTestPtr);

    // dump all subscribers for an event
    std::string command = "cem dump -e " + STRING_EVENT;
    std::string commandResult = ExecuteCommand(command);

    EXPECT_NE(commandResult, "");

    // unsubscribe a common event
    CommonEventManager::UnSubscribeCommonEvent(subscriberTestPtr);
}
