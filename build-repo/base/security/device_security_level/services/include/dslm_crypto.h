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

#ifndef DSLM_CRYPTO_H
#define DSLM_CRYPTO_H

#include <stdint.h>
#include <stdlib.h>

#define RANDOM_MAX_LEN 32

#define TYPE_ECDSA_SHA_256 0
#define TYPE_ECDSA_SHA_384 1

#ifdef __cplusplus
extern "C" {
#endif

typedef struct RandomValue {
    uint32_t length;
    uint8_t value[RANDOM_MAX_LEN];
} RandomValue;

void GenerateRandom(RandomValue *rand, uint32_t length);

#ifdef __cplusplus
}
#endif

#endif /* DSLM_CRYPTO_H */