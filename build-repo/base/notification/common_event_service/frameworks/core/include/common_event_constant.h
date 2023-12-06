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

#ifndef FOUNDATION_EVENT_CESFWK_INNERKITS_INCLUDE_COMMON_EVENT_CONSTANT_H
#define FOUNDATION_EVENT_CESFWK_INNERKITS_INCLUDE_COMMON_EVENT_CONSTANT_H

#include <stdint.h>

namespace OHOS {
namespace EventFwk {
constexpr int8_t ALL_USER = -1;
constexpr int8_t CURRENT_USER = -2;
constexpr int8_t UNDEFINED_USER = -3;
constexpr int8_t SUBSCRIBE_USER_SYSTEM_BEGIN = 0;
constexpr int8_t SUBSCRIBE_USER_SYSTEM_END = 99;
constexpr int8_t UNDEFINED_PID = -1;
constexpr int8_t MAX_HISTORY_SIZE = 100;
constexpr int16_t MAX_SUBSCRIBER_NUM_PER_EVENT = 255;

enum DumpEventType {
    ALL,
    SUBSCRIBER,
    STICKY,
    PENDING,
    HISTORY
};
}  // namespace EventFwk
}  // namespace OHOS
#endif  // FOUNDATION_EVENT_CESFWK_INNERKITS_INCLUDE_COMMON_EVENT_CONSTANT_H