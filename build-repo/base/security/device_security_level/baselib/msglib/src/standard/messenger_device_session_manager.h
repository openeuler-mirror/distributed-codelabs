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

#ifndef SEC_MESSENGER_DEVICE_SESSION_MANAGER_H
#define SEC_MESSENGER_DEVICE_SESSION_MANAGER_H

#include "messenger.h"

#include <stdint.h>
#include <stdbool.h>

#include "utils_work_queue.h"

#ifdef __cplusplus
extern "C" {
#endif

bool InitDeviceSessionManager(WorkQueue *queue, const MessengerConfig *config);

bool DeInitDeviceSessionManager(void);

void MessengerSendMsgTo(uint64_t transNo, const DeviceIdentify *devId, const uint8_t *msg, uint32_t msgLen);

#ifdef __cplusplus
}
#endif

#endif // SEC_MESSENGER_DEVICE_SESSION_MANAGER_H
