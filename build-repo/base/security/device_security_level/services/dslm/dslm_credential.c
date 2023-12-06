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

#include "dslm_credential.h"

#include <stdbool.h>
#include <string.h>

#include "securec.h"

#include "utils_log.h"
#include "utils_mem.h"

#define MAX_CRED_LEN 81920

static inline ProcessDslmCredFunctions *GetFunctionCb()
{
    static ProcessDslmCredFunctions cb = {NULL, NULL, NULL, 0, {0}};
    return &cb;
}

bool InitDslmCredentialFunctions(const ProcessDslmCredFunctions *funcs)
{
    if (funcs == NULL) {
        return false;
    }
    ProcessDslmCredFunctions *cb = GetFunctionCb();
    (void)memcpy_s(cb, sizeof(ProcessDslmCredFunctions), funcs, sizeof(ProcessDslmCredFunctions));
    SECURITY_LOG_INFO("success");
    return true;
}

int32_t DefaultRequestDslmCred(const DeviceIdentify *device, const RequestObject *obj, DslmCredBuff **credBuff)
{
    ProcessDslmCredFunctions *cb = GetFunctionCb();
    RequestDslmCredFunc *request = cb->requestFunc;
    if (request != NULL) {
        return request(device, obj, credBuff);
    }
    SECURITY_LOG_INFO("failed");
    return -1;
}

int32_t DefaultVerifyDslmCred(const DeviceIdentify *device, uint64_t challenge, const DslmCredBuff *credBuff,
    DslmCredInfo *credInfo)
{
    ProcessDslmCredFunctions *cb = GetFunctionCb();
    VerifyDslmCredFunc *verify = cb->verifyFunc;
    if (verify != NULL) {
        return verify(device, challenge, credBuff, credInfo);
    }
    SECURITY_LOG_INFO("invoke DefaultVerifyDslmCred");
    return -1;
}

int32_t DefaultInitDslmCred(DslmCredInfo *credInfo)
{
    ProcessDslmCredFunctions *cb = GetFunctionCb();
    InitDslmCredFunc *init = cb->initFunc;
    if (init != NULL) {
        return init(credInfo);
    }
    SECURITY_LOG_INFO("invoke DefaultInitDslmCred");
    return -1;
}

int32_t GetSupportedCredTypes(CredType *list, uint32_t len)
{
    if (list == NULL || len == 0) {
        return 0;
    }
    ProcessDslmCredFunctions *cb = GetFunctionCb();
    uint32_t outLen = len;
    if (len > cb->credTypeCnt) {
        outLen = cb->credTypeCnt;
    }
    for (uint32_t i = 0; i < outLen; i++) {
        *(list + i) = cb->credTypeArray[i];
    }
    return (int32_t)outLen;
}

DslmCredBuff *CreateDslmCred(CredType type, uint32_t len, uint8_t *value)
{
    if (value == NULL || len == 0 || len > MAX_CRED_LEN) {
        return NULL;
    }

    DslmCredBuff *outBuff = (DslmCredBuff *)MALLOC(sizeof(DslmCredBuff));
    if (outBuff == NULL) {
        return NULL;
    }
    uint8_t *outValue = (uint8_t *)MALLOC(len);
    if (outValue == NULL) {
        FREE(outBuff);
        return NULL;
    }
    (void)memset_s(outValue, len, 0, len);
    if (memcpy_s(outValue, len, value, len) != EOK) {
        FREE(outBuff);
        FREE(outValue);
        return NULL;
    }
    outBuff->credVal = outValue;
    outBuff->type = type;
    outBuff->credLen = len;
    return outBuff;
}

void DestroyDslmCred(DslmCredBuff *credBuff)
{
    if (credBuff == NULL) {
        return;
    }
    if (credBuff->credVal != NULL) {
        FREE(credBuff->credVal);
        credBuff->credVal = NULL;
    }
    FREE(credBuff);
}