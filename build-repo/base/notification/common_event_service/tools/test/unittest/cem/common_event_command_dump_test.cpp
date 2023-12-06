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

#define private public
#include "common_event_command.h"
#undef private
#include "common_event_manager.h"
#include "common_event_subscriber.h"
#include "mock_common_event_stub.h"
#include "singleton.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::EventFwk;

namespace {
const std::string STRING_EVENT = "com.ces.event";

static std::string Concatenate(const std::string &first,  const std::string &second)
{
    return first + second;
}

class CemCommandDumpTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    void MakeMockObjects();
    void SetMockObjects(const CommonEventCommand &cmd) const;

    std::string cmd_ = "dump";
    std::string toolName_ = TOOL_NAME;
    sptr<ICommonEvent> proxyPtr_;
};

void CemCommandDumpTest::SetUpTestCase()
{}

void CemCommandDumpTest::TearDownTestCase()
{}

void CemCommandDumpTest::SetUp()
{
    // reset optind to 0
    optind = 0;

    // make mock objects
    MakeMockObjects();
}

void CemCommandDumpTest::TearDown()
{}

void CemCommandDumpTest::MakeMockObjects()
{
    // mock a stub
    auto stubPtr = sptr<IRemoteObject>(new MockCommonEventStub());

    // mock a proxy
    proxyPtr_ = iface_cast<ICommonEvent>(stubPtr);

    // set the mock proxy
    auto commonEventPtr = DelayedSingleton<CommonEvent>::GetInstance();
    commonEventPtr->isProxyValid_ = true;
    commonEventPtr->commonEventProxy_ = proxyPtr_;
}

void CemCommandDumpTest::SetMockObjects(const CommonEventCommand &cmd) const
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
 * @tc.number: Cem_Command_Dump_0100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem dump" command.
 */
HWTEST_F(CemCommandDumpTest, Cem_Command_Dump_0100, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)toolName_.c_str(),
        (char *)cmd_.c_str(),
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CommonEventCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), Concatenate(HELP_MSG_NO_OPTION, HELP_MSG_DUMP));
}

/**
 * @tc.number: Cem_Command_Dump_0200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem dump xxx" command.
 */
HWTEST_F(CemCommandDumpTest, Cem_Command_Dump_0200, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)toolName_.c_str(),
        (char *)cmd_.c_str(),
        (char *)"xxx",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CommonEventCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), Concatenate(HELP_MSG_NO_OPTION, HELP_MSG_DUMP));
}

/**
 * @tc.number: Cem_Command_Dump_0300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem dump -x" command.
 */
HWTEST_F(CemCommandDumpTest, Cem_Command_Dump_0300, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)toolName_.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-x",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CommonEventCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), Concatenate("error: unknown option.\n", HELP_MSG_DUMP));
}

/**
 * @tc.number: Cem_Command_Dump_0400
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem dump -xxx" command.
 */
HWTEST_F(CemCommandDumpTest, Cem_Command_Dump_0400, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)toolName_.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-xxx",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CommonEventCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), Concatenate("error: unknown option.\n", HELP_MSG_DUMP));
}

/**
 * @tc.number: Cem_Command_Dump_0500
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem dump --x" command.
 */
HWTEST_F(CemCommandDumpTest, Cem_Command_Dump_0500, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)toolName_.c_str(),
        (char *)cmd_.c_str(),
        (char *)"--x",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CommonEventCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), Concatenate("error: unknown option.\n", HELP_MSG_DUMP));
}

/**
 * @tc.number: Cem_Command_Dump_0600
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem dump --xxx" command.
 */
HWTEST_F(CemCommandDumpTest, Cem_Command_Dump_0600, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)toolName_.c_str(),
        (char *)cmd_.c_str(),
        (char *)"--xxx",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CommonEventCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), Concatenate("error: unknown option.\n", HELP_MSG_DUMP));
}

/**
 * @tc.number: Cem_Command_Dump_0700
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem dump -h" command.
 */
HWTEST_F(CemCommandDumpTest, Cem_Command_Dump_0700, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)toolName_.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-h",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CommonEventCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_DUMP);
}

/**
 * @tc.number: Cem_Command_Dump_0800
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem dump --help" command.
 */
HWTEST_F(CemCommandDumpTest, Cem_Command_Dump_0800, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)toolName_.c_str(),
        (char *)cmd_.c_str(),
        (char *)"--help",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CommonEventCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_DUMP);
}

/**
 * @tc.number: Cem_Command_Dump_0900
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem dump -a" command.
 */
HWTEST_F(CemCommandDumpTest, Cem_Command_Dump_0900, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)toolName_.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-a",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CommonEventCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), "");
}

/**
 * @tc.number: Cem_Command_Dump_1000
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem dump --all" command with no subscriber.
 */
HWTEST_F(CemCommandDumpTest, Cem_Command_Dump_1000, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)toolName_.c_str(),
        (char *)cmd_.c_str(),
        (char *)"--all",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CommonEventCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), "");
}

/**
 * @tc.number: Cem_Command_Dump_1100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem dump -a" command with a subscriber.
 */
HWTEST_F(CemCommandDumpTest, Cem_Command_Dump_1100, Function | MediumTest | Level1)
{
    /* Subscribe */

    // make matching skills
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(STRING_EVENT);

    // make subscribe info
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    // make a subscriber object
    auto subscriberTestPtr = std::make_shared<CommonEventSubscriberTest>(subscribeInfo);
    // subscribe a common event
    CommonEventManager::SubscribeCommonEvent(subscriberTestPtr);

    char *argv[] = {
        (char *)toolName_.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-a",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CommonEventCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), STRING_EVENT + "\n");
}

/**
 * @tc.number: Cem_Command_Dump_1200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem dump -e" command.
 */
HWTEST_F(CemCommandDumpTest, Cem_Command_Dump_1200, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)toolName_.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-e",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CommonEventCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), Concatenate("error: option 'e' requires a value.\n", HELP_MSG_DUMP));
}

/**
 * @tc.number: Cem_Command_Dump_1300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem dump -e <name>" command with no subscriber.
 */
HWTEST_F(CemCommandDumpTest, Cem_Command_Dump_1300, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)toolName_.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-e",
        (char *)STRING_EVENT.c_str(),
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CommonEventCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), "");
}

/**
 * @tc.number: Cem_Command_Dump_1400
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem dump -e <event>" command with a subscriber.
 */
HWTEST_F(CemCommandDumpTest, Cem_Command_Dump_1400, Function | MediumTest | Level1)
{
    /* Subscribe */

    // make matching skills
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(STRING_EVENT);

    // make subscribe info
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    // make a subscriber object
    auto subscriberTestPtr = std::make_shared<CommonEventSubscriberTest>(subscribeInfo);
    // subscribe a common event
    CommonEventManager::SubscribeCommonEvent(subscriberTestPtr);

    char *argv[] = {
        (char *)toolName_.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-e",
        (char *)STRING_EVENT.c_str(),
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CommonEventCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), STRING_EVENT + "\n");
}

/**
 * @tc.number: Cem_Command_Dump_1500
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem dump -u" command.
 */
HWTEST_F(CemCommandDumpTest, Cem_Command_Dump_1500, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)toolName_.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-u",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    CommonEventCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), Concatenate("error: option 'u' requires a value.\n", HELP_MSG_DUMP));
}

/**
 * @tc.number: Cem_Command_Dump_1600
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem dump -u <user-id>" command with no subscriber.
 */
HWTEST_F(CemCommandDumpTest, Cem_Command_Dump_1600, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)toolName_.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-u",
        (char *)"100",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    CommonEventCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), "");
}

/**
 * @tc.number: Cem_Command_Dump_1700
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem dump -e <name> -u <user-id>" command with no subscriber.
 */
HWTEST_F(CemCommandDumpTest, Cem_Command_Dump_1700, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)toolName_.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-e",
        (char *)STRING_EVENT.c_str(),
        (char *)"-u",
        (char *)"100",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    CommonEventCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), "");
}

/**
 * @tc.number: Cem_Command_Dump_1800
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem dump -e <name> -u <user-id>" command with a subscriber.
 */
HWTEST_F(CemCommandDumpTest, Cem_Command_Dump_1800, Function | MediumTest | Level1)
{
    // make matching skills
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(STRING_EVENT);
    // make subscribe info
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    // make a subscriber object
    auto subscriberTestPtr = std::make_shared<CommonEventSubscriberTest>(subscribeInfo);
    // subscribe a common event
    CommonEventManager::SubscribeCommonEvent(subscriberTestPtr);
    char *argv[] = {
        (char *)toolName_.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-e",
        (char *)STRING_EVENT.c_str(),
        (char *)"-u",
        (char *)"100",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    CommonEventCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), STRING_EVENT + "\n");
}

/**
 * @tc.number: Cem_Command_Dump_1900
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem dump -p" command.
 * @tc.require: issueI5UINZ
 */
HWTEST_F(CemCommandDumpTest, Cem_Command_Dump_1900, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)toolName_.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-e",
        (char *)STRING_EVENT.c_str(),
        (char *)"-u",
        (char *)"100",
        (char *)STRING_EVENT.c_str(),
        (char *)"-p",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    CommonEventCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), Concatenate("error: unknown option.\n", HELP_MSG_DUMP));
}

/**
 * @tc.number: Cem_Command_Dump_2000
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem dump -p" command.
 * @tc.require: issueI5UINZ
 */
HWTEST_F(CemCommandDumpTest, Cem_Command_Dump_2000, Function | MediumTest | Level1)
{
      /* Subscribe */

    // make matching skills
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(STRING_EVENT);

    // make subscribe info
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    // make a subscriber object
    auto subscriberTestPtr = std::make_shared<CommonEventSubscriberTest>(subscribeInfo);
    // subscribe a common event
    CommonEventManager::SubscribeCommonEvent(subscriberTestPtr);
    char *argv[] = {
        (char *)toolName_.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-p",
        (char *)"11",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    CommonEventCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), Concatenate("error: option 'p' requires a value.\n", HELP_MSG_DUMP));
}
}  // namespace
