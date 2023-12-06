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

#include "lnn_fast_offline.h"

#include "softbus_errcode.h"
#include "softbus_log.h"

int32_t LnnInitFastOffline(void)
{
    SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_INFO, "init virtual lnn fast offline");
    return SOFTBUS_OK;
}

void LnnDeinitFastOffline(void)
{
    SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_INFO, "Deinit virtual lnn fast offline");
}

int32_t LnnSendNotTrustedInfo(NotTrustedDelayInfo *info, uint32_t num)
{
    (void)info;
    (void)num;
    return SOFTBUS_OK;
}

int32_t LnnBleFastOfflineOnceBegin(void)
{
    SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_INFO, "LnnBleFastOfflineOnceBegin virtual ok!");
    return SOFTBUS_OK;
}

void LnnIpAddrChangeEventHandler(void)
{
    SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_INFO, "LnnIpAddrChangeEventHandler virtual ok!");
}
