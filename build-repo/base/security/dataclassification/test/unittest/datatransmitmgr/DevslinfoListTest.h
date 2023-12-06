/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef DEV_SLINFO_LIST_TEST_H
#define DEV_SLINFO_LIST_TEST_H

#include <cstdlib>
#include "dev_slinfo_mgr.h"

#ifdef __cplusplus
extern "C" {
#endif

struct DATASLCallbackParams {
    DEVSLQueryParams queryParams;
    HigestSecInfoCallback *callback;
};

struct DATASLListParams {
    struct DATASLCallbackParams *callbackParams;
    struct DATASLListParams *prev;
    struct DATASLListParams *next;
};


int32_t PushListNode(struct DATASLListParams *list, struct DATASLCallbackParams *callbackParams);

void RemoveListNode(struct DATASLListParams *list, struct DATASLCallbackParams *callbackParams);

void ClearList(struct DATASLListParams *list);

int32_t GetListLength(struct DATASLListParams *list);

int32_t InitPthreadMutex(void);

void DestroyPthreadMutex(void);

void LookupCallback(struct DATASLListParams *list, DEVSLQueryParams *queryParams, int32_t result, uint32_t levelInfo);

struct DATASLListParams* InitList(void);

#ifdef __cplusplus
}
#endif
#endif