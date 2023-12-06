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

#ifndef FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_PUBLIC_MANAGER_H
#define FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_PUBLIC_MANAGER_H

#include <map>
#include <mutex>
#include <vector>
#include <stdint.h>
#include "singleton.h"

namespace OHOS {
namespace EventFwk {
class PublishManager : public DelayedSingleton<PublishManager> {
public:
    PublishManager();

    ~PublishManager();

    /**
     * Checks for flood attacks.
     *
     * @param appUid Indicates the uid of the event sender.
     * @return Returns true if success; false otherwise.
     */
    bool CheckIsFloodAttack(pid_t appUid);

private:
    std::mutex mutex_;
    std::map<pid_t, std::vector<int64_t>> floodAttackAppStatistics_;
    const uint32_t FLOOD_ATTACK_NUMBER_MAX = 20;  // Frequency of decision
    const int64_t FLOOD_ATTACK_INTERVAL_MAX = 5;  // Period of decision (unit: millisecond)
};
}  // namespace EventFwk
}  // namespace OHOS

#endif  // FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_PUBLIC_MANAGER_H