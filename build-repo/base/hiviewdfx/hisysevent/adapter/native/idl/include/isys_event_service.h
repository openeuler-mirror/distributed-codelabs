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

#ifndef OHOS_HIVIEWDFX_ISYS_EVENT_SERVICE_H
#define OHOS_HIVIEWDFX_ISYS_EVENT_SERVICE_H

#include <vector>

#include "iquery_sys_event_callback.h"
#include "iremote_broker.h"
#include "isys_event_callback.h"
#include "query_argument.h"
#include "sys_event_query_rule.h"
#include "sys_event_rule.h"

namespace OHOS {
namespace HiviewDFX {
class ISysEventService : public IRemoteBroker {
public:
    virtual int32_t AddListener(const std::vector<SysEventRule>& rules,
        const sptr<ISysEventCallback>& callback) = 0;
    virtual int32_t RemoveListener(const sptr<ISysEventCallback>& callback) = 0;
    virtual int32_t Query(const QueryArgument& queryArgument, const std::vector<SysEventQueryRule>& rules,
        const sptr<IQuerySysEventCallback>& callback) = 0;
    virtual int32_t SetDebugMode(const sptr<ISysEventCallback>& callback, bool mode) = 0;

    enum {
        ADD_SYS_EVENT_LISTENER = 0,
        REMOVE_SYS_EVENT_LISTENER,
        QUERY_SYS_EVENT,
        SET_DEBUG_MODE
    };

public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.hiviewdfx.ISysEventService");
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // OHOS_HIVIEWDFX_ISYS_EVENT_SERVICE_H
