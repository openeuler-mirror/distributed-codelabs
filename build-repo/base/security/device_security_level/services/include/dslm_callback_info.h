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

#ifndef DSLM_CALLBACK_INFO_H
#define DSLM_CALLBACK_INFO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DslmCallbackInfo {
    uint32_t level;
    const uint8_t *extraBuff;
    uint32_t extraLen;
} DslmCallbackInfo;

typedef void RequestCallback(uint32_t owner, uint32_t cookie, uint32_t result, const DslmCallbackInfo *info);

#ifdef __cplusplus
}
#endif

#endif // DSLM_CALLBACK_INFO_H
