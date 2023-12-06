/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "trans_pending_pkt.h"

#include <sys/time.h>
#include <unistd.h>

#include "softbus_adapter_mem.h"
#include "softbus_adapter_thread.h"
#include "softbus_errcode.h"
#include "softbus_log.h"
#include "softbus_utils.h"

#define TIME_OUT 20

typedef struct {
    ListNode node;
    SoftBusCond cond;
    SoftBusMutex lock;
    int32_t channelId;
    int32_t seq;
    uint8_t status;
} PendingPktInfo;

enum PackageStatus {
    PACKAGE_STATUS_PENDING = 0,
    PACKAGE_STATUS_FINISHED,
    PACKAGE_STATUS_CANCELED
};

static SoftBusList *g_pendingList[PENDING_TYPE_BUTT] = {NULL, NULL};

int32_t PendingInit(int type)
{
    if (type < PENDING_TYPE_PROXY || type >= PENDING_TYPE_BUTT) {
        return SOFTBUS_ERR;
    }

    g_pendingList[type] = CreateSoftBusList();
    if (g_pendingList[type] == NULL) {
        return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}

void PendingDeinit(int type)
{
    if (type < PENDING_TYPE_PROXY || type >= PENDING_TYPE_BUTT) {
        return;
    }

    if (g_pendingList[type] != NULL) {
        DestroySoftBusList(g_pendingList[type]);
        g_pendingList[type] = NULL;
    }
    SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_INFO, "PendigPackManagerDeinit init ok");
}

static inline bool TimeBefore(const SoftBusSysTime *inputTime)
{
    SoftBusSysTime now;
    SoftBusGetTime(&now);
    return (now.sec < inputTime->sec || (now.sec == inputTime->sec && now.usec < inputTime->usec));
}

static PendingPktInfo *CreatePendingItem(int32_t channelId, int32_t seqNum)
{
    PendingPktInfo *item = (PendingPktInfo *)SoftBusCalloc(sizeof(PendingPktInfo));
    if (item == NULL) {
        return NULL;
    }

    SoftBusMutexInit(&item->lock, NULL);
    SoftBusCondInit(&item->cond);
    item->channelId = channelId;
    item->seq = seqNum;
    item->status = PACKAGE_STATUS_PENDING;
    return item;
}

static void ReleasePendingItem(PendingPktInfo *item)
{
    if (item == NULL) {
        return;
    }
    (void)SoftBusMutexDestroy(&item->lock);
    (void)SoftBusCondDestroy(&item->cond);
    SoftBusFree(item);
}

int32_t ProcPendingPacket(int32_t channelId, int32_t seqNum, int type)
{
    if (type < PENDING_TYPE_PROXY || type >= PENDING_TYPE_BUTT) {
        return SOFTBUS_ERR;
    }

    PendingPktInfo *item = NULL;
    SoftBusList *pendingList = g_pendingList[type];
    if (pendingList == NULL) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "pending[%d] list not inited.", type);
        return SOFTBUS_TRANS_TDC_PENDINGLIST_NOT_FOUND;
    }

    SoftBusMutexLock(&pendingList->lock);
    LIST_FOR_EACH_ENTRY(item, &pendingList->list, PendingPktInfo, node)
    {
        if (item->seq == seqNum && item->channelId == channelId) {
            SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "PendingPacket already Created");
            SoftBusMutexUnlock(&pendingList->lock);
            return SOFTBUS_TRANS_TDC_CHANNEL_ALREADY_PENDING;
        }
    }

    item = CreatePendingItem(channelId, seqNum);
    if (item == NULL) {
        SoftBusMutexUnlock(&pendingList->lock);
        return SOFTBUS_MALLOC_ERR;
    }
    ListAdd(&pendingList->list, &item->node);
    pendingList->cnt++;
    SoftBusMutexUnlock(&pendingList->lock);

    SoftBusSysTime outtime;
    SoftBusSysTime now;
    SoftBusGetTime(&now);
    outtime.sec = now.sec + TIME_OUT;
    outtime.usec = now.usec;
    SoftBusMutexLock(&item->lock);
    while (item->status == PACKAGE_STATUS_PENDING && TimeBefore(&outtime)) {
        SoftBusCondWait(&item->cond, &item->lock, &outtime);
    }

    int32_t ret = SOFTBUS_OK;
    if (item->status != PACKAGE_STATUS_FINISHED) {
        ret = SOFTBUS_TIMOUT;
    }
    SoftBusMutexUnlock(&item->lock);

    SoftBusMutexLock(&pendingList->lock);
    ListDelete(&item->node);
    pendingList->cnt--;
    SoftBusMutexUnlock(&pendingList->lock);
    ReleasePendingItem(item);
    return ret;
}

int32_t SetPendingPacket(int32_t channelId, int32_t seqNum, int type)
{
    if (type < PENDING_TYPE_PROXY || type >= PENDING_TYPE_BUTT) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "type[%d] illegal.", type);
        return SOFTBUS_ERR;
    }

    SoftBusList *pendingList = g_pendingList[type];
    if (pendingList == NULL) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "pendind list not exist");
        return SOFTBUS_ERR;
    }
    if (SoftBusMutexLock(&pendingList->lock) != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "set pendind lock failed.");
        return SOFTBUS_ERR;
    }
    PendingPktInfo *item = NULL;
    LIST_FOR_EACH_ENTRY(item, &pendingList->list, PendingPktInfo, node) {
        if (item->seq == seqNum && item->channelId == channelId) {
            item->status = PACKAGE_STATUS_FINISHED;
            SoftBusCondSignal(&item->cond);
            SoftBusMutexUnlock(&pendingList->lock);
            return SOFTBUS_OK;
        }
    }
    SoftBusMutexUnlock(&pendingList->lock);
    return SOFTBUS_ERR;
}

int32_t DelPendingPacket(int32_t channelId, int type)
{
    if (type < PENDING_TYPE_PROXY || type >= PENDING_TYPE_BUTT) {
        return SOFTBUS_ERR;
    }

    SoftBusList *pendingList = g_pendingList[type];
    if (pendingList == NULL) {
        return SOFTBUS_ERR;
    }
    if (SoftBusMutexLock(&pendingList->lock) != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "del pendind lock failed.");
        return SOFTBUS_ERR;
    }
    PendingPktInfo *item = NULL;
    LIST_FOR_EACH_ENTRY(item, &pendingList->list, PendingPktInfo, node) {
        if (item->channelId == channelId) {
            item->status = PACKAGE_STATUS_CANCELED;
            SoftBusCondSignal(&item->cond);
            SoftBusMutexUnlock(&pendingList->lock);
            return SOFTBUS_OK;
        }
    }
    SoftBusMutexUnlock(&pendingList->lock);
    return SOFTBUS_OK;
}

