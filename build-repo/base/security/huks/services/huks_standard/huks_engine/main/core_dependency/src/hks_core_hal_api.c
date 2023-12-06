/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <time.h>
#include <unistd.h>

#include "hks_core_hal_api.h"

#define S_TO_MS 1000
#define MS_TO_NS 1000000

int32_t HksCoreHalElapsedRealTime(uint64_t *timestampMs)
{
    struct timespec curTime;
    int32_t ret = clock_gettime(CLOCK_MONOTONIC, &curTime);
    if (ret != 0) {
        return ret;
    }

    if ((curTime.tv_sec >= ((UINT64_MAX - S_TO_MS) / S_TO_MS)) || (curTime.tv_nsec / MS_TO_NS >= S_TO_MS)) {
        return -1;
    }

    *timestampMs = (uint64_t)(curTime.tv_sec * S_TO_MS + curTime.tv_nsec / MS_TO_NS);
    return ret;
}