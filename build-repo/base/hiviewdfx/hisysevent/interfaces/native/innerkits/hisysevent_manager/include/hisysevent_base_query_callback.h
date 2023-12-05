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

#ifndef HISYSEVENT_BASE_QUERY_CALLBACK_H
#define HISYSEVENT_BASE_QUERY_CALLBACK_H

#include <string>
#include <vector>

#include "hisysevent_record.h"
#include "hisysevent_query_callback.h"

namespace OHOS {
namespace HiviewDFX {
class HiSysEventBaseQueryCallback {
public:
    HiSysEventBaseQueryCallback() = default;
    HiSysEventBaseQueryCallback(std::shared_ptr<HiSysEventQueryCallback> callback): callback(callback) {}
    virtual ~HiSysEventBaseQueryCallback() {}

public:
    virtual void OnQuery(const ::std::vector<std::string>& sysEvents,
        const std::vector<int64_t>& seqs)
    {
        if (callback != nullptr) {
            auto records = std::make_shared<std::vector<HiSysEventRecord>>();
            for_each(sysEvents.cbegin(), sysEvents.cend(), [&records](const std::string& content) {
                records->emplace_back(HiSysEventRecord(content));
            });
            callback->OnQuery(records);
        }
    }

    virtual void OnComplete(int32_t reason, int32_t total)
    {
        if (callback != nullptr) {
            callback->OnComplete(reason, total);
        }
    }

    virtual void OnComplete(int32_t reason, int32_t total, int64_t seq)
    {
        OnComplete(reason, total);
    }

private:
    HiSysEventBaseQueryCallback(const HiSysEventBaseQueryCallback&) = delete;
    HiSysEventBaseQueryCallback& operator=(const HiSysEventBaseQueryCallback&) = delete;
    HiSysEventBaseQueryCallback(const HiSysEventBaseQueryCallback&&) = delete;
    HiSysEventBaseQueryCallback& operator=(const HiSysEventBaseQueryCallback&&) = delete;

private:
    std::shared_ptr<HiSysEventQueryCallback> callback;
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // HISYSEVENT_BASE_QUERY_CALLBACK_H
