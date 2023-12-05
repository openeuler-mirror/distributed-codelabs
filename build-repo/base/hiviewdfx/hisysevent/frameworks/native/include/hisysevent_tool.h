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

#ifndef HISYSEVENT_TOOL_H
#define HISYSEVENT_TOOL_H

#include <functional>
#include <map>
#include <string>
#include <thread>

#include "hisysevent_manager.h"

namespace OHOS {
namespace HiviewDFX {
struct ArgStuct {
    bool real;
    bool checkValidEvent;
    std::string domain;
    std::string eventName;
    std::string tag;
    RuleType ruleType;
    bool history;
    bool isDebug;
    long long beginTime;
    long long endTime;
    int maxEvents;
    uint32_t eventType;
};

using OptHandler = std::function<void(struct ArgStuct&, const char*)>;

class HiSysEventTool {
public:
    HiSysEventTool(bool autoExit = true);
    ~HiSysEventTool() {}

public:
    void DoCmdHelp();
    bool DoAction();
    void NotifyClient();
    bool ParseCmdLine(int argc, char** argv);
    void WaitClient();

private:
    bool CheckCmdLine();
    void HandleInput(int argc, char** argv, const char* selection);

private:
    struct ArgStuct clientCmdArg;
    bool autoExit = true;
    std::condition_variable condvClient;
    std::mutex mutexClient;
    std::map<int, OptHandler> optHandlers;
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // HISYSEVENT_TOOL_H
