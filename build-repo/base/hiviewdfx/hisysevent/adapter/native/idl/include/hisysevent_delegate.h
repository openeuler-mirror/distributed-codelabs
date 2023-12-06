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

#ifndef HISYSEVENT_DELEGATE_H
#define HISYSEVENT_DELEGATE_H

#include <mutex>
#include <string>
#include <thread>

#include "hisysevent_base_listener.h"
#include "hisysevent_base_query_callback.h"
#include "hisysevent_listener_proxy.h"
#include "hisysevent_listenning_operate.h"
#include "hisysevent_manager.h"
#include "sys_event_query_rule.h"
#include "sys_event_rule.h"
#include "system_ability.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace HiviewDFX {
class HiSysEventDelegate : public HiSysEventListenningOperate {
public:
    HiSysEventDelegate() {}
    virtual ~HiSysEventDelegate();

public:
    static void BinderFunc();

public:
    int32_t AddListener(const std::shared_ptr<HiSysEventBaseListener> listener,
        const std::vector<ListenerRule>& rules);
    int32_t RemoveListener(const std::shared_ptr<HiSysEventBaseListener> listener);
    int32_t Query(const struct QueryArg& arg, const std::vector<QueryRule>& rules,
        const std::shared_ptr<HiSysEventBaseQueryCallback> callback) const;
    int32_t SetDebugMode(const std::shared_ptr<HiSysEventBaseListener> listener,
        const bool mode);

private:
    void ConvertListenerRule(const std::vector<ListenerRule>& rules,
        std::vector<SysEventRule>& sysRules) const;
    void ConvertQueryRule(const std::vector<QueryRule>& rules,
        std::vector<SysEventQueryRule>& sysRules) const;
    sptr<IRemoteObject> GetSysEventService() const;

private:
    sptr<HiSysEventListenerProxy> spListenerCallBack = nullptr;
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // HISYSEVENT_DELEGATE_H
