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

#include "utils_base64.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "securec.h"

#include "utils_log.h"
#include "utils_mem.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_MALLOC_LEN  (1 * 1024 * 1024)
#define RESIZE4(n)      (((n) + 3) & ~3)

static const char *g_base64EncodeTable = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const uint8_t g_base64DecodeTable[256] = { /* 256 due to character size */
    65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
    65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
    65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 62, 65, 65, 65, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 65, 65, 65, 0, 65, 65,
    65, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 65, 65, 65, 65, 65,
    65, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 65, 65, 65, 65, 65,
    65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
    65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
    65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
    65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
    65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
    65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
    65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
    65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65
};

/*
 * This function is for Base64 encoding based on the public algorithm
 */
static int32_t Base64Encode(const uint8_t *from, uint32_t fromLen, uint8_t *to, uint32_t toCheckLen)
{
    bool isInvalidParam = ((from == NULL) || (to == NULL) || (fromLen == 0));
    if (isInvalidParam) {
        return -1;
    }

    uint32_t tmpLen = fromLen;
    uint32_t toLen = ((tmpLen + 2) / 3);          /* Base64 encode size, add 2 for padding, and divided by 3 */
    int padding = (int)(tmpLen % 3);              /* len % 3 to get the padding size. This must be signed type! */
    padding = (padding == 0) ? 0 : (3 - padding); /* For the padding block out of 3 */
    if (toLen >= (toLen * 4)) {                   /* For integer overflow and toLen can not be 0, with 4X */
        return -1;
    }
    toLen *= 4; /* For 4 blocks */
    if (toCheckLen < toLen) {
        return -1;
    }

    uint32_t j = 0;
    for (uint32_t i = 0; i < fromLen;) {
        uint32_t a = (i < fromLen) ? (uint8_t)from[i] : 0;
        i++;
        uint32_t b = (i < fromLen) ? (uint8_t)from[i] : 0;
        i++;
        uint32_t c = (i < fromLen) ? (uint8_t)from[i] : 0;
        i++;
        uint32_t byte = (((a << 16) & 0x00FF0000) | ((b << 8) & 0x0000FF00) | ((c << 0) & 0x000000FF)); /* 16, 8 */

        to[j++] = g_base64EncodeTable[(byte >> 18) & 0x3F]; /* get the 1st block by shift 18 */
        to[j++] = g_base64EncodeTable[(byte >> 12) & 0x3F]; /* get the 2nd block by shift 12 */
        to[j++] = g_base64EncodeTable[(byte >> 6) & 0x3F];  /* get the 3rd block by shift 6 */
        to[j++] = g_base64EncodeTable[(byte >> 0) & 0x3F];
    }

    if (padding-- > 0) {
        to[toLen - 1] = '='; /* padding the -1 to "=" */
    }
    if (padding-- > 0) {
        to[toLen - 2] = '='; /* padding the -2 to "=" */
    }

    return toLen;
}

/*
 * This function is for Base64 decoding based on the public algorithm
 */
static int32_t Base64Decode(const uint8_t *from, uint8_t *to, uint32_t toCheckLen)
{
    bool isInvalidParam = ((from == NULL) || (to == NULL));
    if (isInvalidParam) {
        return -1;
    }

    uint32_t fromLen = strlen((char *)from);
    // base64 encode Valid data Length is 4
    bool isInvalidLength = ((fromLen < 4) || ((fromLen % 4) != 0));
    if (isInvalidLength) {
        return -1;
    }

    uint32_t toLen = fromLen / 4 * 3; /* Base64 decode size */
    if (from[fromLen - 1] == '=') {   /* if last 1-bit is null,  make it "=" */
        toLen--;
    }
    if (from[fromLen - 2] == '=') { /* if last 2-bit is null, make it "=" */
        toLen--;
    }
    if (toCheckLen < toLen) {
        return -1;
    }

    uint32_t j = 0;
    for (uint32_t i = 0; i < fromLen;) {
        uint32_t a = g_base64DecodeTable[from[i++]];
        uint32_t b = g_base64DecodeTable[from[i++]];
        uint32_t c = g_base64DecodeTable[from[i++]];
        uint32_t d = g_base64DecodeTable[from[i++]];

        // 64 is decode table max valid num.
        bool isInvalidVariable = ((a > 64) || (b > 64) || (c > 64) || (d > 64));
        if (isInvalidVariable) {
            return -1;
        }

        // Converts four 6-bit data into three 8-bit data
        uint32_t byte =
            (((a << 18) & 0x00FC0000) | ((b << 12) & 0x0003F000) | ((c << 6) & 0x00000FC0) | ((d << 0) & 0x0000003F));

        if (j < toLen) {
            to[j++] = (byte >> 16) & 0xFF; // 16, the 3rd 8-bit
        }
        if (j < toLen) {
            to[j++] = (byte >> 8) & 0xFF; // 8, the 2nd 8-bit
        }
        if (j < toLen) {
            to[j++] = (byte >> 0) & 0xFF;
        }
    }
    to[toLen] = '\0';
    return toLen;
}

uint8_t *Base64EncodeApp(const uint8_t *from, uint32_t fromLen)
{
    if (from == NULL) {
        SECURITY_LOG_DEBUG("invalid param, from is null");
        return NULL;
    }

    uint32_t outSize = (fromLen + 2) / 3 * 4; // get base64 encode size
    if (outSize + 1 > MAX_MALLOC_LEN) {
        SECURITY_LOG_DEBUG("invalid MALLOC length");
        return NULL;
    }
    uint8_t *out = (uint8_t *)MALLOC(outSize + 1);
    if (out == NULL) {
        SECURITY_LOG_DEBUG("malloc failed, out is null");
        return NULL;
    }

    int realLen = Base64Encode(from, fromLen, out, outSize);
    if (realLen < 0) {
        SECURITY_LOG_DEBUG("Base64EncodeApp failed");
        FREE(out);
        return NULL;
    }
    out[realLen] = '\0';
    return out;
}

int32_t Base64DecodeApp(const uint8_t *src, uint8_t **to)
{
    if ((src == NULL) || (to == NULL)) {
        SECURITY_LOG_DEBUG("invalid params");
        return 0;
    }

    uint32_t decodedLen = strlen((char *)src) / 4 * 3; /* Base64 Decode size */
    if (decodedLen + 1 > MAX_MALLOC_LEN) {
        SECURITY_LOG_DEBUG("decodedLen error");
        return 0;
    }

    uint8_t *decoded = (uint8_t *)MALLOC(decodedLen + 1);
    if (decoded == NULL) {
        SECURITY_LOG_DEBUG("malloc failed, decoded is null");
        return 0;
    }

    int realLen = Base64Decode(src, decoded, decodedLen);
    if (realLen < 0) {
        SECURITY_LOG_DEBUG("Base64Decode failed");
        FREE(decoded);
        return 0;
    }

    decoded[realLen] = '\0';
    *to = decoded;
    return realLen;
}

int32_t Base64UrlDecodeApp(const uint8_t *src, uint8_t **to)
{
    if ((src == NULL) || (to == NULL)) {
        SECURITY_LOG_DEBUG("invalid params");
        return 0;
    }

    uint32_t sourceLen = (uint32_t)strlen((char*)src);
    uint32_t alignLen = RESIZE4(sourceLen);
    uint8_t *base64Str = (uint8_t *)malloc(alignLen + 1);
    if (base64Str == NULL) {
        SECURITY_LOG_DEBUG("Base64UrlDecodeApp malloc failed");
        return 0;
    }
    (void)memset_s(base64Str, alignLen + 1, '=', alignLen + 1);
    for (uint32_t i = 0; i < sourceLen; i++) {
        if (src[i] == '-') {
            base64Str[i] = '+';
        } else if (src[i] == '_') {
            base64Str[i] = '/';
        } else {
            base64Str[i] = src[i];
        }
    }
    base64Str[alignLen] = '\0';
    const uint8_t *from = base64Str;
    int32_t realLength = Base64DecodeApp(from, to);
    free(base64Str);
    return realLength;
}

#ifdef __cplusplus
}
#endif