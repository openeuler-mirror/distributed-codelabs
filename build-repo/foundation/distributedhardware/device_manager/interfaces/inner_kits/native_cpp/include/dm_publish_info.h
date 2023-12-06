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

#ifndef OHOS_DM_PUBLISH_INFO_H
#define OHOS_DM_PUBLISH_INFO_H

#include <stdbool.h>

#include "dm_subscribe_info.h"

namespace OHOS {
namespace DistributedHardware {
typedef struct {
    /** Service ID */
    int32_t publishId;
    /** Discovery mode for service publishing. For details, see {@link Discovermode}. */
    DmDiscoverMode mode;
    /** Service publishing frequency. For details, see {@link ExchangeFreq}. */
    DmExchangeFreq freq;
    /** Discovery ranging. For details, see {@link PublishInfo}. */
    bool ranging;
} DmPublishInfo;
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_PUBLISH_INFO_H
