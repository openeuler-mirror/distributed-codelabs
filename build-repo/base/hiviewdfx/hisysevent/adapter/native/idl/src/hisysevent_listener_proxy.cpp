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

#include "hisysevent_listener_proxy.h"

#include "string_ex.h"

namespace OHOS {
namespace HiviewDFX {
void HiSysEventListenerProxy::Handle(const std::u16string& domain, const std::u16string& eventName,
    uint32_t eventType, const std::u16string& eventDetail)
{
    auto eventListener = GetEventListener();
    if (eventListener != nullptr) {
        eventListener->OnEvent(Str16ToStr8(domain), Str16ToStr8(eventName),
            eventType, Str16ToStr8(eventDetail));
    }
}

sptr<CallbackDeathRecipient> HiSysEventListenerProxy::GetCallbackDeathRecipient() const
{
    return callbackDeathRecipient;
}

std::shared_ptr<HiSysEventBaseListener> HiSysEventListenerProxy::GetEventListener() const
{
    if (callbackDeathRecipient != nullptr) {
        return callbackDeathRecipient->GetEventListener();
    }
    return nullptr;
}

void CallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &object)
{
    auto eventListener = GetEventListener();
    if (eventListener != nullptr) {
        eventListener->OnServiceDied();
    }
}

std::shared_ptr<HiSysEventBaseListener> CallbackDeathRecipient::GetEventListener() const
{
    return eventListener;
}
} // namespace HiviewDFX
} // namespace OHOS
