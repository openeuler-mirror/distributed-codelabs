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

#include "hisysevent_query_proxy.h"

#include "string_ex.h"

namespace OHOS {
namespace HiviewDFX {
void HiSysEventQueryProxy::OnQuery(const ::std::vector<std::u16string>& sysEvents,
    const ::std::vector<int64_t>& seq)
{
    if (queryCallback != nullptr) {
        std::vector<std::string> destSysEvents;
        for_each(sysEvents.cbegin(), sysEvents.cend(), [&destSysEvents](const std::u16string& sysEvent) {
            destSysEvents.emplace_back(Str16ToStr8(sysEvent));
        });
        queryCallback->OnQuery(destSysEvents, seq);
    }
}

void HiSysEventQueryProxy::OnComplete(int32_t reason, int32_t total, int64_t seq)
{
    if (queryCallback != nullptr) {
        queryCallback->OnComplete(reason, total, seq);
    }
}
} // namespace HiviewDFX
} // namespace OHOS
