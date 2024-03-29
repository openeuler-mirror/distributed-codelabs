/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef CLIB_TYPES_H
#define CLIB_TYPES_H

#include <stdint.h>
#include <stdio.h>

typedef uint32_t HcBool;
#define HC_TRUE 1
#define HC_FALSE 0

#ifndef NULL
#define NULL 0
#endif

#ifdef __cplusplus
extern "C" {
#endif

void *ClibMalloc(uint32_t size, char val);
void ClibFree(void *addr);

#ifdef __cplusplus
}
#endif
#endif
