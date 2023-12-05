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

#include "hks_session_manager.h"

#include <pthread.h>
#include <sched.h>
#include <securec.h>
#include <stdio.h>

#include "hks_log.h"
#include "hks_mem.h"
#include "hks_param.h"
#include "hks_template.h"
#include "huks_access.h"
#include "securec.h"

#define MAX_OPERATIONS_COUNT 15

static struct DoubleList g_operationList = { &g_operationList, &g_operationList };
static uint32_t g_operationCount = 0;
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;

static void DeleteKeyNode(uint64_t operationHandle)
{
    uint8_t *handle = (uint8_t *)HksMalloc(sizeof(uint64_t));
    if (handle == NULL) {
        HKS_LOG_E("malloc failed");
        return;
    }
    (void)memcpy_s(handle, sizeof(uint64_t), &operationHandle, sizeof(uint64_t));
    struct HksBlob handleBlob = { sizeof(uint64_t), handle };

    struct HksParamSet *paramSet = NULL;
    if (HksInitParamSet(&paramSet) != HKS_SUCCESS) {
        HksFree(handle);
        return;
    }

    (void)HuksAccessAbort(&handleBlob, paramSet);

    HksFreeParamSet(&paramSet);
    HksFree(handle);
}

/* Need to lock before calling FreeOperation */
static void FreeOperation(struct HksOperation **operation)
{
    if (operation == NULL || *operation == NULL) {
        return;
    }
    RemoveDoubleListNode(&(*operation)->listHead);
    HKS_FREE_BLOB((*operation)->processInfo.userId);
    HKS_FREE_BLOB((*operation)->processInfo.processName);
    HKS_FREE_PTR(*operation);
}

/* Need to lock before calling DeleteFirstAbortableOperation */
static bool DeleteFirstAbortableOperation(void)
{
    struct HksOperation *operation = NULL;

    HKS_DLIST_ITER(operation, &g_operationList) {
        if (operation != NULL && operation->abortable) {
            DeleteKeyNode(operation->handle);
            FreeOperation(&operation);
            --g_operationCount;
            HKS_LOG_I("delete operation count:%" LOG_PUBLIC "u", g_operationCount);
            return true;
        }
    }
    return false;
}

static int32_t AddOperation(struct HksOperation *operation)
{
    pthread_mutex_lock(&g_lock);
    if (g_operationCount >= MAX_OPERATIONS_COUNT) {
        HKS_LOG_I("maximum number of sessions reached: delete oldest session.");
        if (!DeleteFirstAbortableOperation()) {
            pthread_mutex_unlock(&g_lock);
            HKS_LOG_E("not found abortable session");
            return HKS_ERROR_SESSION_REACHED_LIMIT;
        }
    }

    AddNodeAtDoubleListTail(&g_operationList, &operation->listHead);
    ++g_operationCount;
    HKS_LOG_I("add operation count:%" LOG_PUBLIC "u", g_operationCount);
    pthread_mutex_unlock(&g_lock);
    return HKS_SUCCESS;
}

static int32_t ConstructOperationProcessInfo(const struct HksProcessInfo *processInfo, struct HksOperation *operation)
{
    /* userIdLen and processNameLen have been checked by calling function */
    uint32_t userIdLen = processInfo->userId.size;
    uint32_t processNameLen = processInfo->processName.size;

    uint8_t *userId = (uint8_t *)HksMalloc(userIdLen);
    HKS_IF_NULL_LOGE_RETURN(userId, HKS_ERROR_MALLOC_FAIL, "malloc operation userId failed")

    uint8_t *processName = (uint8_t *)HksMalloc(processNameLen);
    if (processName == NULL) {
        HKS_LOG_E("malloc operation process name failed");
        HKS_FREE_PTR(userId);
        return HKS_ERROR_MALLOC_FAIL;
    }

    (void)memcpy_s(userId, userIdLen, processInfo->userId.data, userIdLen);
    (void)memcpy_s(processName, processNameLen, processInfo->processName.data, processNameLen);

    operation->processInfo.userId.size = userIdLen;
    operation->processInfo.userId.data = userId;
    operation->processInfo.processName.size = processNameLen;
    operation->processInfo.processName.data = processName;
    operation->accessTokenId = processInfo->accessTokenId;
    return HKS_SUCCESS;
}

static int32_t ConstructOperationHandle(const struct HksBlob *operationHandle, uint64_t *handle)
{
    if (operationHandle->size < sizeof(*handle)) {
        HKS_LOG_E("invalid handle size");
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    if (memcpy_s(handle, sizeof(*handle), operationHandle->data, operationHandle->size) != EOK) {
        HKS_LOG_E("copy handle failed");
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }

    return HKS_SUCCESS;
}

int32_t CreateOperation(const struct HksProcessInfo *processInfo, const struct HksBlob *operationHandle,
    bool abortable)
{
    struct HksOperation *operation = (struct HksOperation *)HksMalloc(sizeof(struct HksOperation));
    HKS_IF_NULL_LOGE_RETURN(operation, HKS_ERROR_MALLOC_FAIL, "malloc hks operation failed")

    int32_t ret = ConstructOperationProcessInfo(processInfo, operation);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("constrtct operation process info failed");
        HKS_FREE_PTR(operation);
        return ret;
    }

    ret = ConstructOperationHandle(operationHandle, &(operation->handle));
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("constrtct operation handle failed");
        HKS_FREE_BLOB(operation->processInfo.processName);
        HKS_FREE_BLOB(operation->processInfo.userId);
        HKS_FREE_PTR(operation);
        return ret;
    }

    operation->abortable = abortable;

    ret = AddOperation(operation);
    if (ret != HKS_SUCCESS) {
        HKS_FREE_BLOB(operation->processInfo.processName);
        HKS_FREE_BLOB(operation->processInfo.userId);
        HKS_FREE_PTR(operation);
    }

    return ret;
}

static bool IsSameProcessName(const struct HksProcessInfo *processInfo, const struct HksOperation *operation)
{
    uint32_t processNameLen = operation->processInfo.processName.size;
    return ((processNameLen == processInfo->processName.size) &&
        (memcmp(operation->processInfo.processName.data, processInfo->processName.data, processNameLen) == 0));
}

static bool IsSameUserId(const struct HksProcessInfo *processInfo, const struct HksOperation *operation)
{
    uint32_t userIdLen = operation->processInfo.userId.size;
    return ((userIdLen == processInfo->userId.size) &&
        (memcmp(operation->processInfo.userId.data, processInfo->userId.data, userIdLen) == 0));
}

struct HksOperation *QueryOperation(const struct HksProcessInfo *processInfo, const struct HksBlob *operationHandle)
{
    uint64_t handle;
    int32_t ret = ConstructOperationHandle(operationHandle, &handle);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, NULL, "construct handle failed when query operation")

    struct HksOperation *operation = NULL;
    pthread_mutex_lock(&g_lock);
    HKS_DLIST_ITER(operation, &g_operationList) {
        if ((operation != NULL) && (operation->handle == handle) && IsSameProcessName(processInfo, operation) &&
            IsSameUserId(processInfo, operation)) {
            pthread_mutex_unlock(&g_lock);
            return operation;
        }
    }
    pthread_mutex_unlock(&g_lock);

    return NULL;
}

void DeleteOperation(const struct HksBlob *operationHandle)
{
    uint64_t handle;
    int32_t ret = ConstructOperationHandle(operationHandle, &handle);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("construct handle failed when delete operation");
        return;
    }

    struct HksOperation *operation = NULL;
    pthread_mutex_lock(&g_lock);
    HKS_DLIST_ITER(operation, &g_operationList) {
        if (operation != NULL && operation->handle == handle) {
            FreeOperation(&operation);
            --g_operationCount;
            HKS_LOG_I("delete operation count:%" LOG_PUBLIC "u", g_operationCount);
            pthread_mutex_unlock(&g_lock);
            return;
        }
    }
    pthread_mutex_unlock(&g_lock);
}

static void DeleteSession(const struct HksProcessInfo *processInfo, struct HksOperation *operation)
{
    bool isNeedDelete = false;
    if (processInfo->processName.size == 0) { /* delete by user id */
        isNeedDelete = IsSameUserId(processInfo, operation);
    } else { /* delete by process name */
        isNeedDelete = IsSameUserId(processInfo, operation) && IsSameProcessName(processInfo, operation);
    }

    if (isNeedDelete) {
        DeleteKeyNode(operation->handle);
        FreeOperation(&operation);
        --g_operationCount;
        HKS_LOG_I("delete session count = %" LOG_PUBLIC "u", g_operationCount);
    }
}

void DeleteSessionByProcessInfo(const struct HksProcessInfo *processInfo)
{
    struct HksOperation *operation = NULL;

    pthread_mutex_lock(&g_lock);
    HKS_DLIST_SAFT_ITER(operation, &g_operationList) {
        if (operation != NULL) {
            DeleteSession(processInfo, operation);
        }
    }
    pthread_mutex_unlock(&g_lock);
}

