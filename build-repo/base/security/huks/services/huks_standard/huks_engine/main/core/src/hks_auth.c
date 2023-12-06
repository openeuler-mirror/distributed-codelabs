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

#include "hks_auth.h"

#include <stddef.h>

#include "hks_base_check.h"
#include "hks_log.h"
#include "hks_param.h"
#include "hks_template.h"

struct HksAuthPolicy {
    uint32_t authId;
    uint32_t policyCnt;
    uint32_t *policyTag;
};

#ifndef _CUT_AUTHENTICATE_
static uint32_t g_symCipherPolicyTag[] = { HKS_TAG_ALGORITHM, HKS_TAG_BLOCK_MODE, HKS_TAG_PADDING, HKS_TAG_PURPOSE };
static uint32_t g_asymCipherPolicyTag[] = { HKS_TAG_ALGORITHM, HKS_TAG_DIGEST, HKS_TAG_PADDING, HKS_TAG_PURPOSE };
static uint32_t g_signVerifyRsaPolicyTag[] = { HKS_TAG_ALGORITHM, HKS_TAG_DIGEST, HKS_TAG_PADDING, HKS_TAG_PURPOSE };
static uint32_t g_signVerifyEccPolicyTag[] = { HKS_TAG_ALGORITHM, HKS_TAG_DIGEST, HKS_TAG_PURPOSE };
static uint32_t g_signVerifyEd25519PolicyTag[] = { HKS_TAG_PURPOSE };
static uint32_t g_macPolicyTag[] = { HKS_TAG_DIGEST, HKS_TAG_PURPOSE };
static uint32_t g_macSm3PolicyTag[] = { HKS_TAG_ALGORITHM, HKS_TAG_DIGEST, HKS_TAG_PURPOSE };
static uint32_t g_derivePolicyTag[] = { HKS_TAG_DIGEST, HKS_TAG_PURPOSE };
static uint32_t g_agreePolicyTag[] = { HKS_TAG_PURPOSE };

struct HksAuthPolicy g_authPolicyList[] = {
    { HKS_AUTH_ID_SYM_CIPHER, HKS_ARRAY_SIZE(g_symCipherPolicyTag), g_symCipherPolicyTag },
    { HKS_AUTH_ID_ASYM_CIPHER, HKS_ARRAY_SIZE(g_asymCipherPolicyTag), g_asymCipherPolicyTag },
    { HKS_AUTH_ID_SIGN_VERIFY_RSA, HKS_ARRAY_SIZE(g_signVerifyRsaPolicyTag), g_signVerifyRsaPolicyTag },
    { HKS_AUTH_ID_SIGN_VERIFY_ECC, HKS_ARRAY_SIZE(g_signVerifyEccPolicyTag), g_signVerifyEccPolicyTag },
    { HKS_AUTH_ID_SIGN_VERIFY_ED25519, HKS_ARRAY_SIZE(g_signVerifyEd25519PolicyTag), g_signVerifyEd25519PolicyTag },
    { HKS_AUTH_ID_MAC_HMAC, HKS_ARRAY_SIZE(g_macPolicyTag), g_macPolicyTag },
    { HKS_AUTH_ID_MAC_SM3, HKS_ARRAY_SIZE(g_macSm3PolicyTag), g_macSm3PolicyTag },
    { HKS_AUTH_ID_DERIVE, HKS_ARRAY_SIZE(g_derivePolicyTag), g_derivePolicyTag },
    { HKS_AUTH_ID_AGREE, HKS_ARRAY_SIZE(g_agreePolicyTag), g_agreePolicyTag }
};

static int32_t CheckPurpose(const struct HksParam *authParam, const struct HksParam *requestParam)
{
    if (requestParam->uint32Param == 0) {
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    if ((requestParam->uint32Param & authParam->uint32Param) != requestParam->uint32Param) {
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    return HKS_SUCCESS;
}

static int32_t OptionalParamCheck(uint32_t authTag, uint32_t alg, uint32_t purpose, const struct HksParamSet *paramSet,
    const struct ParamsValues* paramValues)
{
    HKS_LOG_I("tag is 0x%" LOG_PUBLIC "x", authTag);
    struct HksParam *param = NULL;
    bool isAbsent = false;
    int32_t ret = HksGetParam(paramSet, authTag, &param);
    if (ret == HKS_ERROR_INVALID_ARGUMENT) {
        HKS_LOG_E("get auth param 0x%" LOG_PUBLIC "x failed!", authTag);
        return ret;
    }
    if (ret == HKS_ERROR_PARAM_NOT_EXIST) {
        HKS_LOG_D("when generates key, the tag is absent. tag is 0x%" LOG_PUBLIC "x", authTag);
        isAbsent = true;
    }

    ret = HksCheckOptionalParam(authTag, alg, purpose, isAbsent, param);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("check optional param fail");
        return ret;
    }
    if (((purpose & HKS_KEY_PURPOSE_DERIVE) != 0) || ((purpose & HKS_KEY_PURPOSE_MAC) != 0)) {
        HKS_LOG_E("derive or mac no need to check");
        return HKS_SUCCESS;
    }
    // Parameter check is more strict than above
    return HksCheckGenKeyMutableParams(alg, paramValues);
}

static int32_t GetAlgAndPurposeParam(const struct HksParamSet *keyBlobParamSet, const struct HksParamSet *paramSet,
    struct HksParam **algParam, struct HksParam **purposeParam, struct ParamsValues* paramValues)
{
    int32_t ret = HksGetParam(keyBlobParamSet, HKS_TAG_ALGORITHM, algParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_CHECK_GET_ALG_FAIL,
        "get param  0x%" LOG_PUBLIC "x failed!", HKS_TAG_ALGORITHM);
    ret = HksGetParam(keyBlobParamSet, HKS_TAG_PURPOSE, purposeParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_CHECK_GET_PURPOSE_FAIL,
        "get param  0x%" LOG_PUBLIC "x failed!", HKS_TAG_PURPOSE);
    return GetInputParams(paramSet, paramValues);
}

static int32_t AuthPolicy(const struct HksAuthPolicy *policy, const struct HksParamSet *keyBlobParamSet,
    const struct HksParamSet *paramSet)
{
    uint32_t authTag;
    struct HksParam *authParam = NULL;
    struct HksParam *requestParam = NULL;
    struct HksParam *algParam = NULL;
    struct HksParam *purposeParam = NULL;
    struct ParamsValues paramValues = { { false, 0, false }, { true, 0, false }, { true, 0, false },
        { true, 0, false }, { true, 0, false } };
    int32_t ret = GetAlgAndPurposeParam(keyBlobParamSet, paramSet, &algParam, &purposeParam, &paramValues);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("GetAlgAndPurposeParam failed");
        return ret;
    }
    for (uint32_t i = 0; i < policy->policyCnt; i++) {
        authTag = policy->policyTag[i];
        ret = HksGetParam(keyBlobParamSet, authTag, &authParam);
        if (ret == HKS_ERROR_INVALID_ARGUMENT) {
            HKS_LOG_E("get auth param 0x%" LOG_PUBLIC "x failed!", authTag);
            return ret;
        }
        if (ret == HKS_ERROR_PARAM_NOT_EXIST) {
            ret = OptionalParamCheck(authTag, algParam->uint32Param, purposeParam->uint32Param, paramSet, &paramValues);
            if (ret != HKS_SUCCESS) {
                return ret;
            }
            continue;
        }
        ret = HksGetParam(paramSet, authTag, &requestParam);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("get request param 0x%" LOG_PUBLIC "x failed!", authTag);
            return ret;
        }
        if (authTag != HKS_TAG_PURPOSE) {
            ret = HksCheckParamMatch((const struct HksParam *)authParam, (const struct HksParam *)requestParam);
        } else {
            ret = CheckPurpose((const struct HksParam *)authParam, (const struct HksParam *)requestParam);
        }
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("unmatch policy 0x%" LOG_PUBLIC "x , 0x%" LOG_PUBLIC "x != 0x%" LOG_PUBLIC "x!", authTag,
                requestParam->uint32Param, authParam->uint32Param);
            return ret;
        }
    }
    return HKS_SUCCESS;
}

int32_t HksAuth(uint32_t authId, const struct HksKeyNode *keyNode, const struct HksParamSet *paramSet)
{
    for (uint32_t i = 0; i < HKS_ARRAY_SIZE(g_authPolicyList); i++) {
        if (authId == g_authPolicyList[i].authId) {
            return AuthPolicy(&g_authPolicyList[i], keyNode->paramSet, paramSet);
        }
    }
    return HKS_ERROR_BAD_STATE;
}

int32_t HksThreeStageAuth(uint32_t authId, const struct HuksKeyNode *keyNode)
{
    for (uint32_t i = 0; i < HKS_ARRAY_SIZE(g_authPolicyList); i++) {
        if (authId == g_authPolicyList[i].authId) {
            return AuthPolicy(&g_authPolicyList[i], keyNode->keyBlobParamSet, keyNode->runtimeParamSet);
        }
    }
    return HKS_ERROR_BAD_STATE;
}
#endif /* _CUT_AUTHENTICATE_ */
