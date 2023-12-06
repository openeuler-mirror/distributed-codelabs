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

#include "dev_slinfo_list.h"
#include <pthread.h>
#include "securec.h"
#include "dev_slinfo_adpt.h"
#include "dev_slinfo_log.h"

pthread_mutex_t gMutex;

struct DATASLListParams* InitList(void)
{
    (void)pthread_mutex_lock(&gMutex);
    struct DATASLListParams *list = (struct DATASLListParams *)malloc(sizeof(struct DATASLListParams));
    if (list == NULL) {
        (void)pthread_mutex_unlock(&gMutex);
        return NULL;
    }
    list->next = list;
    list->prev = list;
    list->callbackParams = NULL;
    (void)pthread_mutex_unlock(&gMutex);
    return list;
}

static void UpdateListNode(struct DATASLListParams *newListNode,
    struct DATASLListParams *prevListNode, struct DATASLListParams *nextListNode)
{
    nextListNode->prev = newListNode;
    newListNode->next = nextListNode;
    newListNode->prev = prevListNode;
    prevListNode->next = newListNode;
}

int32_t PushListNode(struct DATASLListParams *list, struct DATASLCallbackParams *callbackParams)
{
    (void)pthread_mutex_lock(&gMutex);
    struct DATASLListParams *newList = (struct DATASLListParams*)malloc(sizeof(struct DATASLListParams));
    if (newList == NULL) {
        (void)pthread_mutex_unlock(&gMutex);
        return DEVSL_ERR_OUT_OF_MEMORY;
    }

    UpdateListNode(newList, list->prev, list);
    newList->callbackParams = callbackParams;
    (void)pthread_mutex_unlock(&gMutex);
    return DEVSL_SUCCESS;
}

void RemoveListNode(struct DATASLListParams *list,  struct DATASLCallbackParams *callbackParams)
{
    (void)pthread_mutex_lock(&gMutex);
    struct DATASLListParams *pList = list->next;
    while (pList != NULL && pList != list) {
        if (CompareUdid(&(pList->callbackParams->queryParams), &(callbackParams->queryParams)) == DEVSL_SUCCESS) {
            pList->prev->next = pList->next;
            pList->next->prev = pList->prev;
            if (pList->callbackParams != NULL) {
                free(pList->callbackParams);
            }
            if (pList != NULL) {
                free(pList);
            }
            break;
        }
        pList = pList->next;
    }
    (void)pthread_mutex_unlock(&gMutex);
}

void ClearList(struct DATASLListParams *list)
{
    (void)pthread_mutex_lock(&gMutex);
    if (list == NULL) {
        (void)pthread_mutex_unlock(&gMutex);
        return;
    }
    struct DATASLListParams *pList = list->next;
    while (pList != NULL && pList != list) {
        struct DATASLListParams *delList = pList;
        pList = pList->next;
        if (delList->callbackParams != NULL) {
            free(delList->callbackParams);
        }
        if (delList != NULL) {
            free(delList);
        }
    }
    if (list->callbackParams != NULL) {
        free(list->callbackParams);
    }
    free(list);
    (void)pthread_mutex_unlock(&gMutex);
}

int32_t GetListLength(struct DATASLListParams *list)
{
    (void)pthread_mutex_lock(&gMutex);
    struct DATASLListParams *pList = list->next;
    int32_t listLength = 0;
    while (pList != NULL && pList != list) {
        listLength++;
        pList = pList->next;
    }
    (void)pthread_mutex_unlock(&gMutex);
    return listLength;
}

void LookupCallback(struct DATASLListParams *list, DEVSLQueryParams *queryParams, int32_t result, uint32_t levelInfo)
{
    struct DATASLCallbackParams tmpCallbackParams;
    (void)memset_s(&tmpCallbackParams, sizeof(struct DATASLCallbackParams), 0, sizeof(struct DATASLCallbackParams));
    int32_t ret = DEVSL_ERROR;
    (void)pthread_mutex_lock(&gMutex);
    struct DATASLListParams *tmpCallback = list->next;
    while (tmpCallback != NULL && tmpCallback != list) {
        struct DATASLListParams *nextCallback = tmpCallback->next;
        ret = CompareUdid(&(tmpCallback->callbackParams->queryParams), queryParams);
        if (ret == DEVSL_SUCCESS) {
            (void)memcpy_s(&tmpCallbackParams.queryParams, sizeof(DEVSLQueryParams),
                queryParams, sizeof(DEVSLQueryParams));
            tmpCallbackParams.callback = tmpCallback->callbackParams->callback;
            tmpCallback->prev->next = tmpCallback->next;
            tmpCallback->next->prev = tmpCallback->prev;
            if (tmpCallback->callbackParams != NULL) {
                free(tmpCallback->callbackParams);
            }
            if (tmpCallback != NULL) {
                free(tmpCallback);
            }
            break;
        }
        tmpCallback = nextCallback;
    }
    (void)pthread_mutex_unlock(&gMutex);
    if (ret == DEVSL_SUCCESS) {
        tmpCallbackParams.callback(&(tmpCallbackParams.queryParams), result, levelInfo);
    }
}

int32_t InitPthreadMutex(void)
{
    int32_t ret;
    ret = pthread_mutex_init(&gMutex, NULL);
    return ret;
}

void DestroyPthreadMutex(void)
{
    pthread_mutex_destroy(&gMutex);
}