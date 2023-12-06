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

#include "js_callback_manager.h"

#include "hilog/log.h"
#include "uv.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, 0xD002D08, "JS_CALLBACK_MANAGER" };
constexpr int CONTEXT_INDEX = 0;
constexpr int CALLBACK_FUNC_INDEX = 1;
constexpr int RELEASE_FUNC_INDEX = 2;
void DeleteWork(uv_work_t* work)
{
    if (work != nullptr) {
        delete work;
    }
}

void RunCallback(CallbackContext* context, std::tuple<CallbackContext*, CALLBACK_FUNC, RELEASE_FUNC>& current)
{
    uv_loop_t* loop = nullptr;
    napi_get_uv_event_loop(context->env, &loop);
    if (loop == nullptr) {
        HiLog::Debug(LABEL, "failed to get uv_loop.");
        return;
    }
    context->callback = std::get<CALLBACK_FUNC_INDEX>(current);
    context->release = std::get<RELEASE_FUNC_INDEX>(current);
    uv_work_t* work = new(std::nothrow) uv_work_t();
    if (work == nullptr) {
        HiLog::Debug(LABEL, "uv_work new failed, no memory left.");
        return;
    }
    work->data = reinterpret_cast<void*>(context);
    uv_queue_work(
        loop,
        work,
        [] (uv_work_t* work) {},
        [] (uv_work_t* work, int status) {
            if (work == nullptr || work->data == nullptr) {
                DeleteWork(work);
                return;
            }
            CallbackContext* context = reinterpret_cast<CallbackContext*>(work->data);
            if (context == nullptr) {
                DeleteWork(work);
                return;
            }
            napi_handle_scope scope = nullptr;
            if (context->env == nullptr) {
                DeleteWork(work);
                return;
            }
            napi_open_handle_scope(context->env, &scope);
            if (scope == nullptr) {
                HiLog::Debug(LABEL, "napi scope is null.");
                DeleteWork(work);
                return;
            }
            if (context->callback != nullptr) {
                context->callback(context->env, context->ref, context->threadId);
            }
            napi_close_handle_scope(context->env, scope);
            DeleteWork(work);
            if (context->release != nullptr) {
                context->release(context->threadId);
            }
        });
}
}

void JsCallbackManager::Add(CallbackContext* context, CALLBACK_FUNC callback,
    RELEASE_FUNC release)
{
    {
        if (IsReleased.load(std::memory_order_acquire)) {
            return;
        }
        std::lock_guard<std::mutex> lock(managerMutex);
        jsCallbacks.emplace(std::make_tuple(context, callback, [this, release] (pid_t threadId) {
            if (release == nullptr) {
                this->ImmediateRun(true);
            } else {
                // Destructor of JsCallbackManager will be called in release callback,
                // so no need to call next callback in queue.
                release(threadId);
            }
        }));
        if (inCalling.load(std::memory_order_acquire)) {
            return;
        }
    }
    ImmediateRun();
}

void JsCallbackManager::Release()
{
    IsReleased = true;
    Clear(jsCallbacks);
}

void JsCallbackManager::ImmediateRun(bool needPop)
{
    inCalling = true;
    std::tuple<CallbackContext*, CALLBACK_FUNC, RELEASE_FUNC> current;
    CallbackContext* context;
    {
        if (IsReleased.load(std::memory_order_acquire)) {
            return;
        }
        std::lock_guard<std::mutex> lock(managerMutex);
        if (needPop && !jsCallbacks.empty()) {
            jsCallbacks.pop();
        }
        if (jsCallbacks.empty() && !IsReleased.load(std::memory_order_acquire)) {
            inCalling = false;
            return;
        }
        current = jsCallbacks.front();
        context = std::get<CONTEXT_INDEX>(current);
        if (context == nullptr || IsReleased.load(std::memory_order_acquire)) {
            inCalling = false;
            return;
        }
    }
    if (IsReleased.load(std::memory_order_acquire)) {
        return;
    }
    RunCallback(context, current);
}

void JsCallbackManager::Clear(TaskQueue& tasks)
{
    TaskQueue empty;
    swap(empty, tasks);
}
} // HiviewDFX
} // OHOS