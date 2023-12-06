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

#ifndef LNN_HEARTBEAT_MEDIUM_MGR_H
#define LNN_HEARTBEAT_MEDIUM_MGR_H

#include <stdint.h>

#include "softbus_common.h"
#include "lnn_heartbeat_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    /** Heartbeat media type. For details, see {@link LnnHeartbeatType}. */
    LnnHeartbeatType type;
    union {
        /** Defines parameters of ble heartbeat. */
        struct BleParam {
            /** minimum time between the start of two consecutive advertising events. */
            uint16_t advMinInterval;
            /** maximum time between the start of two consecutive advertising events. */
            uint16_t advMaxInterval;
            /** Interval between the start of two consecutive scan windows. */
            uint16_t scanInterval;
            /** The duration in which the link layer scans on one channel. */
            uint16_t scanWindow;
        } ble;
    } info;
} LnnHeartbeatMediumParam;

typedef struct {
    void (*onRelay)(const char *udidHash, ConnectionAddrType type, LnnHeartbeatType hbType);
    int32_t (*onReceive)(DeviceInfo *device, int32_t weight, int32_t localMasterWeight, LnnHeartbeatType hbType);
    int32_t (*onRecvHigherWeight)(const char *udidHash, int32_t weight, ConnectionAddrType type, bool isReElect);
} LnnHeartbeatMediumMgrCb;

typedef struct {
    LnnHeartbeatType hbType;
    bool wakeupFlag;
    bool isRelay;
} LnnHeartbeatCustSendData;

typedef struct {
    LnnHeartbeatType supportType;
    int32_t (*init)(const LnnHeartbeatMediumMgrCb *callback);
    int32_t (*onSendOneHbBegin)(const LnnHeartbeatCustSendData *custData);
    int32_t (*onSendOneHbEnd)(void);
    int32_t (*onSetMediumParam)(const LnnHeartbeatMediumParam *param);
    int32_t (*onUpdateSendInfo)(LnnHeartbeatUpdateInfoType type);
    int32_t (*onStopHbByType)(void);
    void (*deinit)(void);
} LnnHeartbeatMediumMgr;

int32_t LnnHbMediumMgrSetParam(const LnnHeartbeatMediumParam *param);
int32_t LnnHbMediumMgrSendBegin(LnnHeartbeatCustSendData *custData);
int32_t LnnHbMediumMgrSendEnd(LnnHeartbeatType *type);
int32_t LnnHbMediumMgrStop(LnnHeartbeatType *type);
int32_t LnnHbMediumMgrUpdateSendInfo(LnnHeartbeatUpdateInfoType type);

int32_t LnnHbMediumMgrInit(void);
void LnnHbMediumMgrDeinit(void);

int32_t LnnRegistHeartbeatMediumMgr(LnnHeartbeatMediumMgr *mgr);
int32_t LnnUnRegistHeartbeatMediumMgr(LnnHeartbeatMediumMgr *mgr);

void LnnDumpHbMgrRecvList(void);
void LnnDumpHbOnlineNodeList(void);

#ifdef __cplusplus
}
#endif
#endif /* LNN_HEARTBEAT_MEDIUM_MGR_H */
