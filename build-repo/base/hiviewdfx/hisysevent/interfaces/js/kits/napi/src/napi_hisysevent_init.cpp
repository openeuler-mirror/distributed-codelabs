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

#include "napi_hisysevent_init.h"

#include <map>

#include "hilog/log.h"
#include "hisysevent.h"
#include "rule_type.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr char EVENT_TYPE_ENUM_NAME[] = "EventType";
constexpr char RULE_TYPE_ENUM_NAME[] = "RuleType";

napi_value ClassConstructor(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value argv = nullptr;
    napi_value thisArg = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, &argc, &argv, &thisArg, &data);

    napi_value global = 0;
    napi_get_global(env, &global);

    return thisArg;
}

void InitEventTypeEnum(napi_env env,
    std::map<const char*, napi_value>& eventTypeMap)
{
    napi_value fault = nullptr;
    napi_create_int32(env, HiSysEvent::FAULT, &fault);
    napi_value statistic = nullptr;
    napi_create_int32(env, HiSysEvent::STATISTIC, &statistic);
    napi_value security = nullptr;
    napi_create_int32(env, HiSysEvent::SECURITY, &security);
    napi_value behavior = nullptr;
    napi_create_int32(env, HiSysEvent::BEHAVIOR, &behavior);

    eventTypeMap["FAULT"] = fault;
    eventTypeMap["STATISTIC"] = statistic;
    eventTypeMap["SECURITY"] = security;
    eventTypeMap["BEHAVIOR"] = behavior;
}

void InitRuleTypeEnum(napi_env env,
    std::map<const char*, napi_value>& ruleTypeMap)
{
    napi_value wholeWord = nullptr;
    napi_create_int32(env, WHOLE_WORD, &wholeWord);
    napi_value prefix = nullptr;
    napi_create_int32(env, PREFIX, &prefix);
    napi_value regular = nullptr;
    napi_create_int32(env, REGULAR, &regular);

    ruleTypeMap["WHOLE_WORD"] = wholeWord;
    ruleTypeMap["PREFIX"] = prefix;
    ruleTypeMap["REGULAR"] = regular;
}

void InitConstClassByName(napi_env env, napi_value exports, std::string name)
{
    std::map<const char*, napi_value> propertyMap;
    if (name == EVENT_TYPE_ENUM_NAME) {
        InitEventTypeEnum(env, propertyMap);
    } else if (name == RULE_TYPE_ENUM_NAME) {
        InitRuleTypeEnum(env, propertyMap);
    } else {
        return;
    }
    int i = 0;
    napi_property_descriptor descriptors[propertyMap.size()];
    for (auto& it : propertyMap) {
        descriptors[i++] = DECLARE_NAPI_STATIC_PROPERTY(it.first, it.second);
    }
    napi_value result = nullptr;
    napi_define_class(env, name.c_str(), NAPI_AUTO_LENGTH, ClassConstructor,
        nullptr, sizeof(descriptors) / sizeof(*descriptors), descriptors, &result);
    napi_set_named_property(env, exports, name.c_str(), result);
}
}
napi_value InitNapiClass(napi_env env, napi_value exports)
{
    InitConstClassByName(env, exports, EVENT_TYPE_ENUM_NAME);
    InitConstClassByName(env, exports, RULE_TYPE_ENUM_NAME);
    return exports;
}
} // namespace HiviewDFX
} // namespace OHOS