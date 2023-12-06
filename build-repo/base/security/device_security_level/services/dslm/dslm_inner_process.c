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

#include "dslm_inner_process.h"

#include <stddef.h>
#include <stdint.h>

#include "device_security_defines.h"
#include "utils_datetime.h"
#include "utils_log.h"
#include "utils_state_machine.h"
#include "dslm_core_defines.h"
#include "dslm_credential.h"
#include "dslm_crypto.h"
#include "dslm_messenger_wrapper.h"
#include "dslm_msg_utils.h"
#include "dslm_cred.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NONCE_ALIVE_TIME 60000

int32_t CheckAndGenerateChallenge(DslmDeviceInfo *device)
{
    if (device == NULL) {
        return ERR_INVALID_PARA;
    }

    uint64_t curr = GetMillisecondSinceBoot();
    if ((curr <= device->nonceTimeStamp) || (curr - device->nonceTimeStamp > NONCE_ALIVE_TIME) || device->nonce == 0) {
        SECURITY_LOG_INFO("update nonce for device %{public}x", device->machine.machineId);
        RandomValue rand = {0};
        GenerateRandom(&rand, RANDOM_MAX_LEN);
        device->nonce = *(uint64_t *)&rand.value[0];
        device->nonceTimeStamp = curr;
    }

    return SUCCESS;
}

int32_t SendDeviceInfoRequest(DslmDeviceInfo *device)
{
    if (device == NULL) {
        return ERR_INVALID_PARA;
    }

    MessageBuff *buff = NULL;
    int32_t ret = BuildDeviceSecInfoRequest(device->nonce, &buff);
    if (ret != SUCCESS) {
        return ERR_INVALID_PARA;
    }
    device->transNum++;
    SendMsgToDevice(device->transNum, &device->identity, buff->buff, buff->length);

    SECURITY_LOG_DEBUG("buff is %s", (char *)buff->buff);
    SECURITY_LOG_INFO("challenge is %{public}x***, transNum is %{public}u",
        (uint32_t)device->nonce, (uint32_t)device->transNum);
    FreeMessageBuff(buff);
    return SUCCESS;
}

int32_t VerifyDeviceInfoResponse(DslmDeviceInfo *device, const MessageBuff *buff)
{
    if (device == NULL || buff == NULL) {
        return ERR_INVALID_PARA;
    }

    DslmCredBuff *cred = NULL;
    uint64_t nonce = 0;
    uint32_t version = 0;
    int32_t ret;

    do {
        // Parse the msg
        ret = ParseDeviceSecInfoResponse(buff, &nonce, &version, &cred);
        if (ret != SUCCESS) {
            SECURITY_LOG_ERROR("ParseDeviceSecInfoResponse failed, ret is %{public}d", ret);
            break;
        }
        device->version = version;
        if (nonce != device->nonce || nonce == 0) {
            ret = ERR_CHALLENGE_ERR;
            SECURITY_LOG_ERROR("nonce not equal");
            DestroyDslmCred(cred);
            break;
        }
        uint64_t curr = GetMillisecondSinceBoot();
        if ((curr <= device->nonceTimeStamp) || (curr - device->nonceTimeStamp > NONCE_ALIVE_TIME)) {
            ret = ERR_CHALLENGE_ERR;
            SECURITY_LOG_ERROR("nonce expired");
            DestroyDslmCred(cred);
            break;
        }
        // process
        ret = DefaultVerifyDslmCred(&device->identity, device->nonce, cred, &device->credInfo);
        DestroyDslmCred(cred);
    } while (0);

    SECURITY_LOG_INFO("challenge is %{public}x***, ret is %{public}d", (uint32_t)nonce, ret);
    return ret;
}

#ifdef __cplusplus
}
#endif
