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

#ifndef DSLM_INNER_PROCESS_H
#define DSLM_INNER_PROCESS_H

#include <stdint.h>

#include "dslm_core_defines.h"
#include "dslm_msg_serialize.h"

#ifdef __cplusplus
extern "C" {
#endif

int32_t CheckAndGenerateChallenge(DslmDeviceInfo *device);

int32_t SendDeviceInfoRequest(DslmDeviceInfo *device);

int32_t VerifyDeviceInfoResponse(DslmDeviceInfo *device, const MessageBuff *buff);
#ifdef __cplusplus
}
#endif

#endif // DSLM_INNER_PROCESS_H
