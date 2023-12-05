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

#ifndef DSLM_CREDENTIAL_H
#define DSLM_CREDENTIAL_H

#include <stdint.h>

#include "device_security_defines.h"
#include "dslm_cred.h"

#ifdef __cplusplus
extern "C" {
#endif

int32_t DefaultRequestDslmCred(const DeviceIdentify *device, const RequestObject *obj, DslmCredBuff **credBuff);

int32_t DefaultVerifyDslmCred(const DeviceIdentify *device, uint64_t challenge, const DslmCredBuff *credBuff,
    DslmCredInfo *credInfo);

int32_t DefaultInitDslmCred(DslmCredInfo *credInfo);

int32_t GetSupportedCredTypes(CredType *list, uint32_t len);

#ifdef __cplusplus
}
#endif
#endif // DSLM_CREDENTIAL_H
