/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <securec.h>
#include "fillpinc.h"
#include "nstackx.h"
#include "nstackx_dfile.h"
#include "softbus_adapter_log.h"
#include "softbus_errcode.h"
#include "softbus_adapter_hisysevent.h"
#include "softbus_hisysevt_nstack.h"

#ifdef FILLP_ENHANCED
/* below define must keep the same with DStream DFinder DMsg DFile */
#define NSTACK_DFX_EVENT_NAME_LEN 33

typedef enum {
    NSTACK_DFX_EVT_TYPE_FAULT,
    NSTACK_DFX_EVT_TYPE_STATISTIC,
    NSTACK_DFX_EVT_TYPE_SECURITY,
    NSTACK_DFX_EVT_TYPE_BEHAVIOR,
    NSTACK_DFX_EVT_TYPE_BUTT
} NstackDfxEvtType;

typedef enum {
    NSTACK_DFX_EVT_LEVEL_CRITICAL,
    NSTACK_DFX_EVT_LEVEL_MINOR,
} NstackDfxEvtLevel;

typedef enum {
    NSTACK_DFX_PARAMTYPE_BOOL,
    NSTACK_DFX_PARAMTYPE_UINT8,
    NSTACK_DFX_PARAMTYPE_UINT16,
    NSTACK_DFX_PARAMTYPE_INT32,
    NSTACK_DFX_PARAMTYPE_UINT32,
    NSTACK_DFX_PARAMTYPE_UINT64,
    NSTACK_DFX_PARAMTYPE_FLOAT,
    NSTACK_DFX_PARAMTYPE_DOUBLE,
    NSTACK_DFX_PARAMTYPE_STRING,
} NstackDfxEvtParamType;

typedef struct {
    NstackDfxEvtParamType type;
    char paramName[NSTACK_DFX_EVENT_NAME_LEN];
    union {
        uint8_t u8v;
        uint16_t u16v;
        int32_t i32v;
        uint32_t u32v;
        uint64_t u64v;
        float f;
        double d;
        char str[NSTACK_DFX_EVENT_NAME_LEN];
    } val;
} NstackDfxEvtParam;

typedef struct {
    char eventName[NSTACK_DFX_EVENT_NAME_LEN];
    NstackDfxEvtType type;
    NstackDfxEvtLevel level;
    uint32_t paramNum;
    NstackDfxEvtParam *paramArray;
} NstackDfxEvent;
/* up define must keep the same with DStream DFinder DMsg DFile */

static int CopyEventParamVal(SoftBusEvtParamType type, void *dst, const void *src)
{
    switch (type) {
        case SOFTBUS_EVT_PARAMTYPE_BOOL:
        case SOFTBUS_EVT_PARAMTYPE_UINT8:
            *(uint8_t *)dst = *(uint8_t *)src;
            break;
        case SOFTBUS_EVT_PARAMTYPE_UINT16:
            *(uint16_t *)dst = *(uint16_t *)src;
            break;
        case SOFTBUS_EVT_PARAMTYPE_INT32:
        case SOFTBUS_EVT_PARAMTYPE_UINT32:
        case SOFTBUS_EVT_PARAMTYPE_FLOAT:
            *(uint32_t *)dst = *(uint32_t *)src;
            break;
        case SOFTBUS_EVT_PARAMTYPE_UINT64:
        case SOFTBUS_EVT_PARAMTYPE_DOUBLE:
            *(uint64_t *)dst = *(uint64_t *)src;
            break;
        case SOFTBUS_EVT_PARAMTYPE_STRING:
            if (strcpy_s(dst, SOFTBUS_HISYSEVT_PARAM_LEN, src) != EOK) {
                LOG_ERR("softbus param string max %d, nstack param string %s",
                    SOFTBUS_HISYSEVT_PARAM_LEN, (char *)src);
                return SOFTBUS_ERR;
            }
            break;
        default:
            LOG_ERR("unknow param type");
            return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}

static int NstackEventParaToSoftBusEventPara(SoftBusEvtParam *dst, const NstackDfxEvtParam *src)
{
    if (src->type >= SOFTBUS_EVT_PARAMTYPE_BUTT) {
        LOG_ERR("softbus paramType max %d, nstack paramType %d",
            SOFTBUS_EVT_PARAMTYPE_BUTT, src->type);
        return SOFTBUS_ERR;
    }
    dst->paramType = (SoftBusEvtParamType)src->type;
    if (strcpy_s(dst->paramName, SOFTBUS_HISYSEVT_NAME_LEN, src->paramName) != EOK) {
        LOG_ERR("softbus paramName max size %d, nstack paramName name %s",
            SOFTBUS_HISYSEVT_NAME_LEN, src->paramName);
        return SOFTBUS_ERR;
    }
    if (CopyEventParamVal(dst->paramType, &dst->paramValue, &src->val) != SOFTBUS_OK) {
        return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}

static int NstackDfxEvtToSoftBusReportMsg(SoftBusEvtReportMsg *msg, const NstackDfxEvent *info)
{
    if (strcpy_s(msg->evtName, SOFTBUS_HISYSEVT_NAME_LEN, info->eventName) != EOK) {
        LOG_ERR("eventName mismatch, nstack event name %s", info->eventName);
        return SOFTBUS_ERR;
    }
    if (info->type >= SOFTBUS_EVT_TYPE_BUTT) {
        LOG_ERR("eventType mismatch, nstack event type %d", info->type);
        return SOFTBUS_ERR;
    }
    msg->evtType = (SoftBusEvtType)(info->type);
    if (info->paramNum != 0 && info->paramArray == NULL) {
        LOG_ERR("param mismatch, nstack paramNum %u paramArray is NULL", info->paramNum);
        return SOFTBUS_ERR;
    }
    msg->paramNum = info->paramNum;
    if (msg->paramNum == 0) {
        return SOFTBUS_OK;
    }
    msg->paramArray = (SoftBusEvtParam *)calloc(msg->paramNum, sizeof(SoftBusEvtParam));
    if (msg->paramArray == NULL) {
        LOG_ERR("calloc paramArray failed! paramNum %u", info->paramNum);
        return SOFTBUS_ERR;
    }
    for (uint8_t i = 0; i < info->paramNum; i++) {
        if (NstackEventParaToSoftBusEventPara(&msg->paramArray[i], &info->paramArray[i]) != SOFTBUS_OK) {
            return SOFTBUS_ERR;
        }
    }
    return SOFTBUS_OK;
}

static void NstackHiEventCb(void *softObj, const NstackDfxEvent *info)
{
    (void)softObj;
    if (info == NULL) {
        LOG_ERR("info is NULL");
        return;
    }
    SoftBusEvtReportMsg msg;
    (void)memset_s(&msg, sizeof(msg), 0, sizeof(msg));
    if (NstackDfxEvtToSoftBusReportMsg(&msg, info) != SOFTBUS_OK) {
        if (msg.paramArray != NULL) {
            free(msg.paramArray);
        }
        LOG_ERR("change NstackDfxEvent to SoftBusEvtReportMsg failed!");
        return;
    }
    (void)SoftbusWriteHisEvt(&msg);
    if (msg.paramArray != NULL) {
        free(msg.paramArray);
    }
}

void DstreamHiEventCb(void *softObj, const FillpDfxEvent *info)
{
    if (softObj == NULL || info == NULL) {
        LOG_ERR("param is NULL");
        return;
    }
    NstackDfxEvent nstackInfo;
    if (memcpy_s(&nstackInfo, sizeof(NstackDfxEvent), info, sizeof(FillpDfxEvent)) != EOK) {
        LOG_ERR("change FillpDfxEvent to NstackDfxEvent failed!");
        return;
    }
    NstackHiEventCb(softObj, &nstackInfo);
}

static void DFileHiEventCb(void *softObj, const DFileEvent *info)
{
    NstackDfxEvent nstackInfo;
    if (memcpy_s(&nstackInfo, sizeof(NstackDfxEvent), info, sizeof(DFileEvent)) != EOK) {
        LOG_ERR("change DFileEvent to NstackDfxEvent failed!");
        return;
    }
    NstackHiEventCb(softObj, &nstackInfo);
}

static void DFinderHiEventCb(void *softObj, const DFinderEvent *info)
{
    NstackDfxEvent nstackInfo;
    if (memcpy_s(nstackInfo.eventName, sizeof(nstackInfo.eventName),
        info->eventName, sizeof(info->eventName)) != EOK) {
        LOG_ERR("change DFinderEvent to NstackDfxEvent failed!");
        return;
    }

    nstackInfo.type = (NstackDfxEvtType)info->type;
    nstackInfo.level = (NstackDfxEvtLevel)info->level;
    nstackInfo.paramNum = info->paramNum;
    nstackInfo.paramArray = (NstackDfxEvtParam *)info->params;
    NstackHiEventCb(softObj, &nstackInfo);
}

void NstackInitHiEvent(void)
{
    NSTACKX_DFileSetEventFunc(NULL, DFileHiEventCb);
    if (NSTACKX_DFinderSetEventFunc(NULL, DFinderHiEventCb) != 0) {
        LOG_ERR("NSTACKX_DFinderSetEventFunc failed!");
    }
}
#endif /* FILLP_ENHANCED */
