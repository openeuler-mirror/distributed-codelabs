/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cstdio>
#include <functional>
#include <string>
#include <hilog/log.h>
#include "hitrace_meter.h"
#include "napi_hitrace_meter.h"

using namespace OHOS::HiviewDFX;
namespace {
constexpr int FIRST_ARG_INDEX = 0;
constexpr int SECOND_ARG_INDEX = 1;
constexpr int THIRD_ARG_INDEX = 2;
constexpr int ARGC_NUMBER_TWO = 2;
constexpr int ARGC_NUMBER_THREE = 3;
constexpr uint64_t HITRACE_METER_TAG = 0xD002D33;
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HITRACE_METER_TAG, "HITRACE_METER_JS"};
using STR_NUM_PARAM_FUNC = std::function<bool(std::string, napi_value&)>;

napi_value ParseParams(napi_env& env, napi_callback_info& info, size_t& argc, napi_value* argv)
{
    napi_value thisVar;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    return nullptr;
}

bool TypeCheck(const napi_env& env, const napi_value& value, const napi_valuetype expectType)
{
    napi_valuetype valueType;
    napi_status status = napi_typeof(env, value, &valueType);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to get the type of the argument.");
        return false;
    }
    if (valueType != expectType) {
        HiLog::Error(LABEL, "Type of the parameter is invalid.");
        return false;
    }
    return true;
}

bool ParseStringParam(const napi_env& env, const napi_value& value, std::string& dest)
{
    if (!TypeCheck(env, value, napi_string)) {
        return false;
    }
    constexpr int nameMaxSize = 1024;
    char buf[nameMaxSize] = {0};
    size_t len = 0;
    napi_get_value_string_utf8(env, value, buf, nameMaxSize, &len);
    dest = std::string {buf};
    return true;
}

bool ParseInt32Param(const napi_env& env, const napi_value& value, int& dest)
{
    if (!TypeCheck(env, value, napi_number)) {
        return false;
    }
    napi_get_value_int32(env, value, &dest);
    return true;
}

bool ParseInt64Param(const napi_env& env, const napi_value& value, int64_t& dest)
{
    if (!TypeCheck(env, value, napi_number)) {
        return false;
    }
    napi_get_value_int64(env, value, &dest);
    return true;
}

bool ParseDoubleParam(const napi_env& env, const napi_value& value, double& dest)
{
    if (!TypeCheck(env, value, napi_number)) {
        return false;
    }
    napi_get_value_double(env, value, &dest);
    return true;
}

bool JsStrNumParamsFunc(napi_env& env, napi_callback_info& info, STR_NUM_PARAM_FUNC nativeCall)
{
    size_t argc = ARGC_NUMBER_TWO;
    napi_value argv[ARGC_NUMBER_TWO];
    ParseParams(env, info, argc, argv);
    if (argc != ARGC_NUMBER_TWO) {
        HiLog::Error(LABEL, "Wrong number of parameters.");
        return false;
    }
    std::string name;
    if (!ParseStringParam(env, argv[FIRST_ARG_INDEX], name)) {
        return false;
    }
    if (!nativeCall(name, argv[SECOND_ARG_INDEX])) {
        return false;
    }
    return true;
}
}

static napi_value JSTraceStart(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_NUMBER_THREE;
    napi_value argv[ARGC_NUMBER_THREE];
    ParseParams(env, info, argc, argv);
    NAPI_ASSERT(env, argc == ARGC_NUMBER_TWO || argc == ARGC_NUMBER_THREE, "Wrong number of arguments");
    if (argc != ARGC_NUMBER_TWO && argc != ARGC_NUMBER_THREE) {
        HiLog::Error(LABEL, "Wrong number of parameters.");
    }
    std::string name;
    if (!ParseStringParam(env, argv[FIRST_ARG_INDEX], name)) {
        return nullptr;
    }
    int taskId = 0;
    if (!ParseInt32Param(env, argv[SECOND_ARG_INDEX], taskId)) {
        return nullptr;
    }
    if (argc == ARGC_NUMBER_TWO) {
        StartAsyncTrace(HITRACE_TAG_APP, name, taskId);
    } else {
        double limit = 0.0;
        if (!ParseDoubleParam(env, argv[THIRD_ARG_INDEX], limit)) {
            return nullptr;
        }
        StartAsyncTrace(HITRACE_TAG_APP, name, taskId, limit);
    }
    return nullptr;
}

static napi_value JSTraceFinish(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_NUMBER_TWO;
    napi_value argv[ARGC_NUMBER_TWO];
    napi_value thisVar;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc == ARGC_NUMBER_TWO, "Wrong number of arguments");
    (void)JsStrNumParamsFunc(env, info, [&env] (std::string name, napi_value& nValue) -> bool {
        int taskId = 0;
        if (!ParseInt32Param(env, nValue, taskId)) {
            return false;
        }
        FinishAsyncTrace(HITRACE_TAG_APP, name, taskId);
        return true;
    });
    return nullptr;
}

static napi_value JSTraceCount(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_NUMBER_TWO;
    napi_value argv[ARGC_NUMBER_TWO];
    napi_value thisVar;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc == ARGC_NUMBER_TWO, "Wrong number of arguments");
    (void)JsStrNumParamsFunc(env, info, [&env] (std::string name, napi_value& nValue) -> bool {
        int64_t count = 0;
        if (!ParseInt64Param(env, nValue, count)) {
            return false;
        }
        CountTrace(HITRACE_TAG_APP, name, count);
        return true;
    });
    return nullptr;
}

/*
 * function for module exports
 */
EXTERN_C_START
static napi_value HiTraceMeterInit(napi_env env, napi_value exports)
{
    static napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("startTrace", JSTraceStart),
        DECLARE_NAPI_FUNCTION("finishTrace", JSTraceFinish),
        DECLARE_NAPI_FUNCTION("traceByValue", JSTraceCount),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}
EXTERN_C_END

/*
 * hiTraceMeter module definition
 */
static napi_module hitracemeter_module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = HiTraceMeterInit,
    .nm_modname = "hiTraceMeter",
    .nm_priv = ((void *)0),
    .reserved = {0}
};

/*
 * Module registration
 */
extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&hitracemeter_module);
}
