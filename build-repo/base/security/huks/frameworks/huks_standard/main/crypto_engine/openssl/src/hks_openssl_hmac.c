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

#ifdef HKS_SUPPORT_HMAC_C

#include "hks_openssl_hmac.h"

#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/ossl_typ.h>
#include <stddef.h>

#include "hks_common_check.h"
#include "hks_log.h"
#include "hks_mem.h"
#include "hks_openssl_common.h"
#include "hks_openssl_engine.h"
#include "hks_template.h"

struct HksOpensslHmacCtx {
    uint32_t digestLen;
    void *append;
};

static int32_t HmacCheckBuffer(const struct HksBlob *key, const struct HksBlob *msg, const struct HksBlob *mac)
{
    HKS_IF_NOT_SUCC_LOGE_RETURN(HksOpensslCheckBlob(key), HKS_ERROR_INVALID_ARGUMENT, "Invalid key point")
    HKS_IF_NOT_SUCC_LOGE_RETURN(HksOpensslCheckBlob(msg), HKS_ERROR_INVALID_ARGUMENT, "Invalid msg")
    HKS_IF_NOT_SUCC_LOGE_RETURN(HksOpensslCheckBlob(mac), HKS_ERROR_INVALID_ARGUMENT, "Invalid mac")
    return HKS_SUCCESS;
}

static int32_t HmacGenKeyCheckParam(const struct HksKeySpec *spec)
{
    if ((spec->keyLen == 0) || (spec->keyLen % BIT_NUM_OF_UINT8 != 0)) {
        HKS_LOG_E("keyLen is wrong, len = %" LOG_PUBLIC "u", spec->keyLen);
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    return HKS_SUCCESS;
}

#ifdef HKS_SUPPORT_HMAC_GENERATE_KEY
int32_t HksOpensslHmacGenerateKey(const struct HksKeySpec *spec, struct HksBlob *key)
{
    HKS_IF_NOT_SUCC_LOGE_RETURN(HmacGenKeyCheckParam(spec),
        HKS_ERROR_INVALID_ARGUMENT, "aes generate key invalid params!")

    return HksOpensslGenerateRandomKey(spec->keyLen, key);
}
#endif /* HKS_SUPPORT_HMAC_GENERATE_KEY */

static int32_t HmacCheckParam(
    const struct HksBlob *key, uint32_t alg, const struct HksBlob *msg, const struct HksBlob *mac)
{
    HKS_IF_NOT_SUCC_LOGE_RETURN(HmacCheckBuffer(key, msg, mac), HKS_ERROR_INVALID_ARGUMENT, "Invalid Buffer Info")

    if ((alg != HKS_DIGEST_SHA1) && (alg != HKS_DIGEST_SHA224) && (alg != HKS_DIGEST_SHA256) &&
        (alg != HKS_DIGEST_SHA384) && (alg != HKS_DIGEST_SHA512) && (alg != HKS_DIGEST_SM3)) {
        HKS_LOG_E("Invalid alg(0x%" LOG_PUBLIC "x)", alg);
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    uint32_t digestLen;
    HKS_IF_NOT_SUCC_LOGE_RETURN(HksGetDigestLen(alg, &digestLen),
        HKS_ERROR_INVALID_ARGUMENT, "Invalid alg(0x%" LOG_PUBLIC "x)", alg)

    if (mac->size < digestLen) {
        HKS_LOG_E("invalid mac->size(0x%" LOG_PUBLIC "x) for digestLen(0x%" LOG_PUBLIC "x)", mac->size, digestLen);
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    return HKS_SUCCESS;
}

#if defined(HKS_SUPPORT_HMAC_SHA1) || defined(HKS_SUPPORT_HMAC_SHA224) || defined(HKS_SUPPORT_HMAC_SHA256) || \
    defined(HKS_SUPPORT_HMAC_SHA384) || defined(HKS_SUPPORT_HMAC_SHA512) || defined(HKS_SUPPORT_HMAC_SM3)
int32_t HksOpensslHmac(const struct HksBlob *key, uint32_t digestAlg, const struct HksBlob *msg, struct HksBlob *mac)
{
    HKS_IF_NOT_SUCC_RETURN(HmacCheckParam(key, digestAlg, msg, mac), HKS_ERROR_INVALID_ARGUMENT)

    const EVP_MD *opensslAlg = NULL;
    if (digestAlg == HKS_DIGEST_SM3) {
        opensslAlg = EVP_sm3();
    } else {
        opensslAlg = GetOpensslAlg(digestAlg);
    }

    HKS_IF_NULL_LOGE_RETURN(opensslAlg, HKS_ERROR_CRYPTO_ENGINE_ERROR, "hmac get openssl algorithm failed")

    uint8_t *hmacData = HMAC(opensslAlg, key->data, (int32_t)key->size, msg->data, msg->size, mac->data, &mac->size);
    HKS_IF_NULL_LOGE_RETURN(hmacData, HKS_ERROR_CRYPTO_ENGINE_ERROR, "hmac process failed.")
    return HKS_SUCCESS;
}

int32_t HksOpensslHmacInit(void **cryptoCtx, const struct HksBlob *key, uint32_t digestAlg)
{
    HKS_IF_NOT_SUCC_LOGE_RETURN(HksOpensslCheckBlob(key), HKS_ERROR_INVALID_ARGUMENT, "Invalid key point")

    if ((digestAlg != HKS_DIGEST_SHA1) && (digestAlg != HKS_DIGEST_SHA224) && (digestAlg != HKS_DIGEST_SHA256) &&
        (digestAlg != HKS_DIGEST_SHA384) && (digestAlg != HKS_DIGEST_SHA512) && (digestAlg != HKS_DIGEST_SM3)) {
        HKS_LOG_E("Invalid alg(0x%" LOG_PUBLIC "x)", digestAlg);
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    const EVP_MD *opensslAlg = NULL;
    if (digestAlg == HKS_DIGEST_SM3) {
        opensslAlg = EVP_sm3();
    } else {
        opensslAlg = GetOpensslAlg(digestAlg);
    }

    HKS_IF_NULL_LOGE_RETURN(opensslAlg, HKS_ERROR_CRYPTO_ENGINE_ERROR, "hmac_init get openssl algorithm fail")

    uint32_t digestLen;
    HKS_IF_NOT_SUCC_LOGE_RETURN(HksGetDigestLen(digestAlg, &digestLen),
        HKS_ERROR_INVALID_ARGUMENT, "Invalid alg(0x%" LOG_PUBLIC "x)", digestAlg)

    HMAC_CTX *tmpCtx = NULL;
    HKS_IF_NULL_LOGE_RETURN((tmpCtx = HMAC_CTX_new()),
        HKS_ERROR_CRYPTO_ENGINE_ERROR, "initialize HksOpensslHmacCtx failed")

    if (!HMAC_Init_ex(tmpCtx, key->data, (int32_t)key->size, opensslAlg, NULL)) {
        HKS_LOG_E("openssl hmac init failed.");
        HMAC_CTX_free(tmpCtx);
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    struct HksOpensslHmacCtx *outCtx = (struct HksOpensslHmacCtx *)HksMalloc(
        sizeof(struct HksOpensslHmacCtx));

    if (outCtx == NULL) {
        HKS_LOG_E("initialize HksOpensslHmacCtx failed");
        HMAC_CTX_free(tmpCtx);
        return HKS_ERROR_MALLOC_FAIL;
    }

    outCtx->digestLen = digestLen;
    outCtx->append = (void *)tmpCtx;
    *cryptoCtx = (void *)outCtx;

    return HKS_SUCCESS;
}

int32_t HksOpensslHmacUpdate(void *cryptoCtx, const struct HksBlob *msg)
{
    HKS_IF_NOT_SUCC_LOGE_RETURN(HksOpensslCheckBlob(msg), HKS_ERROR_INVALID_ARGUMENT, "Invalid key point")

    struct HksOpensslHmacCtx *hmacCtx = (struct HksOpensslHmacCtx *)cryptoCtx;
    HKS_IF_NULL_LOGE_RETURN(hmacCtx, HKS_ERROR_NULL_POINTER, "hmacCtx invalid")

    HMAC_CTX *context = (HMAC_CTX *)hmacCtx->append;
    HKS_IF_NULL_LOGE_RETURN(context, HKS_FAILURE, "context is null")

    int hmacData = HMAC_Update(context, msg->data, msg->size);
    if (!hmacData) {
        HKS_LOG_E("hmac update failed.");
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }
    return HKS_SUCCESS;
}

int32_t HksOpensslHmacFinal(void **cryptoCtx, struct HksBlob *msg, struct HksBlob *mac)
{
    if ((cryptoCtx == NULL) || (*cryptoCtx == NULL)) {
        return HKS_ERROR_NULL_POINTER;
    }

    struct HksOpensslHmacCtx *hmacCtx = (struct HksOpensslHmacCtx *)*cryptoCtx;
    if (hmacCtx == NULL) {
        HKS_LOG_E("hmacCtx invalid");
        HKS_FREE_PTR(*cryptoCtx);
        return HKS_ERROR_NULL_POINTER;
    }

    HMAC_CTX *context = (HMAC_CTX *)hmacCtx->append;
    if (context == NULL) {
        HKS_LOG_E("context is null");
        HKS_FREE_PTR(*cryptoCtx);
        return HKS_FAILURE;
    }

    if (msg == NULL || HksOpensslCheckBlob(mac) != HKS_SUCCESS) {
        HKS_LOG_E("Invalid msg or mac point");
        HksOpensslHmacHalFreeCtx(cryptoCtx);
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    int hmacData;
    if (msg->size != 0) {
        hmacData = HMAC_Update(context, msg->data, msg->size);
        if (!hmacData) {
            HKS_LOG_E("hmac final update failed.");
            HksOpensslHmacHalFreeCtx(cryptoCtx);
            return HKS_ERROR_CRYPTO_ENGINE_ERROR;
        }
    }

    hmacData = HMAC_Final(context, mac->data, &mac->size);
    if (!hmacData) {
        HKS_LOG_E("hmac final failed.");
        HksOpensslHmacHalFreeCtx(cryptoCtx);
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    HksOpensslHmacHalFreeCtx(cryptoCtx);
    return HKS_SUCCESS;
}

void HksOpensslHmacHalFreeCtx(void **cryptoCtx)
{
    if (cryptoCtx == NULL || *cryptoCtx == NULL) {
        HKS_LOG_E("Openssl hmac free ctx is null");
        return;
    }

    struct HksOpensslHmacCtx *opensslHmacCtx = (struct HksOpensslHmacCtx *)*cryptoCtx;
    if (opensslHmacCtx->append != NULL) {
        HMAC_CTX_free((HMAC_CTX *)opensslHmacCtx->append);
        opensslHmacCtx->append = NULL;
    }

    HKS_FREE_PTR(*cryptoCtx);
}
#endif /* HKS_SUPPORT_HMAC_SHA1 */
#endif /* HKS_SUPPORT_HMAC_C */