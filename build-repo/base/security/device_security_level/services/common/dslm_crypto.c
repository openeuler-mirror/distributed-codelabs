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

#include "dslm_crypto.h"

#include <stddef.h>
#include <stdint.h>

#include "openssl/rand.h"

void GenerateRandom(RandomValue *rand, uint32_t length)
{
    if (rand == NULL) {
        return;
    }
    rand->length = (length > RANDOM_MAX_LEN) ? RANDOM_MAX_LEN : length;

    RAND_bytes(&rand->value[0], rand->length);
}
