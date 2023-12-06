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

#ifndef HISYSEVENT_BASE_MANAGER_H
#define HISYSEVENT_BASE_MANAGER_H

#include <string>
#include <vector>

#include "hisysevent_base_listener.h"
#include "hisysevent_base_query_callback.h"
#include "hisysevent_rules.h"

namespace OHOS {
namespace HiviewDFX {
class HiSysEventBaseManager {
public:
    HiSysEventBaseManager() = default;
    ~HiSysEventBaseManager() {}

public:
    static int32_t AddListener(std::shared_ptr<HiSysEventBaseListener> listener,
        std::vector<ListenerRule>& rules);
    static int32_t RemoveListener(std::shared_ptr<HiSysEventBaseListener> listener);
    static int32_t Query(struct QueryArg& arg, std::vector<QueryRule>& rules,
        std::shared_ptr<HiSysEventBaseQueryCallback> callback);
    static int32_t SetDebugMode(std::shared_ptr<HiSysEventBaseListener> listener, bool mode);
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // HISYSEVENT_INNER_MANAGER_H

