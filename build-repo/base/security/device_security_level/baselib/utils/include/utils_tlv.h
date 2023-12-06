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

#ifndef SEC_UTILS_TLV_H
#define SEC_UTILS_TLV_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TLV_OK 0
#define TLV_ERR 1001
#define TLV_ERR_INVALID_PARA 1002
#define TLV_ERR_PARSE_PAYLOAD_ERR 1003
#define TLV_ERR_BUFF_NO_ENOUGH 1004

typedef struct TlvCommon {
    uint16_t tag;
    uint16_t len;
    void *value;
} TlvCommon;

uint32_t Serialize(const TlvCommon *tlv, uint32_t tlvCount, uint8_t *buff, uint32_t maxBuffSize, uint32_t *buffSize);

uint32_t Deserialize(const uint8_t *buff, uint32_t buffSize, TlvCommon *tlv, uint32_t maxTlvCount, uint32_t *tlvCount);

#ifdef __cplusplus
}
#endif

#endif // SEC_UTILS_TLV_H