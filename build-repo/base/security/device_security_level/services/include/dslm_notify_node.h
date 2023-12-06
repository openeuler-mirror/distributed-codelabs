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

#ifndef DSLM_NOTIFY_NODE_H
#define DSLM_NOTIFY_NODE_H

#include <stdint.h>

#include "utils_list.h"

#include "dslm_callback_info.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DslmNotifyListNode {
    ListNode linkNode;
    uint64_t start;
    uint64_t stop;
    uint32_t keep;
    uint32_t owner;
    uint32_t cookie;
    uint32_t result;
    uint32_t extra;
    RequestCallback *requestCallback;
} DslmNotifyListNode;

#ifdef __cplusplus
}
#endif

#endif // DSLM_NOTIFY_NODE_H
