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

#include "softbus_adapter_log.h"
#include "softbus_adapter_mem.h"
#include "softbus_adapter_hisysevent.h"

int32_t SoftbusWriteHisEvt(SoftBusEvtReportMsg* reportMsg)
{
    (void)reportMsg;
    return 0;
}

void SoftbusFreeEvtReporMsg(SoftBusEvtReportMsg* msg)
{
    if (msg == NULL) {
        return;
    }

    if (msg->paramArray != NULL) {
        SoftBusFree(msg->paramArray);
        msg->paramArray = NULL;
    }
    SoftBusFree(msg);
}

SoftBusEvtReportMsg* SoftbusCreateEvtReportMsg(int32_t paramNum)
{
    if (paramNum <= SOFTBUS_EVT_PARAM_ZERO || paramNum >= SOFTBUS_EVT_PARAM_BUTT) {
        HILOG_ERROR(SOFTBUS_HILOG_ID, "param is invalid");
        return NULL;
    }

    SoftBusEvtReportMsg *msg = (SoftBusEvtReportMsg*)SoftBusMalloc(sizeof(SoftBusEvtReportMsg));
    if (msg == NULL) {
        HILOG_ERROR(SOFTBUS_HILOG_ID, "report msg is null");
        return NULL;
    }

    msg->paramArray = (SoftBusEvtParam*)SoftBusMalloc(sizeof(SoftBusEvtParam) * paramNum);
    if (msg->paramArray == NULL) {
        SoftbusFreeEvtReporMsg(msg);
        return NULL;
    }
    return msg;
}
