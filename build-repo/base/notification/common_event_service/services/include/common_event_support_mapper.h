/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_COMMON_EVENT_SUPPORT_MAPPER_H
#define FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_COMMON_EVENT_SUPPORT_MAPPER_H

#include <map>
#include <string>
#include "singleton.h"

namespace OHOS {
namespace EventFwk {
class CommonEventSupportMapper : public DelayedSingleton<CommonEventSupportMapper> {
public:
    CommonEventSupportMapper();

    ~CommonEventSupportMapper() = default;

    /**
     * Inits mapper.
     *
     */
    void Init();

    /**
     * Gets mapped support.
     *
     * @param support the input event name
     * @param mappedSupport the output event name
     * @return Returns true if successful; false otherwise.
     */
    bool GetMappedSupport(const std::string &support, std::string &mappedSupport);
private:
    std::map<std::string, std::string> map_;
};
}  // namespace EventFwk
}  // namespace OHOS

#endif  // FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_COMMON_EVENT_SUPPORT_MAPPER_H