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

#ifndef DSLM_CREDENTIAL_UTILS_H
#define DSLM_CREDENTIAL_UTILS_H

#include <stdint.h>

#include "dslm_cred.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DataBuffer {
    uint32_t length;
    uint8_t *data;
} DataBuffer;

typedef DataBuffer PublicKey;

typedef struct AttestationList {
    PublicKey root;
    PublicKey intermediate;
    PublicKey last;
} AttestationList;

int32_t VerifyDslmCredential(const char *credentialString, DslmCredInfo *credentialInfo, AttestationList *list);

void FreeAttestationList(AttestationList *list);

#ifdef __cplusplus
}
#endif

#endif // DSLM_CREDENTIAL_UTILS_H