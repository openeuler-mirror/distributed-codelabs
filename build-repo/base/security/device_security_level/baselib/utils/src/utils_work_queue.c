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

#include "utils_work_queue.h"

#include <stddef.h>
#include <pthread.h>
#include <sys/prctl.h>

#include "securec.h"

#include "utils_list.h"
#include "utils_mem.h"

#define RUN 0
#define DIE 1

typedef struct WorkQueue {
    ListHead head;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    volatile int32_t state;
    uint32_t capacity;
    uint32_t size;
    pthread_t pthreadId;
    const char *name;
} WorkQueue;

typedef struct {
    ListNode linkNode;
    WorkProcess process; // callback func
    uint32_t dataLen;
    uint8_t *dataBuff; // user data ptr
} Worker;

static void *WorkQueueThread(void *data)
{
    WorkQueue *queue = (WorkQueue *)data;
    Worker *worker = NULL;

    prctl(PR_SET_NAME, queue->name, 0, 0, 0);

    (void)pthread_mutex_lock(&queue->mutex);
    while (queue->state == RUN) {
        while ((IsEmptyList(&queue->head)) && (queue->state == RUN)) {
            pthread_cond_wait(&queue->cond, &queue->mutex);
        }
        // need to check again
        if (queue->state != RUN) {
            break;
        }

        worker = LIST_ENTRY(queue->head.next, Worker, linkNode);
        RemoveListNode(&worker->linkNode);
        queue->size--;

        pthread_mutex_unlock(&queue->mutex);
        worker->process(worker->dataBuff, worker->dataLen);
        FREE(worker);
        (void)pthread_mutex_lock(&queue->mutex);
    }

    // now the queue is stopped, just remove the nodes.
    while (!IsEmptyList(&queue->head)) {
        worker = LIST_ENTRY(queue->head.next, Worker, linkNode);
        RemoveListNode(&worker->linkNode);
        queue->size--;
        FREE(worker);
    }

    pthread_mutex_unlock(&queue->mutex);
    return NULL;
}

WorkQueue *CreateWorkQueue(uint32_t capacity, const char *name)
{
    WorkQueue *queue = MALLOC(sizeof(WorkQueue));
    if (queue == NULL) {
        return NULL;
    }
    (void)memset_s(queue, sizeof(WorkQueue), 0, sizeof(WorkQueue));

    InitListHead(&(queue->head));
    queue->state = RUN;
    queue->capacity = capacity;
    queue->size = 0;
    queue->name = name;

    int32_t iRet = pthread_mutex_init(&(queue->mutex), NULL);
    if (iRet != 0) {
        FREE(queue);
        return NULL;
    }

    iRet = pthread_cond_init(&queue->cond, NULL);
    if (iRet != 0) {
        (void)pthread_mutex_destroy(&(queue->mutex));
        FREE(queue);
        return NULL;
    }

    iRet = pthread_create(&queue->pthreadId, NULL, WorkQueueThread, queue);
    if (iRet != 0) {
        (void)pthread_cond_destroy(&(queue->cond));
        (void)pthread_mutex_destroy(&(queue->mutex));
        FREE(queue);
        return NULL;
    }

    return queue;
}

uint32_t DestroyWorkQueue(WorkQueue *queue)
{
    if (queue == NULL) {
        return WORK_QUEUE_NULL_PTR;
    }

    (void)pthread_mutex_lock(&queue->mutex);
    queue->state = DIE;
    int32_t iRet = pthread_cond_broadcast(&queue->cond);
    if (iRet != 0) {
        (void)pthread_mutex_unlock(&queue->mutex);
        return WORK_QUEUE_THREAD_COND_ERR;
    }
    (void)pthread_mutex_unlock(&queue->mutex);

    iRet = pthread_join(queue->pthreadId, NULL);
    if (iRet != 0) {
        return WORK_QUEUE_THREAD_JOIN_ERR;
    }

    FREE(queue);
    return WORK_QUEUE_OK;
}

uint32_t QueueWork(WorkQueue *queue, WorkProcess process, uint8_t *data, uint32_t length)
{
    if ((queue == NULL) || (process == NULL)) {
        return WORK_QUEUE_NULL_PTR;
    }
    if (queue->state != RUN) {
        return WORK_QUEUE_STATE_ERR;
    }
    if (queue->size >= queue->capacity) {
        return WORK_QUEUE_FULL;
    }

    Worker *worker = MALLOC(sizeof(Worker));
    if (worker == NULL) {
        return WORK_QUEUE_MALLOC_ERR;
    }
    (void)memset_s(worker, sizeof(Worker), 0, sizeof(Worker));

    InitListHead(&worker->linkNode);
    worker->dataLen = length;
    worker->dataBuff = data;
    worker->process = process;

    (void)pthread_mutex_lock(&queue->mutex);
    AddListNodeBefore(&worker->linkNode, &queue->head);
    queue->size++;

    (void)pthread_mutex_unlock(&queue->mutex);
    (void)pthread_cond_broadcast(&queue->cond);
    return WORK_QUEUE_OK;
}
