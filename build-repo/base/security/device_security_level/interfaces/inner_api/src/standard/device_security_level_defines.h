/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef DEVICE_SECURITY_LEVEL_DEFINES_H
#define DEVICE_SECURITY_LEVEL_DEFINES_H

#include <functional>

#include "hilog/log.h"

#include "device_security_defines.h"
#include "device_security_info.h"

struct DeviceSecurityInfo {
    uint32_t magicNum {0};
    uint32_t result {0};
    uint32_t level {0};
};

namespace OHOS {
namespace Security {
namespace DeviceSecurityLevel {
static constexpr uint64_t SECURITY_MAGIC = 0xABCD1234;

static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, 0xD002F00, "DSLM_SDK"};

using ResultCallback = std::function<void(const DeviceIdentify *identify, struct DeviceSecurityInfo *info)>;
} // namespace DeviceSecurityLevel
} // namespace Security
} // namespace OHOS

#endif // DEVICE_SECURITY_LEVEL_DEFINES_H
