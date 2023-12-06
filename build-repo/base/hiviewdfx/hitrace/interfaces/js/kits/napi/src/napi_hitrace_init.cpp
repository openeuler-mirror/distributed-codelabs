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

#include "napi_hitrace_init.h"

#include <map>

#include "hilog/log.h"
#include "hitrace/trace.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr char HITRACE_FLAG_ENUM_NAME[] = "HiTraceFlag";
constexpr char HITRACE_TRACE_POINT_TYPE_ENUM_NAME[] = "HiTraceTracepointType";
constexpr char HITRACE_COMMUNICATION_MODE_ENUM_NAME[] = "HiTraceCommunicationMode";

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

void InitHiTraceFlagEnum(napi_env env, std::map<const char*, napi_value>& traceFlagMap)
{
    napi_value defaultFlag = nullptr;
    napi_create_int32(env, HITRACE_FLAG_DEFAULT, &defaultFlag);
    napi_value includeAsyncFlag = nullptr;
    napi_create_int32(env, HITRACE_FLAG_INCLUDE_ASYNC, &includeAsyncFlag);
    napi_value doNotCreateSpanFlag = nullptr;
    napi_create_int32(env, HITRACE_FLAG_DONOT_CREATE_SPAN, &doNotCreateSpanFlag);
    napi_value tpInfoFlag = nullptr;
    napi_create_int32(env, HITRACE_FLAG_TP_INFO, &tpInfoFlag);
    napi_value noBeInfoFlag = nullptr;
    napi_create_int32(env, HITRACE_FLAG_NO_BE_INFO, &noBeInfoFlag);
    napi_value doNotEnableLogFlag = nullptr;
    napi_create_int32(env, HITRACE_FLAG_DONOT_ENABLE_LOG, &doNotEnableLogFlag);
    napi_value failureTriggerFlag = nullptr;
    napi_create_int32(env, HITRACE_FLAG_FAULT_TRIGGER, &failureTriggerFlag);
    napi_value d2dTpInfoFlag = nullptr;
    napi_create_int32(env, HITRACE_FLAG_D2D_TP_INFO, &d2dTpInfoFlag);

    traceFlagMap["DEFAULT"] = defaultFlag;
    traceFlagMap["INCLUDE_ASYNC"] = includeAsyncFlag;
    traceFlagMap["DONOT_CREATE_SPAN"] = doNotCreateSpanFlag;
    traceFlagMap["TP_INFO"] = tpInfoFlag;
    traceFlagMap["NO_BE_INFO"] = noBeInfoFlag;
    traceFlagMap["DISABLE_LOG"] = doNotEnableLogFlag;
    traceFlagMap["FAILURE_TRIGGER"] = failureTriggerFlag;
    traceFlagMap["D2D_TP_INFO"] = d2dTpInfoFlag;
}

void InitHiTraceTracepointTypeEnum(napi_env env,
    std::map<const char*, napi_value>& tracePointMap)
{
    napi_value csTracePoint = nullptr;
    napi_create_int32(env, HITRACE_TP_CS, &csTracePoint);
    napi_value crTracePoint = nullptr;
    napi_create_int32(env, HITRACE_TP_CR, &crTracePoint);
    napi_value ssTracePoint = nullptr;
    napi_create_int32(env, HITRACE_TP_SS, &ssTracePoint);
    napi_value srTracePoint = nullptr;
    napi_create_int32(env, HITRACE_TP_SR, &srTracePoint);
    napi_value generalTracePoint = nullptr;
    napi_create_int32(env, HITRACE_TP_GENERAL, &generalTracePoint);

    tracePointMap["CS"] = csTracePoint;
    tracePointMap["CR"] = crTracePoint;
    tracePointMap["SS"] = ssTracePoint;
    tracePointMap["SR"] = srTracePoint;
    tracePointMap["GENERAL"] = generalTracePoint;
}

void InitHiTraceCommunicationModeEnum(napi_env env,
    std::map<const char*, napi_value>& commuicationModeMap)
{
    napi_value defaultMode = nullptr;
    napi_create_int32(env, HITRACE_CM_DEFAULT, &defaultMode);
    napi_value threadMode = nullptr;
    napi_create_int32(env, HITRACE_CM_THREAD, &threadMode);
    napi_value processMode = nullptr;
    napi_create_int32(env, HITRACE_CM_PROCESS, &processMode);
    napi_value deviceMode = nullptr;
    napi_create_int32(env, HITRACE_CM_DEVICE, &deviceMode);

    commuicationModeMap["DEFAULT"] = defaultMode;
    commuicationModeMap["THREAD"] = threadMode;
    commuicationModeMap["PROCESS"] = processMode;
    commuicationModeMap["DEVICE"] = deviceMode;
}

void InitConstClassByName(napi_env env, napi_value exports, std::string name)
{
    std::map<const char*, napi_value> propertyMap;
    if (name == HITRACE_FLAG_ENUM_NAME) {
        InitHiTraceFlagEnum(env, propertyMap);
    } else if (name == HITRACE_TRACE_POINT_TYPE_ENUM_NAME) {
        InitHiTraceTracepointTypeEnum(env, propertyMap);
    } else if (name == HITRACE_COMMUNICATION_MODE_ENUM_NAME) {
        InitHiTraceCommunicationModeEnum(env, propertyMap);
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
    InitConstClassByName(env, exports, HITRACE_FLAG_ENUM_NAME);
    InitConstClassByName(env, exports, HITRACE_TRACE_POINT_TYPE_ENUM_NAME);
    InitConstClassByName(env, exports, HITRACE_COMMUNICATION_MODE_ENUM_NAME);
    return exports;
}
} // namespace HiviewDFX
} // namespace OHOS
