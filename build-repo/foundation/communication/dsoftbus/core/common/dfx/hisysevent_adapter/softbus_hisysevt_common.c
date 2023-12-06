/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * miscservices under the License is miscservices on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "softbus_hisysevt_common.h"

#include "securec.h"

#include "message_handler.h"
#include "softbus_adapter_hisysevent.h"
#include "softbus_adapter_mem.h"
#include "softbus_error_code.h"
#include "softbus_hisysevt_bus_center.h"
#include "softbus_hisysevt_connreporter.h"
#include "softbus_hisysevt_discreporter.h"
#include "softbus_hisysevt_transreporter.h"
#include "softbus_log.h"
#include "softbus_hisysevt_nstack.h"

#define MS_OF_DAY (24 * 3600 * 1000)
#define MSG_STATISTIC_EVT_REPORT 0

StatisticEvtReportFunc g_statisticEvtReportFunc[SOFTBUS_STATISTIC_EVT_BUTT] = {NULL};

StatisticEvtReportFunc GetStatisticEvtReportFunc(StatisticEvtType type)
{
    if (type < SOFTBUS_STATISTIC_EVT_START || type >= SOFTBUS_STATISTIC_EVT_BUTT) {
        return NULL;
    }
    
    return g_statisticEvtReportFunc[type];
}

int32_t SetStatisticEvtReportFunc(StatisticEvtType type, StatisticEvtReportFunc func)
{
    if (type < SOFTBUS_STATISTIC_EVT_START || type >= SOFTBUS_STATISTIC_EVT_BUTT || func == NULL) {
        return SOFTBUS_ERR;
    }
    g_statisticEvtReportFunc[type] = func;

    return SOFTBUS_OK;
}

static void InitStatisticEvtReportFunc(void)
{
    for (int i = SOFTBUS_STATISTIC_EVT_START; i < SOFTBUS_STATISTIC_EVT_BUTT; i++) {
        g_statisticEvtReportFunc[i] = NULL;
    }
}

static void ReportStatisticEvent(SoftBusMessage* param)
{
    (void)param;

    for (int i = SOFTBUS_STATISTIC_EVT_START; i < SOFTBUS_STATISTIC_EVT_BUTT; i++) {
        if (g_statisticEvtReportFunc[i] != NULL) {
            g_statisticEvtReportFunc[i]();
        }
    }
}

static void FreeMessageFunc(SoftBusMessage* msg)
{
    if (msg == NULL) {
        return;
    }

    if (msg->handler != NULL) {
        SoftBusFree(msg->handler);
    }
    SoftBusFree(msg);
}

typedef void (*HandleMessageFunc)(SoftBusMessage *msg);

static inline SoftBusHandler* CreateHandler(SoftBusLooper *looper, HandleMessageFunc callback)
{
    SoftBusHandler *handler = SoftBusMalloc(sizeof(SoftBusHandler));
    if (handler == NULL) {
        SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_ERROR, "create handler failed");
        return NULL;
    }
    handler->looper = looper;
    handler->name = "statisticEvtReportHandler";
    handler->HandleMessage = callback;

    return handler;
}

static SoftBusMessage* CreateMessage(SoftBusLooper *looper, HandleMessageFunc callback)
{
    SoftBusMessage* msg = SoftBusMalloc(sizeof(SoftBusMessage));
    if (msg == NULL) {
        SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_ERROR, "malloc softbus message failed");
        return NULL;
    }
    SoftBusHandler *handler = CreateHandler(looper, callback);

    msg->what = MSG_STATISTIC_EVT_REPORT;
    msg->obj = NULL;
    msg->handler = handler;
    msg->FreeMessage = FreeMessageFunc;

    return msg;
}

static int32_t CreateAndPostMsgDelay(SoftBusLooper *looper, HandleMessageFunc callback, uint64_t delayMillis)
{
    if ((looper == NULL) || (callback == NULL)) {
        return SOFTBUS_INVALID_PARAM;
    }

    SoftBusMessage *message = CreateMessage(looper, callback);
    if (message == NULL) {
        return SOFTBUS_MEM_ERR;
    }

    looper->PostMessageDelay(looper, message, delayMillis);
    return SOFTBUS_OK;
}

static void ReportStatisticEvtPeriod(SoftBusMessage* msg)
{
    ReportStatisticEvent(msg);
    
    CreateAndPostMsgDelay(GetLooper(LOOP_TYPE_DEFAULT), ReportStatisticEvtPeriod, MS_OF_DAY);
}

int32_t InitSoftbusSysEvt(void)
{
    InitStatisticEvtReportFunc();
    
    if (InitTransStatisticSysEvt() != SOFTBUS_OK) {
        return SOFTBUS_ERR;
    }
    if (InitBusCenterDfx() != SOFTBUS_OK) {
        return SOFTBUS_ERR;
    }

    if (InitDiscStatisticSysEvt() != SOFTBUS_OK) {
        return SOFTBUS_ERR;
    }

    if (InitConnStatisticSysEvt() != SOFTBUS_OK) {
        return SOFTBUS_ERR;
    }
#ifdef FILLP_ENHANCED
    NstackInitHiEvent();
#endif
    return CreateAndPostMsgDelay(GetLooper(LOOP_TYPE_DEFAULT), ReportStatisticEvtPeriod, MS_OF_DAY);
}
