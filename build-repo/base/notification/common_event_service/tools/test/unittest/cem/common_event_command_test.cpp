/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::EventFwk;

static std::string Concatenate(const std::string &first,  const std::string &second)
{
    return first + second;
}

class CemCommandTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    std::string toolName_ = TOOL_NAME;
};

void CemCommandTest::SetUpTestCase()
{}

void CemCommandTest::TearDownTestCase()
{}

void CemCommandTest::SetUp()
{
    // reset optind to 0
    optind = 0;
}

void CemCommandTest::TearDown()
{}

/**
 * @tc.number: Cem_Command_0100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem" command.
 */
HWTEST_F(CemCommandTest, Cem_Command_0100, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)toolName_.c_str(),
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CommonEventCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG);
}

/**
 * @tc.number: Cem_Command_0200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem xxx" command.
 */
HWTEST_F(CemCommandTest, Cem_Command_0200, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)toolName_.c_str(),
        (char *)"xxx",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CommonEventCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), Concatenate("cem: 'xxx' is not a valid cem command. See 'cem help'.\n", HELP_MSG));
}

/**
 * @tc.number: Cem_Command_0300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem -x" command.
 */
HWTEST_F(CemCommandTest, Cem_Command_0300, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)toolName_.c_str(),
        (char *)"-x",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CommonEventCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), cmd.GetCommandErrorMsg() + HELP_MSG);
}

/**
 * @tc.number: Cem_Command_0400
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem -xxx" command.
 */
HWTEST_F(CemCommandTest, Cem_Command_0400, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)toolName_.c_str(),
        (char *)"-xxx",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CommonEventCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), Concatenate(cmd.GetCommandErrorMsg(), HELP_MSG));
}

/**
 * @tc.number: Cem_Command_0500
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem help" command.
 */
HWTEST_F(CemCommandTest, Cem_Command_0500, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)toolName_.c_str(),
        (char *)"help",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CommonEventCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG);
}

/**
 * @tc.number: Cem_Command_0600
 * @tc.name: Init
 * @tc.desc: Verify the "cem help" command.
 * @tc.require: issueI5UINZ
 */
HWTEST_F(CemCommandTest, Cem_Command_0600, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)toolName_.c_str(),
        (char *)"-xxx",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CommonEventCommand cmd(argc, argv);
    std::shared_ptr<CommonEvent> commonEventPtr_ = std::make_shared<CommonEvent>();
    EXPECT_EQ(cmd.Init(), (int)ERR_OK);
}

/**
 * @tc.number: Cem_Command_0600
 * @tc.name: Init
 * @tc.desc: Verify the "cem help" command.
 * @tc.require: issueI5UINZ
 */
HWTEST_F(CemCommandTest, Cem_Command_0700, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)toolName_.c_str(),
        (char *)"-xxx",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CommonEventCommand cmd(argc, argv);
    std::shared_ptr<CommonEvent> commonEventPtr_ = nullptr;
    EXPECT_EQ(cmd.Init(), (int)ERR_OK);
}

