/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef BASE_NOTIFICATION_CES_STANDARD_CESFWK_TOOLS_CEM_INCLUDE_SHELL_COMMAND_H
#define BASE_NOTIFICATION_CES_STANDARD_CESFWK_TOOLS_CEM_INCLUDE_SHELL_COMMAND_H

#include <functional>
#include <map>
#include <string>
#include <vector>

#include "errors.h"

namespace OHOS {
namespace EventFwk {
namespace {
constexpr char HELP_MSG_NO_OPTION[] = "error: you must specify an option at least.\n";
constexpr int MIN_ARGUMENT_NUMBER = 2;
}  // namespace

class ShellCommand {
public:
    /**
     * Constructor.
     *
     * @param argc Indicates the argument count.
     * @param argv Indicates the argument values.
     * @param name Indicates the tool name.
     */
    ShellCommand(int argc, char *argv[], std::string name);

    virtual ~ShellCommand();

    /**
     * Processes the command.
     *
     * @return Returns result code.
     */
    ErrCode OnCommand();

    /**
     * Executes the command.
     *
     * @return Returns result.
     */
    std::string ExecCommand();

    /**
     * Gets the error message of the command.
     *
     * @return Returns the error message of the command.
     */
    std::string GetCommandErrorMsg() const;

    /**
     * Creates the command map.
     *
     * @return Returns result code.
     */
    virtual ErrCode CreateCommandMap() = 0;

    /**
     * Inits.
     *
     * @return Returns result code.
     */
    virtual ErrCode Init() = 0;

protected:
    int argc_;
    char **argv_;
    std::string cmd_;
    std::vector<std::string> argList_;
    std::string name_;
    std::map<std::string, std::function<int()>> commandMap_;
    std::string resultReceiver_;
};
}  // namespace EventFwk
}  // namespace OHOS
#endif  // BASE_NOTIFICATION_CES_STANDARD_CESFWK_TOOLS_CEM_INCLUDE_SHELL_COMMAND_H