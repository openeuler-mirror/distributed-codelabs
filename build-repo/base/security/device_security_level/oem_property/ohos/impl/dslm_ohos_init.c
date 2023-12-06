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

#include "dslm_ohos_init.h"

#include <string.h>

#include "utils_log.h"
#include "dslm_credential_utils.h"
#include "dslm_ohos_request.h"
#include "device_security_defines.h"

#define DSLM_CRED_STR_LEN_MAX 4096

int32_t InitOhosDslmCred(DslmCredInfo *credInfo)
{
    SECURITY_LOG_INFO("start");
    char credStr[DSLM_CRED_STR_LEN_MAX] = {0};
    int32_t ret = GetCredFromCurrentDevice(credStr, DSLM_CRED_STR_LEN_MAX);
    if (ret != SUCCESS) {
        SECURITY_LOG_ERROR("read cred data from file failed");
        return ret;
    }

    ret = VerifyDslmCredential(credStr, credInfo, NULL);
    if (ret != SUCCESS) {
        SECURITY_LOG_ERROR("verifyCredData failed!");
        return ret;
    }
    credInfo->credType = CRED_TYPE_STANDARD;

    SECURITY_LOG_INFO("success, self security level is %{public}d", credInfo->credLevel);
    return SUCCESS;
}