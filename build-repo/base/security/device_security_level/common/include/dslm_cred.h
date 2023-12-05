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

#ifndef DSLM_CRED_H
#define DSLM_CRED_H

#include <stdbool.h>
#include <stdint.h>

#include "device_security_defines.h"

#define CRED_INFO_VERSION_LEN 32
#define CRED_INFO_TYPE_LEN 32
#define CRED_INFO_SIGNTIME_LEN 32
#define CRED_INFO_UDID_LEN 80
#define CRED_INFO_MANU_LEN 64
#define CRED_INFO_MODEL_LEN 64
#define CRED_INFO_BRAND_LEN 64
#define CRED_INFO_LEVEL_LEN 16
#define CRED_INFO_SOFTVERSION_LEN 64

typedef enum {
    CRED_TYPE_MINI = 1000,
    CRED_TYPE_SMALL = 2000,
    CRED_TYPE_STANDARD = 3000,
    CRED_TYPE_LARGE = 4000,
} CredType;

typedef struct DslmCredInfo {
    char version[CRED_INFO_VERSION_LEN];  // the cred version
    char releaseType[CRED_INFO_TYPE_LEN]; // debug or release
    char signTime[CRED_INFO_SIGNTIME_LEN];
    char udid[CRED_INFO_UDID_LEN];
    char manufacture[CRED_INFO_MANU_LEN];
    char model[CRED_INFO_MODEL_LEN];
    char brand[CRED_INFO_BRAND_LEN];
    char securityLevel[CRED_INFO_LEVEL_LEN];
    char softwareVersion[CRED_INFO_SOFTVERSION_LEN];
    CredType credType;  // the parsed and validated type value, assigned only after verification.
    uint32_t credLevel; // the parsed level value, assigned only after verification.
} DslmCredInfo;

#define MAX_CRED_ARRAY_SIZE 48
typedef struct RequestObject {
    uint32_t version;
    uint64_t challenge;
    uint32_t arraySize;
    CredType credArray[MAX_CRED_ARRAY_SIZE];
} RequestObject;

typedef struct DslmCredBuff {
    CredType type;
    uint32_t credLen;
    uint8_t *credVal;
} DslmCredBuff;

typedef int32_t InitDslmCredFunc(DslmCredInfo *credInfo);

typedef int32_t RequestDslmCredFunc(const DeviceIdentify *device, const RequestObject *obj, DslmCredBuff **credBuff);
typedef int32_t VerifyDslmCredFunc(const DeviceIdentify *device, uint64_t challenge, const DslmCredBuff *credBuff,
    DslmCredInfo *credInfo);

typedef struct ProcessDslmCredFunctions {
    InitDslmCredFunc *initFunc;
    RequestDslmCredFunc *requestFunc;
    VerifyDslmCredFunc *verifyFunc;
    uint32_t credTypeCnt;
    CredType credTypeArray[MAX_CRED_ARRAY_SIZE];
} ProcessDslmCredFunctions;

#ifdef __cplusplus
extern "C" {
#endif

bool InitDslmCredentialFunctions(const ProcessDslmCredFunctions *func);

DslmCredBuff *CreateDslmCred(CredType type, uint32_t len, uint8_t *value);

void DestroyDslmCred(DslmCredBuff *credBuff);

#ifdef __cplusplus
}
#endif
#endif // DSLM_CRED_H
