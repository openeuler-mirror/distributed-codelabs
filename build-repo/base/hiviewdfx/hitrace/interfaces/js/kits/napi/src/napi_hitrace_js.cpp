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

#include "napi_hitrace_init.h"

#include "hilog/log.h"
#include "napi_hitrace_param.h"
#include "napi_hitrace_util.h"

using namespace OHOS::HiviewDFX;

namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, 0xD002D03, "HITRACE_JS_NAPI" };
constexpr uint32_t BUF_SIZE_64 = 64;

bool ParseInt32Param(const napi_env& env, const napi_value& origin, int& dest)
{
    if (!NapiHitraceUtil::CheckValueTypeValidity(env, origin, napi_valuetype::napi_number)) {
        return false;
    }
    napi_get_value_int32(env, origin, &dest);
    return true;
}

bool ParseStringParam(const napi_env& env, const napi_value& origin, std::string& dest)
{
    if (!NapiHitraceUtil::CheckValueTypeValidity(env, origin, napi_valuetype::napi_string)) {
        return false;
    }
    char buf[BUF_SIZE_64] = {0};
    size_t bufLength = 0;
    napi_get_value_string_utf8(env, origin, buf, BUF_SIZE_64, &bufLength);
    dest = std::string {buf};
    return true;
}

bool ParseTraceIdObject(const napi_env& env, const napi_value& origin, HiTraceId& traceId)
{
    if (!NapiHitraceUtil::CheckValueTypeValidity(env, origin, napi_valuetype::napi_object)) {
        return false;
    }
    NapiHitraceUtil::TransHiTraceIdJsObjectToNative(env, traceId, origin);
    return true;
}
}

static napi_value Begin(napi_env env, napi_callback_info info)
{
    size_t paramNum = ParamNum::TOTAL_TWO;
    napi_value params[ParamNum::TOTAL_TWO] = {0};
    napi_value thisArg = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &paramNum, params, &thisArg, &data));
    HiTraceId traceId;
    napi_value val = nullptr;
    NapiHitraceUtil::CreateHiTraceIdJsObject(env, traceId, val);
    if (paramNum != ParamNum::TOTAL_ONE && paramNum != ParamNum::TOTAL_TWO) {
        HiLog::Error(LABEL,
            "failed to begin a new trace, count of parameters is not equal to 1 or 2");
        return val;
    }
    std::string name;
    if (!ParseStringParam(env, params[ParamIndex::PARAM_FIRST], name)) {
        HiLog::Error(LABEL, "name type must be string.");
        return val;
    }
    int flag = HiTraceFlag::HITRACE_FLAG_DEFAULT;
    if (paramNum == ParamNum::TOTAL_TWO &&
            !ParseInt32Param(env, params[ParamIndex::PARAM_SECOND], flag)) {
        HiLog::Error(LABEL, "flag type must be number.");
        return val;
    }
    traceId = HiTraceChain::Begin(name, flag);
    NapiHitraceUtil::CreateHiTraceIdJsObject(env, traceId, val);
    return val;
}

static napi_value End(napi_env env, napi_callback_info info)
{
    size_t paramNum = ParamNum::TOTAL_ONE;
    napi_value params[ParamNum::TOTAL_ONE] = {0};
    napi_value thisArg = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &paramNum, params, &thisArg, &data));
    if (paramNum != ParamNum::TOTAL_ONE) {
        HiLog::Error(LABEL,
            "failed to end trace by trace id, count of parameters is not equal to 1.");
        return nullptr;
    }
    HiTraceId traceId;
    if (!ParseTraceIdObject(env, params[ParamIndex::PARAM_FIRST], traceId)) {
        HiLog::Error(LABEL, "hitarce id type must be object.");
        return nullptr;
    }
    HiTraceChain::End(traceId);
    return nullptr;
}

static napi_value GetId(napi_env env, napi_callback_info info)
{
    HiTraceId traceId = HiTraceChain::GetId();
    napi_value val = nullptr;
    NapiHitraceUtil::CreateHiTraceIdJsObject(env, traceId, val);
    return val;
}

static napi_value SetId(napi_env env, napi_callback_info info)
{
    size_t paramNum = ParamNum::TOTAL_ONE;
    napi_value params[ParamNum::TOTAL_ONE] = {0};
    napi_value thisArg = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &paramNum, params, &thisArg, &data));
    if (paramNum != ParamNum::TOTAL_ONE) {
        HiLog::Error(LABEL,
            "failed to set a new id for a trace, count of parameters is not equal to 1.");
        return nullptr;
    }
    HiTraceId traceId;
    if (!ParseTraceIdObject(env, params[ParamIndex::PARAM_FIRST], traceId)) {
        HiLog::Error(LABEL, "hitarce id type must be object.");
        return nullptr;
    }
    HiTraceChain::SetId(traceId);
    return nullptr;
}

static napi_value ClearId(napi_env env, napi_callback_info info)
{
    HiTraceChain::ClearId();
    return nullptr;
}

static napi_value CreateSpan(napi_env env, napi_callback_info info)
{
    HiTraceId traceId = HiTraceChain::CreateSpan();
    napi_value val = nullptr;
    NapiHitraceUtil::CreateHiTraceIdJsObject(env, traceId, val);
    return val;
}

static napi_value Tracepoint(napi_env env, napi_callback_info info)
{
    size_t paramNum = ParamNum::TOTAL_FOUR;
    napi_value params[ParamNum::TOTAL_FOUR] = {0};
    napi_value thisArg = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &paramNum, params, &thisArg, &data));
    if (paramNum != ParamNum::TOTAL_FOUR) {
        HiLog::Error(LABEL,
            "failed to trace point, count of parameters is not equal to 4.");
        return nullptr;
    }
    int communicationModeInt = 0;
    if (!ParseInt32Param(env, params[ParamIndex::PARAM_FIRST], communicationModeInt)) {
        HiLog::Error(LABEL, "HiTraceCommunicationMode type must be number.");
        return nullptr;
    }
    HiTraceCommunicationMode communicationMode = HiTraceCommunicationMode(communicationModeInt);
    int tracePointTypeInt = 0;
    if (!ParseInt32Param(env, params[ParamIndex::PARAM_SECOND], tracePointTypeInt)) {
        HiLog::Error(LABEL, "HiTraceTracePointType type must be number.");
        return nullptr;
    }
    HiTraceTracepointType tracePointType = HiTraceTracepointType(tracePointTypeInt);
    HiTraceId traceId;
    if (!ParseTraceIdObject(env, params[ParamIndex::PARAM_THIRD], traceId)) {
        HiLog::Error(LABEL, "hitarce id type must be object.");
        return nullptr;
    }
    std::string description;
    if (!ParseStringParam(env, params[ParamIndex::PARAM_FORTH], description)) {
        HiLog::Error(LABEL, "descriptione type must be string.");
        return nullptr;
    }
    HiTraceChain::Tracepoint(communicationMode, tracePointType, traceId, "%s", description.c_str());
    return nullptr;
}

static napi_value IsValid(napi_env env, napi_callback_info info)
{
    size_t paramNum = ParamNum::TOTAL_ONE;
    napi_value params[ParamNum::TOTAL_ONE] = {0};
    napi_value thisArg = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &paramNum, params, &thisArg, &data));
    bool isValid = false;
    napi_value val = nullptr;
    napi_get_boolean(env, isValid, &val);
    if (paramNum != ParamNum::TOTAL_ONE) {
        HiLog::Error(LABEL,
            "failed to check whether a id is valid or not, count of parameters is not equal to 1.");
        return val;
    }
    HiTraceId traceId;
    if (!ParseTraceIdObject(env, params[ParamIndex::PARAM_FIRST], traceId)) {
        HiLog::Error(LABEL, "hitarce id type must be object.");
        return val;
    }
    isValid = traceId.IsValid();
    napi_get_boolean(env, isValid, &val);
    return val;
}

static napi_value IsFlagEnabled(napi_env env, napi_callback_info info)
{
    size_t paramNum = ParamNum::TOTAL_TWO;
    napi_value params[ParamNum::TOTAL_TWO] = {0};
    napi_value thisArg = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &paramNum, params, &thisArg, &data));
    bool isFalgEnabled = false;
    napi_value val = nullptr;
    napi_get_boolean(env, isFalgEnabled, &val);
    if (paramNum != ParamNum::TOTAL_TWO) {
        HiLog::Error(LABEL,
            "failed to check whether a flag is enabled in a trace id, count of parameters is not equal to 2.");
        return val;
    }
    HiTraceId traceId;
    if (!ParseTraceIdObject(env, params[ParamIndex::PARAM_FIRST], traceId)) {
        HiLog::Error(LABEL, "hitarce id type must be object.");
        return val;
    }
    int traceFlagInt = 0;
    if (!ParseInt32Param(env, params[ParamIndex::PARAM_SECOND], traceFlagInt)) {
        HiLog::Error(LABEL, "HiTraceFlag type must be number.");
        return val;
    }
    HiTraceFlag traceFlag = HiTraceFlag(traceFlagInt);
    isFalgEnabled = traceId.IsFlagEnabled(traceFlag);
    napi_get_boolean(env, isFalgEnabled, &val);
    return val;
}

static napi_value EnableFlag(napi_env env, napi_callback_info info)
{
    size_t paramNum = ParamNum::TOTAL_TWO;
    napi_value params[ParamNum::TOTAL_TWO] = {0};
    napi_value thisArg = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &paramNum, params, &thisArg, &data));
    if (paramNum != ParamNum::TOTAL_TWO) {
        HiLog::Error(LABEL,
            "failed to enable a flag for a trace id, count of parameters is not equal to 2.");
        return nullptr;
    }
    HiTraceId traceId;
    if (!ParseTraceIdObject(env, params[ParamIndex::PARAM_FIRST], traceId)) {
        HiLog::Error(LABEL, "hitarce id type must be object.");
        return nullptr;
    }
    int traceFlagInt = 0;
    if (!ParseInt32Param(env, params[ParamIndex::PARAM_SECOND], traceFlagInt)) {
        HiLog::Error(LABEL, "HiTraceFlag type must be number.");
        return nullptr;
    }
    HiTraceFlag traceFlag = HiTraceFlag(traceFlagInt);
    traceId.EnableFlag(traceFlag);
    NapiHitraceUtil::EnableTraceIdObjectFlag(env, traceId, params[ParamIndex::PARAM_FIRST]);
    return nullptr;
}

EXTERN_C_START
static napi_value TraceNapiInit(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("begin", Begin),
        DECLARE_NAPI_FUNCTION("end", End),
        DECLARE_NAPI_FUNCTION("getId", GetId),
        DECLARE_NAPI_FUNCTION("setId", SetId),
        DECLARE_NAPI_FUNCTION("clearId", ClearId),
        DECLARE_NAPI_FUNCTION("createSpan", CreateSpan),
        DECLARE_NAPI_FUNCTION("tracepoint", Tracepoint),
        DECLARE_NAPI_FUNCTION("isValid", IsValid),
        DECLARE_NAPI_FUNCTION("isFlagEnabled", IsFlagEnabled),
        DECLARE_NAPI_FUNCTION("enableFlag", EnableFlag),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(napi_property_descriptor), desc));

    // init HiTraceFlag class, HiTraceTracePointType class and HiTraceCommunicationMode class
    InitNapiClass(env, exports);
    return exports;
}
EXTERN_C_END

static napi_module hitrace_module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = TraceNapiInit,
    .nm_modname = "hiTraceChain",
    .nm_priv = (reinterpret_cast<void *>(0)),
    .reserved = {0}
};

extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&hitrace_module);
}
