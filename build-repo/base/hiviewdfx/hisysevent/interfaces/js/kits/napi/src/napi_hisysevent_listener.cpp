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

#include "napi_hisysevent_listener.h"

#include "hilog/log.h"
#include "napi_hisysevent_util.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, 0xD002D08, "NAPI_HISYSEVENT_LISTENER" };
constexpr char ON_EVENT_ATTR[] = "onEvent";
constexpr char ON_SERVICE_DIED_ATTR[] = "onServiceDied";
constexpr size_t ON_EVENT_PARAM_COUNT = 1;
constexpr size_t ON_SERVICE_DIED_PARAM_COUNT = 0;
}

NapiHiSysEventListener::NapiHiSysEventListener(CallbackContext* context)
{
    callbackContext = context;
    jsCallbackManager = std::make_shared<JsCallbackManager>();
}

NapiHiSysEventListener::~NapiHiSysEventListener()
{
    if (jsCallbackManager != nullptr) {
        jsCallbackManager->Release();
    }
    if (callbackContext->threadId == syscall(SYS_gettid)) {
        napi_delete_reference(callbackContext->env, callbackContext->ref);
    }
    delete callbackContext;
}

void NapiHiSysEventListener::OnEvent(const std::string& domain, const std::string& eventName, const int eventType,
    const std::string& eventDetail)
{
    jsCallbackManager->Add(callbackContext,
        [this, domain, eventName, eventType, eventDetail] (const napi_env env, const napi_ref ref, pid_t threadId) {
            if (threadId != syscall(SYS_gettid)) {
                return;
            }
            napi_value sysEventInfo = nullptr;
            NapiHiSysEventUtil::CreateHiSysEventInfoJsObject(env, eventDetail, sysEventInfo);
            napi_value argv[ON_EVENT_PARAM_COUNT] = {sysEventInfo};
            napi_value listener = nullptr;
            napi_status status = napi_get_reference_value(env, ref, &listener);
            if (status != napi_ok) {
                HiLog::Error(LABEL, "failed to get JS reference of event listener.");
            }
            napi_value onEvent = NapiHiSysEventUtil::GetPropertyByName(env, listener, ON_EVENT_ATTR);
            napi_value ret = nullptr;
            status = napi_call_function(env, listener, onEvent, ON_EVENT_PARAM_COUNT, argv, &ret);
            if (status != napi_ok) {
                HiLog::Error(LABEL, "failed to call onEvent JS function.");
            }
        });
}

void NapiHiSysEventListener::OnServiceDied()
{
    jsCallbackManager->Add(callbackContext,
        [this] (const napi_env env, const napi_ref ref, pid_t threadId) {
            if (threadId != syscall(SYS_gettid)) {
                return;
            }
            napi_value listener = nullptr;
            napi_status status = napi_get_reference_value(env, ref, &listener);
            if (status != napi_ok) {
                HiLog::Error(LABEL, "failed to get JS reference of event listener.");
            }
            napi_value onServiceDied = NapiHiSysEventUtil::GetPropertyByName(env, listener, ON_SERVICE_DIED_ATTR);
            napi_value ret = nullptr;
            status = napi_call_function(env, listener, onServiceDied, ON_SERVICE_DIED_PARAM_COUNT,
                nullptr, &ret);
            if (status != napi_ok) {
                HiLog::Error(LABEL, "failed to call onServiceDied JS function.");
            }
        });
}
} // HiviewDFX
} // OHOS