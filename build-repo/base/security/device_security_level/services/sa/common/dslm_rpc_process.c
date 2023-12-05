/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "dslm_rpc_process.h"

#include <stdbool.h>
#include <stddef.h>
#include <unistd.h>

#include "utils_log.h"

#include "device_security_defines.h"
#include "dslm_core_process.h"
#include "dslm_hievent.h"
#include "dslm_hitrace.h"
#include "dslm_messenger_wrapper.h"
#include "dslm_msg_serialize.h"

#define SLEEP_TIME (1000 * 500)
#define TRY_TIMES 20

int32_t OnPeerMsgReceived(const DeviceIdentify *devId, const uint8_t *msg, uint32_t len)
{
    if (devId == NULL || msg == NULL || len == 0) {
        SECURITY_LOG_ERROR("invalid params, len = %{public}u", len);
        return ERR_INVALID_PARA;
    }

    const MessageBuff buff = {.buff = (uint8_t *)msg, .length = len};
    int32_t ret = SUCCESS;
    MessagePacket *packet = ParseMessage(&buff);
    if (packet == NULL) {
        SECURITY_LOG_ERROR("packet is null");
        return ERR_INVALID_PARA;
    }
    if (packet->payload == NULL) {
        FreeMessagePacket(packet);
        SECURITY_LOG_ERROR("packet->payload is null");
        return ERR_INVALID_PARA;
    }

    switch (packet->type) {
        case MSG_TYPE_DSLM_CRED_REQUEST:
            ret = OnPeerMsgRequestInfoReceived(devId, packet->payload, packet->length);
            break;
        case MSG_TYPE_DSLM_CRED_RESPONSE:
            ret = OnPeerMsgResponseInfoReceived(devId, packet->payload, packet->length);
            break;
        default:
            ret = ERR_INVALID_PARA;
            break;
    }
    if (ret != SUCCESS) {
        SECURITY_LOG_ERROR("ret = %{public}d, packet->type = %{public}u", ret, packet->type);
    }
    FreeMessagePacket(packet);
    return ret;
}

int32_t OnSendResultNotifier(const DeviceIdentify *devId, uint64_t transNo, uint32_t result)
{
    return OnMsgSendResultNotifier(devId, transNo, result);
}

uint32_t InitService(void)
{
    uint32_t times = 0;
    DslmStartProcessTrace("InitService");
    uint32_t ret = InitMessenger(OnPeerMsgReceived, OnPeerStatusReceiver, OnSendResultNotifier);
    if (ret != SUCCESS) {
        DslmFinishProcessTrace();
        ReportHiEventServiceStartFailed(ret);
        SECURITY_LOG_ERROR("InitMessenger ret = %{public}u", ret);
        return ret;
    }

    SECURITY_LOG_INFO("InitService InitMessenger success");

    while (true) {
        DslmCountTrace("InitDslmProcess", times);
        if (InitDslmProcess()) {
            break;
        }
        usleep(SLEEP_TIME);
        if (times > TRY_TIMES) {
            SECURITY_LOG_ERROR("wait SoftBus timeout");
            break;
        }
        times++;
    }
    DslmFinishProcessTrace();

    return SUCCESS;
}

void UnInitService(void)
{
    DeinitDslmProcess();
    DeinitMessenger();
}
