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

#include "napi_hisysevent_util.h"

#include <cinttypes>

#include "def.h"
#include "hilog/log.h"
#include "json/json.h"
#include "ret_code.h"
#include "ret_def.h"
#include "stringfilter.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, 0xD002D08, "NAPI_HISYSEVENT_UTIL" };
constexpr uint32_t JS_STR_PARM_LEN_LIMIT = 1024 * 10; // 10k
constexpr uint32_t BUF_SIZE = 1024 * 11; // 11k
constexpr int SYS_EVENT_INFO_PARAM_INDEX = 0;
constexpr long long DEFAULT_TIME_STAMP = -1;
constexpr long long DEFAULT_SEQ = 0;
constexpr int32_t DEFAULT_MAX_EVENTS = 1000;
constexpr char PARAMS_ATTR[] = "params";
constexpr char TAG_ATTR[] = "tag";
constexpr char RULE_TYPE_ATTR[] = "ruleType";
constexpr char BEGIN_TIME_ATTR[] = "beginTime";
constexpr char END_TIME_ATTR[] = "endTime";
constexpr char MAX_EVENTS_ATTR[] = "maxEvents";
constexpr char BEGIN_SEQ_ATTR[] = "fromSeq";
constexpr char END_SEQ_ATTR[] = "toSeq";
constexpr char NAMES_ATTR[] = "names";
constexpr char DOMAIN__KEY[] = "domain_";
constexpr char NAME__KEY[] = "name_";
constexpr char TYPE__KEY[] = "type_";
const std::string INVALID_KEY_TYPE_ARR[] = {
    "[object Object]",
    "null",
    "()",
    ","
};

bool CheckKeyTypeString(const std::string& str)
{
    bool ret = true;
    for (auto invalidType : INVALID_KEY_TYPE_ARR) {
        if (str.find(invalidType) != std::string::npos) {
            ret = false;
            break;
        }
    }
    return ret;
}

napi_valuetype GetValueType(const napi_env env, const napi_value& value)
{
    napi_valuetype valueType = napi_undefined;
    napi_status ret = napi_typeof(env, value, &valueType);
    if (ret != napi_ok) {
        HiLog::Error(LABEL, "failed to parse the type of napi value.");
    }
    return valueType;
}

bool IsValueTypeValid(const napi_env env, const napi_value& jsObj,
    const napi_valuetype typeName)
{
    napi_valuetype valueType = GetValueType(env, jsObj);
    if (valueType != typeName) {
        HiLog::Error(LABEL, "napi value type not match: valueType=%{public}d, typeName=%{public}d.",
            valueType, typeName);
        return false;
    }
    return true;
}

bool CheckValueIsArray(const napi_env env, const napi_value& jsObj)
{
    if (!IsValueTypeValid(env, jsObj, napi_valuetype::napi_object)) {
        return false;
    }
    bool isArray = false;
    napi_status ret = napi_is_array(env, jsObj, &isArray);
    if (ret != napi_ok) {
        HiLog::Error(LABEL, "failed to check array napi value.");
    }
    return isArray;
}

bool ParseBoolValue(const napi_env env, const napi_value& value, bool defalutValue = false)
{
    bool boolValue = defalutValue;
    napi_status ret = napi_get_value_bool(env, value, &boolValue);
    if (ret != napi_ok) {
        HiLog::Error(LABEL, "failed to parse napi value of boolean type.");
    }
    return boolValue;
}

double ParseNumberValue(const napi_env env, const napi_value& value, double defaultValue = 0.0)
{
    double numValue = defaultValue;
    napi_status ret = napi_get_value_double(env, value, &numValue);
    if (ret != napi_ok) {
        HiLog::Error(LABEL, "failed to parse napi value of number type.");
    }
    return numValue;
}

double ParseBigIntValue(const napi_env env, const napi_value& value, uint64_t defaultValue = 0)
{
    uint64_t bigIntValue = defaultValue;
    bool lossless = true;
    napi_status ret = napi_get_value_bigint_uint64(env, value, &bigIntValue, &lossless);
    if (ret != napi_ok) {
        HiLog::Error(LABEL, "failed to parse napi value of big int type.");
    }
    return static_cast<double>(bigIntValue);
}

std::string ParseStringValue(const napi_env env, const napi_value& value, std::string defaultValue = "")
{
    char buf[BUF_SIZE] = {0};
    size_t bufLength = 0;
    napi_status status = napi_get_value_string_utf8(env, value, buf, BUF_SIZE - 1, &bufLength);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "failed to parse napi value of string type.");
        return defaultValue;
    }
    std::string dest = std::string {buf};
    return dest;
}

std::string GetTagAttribute(const napi_env env, const napi_value& object, std::string defaultValue = "")
{
    napi_value propertyValue = NapiHiSysEventUtil::GetPropertyByName(env, object, TAG_ATTR);
    if (IsValueTypeValid(env, propertyValue, napi_valuetype::napi_null) ||
        IsValueTypeValid(env, propertyValue, napi_valuetype::napi_undefined)) {
        return defaultValue;
    }
    if (IsValueTypeValid(env, propertyValue, napi_valuetype::napi_string)) {
        return ParseStringValue(env, propertyValue, defaultValue);
    }
    NapiHiSysEventUtil::ThrowParamTypeError(env, TAG_ATTR, "string");
    HiLog::Error(LABEL, "type of listener tag is not napi_string.");
    return defaultValue;
}

std::string GetStringTypeAttribute(const napi_env env, const napi_value& object,
    const std::string& propertyName, std::string defaultValue = "")
{
    napi_value propertyValue = NapiHiSysEventUtil::GetPropertyByName(env, object, propertyName);
    if (!IsValueTypeValid(env, propertyValue, napi_valuetype::napi_string)) {
        NapiHiSysEventUtil::ThrowParamTypeError(env, propertyName, "string");
        HiLog::Error(LABEL, "type is not napi_string.");
        return defaultValue;
    }
    return ParseStringValue(env, propertyValue, defaultValue);
}

long long GetLonglongTypeAttribute(const napi_env env, const napi_value& object,
    const std::string& propertyName, long long defaultValue = 0)
{
    napi_value propertyValue = NapiHiSysEventUtil::GetPropertyByName(env, object, propertyName);
    bool isNumberType = IsValueTypeValid(env, propertyValue, napi_valuetype::napi_number);
    bool isBigIntType = IsValueTypeValid(env, propertyValue, napi_valuetype::napi_bigint);
    if (!isNumberType && !isBigIntType) {
        HiLog::Error(LABEL, "type is not napi_number or napi_bigint.");
        return defaultValue;
    }
    if (isBigIntType) {
        return static_cast<long long>(ParseBigIntValue(env, propertyValue));
    }
    return static_cast<long long>(ParseNumberValue(env, propertyValue, defaultValue));
}

int32_t ParseInt32Value(const napi_env env, const napi_value& value, int32_t defaultValue = 0)
{
    int32_t int32Value = 0;
    napi_status ret = napi_get_value_int32(env, value, &int32Value);
    if (ret != napi_ok) {
        HiLog::Error(LABEL, "failed to parse napi value of number type.");
        return defaultValue;
    }
    return int32Value;
}

int32_t GetInt32TypeAttribute(const napi_env env, const napi_value& object,
    const std::string& propertyName, int32_t defaultValue = 0)
{
    napi_value propertyValue = NapiHiSysEventUtil::GetPropertyByName(env, object, propertyName);
    if (!IsValueTypeValid(env, propertyValue, napi_valuetype::napi_number)) {
        NapiHiSysEventUtil::ThrowParamTypeError(env, propertyName, "number");
        HiLog::Error(LABEL, "type is not napi_number.");
        return defaultValue;
    }
    return ParseInt32Value(env, propertyValue);
}

void AppendBoolArrayData(const napi_env env, HiSysEventInfo& info, const std::string& key,
    const napi_value array, size_t len)
{
    std::vector<bool> values;
    napi_value element;
    napi_status status;
    for (uint32_t i = 0; i < len; i++) {
        status = napi_get_element(env, array, i, &element);
        if (status != napi_ok) {
            HiLog::Error(LABEL, "failed to get the element of bool array.");
            continue;
        }
        if (IsValueTypeValid(env, element, napi_valuetype::napi_boolean)) {
            values.emplace_back(ParseBoolValue(env, element));
        }
    }
    info.boolArrayParams[key] = values;
}

void AppendNumberArrayData(const napi_env env, HiSysEventInfo& info, const std::string& key,
    const napi_value array, size_t len)
{
    std::vector<double> values;
    napi_value element;
    napi_status status;
    for (uint32_t i = 0; i < len; i++) {
        status = napi_get_element(env, array, i, &element);
        if (status != napi_ok) {
            HiLog::Error(LABEL, "failed to get the element of number array.");
            continue;
        }
        if (IsValueTypeValid(env, element, napi_valuetype::napi_number)) {
            values.emplace_back(ParseNumberValue(env, element));
        }
    }
    info.doubleArrayParams[key] = values;
}

void AppendBigIntArrayData(const napi_env env, HiSysEventInfo& info, const std::string& key,
    const napi_value array, size_t len)
{
    std::vector<double> values;
    napi_value element;
    napi_status status;
    for (uint32_t i = 0; i < len; i++) {
        status = napi_get_element(env, array, i, &element);
        if (status != napi_ok) {
            HiLog::Error(LABEL, "failed to get the element of big int array.");
            continue;
        }
        if (IsValueTypeValid(env, element, napi_valuetype::napi_bigint)) {
            values.emplace_back(ParseBigIntValue(env, element));
        }
    }
    info.doubleArrayParams[key] = values;
}

void AppendStringArrayData(const napi_env env, HiSysEventInfo& info, const std::string& key,
    const napi_value array, size_t len)
{
    std::vector<std::string> values;
    napi_value element;
    napi_status status;
    for (uint32_t i = 0; i < len; i++) {
        status = napi_get_element(env, array, i, &element);
        if (status != napi_ok) {
            HiLog::Error(LABEL, "failed to get the element of string array.");
            continue;
        }
        if (IsValueTypeValid(env, element, napi_valuetype::napi_string)) {
            values.emplace_back(ParseStringValue(env, element));
        }
    }
    info.stringArrayParams[key] = values;
}

void AddArrayParamToEventInfo(const napi_env env, HiSysEventInfo& info, const std::string& key, napi_value& array)
{
    uint32_t len = 0;
    napi_status status = napi_get_array_length(env, array, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "failed to get the length of param array.");
        return;
    }
    if (len == 0) {
        HiLog::Warn(LABEL, "array is empty.");
        return;
    }
    napi_value firstItem;
    status = napi_get_element(env, array, 0, &firstItem);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "failed to get the first element in array.");
        return;
    }
    napi_valuetype type;
    status = napi_typeof(env, firstItem, &type);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "failed to get the type of the first element in array.");
        return;
    }
    switch (type) {
        case napi_valuetype::napi_boolean:
            AppendBoolArrayData(env, info, key, array, len);
            HiLog::Debug(LABEL, "AppendBoolArrayData: %{public}s.", key.c_str());
            break;
        case napi_valuetype::napi_number:
            AppendNumberArrayData(env, info, key, array, len);
            HiLog::Debug(LABEL, "AppendNumberArrayData: %{public}s.", key.c_str());
            break;
        case napi_valuetype::napi_bigint:
            AppendBigIntArrayData(env, info, key, array, len);
            HiLog::Debug(LABEL, "AppendBigIntArrayData: %{public}s.", key.c_str());
            break;
        case napi_valuetype::napi_string:
            AppendStringArrayData(env, info, key, array, len);
            HiLog::Debug(LABEL, "AppendStringArrayData: %{public}s.", key.c_str());
            break;
        default:
            break;
    }
}

void AddParamToEventInfo(const napi_env env, HiSysEventInfo& info, const std::string& key, napi_value& value)
{
    if (CheckValueIsArray(env, value)) {
        AddArrayParamToEventInfo(env, info, key, value);
        return;
    }
    napi_valuetype type = GetValueType(env, value);
    switch (type) {
        case napi_valuetype::napi_boolean:
            info.boolParams[key] = ParseBoolValue(env, value);
            break;
        case napi_valuetype::napi_number:
            info.doubleParams[key] = ParseNumberValue(env, value);
            break;
        case napi_valuetype::napi_string:
            info.stringParams[key] = ParseStringValue(env, value);
            break;
        case napi_valuetype::napi_bigint:
            info.boolParams[key] = ParseBigIntValue(env, value);
            break;
        default:
            break;
    }
}

void GetObjectTypeAttribute(const napi_env env, const napi_value& object,
    const std::string& propertyName, HiSysEventInfo& info)
{
    napi_value propertyValue = NapiHiSysEventUtil::GetPropertyByName(env, object, propertyName);
    if (!IsValueTypeValid(env, propertyValue, napi_valuetype::napi_object)) {
        HiLog::Error(LABEL, "type is not napi_object.");
        return;
    }
    napi_value keyArr = nullptr;
    napi_status status = napi_get_property_names(env, propertyValue, &keyArr);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "failed to parse property names of a js object.");
        return;
    }
    uint32_t len = 0;
    status = napi_get_array_length(env, keyArr, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "failed to get the length of the key-value pairs.");
        return;
    }
    for (uint32_t i = 0; i < len; i++) {
        napi_value key = nullptr;
        napi_get_element(env, keyArr, i, &key);
        if (!IsValueTypeValid(env, key, napi_valuetype::napi_string)) {
            HiLog::Warn(LABEL, "this param would be discarded because of invalid type of the key.");
            continue;
        }
        char buf[BUF_SIZE] = {0};
        size_t valueLen = 0;
        napi_get_value_string_utf8(env, key, buf, BUF_SIZE - 1, &valueLen);
        if (!CheckKeyTypeString(buf)) {
            HiLog::Warn(LABEL, "this param would be discarded because of invalid format of the key.");
            continue;
        }
        napi_value val = NapiHiSysEventUtil::GetPropertyByName(env, propertyValue, buf);
        AddParamToEventInfo(env, info, buf, val);
    }
}

void ParseStringArray(const napi_env env, napi_value& arrayValue, std::vector<std::string>& arrayDest)
{
    if (!CheckValueIsArray(env, arrayValue)) {
        HiLog::Error(LABEL, "try to parse a array from a napi value without array type");
        return;
    }
    uint32_t len = 0;
    napi_status status = napi_get_array_length(env, arrayValue, &len);
    if (status != napi_ok) {
        return;
    }
    napi_value element;
    for (uint32_t i = 0; i < len; i++) {
        status = napi_get_element(env, arrayValue, i, &element);
        if (status != napi_ok) {
            return;
        }
        if (IsValueTypeValid(env, element, napi_valuetype::napi_string)) {
            std::string str = ParseStringValue(env, element);
            HiLog::Debug(LABEL, "parse string: %{public}s.", str.c_str());
            arrayDest.emplace_back(str);
        }
    }
}

ListenerRule ParseListenerRule(const napi_env env, const napi_value& jsObj)
{
    if (!IsValueTypeValid(env, jsObj, napi_valuetype::napi_object)) {
        return ListenerRule("", RuleType::WHOLE_WORD);
    }
    std::string domain = GetStringTypeAttribute(env, jsObj, NapiHiSysEventUtil::DOMAIN_ATTR);
    HiLog::Debug(LABEL, "domain is %{public}s.", domain.c_str());
    std::string name = GetStringTypeAttribute(env, jsObj, NapiHiSysEventUtil::NAME_ATTR);
    HiLog::Debug(LABEL, "name is %{public}s.", name.c_str());
    int32_t ruleType = GetInt32TypeAttribute(env, jsObj, RULE_TYPE_ATTR, RuleType::WHOLE_WORD);
    HiLog::Debug(LABEL, "ruleType is %{public}d.", ruleType);
    std::string tag = GetTagAttribute(env, jsObj);
    HiLog::Debug(LABEL, "tag is %{public}s.", tag.c_str());
    return ListenerRule(domain, name, tag, RuleType(ruleType));
}

bool IsQueryRuleValid(const napi_env env, const QueryRule& rule)
{
    auto domain = rule.GetDomain();
    if (!StringFilter::GetInstance().IsValidName(domain, MAX_DOMAIN_LENGTH)) {
        NapiHiSysEventUtil::ThrowErrorByRet(env, NapiInnerError::ERR_INVALID_DOMAIN_IN_QUERY_RULE);
        return false;
    }
    auto names = rule.GetEventList();
    if (std::any_of(names.begin(), names.end(), [] (auto& name) {
        return !StringFilter::GetInstance().IsValidName(name, MAX_EVENT_NAME_LENGTH);
    })) {
        NapiHiSysEventUtil::ThrowErrorByRet(env, NapiInnerError::ERR_INVALID_EVENT_NAME_IN_QUERY_RULE);
        return false;
    }
    return true;
}

QueryRule ParseQueryRule(const napi_env env, napi_value& jsObj)
{
    std::vector<std::string> names;
    if (!IsValueTypeValid(env, jsObj, napi_valuetype::napi_object)) {
        return QueryRule("", names);
    }
    std::string domain = GetStringTypeAttribute(env, jsObj, NapiHiSysEventUtil::DOMAIN_ATTR);
    HiLog::Debug(LABEL, "domain is %{public}s.", domain.c_str());
    napi_value propertyValue = NapiHiSysEventUtil::GetPropertyByName(env, jsObj, NAMES_ATTR);
    ParseStringArray(env, propertyValue, names);
    for (auto& name : names) {
        HiLog::Debug(LABEL, "    event name is %{public}s.", name.c_str());
    }
    return QueryRule(domain, names);
}

void SetNamedProperty(const napi_env env, napi_value& object, const std::string& propertyName,
    napi_value& propertyValue)
{
    napi_status status = napi_set_named_property(env, object, propertyName.c_str(), propertyValue);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "set property %{public}s failed.", propertyName.c_str());
    }
}

bool IsBaseInfoKey(const std::string& propertyName)
{
    return propertyName == DOMAIN__KEY || propertyName == NAME__KEY || propertyName == TYPE__KEY;
}

std::string translateKeyToAttrName(const std::string& key)
{
    if (key == DOMAIN__KEY) {
        return NapiHiSysEventUtil::DOMAIN_ATTR;
    }
    if (key == NAME__KEY) {
        return NapiHiSysEventUtil::NAME_ATTR;
    }
    if (key == TYPE__KEY) {
        return NapiHiSysEventUtil::EVENT_TYPE_ATTR;
    }
    return "";
}

void AppendBaseInfo(const napi_env env, napi_value& sysEventInfo, const std::string& key, Json::Value& value)
{
    if ((key == DOMAIN__KEY || key == NAME__KEY) && value.isString()) {
        NapiHiSysEventUtil::AppendStringPropertyToJsObject(env, translateKeyToAttrName(key),
            value.asString(), sysEventInfo);
    }
    if (key == TYPE__KEY && value.isInt()) {
        NapiHiSysEventUtil::AppendInt32PropertyToJsObject(env, translateKeyToAttrName(key),
            static_cast<int32_t>(value.asInt()), sysEventInfo);
    }
}

void CreateBoolValue(const napi_env env, bool value, napi_value& val)
{
    napi_status status = napi_get_boolean(env, value, &val);
    HiLog::Debug(LABEL, "create napi value of bool type, value is %{public}d.", value);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "failed to get create napi value of bool type.");
    }
}

void CreateDoubleValue(const napi_env env, double value, napi_value& val)
{
    napi_status status = napi_create_double(env, value, &val);
    HiLog::Debug(LABEL, "create napi value of double type, value is %{public}f.", value);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "failed to get create napi value of double type.");
    }
}

void CreateUint32Value(const napi_env env, uint32_t value, napi_value& val)
{
    napi_status status = napi_create_uint32(env, value, &val);
    HiLog::Debug(LABEL, "create napi value of uint32 type, value is %{public}u.", value);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "failed to get create napi value of uint32 type.");
    }
}

void CreateParamItemTypeValue(const napi_env env, Json::Value& jsonValue, napi_value& value)
{
    if (jsonValue.isBool()) {
        CreateBoolValue(env, jsonValue.asBool(), value);
        return;
    }
    if (jsonValue.isInt()) {
        NapiHiSysEventUtil::CreateInt32Value(env, static_cast<int32_t>(jsonValue.asInt()), value);
        return;
    }
    if (jsonValue.isUInt()) {
        CreateUint32Value(env, static_cast<uint32_t>(jsonValue.asUInt()), value);
        return;
    }
#ifdef JSON_HAS_INT64
    if (jsonValue.isInt64() && jsonValue.type() != Json::ValueType::uintValue) {
        NapiHiSysEventUtil::CreateInt64Value(env, jsonValue.asInt64(), value);
        return;
    }
    if (jsonValue.isUInt64() && jsonValue.type() != Json::ValueType::intValue) {
        NapiHiSysEventUtil::CreateUInt64Value(env, jsonValue.asUInt64(), value);
        return;
    }
#endif
    if (jsonValue.isDouble()) {
        CreateDoubleValue(env, jsonValue.asDouble(), value);
        return;
    }
    if (jsonValue.isString()) {
        NapiHiSysEventUtil::CreateStringValue(env, jsonValue.asString(), value);
        return;
    }
}

void AppendArrayParams(const napi_env env, napi_value& params, const std::string& key, Json::Value& value)
{
    size_t len = value.size();
    napi_value array = nullptr;
    napi_create_array_with_length(env, len, &array);
    for (size_t i = 0; i < len; i++) {
        napi_value item;
        CreateParamItemTypeValue(env, value[static_cast<int>(i)], item);
        napi_set_element(env, array, i, item);
    }
    SetNamedProperty(env, params, key, array);
}

void AppendParamsInfo(const napi_env env, napi_value& params, const std::string& key, Json::Value& jsonValue)
{
    if (jsonValue.isArray()) {
        AppendArrayParams(env, params, key, jsonValue);
        return;
    }
    napi_value property = nullptr;
    CreateParamItemTypeValue(env, jsonValue, property);
    SetNamedProperty(env, params, key, property);
}
}

napi_value NapiHiSysEventUtil::GetPropertyByName(const napi_env env, const napi_value& object,
    const std::string& propertyName)
{
    napi_value result = nullptr;
    napi_status status = napi_get_named_property(env, object, propertyName.c_str(), &result);
    if (status != napi_ok) {
        HiLog::Debug(LABEL, "failed to parse property named %{public}s from JS object.", propertyName.c_str());
    }
    return result;
}

void NapiHiSysEventUtil::ParseHiSysEventInfo(const napi_env env, napi_value* param,
    size_t paramNum, HiSysEventInfo& info)
{
    if (paramNum <= SYS_EVENT_INFO_PARAM_INDEX) {
        return;
    }
    if (!IsValueTypeValid(env, param[SYS_EVENT_INFO_PARAM_INDEX], napi_valuetype::napi_object)) {
        NapiHiSysEventUtil::ThrowParamTypeError(env, "info", "object");
        return;
    }
    info.domain = GetStringTypeAttribute(env, param[SYS_EVENT_INFO_PARAM_INDEX], NapiHiSysEventUtil::DOMAIN_ATTR);
    HiLog::Debug(LABEL, "domain is %{public}s.", info.domain.c_str());
    info.name = GetStringTypeAttribute(env, param[SYS_EVENT_INFO_PARAM_INDEX], NapiHiSysEventUtil::NAME_ATTR);
    HiLog::Debug(LABEL, "name is %{public}s.", info.name.c_str());
    info.eventType = HiSysEvent::EventType(GetInt32TypeAttribute(env,
        param[SYS_EVENT_INFO_PARAM_INDEX], EVENT_TYPE_ATTR, HiSysEvent::EventType::FAULT));
    HiLog::Debug(LABEL, "eventType is %{public}d.", info.eventType);
    GetObjectTypeAttribute(env, param[SYS_EVENT_INFO_PARAM_INDEX], PARAMS_ATTR, info);
}

bool NapiHiSysEventUtil::HasStrParamLenOverLimit(HiSysEventInfo& info)
{
    return any_of(info.stringParams.begin(), info.stringParams.end(), [] (auto& item) {
        return item.second.size() > JS_STR_PARM_LEN_LIMIT;
    }) || any_of(info.stringArrayParams.begin(), info.stringArrayParams.end(), [] (auto& item) {
        auto allStr = item.second;
        return any_of(allStr.begin(), allStr.end(), [] (auto& item) {
            return item.size() > JS_STR_PARM_LEN_LIMIT;
        });
    });
}

void NapiHiSysEventUtil::CreateHiSysEventInfoJsObject(const napi_env env, const std::string& jsonStr,
    napi_value& sysEventInfo)
{
    Json::Value eventJson;
#ifdef JSONCPP_VERSION_STRING
    Json::CharReaderBuilder jsonRBuilder;
    Json::CharReaderBuilder::strictMode(&jsonRBuilder.settings_);
    std::unique_ptr<Json::CharReader> const reader(jsonRBuilder.newCharReader());
    JSONCPP_STRING errs;
    if (!reader->parse(jsonStr.data(), jsonStr.data() + jsonStr.size(), &eventJson, &errs)) {
#else
    Json::Reader reader(Json::Features::strictMode());
    if (!reader.parse(jsonStr, eventJson)) {
#endif
        HiLog::Error(LABEL, "parse event detail info failed, please check the style of json infomation: %{public}s",
            jsonStr.c_str());
        return;
    }
    napi_create_object(env, &sysEventInfo);
    napi_value params = nullptr;
    napi_create_object(env, &params);
    auto eventNameList = eventJson.getMemberNames();
    for (auto it = eventNameList.cbegin(); it != eventNameList.cend(); it++) {
        auto propertyName = *it;
        if (IsBaseInfoKey(propertyName)) {
            AppendBaseInfo(env, sysEventInfo, propertyName, eventJson[propertyName]);
        } else {
            AppendParamsInfo(env, params, propertyName, eventJson[propertyName]);
        }
    }
    SetNamedProperty(env, sysEventInfo, PARAMS_ATTR, params);
}

void NapiHiSysEventUtil::CreateJsSysEventInfoArray(const napi_env env, const std::vector<std::string>& originValues,
    napi_value& array)
{
    auto len = originValues.size();
    for (size_t i = 0; i < len; i++) {
        napi_value item;
        CreateHiSysEventInfoJsObject(env, originValues[i], item);
        napi_status status = napi_set_element(env, array, i, item);
        if (status != napi_ok) {
            HiLog::Error(LABEL, "napi_set_element failed");
        }
    }
}

void NapiHiSysEventUtil::AppendStringPropertyToJsObject(const napi_env env, const std::string& key,
    const std::string& value, napi_value& jsObj)
{
    napi_value property = nullptr;
    NapiHiSysEventUtil::CreateStringValue(env, value, property);
    SetNamedProperty(env, jsObj, key, property);
}

void NapiHiSysEventUtil::AppendInt32PropertyToJsObject(const napi_env env, const std::string& key,
    const int32_t& value, napi_value& jsObj)
{
    napi_value property = nullptr;
    NapiHiSysEventUtil::CreateInt32Value(env, value, property);
    SetNamedProperty(env, jsObj, key, property);
}

int32_t NapiHiSysEventUtil::ParseListenerRules(const napi_env env, napi_value& array,
    std::vector<ListenerRule>& listenerRules)
{
    if (!CheckValueIsArray(env, array)) {
        ThrowParamTypeError(env, "rules", "array");
        return ERR_LISTENER_RULES_TYPE_NOT_ARRAY;
    }
    uint32_t len = 0;
    napi_status status = napi_get_array_length(env, array, &len);
    if (status != napi_ok) {
        return ERR_NAPI_PARSED_FAILED;
    }
    napi_value firstItem;
    status = napi_get_element(env, array, 0, &firstItem);
    if (status != napi_ok) {
        return ERR_NAPI_PARSED_FAILED;
    }
    napi_valuetype type;
    status = napi_typeof(env, firstItem, &type);
    if (status != napi_ok) {
        return ERR_NAPI_PARSED_FAILED;
    }
    napi_value element;
    for (uint32_t i = 0; i < len; i++) {
        status = napi_get_element(env, array, i, &element);
        if (status != napi_ok) {
            return ERR_NAPI_PARSED_FAILED;
        }
        if (IsValueTypeValid(env, element, napi_valuetype::napi_object)) {
            listenerRules.emplace_back(ParseListenerRule(env, element));
        }
    }
    return NAPI_SUCCESS;
}

int32_t NapiHiSysEventUtil::ParseQueryRules(const napi_env env, napi_value& array, std::vector<QueryRule>& queryRules)
{
    if (!CheckValueIsArray(env, array)) {
        ThrowParamTypeError(env, "rules", "array");
        return ERR_QUERY_RULES_TYPE_NOT_ARRAY;
    }
    uint32_t len = 0;
    napi_status status = napi_get_array_length(env, array, &len);
    if (status != napi_ok) {
        return ERR_NAPI_PARSED_FAILED;
    }
    napi_value firstItem;
    status = napi_get_element(env, array, 0, &firstItem);
    if (status != napi_ok) {
        return ERR_NAPI_PARSED_FAILED;
    }
    napi_valuetype type;
    status = napi_typeof(env, firstItem, &type);
    if (status != napi_ok) {
        return ERR_NAPI_PARSED_FAILED;
    }
    napi_value element;
    for (uint32_t i = 0; i < len; i++) {
        status = napi_get_element(env, array, i, &element);
        if (status != napi_ok) {
            return ERR_NAPI_PARSED_FAILED;
        }
        if (IsValueTypeValid(env, element, napi_valuetype::napi_object)) {
            auto queryRule = ParseQueryRule(env, element);
            if (IsQueryRuleValid(env, queryRule)) {
                queryRules.emplace_back(queryRule);
            }
        }
    }
    return NAPI_SUCCESS;
}

int32_t NapiHiSysEventUtil::ParseQueryArg(const napi_env env, napi_value& jsObj, QueryArg& queryArg)
{
    if (!IsValueTypeValid(env, jsObj, napi_valuetype::napi_object)) {
        ThrowParamTypeError(env, "queryArg", "object");
        return ERR_QUERY_ARG_TYPE_INVALID;
    }
    queryArg.beginTime = GetLonglongTypeAttribute(env, jsObj, BEGIN_TIME_ATTR, DEFAULT_TIME_STAMP);
    HiLog::Debug(LABEL, "queryArg.beginTime is %{public}lld.", queryArg.beginTime);
    queryArg.endTime = GetLonglongTypeAttribute(env, jsObj, END_TIME_ATTR, DEFAULT_TIME_STAMP);
    HiLog::Debug(LABEL, "queryArg.endTime is %{public}lld.", queryArg.endTime);
    queryArg.maxEvents = GetInt32TypeAttribute(env, jsObj, MAX_EVENTS_ATTR, DEFAULT_MAX_EVENTS);
    HiLog::Debug(LABEL, "queryArg.maxEvents is %{public}d.", queryArg.maxEvents);
    queryArg.fromSeq = GetLonglongTypeAttribute(env, jsObj, BEGIN_SEQ_ATTR, DEFAULT_SEQ);
    HiLog::Debug(LABEL, "queryArg.fromSeq is %{public}lld.", queryArg.fromSeq);
    queryArg.toSeq = GetLonglongTypeAttribute(env, jsObj, END_SEQ_ATTR, DEFAULT_SEQ);
    HiLog::Debug(LABEL, "queryArg.endSeq is %{public}lld.", queryArg.toSeq);
    return NAPI_SUCCESS;
}

void NapiHiSysEventUtil::CreateNull(const napi_env env, napi_value& ret)
{
    napi_status status = napi_get_null(env, &ret);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "failed to create napi value of null.");
    }
}

void NapiHiSysEventUtil::CreateInt32Value(const napi_env env, int32_t value, napi_value& ret)
{
    napi_status status = napi_create_int32(env, value, &ret);
    HiLog::Debug(LABEL, "create napi value of int32 type, value is %{public}d.", value);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "failed to create napi value of int32 type.");
    }
}

void NapiHiSysEventUtil::CreateInt64Value(const napi_env env, int64_t value, napi_value& ret)
{
    napi_status status = napi_create_bigint_int64(env, value, &ret);
    HiLog::Debug(LABEL, "create napi value of int64_t type, value is %{public}" PRId64 ".", value);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "failed to create napi value of int64_t type.");
    }
}

void NapiHiSysEventUtil::CreateUInt64Value(const napi_env env, uint64_t value, napi_value& ret)
{
    napi_status status = napi_create_bigint_uint64(env, value, &ret);
    HiLog::Debug(LABEL, "create napi value of uint64_t type, value is %{public}" PRIu64 ".", value);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "failed to create napi value of uint64_t type.");
    }
}

void NapiHiSysEventUtil::CreateStringValue(const napi_env env, std::string value, napi_value& ret)
{
    napi_status status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &ret);
    HiLog::Debug(LABEL, "create napi value of string type, value is %{public}s.", value.c_str());
    if (status != napi_ok) {
        HiLog::Error(LABEL, "failed to create napi value of string type.");
    }
}

void NapiHiSysEventUtil::ThrowParamMandatoryError(napi_env env, const std::string paramName)
{
    ThrowError(env, NapiError::ERR_PARAM_CHECK, "Parameter error. The " + paramName + " parameter is mandatory.");
}

void NapiHiSysEventUtil::ThrowParamTypeError(napi_env env, const std::string paramName, std::string paramType)
{
    ThrowError(env, NapiError::ERR_PARAM_CHECK, "Parameter error. The type of " + paramName + " must be "
        + paramType + ".");
}

napi_value NapiHiSysEventUtil::CreateError(napi_env env, int32_t code, const std::string& msg)
{
    napi_value err = nullptr;
    napi_value napiCode = nullptr;
    NapiHiSysEventUtil::CreateStringValue(env, std::to_string(code), napiCode);
    napi_value napiStr = nullptr;
    NapiHiSysEventUtil::CreateStringValue(env, msg, napiStr);
    if (napi_create_error(env, napiCode, napiStr, &err) != napi_ok) {
        HiLog::Error(LABEL, "failed to create napi error");
    }
    return err;
}

void NapiHiSysEventUtil::ThrowError(napi_env env, const int32_t code, const std::string& msg)
{
    if (napi_throw_error(env, std::to_string(code).c_str(), msg.c_str()) != napi_ok) {
        HiLog::Error(LABEL, "failed to throw err, code=%{public}d, msg=%{public}s.", code, msg.c_str());
    }
}

std::pair<int32_t, std::string> NapiHiSysEventUtil::GetErrorDetailByRet(napi_env env, const int32_t retCode)
{
    HiLog::Info(LABEL, "origin result code is %{public}d.", retCode);
    const std::unordered_map<int32_t, std::pair<int32_t, std::string>> errMap = {
        // common
        {ERR_NO_PERMISSION, {NapiError::ERR_PERMISSION_CHECK,
            "Permission denied. An attempt was made to read sysevent forbidden"
            " by permission: ohos.permission.READ_DFX_SYSEVENT."}},
        // write refer
        {ERR_DOMAIN_NAME_INVALID, {NapiError::ERR_INVALID_DOMAIN, "Domain is invalid"}},
        {ERR_EVENT_NAME_INVALID, {NapiError::ERR_INVALID_EVENT_NAME, "Event name is invalid"}},
        {ERR_DOES_NOT_INIT, {NapiError::ERR_ENV_ABNORMAL, "Environment is abnormal"}},
        {ERR_OVER_SIZE, {NapiError::ERR_CONTENT_OVER_LIMIT, "Size of event content is over limit"}},
        {ERR_KEY_NAME_INVALID, {NapiError::ERR_INVALID_PARAM_NAME, "Name of param is invalid"}},
        {ERR_VALUE_LENGTH_TOO_LONG, {NapiError::ERR_STR_LEN_OVER_LIMIT,
            "Size of string type param is over limit"}},
        {ERR_KEY_NUMBER_TOO_MUCH, {NapiError::ERR_PARAM_COUNT_OVER_LIMIT, "Count of params is over limit"}},
        {ERR_ARRAY_TOO_MUCH, {NapiError::ERR_ARRAY_SIZE_OVER_LIMIT, "Size of array type param is over limit"}},
        // ipc common
        {ERR_SYS_EVENT_SERVICE_NOT_FOUND, {NapiError::ERR_ENV_ABNORMAL, "Environment is abnormal"}},
        {ERR_CAN_NOT_WRITE_DESCRIPTOR, {NapiError::ERR_ENV_ABNORMAL, "Environment is abnormal"}},
        {ERR_CAN_NOT_WRITE_PARCEL, {NapiError::ERR_ENV_ABNORMAL, "Environment is abnormal"}},
        {ERR_CAN_NOT_WRITE_REMOTE_OBJECT, {NapiError::ERR_ENV_ABNORMAL, "Environment is abnormal"}},
        {ERR_CAN_NOT_SEND_REQ, {NapiError::ERR_ENV_ABNORMAL, "Environment is abnormal"}},
        {ERR_CAN_NOT_READ_PARCEL, {NapiError::ERR_ENV_ABNORMAL, "Environment is abnormal"}},
        {ERR_SEND_FAIL, {NapiError::ERR_ENV_ABNORMAL, "Environment is abnormal"}},
        // add watcher
        {ERR_TOO_MANY_WATCHERS, {NapiError::ERR_WATCHER_COUNT_OVER_LIMIT, "Count of watchers is over limit"}},
        {ERR_TOO_MANY_WATCH_RULES, {NapiError::ERR_WATCH_RULE_COUNT_OVER_LIMIT,
            "Count of watch rules is over limit"}},
        // remove watcher
        {ERR_LISTENER_NOT_EXIST, {NapiError::ERR_WATCHER_NOT_EXIST, "Watcher is not exist"}},
        {ERR_NAPI_LISTENER_NOT_FOUND, {NapiError::ERR_WATCHER_NOT_EXIST, "Watcher is not exist"}},
        // query refer
        {ERR_TOO_MANY_QUERY_RULES, {NapiError::ERR_QUERY_RULE_COUNT_OVER_LIMIT,
            "Count of query rules is over limit"}},
        {ERR_TOO_MANY_CONCURRENT_QUERIES, {NapiError::ERR_CONCURRENT_QUERY_COUNT_OVER_LIMIT,
            "cCount of concurrent queries is over limit"}},
        {ERR_QUERY_TOO_FREQUENTLY, {NapiError::ERR_QUERY_TOO_FREQUENTLY, "Frequency of event query is over limit"}},
        {NapiInnerError::ERR_INVALID_DOMAIN_IN_QUERY_RULE,
            {NapiError::ERR_INVALID_QUERY_RULE, "Query rule is invalid"}},
        {ERR_QUERY_RULE_INVALID, {NapiError::ERR_INVALID_QUERY_RULE, "Query rule is invalid"}},
        {NapiInnerError::ERR_INVALID_EVENT_NAME_IN_QUERY_RULE,
            {NapiError::ERR_INVALID_QUERY_RULE, "Query rule is invalid"}},
    };
    return errMap.find(retCode) == errMap.end() ?
        std::make_pair(NapiError::ERR_ENV_ABNORMAL, "environment is abnormal") : errMap.at(retCode);
}

napi_value NapiHiSysEventUtil::CreateErrorByRet(napi_env env, const int32_t retCode)
{
    auto detail = GetErrorDetailByRet(env, retCode);
    return CreateError(env, detail.first, detail.second);
}

void NapiHiSysEventUtil::ThrowErrorByRet(napi_env env, const int32_t retCode)
{
    auto detail = GetErrorDetailByRet(env, retCode);
    ThrowError(env, detail.first, detail.second);
}
} // namespace HiviewDFX
} // namespace OHOS