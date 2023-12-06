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

#include "lnn_decision_db.h"

#include "softbus_errcode.h"
#include "softbus_log.h"

int32_t LnnInsertSpecificTrustedDevInfo(const char *udid)
{
    (void)udid;

    SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "insert trusted dev info not implemented.");
    return SOFTBUS_NOT_IMPLEMENT;
}

int32_t LnnDeleteSpecificTrustedDevInfo(const char *udid)
{
    (void)udid;

    SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "remove trusted dev info not implemented.");
    return SOFTBUS_NOT_IMPLEMENT;
}

int32_t LnnGetTrustedDevInfoFromDb(char **udidArray, uint32_t *num)
{
    (void)udidArray;
    (void)num;

    SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "get trusted dev info not implemented.");
    return SOFTBUS_NOT_IMPLEMENT;
}

int32_t LnnInitDecisionDbDelay(void)
{
    SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "init decision db not implemented.");
    return SOFTBUS_OK;
}
