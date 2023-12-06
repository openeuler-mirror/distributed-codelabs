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

#ifndef BASE_NOTIFICATION_CES_STANDARD_CESFWK_TOOLS_CEM_INCLUDE_COMMON_EVENT_COMMAND_H
#define BASE_NOTIFICATION_CES_STANDARD_CESFWK_TOOLS_CEM_INCLUDE_COMMON_EVENT_COMMAND_H

#include "common_event.h"
#include "common_event_constant.h"
#include "shell_command.h"

namespace OHOS {
namespace EventFwk {
namespace {
constexpr char TOOL_NAME[] = "cem";
constexpr char HELP_MSG[] =
    "usage: cem <command> [<options>]\n"
    "These are common cem commands list:\n"
    "  help                         list available commands\n"
    "  publish                      publish a common event with options\n"
    "  dump                         dump the info of events\n";

constexpr char HELP_MSG_PUBLISH[] =
    "usage: cem publish [<options>]\n"
    "options list:\n"
    "  -h, --help                                       list available commands\n"
    "  -e, --event <name> [-s, --sticky] [-o, --ordered] [-c, --code <code>] [-d, --data <data>]\n"
    "                                                   publish a common event\n";

constexpr char HELP_MSG_DUMP[] =
    "usage: cem dump [<options>]\n"
    "options list:\n"
    "  -h, --help                   list available commands\n"
    "  -a, --all                    dump the info of all events\n"
    "  -e, --event <name>           dump the info filter by the specified event\n"
    "  -u, --user-id <userId>       dump the info filter by the specified userId\n"
    "  -p, --part <name>            dump the info of part events\n"
    "       subscriber              all subscribers\n"
    "       sticky                  sticky events\n"
    "       pending                 pending events\n"
    "       history                 history events\n";

constexpr char HELP_MSG_NO_EVENT_OPTION[] = "error: you must specify an event name with '-e' or '--event'.\n";
constexpr char STRING_PUBLISH_COMMON_EVENT_OK[] = "publish the common event successfully.\n";
constexpr char STRING_PUBLISH_COMMON_EVENT_NG[] = "error: failed to publish the common event.\n";
constexpr char STRING_DUMP_COMMON_EVENT_NG[] = "error: failed to dump the common event(s).\n";
}  // namespace

struct PublishCmdInfo {
    bool isSticky = false;
    bool isOrdered = false;
    int32_t code = 0;
    int32_t userId = ALL_USER;
    std::string action;
    std::string data;
};

struct DumpCmdInfo {
    std::string action;
    int32_t userId = ALL_USER;
    DumpEventType eventType = DumpEventType::ALL;
};

class CommonEventCommand : public OHOS::EventFwk::ShellCommand {
public:
    CommonEventCommand(int argc, char *argv[]) : ShellCommand(argc, argv, TOOL_NAME) {}
    ~CommonEventCommand() override = default;

private:
    ErrCode CreateCommandMap() override;
    ErrCode Init() override;
    ErrCode RunAsHelpCommand();
    ErrCode RunAsPublishCommand();
    ErrCode RunAsDumpCommand();
    void CheckPublishOpt();
    void SetPublishCmdInfo(PublishCmdInfo &cmdInfo, ErrCode &result, bool &hasOption);
    void CheckDumpOpt();
    void SetDumpCmdInfo(DumpCmdInfo &cmdInfo, ErrCode &result, bool &hasOption);
    void CheckDumpEventType(DumpCmdInfo &cmdInfo, ErrCode &result);

    std::shared_ptr<CommonEvent> commonEventPtr_ = nullptr;
};
}  // namespace EventFwk
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_CES_STANDARD_CESFWK_TOOLS_CEM_INCLUDE_COMMON_EVENT_COMMAND_H
