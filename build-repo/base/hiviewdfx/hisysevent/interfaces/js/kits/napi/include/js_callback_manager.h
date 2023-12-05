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

#ifndef JS_CALLBACK_MANAGER_H
#define JS_CALLBACK_MANAGER_H

#include <atomic>
#include <mutex>
#include <queue>
#include <tuple>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_callback_context.h"

namespace OHOS {
namespace HiviewDFX {
using TaskQueue = std::queue<std::tuple<CallbackContext*, CALLBACK_FUNC, RELEASE_FUNC>>;
class JsCallbackManager final {
public:
    explicit JsCallbackManager() {}
public:
    void Add(CallbackContext* context, CALLBACK_FUNC callback, RELEASE_FUNC release = nullptr);
    void Release();

private:
    void ImmediateRun(bool needPop = false);
    void Clear(TaskQueue& tasks);

private:
    std::atomic<bool> inCalling = false;
    std::atomic<bool> IsReleased = false;
    std::mutex managerMutex;
    TaskQueue jsCallbacks;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // JS_CALLBACK_MANAGER_H