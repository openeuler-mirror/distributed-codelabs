/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "hks_client_check.h"

#include <stddef.h>

#include "hks_base_check.h"
#include "hks_common_check.h"
#include "hks_log.h"
#include "hks_param.h"
#include "hks_template.h"

#ifndef _CUT_AUTHENTICATE_
static int32_t CheckProcessNameAndKeyAliasSize(uint32_t processNameSize, uint32_t keyAliasSize)
{
    if (processNameSize > HKS_MAX_PROCESS_NAME_LEN) {
        HKS_LOG_E("processName size too long, size %" LOG_PUBLIC "u", processNameSize);
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    if (keyAliasSize > HKS_MAX_KEY_ALIAS_LEN) {
        HKS_LOG_E("keyAlias size too long, size %" LOG_PUBLIC "u", keyAliasSize);
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    return HKS_SUCCESS;
}

int32_t HksCheckProcessNameAndKeyAlias(const struct HksBlob *processName, const struct HksBlob *keyAlias)
{
    HKS_IF_NOT_SUCC_RETURN(HksCheckBlob2(processName, keyAlias), HKS_ERROR_INVALID_ARGUMENT)

    return CheckProcessNameAndKeyAliasSize(processName->size, keyAlias->size);
}

int32_t HksCheckGenAndImportKeyParams(const struct HksBlob *processName, const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSetIn, const struct HksBlob *key)
{
    int32_t ret = HksCheckBlob3AndParamSet(processName, keyAlias, key, paramSetIn);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    return CheckProcessNameAndKeyAliasSize(processName->size, keyAlias->size);
}

int32_t HksCheckImportWrappedKeyParams(const struct HksBlob *processName, const struct HksBlob *keyAlias,
    const struct HksBlob *wrappingKeyAlias, const struct HksParamSet *paramSetIn, const struct HksBlob *wrappedKeyData)
{
    int32_t ret = HksCheckBlob4AndParamSet(processName, keyAlias, wrappingKeyAlias, wrappedKeyData, paramSetIn);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    ret = CheckProcessNameAndKeyAliasSize(processName->size, keyAlias->size);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    return CheckProcessNameAndKeyAliasSize(processName->size, wrappingKeyAlias->size);
}

int32_t HksCheckAllParams(const struct HksBlob *processName, const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSet, const struct HksBlob *data1, const struct HksBlob *data2)
{
    int32_t ret = HksCheckBlob4AndParamSet(processName, keyAlias, data1, data2, paramSet);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    return CheckProcessNameAndKeyAliasSize(processName->size, keyAlias->size);
}

int32_t HksCheckServiceInitParams(const struct HksBlob *processName, const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSet)
{
    int32_t ret = HksCheckBlob2AndParamSet(processName, keyAlias, paramSet);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    return CheckProcessNameAndKeyAliasSize(processName->size, keyAlias->size);
}

int32_t HksCheckGetKeyParamSetParams(const struct HksBlob *processName, const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSet)
{
    HKS_IF_NOT_SUCC_RETURN(HksCheckProcessNameAndKeyAlias(processName, keyAlias), HKS_ERROR_INVALID_ARGUMENT)

    if ((paramSet == NULL) || (paramSet->paramSetSize == 0)) {
        HKS_LOG_E("invalid paramSet");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    return HKS_SUCCESS;
}

int32_t HksCheckExportPublicKeyParams(const struct HksBlob *processName, const struct HksBlob *keyAlias,
    const struct HksBlob *key)
{
    HKS_IF_NOT_SUCC_RETURN(HksCheckBlob3(processName, keyAlias, key), HKS_ERROR_INVALID_ARGUMENT)

    return CheckProcessNameAndKeyAliasSize(processName->size, keyAlias->size);
}

int32_t HksCheckDeriveKeyParams(const struct HksBlob *processName, const struct HksParamSet *paramSet,
    const struct HksBlob *mainKey, const struct HksBlob *derivedKey)
{
    return HksCheckGenAndImportKeyParams(processName, mainKey, paramSet, derivedKey);
}

int32_t HksCheckGetKeyInfoListParams(const struct HksBlob *processName, const struct HksKeyInfo *keyInfoList,
    const uint32_t *listCount)
{
    HKS_IF_NOT_SUCC_LOGE_RETURN(CheckBlob(processName), HKS_ERROR_INVALID_ARGUMENT, "invalid processName")

    if (processName->size > HKS_MAX_PROCESS_NAME_LEN) {
        HKS_LOG_E("processName size too long, size %" LOG_PUBLIC "u", processName->size);
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    if ((keyInfoList == NULL) || (listCount == NULL)) {
        HKS_LOG_E("keyInfoList or listCount null.");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    for (uint32_t i = 0; i < *listCount; ++i) {
        if ((CheckBlob(&(keyInfoList[i].alias)) != HKS_SUCCESS) ||
            (keyInfoList[i].paramSet == NULL) || (keyInfoList[i].paramSet->paramSetSize == 0)) {
            return HKS_ERROR_INVALID_ARGUMENT;
        }
    }

    return HKS_SUCCESS;
} 
#endif /* _CUT_AUTHENTICATE_ */

int32_t HksCheckGenerateRandomParams(const struct HksBlob *processName, const struct HksBlob *random)
{
    HKS_IF_NOT_SUCC_RETURN(HksCheckBlob2(processName, random), HKS_ERROR_INVALID_ARGUMENT)

    if (processName->size > HKS_MAX_PROCESS_NAME_LEN) {
        HKS_LOG_E("processName size too long, size %" LOG_PUBLIC "u.", processName->size);
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    if (random->size > HKS_MAX_RANDOM_LEN) {
        HKS_LOG_E("random size too long, size %" LOG_PUBLIC "u.", random->size);
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    return HKS_SUCCESS;
}

#ifdef HKS_SUPPORT_API_ATTEST_KEY
int32_t HksCheckAttestKeyParams(const struct HksBlob *processName, const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSet, struct HksBlob *certChain)
{
    return HksCheckGenAndImportKeyParams(processName, keyAlias, paramSet, certChain);
}
#endif

#ifdef HKS_SUPPORT_USER_AUTH_ACCESS_CONTROL
static int32_t CheckAuthAccessLevel(const struct HksParamSet *paramSet)
{
    struct HksParam *authAccess = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_KEY_AUTH_ACCESS_TYPE, &authAccess);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_CHECK_GET_ACCESS_TYPE_FAILED, "get auth access type fail")

    if (authAccess->uint32Param < HKS_AUTH_ACCESS_INVALID_CLEAR_PASSWORD) {
        HKS_LOG_E("auth access level is too low");
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    return HKS_SUCCESS;
}

static int32_t CheckUserAuthParamsValidity(const struct HksParamSet *paramSet, uint32_t userAuthType,
    uint32_t authAccessType, uint32_t challengeType)
{
    int32_t ret = HksCheckUserAuthParams(userAuthType, authAccessType, challengeType);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "check user auth params failed")

    if (challengeType == HKS_CHALLENGE_TYPE_NONE) {
        struct HksParam *authTimeout = NULL;
        ret = HksGetParam(paramSet, HKS_TAG_AUTH_TIMEOUT, &authTimeout);
        if (ret == HKS_SUCCESS) {
            if (authTimeout->uint32Param > MAX_AUTH_TIMEOUT_SECOND || authTimeout->uint32Param == 0) {
                HKS_LOG_E("invalid auth timeout param");
                return HKS_ERROR_INVALID_TIME_OUT;
            }
        }
    }

    struct HksParam *secureSignType = NULL;
    ret = HksGetParam(paramSet, HKS_TAG_KEY_SECURE_SIGN_TYPE, &secureSignType);
    if (ret == HKS_SUCCESS) {
        ret = HksCheckSecureSignParams(secureSignType->uint32Param);
        HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_INVALID_SECURE_SIGN_TYPE, "secure sign type is invalid")

        /* secure sign ability only support sign-purpose algorithm */
        struct HksParam *purposeParam = NULL;
        ret = HksGetParam(paramSet, HKS_TAG_PURPOSE, &purposeParam);
        if (ret != HKS_SUCCESS || (purposeParam->uint32Param & HKS_KEY_PURPOSE_SIGN) == 0) {
            HKS_LOG_E("secure sign tag only support sign-purpose alg");
            return HKS_ERROR_INVALID_ARGUMENT;
        }
        ret = CheckAuthAccessLevel(paramSet);
        HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_INVALID_ARGUMENT, "check auth access level fail")
    }

    return HKS_SUCCESS;
}
#endif

int32_t HksCheckAndGetUserAuthInfo(const struct HksParamSet *paramSet, uint32_t *userAuthType,
    uint32_t *authAccessType)
{
#ifdef HKS_SUPPORT_USER_AUTH_ACCESS_CONTROL
    HKS_IF_NULL_LOGE_RETURN(paramSet, HKS_ERROR_NOT_SUPPORTED, "null init paramSet: not support user auth!")

    struct HksParam *noRequireAuth = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_NO_AUTH_REQUIRED, &noRequireAuth);
    if (ret == HKS_SUCCESS && noRequireAuth->boolParam == true) {
        HKS_LOG_I("no require auth=true");
        return HKS_ERROR_NOT_SUPPORTED;
    }

    struct HksParam *userAuthTypeParam = NULL;
    ret = HksGetParam(paramSet, HKS_TAG_USER_AUTH_TYPE, &userAuthTypeParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_NOT_SUPPORTED, "no user auth type param: not support user auth!")

    struct HksParam *accessTypeParam = NULL;
    ret = HksGetParam(paramSet, HKS_TAG_KEY_AUTH_ACCESS_TYPE, &accessTypeParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_CHECK_GET_ACCESS_TYPE_FAILED, "get auth access type param failed")

    struct HksParam *challengeTypeParam = NULL;
    ret = HksGetParam(paramSet, HKS_TAG_CHALLENGE_TYPE, &challengeTypeParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_CHECK_GET_CHALLENGE_TYPE_FAILED, "get challenge type param failed")

    ret = CheckUserAuthParamsValidity(paramSet, userAuthTypeParam->uint32Param, accessTypeParam->uint32Param,
        challengeTypeParam->uint32Param);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "check user auth params validity failed")

    *userAuthType = userAuthTypeParam->uint32Param;
    *authAccessType = accessTypeParam->uint32Param;
    return HKS_SUCCESS;
#else
    (void)paramSet;
    (void)userAuthType;
    (void)authAccessType;
    return HKS_SUCCESS;
#endif
}