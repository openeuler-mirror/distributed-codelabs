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

#include "hisysevent_c.h"

#include <string>

#include "hilog/log.h"
#include "hisysevent.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, 0xD002D08, "HISYSEVENT" };
}

int HiSysEventInnerWrite(const std::string& domain, const std::string& name, HiSysEventEventType type,
    HiSysEventParam params[], size_t size)
{
    HiLog::Info(LABEL, "domain=%{public}s, name=%{public}s, type=%{public}d, param szie=%{public}zu",
        domain.c_str(), name.c_str(), type, size);
    return HiSysEvent::Write(domain, name, HiSysEvent::EventType(type), params, size);
}
} // namespace HiviewDFX
} // namespace OHOS

#ifdef __cplusplus
extern "C" {
#endif

int OH_HiSysEvent_Write(const char* domain, const char* name, HiSysEventEventType type,
    HiSysEventParam params[], size_t size)
{
    if (domain == nullptr) {
        return OHOS::HiviewDFX::ERR_DOMAIN_NAME_INVALID;
    }
    if (name == nullptr) {
        return OHOS::HiviewDFX::ERR_EVENT_NAME_INVALID;
    }
    return OHOS::HiviewDFX::HiSysEventInnerWrite(domain, name, type, params, size);
}

#ifdef __cplusplus
}
#endif