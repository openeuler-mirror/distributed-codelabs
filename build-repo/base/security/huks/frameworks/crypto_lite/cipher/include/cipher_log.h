/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CIPHER_LOG_H
#define CIPHER_LOG_H

#include "securec.h"

#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
#endif

#undef LOG_TAG
#define LOG_TAG "CIPHER"
#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002F00 /* Security subsystem's domain id */


enum CipherLogLevel {
    CIPHER_LOG_LEVEL_E,
};

#ifdef __cplusplus
extern "C" {
#endif

void CipherLog(uint32_t logLevel, const char *funcName, uint32_t lineNo, const char *format, ...);

#define CIPHER_LOG_E(...) CipherLog(CIPHER_LOG_LEVEL_E, __func__, __LINE__, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif // CIPHER_LOG_H