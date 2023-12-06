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

#include "common_event_command.h"

#include <getopt.h>

#include "common_event.h"
#include "common_event_constant.h"
#include "common_event_manager.h"
#include "event_log_wrapper.h"
#include "singleton.h"

namespace OHOS {
namespace EventFwk {
namespace {
const std::string SHORT_OPTIONS = "he:asoc:d:u:p:";
const struct option LONG_OPTIONS[] = {
    {"help", no_argument, nullptr, 'h'},
    {"all", no_argument, nullptr, 'a'},
    {"event", required_argument, nullptr, 'e'},
    {"sticky", no_argument, nullptr, 's'},
    {"ordered", no_argument, nullptr, 'o'},
    {"code", required_argument, nullptr, 'c'},
    {"data", required_argument, nullptr, 'd'},
    {"user-id", required_argument, nullptr, 'u'},
};
}  // namespace

ErrCode CommonEventCommand::CreateCommandMap()
{
    commandMap_ = {
        {"help", std::bind(&CommonEventCommand::RunAsHelpCommand, this)},
        {"publish", std::bind(&CommonEventCommand::RunAsPublishCommand, this)},
        {"dump", std::bind(&CommonEventCommand::RunAsDumpCommand, this)},
    };
    return ERR_OK;
}

ErrCode CommonEventCommand::Init()
{
    EVENT_LOGI("enter");
    if (!commonEventPtr_) {
        commonEventPtr_ = DelayedSingleton<CommonEvent>::GetInstance();
    }
    if (!commonEventPtr_) {
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

ErrCode CommonEventCommand::RunAsHelpCommand()
{
    EVENT_LOGI("enter");
    resultReceiver_.append(HELP_MSG);
    return ERR_OK;
}

ErrCode CommonEventCommand::RunAsPublishCommand()
{
    EVENT_LOGI("enter");
    ErrCode result = ERR_OK;
    PublishCmdInfo cmdInfo;
    bool hasOption = false;
    SetPublishCmdInfo(cmdInfo, result, hasOption);
    if (!hasOption) {
        resultReceiver_.append(HELP_MSG_NO_OPTION);
        resultReceiver_.append(HELP_MSG_PUBLISH);
        return ERR_INVALID_VALUE;
    }
    if (result == ERR_OK && resultReceiver_.empty() && cmdInfo.action.empty()) {
        resultReceiver_.append(HELP_MSG_NO_EVENT_OPTION);
        result = ERR_INVALID_VALUE;
    }
    if (result == ERR_OK) {
        Want want;
        want.SetAction(cmdInfo.action);
        CommonEventData commonEventData;
        commonEventData.SetWant(want);
        commonEventData.SetCode(cmdInfo.code);
        commonEventData.SetData(cmdInfo.data);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(cmdInfo.isSticky);
        publishInfo.SetOrdered(cmdInfo.isOrdered);
        // publish the common event
        int32_t publishResult = commonEventPtr_->PublishCommonEventAsUser(commonEventData, publishInfo, nullptr,
            cmdInfo.userId);
        if (publishResult == ERR_OK) {
            resultReceiver_ = STRING_PUBLISH_COMMON_EVENT_OK;
        } else {
            resultReceiver_ = STRING_PUBLISH_COMMON_EVENT_NG;
        }
    } else {
        resultReceiver_.append(HELP_MSG_PUBLISH);
    }
    return result;
}

void CommonEventCommand::SetPublishCmdInfo(PublishCmdInfo &cmdInfo, ErrCode &result, bool &hasOption)
{
    int option;
    while ((option = getopt_long(argc_, argv_, SHORT_OPTIONS.c_str(), LONG_OPTIONS, nullptr)) != -1) {
        hasOption = true;
        if (option == '?') {
            CheckPublishOpt();
            result = ERR_INVALID_VALUE;
            return;
        }
        switch (option) {
            case 'h':
                result = ERR_INVALID_VALUE;
                break;
            case 'e':
                cmdInfo.action = optarg;
                break;
            case 's':
                cmdInfo.isSticky = true;
                break;
            case 'o':
                cmdInfo.isOrdered = true;
                break;
            case 'c':
                cmdInfo.code = atoi(optarg);
                break;
            case 'd':
                cmdInfo.data = optarg;
                break;
            case 'u':
                cmdInfo.userId = atoi(optarg);
                break;
            default:
                break;
        }
    }
}

void CommonEventCommand::CheckPublishOpt()
{
    switch (optopt) {
        case 'e': {
            resultReceiver_.append("error: option 'e' requires a value.\n");
            break;
        }
        case 'c': {
            resultReceiver_.append("error: option 'c' requires a value.\n");
            break;
        }
        case 'd': {
            resultReceiver_.append("error: option 'd' requires a value.\n");
            break;
        }
        case 'u': {
            resultReceiver_.append("error: option 'u' requires a value.\n");
            break;
        }
        default: {
            resultReceiver_.append("error: unknown option.\n");
            break;
        }
    }
}

ErrCode CommonEventCommand::RunAsDumpCommand()
{
    EVENT_LOGI("enter");
    ErrCode result = ERR_OK;
    bool hasOption = false;
    DumpCmdInfo cmdInfo;
    SetDumpCmdInfo(cmdInfo, result, hasOption);
    if (!hasOption) {
        resultReceiver_.append(HELP_MSG_NO_OPTION);
        resultReceiver_.append(HELP_MSG_DUMP);
        return ERR_INVALID_VALUE;
    }
    if (result == ERR_OK) {
        std::vector<std::string> dumpResults;
        bool dumpResult = commonEventPtr_->DumpState(static_cast<int32_t>(cmdInfo.eventType),
            cmdInfo.action, cmdInfo.userId, dumpResults);
        if (dumpResult) {
            for (const auto &it : dumpResults) {
                resultReceiver_.append(it + "\n");
            }
        } else {
            resultReceiver_ = STRING_DUMP_COMMON_EVENT_NG;
        }
    } else {
        resultReceiver_.append(HELP_MSG_DUMP);
    }
    return result;
}

void CommonEventCommand::SetDumpCmdInfo(DumpCmdInfo &cmdInfo, ErrCode &result, bool &hasOption)
{
    int option;
    while ((option = getopt_long(argc_, argv_, SHORT_OPTIONS.c_str(), LONG_OPTIONS, nullptr)) != -1) {
        hasOption = true;
        if (option == '?') {
            CheckDumpOpt();
            result = ERR_INVALID_VALUE;
            return;
        }
        switch (option) {
            case 'h':
                result = ERR_INVALID_VALUE;
                break;
            case 'e':
                cmdInfo.action = optarg;
                break;
            case 'u':
                cmdInfo.userId = atoi(optarg);
                break;
            case 'p':
                CheckDumpEventType(cmdInfo, result);
                break;
            default:
                break;
        }
    }
}

void CommonEventCommand::CheckDumpOpt()
{
    switch (optopt) {
        case 'e':
            resultReceiver_.append("error: option 'e' requires a value.\n");
            break;
        case 'u':
            resultReceiver_.append("error: option 'u' requires a value.\n");
            break;
        default:
            resultReceiver_.append("error: unknown option.\n");
            break;
    }
}

void CommonEventCommand::CheckDumpEventType(DumpCmdInfo &cmdInfo, ErrCode &result)
{
    if (strcmp(optarg, "subscriber") == 0) {
        cmdInfo.eventType = DumpEventType::SUBSCRIBER;
    } else if (strcmp(optarg, "sticky") == 0) {
        cmdInfo.eventType = DumpEventType::STICKY;
    } else if (strcmp(optarg, "pending") == 0) {
        cmdInfo.eventType = DumpEventType::PENDING;
    } else if (strcmp(optarg, "history") == 0) {
        cmdInfo.eventType = DumpEventType::HISTORY;
    } else {
        resultReceiver_.append("error: option 'p' requires a value.\n");
        result = ERR_INVALID_VALUE;
    }
}
}  // namespace EventFwk
}  // namespace OHOS
