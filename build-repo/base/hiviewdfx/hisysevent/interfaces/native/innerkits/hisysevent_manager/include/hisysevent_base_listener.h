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

#ifndef HISYSEVENT_BASE_LISTENER_H
#define HISYSEVENT_BASE_LISTENER_H

#include <string>
#include <memory>

#include "hisysevent_record.h"
#include "hisysevent_listener.h"

namespace OHOS {
namespace HiviewDFX {
class HiSysEventListenningOperate;
class HiSysEventBaseListener {
public:
    HiSysEventBaseListener() = default;
    HiSysEventBaseListener(std::shared_ptr<HiSysEventListener> listener): listener(listener) {}
    virtual ~HiSysEventBaseListener() {}

public:
    virtual void OnEvent(const std::string& domain, const std::string& eventName, const int eventType,
        const std::string& eventDetail)
    {
        if (listener != nullptr) {
            std::shared_ptr<HiSysEventRecord> sysEvent = std::make_shared<HiSysEventRecord>(eventDetail);
            listener->OnEvent(sysEvent);
        }
    }

    virtual void OnServiceDied()
    {
        if (listener != nullptr) {
            listener->OnServiceDied();
        }
    }

protected:
    HiSysEventListenningOperate* listenerProxy = nullptr;

private:
    HiSysEventBaseListener(const HiSysEventBaseListener&) = delete;
    HiSysEventBaseListener& operator=(const HiSysEventBaseListener&) = delete;
    HiSysEventBaseListener(const HiSysEventBaseListener&&) = delete;
    HiSysEventBaseListener& operator=(const HiSysEventBaseListener&&) = delete;

private:
    std::shared_ptr<HiSysEventListener> listener;

friend class HiSysEventBaseManager;
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // HISYSEVENT_BASE_LISTENER_H
