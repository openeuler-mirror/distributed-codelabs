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

#include "utils_tlv.h"

#include <stddef.h>

#include "securec.h"

#define TLV_TAG_LEN sizeof(uint16_t)
#define TLV_LENGTH_LEN sizeof(uint16_t)
#define TLV_TLV_HEAD_LEN (TLV_TAG_LEN + TLV_LENGTH_LEN)

static uint8_t *GetNextTlv(const uint8_t *buffer)
{
    return (uint8_t *)buffer + ((TlvCommon *)buffer)->len + TLV_TLV_HEAD_LEN;
}

static uint8_t *ParseTlv(const uint8_t *buffer, TlvCommon *tlv, const uint8_t *boundary, uint32_t *retCode)
{
    if (buffer + TLV_TLV_HEAD_LEN > boundary) {
        *retCode = TLV_ERR_PARSE_PAYLOAD_ERR;
        return NULL;
    }
    if (GetNextTlv(buffer) > boundary) {
        *retCode = TLV_ERR_PARSE_PAYLOAD_ERR;
        return NULL;
    }
    tlv->tag = ((TlvCommon *)buffer)->tag;
    tlv->len = ((TlvCommon *)buffer)->len;
    tlv->value = (uint8_t *)buffer + TLV_TLV_HEAD_LEN;
    *retCode = TLV_OK;
    return GetNextTlv(buffer);
}

static uint8_t *AppendTlv(uint8_t *buffer, const TlvCommon *tlv, const uint8_t *boundary, uint32_t *retCode)
{
    if (buffer > boundary) {
        *retCode = TLV_ERR_BUFF_NO_ENOUGH;
        return NULL;
    }
    if (buffer + ((TlvCommon *)tlv)->len + TLV_TLV_HEAD_LEN > boundary) {
        *retCode = TLV_ERR_BUFF_NO_ENOUGH;
        return NULL;
    }
    ((TlvCommon *)buffer)->tag = tlv->tag;
    ((TlvCommon *)buffer)->len = tlv->len;
    if (tlv->len != 0 && tlv->value != NULL) {
        if (memcpy_s(buffer + TLV_TLV_HEAD_LEN, boundary - buffer - TLV_TLV_HEAD_LEN, tlv->value, tlv->len) != EOK) {
            *retCode = TLV_ERR_BUFF_NO_ENOUGH;
            return NULL;
        }
    }
    *retCode = TLV_OK;
    return GetNextTlv(buffer);
}

uint32_t Serialize(const TlvCommon *tlv, uint32_t tlvCount, uint8_t *buff, uint32_t maxBuffSize, uint32_t *buffSize)
{
    if (tlv == NULL || buff == NULL || buffSize == NULL) {
        return TLV_ERR_INVALID_PARA;
    }

    uint8_t *curr = buff;
    uint8_t *boundary = buff + maxBuffSize;

    uint32_t retCode = TLV_OK;
    for (uint32_t index = 0; index < tlvCount; index++) {
        curr = AppendTlv(curr, &tlv[index], boundary, &retCode);
        if (curr == NULL || retCode != TLV_OK) {
            return retCode;
        }
    }
    *buffSize = curr - buff;
    return TLV_OK;
}

uint32_t Deserialize(const uint8_t *buff, uint32_t buffSize, TlvCommon *tlv, uint32_t maxTlvCount, uint32_t *tlvCount)
{
    if (buff == NULL || tlv == NULL || tlvCount == NULL) {
        return TLV_ERR_INVALID_PARA;
    }

    uint8_t *msg = (uint8_t *)buff;
    const uint8_t *boundary = buff + buffSize;
    uint32_t index = 0;

    while (msg != NULL) {
        if (index >= maxTlvCount) {
            return TLV_ERR_BUFF_NO_ENOUGH;
        }
        uint32_t retCode = TLV_OK;
        msg = ParseTlv(msg, &tlv[index], boundary, &retCode);
        if (msg == NULL || retCode != TLV_OK) {
            break;
        }
        index++;
    }

    *tlvCount = index;
    return TLV_OK;
}
