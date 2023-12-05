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

#include "hisysevent_base_manager.h"

#include "hilog/log.h"
#include "hisysevent_delegate.h"
#include "ret_code.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, 0xD002D08, "HISYSEVENT_BASE_MANAGER" };
}

int32_t HiSysEventBaseManager::AddListener(std::shared_ptr<HiSysEventBaseListener> listener,
    std::vector<ListenerRule>& rules)
{
    if (listener == nullptr) {
        HiLog::Warn(LABEL, "no need to add a listener which is null.");
        return ERR_LISTENER_NOT_EXIST;
    }
    if (listener->listenerProxy == nullptr) {
        listener->listenerProxy = new HiSysEventDelegate();
    }
    return listener->listenerProxy->AddListener(listener, rules);
}

int32_t HiSysEventBaseManager::RemoveListener(std::shared_ptr<HiSysEventBaseListener> listener)
{
    if (listener == nullptr || listener->listenerProxy == nullptr) {
        HiLog::Warn(LABEL, "no need to remove a base listener which has not been added.");
        return ERR_LISTENER_NOT_EXIST;
    }
    auto ret = listener->listenerProxy->RemoveListener(listener);
    delete listener->listenerProxy;
    listener->listenerProxy = nullptr;
    return ret;
}

int32_t HiSysEventBaseManager::Query(struct QueryArg& arg, std::vector<QueryRule>& rules,
    std::shared_ptr<HiSysEventBaseQueryCallback> callback)
{
    auto proxy = std::make_unique<HiSysEventDelegate>();
    if (proxy != nullptr) {
        return proxy->Query(arg, rules, callback);
    }
    return ERR_LISTENER_NOT_EXIST;
}

int32_t HiSysEventBaseManager::SetDebugMode(std::shared_ptr<HiSysEventBaseListener> listener, bool mode)
{
    if (listener == nullptr || listener->listenerProxy == nullptr) {
        HiLog::Warn(LABEL, "no need to set debug mode on a base listener which has not been added.");
        return ERR_LISTENER_NOT_EXIST;
    }
    return listener->listenerProxy->SetDebugMode(listener, mode);
}
} // namespace HiviewDFX
} // namespace OHOS
