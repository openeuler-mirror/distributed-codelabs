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

#ifndef SEC_UTILS_WORK_QUEUE_H
#define SEC_UTILS_WORK_QUEUE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define WORK_QUEUE_OK 0
#define WORK_QUEUE_ERROR (6600)
#define WORK_QUEUE_NULL_PTR (WORK_QUEUE_ERROR + 0x1)
#define WORK_QUEUE_MALLOC_ERR (WORK_QUEUE_ERROR + 0x2)
#define WORK_QUEUE_THREAD_COND_ERR (WORK_QUEUE_ERROR + 0x3)
#define WORK_QUEUE_THREAD_JOIN_ERR (WORK_QUEUE_ERROR + 0x4)
#define WORK_QUEUE_STATE_ERR (WORK_QUEUE_ERROR + 0x5)
#define WORK_QUEUE_FULL (WORK_QUEUE_ERROR + 0x6)

typedef struct WorkQueue WorkQueue;

typedef void (*WorkProcess)(const uint8_t *data, uint32_t len);

uint32_t QueueWork(WorkQueue *queue, WorkProcess process, uint8_t *data, uint32_t length);

WorkQueue *CreateWorkQueue(uint32_t capacity, const char *name);

uint32_t DestroyWorkQueue(WorkQueue *queue);

#ifdef __cplusplus
}
#endif
#endif /* SEC_UTILS_WORK_QUEUE_H */