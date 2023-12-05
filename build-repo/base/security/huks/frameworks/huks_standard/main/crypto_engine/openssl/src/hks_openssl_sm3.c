/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef HKS_CONFIG_FILE
#include HKS_CONFIG_FILE
#else
#include "hks_config.h"
#endif

#ifdef HKS_SUPPORT_SM3_C

#include "hks_log.h"
#include "hks_openssl_common.h"
#include "hks_openssl_engine.h"
#include "hks_openssl_sm3.h"
#include "hks_template.h"

#ifdef HKS_SUPPORT_SM3_GENERATE_KEY
static int32_t Sm3GenKeyCheckParam(const struct HksKeySpec *spec)
{
    if ((spec->keyLen == 0) || (spec->keyLen % BIT_NUM_OF_UINT8 != 0)) {
        HKS_LOG_E("keyLen is wrong, len = %" LOG_PUBLIC "u", spec->keyLen);
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    return HKS_SUCCESS;
}

int32_t HksOpensslSm3GenerateKey(const struct HksKeySpec *spec, struct HksBlob *key)
{
    HKS_IF_NOT_SUCC_LOGE_RETURN(Sm3GenKeyCheckParam(spec),
        HKS_ERROR_INVALID_ARGUMENT, "sm3 generate key invalid params!")
    return HksOpensslGenerateRandomKey(spec->keyLen, key);
}
#endif /* HKS_SUPPORT_SM3_GENERATE_KEY */

#endif /* HKS_SUPPORT_SM3_C */
