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

#ifndef HISYSEVENT_LISTENER_PROXY_H
#define HISYSEVENT_LISTENER_PROXY_H

#include <string>
#include <vector>

#include "hisysevent_base_listener.h"
#include "sys_event_callback_stub.h"

namespace OHOS {
namespace HiviewDFX {
class CallbackDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    explicit CallbackDeathRecipient(const std::shared_ptr<HiSysEventBaseListener> listener)
        : eventListener(listener) {}
    virtual ~CallbackDeathRecipient() {}

public:
    void OnRemoteDied(const wptr<IRemoteObject> &object) override;
    std::shared_ptr<HiSysEventBaseListener> GetEventListener() const;

private:
    std::shared_ptr<HiSysEventBaseListener> eventListener;
};

class HiSysEventListenerProxy : public SysEventCallbackStub {
public:
    explicit HiSysEventListenerProxy(const std::shared_ptr<HiSysEventBaseListener> listener)
        : callbackDeathRecipient(new CallbackDeathRecipient(listener)) {}
    virtual ~HiSysEventListenerProxy() {}

public:
    void Handle(const std::u16string& domain, const std::u16string& eventName, uint32_t eventType,
        const std::u16string& eventDetail) override;
    sptr<CallbackDeathRecipient> GetCallbackDeathRecipient() const;
    std::shared_ptr<HiSysEventBaseListener> GetEventListener() const;

private:
    sptr<CallbackDeathRecipient> callbackDeathRecipient;
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // HISYSEVENT_LISTENER_PROXY_H
