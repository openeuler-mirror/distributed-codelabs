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

#include <memory>
#include <unordered_map>

#include "def.h"
#include "hilog/log.h"
#include "hisysevent_base_manager.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_callback_context.h"
#include "napi_hisysevent_init.h"
#include "napi_hisysevent_listener.h"
#include "napi_hisysevent_querier.h"
#include "napi_hisysevent_util.h"
#include "ret_def.h"

using namespace OHOS::HiviewDFX;

namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, 0xD002D08, "NAPI_HISYSEVENT_JS" };
constexpr char RULES_ATTR[] = "rules";
constexpr size_t WRITE_FUNC_MAX_PARAM_NUM = 2;
constexpr size_t ADD_LISTENER_FUNC_MAX_PARAM_NUM = 1;
constexpr size_t REMOVE_LISTENER_FUNC_MAX_PARAM_NUM = 1;
constexpr size_t QUERY_FUNC_MAX_PARAM_NUM = 3;
constexpr size_t ADD_LISTENER_LISTENER_PARAM_INDEX = 0;
constexpr size_t REMOVE_LISTENER_LISTENER_PARAM_INDEX = 0;
constexpr size_t QUERY_QUERY_ARG_PARAM_INDEX = 0;
constexpr size_t QUERY_RULE_ARRAY_PARAM_INDEX = 1;
constexpr size_t QUERY_QUERIER_PARAM_INDEX = 2;
constexpr long long DEFAULT_TIME_STAMP = -1;
constexpr int DEFAULT_EVENT_COUNT = 1000;
using NAPI_LISTENER_PAIR = std::pair<pid_t, std::shared_ptr<NapiHiSysEventListener>>;
using NAPI_QUERIER_PAIR = std::pair<pid_t, std::shared_ptr<NapiHiSysEventQuerier>>;
std::unordered_map<napi_ref, NAPI_LISTENER_PAIR> listeners;
std::unordered_map<napi_ref, NAPI_QUERIER_PAIR> queriers;
}

static napi_value Write(napi_env env, napi_callback_info info)
{
    size_t paramNum = WRITE_FUNC_MAX_PARAM_NUM;
    napi_value params[WRITE_FUNC_MAX_PARAM_NUM] = {0};
    napi_value thisArg = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &paramNum, params, &thisArg, &data));
    napi_value val = nullptr;
    napi_get_undefined(env, &val);
    if (paramNum < WRITE_FUNC_MAX_PARAM_NUM - 1) {
        HiLog::Error(LABEL,
            "count of parameters is not equal to %{public}zu or %{public}zu.",
            WRITE_FUNC_MAX_PARAM_NUM - 1, WRITE_FUNC_MAX_PARAM_NUM);
        NapiHiSysEventUtil::ThrowParamMandatoryError(env, "info");
        return val;
    }
    HiSysEventAsyncContext* asyncContext = new(std::nothrow) HiSysEventAsyncContext {
        .env = env,
        .asyncWork = nullptr,
        .deferred = nullptr,
    };
    if (asyncContext == nullptr) {
        HiLog::Error(LABEL, "failed to new HiSysEventAsyncContext.");
        return val;
    }
    NapiHiSysEventUtil::ParseHiSysEventInfo(env, params, paramNum, asyncContext->eventInfo);
    asyncContext->eventWroteResult = SUCCESS;
    // set callback function if it exists
    if (paramNum == WRITE_FUNC_MAX_PARAM_NUM) {
        napi_valuetype lastParamType;
        napi_typeof(env, params[paramNum - 1], &lastParamType);
        if (lastParamType == napi_valuetype::napi_function) {
            napi_create_reference(env, params[paramNum - 1], 1, &asyncContext->callback);
        }
    } else if (paramNum > WRITE_FUNC_MAX_PARAM_NUM) {
        HiLog::Warn(LABEL, "count of params is invalid =%{public}d.", static_cast<int>(paramNum));
    }
    // set promise object if callback function is null
    napi_value promise = nullptr;
    napi_get_undefined(env, &promise);
    if (asyncContext->callback == nullptr) {
        napi_create_promise(env, &asyncContext->deferred, &promise);
    }
    NapiHiSysEventAdapter::Write(env, asyncContext);
    return promise;
}

static napi_value AddWatcher(napi_env env, napi_callback_info info)
{
    size_t paramNum = ADD_LISTENER_FUNC_MAX_PARAM_NUM;
    napi_value params[ADD_LISTENER_FUNC_MAX_PARAM_NUM] = {0};
    napi_value thisArg = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &paramNum, params, &thisArg, &data));
    if (paramNum < ADD_LISTENER_FUNC_MAX_PARAM_NUM) {
        HiLog::Error(LABEL, "count of parameters is less than %{public}zu.", ADD_LISTENER_FUNC_MAX_PARAM_NUM);
        NapiHiSysEventUtil::ThrowParamMandatoryError(env, "watcher");
        return nullptr;
    }
    std::vector<ListenerRule> rules;
    napi_value jsRulesVal = NapiHiSysEventUtil::GetPropertyByName(env, params[ADD_LISTENER_LISTENER_PARAM_INDEX],
        RULES_ATTR);
    if (auto ret = NapiHiSysEventUtil::ParseListenerRules(env, jsRulesVal, rules);
        ret != SUCCESS) {
        HiLog::Error(LABEL, "failed to parse watch rules, result code is %{public}d.", ret);
        return nullptr;
    }
    CallbackContext* callbackContext = new CallbackContext();
    callbackContext->env = env;
    callbackContext->threadId = syscall(SYS_gettid);
    napi_create_reference(env, params[ADD_LISTENER_LISTENER_PARAM_INDEX], 1, &callbackContext->ref);
    std::shared_ptr<NapiHiSysEventListener> listener = std::make_shared<NapiHiSysEventListener>(callbackContext);
    auto ret = HiSysEventBaseManager::AddListener(listener, rules);
    if (ret != NAPI_SUCCESS) {
        HiLog::Error(LABEL, "failed to add event listener, result code is %{public}d.", ret);
        NapiHiSysEventUtil::ThrowErrorByRet(env, ret);
        return nullptr;
    }
    listeners[callbackContext->ref] = std::make_pair(callbackContext->threadId, listener);
    return nullptr;
}

static napi_value RemoveWatcher(napi_env env, napi_callback_info info)
{
    size_t paramNum = REMOVE_LISTENER_FUNC_MAX_PARAM_NUM;
    napi_value params[REMOVE_LISTENER_FUNC_MAX_PARAM_NUM] = {0};
    napi_value thisArg = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &paramNum, params, &thisArg, &data));
    if (paramNum < REMOVE_LISTENER_FUNC_MAX_PARAM_NUM) {
        HiLog::Error(LABEL, "count of parameters is less than %{public}zu.", REMOVE_LISTENER_FUNC_MAX_PARAM_NUM);
        NapiHiSysEventUtil::ThrowParamMandatoryError(env, "watcher");
        return nullptr;
    }
    auto iter = NapiHiSysEventUtil::CompareAndReturnCacheItem<NapiHiSysEventListener>(env,
        params[REMOVE_LISTENER_LISTENER_PARAM_INDEX], listeners);
    if (iter == listeners.end()) {
        HiLog::Error(LABEL, "listener not exist.");
        NapiHiSysEventUtil::ThrowErrorByRet(env, ERR_NAPI_LISTENER_NOT_FOUND);
        return nullptr;
    }
    listeners.erase(iter->first);
    if (auto ret = HiSysEventBaseManager::RemoveListener(iter->second.second);
        ret != NAPI_SUCCESS) {
        HiLog::Error(LABEL, "failed to remove event listener, result code is %{public}d.", ret);
        NapiHiSysEventUtil::ThrowErrorByRet(env, ret);
    }
    return nullptr;
}

static napi_value Query(napi_env env, napi_callback_info info)
{
    size_t paramNum = QUERY_FUNC_MAX_PARAM_NUM;
    napi_value params[QUERY_FUNC_MAX_PARAM_NUM] = {0};
    napi_value thisArg = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &paramNum, params, &thisArg, &data));
    if (paramNum < QUERY_FUNC_MAX_PARAM_NUM) {
        std::unordered_map<int32_t, std::string> paramError = {
            {QUERY_QUERY_ARG_PARAM_INDEX, "queryArg"},
            {QUERY_RULE_ARRAY_PARAM_INDEX, "rules"},
            {QUERY_QUERIER_PARAM_INDEX, "querier"},
        };
        HiLog::Error(LABEL, "count of parameters is less than %{public}zu.", QUERY_FUNC_MAX_PARAM_NUM);
        NapiHiSysEventUtil::ThrowParamMandatoryError(env, paramError.at(paramNum));
        return nullptr;
    }
    QueryArg queryArg = { DEFAULT_TIME_STAMP, DEFAULT_TIME_STAMP, DEFAULT_EVENT_COUNT };
    if (auto ret = NapiHiSysEventUtil::ParseQueryArg(env, params[QUERY_QUERY_ARG_PARAM_INDEX], queryArg);
        ret != SUCCESS) {
        HiLog::Error(LABEL, "failed to parse query arg, result code is %{public}d.", ret);
        return nullptr;
    }
    std::vector<QueryRule> rules;
    if (auto ret = NapiHiSysEventUtil::ParseQueryRules(env, params[QUERY_RULE_ARRAY_PARAM_INDEX], rules);
        ret != SUCCESS) {
        HiLog::Error(LABEL, "failed to parse query rules, result code is %{public}d.", ret);
        return nullptr;
    }
    CallbackContext* callbackContext = new CallbackContext();
    callbackContext->env = env;
    callbackContext->threadId = syscall(SYS_gettid);
    napi_create_reference(env, params[QUERY_QUERIER_PARAM_INDEX], 1, &callbackContext->ref);
    std::shared_ptr<NapiHiSysEventQuerier> querier = std::make_shared<NapiHiSysEventQuerier>(callbackContext,
        [] (const napi_env env, const napi_ref ref) {
            napi_value querier = nullptr;
            napi_get_reference_value(env, ref, &querier);
            auto iter = NapiHiSysEventUtil::CompareAndReturnCacheItem<NapiHiSysEventQuerier>(env, querier, queriers);
            if (iter != queriers.end()) {
                queriers.erase(iter->first);
            }
        });
    auto ret = HiSysEventBaseManager::Query(queryArg, rules, querier);
    if (ret != NAPI_SUCCESS) {
        HiLog::Error(LABEL, "failed to query hisysevent, result code is %{public}d.", ret);
        NapiHiSysEventUtil::ThrowErrorByRet(env, ret);
    }
    queriers[callbackContext->ref] = std::make_pair(callbackContext->threadId, querier);
    return nullptr;
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("write", Write),
        DECLARE_NAPI_FUNCTION("addWatcher", AddWatcher),
        DECLARE_NAPI_FUNCTION("removeWatcher", RemoveWatcher),
        DECLARE_NAPI_FUNCTION("query", Query),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(napi_property_descriptor), desc));

    // init EventType class, Event class and Param class
    InitNapiClass(env, exports);

    return exports;
}
EXTERN_C_END

static napi_module _module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "hiSysEvent",
    .nm_priv = ((void*)0),
    .reserved = {0}
};

extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&_module);
}
