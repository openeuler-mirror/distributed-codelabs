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

#ifdef HKS_CONFIG_FILE
#include HKS_CONFIG_FILE
#else
#include "hks_config.h"
#endif

#include "hks_check_paramset.h"

#include <stddef.h>

#include "hks_base_check.h"
#include "hks_common_check.h"
#include "hks_log.h"
#include "hks_param.h"
#include "hks_template.h"
#include "securec.h"

#ifdef _CUT_AUTHENTICATE_
#undef HKS_SUPPORT_HASH_C
#undef HKS_SUPPORT_RSA_C
#undef HKS_SUPPORT_ECC_C
#undef HKS_SUPPORT_X25519_C
#undef HKS_SUPPORT_ED25519_C
#undef HKS_SUPPORT_KDF_PBKDF2
#endif /* _CUT_AUTHENTICATE_ */

#define HKS_DEFAULT_PBKDF2_ITERATION 1000
#define HKS_DEFAULT_PBKDF2_SALT_SIZE 16

#ifndef _CUT_AUTHENTICATE_
static uint32_t g_genKeyAlg[] = {
#ifdef HKS_SUPPORT_RSA_C
    HKS_ALG_RSA,
#endif
#ifdef HKS_SUPPORT_AES_C
    HKS_ALG_AES,
#endif
#ifdef HKS_SUPPORT_ECC_C
    HKS_ALG_ECC,
#endif
#ifdef HKS_SUPPORT_HMAC_C
    HKS_ALG_HMAC,
#endif
#ifdef HKS_SUPPORT_X25519_C
    HKS_ALG_X25519,
#endif
#ifdef HKS_SUPPORT_ED25519_C
    HKS_ALG_ED25519,
#endif
#ifdef HKS_SUPPORT_DSA_C
    HKS_ALG_DSA,
#endif
#ifdef HKS_SUPPORT_DH_C
    HKS_ALG_DH,
#endif
#ifdef HKS_SUPPORT_ECDH_C
    HKS_ALG_ECDH,
#endif
#ifdef HKS_SUPPORT_SM2_C
    HKS_ALG_SM2,
#endif
#ifdef HKS_SUPPORT_SM3_C
    HKS_ALG_SM3,
#endif
#ifdef HKS_SUPPORT_SM4_C
    HKS_ALG_SM4,
#endif
};

static uint32_t g_importKeyAlg[] = {
#ifdef HKS_SUPPORT_RSA_C
    HKS_ALG_RSA,
#endif
#ifdef HKS_SUPPORT_AES_C
    HKS_ALG_AES,
#endif
#ifdef HKS_SUPPORT_ECC_C
    HKS_ALG_ECC,
#endif
#ifdef HKS_SUPPORT_X25519_C
    HKS_ALG_X25519,
#endif
#ifdef HKS_SUPPORT_ED25519_C
    HKS_ALG_ED25519,
#endif
#ifdef HKS_SUPPORT_DSA_C
    HKS_ALG_DSA,
#endif
#ifdef HKS_SUPPORT_DH_C
    HKS_ALG_DH,
#endif
#ifdef HKS_SUPPORT_HMAC_C
    HKS_ALG_HMAC,
#endif
#ifdef HKS_SUPPORT_SM2_C
    HKS_ALG_SM2,
#endif
#ifdef HKS_SUPPORT_SM3_C
    HKS_ALG_SM3,
#endif
#ifdef HKS_SUPPORT_SM4_C
    HKS_ALG_SM4,
#endif
};

static uint32_t g_cipherAlg[] = {
#ifdef HKS_SUPPORT_RSA_C
    HKS_ALG_RSA,
#endif
#ifdef HKS_SUPPORT_AES_C
    HKS_ALG_AES,
#endif
};
#ifdef HKS_SUPPORT_API_SIGN_VERIFY
static uint32_t g_signAlg[] = {
#ifdef HKS_SUPPORT_RSA_C
    HKS_ALG_RSA,
#endif
#ifdef HKS_SUPPORT_DSA_C
    HKS_ALG_DSA,
#endif
#ifdef HKS_SUPPORT_ECC_C
    HKS_ALG_ECC,
#endif
#ifdef HKS_SUPPORT_ED25519_C
    HKS_ALG_ED25519,
#endif
};
#endif

static uint32_t g_agreeAlg[] = {
#ifdef HKS_SUPPORT_ECDH_C
    HKS_ALG_ECDH,
#endif
#ifdef HKS_SUPPORT_X25519_C
    HKS_ALG_X25519,
#endif
#ifdef HKS_SUPPORT_DH_C
    HKS_ALG_DH,
#endif
};

static uint32_t g_agreeAlgLocal[] = {
#ifdef HKS_SUPPORT_ECDH_C
    HKS_ALG_ECDH,
#endif
#ifdef HKS_SUPPORT_X25519_C
    HKS_ALG_X25519,
#endif
#ifdef HKS_SUPPORT_DH_C
    HKS_ALG_DH,
#endif
};

static uint32_t g_unwrapSuite[] = {
#if defined(HKS_SUPPORT_X25519_C) && defined(HKS_SUPPORT_AES_GCM)
    HKS_UNWRAP_SUITE_X25519_AES_256_GCM_NOPADDING,
#endif
#if defined(HKS_SUPPORT_ECDH_C) && defined(HKS_SUPPORT_AES_GCM)
    HKS_UNWRAP_SUITE_ECDH_AES_256_GCM_NOPADDING,
#endif
};
#endif /* _CUT_AUTHENTICATE_ */

static uint32_t g_deriveAlg[] = {
#ifdef HKS_SUPPORT_KDF_HKDF
    HKS_ALG_HKDF,
#endif
#ifdef HKS_SUPPORT_KDF_PBKDF2
    HKS_ALG_PBKDF2,
#endif
};

static uint32_t g_deriveAlgLocal[] = {
#ifdef HKS_SUPPORT_KDF_HKDF
    HKS_ALG_HKDF,
#endif
};

static uint32_t g_digest[] = {
    HKS_DIGEST_SHA1,
    HKS_DIGEST_SHA224,
    HKS_DIGEST_SHA256,
    HKS_DIGEST_SHA384,
    HKS_DIGEST_SHA512,
};
static uint32_t g_macDigest[] = {
    HKS_DIGEST_SHA1,
    HKS_DIGEST_SHA224,
    HKS_DIGEST_SHA256,
    HKS_DIGEST_SHA384,
    HKS_DIGEST_SHA512,
    HKS_DIGEST_SM3
};
#ifdef HKS_SUPPORT_AES_C
static uint32_t g_aesKeySizeLocal[] = {
    HKS_AES_KEY_SIZE_128,
    HKS_AES_KEY_SIZE_192,
    HKS_AES_KEY_SIZE_256,
};
#endif
#ifdef HKS_SUPPORT_RSA_C
static uint32_t g_rsaKeySizeLocal[] = {
    HKS_RSA_KEY_SIZE_512,
    HKS_RSA_KEY_SIZE_768,
    HKS_RSA_KEY_SIZE_1024,
    HKS_RSA_KEY_SIZE_2048,
    HKS_RSA_KEY_SIZE_3072,
    HKS_RSA_KEY_SIZE_4096,
};
#endif

static uint32_t g_cipherAlgLocal[] = {
#ifdef HKS_SUPPORT_AES_C
    HKS_ALG_AES,
#endif
#ifdef HKS_SUPPORT_RSA_C
    HKS_ALG_RSA,
#endif
};

static int32_t CheckAndGetAlgorithm(
    const struct HksParamSet *paramSet, const uint32_t *expectAlg, uint32_t expectCnt, uint32_t *alg)
{
    struct HksParam *algParam = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_ALGORITHM, &algParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_CHECK_GET_ALG_FAIL,
        "get param 0x%" LOG_PUBLIC "x failed!", HKS_TAG_ALGORITHM)

    ret = HksCheckValue(algParam->uint32Param, expectAlg, expectCnt);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_INVALID_ALGORITHM,
        "alg value %" LOG_PUBLIC "u not expected", algParam->uint32Param)

    *alg = algParam->uint32Param;
    return ret;
}

static int32_t CheckAndGetDigest(
    const struct HksParamSet *paramSet, const uint32_t *expectDigest, uint32_t expectCnt, uint32_t *digest)
{
    struct HksParam *digestParam = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_DIGEST, &digestParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_CHECK_GET_DIGEST_FAIL,
        "get param get 0x%" LOG_PUBLIC "x failed!", HKS_TAG_DIGEST)

    ret = HksCheckValue(digestParam->uint32Param, expectDigest, expectCnt);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_INVALID_DIGEST,
        "digest value %" LOG_PUBLIC "u not expected", digestParam->uint32Param)

    *digest = digestParam->uint32Param;
    return ret;
}

#ifndef _CUT_AUTHENTICATE_
static int32_t CheckGenKeyParamsByAlg(uint32_t alg, const struct HksParamSet *paramSet,
    struct ParamsValues *params)
{
    int32_t ret = HksGetInputParmasByAlg(alg, HKS_CHECK_TYPE_GEN_KEY, paramSet, params);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret,
        "get input params by algorithm failed, ret = %" LOG_PUBLIC "d", ret)

    ret = HksCheckFixedParams(alg, HKS_CHECK_TYPE_GEN_KEY, params);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    ret = HksCheckGenKeyPurpose(alg, params->purpose.value);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret,
        "check purpose not expected, purpose =  %" LOG_PUBLIC "d", params->purpose.value);
    return HksCheckGenKeyMutableParams(alg, params);
}

static int32_t CheckGenKeyMacDeriveParams(
    uint32_t alg, uint32_t inputPurpose, const struct HksParamSet *paramSet, struct ParamsValues *params)
{
    if (alg != HKS_ALG_AES && alg != HKS_ALG_HMAC && alg != HKS_ALG_SM3) {
        HKS_LOG_E("check mac or derive, not aes alg, alg: %" LOG_PUBLIC "u", alg);
        return HKS_ERROR_INVALID_PURPOSE;
    }

    int32_t ret = HksCheckGenKeyPurpose(alg, inputPurpose);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "check purpose invalid, purpose 0x%" LOG_PUBLIC "x", inputPurpose)

    if (inputPurpose == HKS_KEY_PURPOSE_MAC) {
        ret = HksGetInputParmasByAlg(alg, HKS_CHECK_TYPE_GEN_MAC_KEY, paramSet, params);
    } else {
        ret = HksGetInputParmasByAlg(alg, HKS_CHECK_TYPE_GEN_DERIVE_KEY, paramSet, params);
    }
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get input params by algorithm failed, ret = %" LOG_PUBLIC "d", ret)

    if (inputPurpose == HKS_KEY_PURPOSE_MAC) {
        ret = HksCheckFixedParams(alg, HKS_CHECK_TYPE_GEN_MAC_KEY, params);
    } else {
        ret = HksCheckFixedParams(alg, HKS_CHECK_TYPE_GEN_DERIVE_KEY, params);
    }
    HKS_IF_NOT_SUCC_LOGE(ret, "get input params by algorithm failed, ret = %" LOG_PUBLIC "d", ret)

    return ret;
}

static int32_t CoreCheckGenKeyParams(const struct HksParamSet *paramSet, struct ParamsValues *params)
{
    uint32_t alg;
    int32_t ret = HksCheckParamSetTag(paramSet);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    ret = CheckAndGetAlgorithm(paramSet, g_genKeyAlg, HKS_ARRAY_SIZE(g_genKeyAlg), &alg);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "check and get alg failed")

    struct HksParam *purposeParam = NULL;
    ret = HksGetParam(paramSet, HKS_TAG_PURPOSE, &purposeParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_CHECK_GET_PURPOSE_FAIL,
        "get param get 0x%" LOG_PUBLIC "x failed", HKS_TAG_PURPOSE)

    if (((purposeParam->uint32Param & HKS_KEY_PURPOSE_DERIVE) != 0) ||
        ((purposeParam->uint32Param & HKS_KEY_PURPOSE_MAC) != 0)) {
        return CheckGenKeyMacDeriveParams(alg, purposeParam->uint32Param, paramSet, params);
    }

    return CheckGenKeyParamsByAlg(alg, paramSet, params);
}

static int32_t CheckImportKeySize(uint32_t alg, const struct ParamsValues *params, const struct HksBlob *key)
{
    int32_t ret = HKS_SUCCESS;
    switch (alg) {
        case HKS_ALG_ED25519:
        case HKS_ALG_X25519:
        case HKS_ALG_RSA:
        case HKS_ALG_ECC:
        case HKS_ALG_SM2:
        case HKS_ALG_DH: {
            if (key->size < sizeof(struct HksPubKeyInfo)) {
                ret = HKS_ERROR_INVALID_KEY_INFO;
                break;
            }
            struct HksPubKeyInfo *keyMaterial = (struct HksPubKeyInfo *)(key->data);
            if ((keyMaterial->keyAlg != alg) || (keyMaterial->keySize != params->keyLen.value)) {
                ret = HKS_ERROR_INVALID_KEY_INFO;
            }
            break;
        }
#ifdef HKS_SUPPORT_DSA_C
        case HKS_ALG_DSA:
            break;
#endif
        default:
            return HKS_ERROR_INVALID_ALGORITHM;
    }
    return ret;
}

static int32_t CheckAndGetWrappedKeyUnwrapAlgSuite(const struct HksParamSet *paramSet, uint32_t *algSuite)
{
    struct HksParam *algorithmSuite = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_UNWRAP_ALGORITHM_SUITE, &algorithmSuite);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_CHECK_GET_ALG_FAIL, "get unwrap algorithm suite fail")

    ret = HksCheckValue(algorithmSuite->uint32Param, g_unwrapSuite, HKS_ARRAY_SIZE(g_unwrapSuite));
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_INVALID_ALGORITHM,
        "unwrap algorithm suite value %" LOG_PUBLIC "u not expected", algorithmSuite->uint32Param)

    *algSuite = algorithmSuite->uint32Param;
    return HKS_SUCCESS;
}

#ifdef HKS_SUPPORT_API_SIGN_VERIFY
static int32_t CheckSignVerifyParamsByAlg(uint32_t cmdId, uint32_t alg, const struct ParamsValues *inputParams)
{
    int32_t ret = HksCheckFixedParams(alg, HKS_CHECK_TYPE_USE_KEY, inputParams);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "check sign or verify fixed params failed, ret = %" LOG_PUBLIC "d", ret)

    ret = HksCheckSignVerifyMutableParams(cmdId, alg, inputParams);
    HKS_IF_NOT_SUCC_LOGE(ret, "check sign or verify mutable params failed, ret = %" LOG_PUBLIC "d", ret)

    return ret;
}
#endif
#endif /* _CUT_AUTHENTICATE_ */

static int32_t CheckCipherParamsByAlg(
    uint32_t cmdId, uint32_t alg, const struct HksParamSet *paramSet, const struct ParamsValues *inputParams)
{
    int32_t ret = HksCheckFixedParams(alg, HKS_CHECK_TYPE_USE_KEY, inputParams);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "cipher check fixed params failed, ret = %" LOG_PUBLIC "d", ret)

    ret = HksCheckCipherMutableParams(cmdId, alg, inputParams);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "cipher check mutable params failed, ret = %" LOG_PUBLIC "d", ret)

    ret = HksCheckCipherMaterialParams(alg, inputParams, paramSet);
    HKS_IF_NOT_SUCC_LOGE(ret, "cipher check material params failed, ret = %" LOG_PUBLIC "d", ret)

    return ret;
}

#ifndef _CUT_AUTHENTICATE_
#ifdef HKS_SUPPORT_KDF_PBKDF2
static int32_t CheckPbkdf2DeriveKeyParams(const struct HksParamSet *paramSet)
{
    struct HksParam *iterationParam = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_ITERATION, &iterationParam);
    HKS_IF_NOT_SUCC_RETURN(ret, HKS_ERROR_CHECK_GET_ITERATION_FAIL)

    if (iterationParam->uint32Param < HKS_DEFAULT_PBKDF2_ITERATION) {
        return HKS_ERROR_INVALID_ITERATION;
    }

    struct HksParam *saltParam = NULL;
    ret = HksGetParam(paramSet, HKS_TAG_SALT, &saltParam);
    HKS_IF_NOT_SUCC_RETURN(ret, HKS_ERROR_CHECK_GET_SALT_FAIL)

    if ((CheckBlob(&saltParam->blob) != HKS_SUCCESS) || (saltParam->blob.size < HKS_DEFAULT_PBKDF2_SALT_SIZE)) {
        return HKS_ERROR_INVALID_SALT;
    }

    return HKS_SUCCESS;
}
#endif

int32_t HksCoreCheckGenKeyParams(const struct HksBlob *keyAlias, const struct HksParamSet *paramSet,
    const struct HksBlob *keyIn, const struct HksBlob *keyOut)
{
    (void)keyAlias;
    (void)keyIn;
    (void)keyOut;
    struct ParamsValues params;
    (void)memset_s(&params, sizeof(params), 0, sizeof(params));
    return CoreCheckGenKeyParams(paramSet, &params);
}

static int32_t CheckRsaKeyLen(uint32_t alg, uint32_t keyType, const struct ParamsValues *params,
    const struct HksBlob *key)
{
    (void)keyType;
    if (key->size < sizeof(struct HksKeyMaterialRsa)) {
        HKS_LOG_E("invalid import key size: %" LOG_PUBLIC "u", key->size);
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    struct HksKeyMaterialRsa *keyMaterial = (struct HksKeyMaterialRsa *)(key->data);
    if ((keyMaterial->keyAlg != alg) || (keyMaterial->keySize != params->keyLen.value)) {
        HKS_LOG_E("invalid import key material");
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    if ((keyMaterial->nSize > HKS_RSA_KEY_SIZE_4096) || (keyMaterial->nSize == 0) ||
        (keyMaterial->dSize > HKS_RSA_KEY_SIZE_4096) || (keyMaterial->dSize == 0) ||
        (keyMaterial->eSize > HKS_RSA_KEY_SIZE_4096) || (keyMaterial->eSize == 0)) {
        HKS_LOG_E("invalid import key material n/d/e size");
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    uint32_t keySize = sizeof(struct HksKeyMaterialRsa) + keyMaterial->nSize + keyMaterial->dSize + keyMaterial->eSize;
    if (key->size < keySize) {
        HKS_LOG_E("import key size[%" LOG_PUBLIC "u] smaller than keySize[%" LOG_PUBLIC "u]", key->size, keySize);
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    return HKS_SUCCESS;
}

static int32_t CheckEccKeyLen(uint32_t alg, uint32_t keyType, const struct ParamsValues *params,
    const struct HksBlob *key)
{
    if (key->size < sizeof(struct HksKeyMaterialEcc)) {
        HKS_LOG_E("invalid import key size: %" LOG_PUBLIC "u", key->size);
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    struct HksKeyMaterialEcc *keyMaterial = (struct HksKeyMaterialEcc *)(key->data);
    if ((keyMaterial->keyAlg != alg) || (keyMaterial->keySize != params->keyLen.value)) {
        HKS_LOG_E("invalid import key material");
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    if ((keyMaterial->xSize > HKS_ECC_KEY_SIZE_521) || (keyMaterial->ySize > HKS_ECC_KEY_SIZE_521) ||
        (keyMaterial->zSize > HKS_ECC_KEY_SIZE_521)) {
        HKS_LOG_E("invalid import key material x/y/z size, bigger than 521");
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    if (keyMaterial->zSize == 0) {
        HKS_LOG_E("invalid import key material z size: 0");
        return HKS_ERROR_INVALID_KEY_INFO;
    }
    if ((keyType == HKS_KEY_TYPE_KEY_PAIR) && ((keyMaterial->xSize == 0) || (keyMaterial->ySize == 0))) {
        HKS_LOG_E("invalid import key material x/y size: 0");
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    uint32_t keySize = sizeof(struct HksKeyMaterialEcc) + keyMaterial->xSize + keyMaterial->ySize + keyMaterial->zSize;
    if (key->size < keySize) {
        HKS_LOG_E("import key size[%" LOG_PUBLIC "u] smaller than keySize[%" LOG_PUBLIC "u]", key->size, keySize);
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    return HKS_SUCCESS;
}

static int32_t CheckDsaKeyLen(uint32_t alg, uint32_t keyType, const struct ParamsValues *params,
    const struct HksBlob *key)
{
    if (key->size < sizeof(struct HksKeyMaterialDsa)) {
        HKS_LOG_E("invalid import key size: %" LOG_PUBLIC "u", key->size);
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    struct HksKeyMaterialDsa *keyMaterial = (struct HksKeyMaterialDsa *)(key->data);
    if ((keyMaterial->keyAlg != alg) || (keyMaterial->keySize != params->keyLen.value)) {
        HKS_LOG_E("invalid import key material");
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    if ((keyMaterial->xSize > MAX_KEY_SIZE) || (keyMaterial->ySize > MAX_KEY_SIZE) ||
        (keyMaterial->pSize > MAX_KEY_SIZE) || (keyMaterial->qSize > MAX_KEY_SIZE) ||
        (keyMaterial->gSize > MAX_KEY_SIZE)) {
        HKS_LOG_E("invalid import key material x/y/p/q/g size, bigger than 2048");
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    if ((keyMaterial->xSize == 0) ||
        (keyMaterial->pSize == 0) || (keyMaterial->qSize == 0) || (keyMaterial->gSize == 0)) {
        HKS_LOG_E("invalid import key material x/p/q/g size: 0");
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    if ((keyType == HKS_KEY_TYPE_KEY_PAIR) && (keyMaterial->ySize == 0)) {
        HKS_LOG_E("invalid import key material y size: 0");
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    uint32_t keySize = sizeof(struct HksKeyMaterialDsa) + keyMaterial->xSize + keyMaterial->ySize +
        keyMaterial->pSize + keyMaterial->qSize + keyMaterial->gSize;
    if (key->size < keySize) {
        HKS_LOG_E("import key size[%" LOG_PUBLIC "u] smaller than keySize[%" LOG_PUBLIC "u]", key->size, keySize);
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    return HKS_SUCCESS;
}

static int32_t CheckCurve25519KeyLen(uint32_t alg, uint32_t keyType, const struct ParamsValues *params,
    const struct HksBlob *key)
{
    if (key->size < sizeof(struct HksKeyMaterial25519)) {
        HKS_LOG_E("invalid import key size: %" LOG_PUBLIC "u", key->size);
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    struct HksKeyMaterial25519 *keyMaterial = (struct HksKeyMaterial25519 *)(key->data);
    if ((keyMaterial->keyAlg != alg) || (keyMaterial->keySize != params->keyLen.value)) {
        HKS_LOG_E("invalid import key material");
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    if ((keyMaterial->pubKeySize > HKS_CURVE25519_KEY_SIZE_256) ||
        (keyMaterial->priKeySize > HKS_CURVE25519_KEY_SIZE_256)) {
        HKS_LOG_E("invalid import key material pubKey/priKey size, bigger than 256");
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    if (keyMaterial->priKeySize == 0) {
        HKS_LOG_E("invalid import key material priKey size: 0");
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    if ((keyType == HKS_KEY_TYPE_KEY_PAIR) && (keyMaterial->pubKeySize == 0)) {
        HKS_LOG_E("invalid import key material pubKey size: 0");
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    uint32_t keySize = sizeof(struct HksKeyMaterial25519) + keyMaterial->pubKeySize + keyMaterial->priKeySize;
    if (key->size < keySize) {
        HKS_LOG_E("import key size[%" LOG_PUBLIC "u] smaller than keySize[%" LOG_PUBLIC "u]", key->size, keySize);
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    return HKS_SUCCESS;
}

static int32_t CheckDHKeyLen(uint32_t alg, uint32_t keyType, const struct ParamsValues *params,
    const struct HksBlob *key)
{
    if (key->size < sizeof(struct HksKeyMaterialDh)) {
        HKS_LOG_E("invalid import key size: %" LOG_PUBLIC "u", key->size);
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    struct HksKeyMaterialDh *keyMaterial = (struct HksKeyMaterialDh *)(key->data);
    if ((keyMaterial->keyAlg != alg) || (keyMaterial->keySize != params->keyLen.value)) {
        HKS_LOG_E("invalid import key material");
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    if ((keyMaterial->pubKeySize > HKS_DH_KEY_SIZE_4096) || (keyMaterial->priKeySize > HKS_DH_KEY_SIZE_4096)) {
        HKS_LOG_E("invalid import key material pubKey/priKey size, bigger than 4096");
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    if (keyMaterial->priKeySize == 0) {
        HKS_LOG_E("invalid import key material priKey size: 0");
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    if ((keyType == HKS_KEY_TYPE_KEY_PAIR) && (keyMaterial->pubKeySize == 0)) {
        HKS_LOG_E("invalid import key material pubKey size: 0");
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    uint32_t keySize = sizeof(struct HksKeyMaterialDh) + keyMaterial->pubKeySize + keyMaterial->priKeySize;
    if (key->size < keySize) {
        HKS_LOG_E("import key size[%" LOG_PUBLIC "u] smaller than keySize[%" LOG_PUBLIC "u]", key->size, keySize);
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    return HKS_SUCCESS;
}

static int32_t CheckKeyLen(uint32_t alg, uint32_t keyType, const struct ParamsValues *params,
    const struct HksBlob *key)
{
    switch (alg) {
        case HKS_ALG_RSA:
            return CheckRsaKeyLen(alg, keyType, params, key);
        case HKS_ALG_ECC:
        case HKS_ALG_SM2:
            return CheckEccKeyLen(alg, keyType, params, key);
        case HKS_ALG_DSA:
            return CheckDsaKeyLen(alg, keyType, params, key);
        case HKS_ALG_X25519:
        case HKS_ALG_ED25519:
            return CheckCurve25519KeyLen(alg, keyType, params, key);
        case HKS_ALG_DH:
            return CheckDHKeyLen(alg, keyType, params, key);
        default:
            return HKS_ERROR_INVALID_ALGORITHM;
    }
}

static int32_t CheckMutableParams(uint32_t alg, uint32_t keyType, const struct ParamsValues *params)
{
    if (keyType == HKS_KEY_TYPE_KEY_PAIR) {
        return HKS_SUCCESS;
    }

    switch (alg) {
        case HKS_ALG_RSA:
            if ((params->purpose.value != HKS_KEY_PURPOSE_SIGN) &&
                (params->purpose.value != HKS_KEY_PURPOSE_DECRYPT)) {
                HKS_LOG_E("Import rsa private key check purpose failed.");
                return HKS_ERROR_INVALID_PURPOSE;
            }
            return HKS_SUCCESS;
        case HKS_ALG_ECC:
            if ((params->purpose.value != HKS_KEY_PURPOSE_SIGN) && (params->purpose.value != HKS_KEY_PURPOSE_AGREE) &&
                (params->purpose.value != HKS_KEY_PURPOSE_UNWRAP)) {
                HKS_LOG_E("Import ecc private key check purpose failed.");
                return HKS_ERROR_INVALID_PURPOSE;
            }
            return HKS_SUCCESS;
        case HKS_ALG_SM2:
        case HKS_ALG_DSA:
        case HKS_ALG_ED25519:
            if (params->purpose.value != HKS_KEY_PURPOSE_SIGN) {
                HKS_LOG_E("Import sm2 or dsa or ed25519 private key check purpose failed.");
                return HKS_ERROR_INVALID_PURPOSE;
            }
            return HKS_SUCCESS;
        case HKS_ALG_X25519:
        case HKS_ALG_DH:
            return HKS_SUCCESS;
        default:
            return HKS_ERROR_INVALID_ALGORITHM;
    }
}

static int32_t CheckImportKey(uint32_t alg, uint32_t keyType, const struct ParamsValues *params,
    const struct HksBlob *key)
{
    int32_t ret = CheckKeyLen(alg, keyType, params, key);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "check key len failed, ret = %" LOG_PUBLIC "d", ret)

    ret = CheckMutableParams(alg, keyType, params);
    HKS_IF_NOT_SUCC_LOGE(ret, "check mutable params faile, ret = %" LOG_PUBLIC "d", ret)
    return ret;
}

static int32_t CheckImportSymmetricKeySize(const struct ParamsValues *params, const struct HksBlob *key)
{
    if (key->size != HKS_KEY_BYTES(params->keyLen.value)) {
        return HKS_ERROR_INVALID_KEY_INFO;
    }
    return HKS_SUCCESS;
}

int32_t HksCoreCheckImportKeyParams(const struct HksBlob *keyAlias, const struct HksBlob *key,
    const struct HksParamSet *paramSet, const struct HksBlob *keyOut)
{
    (void)keyAlias;
    (void)keyOut;
    /* import key paramset is subset of generate key paramset */
    struct ParamsValues params;
    (void)memset_s(&params, sizeof(params), 0, sizeof(params));
    int32_t ret = CoreCheckGenKeyParams(paramSet, &params);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "CheckImportKeyParams failed")

    uint32_t alg;
    ret = CheckAndGetAlgorithm(paramSet, g_importKeyAlg, HKS_ARRAY_SIZE(g_importKeyAlg), &alg);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "import key check and get alg failed")

    if ((alg == HKS_ALG_AES) || (alg == HKS_ALG_SM3) || (alg == HKS_ALG_SM4) || (alg == HKS_ALG_HMAC)) {
        return CheckImportSymmetricKeySize(&params, key);
    }

    struct HksParam *importKeyTypeParam = NULL;
    ret = HksGetParam(paramSet, HKS_TAG_IMPORT_KEY_TYPE, &importKeyTypeParam);
    if ((ret == HKS_SUCCESS) &&
        ((importKeyTypeParam->uint32Param == HKS_KEY_TYPE_PRIVATE_KEY) ||
        (importKeyTypeParam->uint32Param == HKS_KEY_TYPE_KEY_PAIR))) {
        /* check private key or keypair key params */
        return CheckImportKey(alg, importKeyTypeParam->uint32Param, &params, key);
    }

    /* check public key params: 1. check keySize */
    ret = CheckImportKeySize(alg, &params, key);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "import key check key size invalid")

    /* check public key params: 2. check mutable params */
    return CheckImportMutableParams(alg, &params);
}

int32_t HksCoreCheckImportWrappedKeyParams(const struct HksBlob *key, const struct HksBlob *wrappedKeyData,
    const struct HksParamSet *paramSet, struct HksBlob *keyOut, uint32_t *outUnwrapSuite)
{
    (void)key;
    (void)keyOut;

    /* first check wrapping-related params and wrapped key data */
    uint32_t unwrapSuite = 0;
    int32_t ret = CheckAndGetWrappedKeyUnwrapAlgSuite(paramSet, &unwrapSuite);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "check import wrapped params set failed")

    ret = HksCheckWrappedDataFormatValidity(wrappedKeyData, HKS_IMPORT_WRAPPED_KEY_TOTAL_BLOBS, NULL);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "check import wrapped key data format failed")

    /* then check the origin key paramset which is the same as import key */
    struct ParamsValues params;
    (void)memset_s(&params, sizeof(params), 0, sizeof(params));
    ret = CoreCheckGenKeyParams(paramSet, &params);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "check origin key param set failed")

    uint32_t alg;
    ret = CheckAndGetAlgorithm(paramSet, g_importKeyAlg, HKS_ARRAY_SIZE(g_importKeyAlg), &alg);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "CheckImportKeyParams get alg failed")
    *outUnwrapSuite = unwrapSuite;
    return HKS_SUCCESS;
}

int32_t HksCoreCheckSignVerifyParams(uint32_t cmdId, const struct HksBlob *key, const struct HksParamSet *paramSet,
    const struct HksBlob *srcData, const struct HksBlob *signature)
{
#ifdef HKS_SUPPORT_API_SIGN_VERIFY
    (void)srcData;
    uint32_t alg;
    int32_t ret = CheckAndGetAlgorithm(paramSet, g_signAlg, HKS_ARRAY_SIZE(g_signAlg), &alg);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "check and get alg failed")

    struct ParamsValues params;
    (void)memset_s(&params, sizeof(params), 0, sizeof(params));

    ret = HksGetInputParmasByAlg(alg, HKS_CHECK_TYPE_USE_KEY, paramSet, &params);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "sign or verify get input params failed, ret = %" LOG_PUBLIC "d", ret)

    ret = CheckSignVerifyParamsByAlg(cmdId, alg, &params);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "sign or verify check params failed, ret = %" LOG_PUBLIC "d", ret)

    uint32_t keySize = 0;
    ret = HksGetKeySize(alg, key, &keySize);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get keySize failed!")

    ret = HksCheckSignature(cmdId, alg, keySize, signature);
    HKS_IF_NOT_SUCC_LOGE(ret, "check signature failed, ret = %" LOG_PUBLIC "d", ret)

    return ret;
#else
    (void)cmdId;
    (void)key;
    (void)paramSet;
    (void)srcData;
    (void)signature;
    return HKS_ERROR_NOT_SUPPORTED;
#endif
}

int32_t HksLocalCheckSignVerifyParams(uint32_t cmdId, uint32_t keySize, const struct HksParamSet *paramSet,
    const struct HksBlob *srcData, const struct HksBlob *signature)
{
#ifdef HKS_SUPPORT_API_SIGN_VERIFY
    (void)srcData;
    uint32_t alg;
    int32_t ret = CheckAndGetAlgorithm(paramSet, g_signAlg, HKS_ARRAY_SIZE(g_signAlg), &alg);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "check and get alg failed")

    struct ParamsValues params;
    (void)memset_s(&params, sizeof(params), 0, sizeof(params));

    ret = HksGetInputParmasByAlg(alg, HKS_CHECK_TYPE_USE_KEY, paramSet, &params);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "sign or verify get input params failed, ret = %" LOG_PUBLIC "d", ret)

    ret = CheckSignVerifyParamsByAlg(cmdId, alg, &params);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "sign or verify check params failed, ret = %" LOG_PUBLIC "d", ret)

    ret = HksCheckSignature(cmdId, alg, keySize, signature);
    HKS_IF_NOT_SUCC_LOGE(ret, "check signature failed, ret = %" LOG_PUBLIC "d", ret)

    return ret;
#else
    (void)cmdId;
    (void)keySize;
    (void)paramSet;
    (void)srcData;
    (void)signature;
    return HKS_ERROR_NOT_SUPPORTED;
#endif
}

int32_t HksCoreCheckAgreeKeyParams(const struct HksParamSet *paramSet, const struct HksBlob *privateKey,
    const struct HksBlob *peerPublicKey, const struct HksBlob *agreedKey, bool isLocalCheck)
{
    uint32_t alg;
    int32_t ret;

    if (isLocalCheck) {
        ret = CheckAndGetAlgorithm(paramSet, g_agreeAlgLocal, HKS_ARRAY_SIZE(g_agreeAlgLocal), &alg);
    } else {
        ret = CheckAndGetAlgorithm(paramSet, g_agreeAlg, HKS_ARRAY_SIZE(g_agreeAlg), &alg);
    }
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "check alg failed")

    uint32_t keySize = 0;
    if (isLocalCheck) {
        if (alg == HKS_ALG_ED25519) {
            if ((privateKey->size != HKS_KEY_BYTES(HKS_CURVE25519_KEY_SIZE_256)) ||
                (peerPublicKey->size != HKS_KEY_BYTES(HKS_CURVE25519_KEY_SIZE_256))) {
                return HKS_ERROR_INVALID_KEY_SIZE;
            }
        }

        if (alg == HKS_ALG_DH || alg == HKS_ALG_ECC || alg == HKS_ALG_ECDH) {
            if (privateKey->size < sizeof(struct HksKeyMaterialHeader)) {
                return HKS_ERROR_INVALID_ARGUMENT;
            }
            keySize = ((struct HksKeyMaterialHeader *)privateKey->data)->keySize;
        } else if (alg == HKS_ALG_ED25519) {
            keySize = privateKey->size * HKS_BITS_PER_BYTE;
        }
    } else {
        ret = HksGetKeySize(alg, privateKey, &keySize);
        HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get key size failed")
    }

    uint32_t size = keySize / HKS_BITS_PER_BYTE + keySize % HKS_BITS_PER_BYTE;
    if (agreedKey->size < size) {
        HKS_LOG_E("agreeKey buffer too small, size %" LOG_PUBLIC "u", agreedKey->size);
        return HKS_ERROR_BUFFER_TOO_SMALL;
    }

    return HKS_SUCCESS;
}

int32_t HksCoreCheckCipherParams(uint32_t cmdId, const struct HksBlob *key, const struct HksParamSet *paramSet,
    const struct HksBlob *inData, const struct HksBlob *outData)
{
    uint32_t alg;
    int32_t ret = CheckAndGetAlgorithm(paramSet, g_cipherAlg, HKS_ARRAY_SIZE(g_cipherAlg), &alg);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "check alg failed")

    struct ParamsValues params;
    (void)memset_s(&params, sizeof(params), 0, sizeof(params));

    ret = HksGetInputParmasByAlg(alg, HKS_CHECK_TYPE_USE_KEY, paramSet, &params);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "cipher get input params failed, ret = %" LOG_PUBLIC "d", ret)

    if ((alg == HKS_ALG_RSA) || (alg == HKS_ALG_SM4)) {
        ret = HksGetKeySize(alg, key, &params.keyLen.value);
        HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "rsa cipher get key size failed")
    }

    ret = CheckCipherParamsByAlg(cmdId, alg, paramSet, &params);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "cipher check params failed, ret = %" LOG_PUBLIC "d", ret)

    ret = HksCheckCihperData(cmdId, alg, &params, inData, outData);
    HKS_IF_NOT_SUCC_LOGE(ret, "cipher check input or output data failed, ret = %" LOG_PUBLIC "d", ret)

    return ret;
}
#endif /* _CUT_AUTHENTICATE_ */

int32_t HksLocalCheckCipherParams(uint32_t cmdId, uint32_t keySize, const struct HksParamSet *paramSet,
    const struct HksBlob *inData, const struct HksBlob *outData)
{
    uint32_t alg;
    int32_t ret = CheckAndGetAlgorithm(paramSet, g_cipherAlgLocal, HKS_ARRAY_SIZE(g_cipherAlgLocal), &alg);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "check alg failed")

    if (alg == HKS_ALG_AES) {
#ifdef HKS_SUPPORT_AES_C
        ret = HksCheckValue(keySize, g_aesKeySizeLocal, HKS_ARRAY_SIZE(g_aesKeySizeLocal));
#else
        ret = HKS_ERROR_NOT_SUPPORTED;
#endif
    } else if (alg == HKS_ALG_RSA) {
#ifdef HKS_SUPPORT_RSA_C
        ret = HksCheckValue(keySize, g_rsaKeySizeLocal, HKS_ARRAY_SIZE(g_rsaKeySizeLocal));
#else
        ret = HKS_ERROR_NOT_SUPPORTED;
#endif
    }
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_INVALID_KEY_SIZE,
        "keySize value %" LOG_PUBLIC "u not expected", keySize)

    struct ParamsValues params;
    (void)memset_s(&params, sizeof(params), 0, sizeof(params));

    ret = HksGetInputParmasByAlg(alg, HKS_CHECK_TYPE_USE_KEY, paramSet, &params);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "local cipher get input params failed, ret = %" LOG_PUBLIC "d", ret)

    ret = CheckCipherParamsByAlg(cmdId, alg, paramSet, &params);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "local cipher check params failed, ret = %" LOG_PUBLIC "d", ret)

    ret = HksCheckCihperData(cmdId, alg, &params, inData, outData);
    HKS_IF_NOT_SUCC_LOGE(ret, "local cipher check input or output data failed, ret = %" LOG_PUBLIC "d", ret)

    return ret;
}

int32_t HksCoreCheckDeriveKeyParams(const struct HksParamSet *paramSet, const struct HksBlob *mainKey,
    const struct HksBlob *derivedKey, bool isLocalCheck)
{
    (void)mainKey;
    (void)derivedKey;
    uint32_t alg;
    int32_t ret;
    if (isLocalCheck) {
        ret = CheckAndGetAlgorithm(paramSet, g_deriveAlgLocal, HKS_ARRAY_SIZE(g_deriveAlgLocal), &alg);
    } else {
        ret = CheckAndGetAlgorithm(paramSet, g_deriveAlg, HKS_ARRAY_SIZE(g_deriveAlg), &alg);
    }
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "check alg failed")

    struct HksParam *purposeParam = NULL;
    ret = HksGetParam(paramSet, HKS_TAG_PURPOSE, &purposeParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_CHECK_GET_PURPOSE_FAIL,
        "get param get 0x%" LOG_PUBLIC "x failed", HKS_TAG_PURPOSE)

    if (purposeParam->uint32Param != HKS_KEY_PURPOSE_DERIVE) {
        return HKS_ERROR_INVALID_PURPOSE;
    }

    /* according to RFC5869, HKDF no need check salt and info */
    uint32_t digest;
    ret = CheckAndGetDigest(paramSet, g_digest, HKS_ARRAY_SIZE(g_digest), &digest);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "check digest failed")

#ifdef HKS_SUPPORT_KDF_PBKDF2
    if (alg == HKS_ALG_PBKDF2) {
        return CheckPbkdf2DeriveKeyParams(paramSet);
    }
#endif

    return HKS_SUCCESS;
}

static int32_t CheckMacPurpose(const struct HksParamSet *paramSet)
{
    struct HksParam *purposeParam = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_PURPOSE, &purposeParam);
    HKS_IF_NOT_SUCC_RETURN(ret, HKS_ERROR_CHECK_GET_PURPOSE_FAIL)

    if (purposeParam->uint32Param != HKS_KEY_PURPOSE_MAC) {
        return HKS_ERROR_INVALID_PURPOSE;
    }

    return HKS_SUCCESS;
}

static int32_t CheckMacOutput(
    const struct HksBlob *key, const struct HksParamSet *paramSet, const struct HksBlob *mac, bool isLocalCheck)
{
    uint32_t digest;
    int32_t ret;
    if (isLocalCheck) {
        ret = CheckAndGetDigest(paramSet, g_digest, HKS_ARRAY_SIZE(g_digest), &digest);
    } else {
        ret = CheckAndGetDigest(paramSet, g_macDigest, HKS_ARRAY_SIZE(g_macDigest), &digest);
    }
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "check digest failed")

    uint32_t digestLen;
    ret = HksGetDigestLen(digest, &digestLen);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get digest length failed, ret = %" LOG_PUBLIC "d", ret)

    if (mac->size < digestLen) {
        HKS_LOG_E("mac buffer too small, size %" LOG_PUBLIC "u", mac->size);
        return HKS_ERROR_BUFFER_TOO_SMALL;
    }

    if ((isLocalCheck) && (key->size < digestLen)) { /* the unit of local engine input key size is byte */
        HKS_LOG_E("key size too small, size = %" LOG_PUBLIC "u", key->size);
        return HKS_ERROR_INVALID_KEY_SIZE;
    }

    return HKS_SUCCESS;
}

int32_t HksCoreCheckMacParams(const struct HksBlob *key, const struct HksParamSet *paramSet,
    const struct HksBlob *srcData, const struct HksBlob *mac, bool isLocalCheck)
{
    (void)srcData;
    int32_t ret = CheckMacPurpose(paramSet);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "check Mac purpose failed")

    return CheckMacOutput(key, paramSet, mac, isLocalCheck);
}
