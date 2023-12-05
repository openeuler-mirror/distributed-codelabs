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

#include "dslm_msg_serialize.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "securec.h"

#include "utils_json.h"
#include "utils_log.h"
#include "utils_mem.h"

#define ASCII_MAX 0x7fU

static inline bool IsAscii(const uint8_t ch)
{
    return (((ch) & (~ASCII_MAX)) == 0);
}

bool CheckMessage(const uint8_t *msg, uint32_t length)
{
    if (msg == NULL || length <= 1 || length > MSG_BUFF_MAX_LENGTH) {
        return false;
    }

    // our msgs is a printable string
    if (msg[length - 1] != '\0') {
        return false;
    }
    for (uint32_t i = 0; i < length - 1; i++) {
        if (!IsAscii(msg[i])) {
            return false;
        }
    }
    return true;
}

MessagePacket *ParseMessage(const MessageBuff *buff)
{
    if (buff == NULL) {
        SECURITY_LOG_DEBUG("EMPTY BUFF");
        return NULL;
    }

    if (!CheckMessage(buff->buff, buff->length)) {
        SECURITY_LOG_DEBUG("ERR MSG");
        return NULL;
    }

    JsonHandle handle = CreateJson((const char *)buff->buff);
    if (handle == NULL) {
        SECURITY_LOG_DEBUG("ERR JSON MSG");
        return NULL;
    }

    char *payload = NULL;
    MessagePacket *packet = NULL;
    do {
        int32_t msgType = GetJsonFieldInt(handle, FIELD_MESSAGE);
        if (msgType < 0) {
            break;
        }
        payload = ConvertJsonToString(GetJsonFieldJson(handle, FIELD_PAYLOAD));
        if (payload == NULL) {
            break;
        }
        packet = MALLOC(sizeof(MessagePacket));
        if (packet == NULL) {
            free(payload);
            break;
        }
        packet->type = (uint32_t)msgType;
        packet->payload = (uint8_t *)payload;
        packet->length = strlen(payload) + 1; // for the end flag '\0'
    } while (0);

    DestroyJson(handle);
    return packet;
}

void FreeMessagePacket(MessagePacket *packet)
{
    if (packet == NULL) {
        return;
    }
    if (packet->payload != NULL) {
        FREE(packet->payload);
        packet->payload = NULL;
    }
    FREE((void *)packet);
}

void FreeMessageBuff(MessageBuff *buff)
{
    if (buff == NULL) {
        return;
    }
    if (buff->buff != NULL) {
        FREE(buff->buff);
        buff->buff = NULL;
    }
    FREE((void *)buff);
}
