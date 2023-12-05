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

#include "hisysevent_manager_c.h"

#include <string>

#include "hisysevent_base_manager.h"
#include "hisysevent_query_callback_c.h"
#include "ret_code.h"

namespace {
using OHOS::HiviewDFX::HiSysEventBaseManager;
using OHOS::HiviewDFX::HiSysEventBaseQueryCallback;
using QueryArgCls = OHOS::HiviewDFX::QueryArg;
using QueryRuleCls = OHOS::HiviewDFX::QueryRule;
using OHOS::HiviewDFX::RuleType::WHOLE_WORD;
using OHOS::HiviewDFX::ERR_QUERY_RULE_INVALID;

int HiSysEventQuery(const HiSysEventQueryArg& arg, HiSysEventQueryRule rules[], size_t ruleSize,
    HiSysEventQueryCallback& callback)
{
    std::vector<QueryRuleCls> queryRules;
    for (size_t i = 0; i < ruleSize; ++i) {
        if (strlen(rules[i].domain) == 0 || rules[i].eventListSize == 0) {
            return ERR_QUERY_RULE_INVALID;
        }
        std::vector<std::string> eventList;
        for (size_t j = 0; j < rules[i].eventListSize; ++j) {
            eventList.emplace_back(rules[i].eventList[j]);
        }
        std::string cond = rules[i].condition == nullptr ? "" : rules[i].condition;
        queryRules.emplace_back(rules[i].domain, eventList, WHOLE_WORD, 0, cond);
    }
    QueryArgCls argCls(arg.beginTime, arg.endTime, arg.maxEvents);
    auto callbackC = std::make_shared<HiSysEventQueryCallbackC>(callback.OnQuery, callback.OnComplete);
    return HiSysEventBaseManager::Query(argCls, queryRules, std::make_shared<HiSysEventBaseQueryCallback>(callbackC));
}
}

#ifdef __cplusplus
extern "C" {
#endif

int OH_HiSysEvent_Query(const HiSysEventQueryArg& arg, HiSysEventQueryRule rules[], size_t ruleSize,
    HiSysEventQueryCallback& callback)
{
    return HiSysEventQuery(arg, rules, ruleSize, callback);
}

#ifdef __cplusplus
}
#endif
