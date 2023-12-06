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

#include "utils_hexstring.h"

#include <stdio.h>

#include "utils_log.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ERR (-1)
#define OK 0

#define BYTE_TO_HEX_OPER_LENGTH 2
#define MAX_MALLOC_LEN (1 * 1024 * 1024)

static char HexToChar(uint8_t hex)
{
    return (hex > 9) ? (hex + 0x37) : (hex + 0x30); /* check if bigger than 9, then add 0x37 or 0x30 */
}

void ByteToHexString(const uint8_t *hex, uint32_t hexLen, uint8_t *str, uint32_t strLen)
{
    if ((hex == NULL) || (str == NULL) || (strLen < hexLen * BYTE_TO_HEX_OPER_LENGTH)) {
        return;
    }

    for (uint32_t i = 0; i < hexLen; i++) {
        str[i * BYTE_TO_HEX_OPER_LENGTH] = HexToChar((hex[i] & 0xF0) >> 4); /* shift 4 right for filling */
        str[i * BYTE_TO_HEX_OPER_LENGTH + 1] = HexToChar(hex[i] & 0x0F);    /* get low four bits */
    }
}

int32_t HexStringToByte(const char *str, uint32_t strLen, uint8_t *hex, uint32_t hexLen)
{
    if ((str == NULL) || (hex == NULL) || (strLen % BYTE_TO_HEX_OPER_LENGTH)) { /* even number or not */
        return ERR;
    }
    uint32_t outLen = strLen / BYTE_TO_HEX_OPER_LENGTH;

    if (hexLen < outLen) { /* test the length */
        SECURITY_LOG_DEBUG("length error");
        return ERR;
    }

    uint8_t nibble[BYTE_TO_HEX_OPER_LENGTH]; /* create array */

    for (uint32_t i = 0; i < outLen; i++) {
        nibble[0] = str[i * BYTE_TO_HEX_OPER_LENGTH];           /* hex conversion */
        nibble[1] = str[i * BYTE_TO_HEX_OPER_LENGTH + 1];       /* hex conversion */
        for (int32_t j = 0; j < BYTE_TO_HEX_OPER_LENGTH; j++) { /* iterate through array */
            if ((nibble[j] <= 'F') && (nibble[j] >= 'A')) {
                nibble[j] = nibble[j] - 'A' + 10; /* decimal conversion, add 10 */
            } else if ((nibble[j] <= 'f') && (nibble[j] >= 'a')) {
                nibble[j] = nibble[j] - 'a' + 10; /* decimal conversion, add 10 */
            } else if ((nibble[j] >= '0') && (nibble[j] <= '9')) {
                nibble[j] = nibble[j] - '0';
            } else {
                return ERR;
            }
        }
        hex[i] = nibble[0] << 4; /* Set the high nibble, shift 4 */
        hex[i] |= nibble[1];     /* Set the low nibble */
    }
    return OK;
}

#ifdef __cplusplus
}
#endif