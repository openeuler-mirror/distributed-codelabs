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

#ifndef DSLM_MSG_UTILS_H
#define DSLM_MSG_UTILS_H

#include <stdint.h>

#include "dslm_cred.h"
#include "dslm_msg_serialize.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FIELD_VERSION "version"
#define FIELD_CHALLENGE "challenge"
#define FIELD_SUPPORT "support"
#define FIELD_CRED_TYPE "type"
#define FIELD_CRED_INFO "info"

int32_t BuildDeviceSecInfoRequest(uint64_t challenge, MessageBuff **msg);

int32_t BuildDeviceSecInfoResponse(uint64_t challenge, const DslmCredBuff *cred, MessageBuff **msg);

int32_t ParseDeviceSecInfoRequest(const MessageBuff *msg, RequestObject *obj);

int32_t ParseDeviceSecInfoResponse(const MessageBuff *msg, uint64_t *challenge, uint32_t *version, DslmCredBuff **cred);

#ifdef __cplusplus
}
#endif

#endif // DSLM_MSG_UTILS_H
