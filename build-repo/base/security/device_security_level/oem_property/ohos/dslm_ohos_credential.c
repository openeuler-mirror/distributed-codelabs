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

#include "dslm_ohos_credential.h"

#include "dslm_cred.h"
#include "impl/dslm_ohos_init.h"
#include "impl/dslm_ohos_request.h"
#include "impl/dslm_ohos_verify.h"

__attribute__((constructor)) static void Constructor(void)
{
    const ProcessDslmCredFunctions func = {
        .initFunc = InitOhosDslmCred,
        .requestFunc = RequestOhosDslmCred,
        .verifyFunc = VerifyOhosDslmCred,
        .credTypeCnt = 2,
        .credTypeArray = { CRED_TYPE_STANDARD, CRED_TYPE_SMALL },
    };
    InitDslmCredentialFunctions(&func);
}