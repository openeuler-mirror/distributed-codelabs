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

#ifdef HKS_SUPPORT_SM4_C

#include "hks_openssl_sm4.h"

#include <openssl/evp.h>
#include <openssl/ossl_typ.h>
#include <stdbool.h>
#include <stddef.h>

#include "hks_log.h"
#include "hks_mem.h"
#include "hks_openssl_aes.h"
#include "hks_openssl_common.h"
#include "hks_template.h"

#ifdef HKS_SUPPORT_SM4_GENERATE_KEY
static int32_t Sm4GenKeyCheckParam(const struct HksKeySpec *spec)
{
    if (spec->keyLen != HKS_SM4_KEY_SIZE_128) {
        HKS_LOG_E("Invlid sm4 key len %" LOG_PUBLIC "x!", spec->keyLen);
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    return HKS_SUCCESS;
}

int32_t HksOpensslSm4GenerateKey(const struct HksKeySpec *spec, struct HksBlob *key)
{
    HKS_IF_NOT_SUCC_LOGE_RETURN(Sm4GenKeyCheckParam(spec),
        HKS_ERROR_INVALID_ARGUMENT, "sm4 generate key invalid params!")

    return HksOpensslGenerateRandomKey(spec->keyLen, key);
}
#endif

static const EVP_CIPHER *GetSm4CbcCipherType(uint32_t keySize)
{
    switch (keySize) {
        case HKS_KEY_BYTES(HKS_AES_KEY_SIZE_128):
            return EVP_sm4_cbc();
        default:
            return NULL;
    }
}

static const EVP_CIPHER *GetSm4CtrCipherType(uint32_t keySize)
{
    switch (keySize) {
        case HKS_KEY_BYTES(HKS_AES_KEY_SIZE_128):
            return EVP_sm4_ctr();
        default:
            return NULL;
    }
}

static const EVP_CIPHER *GetSm4EcbCipherType(uint32_t keySize)
{
    switch (keySize) {
        case HKS_KEY_BYTES(HKS_AES_KEY_SIZE_128):
            return EVP_sm4_ecb();
        default:
            return NULL;
    }
}

static const EVP_CIPHER *GetSm4CipherType(uint32_t keySize, uint32_t mode)
{
    return GetBlockCipherType(keySize, mode, GetSm4CbcCipherType, GetSm4CtrCipherType, GetSm4EcbCipherType);
}

int32_t HksOpensslSm4EncryptInit(void **cryptoCtx, const struct HksBlob *key, const struct HksUsageSpec *usageSpec)
{
    int32_t ret;
    switch (usageSpec->mode) {
#if defined(HKS_SUPPORT_SM4_CBC_NOPADDING) || defined(HKS_SUPPORT_SM4_CBC_PKCS7) ||     \
    defined(HKS_SUPPORT_SM4_CTR_NOPADDING) || defined(HKS_SUPPORT_SM4_ECB_NOPADDING) || \
    defined(HKS_SUPPORT_SM4_ECB_PKCS7)
        case HKS_MODE_CBC:
        case HKS_MODE_CTR:
        case HKS_MODE_ECB:
            ret = OpensslBlockCipherCryptInit(key, usageSpec, true, cryptoCtx, GetSm4CipherType);
            HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret,
                "OpensslBlockCipherCryptInit for sm4 fail, ret = %" LOG_PUBLIC "d", ret)
            break;
#endif
        default:
            HKS_LOG_E("Unsupport sm4 mode! mode = 0x%" LOG_PUBLIC "x", usageSpec->mode);
            return HKS_ERROR_INVALID_ARGUMENT;
    }

    return HKS_SUCCESS;
}

int32_t HksOpensslSm4EncryptUpdate(void *cryptoCtx, const struct HksBlob *message, struct HksBlob *cipherText)
{
    HKS_IF_NULL_LOGE_RETURN(cryptoCtx, HKS_ERROR_INVALID_ARGUMENT, "cryptoCtx is NULL")

    struct HksOpensslBlockCipherCtx *context = (struct HksOpensslBlockCipherCtx *)cryptoCtx;
    uint32_t mode = context->mode;

    int32_t ret;
    switch (mode) {
#if defined(HKS_SUPPORT_SM4_CBC_NOPADDING) || defined(HKS_SUPPORT_SM4_CBC_PKCS7) ||     \
    defined(HKS_SUPPORT_SM4_CTR_NOPADDING) || defined(HKS_SUPPORT_SM4_ECB_NOPADDING) || \
    defined(HKS_SUPPORT_SM4_ECB_PKCS7)
        case HKS_MODE_CBC:
        case HKS_MODE_CTR:
        case HKS_MODE_ECB:
            ret = OpensslBlockCipherEncryptUpdate(cryptoCtx, message, cipherText);
            HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret,
                "OpensslBlockCipherEncryptUpdate for sm4 fail, ret = %" LOG_PUBLIC "d", ret)
            break;
#endif
        default:
            HKS_LOG_E("Unsupport sm4 mode! mode = 0x%" LOG_PUBLIC "x", mode);
            return HKS_ERROR_INVALID_ARGUMENT;
    }

    return HKS_SUCCESS;
}

int32_t HksOpensslSm4EncryptFinal(void **cryptoCtx, const struct HksBlob *message, struct HksBlob *cipherText,
    struct HksBlob *tagAead)
{
    (void)(tagAead); // sm4 do not support CCM or GCM, therefore tag is not used
    struct HksOpensslBlockCipherCtx *ctx = (struct HksOpensslBlockCipherCtx *)*cryptoCtx;
    uint32_t mode = ctx->mode;

    int32_t ret;
    switch (mode) {
#if defined(HKS_SUPPORT_SM4_CBC_NOPADDING) || defined(HKS_SUPPORT_SM4_CBC_PKCS7) ||     \
    defined(HKS_SUPPORT_SM4_CTR_NOPADDING) || defined(HKS_SUPPORT_SM4_ECB_NOPADDING) || \
    defined(HKS_SUPPORT_SM4_ECB_PKCS7)
        case HKS_MODE_CBC:
        case HKS_MODE_CTR:
        case HKS_MODE_ECB:
            ret = OpensslBlockCipherEncryptFinalThree(cryptoCtx, message, cipherText);
            HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret,
                "OpensslBlockCipherEncryptFinalThree for sm4 fail, ret = %" LOG_PUBLIC "d", ret)
            break;
#endif
        default:
            HKS_LOG_E("Unsupport sm4 mode! mode = 0x%" LOG_PUBLIC "x", mode);
            return HKS_ERROR_INVALID_ARGUMENT;
    }

    return HKS_SUCCESS;
}

int32_t HksOpensslSm4DecryptInit(void **cryptoCtx, const struct HksBlob *key,
    const struct HksUsageSpec *usageSpec)
{
    int32_t ret;
    switch (usageSpec->mode) {
        case HKS_MODE_CBC:
        case HKS_MODE_CTR:
        case HKS_MODE_ECB:
            ret = OpensslBlockCipherCryptInit(key, usageSpec, false, cryptoCtx, GetSm4CipherType);
            HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret,
                "OpensslBlockCipherCryptInit for sm4 fail, ret = %" LOG_PUBLIC "d", ret)
            break;
        default:
            HKS_LOG_E("Unsupport sm4 mode! mode = 0x%" LOG_PUBLIC "x", usageSpec->mode);
            return HKS_ERROR_INVALID_ARGUMENT;
    }

    return ret;
}

int32_t HksOpensslSm4DecryptUpdate(void *cryptoCtx, const struct HksBlob *message, struct HksBlob *plainText)
{
    struct HksOpensslBlockCipherCtx *contex = (struct HksOpensslBlockCipherCtx *)cryptoCtx;
    uint32_t mode = contex->mode;

    int32_t ret;
    switch (mode) {
        case HKS_MODE_CBC:
        case HKS_MODE_CTR:
        case HKS_MODE_ECB:
            ret = OpensslBlockCipherDecryptUpdate(cryptoCtx, message, plainText);
            HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret,
                "OpensslBlockCipherDecryptUpdate for sm4 fail, ret = %" LOG_PUBLIC "d", ret)
            break;
        default:
            HKS_LOG_E("Unsupport sm4 mode! mode = 0x%" LOG_PUBLIC "x", mode);
            return HKS_ERROR_INVALID_ARGUMENT;
    }

    return ret;
}

int32_t HksOpensslSm4DecryptFinal(void **cryptoCtx, const struct HksBlob *message, struct HksBlob *cipherText,
    struct HksBlob *tagAead)
{
    HksOpensslBlockCipherCtx *context = (HksOpensslBlockCipherCtx *)*cryptoCtx;
    uint32_t mode = context->mode;

    int32_t ret;
    switch (mode) {
#if defined(HKS_SUPPORT_SM4_CBC_NOPADDING) || defined(HKS_SUPPORT_SM4_CBC_PKCS7) ||     \
    defined(HKS_SUPPORT_SM4_CTR_NOPADDING) || defined(HKS_SUPPORT_SM4_ECB_NOPADDING) || \
    defined(HKS_SUPPORT_SM4_ECB_PKCS7)
        case HKS_MODE_CBC:
        case HKS_MODE_CTR:
        case HKS_MODE_ECB:
            ret = OpensslBlockCipherDecryptFinalThree(cryptoCtx, message, cipherText);
            HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret,
                "OpensslBlockCipherDecryptFinalThree for sm4 fail, ret = %" LOG_PUBLIC "d", ret)
            break;
#endif
        default:
            HKS_LOG_E("Unsupport sm4 mode! mode = 0x%" LOG_PUBLIC "x", mode);
            return HKS_ERROR_INVALID_ARGUMENT;
    }

    return HKS_SUCCESS;
}

void HksOpensslSm4HalFreeCtx(void **cryptoCtx)
{
    if (cryptoCtx == NULL || *cryptoCtx == NULL) {
        HKS_LOG_E("Openssl sm4 free ctx is null");
        return;
    }

    HksOpensslBlockCipherCtx *opensslSm4Ctx = (HksOpensslBlockCipherCtx *)*cryptoCtx;
    switch (opensslSm4Ctx->mode) {
#if defined(HKS_SUPPORT_SM4_CBC_NOPADDING) || defined(HKS_SUPPORT_SM4_CBC_PKCS7) ||     \
    defined(HKS_SUPPORT_SM4_CTR_NOPADDING) || defined(HKS_SUPPORT_SM4_ECB_NOPADDING) || \
    defined(HKS_SUPPORT_SM4_ECB_PKCS7)
        case HKS_MODE_CBC:
        case HKS_MODE_CTR:
        case HKS_MODE_ECB:
            if ((EVP_CIPHER_CTX *)opensslSm4Ctx->append != NULL) {
                EVP_CIPHER_CTX_free((EVP_CIPHER_CTX *)opensslSm4Ctx->append);
                opensslSm4Ctx->append = NULL;
            }
            break;
#endif
        default:
            HKS_LOG_E("Unsupport sm4 mode! mode = 0x%" LOG_PUBLIC "x", opensslSm4Ctx->mode);
            break;
    }

    HKS_FREE_PTR(*cryptoCtx);
}

#endif
