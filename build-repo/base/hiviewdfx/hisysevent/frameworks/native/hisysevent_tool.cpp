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

#include "hisysevent_tool.h"

#include <getopt.h>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <unistd.h>

#include "hisysevent.h"
#include "hisysevent_tool_listener.h"
#include "hisysevent_tool_query.h"

#include "ret_code.h"

using namespace std;

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr char ARG_SELECTION[] = "vrc:o:n:t:lS:s:E:e:m:dhg:";
constexpr uint32_t INVALID_EVENT_TYPE = 0;
constexpr int INVALID_ARG_OPT = -1;
constexpr long long DEFAULT_TIME_STAMP = -1;
constexpr long long SECONDS_2_MILLS = 1000;

RuleType GetRuleTypeFromArg(const string& fromArgs)
{
    static std::map<const string, RuleType> ruleTypeMap {
        { "WHOLE_WORD", RuleType::WHOLE_WORD },
        { "PREFIX", RuleType::PREFIX },
        { "REGULAR", RuleType::REGULAR }
    };
    if (ruleTypeMap.find(fromArgs) != ruleTypeMap.end()) {
        return ruleTypeMap[fromArgs];
    }
    return RuleType::WHOLE_WORD;
}

uint32_t GetEventTypeFromArg(const string& fromArgs)
{
    static std::map<const string, HiSysEvent::EventType> eventTypeMap {
        { "FAULT", HiSysEvent::EventType::FAULT },
        { "STATISTIC", HiSysEvent::EventType::STATISTIC },
        { "SECURITY", HiSysEvent::EventType::SECURITY },
        { "BEHAVIOR", HiSysEvent::EventType::BEHAVIOR }
    };
    if (eventTypeMap.find(fromArgs) != eventTypeMap.end()) {
        return static_cast<uint32_t>(eventTypeMap[fromArgs]);
    }
    return INVALID_EVENT_TYPE;
}

long long ParseTimeStampFromArgs(const string& fromArgs)
{
    regex formatRegex("[0-9]{4}-"
        "((0[13578]|1[02])-(0[1-9]|[12][0-9]|3[01])|(0[2469]|11)-(0[1-9]|[12][0-9]|30))"
        " ([01][0-9]|2[0-3])(:[0-5][0-9]){2}");
    smatch matchRet;
    if (!std::regex_match(fromArgs, matchRet, formatRegex)) {
        return DEFAULT_TIME_STAMP;
    }
    std::istringstream is(fromArgs);
    struct tm time = {};
    is >> std::get_time(&time, "%Y-%m-%d %H:%M:%S");
    return static_cast<long long>(mktime(&time)) * SECONDS_2_MILLS;
}

std::string GetErrorDescription(int32_t errCode)
{
    std::map<int32_t, std::string> errMap = {
        { ERR_SYS_EVENT_SERVICE_NOT_FOUND, "service not found." },
        { ERR_PARCEL_DATA_IS_NULL, "parcel data is null." },
        { ERR_REMOTE_SERVICE_IS_NULL, "remote service is null." },
        { ERR_CAN_NOT_WRITE_DESCRIPTOR, "descriptor wrote failed." },
        { ERR_CAN_NOT_WRITE_PARCEL, "parcel wrote failed." },
        { ERR_CAN_NOT_WRITE_REMOTE_OBJECT, "remote object wrote failed." },
        { ERR_CAN_NOT_SEND_REQ, "request sent failed." },
        { ERR_CAN_NOT_READ_PARCEL, "parcel read failed." },
        { ERR_ADD_DEATH_RECIPIENT, "add death recipient failed." },
        { ERR_QUERY_RULE_INVALID, "invalid query rule." },
        { ERR_TOO_MANY_WATCHERS, "too many wathers subscribed." },
        { ERR_QUERY_TOO_FREQUENTLY, "query too frequently." },
    };
    return errMap.find(errCode) == errMap.end() ?
        "unknown error." : errMap.at(errCode);
}

void InitOptHandlers(std::map<int, OptHandler>& optHandlers)
{
    std::map<int, OptHandler> tmpHandlers = {
        {'v', [] (struct ArgStuct& cmdArg, const char* optarg) {
            cmdArg.checkValidEvent = true;
        }}, {'r', [] (struct ArgStuct& cmdArg, const char* optarg) {
            cmdArg.real = true;
        }}, {'c', [] (struct ArgStuct& cmdArg, const char* optarg) {
            cmdArg.ruleType = GetRuleTypeFromArg(optarg);
        }}, {'o', [] (struct ArgStuct& cmdArg, const char* optarg) {
            cmdArg.domain = optarg;
        }}, {'n', [] (struct ArgStuct& cmdArg, const char* optarg) {
            cmdArg.eventName = optarg;
        }}, {'t', [] (struct ArgStuct& cmdArg, const char* optarg) {
            cmdArg.tag = optarg;
        }}, {'l', [] (struct ArgStuct& cmdArg, const char* optarg) {
            cmdArg.history = true;
        }}, {'s', [] (struct ArgStuct& cmdArg, const char* optarg) {
            cmdArg.beginTime = strtoll(optarg, nullptr, 0);
        }}, {'S', [] (struct ArgStuct& cmdArg, const char* optarg) {
            cmdArg.beginTime = ParseTimeStampFromArgs(std::string(optarg));
        }}, {'e', [] (struct ArgStuct& cmdArg, const char* optarg) {
            cmdArg.endTime = strtoll(optarg, nullptr, 0);
        }}, {'E', [] (struct ArgStuct& cmdArg, const char* optarg) {
            cmdArg.endTime = ParseTimeStampFromArgs(std::string(optarg));
        }}, {'m', [] (struct ArgStuct& cmdArg, const char* optarg) {
            cmdArg.maxEvents = strtol(optarg, nullptr, 0);
        }}, {'d', [] (struct ArgStuct& cmdArg, const char* optarg) {
            cmdArg.isDebug = true;
        }}, {'g', [] (struct ArgStuct& cmdArg, const char* optarg) {
            cmdArg.eventType = GetEventTypeFromArg(optarg);
        }},
    };
    optHandlers.insert(tmpHandlers.begin(), tmpHandlers.end());
}
}

HiSysEventTool::HiSysEventTool(bool autoExit) : clientCmdArg {
    false, false, "", "", "", RuleType::WHOLE_WORD,
    false, false, -1, -1, 10000, 0}, autoExit(autoExit)
{
    InitOptHandlers(optHandlers);
}

bool HiSysEventTool::ParseCmdLine(int argc, char** argv)
{
    if (argv == nullptr) {
        return false;
    }
    if (argc > 1) {
        HandleInput(argc, argv, ARG_SELECTION);
    }
    return CheckCmdLine();
}

bool HiSysEventTool::CheckCmdLine()
{
    if (!clientCmdArg.real && !clientCmdArg.history) {
        return false;
    }

    if (clientCmdArg.real && clientCmdArg.history) {
        cout << "canot read both read && history hisysevent" << endl;
        return false;
    }

    if (clientCmdArg.isDebug && !clientCmdArg.real) {
        cout << "debug must follow with real log" << endl;
        return false;
    }

    if (clientCmdArg.history) {
        auto timestampValidCheck = clientCmdArg.endTime > 0
            && clientCmdArg.beginTime > clientCmdArg.endTime;
        if (timestampValidCheck) {
            cout << "invalid time startTime must less than endTime(";
            cout << clientCmdArg.beginTime << " > " << clientCmdArg.endTime << ")." << endl;
            return false;
        }
    }
    return true;
}

void HiSysEventTool::HandleInput(int argc, char** argv, const char* selection)
{
    int opt;
    while ((opt = getopt(argc, argv, selection)) != INVALID_ARG_OPT) {
        if (opt == 'h') {
            DoCmdHelp();
            if (autoExit) {
                _exit(0);
            }
        }
        if (optHandlers.find(opt) != optHandlers.end()) {
            optHandlers.at(opt)(clientCmdArg, optarg);
        }
    }
}

void HiSysEventTool::DoCmdHelp()
{
    cout << "hisysevent [[-v] -r [-d | -c [WHOLE_WORD|PREFIX|REGULAR] -t <tag> "
        << "| -c [WHOLE_WORD|PREFIX|REGULAR] -o <domain> -n <eventName> "
        << "| -g [FAULT|STATISTIC|SECURITY|BEHAVIOR]] "
        << "| -l [[-s <begin time> -e <end time> | -S <formatted begin time> -E <formatted end time>] "
        << "-m <count> -c [WHOLE_WORD] -o <domain> -n <eventName> -g [FAULT|STATISTIC|SECURITY|BEHAVIOR]]]" << endl;
    cout << "-r,    subscribe on all domains, event names and tags." << endl;
    cout << "-r -c [WHOLE_WORD|PREFIX|REGULAR] -t <tag>"
        << ", subscribe on tag." << endl;
    cout << "-r -c [WHOLE_WORD|PREFIX|REGULAR] -o <domain> -n <eventName>"
        << ", subscribe on domain and event name." << endl;
    cout << "-r -g [FAULT|STATISTIC|SECURITY|BEHAVIOR]"
        << ", subscribe on event type." << endl;
    cout << "-r -d set debug mode, both options must appear at the same time." << endl;
    cout << "-l -s <begin time> -e <end time> -m <max hisysevent count>"
        << ", get history hisysevent log with time stamps, end time should not be "
        << "earlier than begin time." << endl;
    cout << "-l -S <formatted begin time> -E <formatted end time> -m <max hisysevent count>"
        << ", get history hisysevent log with formatted time string, end time should not be "
        << "earlier than begin time." << endl;
    cout << "-l -c [WHOLE_WORD] -o <domain> -n <eventName> -m <max hisysevent count>"
        << ", get history hisysevent log with domain and event name." << endl;
    cout << "-l -g [FAULT|STATISTIC|SECURITY|BEHAVIOR] -m <max hisysevent count>"
        << ", get history hisysevent log with event type." << endl;
    cout << "-v,    open valid event checking mode." << endl;
    cout << "-h,    help manual." << endl;
}

bool HiSysEventTool::DoAction()
{
    if (clientCmdArg.real) {
        auto toolListener = std::make_shared<HiSysEventToolListener>(clientCmdArg.checkValidEvent);
        if (toolListener == nullptr) {
            return false;
        }
        std::vector<ListenerRule> sysRules;
        ListenerRule listenerRule(clientCmdArg.domain, clientCmdArg.eventName,
            clientCmdArg.tag, clientCmdArg.ruleType, clientCmdArg.eventType);
        sysRules.emplace_back(listenerRule);
        auto retCode = HiSysEventManager::AddListener(toolListener, sysRules);
        if (retCode != IPC_CALL_SUCCEED ||
            (clientCmdArg.isDebug && HiSysEventManager::SetDebugMode(toolListener, true) != 0)) {
            cout << "failed to subscribe system event: " << GetErrorDescription(retCode) << endl;
        }
        return true;
    }

    if (clientCmdArg.history) {
        auto queryCallBack = std::make_shared<HiSysEventToolQuery>(clientCmdArg.checkValidEvent, autoExit);
        if (queryCallBack == nullptr) {
            return false;
        }
        struct QueryArg args(clientCmdArg.beginTime, clientCmdArg.endTime, clientCmdArg.maxEvents);
        std::vector<QueryRule> queryRules;
        if (clientCmdArg.ruleType != RuleType::WHOLE_WORD) {
            cout << "only \"-c WHOLE_WORD\" supported with \"hisysevent -l\" cmd." << endl;
            return false;
        }
        if (!clientCmdArg.domain.empty() || !clientCmdArg.eventName.empty() ||
            clientCmdArg.eventType != INVALID_EVENT_TYPE) {
            QueryRule rule(clientCmdArg.domain, { clientCmdArg.eventName },
                clientCmdArg.ruleType, clientCmdArg.eventType);
            queryRules.push_back(rule);
        }
        auto retCode = HiSysEventManager::Query(args, queryRules, queryCallBack);
        if (retCode != IPC_CALL_SUCCEED) {
            cout << "failed to query system event: " << GetErrorDescription(retCode) << endl;
        }
        return true;
    }
    return false;
}

void HiSysEventTool::WaitClient()
{
    unique_lock<mutex> lock(mutexClient);
    condvClient.wait(lock);
}

void HiSysEventTool::NotifyClient()
{
    condvClient.notify_one();
}
} // namespace HiviewDFX
} // namespace OHOS
