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

#ifndef LNN_LANE_DEF_H
#define LNN_LANE_DEF_H

#include <stdint.h>
#include "lnn_lane_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    LANE_BW_RANDOM = 0x0,
    LANE_BW_20M,
    LANE_BW_40M,
    LANE_BW_80M,
    LANE_BW_160M,
    LANE_BW_BUTT = 0xFF,
} LaneBandwidth;

typedef enum {
    LANE_TS_RANDOM = 0x0,
    LANE_TS_BUTT = 0xFF,
} LaneTimeSlotPolicy;

typedef enum {
    LANE_POWER_RANDOM = 0x0,
    LANE_POWER_SMART,
    LANE_POWER_VSR,
    LANE_POWER_BUTT = 0xFF,
} LanePowerType;

typedef enum {
    LANE_PRI_LOW = 0x0,
    LANE_PRI_HIGH,
    LANE_PRI_BUTT = 0xFF,
} LaneTransPriority;

typedef struct {
    uint32_t serialNum;
    LaneLinkType linkType;
    int32_t phyChannel;
    LaneBandwidth bw;
    LaneTimeSlotPolicy ts;
    LanePowerType energy;
    LaneTransType content;
    LaneTransPriority priority;
    uint16_t baseProfileNum;
    uint32_t maxSpeed;
} LaneProfile;

#ifdef __cplusplus
}
#endif
#endif