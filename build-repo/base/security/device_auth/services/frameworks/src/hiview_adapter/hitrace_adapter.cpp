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

#include "hitrace_adapter.h"

// #include "hitrace_meter.h"

#ifdef __cplusplus
extern "C" {
#endif

void DevAuthStartTrace(const char *value)
{   
    return;
    if (value == nullptr) {
        return;
    }

    // StartTrace(HITRACE_TAG_APP, value);
}

void DevAuthFinishTrace(void)
{
    return;
    // FinishTrace(HITRACE_TAG_APP);
}

#ifdef __cplusplus
}
#endif