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

#ifndef SEC_MESSENGER_UTILS_H
#define SEC_MESSENGER_UTILS_H

#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>

#include "securec.h"
#include "utils_hexstring.h"

#include "messenger.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_TRY_TIMES 512

static inline uint32_t MaskDeviceIdentity(const char *deviceId, uint32_t length)
{
#define MASK_LEN 4U
#define SHIFT_LENGTH 8U
#define MASK_LOW 0x00ffU
#define MASK_HIGH 0xff00U
    if (deviceId == NULL || length < MASK_LEN) {
        return 0;
    }

    uint16_t maskId = 0;
    HexStringToByte(deviceId, MASK_LEN, (uint8_t *)&maskId, sizeof(maskId));
    return ((maskId & MASK_HIGH) >> SHIFT_LENGTH) | ((maskId & MASK_LOW) << SHIFT_LENGTH);
}

static inline void MessengerSleep(uint32_t seconds)
{
    int ret;
    struct timeval tmv = {
        .tv_sec = seconds,
        .tv_usec = 0,
    };
    do {
        ret = select(0, NULL, NULL, NULL, &tmv);
    } while ((ret == -1) && (errno == EINTR));
}

static inline bool IsSameDevice(const DeviceIdentify *left, const DeviceIdentify *right)
{
    if ((left == NULL) || (right == NULL)) {
        return false;
    }

    if (left->length != right->length) {
        return false;
    }

    if (memcmp(left->identity, right->identity, left->length) != 0) {
        return false;
    }

    return true;
}

#ifdef __cplusplus
}
#endif

#endif // SEC_MESSENGER_UTILS_H
