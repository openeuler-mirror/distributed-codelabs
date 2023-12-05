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

#ifndef DSLM_MSG_SERIALIZE_H
#define DSLM_MSG_SERIALIZE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MSG_TYPE_DSLM_CRED_REQUEST 0x1
#define MSG_TYPE_DSLM_CRED_RESPONSE 0x2

#define MSG_BUFF_MAX_LENGTH (81920 * 4)

#define FIELD_MESSAGE "message"
#define FIELD_PAYLOAD "payload"

/* raw msg from soft bus */
typedef struct MessageBuff {
    uint32_t length;
    uint8_t *buff;
} MessageBuff;

/* parsed fist level msg */
typedef struct MessagePacket {
    uint32_t type;
    uint32_t length;
    uint8_t *payload;
} MessagePacket;

MessagePacket *ParseMessage(const MessageBuff *buff);

MessageBuff *SerializeMessage(const MessagePacket *packet);

void FreeMessagePacket(MessagePacket *packet);

void FreeMessageBuff(MessageBuff *buff);

#ifdef __cplusplus
}
#endif

#endif /* DSLM_MSG_SERIALIZE_H */