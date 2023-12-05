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

#include "hks_core_service_three_stage.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "hks_auth.h"
#include "hks_base_check.h"
#include "hks_client_service_adapter_common.h"
#include "hks_cmd_id.h"
#include "hks_common_check.h"
#include "hks_core_service.h"
#include "hks_crypto_adapter.h"
#include "hks_crypto_hal.h"
#include "hks_keyblob.h"
#include "hks_log.h"
#include "hks_mem.h"
#include "hks_param.h"
#include "hks_template.h"
#include "securec.h"

#ifndef _HARDWARE_ROOT_KEY_
#include "hks_rkc.h"
#endif

#ifdef HKS_SUPPORT_UPGRADE_STORAGE_DATA
#include "hks_upgrade_key_info.h"
#endif

#ifndef _CUT_AUTHENTICATE_
#define CURVE25519_KEY_BYTE_SIZE HKS_KEY_BYTES(HKS_CURVE25519_KEY_SIZE_256)

#ifdef HKS_SUPPORT_ED25519_TO_X25519

#endif
#define HKS_RSA_OAEP_DIGEST_NUM     2
#define HKS_BLOCK_CIPHER_CBC_BLOCK_SIZE 16
#define HKS_TEMP_SIZE               32
#define MAX_BUF_SIZE                (5 * 1024 * 1024)

static int32_t CheckRsaCipherData(bool isEncrypt, uint32_t keyLen, struct HksUsageSpec *usageSpec,
    const struct HksBlob *outData)
{
    uint32_t keySize = keyLen / HKS_BITS_PER_BYTE;
    uint32_t padding = usageSpec->padding;
    uint32_t digest = usageSpec->digest;

    if (padding == HKS_PADDING_NONE) {
        if (outData->size < keySize) {
            HKS_LOG_E("outData buffer too small size: %" LOG_PUBLIC "u, keySize: %" LOG_PUBLIC "u",
                outData->size, keySize);
            return HKS_ERROR_BUFFER_TOO_SMALL;
        }
    } else if (padding == HKS_PADDING_OAEP) {
        uint32_t digestLen;
        int32_t ret = HksGetDigestLen(digest, &digestLen);
        HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "GetDigestLen failed, ret = %" LOG_PUBLIC "x", ret)

        if (keySize <= (HKS_RSA_OAEP_DIGEST_NUM * digestLen + HKS_RSA_OAEP_DIGEST_NUM)) {
            return HKS_ERROR_INVALID_KEY_FILE;
        }
        uint32_t size = keySize - HKS_RSA_OAEP_DIGEST_NUM * digestLen - HKS_RSA_OAEP_DIGEST_NUM;
        if (isEncrypt) {
            if (outData->size < keySize) {
                HKS_LOG_E("encrypt, outData buffer too small size: %" LOG_PUBLIC "u, keySize: %" LOG_PUBLIC "u",
                    outData->size, keySize);
                return HKS_ERROR_BUFFER_TOO_SMALL;
            }
        } else {
            if (outData->size < size) {
                HKS_LOG_E("decrypt, outData buffer too small size: %" LOG_PUBLIC "u, keySize: %" LOG_PUBLIC "u",
                    outData->size, keySize);
                return HKS_ERROR_BUFFER_TOO_SMALL;
            }
        }
    }

    return HKS_SUCCESS;
}

static int32_t CheckAesCipherAead(bool isEncrypt, const struct HksBlob *inData,
    const struct HksBlob *outData)
{
    if (isEncrypt) {
        if (outData->size < (inData->size + HKS_AE_TAG_LEN)) {
            HKS_LOG_E("encrypt, out buffer too small size: %" LOG_PUBLIC "u, inSize: %" LOG_PUBLIC "u",
                outData->size, inData->size);
            return HKS_ERROR_BUFFER_TOO_SMALL;
        }
    } else {
        if (outData->size < inData->size) {
            HKS_LOG_E("decryptfinal, out buffer too small size: %" LOG_PUBLIC "u, inSize: %" LOG_PUBLIC "u",
                outData->size, inData->size);
            return HKS_ERROR_BUFFER_TOO_SMALL;
        }
    }

    return HKS_SUCCESS;
}

static int32_t CheckBlockCipherOther(bool isEncrypt, uint32_t padding, const struct HksBlob *inData,
    const struct HksBlob *outData)
{
    uint32_t paddingSize = 0;

    if (isEncrypt) {
        if (padding == HKS_PADDING_NONE) {
            if (inData->size % HKS_BLOCK_CIPHER_CBC_BLOCK_SIZE != 0) {
                HKS_LOG_E("encrypt cbc no-padding, invalid inSize: %" LOG_PUBLIC "u", inData->size);
                return HKS_ERROR_INVALID_ARGUMENT;
            }
        } else {
            paddingSize = HKS_BLOCK_CIPHER_CBC_BLOCK_SIZE - inData->size % HKS_BLOCK_CIPHER_CBC_BLOCK_SIZE;
            if (inData->size > (UINT32_MAX - paddingSize)) {
                HKS_LOG_E("encrypt, invalid inData size: %" LOG_PUBLIC "u", inData->size);
                return HKS_ERROR_INVALID_ARGUMENT;
            }
        }
        if (outData->size < (inData->size + paddingSize)) {
            HKS_LOG_E("encrypt, outData buffer too small size: %" LOG_PUBLIC "u, need: %" LOG_PUBLIC "u",
                outData->size, inData->size + paddingSize);
            return HKS_ERROR_BUFFER_TOO_SMALL;
        }
    } else {
        if (outData->size < inData->size) {
            HKS_LOG_E("decrypt, outData buffer too small size: %" LOG_PUBLIC "u, inDataSize: %" LOG_PUBLIC "u",
                outData->size, inData->size);
            return HKS_ERROR_BUFFER_TOO_SMALL;
        }
    }

    return HKS_SUCCESS;
}

static int32_t CheckBlockCipherData(bool isEncrypt, const struct HksUsageSpec *usageSpec,
    const struct HksBlob *inData, const struct HksBlob *outData)
{
    const uint32_t padding = usageSpec->padding;
    const uint32_t mode = usageSpec->mode;
    const uint32_t alg = usageSpec->algType;
    int32_t ret = HKS_ERROR_NOT_SUPPORTED;
    if (((alg == HKS_ALG_AES) || (alg == HKS_ALG_SM4)) &&
        ((mode == HKS_MODE_CBC) || (mode == HKS_MODE_CTR) || (mode == HKS_MODE_ECB))) {
        ret = CheckBlockCipherOther(isEncrypt, padding, inData, outData);
    } else if ((alg == HKS_ALG_AES) && ((mode == HKS_MODE_GCM) || (mode == HKS_MODE_CCM))) {
        ret = CheckAesCipherAead(isEncrypt, inData, outData);
    }

    return ret;
}

static int32_t HksCheckFinishOutSize(bool isEncrypt, struct HksParamSet *paramSet,
    const struct HksBlob *inData, const struct HksBlob *outData)
{
    struct HksUsageSpec usageSpec = {0};
    HksFillUsageSpec(paramSet, &usageSpec);
    struct HksKeySpec cihperSpec = {0};
    HksFillKeySpec(paramSet, &cihperSpec);
    uint32_t alg = usageSpec.algType;

    switch (alg) {
        case HKS_ALG_RSA:
            return CheckRsaCipherData(isEncrypt, cihperSpec.keyLen, &usageSpec, outData);
        case HKS_ALG_AES:
            return CheckBlockCipherData(isEncrypt, &usageSpec, inData, outData);
        case HKS_ALG_SM4:
            return CheckBlockCipherData(isEncrypt, &usageSpec, inData, outData);
        default:
            return HKS_ERROR_INVALID_ALGORITHM;
    }
}

static int32_t SignVerifyAuth(const struct HuksKeyNode *keyNode, const struct HksParamSet *paramSet)
{
    struct HksParam *algParam = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_ALGORITHM, &algParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "append sign/verify get alg param failed!")

    if (algParam->uint32Param == HKS_ALG_ECC || algParam->uint32Param == HKS_ALG_SM2 ||
        algParam->uint32Param == HKS_ALG_DSA) {
        return HksThreeStageAuth(HKS_AUTH_ID_SIGN_VERIFY_ECC, keyNode);
    } else if (algParam->uint32Param == HKS_ALG_RSA) {
        return HksThreeStageAuth(HKS_AUTH_ID_SIGN_VERIFY_RSA, keyNode);
    } else if (algParam->uint32Param == HKS_ALG_ED25519) {
        return HksThreeStageAuth(HKS_AUTH_ID_SIGN_VERIFY_ED25519, keyNode);
    }
    return HKS_ERROR_INVALID_ALGORITHM;
}

static int32_t AgreeAuth(const struct HuksKeyNode *keyNode, const struct HksParamSet *paramSet)
{
    struct HksParam *algParam = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_ALGORITHM, &algParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "append agree get alg param failed!")

    if (algParam->uint32Param == HKS_ALG_ECDH || algParam->uint32Param == HKS_ALG_X25519 ||
        algParam->uint32Param == HKS_ALG_DH) {
        return HksThreeStageAuth(HKS_AUTH_ID_AGREE, keyNode);
    }
    return HKS_ERROR_INVALID_ALGORITHM;
}

static int32_t HmacAuth(const struct HuksKeyNode *keyNode, const struct HksParamSet *paramSet)
{
    struct HksParam *algParam = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_ALGORITHM, &algParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "append hmac get alg param failed!")

    if (algParam->uint32Param == HKS_ALG_HMAC) {
        return HksThreeStageAuth(HKS_AUTH_ID_MAC_HMAC, keyNode);
    } else if (algParam->uint32Param == HKS_ALG_SM3) {
        return HksThreeStageAuth(HKS_AUTH_ID_MAC_SM3, keyNode);
    }
    return HKS_ERROR_INVALID_ALGORITHM;
}

static int32_t CipherAuth(const struct HuksKeyNode *keyNode, const struct HksParamSet *paramSet)
{
    struct HksParam *algParam = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_ALGORITHM, &algParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "append cipher get alg param failed!")

    if (algParam->uint32Param == HKS_ALG_AES) {
        return HksThreeStageAuth(HKS_AUTH_ID_SYM_CIPHER, keyNode);
    } else if (algParam->uint32Param == HKS_ALG_RSA) {
        return HksThreeStageAuth(HKS_AUTH_ID_ASYM_CIPHER, keyNode);
    } else if (algParam->uint32Param == HKS_ALG_SM4) {
        return HksThreeStageAuth(HKS_AUTH_ID_SYM_CIPHER, keyNode);
    }
    return HKS_ERROR_INVALID_ALGORITHM;
}

static void *GetCryptoCtx(const struct HuksKeyNode *keyNode)
{
    struct HksParam *ctxParam = NULL;
    int32_t ret = HksGetParam(keyNode->runtimeParamSet, HKS_TAG_CRYPTO_CTX, &ctxParam);

    return (ret != HKS_SUCCESS || ctxParam == NULL) ? NULL : (void *)(uintptr_t)ctxParam->uint64Param;
}

static void ClearCryptoCtx(const struct HuksKeyNode *keyNode)
{
    struct HksParam *ctxParam = NULL;
    int32_t ret = HksGetParam(keyNode->runtimeParamSet, HKS_TAG_CRYPTO_CTX, &ctxParam);
    if (ret != HKS_SUCCESS || ctxParam == NULL) {
        return;
    }

    ctxParam->uint64Param = (uint64_t)(uintptr_t)NULL;
    return;
}

static int32_t SetCacheModeCtx(const struct HuksKeyNode *keyNode)
{
    struct HksParam *ctxParam = NULL;
    int32_t ret = HksGetParam(keyNode->runtimeParamSet, HKS_TAG_CRYPTO_CTX, &ctxParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_BAD_STATE, "get ctx from keyNode failed!")

    struct HksBlob *tempData = (struct HksBlob *)HksMalloc(sizeof(struct HksBlob));
    HKS_IF_NULL_LOGE_RETURN(tempData, HKS_ERROR_MALLOC_FAIL, "get cache mode ctx malloc fail.")

    tempData->size = 0;
    tempData->data = NULL;
    ctxParam->uint64Param = (uint64_t)(uintptr_t)tempData;
    return HKS_SUCCESS;
}

static int32_t CopyNewCachedData(const struct HksBlob *cachedBlob, const struct HksBlob *inData,
    uint8_t *newData, uint32_t newSize)
{
    if (cachedBlob->size != 0) {
        if (memcpy_s(newData, newSize, cachedBlob->data, cachedBlob->size) != EOK) {
            HKS_LOG_E("memcpy cached data failed");
            return HKS_ERROR_INSUFFICIENT_MEMORY;
        }
    }
    if (inData->size != 0) {
        if (memcpy_s(newData + cachedBlob->size, newSize - cachedBlob->size, inData->data, inData->size) != EOK) {
            HKS_LOG_E("memcpy in data failed");
            return HKS_ERROR_INSUFFICIENT_MEMORY;
        }
    }
    return HKS_SUCCESS;
}

static int32_t GetNewCachedData(const struct HksBlob *cachedBlob, const struct HksBlob *inData,
    struct HksBlob *newBlob)
{
    if ((cachedBlob->size > MAX_BUF_SIZE) || (inData->size > (MAX_BUF_SIZE - cachedBlob->size))) {
        HKS_LOG_E("input data size too large, size = %" LOG_PUBLIC "u", inData->size);
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    uint32_t newSize = cachedBlob->size + inData->size;
    uint8_t *newData = (uint8_t *)HksMalloc(newSize);
    HKS_IF_NULL_LOGE_RETURN(newData, HKS_ERROR_MALLOC_FAIL, "update cache data malloc fail.")

    int32_t ret = CopyNewCachedData(cachedBlob, inData, newData, newSize);
    if (ret != HKS_SUCCESS) {
        HKS_FREE_PTR(newData);
        return ret;
    }

    newBlob->data = newData;
    newBlob->size = newSize;
    return HKS_SUCCESS;
}

static int32_t UpdateCachedData(const struct HuksKeyNode *keyNode, const struct HksBlob *srcData)
{
    struct HksParam *ctxParam = NULL;
    int32_t ret = HksGetParam(keyNode->runtimeParamSet, HKS_TAG_CRYPTO_CTX, &ctxParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_BAD_STATE, "get ctx from keyNode failed!")

    void *ctx = (void *)(uintptr_t)ctxParam->uint64Param;
    HKS_IF_NULL_LOGE_RETURN(ctx, HKS_ERROR_BAD_STATE, "ctx is invalid: null!")

    struct HksBlob *cachedData = (struct HksBlob *)ctx;
    struct HksBlob *newCachedBlob = (struct HksBlob *)HksMalloc(sizeof(struct HksBlob));
    HKS_IF_NULL_LOGE_RETURN(newCachedBlob, HKS_ERROR_MALLOC_FAIL, "malloc new blob failed")

    ret = GetNewCachedData(cachedData, srcData, newCachedBlob);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("get mew cached data failed, ret = %" LOG_PUBLIC "d", ret);
        HKS_FREE_PTR(newCachedBlob);
        return ret;
    }

    HKS_FREE_PTR(cachedData->data);
    HKS_FREE_PTR(cachedData);
    ctxParam->uint64Param = (uint64_t)(uintptr_t)newCachedBlob;
    return HKS_SUCCESS;
}

static void FreeCachedData(struct HksBlob **cachedData)
{
    if ((cachedData == NULL) || (*cachedData == NULL)) {
        return;
    }
    if ((*cachedData)->data != NULL) {
        (void)memset_s((*cachedData)->data, (*cachedData)->size, 0, (*cachedData)->size);
        HKS_FREE_PTR((*cachedData)->data);
    }
    HKS_FREE_PTR(*cachedData);
}

static int32_t FinishCachedData(const struct HuksKeyNode *keyNode, const struct HksBlob *srcData,
    struct HksBlob *outData)
{
    struct HksParam *ctxParam = NULL;
    int32_t ret = HksGetParam(keyNode->runtimeParamSet, HKS_TAG_CRYPTO_CTX, &ctxParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_BAD_STATE, "get ctx from keyNode failed!")

    void *ctx = (void *)(uintptr_t)ctxParam->uint64Param;
    HKS_IF_NULL_LOGE_RETURN(ctx, HKS_ERROR_BAD_STATE, "ctx is invalid: null!")

    struct HksBlob *cachedData = (struct HksBlob *)ctx;
    ret = GetNewCachedData(cachedData, srcData, outData);
    HKS_IF_NOT_SUCC_LOGE(ret, "get new cached data failed, ret = %" LOG_PUBLIC "d", ret)

    FreeCachedData(&cachedData);
    ctxParam->uint64Param = 0; /* clear ctx to NULL */
    return ret;
}

static int32_t CoreHashInit(const struct HuksKeyNode *keyNode, uint32_t alg)
{
    struct HksParam *ctxParam = NULL;
    int32_t ret = HksGetParam(keyNode->runtimeParamSet, HKS_TAG_CRYPTO_CTX, &ctxParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_BAD_STATE, "get ctx from keyNode failed!")

    void *ctx = NULL;
    HKS_LOG_I("Hal hash init.");

    ret = HksCryptoHalHashInit(alg, &ctx);
    if (ret != HKS_SUCCESS)  {
        HKS_LOG_E("hal hash init failed ret : %" LOG_PUBLIC "d", ret);
        return ret;
    }
    ctxParam->uint64Param = (uint64_t)(uintptr_t)ctx;
    return HKS_SUCCESS;
}

static int32_t CoreHashUpdate(const struct HuksKeyNode *keyNode, const struct HksBlob *srcData)
{
    void *ctx = GetCryptoCtx(keyNode);
    HKS_IF_NULL_LOGE_RETURN(ctx, HKS_ERROR_BAD_STATE, "ctx is invalid: null!")

    HKS_LOG_I("Hal hash update.");

    int32_t ret = HksCryptoHalHashUpdate(srcData, ctx);
    HKS_IF_NOT_SUCC_LOGE(ret, "hal hash update failed ret : %" LOG_PUBLIC "d", ret)

    return ret;
}

static int32_t CoreHashFinish(const struct HuksKeyNode *keyNode, const struct HksBlob *srcData,
    struct HksBlob *outData)
{
    struct HksParam *ctxParam = NULL;
    int32_t ret = HksGetParam(keyNode->runtimeParamSet, HKS_TAG_CRYPTO_CTX, &ctxParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_BAD_STATE, "get ctx from keyNode failed!")

    void *ctx = (void *)(uintptr_t)ctxParam->uint64Param;
    HKS_IF_NULL_LOGE_RETURN(ctx, HKS_ERROR_BAD_STATE, "ctx is invalid: null!")

    outData->size = MAX_HASH_SIZE;
    outData->data = (uint8_t *)HksMalloc(MAX_HASH_SIZE);
    HKS_IF_NULL_LOGE_RETURN(outData->data, HKS_ERROR_MALLOC_FAIL, "malloc fail.")

    HKS_LOG_I("Hal hash final.");

    ret = HksCryptoHalHashFinal(srcData, &ctx, outData);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("hal hash final failed ret : %" LOG_PUBLIC "d", ret);
        HKS_FREE_BLOB(*outData);
    }

    ctxParam->uint64Param = 0; /* clear ctx to NULL */
    return ret;
}

static int32_t CheckSignVerifyParams(const struct HuksKeyNode *keyNode, const struct HksBlob *outData)
{
    HKS_IF_NOT_SUCC_LOGE_RETURN(CheckBlob(outData), HKS_ERROR_INVALID_ARGUMENT, "invalid outData")

    struct HksParam *tmpParam = NULL;
    int32_t ret = HksGetParam(keyNode->keyBlobParamSet, HKS_TAG_ALGORITHM, &tmpParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get alg from keyNode failed!")

    uint32_t alg = tmpParam->uint32Param;

    ret = HksGetParam(keyNode->runtimeParamSet, HKS_TAG_PURPOSE, &tmpParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get purpoes from keyNode failed!")

    uint32_t purpose = tmpParam->uint32Param;

    ret = HksGetParam(keyNode->keyBlobParamSet, HKS_TAG_KEY_SIZE, &tmpParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get key size from keyNode failed!")

    uint32_t keySize = tmpParam->uint32Param;

    ret = HksCheckSignature((purpose == HKS_KEY_PURPOSE_SIGN) ? HKS_CMD_ID_SIGN : HKS_CMD_ID_VERIFY,
        alg, keySize, outData);
    HKS_IF_NOT_SUCC_LOGE(ret, "check signature failed!")

    return ret;
}

static int32_t CoreSignVerify(const struct HuksKeyNode *keyNode, const struct HksBlob *inData,
    struct HksBlob *outData)
{
    int32_t ret = CheckSignVerifyParams(keyNode, outData);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    struct HksBlob rawKey = { 0, NULL };
    ret = HksGetRawKey(keyNode->keyBlobParamSet, &rawKey);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "SignVerify get raw key failed!")

    struct HksUsageSpec usageSpec;
    (void)memset_s(&usageSpec, sizeof(struct HksUsageSpec), 0, sizeof(struct HksUsageSpec));
    HksFillUsageSpec(keyNode->runtimeParamSet, &usageSpec);

    if (usageSpec.purpose == HKS_KEY_PURPOSE_SIGN) {
        ret = HksCryptoHalSign(&rawKey, &usageSpec, inData, outData);
    } else {
        ret = HksCryptoHalVerify(&rawKey, &usageSpec, inData, outData);
    }
    HKS_IF_NOT_SUCC_LOGE(ret, "SignVerify Finish failed, purpose = 0x%" LOG_PUBLIC "x, ret = %" LOG_PUBLIC "d",
            usageSpec.purpose, ret)

    (void)memset_s(rawKey.data, rawKey.size, 0, rawKey.size);
    HKS_FREE_PTR(rawKey.data);
    return ret;
}

static void FreeSignVerifyCtx(const struct HuksKeyNode *keyNode)
{
    struct HksParam *ctxParam = NULL;
    int32_t ret = HksGetParam(keyNode->runtimeParamSet, HKS_TAG_CRYPTO_CTX, &ctxParam);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("get ctx from keyNode failed!");
        return;
    }
    void *ctx = (void *)(uintptr_t)ctxParam->uint64Param;
    if (ctx == NULL) {
        return;
    }

    struct HksParam *algParam = NULL;
    ret = HksGetParam(keyNode->runtimeParamSet, HKS_TAG_ALGORITHM, &algParam);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("append cipher get alg param failed!");
        return;
    }
    struct HksParam *digestParam = NULL;
    ret = HksGetParam(keyNode->runtimeParamSet, HKS_TAG_DIGEST, &digestParam);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("append cipher get digest param failed!");
        return;
    }
    if (HksCheckNeedCache(algParam->uint32Param, digestParam->uint32Param) == HKS_SUCCESS) {
        struct HksBlob *cachedData = (struct HksBlob *)ctx;
        FreeCachedData(&cachedData);
    } else {
        HksCryptoHalHashFreeCtx(&ctx);
    }

    ctxParam->uint64Param = 0; /* clear ctx to NULL */
}

static int32_t CoreCipherInit(const struct HuksKeyNode *keyNode)
{
    struct HksParam *ctxParam = NULL;
    int32_t ret = HksGetParam(keyNode->runtimeParamSet, HKS_TAG_CRYPTO_CTX, &ctxParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_BAD_STATE, "get ctx from keyNode failed!")

    struct HksParam *purposeParam = NULL;
    ret = HksGetParam(keyNode->runtimeParamSet, HKS_TAG_PURPOSE, &purposeParam);
    HKS_IF_NOT_SUCC_RETURN(ret, HKS_ERROR_CHECK_GET_PURPOSE_FAIL)

    struct HksBlob rawKey = { 0, NULL };
    struct HksUsageSpec *usageSpec = NULL;
    do {
        uint8_t tmpData[HKS_TEMP_SIZE] = {0};
        struct HksBlob tmpInData = { HKS_TEMP_SIZE, tmpData };
        bool isEncrypt = (purposeParam->uint32Param == HKS_KEY_PURPOSE_ENCRYPT) ? true : false;
        ret = HksBuildCipherUsageSpec(keyNode->runtimeParamSet, isEncrypt, &tmpInData, &usageSpec);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "build cipher usage failed")

        ret = HksGetRawKey(keyNode->keyBlobParamSet, &rawKey);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "cipher get raw key failed")

        HKS_LOG_I("Hal encrypt or decrypt init.");

        void *ctx = NULL;
        if (purposeParam->uint32Param == HKS_KEY_PURPOSE_ENCRYPT) {
            ret = HksCryptoHalEncryptInit(&rawKey, usageSpec, &ctx);
        } else {
            ret = HksCryptoHalDecryptInit(&rawKey, usageSpec, &ctx);
        }
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "cipher ctx init failed, ret = %" LOG_PUBLIC "d", ret)
        ctxParam->uint64Param = (uint64_t)(uintptr_t)ctx;
    } while (0);

    if (rawKey.data != NULL) {
        (void)memset_s(rawKey.data, rawKey.size, 0, rawKey.size);
    }
    HKS_FREE_PTR(rawKey.data);
    HksFreeUsageSpec(&usageSpec);
    return ret;
}

static int32_t CoreCipherUpdate(const struct HuksKeyNode *keyNode, const struct HksBlob *inData,
    struct HksBlob *outData, uint32_t alg)
{
    HKS_IF_NOT_SUCC_LOGE_RETURN(CheckBlob(outData), HKS_ERROR_INVALID_ARGUMENT, "invalid outData")

    if (outData->size < inData->size) {
        HKS_LOG_E("cipher update, out buffer too small size: %" LOG_PUBLIC "u, inSize: %" LOG_PUBLIC "u",
            outData->size, inData->size);
        return HKS_ERROR_BUFFER_TOO_SMALL;
    }

    void *ctx = GetCryptoCtx(keyNode);
    HKS_IF_NULL_LOGE_RETURN(ctx, HKS_ERROR_NULL_POINTER, "ctx is invalid: null!")

    struct HksParam *purposeParam = NULL;
    int32_t ret = HksGetParam(keyNode->runtimeParamSet, HKS_TAG_PURPOSE, &purposeParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "append cipher get purpose param failed!")

    HKS_LOG_I("Hal encrypt or decrypt.");

    if (purposeParam->uint32Param == HKS_KEY_PURPOSE_ENCRYPT) {
        ret = HksCryptoHalEncryptUpdate(inData, ctx, outData, alg);
    } else {
        ret = HksCryptoHalDecryptUpdate(inData, ctx, outData, alg);
    }
    HKS_IF_NOT_SUCC_LOGE(ret, "cipher update failed! ret : %" LOG_PUBLIC "d", ret)
    return ret;
}

static int32_t CoreAesEncryptFinish(const struct HuksKeyNode *keyNode,
    const struct HksBlob *inData, struct HksBlob *outData, uint32_t alg)
{
    struct HksBlob tag = { 0, NULL };
    int32_t ret = HksGetEncryptAeTag(keyNode->runtimeParamSet, inData, outData, &tag);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "cipher encrypt get ae tag failed!")

    ret = HksCheckFinishOutSize(true, keyNode->runtimeParamSet, inData, outData);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "aes encrypt finish check data size failed")

    struct HksParam *ctxParam = NULL;
    ret = HksGetParam(keyNode->runtimeParamSet, HKS_TAG_CRYPTO_CTX, &ctxParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get ctx from keyNode failed!")

    void *ctx = (void *)(uintptr_t)ctxParam->uint64Param;
    HKS_IF_NULL_LOGE_RETURN(ctx, HKS_ERROR_NULL_POINTER, "ctx is invalid: null!")

    ret = HksCryptoHalEncryptFinal(inData, &ctx, outData, &tag, alg);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("aes encrypt Finish failed! ret : %" LOG_PUBLIC "d", ret);
        ctxParam->uint64Param = 0; /* clear ctx to NULL */
        return ret;
    }

    outData->size += tag.size;
    ctxParam->uint64Param = 0; /* clear ctx to NULL */
    return HKS_SUCCESS;
}

static int32_t CoreAesDecryptFinish(const struct HuksKeyNode *keyNode,
    const struct HksBlob *inData, struct HksBlob *outData, uint32_t alg)
{
    bool isAes = false;
    bool isAeMode = false;
    int32_t ret = HksCheckAesAeMode(keyNode->runtimeParamSet, &isAes, &isAeMode);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get aeMode failed!")

    struct HksBlob tag = { 0, NULL };
    if (isAes && isAeMode) {
        struct HksParam *tagParam = NULL;
        ret = HksGetParam(keyNode->runtimeParamSet, HKS_TAG_AE_TAG, &tagParam);
        if (ret != HKS_SUCCESS || tagParam == NULL) {
            HKS_LOG_E("get tag failed!");
            return ret;
        }
        tag = tagParam->blob;
    }

    ret = HksCheckFinishOutSize(false, keyNode->runtimeParamSet, inData, outData);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "aes decrypt finish check data size failed")

    struct HksParam *ctxParam = NULL;
    ret = HksGetParam(keyNode->runtimeParamSet, HKS_TAG_CRYPTO_CTX, &ctxParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get ctx from keyNode failed!")

    void *ctx = (void *)(uintptr_t)ctxParam->uint64Param;
    HKS_IF_NULL_LOGE_RETURN(ctx, HKS_ERROR_NULL_POINTER, "ctx is invalid: null!")

    ret = HksCryptoHalDecryptFinal(inData, &ctx, outData, &tag, alg);
    HKS_IF_NOT_SUCC_LOGE(ret, "cipher DecryptFinish failed! ret : %" LOG_PUBLIC "d", ret)

    ctxParam->uint64Param = 0; /* clear ctx to NULL */
    return ret;
}


static int32_t CoreSm4EncryptFinish(const struct HuksKeyNode *keyNode,
    const struct HksBlob *inData, struct HksBlob *outData, uint32_t alg)
{
    int32_t ret = HksCheckFinishOutSize(true, keyNode->runtimeParamSet, inData, outData);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "aes encrypt finish check data size failed")

    struct HksParam *ctxParam = NULL;
    ret = HksGetParam(keyNode->runtimeParamSet, HKS_TAG_CRYPTO_CTX, &ctxParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get ctx from keyNode failed!")

    void *ctx = (void *)(uintptr_t)ctxParam->uint64Param;
    HKS_IF_NULL_LOGE_RETURN(ctx, HKS_ERROR_NULL_POINTER, "ctx is invalid: null!")

    ret = HksCryptoHalEncryptFinal(inData, &ctx, outData, NULL, alg);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("aes encrypt Finish failed! ret : %" LOG_PUBLIC "d", ret);
        ctxParam->uint64Param = 0; /* clear ctx to NULL */
        return ret;
    }

    ctxParam->uint64Param = 0; /* clear ctx to NULL */
    return HKS_SUCCESS;
}

static int32_t CoreSm4DecryptFinish(const struct HuksKeyNode *keyNode,
    const struct HksBlob *inData, struct HksBlob *outData, uint32_t alg)
{
    int32_t ret = HksCheckFinishOutSize(false, keyNode->runtimeParamSet, inData, outData);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "sm4 decrypt finish check data size failed")

    struct HksParam *ctxParam = NULL;
    ret = HksGetParam(keyNode->runtimeParamSet, HKS_TAG_CRYPTO_CTX, &ctxParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get ctx from keyNode failed!")

    void *ctx = (void *)(uintptr_t)ctxParam->uint64Param;
    HKS_IF_NULL_LOGE_RETURN(ctx, HKS_ERROR_NULL_POINTER, "ctx is invalid: null!")

    ret = HksCryptoHalDecryptFinal(inData, &ctx, outData, NULL, alg);
    HKS_IF_NOT_SUCC_LOGE(ret, "cipher DecryptFinish failed! ret : %" LOG_PUBLIC "d", ret)

    ctxParam->uint64Param = 0; /* clear ctx to NULL */
    return ret;
}

static int32_t RsaCipherFinish(const struct HuksKeyNode *keyNode, const struct HksBlob *inData,
    struct HksBlob *outData)
{
    HKS_LOG_E("rsa inData.size = %" LOG_PUBLIC "u", inData->size);
    struct HksBlob rawKey = { 0, NULL };
    int32_t ret = HksGetRawKey(keyNode->keyBlobParamSet, &rawKey);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "SignVerify get raw key failed!")

    struct HksUsageSpec usageSpec;
    (void)memset_s(&usageSpec, sizeof(struct HksUsageSpec), 0, sizeof(struct HksUsageSpec));
    HksFillUsageSpec(keyNode->runtimeParamSet, &usageSpec);

    bool isEncrypt = (usageSpec.purpose == HKS_KEY_PURPOSE_ENCRYPT) ? true : false;
    ret = HksCheckFinishOutSize(isEncrypt, keyNode->runtimeParamSet, inData, outData);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("rsa cipher finish check data size failed");
        (void)memset_s(rawKey.data, rawKey.size, 0, rawKey.size);
        HKS_FREE_PTR(rawKey.data);
        return ret;
    }

    if (usageSpec.purpose == HKS_KEY_PURPOSE_ENCRYPT) {
        struct HksBlob tag = { 0, NULL };
        ret = HksCryptoHalEncrypt(&rawKey, &usageSpec, inData, outData, &tag);
    } else {
        ret = HksCryptoHalDecrypt(&rawKey, &usageSpec, inData, outData);
    }
    HKS_IF_NOT_SUCC_LOGE(ret, "rsa cipher Finish failed, purpose = 0x%" LOG_PUBLIC "x, ret = %" LOG_PUBLIC "d",
            usageSpec.purpose, ret)

    (void)memset_s(rawKey.data, rawKey.size, 0, rawKey.size);
    HKS_FREE_PTR(rawKey.data);
    return ret;
}

static int32_t CoreRsaCipherFinish(const struct HuksKeyNode *keyNode, const struct HksBlob *inData,
    struct HksBlob *outData)
{
    struct HksBlob tempInData = { 0, NULL };
    int32_t ret = FinishCachedData(keyNode, inData, &tempInData);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get rsa cipher cached data faile")

    ret = RsaCipherFinish(keyNode, &tempInData, outData);
    HKS_FREE_BLOB(tempInData);
    return ret;
}

static void FreeCryptoCtx(const struct HuksKeyNode *keyNode, uint32_t alg)
{
    struct HksParam *ctxParam = NULL;
    int32_t ret = HksGetParam(keyNode->runtimeParamSet, HKS_TAG_CRYPTO_CTX, &ctxParam);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("get ctx from keyNode failed!");
        return;
    }
    void *ctx = (void *)(uintptr_t)ctxParam->uint64Param;
    if (ctx == NULL) {
        return;
    }

    if (alg == HKS_ALG_AES || alg == HKS_ALG_SM4) {
        HksCryptoHalEncryptFreeCtx(&ctx, alg);
    } else {
        struct HksBlob *cachedData = (struct HksBlob *)ctx;
        FreeCachedData(&cachedData);
    }
    ctxParam->uint64Param = 0; /* clear ctx to NULL */
}

static int32_t GetRawkey(const struct HuksKeyNode *keyNode, struct HksBlob *rawKey)
{
    if (GetCryptoCtx(keyNode) != NULL) {
        HKS_LOG_E("avoid running into this function multiple times!");
        return HKS_FAILURE;
    }

    int32_t ret = HksThreeStageAuth(HKS_AUTH_ID_DERIVE, keyNode);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "derive auth failed!")

    ret = HksGetRawKey(keyNode->keyBlobParamSet, rawKey);
    HKS_IF_NOT_SUCC_LOGE(ret, "Derive get raw key failed!")

    return ret;
}

static void FreeOutBlob(struct HksBlob **out)
{
    if ((out == NULL) || (*out == NULL)) {
        return;
    }
    HKS_FREE_PTR((*out)->data);
    HKS_FREE_PTR(*out);
}

static int32_t ConstructDervieBlob(const struct HksParamSet *paramSet, struct HksBlob **out)
{
    struct HksParam *deriveSizeParam = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_DERIVE_KEY_SIZE, &deriveSizeParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_INVALID_ARGUMENT, "get derive size failed, ret = %" LOG_PUBLIC "d", ret)

    uint32_t deriveSize = deriveSizeParam->uint32Param;
    if ((deriveSize == 0) || (deriveSize > MAX_OUT_BLOB_SIZE)) {
        HKS_LOG_E("derive size invalid, size = %" LOG_PUBLIC "u", deriveSize);
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    struct HksBlob *tempOut = (struct HksBlob *)HksMalloc(sizeof(struct HksBlob));
    HKS_IF_NULL_LOGE_RETURN(tempOut, HKS_ERROR_MALLOC_FAIL, "construct derive blob malloc failed")

    tempOut->data = (uint8_t *)HksMalloc(deriveSize);
    if (tempOut->data == NULL) {
        HKS_FREE_PTR(tempOut);
        HKS_LOG_E("malloc out derive blob data failed.");
        return HKS_ERROR_MALLOC_FAIL;
    }
    tempOut->size = deriveSize;
    *out = tempOut;
    return HKS_SUCCESS;
}

static int32_t ConstructAgreeBlob(struct HksBlob **agreeOut)
{
    struct HksBlob *agreeTemp = (struct HksBlob *)HksMalloc(sizeof(struct HksBlob));
    HKS_IF_NULL_LOGE_RETURN(agreeTemp, HKS_ERROR_MALLOC_FAIL, "malloc agreeTemp failed.")

    agreeTemp->size = MAX_KEY_SIZE;
    agreeTemp->data = (uint8_t *)HksMalloc(MAX_KEY_SIZE);
    if (agreeTemp->data == NULL) {
        HKS_LOG_E("malloc agreeTemp data failed.");
        HKS_FREE_PTR(agreeTemp);
        return HKS_ERROR_MALLOC_FAIL;
    }
    *agreeOut = agreeTemp;
    return HKS_SUCCESS;
}


int32_t HksCoreSignVerifyThreeStageInit(const struct HuksKeyNode *keyNode, const struct HksParamSet *paramSet,
    uint32_t alg)
{
    (void)paramSet;
    int32_t ret = SignVerifyAuth(keyNode, keyNode->runtimeParamSet);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "HksCoreSignVerifyThreeStageInit SignAuth fail ret : %" LOG_PUBLIC "d", ret)

    struct HksParam *algParam = NULL;
    ret = HksGetParam(keyNode->runtimeParamSet, HKS_TAG_ALGORITHM, &algParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_CHECK_GET_ALG_FAIL,
        "get param get 0x%" LOG_PUBLIC "x failed", HKS_TAG_ALGORITHM)

    uint32_t digest = alg;  // In signature or verify scenario, alg represents digest. See code {GetPurposeAndAlgorithm}
    HKS_LOG_I("Init cache or hash init.");
    if (HksCheckNeedCache(algParam->uint32Param, digest) == HKS_SUCCESS) {
        return SetCacheModeCtx(keyNode);
    } else {
        return CoreHashInit(keyNode, alg);
    }
}

int32_t HksCoreSignVerifyThreeStageUpdate(const struct HuksKeyNode *keyNode, const struct HksParamSet *paramSet,
    const struct HksBlob *srcData, struct HksBlob *signature, uint32_t alg)
{
    (void)signature;
    (void)alg;
    (void)paramSet;

    struct HksParam *algParam = NULL;
    int32_t ret = HksGetParam(keyNode->runtimeParamSet, HKS_TAG_ALGORITHM, &algParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_CHECK_GET_ALG_FAIL,
        "get param get 0x%" LOG_PUBLIC "x failed", HKS_TAG_ALGORITHM)

    uint32_t digest = alg;  // In signature or verify scenario, alg represents digest. See code {GetPurposeAndAlgorithm}
    HKS_LOG_I("Update cache or hash update.");
    if (HksCheckNeedCache(algParam->uint32Param, digest) == HKS_SUCCESS) {
        return UpdateCachedData(keyNode, srcData);
    } else {
        return CoreHashUpdate(keyNode, srcData);
    }
}

int32_t HksCoreSignVerifyThreeStageFinish(const struct HuksKeyNode *keyNode, const struct HksParamSet *paramSet,
    const struct HksBlob *inData, struct HksBlob *outData, uint32_t alg)
{
    (void)paramSet;
    (void)alg;

    struct HksBlob message = { 0, NULL };
    struct HksParam *purposeParam = NULL;
    int32_t ret = HksGetParam(keyNode->runtimeParamSet, HKS_TAG_PURPOSE, &purposeParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_CHECK_GET_PURPOSE_FAIL,
        "get param get 0x%" LOG_PUBLIC "x failed", HKS_TAG_PURPOSE)

    if (purposeParam->uint32Param == HKS_KEY_PURPOSE_SIGN) { /* inData indicates signature when processing verify */
        message.data = inData->data;
        message.size = inData->size;
    }

    struct HksParam *algParam = NULL;
    ret = HksGetParam(keyNode->runtimeParamSet, HKS_TAG_ALGORITHM, &algParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_CHECK_GET_ALG_FAIL,
        "get param get 0x%" LOG_PUBLIC "x failed", HKS_TAG_ALGORITHM)

    uint32_t digest = alg;  // In signature or verify scenario, alg represents digest. See code {GetPurposeAndAlgorithm}
    HKS_LOG_I("Finish cache or hash finish.");
    struct HksBlob signVerifyData = { 0, NULL };
    if (HksCheckNeedCache(algParam->uint32Param, digest) == HKS_SUCCESS) {
        ret = FinishCachedData(keyNode, &message, &signVerifyData);
    } else {
        ret = CoreHashFinish(keyNode, &message, &signVerifyData);
    }
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "signVerify Finish get Data failed, ret = %" LOG_PUBLIC "d", ret)

    /* inData indicates signature when processing verify */
    ret = CoreSignVerify(keyNode, &signVerifyData,
        (purposeParam->uint32Param == HKS_KEY_PURPOSE_SIGN) ? outData : (struct HksBlob *)inData);
    HKS_FREE_BLOB(signVerifyData);
    return ret;
}

int32_t HksCoreSignVerifyThreeStageAbort(const struct HuksKeyNode *keyNode, const struct HksParamSet *paramSet,
    uint32_t alg)
{
    (void)paramSet;
    (void)alg;

    FreeSignVerifyCtx(keyNode);
    return HKS_SUCCESS;
}

int32_t HksCoreCryptoThreeStageInit(const struct HuksKeyNode *keyNode, const struct HksParamSet *paramSet,
    uint32_t alg)
{
    (void)alg;

    int32_t ret = CipherAuth(keyNode, paramSet);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "cipher init failed, ret = %" LOG_PUBLIC "d", ret)

    struct HksParam *algParam = NULL;
    ret = HksGetParam(keyNode->runtimeParamSet, HKS_TAG_ALGORITHM, &algParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_CHECK_GET_ALG_FAIL,
        "get param get 0x%" LOG_PUBLIC "x failed", HKS_TAG_ALGORITHM)

    HKS_LOG_I("Init cache or cipher init.");

    if (algParam->uint32Param == HKS_ALG_RSA) {
        return SetCacheModeCtx(keyNode);
    } else if (algParam->uint32Param == HKS_ALG_AES) {
        return CoreCipherInit(keyNode);
    } else if (algParam->uint32Param == HKS_ALG_SM4) {
        return CoreCipherInit(keyNode);
    } else {
        return HKS_ERROR_INVALID_ALGORITHM;
    }
}

int32_t HksCoreCryptoThreeStageUpdate(const struct HuksKeyNode *keyNode, const struct HksParamSet *paramSet,
    const struct HksBlob *inData, struct HksBlob *outData, uint32_t alg)
{
    (void)paramSet;
    struct HksParam *algParam = NULL;
    int32_t ret = HksGetParam(keyNode->runtimeParamSet, HKS_TAG_ALGORITHM, &algParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_CHECK_GET_ALG_FAIL,
        "get param get 0x%" LOG_PUBLIC "x failed", HKS_TAG_ALGORITHM)

    HKS_LOG_I("Update cache or cipher update.");

    if (algParam->uint32Param == HKS_ALG_RSA) {
        return UpdateCachedData(keyNode, inData);
    } else if (algParam->uint32Param == HKS_ALG_AES) {
        return CoreCipherUpdate(keyNode, inData, outData, alg);
    } else if (algParam->uint32Param == HKS_ALG_SM4) {
        return CoreCipherUpdate(keyNode, inData, outData, alg);
    } else {
        return HKS_ERROR_INVALID_ALGORITHM;
    }
}

int32_t HksCoreEncryptThreeStageFinish(const struct HuksKeyNode *keyNode, const struct HksParamSet *paramSet,
    const struct HksBlob *inData, struct HksBlob *outData, uint32_t alg)
{
    (void)paramSet;
    HKS_IF_NOT_SUCC_LOGE_RETURN(CheckBlob(outData), HKS_ERROR_INVALID_ARGUMENT, "invalid outData")

    struct HksParam *algParam = NULL;
    int32_t ret = HksGetParam(keyNode->runtimeParamSet, HKS_TAG_ALGORITHM, &algParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_CHECK_GET_ALG_FAIL,
        "get param get 0x%" LOG_PUBLIC "x failed", HKS_TAG_ALGORITHM)

    HKS_LOG_I("Finish cache or encrypt finish.");

    if (algParam->uint32Param == HKS_ALG_RSA) {
        return CoreRsaCipherFinish(keyNode, inData, outData);
    } else if (algParam->uint32Param == HKS_ALG_AES) {
        return CoreAesEncryptFinish(keyNode, inData, outData, alg);
    } else if (algParam->uint32Param == HKS_ALG_SM4) {
        return CoreSm4EncryptFinish(keyNode, inData, outData, alg);
    } else {
        return HKS_ERROR_INVALID_ALGORITHM;
    }
}

int32_t HksCoreDecryptThreeStageFinish(const struct HuksKeyNode *keyNode, const struct HksParamSet *paramSet,
    const struct HksBlob *inData, struct HksBlob *outData, uint32_t alg)
{
    (void)paramSet;
    HKS_IF_NOT_SUCC_LOGE_RETURN(CheckBlob(outData), HKS_ERROR_INVALID_ARGUMENT, "invalid outData")

    struct HksParam *algParam = NULL;
    int32_t ret = HksGetParam(keyNode->runtimeParamSet, HKS_TAG_ALGORITHM, &algParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_CHECK_GET_ALG_FAIL,
        "get param get 0x%" LOG_PUBLIC "x failed", HKS_TAG_ALGORITHM)

    HKS_LOG_I("Finish cache or decrypt finish.");

    if (algParam->uint32Param == HKS_ALG_RSA) {
        return CoreRsaCipherFinish(keyNode, inData, outData);
    } else if (algParam->uint32Param == HKS_ALG_AES) {
        return CoreAesDecryptFinish(keyNode, inData, outData, alg);
    } else if (algParam->uint32Param == HKS_ALG_SM4) {
        return CoreSm4DecryptFinish(keyNode, inData, outData, alg);
    } else {
        return HKS_ERROR_INVALID_ALGORITHM;
    }
}

int32_t HksCoreCryptoThreeStageAbort(const struct HuksKeyNode *keyNode, const struct HksParamSet *paramSet,
    uint32_t alg)
{
    (void)paramSet;
    FreeCryptoCtx(keyNode, alg);
    return HKS_SUCCESS;
}

int32_t HksCoreDeriveThreeStageInit(const struct HuksKeyNode *keyNode, const struct HksParamSet *paramSet,
    uint32_t alg)
{
    (void)keyNode;
    (void)paramSet;
    (void)alg;

    HKS_LOG_D("HksCoreDeriveThreeStageInit start");
    return HKS_SUCCESS;
}

int32_t HksCoreDeriveThreeStageUpdate(const struct HuksKeyNode *keyNode, const struct HksParamSet *paramSet,
    const struct HksBlob *srcData, struct HksBlob *derive, uint32_t alg)
{
    (void)srcData;
    (void)alg;
    (void)derive;
    (void)paramSet;
    struct HksParam *ctxParam = NULL;
    int32_t ret = HksGetParam(keyNode->runtimeParamSet, HKS_TAG_CRYPTO_CTX, &ctxParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_BAD_STATE, "get ctx from keyNode failed!")

    struct HksBlob rawKey = { 0, NULL };
    do {
        ret = GetRawkey(keyNode, &rawKey);
        HKS_IF_NOT_SUCC_BREAK(ret)

        struct HksBlob *deriveBlob = NULL;
        ret = ConstructDervieBlob(keyNode->runtimeParamSet, &deriveBlob);
        HKS_IF_NOT_SUCC_BREAK(ret)

        struct HksKeyDerivationParam derParam = { { 0, NULL }, { 0, NULL }, 0, 0 };
        struct HksKeySpec derivationSpec = { 0, 0, &derParam };
        HksFillKeySpec(keyNode->runtimeParamSet, &derivationSpec);
        HksFillKeyDerivationParam(keyNode->runtimeParamSet, &derParam);

        HKS_LOG_I("Hal derive update.");

        ret = HksCryptoHalDeriveKey(&rawKey, &derivationSpec, deriveBlob);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksCryptoHalDeriveKey fail");
            FreeOutBlob(&deriveBlob);
            break;
        }

        ctxParam->uint64Param = (uint64_t)(uintptr_t)deriveBlob;
    } while (0);

    if (rawKey.data != NULL) {
        (void)memset_s(rawKey.data, rawKey.size, 0, rawKey.size);
    }
    HKS_FREE_PTR(rawKey.data);

    return ret;
}

static int32_t BuildKeyBlobOrGetOutData(const struct HksParamSet *paramSet, const struct HksBlob *restoreData,
    struct HksBlob *outData, uint8_t keyFlag)
{
    bool needStore = false;
    struct HksParam *storage = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_KEY_STORAGE_FLAG, &storage);
    if (ret == HKS_SUCCESS) {
        if (storage != NULL && storage->uint32Param == HKS_STORAGE_PERSISTENT) {
            needStore = true;
        }
    }

    do {
        if (needStore) {
            ret = HksBuildKeyBlob(NULL, keyFlag, restoreData, paramSet, outData);
            HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksBuildKeyBlob failed! ret: %" LOG_PUBLIC "d", ret)
        } else {
            if (outData->size < restoreData->size) {
                HKS_LOG_E("outData size is too small, size : %" LOG_PUBLIC "u", outData->size);
                ret = HKS_ERROR_BUFFER_TOO_SMALL;
                break;
            }
            outData->size = restoreData->size;
            (void)memcpy_s(outData->data, outData->size, restoreData->data, outData->size);
            ret = HKS_SUCCESS;
        }
    } while (0);
    return ret;
}

int32_t HksCoreDeriveThreeStageFinish(const struct HuksKeyNode *keyNode, const struct HksParamSet *paramSet,
    const struct HksBlob *inData, struct HksBlob *outData, uint32_t alg)
{
    HKS_LOG_D("HksCoreDeriveThreeStageFinish start");
    (void)inData;
    (void)alg;
    int32_t ret = CheckBlob(outData);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_INVALID_ARGUMENT, "invalid outData")

    void *ctx = GetCryptoCtx(keyNode);
    HKS_IF_NULL_LOGE_RETURN(ctx, HKS_ERROR_NULL_POINTER, "ctx is NULL!")

    struct HksBlob *restoreData = (struct HksBlob *)ctx;

    ret = BuildKeyBlobOrGetOutData(paramSet, restoreData, outData, HKS_KEY_FLAG_DERIVE_KEY);

    FreeCachedData(&restoreData);
    ClearCryptoCtx(keyNode);
    return ret;
}

int32_t HksCoreDeriveThreeStageAbort(const struct HuksKeyNode *keyNode, const struct HksParamSet *paramSet,
    uint32_t alg)
{
    (void)paramSet;
    (void)alg;

    void *ctx = GetCryptoCtx(keyNode);
    HKS_IF_NULL_LOGE_RETURN(ctx, HKS_ERROR_NULL_POINTER, "ctx is NULL!")

    struct HksBlob *restoreData = (struct HksBlob *)ctx;

    FreeCachedData(&restoreData);
    ClearCryptoCtx(keyNode);
    return HKS_SUCCESS;
}

int32_t HksCoreAgreeThreeStageInit(const struct HuksKeyNode *keyNode, const struct HksParamSet *paramSet,
    uint32_t alg)
{
    (void)keyNode;
    (void)paramSet;
    (void)alg;

    int32_t ret = AgreeAuth(keyNode, keyNode->runtimeParamSet);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "HksCoreAgreeThreeStageInit AgreeAuth fail ret : %" LOG_PUBLIC "d", ret)

    return HKS_SUCCESS;
}

int32_t HksCoreAgreeThreeStageUpdate(const struct HuksKeyNode *keyNode, const struct HksParamSet *paramSet,
    const struct HksBlob *srcData, struct HksBlob *signature, uint32_t alg)
{
    (void)signature;
    (void)paramSet;
    (void)alg;

    struct HksParam *ctxParam = NULL;
    int32_t ret = HksGetParam(keyNode->runtimeParamSet, HKS_TAG_CRYPTO_CTX, &ctxParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_BAD_STATE, "get ctx from keyNode failed!")

    if (ctxParam->uint64Param != 0) {
        HKS_LOG_E("avoid running into this function multiple times!");
        return HKS_FAILURE;
    }

    struct HksBlob rawKey = { 0, NULL };
    struct HksBlob publicKey = { 0, NULL };

    do {
        ret = GetHksPubKeyInnerFormat(keyNode->runtimeParamSet, srcData, &publicKey);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "get public key from x509 format failed, ret = %" LOG_PUBLIC "d.", ret)

        struct HksBlob *agreeTemp = NULL;
        ret = ConstructAgreeBlob(&agreeTemp);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksCoreAgreeBuildData failed, ret = %" LOG_PUBLIC "d.", ret)

        ret = HksGetRawKey(keyNode->keyBlobParamSet, &rawKey);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("agree get raw key failed!");
            FreeOutBlob(&agreeTemp);
            break;
        }

        struct HksKeySpec agreeSpec = { 0 };
        HksFillKeySpec(keyNode->runtimeParamSet, &agreeSpec);

        HKS_LOG_I("Hal agree update.");

        ret = HksCryptoHalAgreeKey(&rawKey, &publicKey, &agreeSpec, agreeTemp);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksCryptoHalAgreeKey failed, ret = %" LOG_PUBLIC "d.", ret);
            FreeOutBlob(&agreeTemp);
            break;
        }

        ctxParam->uint64Param = (uint64_t)(uintptr_t)agreeTemp;
    } while (0);

    if (rawKey.data != NULL) {
        (void)memset_s(rawKey.data, rawKey.size, 0, rawKey.size);
    }
    HKS_FREE_PTR(rawKey.data);
    HKS_FREE_PTR(publicKey.data);
    return ret;
}

int32_t HksCoreAgreeThreeStageFinish(const struct HuksKeyNode *keyNode, const struct HksParamSet *paramSet,
    const struct HksBlob *inData, struct HksBlob *outData, uint32_t alg)
{
    (void)inData;
    (void)alg;
    int32_t ret = CheckBlob(outData);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_INVALID_ARGUMENT, "invalid outData")

    void *ctx = GetCryptoCtx(keyNode);
    HKS_IF_NULL_LOGE_RETURN(ctx, HKS_ERROR_NULL_POINTER, "ctx is NULL!")

    struct HksBlob *restoreData = (struct HksBlob *)ctx;

    ret = BuildKeyBlobOrGetOutData(paramSet, restoreData, outData, HKS_KEY_FLAG_AGREE_KEY);

    FreeCachedData(&restoreData);
    ClearCryptoCtx(keyNode);
    return ret;
}

int32_t HksCoreAgreeThreeStageAbort(const struct HuksKeyNode *keyNode, const struct HksParamSet *paramSet, uint32_t alg)
{
    (void)paramSet;
    (void)alg;

    void *ctx = GetCryptoCtx(keyNode);
    HKS_IF_NULL_LOGE_RETURN(ctx, HKS_ERROR_NULL_POINTER, "ctx is NULL!")

    struct HksBlob *restoreData = (struct HksBlob *)ctx;

    FreeCachedData(&restoreData);
    ClearCryptoCtx(keyNode);
    return HKS_SUCCESS;
}

int32_t HksCoreMacThreeStageInit(const struct HuksKeyNode *keyNode, const struct HksParamSet *paramSet,
    uint32_t alg)
{
    (void)paramSet;
    int32_t ret = HmacAuth(keyNode, keyNode->runtimeParamSet);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "HksCoreMacThreeStageInit MacAuth fail ret : %" LOG_PUBLIC "d", ret)

    struct HksParam *ctxParam = NULL;
    ret = HksGetParam(keyNode->runtimeParamSet, HKS_TAG_CRYPTO_CTX, &ctxParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_BAD_STATE, "get ctx from keyNode failed!")

    struct HksBlob rawKey = { 0, NULL };
    do {
        ret = HksGetRawKey(keyNode->keyBlobParamSet, &rawKey);
        HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "Derive get raw key failed!")

        HKS_LOG_I("Init hmac.");

        void *ctx = NULL;
        ret = HksCryptoHalHmacInit(&rawKey, alg, &ctx);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "hmac init failed! ret : %" LOG_PUBLIC "d", ret)

        ctxParam->uint64Param = (uint64_t)(uintptr_t)ctx;
    } while (0);

    if (rawKey.data != NULL) {
        (void)memset_s(rawKey.data, rawKey.size, 0, rawKey.size);
    }
    HKS_FREE_PTR(rawKey.data);

    return ret;
}

int32_t HksCoreMacThreeStageUpdate(const struct HuksKeyNode *keyNode, const struct HksParamSet *paramSet,
    const struct HksBlob *srcData, struct HksBlob *mac, uint32_t alg)
{
    (void)paramSet;
    (void)mac;
    (void)alg;
    int32_t ret;

    void *ctx = GetCryptoCtx(keyNode);
    HKS_IF_NULL_LOGE_RETURN(ctx, HKS_ERROR_NULL_POINTER, "ctx is NULL!")

    HKS_LOG_I("Hal hmac update.");

    ret = HksCryptoHalHmacUpdate(srcData, ctx);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "hmac update failed! ret : %" LOG_PUBLIC "d", ret)

    return HKS_SUCCESS;
}

int32_t HksCoreMacThreeStageFinish(const struct HuksKeyNode *keyNode, const struct HksParamSet *paramSet,
    const struct HksBlob *inData, struct HksBlob *outData, uint32_t alg)
{
    (void)paramSet;
    (void)alg;

    struct HksParam *digestParam = NULL;
    int32_t ret = HksGetParam(keyNode->runtimeParamSet, HKS_TAG_DIGEST, &digestParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_CHECK_GET_DIGEST_FAIL, "get ctx from keyNode failed!")

    uint32_t macLen;
    ret = HksGetDigestLen(digestParam->uint32Param, &macLen);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get digest len failed")

    if ((CheckBlob(outData) != HKS_SUCCESS) || (outData->size < macLen)) {
        HKS_LOG_E("out buffer too small");
        return HKS_ERROR_BUFFER_TOO_SMALL;
    }

    struct HksParam *ctxParam = NULL;
    ret = HksGetParam(keyNode->runtimeParamSet, HKS_TAG_CRYPTO_CTX, &ctxParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_BAD_STATE, "get ctx from keyNode failed!")

    void *ctx = (void *)(uintptr_t)ctxParam->uint64Param;
    HKS_IF_NULL_LOGE_RETURN(ctx, HKS_ERROR_NULL_POINTER, "ctx invalid")

    HKS_LOG_I("Finish cache or hmac finish.");

    ret = HksCryptoHalHmacFinal(inData, &ctx, outData);
    HKS_IF_NOT_SUCC_LOGE(ret, "hmac final failed! ret : %" LOG_PUBLIC "d", ret)

    ctxParam->uint64Param = 0; /* clear ctx to NULL */
    return ret;
}

int32_t HksCoreMacThreeStageAbort(const struct HuksKeyNode *keyNode, const struct HksParamSet *paramSet, uint32_t alg)
{
    (void)alg;
    (void)paramSet;

    void *ctx = GetCryptoCtx(keyNode);
    HKS_IF_NULL_LOGE_RETURN(ctx, HKS_ERROR_NULL_POINTER, "ctx invalid")

    HksCryptoHalHmacFreeCtx(&ctx);
    ClearCryptoCtx(keyNode);

    return HKS_SUCCESS;
}

#endif /* _CUT_AUTHENTICATE_ */
