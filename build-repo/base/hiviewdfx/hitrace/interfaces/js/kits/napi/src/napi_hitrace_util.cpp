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

#include "napi_hitrace_util.h"

#include "hilog/log.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, 0xD002D03, "HITRACE_UTIL_NAPI" };
constexpr uint32_t UINT32_T_PRO_DEFAULT_VALUE = 0;
constexpr uint64_t UINT64_T_PRO_DEFAULT_VALUE = 0;
constexpr uint64_t INVALID_CHAIN_ID = 0;
constexpr char CHAIN_ID_ATTR[] = "chainId";
constexpr char SPAN_ID_ATTR[] = "spanId";
constexpr char PARENT_SPAN_ID_ATTR[] = "parentSpanId";
constexpr char FLAGS_ATTR[] = "flags";

napi_value CreateInt32Value(const napi_env env, int32_t value)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, value, &result));
    return result;
}

napi_value CreateInt64Value(const napi_env env, int64_t value)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int64(env, value, &result));
    return result;
}

napi_value CreateBigInt64Value(const napi_env env, uint64_t value)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_bigint_uint64(env, value, &result));
    return result;
}

napi_status SetNamedProperty(const napi_env env, napi_value& object,
    const std::string& propertyName, napi_value& propertyValue)
{
    napi_status status = napi_set_named_property(env, object, propertyName.c_str(), propertyValue);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "set property %{public}s failed.", propertyName.c_str());
    }
    return status;
}

napi_value GetPropertyByName(const napi_env env, const napi_value& object,
    const std::string& propertyName)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, object, propertyName.c_str(), &result));
    return result;
}
}

bool NapiHitraceUtil::CheckValueTypeValidity(const napi_env env, const napi_value& jsObj,
    const napi_valuetype typeName)
{
    napi_valuetype valueType = napi_undefined;
    napi_status ret = napi_typeof(env, jsObj, &valueType);
    if (ret != napi_ok) {
        HiLog::Error(LABEL, "failed to parse the type of napi value.");
        return false;
    }
    if (valueType != typeName) {
        HiLog::Error(LABEL, "you have called a function with parameters of wrong type.");
        return false;
    }
    return true;
}

void NapiHitraceUtil::CreateHiTraceIdJsObject(const napi_env env, HiTraceId& traceId,
    napi_value& valueObject)
{
    napi_create_object(env, &valueObject);
    NapiHitraceUtil::SetPropertyBigInt64(env, valueObject, CHAIN_ID_ATTR,
        traceId.GetChainId());
    HiLog::Debug(LABEL, "Native2Js: chainId is %{public}llx.",
        static_cast<unsigned long long>(traceId.GetChainId()));
    NapiHitraceUtil::SetPropertyInt64(env, valueObject, SPAN_ID_ATTR, traceId.GetSpanId());
    HiLog::Debug(LABEL, "Native2Js: spanId is %{public}llx.",
        static_cast<unsigned long long>(traceId.GetSpanId()));
    NapiHitraceUtil::SetPropertyInt64(env, valueObject, PARENT_SPAN_ID_ATTR,
        traceId.GetParentSpanId());
    HiLog::Debug(LABEL, "Native2Js: parentSpanId is %{public}llx.",
        static_cast<unsigned long long>(traceId.GetParentSpanId()));
    NapiHitraceUtil::SetPropertyInt32(env, valueObject, FLAGS_ATTR,
        traceId.GetFlags());
    HiLog::Debug(LABEL, "Native2Js: flags is %{public}d.", traceId.GetFlags());
}

void NapiHitraceUtil::TransHiTraceIdJsObjectToNative(const napi_env env, HiTraceId& traceId,
    const napi_value& valueObject)
{
    uint64_t chainId = NapiHitraceUtil::GetPropertyBigInt64(env, valueObject, CHAIN_ID_ATTR);
    HiLog::Debug(LABEL, "Js2Native: chainId is %{public}llx.",
        static_cast<unsigned long long>(chainId));
    if (chainId == INVALID_CHAIN_ID) {
        return;
    }
    traceId.SetChainId(chainId);
    uint64_t spanId = NapiHitraceUtil::GetPropertyInt64(env, valueObject, SPAN_ID_ATTR);
    HiLog::Debug(LABEL, "Js2Native: spanId is %{public}llx.",
        static_cast<unsigned long long>(spanId));
    traceId.SetSpanId(spanId);
    uint64_t parentSpanId = NapiHitraceUtil::GetPropertyInt64(env, valueObject,
        PARENT_SPAN_ID_ATTR);
    HiLog::Debug(LABEL, "Js2Native: parentSpanId is %{public}llx.",
        static_cast<unsigned long long>(parentSpanId));
    traceId.SetParentSpanId(parentSpanId);
    uint32_t flags = NapiHitraceUtil::GetPropertyInt32(env, valueObject, FLAGS_ATTR);
    HiLog::Debug(LABEL, "Js2Native: flags is %{public}d.", flags);
    traceId.SetFlags(flags);
}

void NapiHitraceUtil::EnableTraceIdObjectFlag(const napi_env env, HiTraceId& traceId,
    napi_value& traceIdObject)
{
    NapiHitraceUtil::SetPropertyInt32(env, traceIdObject, FLAGS_ATTR, traceId.GetFlags());
}

void NapiHitraceUtil::SetPropertyInt32(const napi_env env, napi_value& object,
    const std::string& propertyName, uint32_t value)
{
    napi_value peropertyValue = CreateInt32Value(env, value);
    SetNamedProperty(env, object, propertyName, peropertyValue);
}

void NapiHitraceUtil::SetPropertyInt64(const napi_env env, napi_value& object,
    const std::string& propertyName, uint64_t value)
{
    napi_value peropertyValue = CreateInt64Value(env, value);
    SetNamedProperty(env, object, propertyName, peropertyValue);
}

void NapiHitraceUtil::SetPropertyBigInt64(const napi_env env, napi_value& object,
    const std::string& propertyName, uint64_t value)
{
    napi_value peropertyValue = CreateBigInt64Value(env, value);
    SetNamedProperty(env, object, propertyName, peropertyValue);
}

uint32_t NapiHitraceUtil::GetPropertyInt32(const napi_env env, const napi_value& object,
    const std::string& propertyName)
{
    napi_value propertyValue = GetPropertyByName(env, object, propertyName);
    napi_valuetype type;
    napi_status status = napi_typeof(env, propertyValue, &type);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "failed to get %{public}s from HiTraceId Js Object.",
            propertyName.c_str());
        return UINT32_T_PRO_DEFAULT_VALUE;
    }
    if (type != napi_valuetype::napi_number) {
        HiLog::Error(LABEL, "type is not napi_number property.");
        return UINT32_T_PRO_DEFAULT_VALUE;
    }
    int32_t numberValue = 0;
    status = napi_get_value_int32(env, propertyValue, &numberValue);
    if (status == napi_ok) {
        return numberValue;
    }
    HiLog::Error(LABEL, "failed to get napi_number property from HiTraceId Js Object.");
    return UINT32_T_PRO_DEFAULT_VALUE;
}

uint64_t NapiHitraceUtil::GetPropertyInt64(const napi_env env, const napi_value& object,
    const std::string& propertyName)
{
    napi_value propertyValue = GetPropertyByName(env, object, propertyName);
    napi_valuetype type;
    napi_status status = napi_typeof(env, propertyValue, &type);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "failed to get %{public}s from HiTraceId Js Object.",
            propertyName.c_str());
        return UINT64_T_PRO_DEFAULT_VALUE;
    }
    if (type != napi_valuetype::napi_number) {
        HiLog::Error(LABEL, "type is not napi_number property.");
        return UINT64_T_PRO_DEFAULT_VALUE;
    }
    int64_t numberValue = 0;
    status = napi_get_value_int64(env, propertyValue, &numberValue);
    if (status == napi_ok) {
        return numberValue;
    }
    HiLog::Error(LABEL, "failed to get napi_number property from HiTraceId Js Object.");
    return UINT64_T_PRO_DEFAULT_VALUE;
}

uint64_t NapiHitraceUtil::GetPropertyBigInt64(const napi_env env, const napi_value& object,
    const std::string& propertyName)
{
    napi_value propertyValue = GetPropertyByName(env, object, propertyName);
    napi_valuetype type;
    napi_status status = napi_typeof(env, propertyValue, &type);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "failed to get %{public}s from HiTraceId Js Object.",
            propertyName.c_str());
        return UINT64_T_PRO_DEFAULT_VALUE;
    }
    uint64_t bigInt64Value = 0;
    bool lossless = true;
    napi_get_value_bigint_uint64(env, propertyValue, &bigInt64Value, &lossless);
    return bigInt64Value;
}
} // namespace HiviewDFX
} // namespace OHOS
