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

#include "lnn_lane_score.h"
#include "softbus_log.h"
#include "softbus_errcode.h"

#define VIRTUAL_DEFAULT_SCORE 60

int32_t LnnInitScore(void)
{
    SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_INFO, "init laneScore virtual");
    return SOFTBUS_OK;
}

void LnnDeinitScore(void)
{
    return;
}

int32_t LnnGetWlanLinkedInfo(LnnWlanLinkedInfo *info)
{
    (void)info;
    return SOFTBUS_ERR;
}

int32_t LnnGetCurrChannelScore(int32_t channelId)
{
    (void)channelId;
    return VIRTUAL_DEFAULT_SCORE;
}

int32_t LnnStartScoring(int32_t interval)
{
    (void)interval;
    return SOFTBUS_OK;
}

int32_t LnnStopScoring(void)
{
    return SOFTBUS_OK;
}