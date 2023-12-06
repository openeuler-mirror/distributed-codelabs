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

#include "hks_openssl_aes.h"

#include <openssl/evp.h>
#include <stddef.h>

#include "hks_log.h"
#include "hks_mem.h"
#include "hks_openssl_common.h"
#include "hks_openssl_engine.h"
#include "hks_template.h"

#if defined(HKS_SUPPORT_AES_C) || defined(HKS_SUPPORT_SM4_C)
#if defined(HKS_SUPPORT_AES_CBC_NOPADDING) || defined(HKS_SUPPORT_AES_CBC_PKCS7) ||        \
    defined(HKS_SUPPORT_AES_ECB_PKCS7PADDING) || defined(HKS_SUPPORT_AES_CTR_NOPADDING) || \
    defined(HKS_SUPPORT_AES_ECB_NOPADDING) || defined(HKS_SUPPORT_AES_GCM) ||              \
    defined(HKS_SUPPORT_SM4_CBC_NOPADDING) || defined(HKS_SUPPORT_SM4_CBC_PKCS7) ||        \
    defined(HKS_SUPPORT_SM4_CTR_NOPADDING) || defined(HKS_SUPPORT_SM4_ECB_NOPADDING) ||    \
    defined(HKS_SUPPORT_SM4_ECB_PKCS7)

const EVP_CIPHER *GetBlockCipherType(uint32_t keySize, uint32_t mode,
    const EVP_CIPHER *(*getCbcCipherType)(uint32_t keySize),
    const EVP_CIPHER *(*getCtrCipherType)(uint32_t keySize),
    const EVP_CIPHER *(*getEcbCipherType)(uint32_t keySize))
{
    if (mode == HKS_MODE_CBC) {
        return getCbcCipherType(keySize);
    } else if (mode == HKS_MODE_CTR) {
        return getCtrCipherType(keySize);
    } else if (mode == HKS_MODE_ECB) {
        return getEcbCipherType(keySize);
    }
    return NULL;
}

static int32_t OpensslBlockCipherCryptInitParams(const struct HksBlob *key, EVP_CIPHER_CTX *ctx,
    struct HksCipherParam *cipherParam, bool isEncrypt, const struct HksUsageSpec *usageSpec)
{
    int32_t ret;
    if (isEncrypt) {
        ret = EVP_EncryptInit_ex(ctx, NULL, NULL, key->data, (cipherParam == NULL) ? NULL : cipherParam->iv.data);
    } else {
        ret = EVP_DecryptInit_ex(ctx, NULL, NULL, key->data, (cipherParam == NULL) ? NULL : cipherParam->iv.data);
    }
    if (ret != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    if (usageSpec->padding == HKS_PADDING_PKCS7) {
        // set chipher padding enable
        ret = EVP_CIPHER_CTX_set_padding(ctx, OPENSSL_CTX_PADDING_ENABLE);
    } else if (usageSpec->padding == HKS_PADDING_NONE) {
        // set chipher padding none
        ret = EVP_CIPHER_CTX_set_padding(ctx, OPENSSL_CTX_PADDING_NONE);
    }
    if (ret != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }
    return HKS_SUCCESS;
}

int32_t OpensslBlockCipherCryptInit(
    const struct HksBlob *key, const struct HksUsageSpec *usageSpec, bool isEncrypt, void **cryptoCtx,
    const EVP_CIPHER *(*getCipherType)(uint32_t keySize, uint32_t mode))
{
    int32_t ret;
    struct HksCipherParam *cipherParam = (struct HksCipherParam *)usageSpec->algParam;

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL) {
        HksLogOpensslError();
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    const EVP_CIPHER *cipher = getCipherType(key->size, usageSpec->mode);
    if (cipher == NULL) {
        EVP_CIPHER_CTX_free(ctx);
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    if (isEncrypt) {
        ret = EVP_EncryptInit_ex(ctx, cipher, NULL, NULL, NULL);
    } else {
        ret = EVP_DecryptInit_ex(ctx, cipher, NULL, NULL, NULL);
    }
    if (ret != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        EVP_CIPHER_CTX_free(ctx);
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    ret = OpensslBlockCipherCryptInitParams(key, ctx, cipherParam, isEncrypt, usageSpec);
    if (ret != HKS_SUCCESS) {
        EVP_CIPHER_CTX_free(ctx);
        HKS_LOG_E("OpensslBlockCipherCryptInitParams fail, ret = %" LOG_PUBLIC "d", ret);
        return ret;
    }

    struct HksOpensslBlockCipherCtx *outCtx =
        (struct HksOpensslBlockCipherCtx *)HksMalloc(sizeof(HksOpensslBlockCipherCtx));
    if (outCtx == NULL) {
        EVP_CIPHER_CTX_free(ctx);
        return HKS_ERROR_MALLOC_FAIL;
    }
    outCtx->algType = usageSpec->algType;
    outCtx->mode = usageSpec->mode;
    outCtx->padding = usageSpec->padding;
    outCtx->append = (void *)ctx;

    *cryptoCtx = (void *)outCtx;

    return HKS_SUCCESS;
}

// blob data and size have been checked in hks_core_service_three_stage.c
int32_t OpensslBlockCipherEncryptUpdate(
    void *cryptoCtx, const struct HksBlob *message, struct HksBlob *cipherText)
{
    struct HksOpensslBlockCipherCtx *blockCipherCtx = (struct HksOpensslBlockCipherCtx *)cryptoCtx;
    EVP_CIPHER_CTX *ctx = (EVP_CIPHER_CTX *)blockCipherCtx->append;

    HKS_IF_NULL_RETURN(ctx, HKS_ERROR_NULL_POINTER)

    int32_t outLen = 0;
    if (EVP_EncryptUpdate(ctx, cipherText->data, &outLen, message->data, message->size) != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }
    cipherText->size = (uint32_t)outLen;

    return HKS_SUCCESS;
}

// blob data and size have been checked in hks_core_service_three_stage.c
int32_t OpensslBlockCipherEncryptFinalThree(
    void **cryptoCtx, const struct HksBlob *message, struct HksBlob *cipherText)
{
    struct HksOpensslBlockCipherCtx *blockCipherCtx = (struct HksOpensslBlockCipherCtx *)*cryptoCtx;
    EVP_CIPHER_CTX *ctx = (EVP_CIPHER_CTX *)blockCipherCtx->append;

    if (ctx == NULL) {
        HKS_FREE_PTR(*cryptoCtx);
        return HKS_ERROR_NULL_POINTER;
    }

    int32_t ret = HKS_SUCCESS;
    do {
        int32_t outLen = 0;
        if (message->size != 0) {
            if (EVP_EncryptUpdate(ctx, cipherText->data, &outLen, message->data,
                message->size) != HKS_OPENSSL_SUCCESS) {
                HksLogOpensslError();
                ret = HKS_ERROR_CRYPTO_ENGINE_ERROR;
                break;
            }
        }
        cipherText->size = (uint32_t)outLen;

        if (EVP_EncryptFinal_ex(ctx, (cipherText->data + outLen), &outLen) != HKS_OPENSSL_SUCCESS) {
            HksLogOpensslError();
            ret = HKS_ERROR_CRYPTO_ENGINE_ERROR;
            break;
        }
        cipherText->size += (uint32_t)outLen;
    } while (0);

    EVP_CIPHER_CTX_free(ctx);
    blockCipherCtx->append = NULL;
    HKS_FREE_PTR(*cryptoCtx);

    return ret;
}

// blob data and size have been checked in hks_core_service_three_stage.c
int32_t OpensslBlockCipherDecryptUpdate(
    void *cryptoCtx, const struct HksBlob *message, struct HksBlob *plainText)
{
    struct HksOpensslBlockCipherCtx *blockCipherCtx = (struct HksOpensslBlockCipherCtx *)cryptoCtx;
    EVP_CIPHER_CTX *ctx = (EVP_CIPHER_CTX *)blockCipherCtx->append;

    HKS_IF_NULL_RETURN(ctx, HKS_ERROR_NULL_POINTER)

    int32_t outLen = 0;
    if (EVP_DecryptUpdate(ctx, plainText->data, &outLen, message->data, message->size) != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }
    plainText->size = (uint32_t)outLen;

    return HKS_SUCCESS;
}

// blob data and size have been checked in hks_core_service_three_stage.c
int32_t OpensslBlockCipherDecryptFinalThree(
    void **cryptoCtx, const struct HksBlob *message, struct HksBlob *plainText)
{
    struct HksOpensslBlockCipherCtx *blockCipherCtx = (struct HksOpensslBlockCipherCtx *)*cryptoCtx;
    EVP_CIPHER_CTX *ctx = (EVP_CIPHER_CTX *)blockCipherCtx->append;
    if (ctx == NULL) {
        HKS_FREE_PTR(*cryptoCtx);
        return HKS_ERROR_NULL_POINTER;
    }

    int32_t ret = HKS_SUCCESS;
    do {
        int32_t outLen = 0;
        if (message->size != 0) {
            if (EVP_DecryptUpdate(ctx, plainText->data, &outLen, message->data, message->size) != HKS_OPENSSL_SUCCESS) {
                HksLogOpensslError();
                ret = HKS_ERROR_CRYPTO_ENGINE_ERROR;
                break;
            }
        }
        plainText->size = (uint32_t)outLen;

        if (EVP_DecryptFinal_ex(ctx, plainText->data + outLen, &outLen) != HKS_OPENSSL_SUCCESS) {
            HksLogOpensslError();
            ret = HKS_ERROR_CRYPTO_ENGINE_ERROR;
            break;
        }
        plainText->size += (uint32_t)outLen;
    } while (0);

    EVP_CIPHER_CTX_free(ctx);
    blockCipherCtx->append = NULL;
    HKS_FREE_PTR(*cryptoCtx);
    return ret;
}
#endif
#endif /* defined(HKS_SUPPORT_AES_C) || defined(HKS_SUPPORT_SM4_C) */

#ifdef HKS_SUPPORT_AES_C
#ifdef HKS_SUPPORT_AES_GENERATE_KEY
static int32_t AesGenKeyCheckParam(const struct HksKeySpec *spec)
{
    if ((spec->keyLen != HKS_AES_KEY_SIZE_128) && (spec->keyLen != HKS_AES_KEY_SIZE_192) &&
        (spec->keyLen != HKS_AES_KEY_SIZE_256)) {
        HKS_LOG_E("Invlid aes key len %" LOG_PUBLIC "x!", spec->keyLen);
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    return HKS_SUCCESS;
}

int32_t HksOpensslAesGenerateKey(const struct HksKeySpec *spec, struct HksBlob *key)
{
    HKS_IF_NOT_SUCC_LOGE_RETURN(AesGenKeyCheckParam(spec), HKS_ERROR_INVALID_ARGUMENT,
        "aes generate key invalid params!")

    return HksOpensslGenerateRandomKey(spec->keyLen, key);
}
#endif

static const EVP_CIPHER *GetAesCbcCipherType(uint32_t keySize)
{
    switch (keySize) {
        case HKS_KEY_BYTES(HKS_AES_KEY_SIZE_128):
            return EVP_aes_128_cbc();
        case HKS_KEY_BYTES(HKS_AES_KEY_SIZE_192):
            return EVP_aes_192_cbc();
        case HKS_KEY_BYTES(HKS_AES_KEY_SIZE_256):
            return EVP_aes_256_cbc();
        default:
            return NULL;
    }
}

static const EVP_CIPHER *GetAesCtrCipherType(uint32_t keySize)
{
    switch (keySize) {
        case HKS_KEY_BYTES(HKS_AES_KEY_SIZE_128):
            return EVP_aes_128_ctr();
        case HKS_KEY_BYTES(HKS_AES_KEY_SIZE_192):
            return EVP_aes_192_ctr();
        case HKS_KEY_BYTES(HKS_AES_KEY_SIZE_256):
            return EVP_aes_256_ctr();
        default:
            return NULL;
    }
}

static const EVP_CIPHER *GetAesEcbCipherType(uint32_t keySize)
{
    switch (keySize) {
        case HKS_KEY_BYTES(HKS_AES_KEY_SIZE_128):
            return EVP_aes_128_ecb();
        case HKS_KEY_BYTES(HKS_AES_KEY_SIZE_192):
            return EVP_aes_192_ecb();
        case HKS_KEY_BYTES(HKS_AES_KEY_SIZE_256):
            return EVP_aes_256_ecb();
        default:
            return NULL;
    }
}

static const EVP_CIPHER *GetGcmCipherType(uint32_t keySize)
{
    switch (keySize) {
        case (HKS_AES_KEY_SIZE_128 / HKS_BITS_PER_BYTE):
            return EVP_aes_128_gcm();
        case (HKS_AES_KEY_SIZE_192 / HKS_BITS_PER_BYTE):
            return EVP_aes_192_gcm();
        case (HKS_AES_KEY_SIZE_256 / HKS_BITS_PER_BYTE):
            return EVP_aes_256_gcm();
        default:
            return NULL;
    }
}

static const EVP_CIPHER *GetAesCipherType(uint32_t keySize, uint32_t mode)
{
    return GetBlockCipherType(keySize, mode, GetAesCbcCipherType, GetAesCtrCipherType, GetAesEcbCipherType);
}

static const EVP_CIPHER *GetAeadCipherType(uint32_t keySize, uint32_t mode)
{
    if (mode == HKS_MODE_GCM) {
        return GetGcmCipherType(keySize);
    }
    return NULL;
}

#ifdef HKS_SUPPORT_AES_GCM
static int32_t OpensslAesAeadInit(
    const struct HksBlob *key, const struct HksUsageSpec *usageSpec, bool isEncrypt, EVP_CIPHER_CTX **ctx)
{
    int32_t ret;
    struct HksAeadParam *aeadParam = (struct HksAeadParam *)usageSpec->algParam;
    HKS_IF_NULL_RETURN(aeadParam, HKS_ERROR_INVALID_ARGUMENT)

    *ctx = EVP_CIPHER_CTX_new();
    if (*ctx == NULL) {
        HksLogOpensslError();
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    if (isEncrypt) {
        ret = EVP_EncryptInit_ex(*ctx, GetAeadCipherType(key->size, usageSpec->mode), NULL, NULL, NULL);
    } else {
        ret = EVP_DecryptInit_ex(*ctx, GetAeadCipherType(key->size, usageSpec->mode), NULL, NULL, NULL);
    }
    if (ret != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        EVP_CIPHER_CTX_free(*ctx);
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    ret = EVP_CIPHER_CTX_ctrl(*ctx, EVP_CTRL_AEAD_SET_IVLEN, aeadParam->nonce.size, NULL);
    if (ret != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        EVP_CIPHER_CTX_free(*ctx);
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    if (isEncrypt) {
        ret = EVP_EncryptInit_ex(*ctx, NULL, NULL, key->data, aeadParam->nonce.data);
    } else {
        ret = EVP_DecryptInit_ex(*ctx, NULL, NULL, key->data, aeadParam->nonce.data);
    }
    if (ret != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        EVP_CIPHER_CTX_free(*ctx);
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    return HKS_SUCCESS;
}

static int32_t OpensslAesAeadEncryptFinal(EVP_CIPHER_CTX *ctx, const struct HksUsageSpec *usageSpec,
    const struct HksBlob *message, struct HksBlob *cipherText, struct HksBlob *tagAead)
{
    int outLen = 0;
    struct HksAeadParam *aeadParam = (struct HksAeadParam *)usageSpec->algParam;

    if (EVP_EncryptUpdate(ctx, NULL, &outLen, aeadParam->aad.data, aeadParam->aad.size) != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        EVP_CIPHER_CTX_free(ctx);
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    if (EVP_EncryptUpdate(ctx, cipherText->data, &outLen, message->data, message->size) != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        EVP_CIPHER_CTX_free(ctx);
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }
    cipherText->size = (uint32_t)outLen;

    if (EVP_EncryptFinal_ex(ctx, cipherText->data, &outLen) != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        EVP_CIPHER_CTX_free(ctx);
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_GET_TAG, HKS_AE_TAG_LEN, tagAead->data) != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        EVP_CIPHER_CTX_free(ctx);
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    EVP_CIPHER_CTX_free(ctx);
    return HKS_SUCCESS;
}

static int32_t OpensslAesAeadDecryptFinal(
    EVP_CIPHER_CTX *ctx, const struct HksUsageSpec *usageSpec, const struct HksBlob *message, struct HksBlob *plainText)
{
    int outLen = 0;
    struct HksAeadParam *aeadParam = (struct HksAeadParam *)usageSpec->algParam;

    if (EVP_DecryptUpdate(ctx, NULL, &outLen, aeadParam->aad.data, aeadParam->aad.size) != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        EVP_CIPHER_CTX_free(ctx);
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    if (EVP_DecryptUpdate(ctx, plainText->data, &outLen, message->data, message->size) != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        EVP_CIPHER_CTX_free(ctx);
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }
    plainText->size = (uint32_t)outLen;

    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_TAG, aeadParam->tagDec.size, aeadParam->tagDec.data) !=
        HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        EVP_CIPHER_CTX_free(ctx);
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    if (EVP_DecryptFinal_ex(ctx, plainText->data, &outLen) != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        EVP_CIPHER_CTX_free(ctx);
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    EVP_CIPHER_CTX_free(ctx);
    return HKS_SUCCESS;
}

static int32_t OpensslAesAeadCryptSetParam(const struct HksBlob *key, struct HksAeadParam *aeadParam,
    bool isEncrypt, EVP_CIPHER_CTX *ctx)
{
    if (aeadParam == NULL || key == NULL) {
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    int32_t ret = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_IVLEN, aeadParam->nonce.size, NULL);
    if (ret != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    if (isEncrypt) {
        ret = EVP_EncryptInit_ex(ctx, NULL, NULL, key->data, aeadParam->nonce.data);
    } else {
        ret = EVP_DecryptInit_ex(ctx, NULL, NULL, key->data, aeadParam->nonce.data);
    }
    if (ret != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    return HKS_SUCCESS;
}


static int32_t OpensslAesAeadCryptInit(
    const struct HksBlob *key, const struct HksUsageSpec *usageSpec, bool isEncrypt, void **cryptoCtx)
{
    int32_t ret;
    int outLen = 0;
    struct HksAeadParam *aeadParam = (struct HksAeadParam *)usageSpec->algParam;
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL) {
        HksLogOpensslError();
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    if (isEncrypt) {
        ret = EVP_EncryptInit_ex(ctx, GetAeadCipherType(key->size, usageSpec->mode), NULL, NULL, NULL);
    } else {
        ret = EVP_DecryptInit_ex(ctx, GetAeadCipherType(key->size, usageSpec->mode), NULL, NULL, NULL);
    }
    if (ret != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        EVP_CIPHER_CTX_free(ctx);
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    ret = OpensslAesAeadCryptSetParam(key, aeadParam, isEncrypt, ctx);
    if (ret != HKS_SUCCESS) {
        HksLogOpensslError();
        EVP_CIPHER_CTX_free(ctx);
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    if (isEncrypt) {
        ret = EVP_EncryptUpdate(ctx, NULL, &outLen, aeadParam->aad.data, aeadParam->aad.size);
    } else {
        ret = EVP_DecryptUpdate(ctx, NULL, &outLen, aeadParam->aad.data, aeadParam->aad.size);
    }
    if (ret != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        HKS_LOG_E("update aad faild, outLen->%" LOG_PUBLIC "d", outLen);
        EVP_CIPHER_CTX_free(ctx);
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    struct HksOpensslBlockCipherCtx *outCtx =
        (struct HksOpensslBlockCipherCtx *)HksMalloc(sizeof(HksOpensslBlockCipherCtx));
    if (outCtx == NULL) {
        HKS_LOG_E("HksOpensslBlockCipherCtx malloc fail");
        EVP_CIPHER_CTX_free(ctx);
        return HKS_ERROR_MALLOC_FAIL;
    }
    outCtx->algType = usageSpec->algType;
    outCtx->mode = usageSpec->mode;
    outCtx->append = (void *)ctx;

    *cryptoCtx = (void *)outCtx;

    return HKS_SUCCESS;
}

static int32_t OpensslAesAeadEnryptUpdate(void *cryptoCtx, const struct HksBlob *message,
    struct HksBlob *cipherText)
{
    struct HksOpensslBlockCipherCtx *aesCtx = (struct HksOpensslBlockCipherCtx *)cryptoCtx;
    EVP_CIPHER_CTX *ctx = (EVP_CIPHER_CTX *)aesCtx->append;

    int32_t outLen = 0;

    if (EVP_EncryptUpdate(ctx, cipherText->data, &outLen, message->data, message->size) != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }
    cipherText->size = (uint32_t)outLen;

    return HKS_SUCCESS;
}

static int32_t OpensslAesAeadDecryptUpdate(void *cryptoCtx,
    const struct HksBlob *message, struct HksBlob *plainText)
{
    struct HksOpensslBlockCipherCtx *aesCtx = (struct HksOpensslBlockCipherCtx *)cryptoCtx;
    EVP_CIPHER_CTX *ctx = (EVP_CIPHER_CTX *)aesCtx->append;
    int32_t outLen = 0;

    if (EVP_DecryptUpdate(ctx, plainText->data, &outLen, message->data, message->size) != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    plainText->size = (uint32_t)outLen;

    return HKS_SUCCESS;
}

static int32_t OpensslAesAeadEncryptFinalGCM(void **cryptoCtx, const struct HksBlob *message,
    struct HksBlob *cipherText, struct HksBlob *tagAead)
{
    struct HksOpensslBlockCipherCtx *aesCtx = (struct HksOpensslBlockCipherCtx *)*cryptoCtx;
    EVP_CIPHER_CTX *ctx = (EVP_CIPHER_CTX *)aesCtx->append;

    if (ctx == NULL) {
        HKS_FREE_PTR(*cryptoCtx);
        return HKS_ERROR_NULL_POINTER;
    }

    int32_t ret = HKS_SUCCESS;
    do {
        int32_t outLen = 0;
        if (message->size != 0) {
            if (EVP_EncryptUpdate(ctx, cipherText->data, &outLen, message->data,
                message->size) != HKS_OPENSSL_SUCCESS) {
                HksLogOpensslError();
                HKS_LOG_E("EVP_EncryptUpdate cipherText data faild, outLen->%" LOG_PUBLIC "d", outLen);
                ret = HKS_ERROR_CRYPTO_ENGINE_ERROR;
                break;
            }
        }
        cipherText->size = (uint32_t)outLen;

        if (EVP_EncryptFinal_ex(ctx, cipherText->data + outLen, &outLen) != HKS_OPENSSL_SUCCESS) {
            HksLogOpensslError();
            HKS_LOG_E("EVP_EncryptFinal_ex faild, outLen->%" LOG_PUBLIC "d", outLen);
            ret = HKS_ERROR_CRYPTO_ENGINE_ERROR;
            break;
        }
        cipherText->size += (uint32_t)outLen;
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_GET_TAG, HKS_AE_TAG_LEN, tagAead->data) != HKS_OPENSSL_SUCCESS) {
            HksLogOpensslError();
            ret = HKS_ERROR_CRYPTO_ENGINE_ERROR;
            break;
        }
    } while (0);

    EVP_CIPHER_CTX_free(ctx);
    aesCtx->append = NULL;
    HKS_FREE_PTR(*cryptoCtx);

    return ret;
}

static int32_t OpensslAesAeadDecryptFinalGCM(void **cryptoCtx, const struct HksBlob *message,
    struct HksBlob *plainText, struct HksBlob *tagAead)
{
    struct HksOpensslBlockCipherCtx *aesCtx = (struct HksOpensslBlockCipherCtx *)*cryptoCtx;
    EVP_CIPHER_CTX *ctx = (EVP_CIPHER_CTX *)aesCtx->append;

    if (ctx == NULL) {
        HksFree(aesCtx);
        *cryptoCtx = NULL;
        return HKS_ERROR_NULL_POINTER;
    }

    int32_t ret = HKS_SUCCESS;
    do {
        int32_t outLen = 0;
        if (message->size != 0) {
            if (EVP_DecryptUpdate(ctx, plainText->data, &outLen, message->data, message->size) !=
                HKS_OPENSSL_SUCCESS) {
                HksLogOpensslError();
                HKS_LOG_E("EVP_DecryptUpdate plainText data faild, outLen->%" LOG_PUBLIC "d", outLen);
                ret = HKS_ERROR_CRYPTO_ENGINE_ERROR;
                break;
            }
        }
        plainText->size = (uint32_t)outLen;

        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_TAG, tagAead->size, tagAead->data) !=
            HKS_OPENSSL_SUCCESS) {
            HKS_LOG_E("EVP_CIPHER_CTX_ctrl failed, tagAead->size->%" LOG_PUBLIC "d", tagAead->size);
            HksLogOpensslError();
            ret = HKS_ERROR_CRYPTO_ENGINE_ERROR;
            break;
        }

        if (EVP_DecryptFinal_ex(ctx, plainText->data + outLen, &outLen) != HKS_OPENSSL_SUCCESS) {
            HksLogOpensslError();
            HKS_LOG_E("EVP_DecryptFinal_ex faild, outLen->%" LOG_PUBLIC "d", outLen);
            ret = HKS_ERROR_CRYPTO_ENGINE_ERROR;
            break;
        }
        plainText->size += (uint32_t)outLen;
    } while (0);

    EVP_CIPHER_CTX_free(ctx);
    aesCtx->append = NULL;
    HKS_FREE_PTR(*cryptoCtx);
    return ret;
}
#endif

#if defined(HKS_SUPPORT_AES_CBC_NOPADDING) || defined(HKS_SUPPORT_AES_CBC_PKCS7) ||     \
    defined(HKS_SUPPORT_AES_CTR_NOPADDING) || defined(HKS_SUPPORT_AES_ECB_NOPADDING) || \
    defined(HKS_SUPPORT_AES_ECB_PKCS7PADDING)
static int32_t OpensslAesCipherInit(
    const struct HksBlob *key, const struct HksUsageSpec *usageSpec, bool isEncrypt, EVP_CIPHER_CTX **ctx)
{
    int32_t ret;
    struct HksCipherParam *cipherParam = (struct HksCipherParam *)usageSpec->algParam;

    *ctx = EVP_CIPHER_CTX_new();
    if (*ctx == NULL) {
        HksLogOpensslError();
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    const EVP_CIPHER *cipher = GetAesCipherType(key->size, usageSpec->mode);
    if (cipher == NULL) {
        EVP_CIPHER_CTX_free(*ctx);
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    if (isEncrypt) {
        ret = EVP_EncryptInit_ex(*ctx, cipher, NULL, NULL, NULL);
    } else {
        ret = EVP_DecryptInit_ex(*ctx, cipher, NULL, NULL, NULL);
    }
    if (ret != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        EVP_CIPHER_CTX_free(*ctx);
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    if (isEncrypt) {
        ret = EVP_EncryptInit_ex(*ctx, NULL, NULL, key->data, (cipherParam == NULL) ? NULL : cipherParam->iv.data);
    } else {
        ret = EVP_DecryptInit_ex(*ctx, NULL, NULL, key->data, (cipherParam == NULL) ? NULL : cipherParam->iv.data);
    }
    if (ret != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        EVP_CIPHER_CTX_free(*ctx);
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    if (usageSpec->padding == HKS_PADDING_PKCS7) {
        ret = EVP_CIPHER_CTX_set_padding(*ctx, OPENSSL_CTX_PADDING_ENABLE);
    } else if (usageSpec->padding == HKS_PADDING_NONE) {
        ret = EVP_CIPHER_CTX_set_padding(*ctx, OPENSSL_CTX_PADDING_NONE);
    }
    if (ret != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        EVP_CIPHER_CTX_free(*ctx);
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    return HKS_SUCCESS;
}

static int32_t OpensslAesCipherEncryptFinal(
    EVP_CIPHER_CTX *ctx, const struct HksBlob *message, struct HksBlob *cipherText)
{
    int32_t outLen = 0;

    if (EVP_EncryptUpdate(ctx, cipherText->data, &outLen, message->data, message->size) != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        EVP_CIPHER_CTX_free(ctx);
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }
    cipherText->size = (uint32_t)outLen;

    if (EVP_EncryptFinal_ex(ctx, cipherText->data + outLen, &outLen) != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        EVP_CIPHER_CTX_free(ctx);
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }
    cipherText->size += (uint32_t)outLen;

    EVP_CIPHER_CTX_free(ctx);
    return HKS_SUCCESS;
}

static int32_t OpensslAesCipherDecryptFinal(
    EVP_CIPHER_CTX *ctx, const struct HksBlob *message, struct HksBlob *plainText)
{
    int32_t outLen = 0;

    if (EVP_DecryptUpdate(ctx, plainText->data, &outLen, message->data, message->size) != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        EVP_CIPHER_CTX_free(ctx);
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }
    plainText->size = (uint32_t)outLen;

    if (EVP_DecryptFinal_ex(ctx, plainText->data + outLen, &outLen) != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        EVP_CIPHER_CTX_free(ctx);
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }
    plainText->size += (uint32_t)outLen;

    EVP_CIPHER_CTX_free(ctx);
    return HKS_SUCCESS;
}
#endif

int32_t HksOpensslAesEncryptInit(void **cryptoCtx, const struct HksBlob *key, const struct HksUsageSpec *usageSpec)
{
    int32_t ret;
    switch (usageSpec->mode) {
#ifdef HKS_SUPPORT_AES_GCM
        case HKS_MODE_GCM:
            ret = OpensslAesAeadCryptInit(key, usageSpec, true, cryptoCtx);
            HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "OpensslAesAeadInit fail, ret = %" LOG_PUBLIC "d", ret)
            break;
#endif
#if defined(HKS_SUPPORT_AES_CBC_NOPADDING) || defined(HKS_SUPPORT_AES_CBC_PKCS7) ||     \
    defined(HKS_SUPPORT_AES_CTR_NOPADDING) || defined(HKS_SUPPORT_AES_ECB_NOPADDING) || \
    defined(HKS_SUPPORT_AES_ECB_PKCS7PADDING)
        case HKS_MODE_CBC:
        case HKS_MODE_CTR:
        case HKS_MODE_ECB:
            ret = OpensslBlockCipherCryptInit(key, usageSpec, true, cryptoCtx, GetAesCipherType);
            HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret,
                "OpensslBlockCipherCryptInit for aes fail, ret = %" LOG_PUBLIC "d", ret)
            break;
#endif
        default:
            HKS_LOG_E("Unsupport aes mode! mode = 0x%" LOG_PUBLIC "x", usageSpec->mode);
            return HKS_ERROR_INVALID_ARGUMENT;
    }

    return HKS_SUCCESS;
}

int32_t HksOpensslAesEncryptUpdate(void *cryptoCtx, const struct HksBlob *message, struct HksBlob *cipherText)
{
    struct HksOpensslBlockCipherCtx *contex = (struct HksOpensslBlockCipherCtx *)cryptoCtx;
    uint32_t mode = contex->mode;

    int32_t ret;
    switch (mode) {
#ifdef HKS_SUPPORT_AES_GCM
        case HKS_MODE_GCM:
            ret = OpensslAesAeadEnryptUpdate(cryptoCtx, message, cipherText);
            HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret,
                "OpensslAesAeadEnryptUpdate for aes fail, ret = %" LOG_PUBLIC "d", ret)
            break;
#endif
#if defined(HKS_SUPPORT_AES_CBC_NOPADDING) || defined(HKS_SUPPORT_AES_CBC_PKCS7) ||     \
    defined(HKS_SUPPORT_AES_CTR_NOPADDING) || defined(HKS_SUPPORT_AES_ECB_NOPADDING) || \
    defined(HKS_SUPPORT_AES_ECB_PKCS7PADDING)
        case HKS_MODE_CBC:
        case HKS_MODE_CTR:
        case HKS_MODE_ECB:
            ret = OpensslBlockCipherEncryptUpdate(cryptoCtx, message, cipherText);
            HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret,
                "OpensslBlockCipherEncryptUpdate for aes fail, ret = %" LOG_PUBLIC "d", ret)
            break;
#endif
        default:
            HKS_LOG_E("Unsupport aes mode! mode = 0x%" LOG_PUBLIC "x", mode);
            return HKS_ERROR_INVALID_ARGUMENT;
    }

    return HKS_SUCCESS;
}

int32_t HksOpensslAesEncryptFinal(void **cryptoCtx, const struct HksBlob *message, struct HksBlob *cipherText,
    struct HksBlob *tagAead)
{
    struct HksOpensslBlockCipherCtx *contex = (struct HksOpensslBlockCipherCtx *)*cryptoCtx;
    uint32_t mode = contex->mode;

    int32_t ret;
    switch (mode) {
#ifdef HKS_SUPPORT_AES_GCM
        case HKS_MODE_GCM:
            ret = OpensslAesAeadEncryptFinalGCM(cryptoCtx, message, cipherText, tagAead);
            HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret,
                "OpensslAesAeadEncryptFinalGCM for aes fail, ret = %" LOG_PUBLIC "d", ret)
            break;
#endif
#if defined(HKS_SUPPORT_AES_CBC_NOPADDING) || defined(HKS_SUPPORT_AES_CBC_PKCS7) ||     \
    defined(HKS_SUPPORT_AES_CTR_NOPADDING) || defined(HKS_SUPPORT_AES_ECB_NOPADDING) || \
    defined(HKS_SUPPORT_AES_ECB_PKCS7PADDING)
        case HKS_MODE_CBC:
        case HKS_MODE_CTR:
        case HKS_MODE_ECB:
            ret = OpensslBlockCipherEncryptFinalThree(cryptoCtx, message, cipherText);
            HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret,
                "OpensslBlockCipherEncryptFinalThree for aes fail, ret = %" LOG_PUBLIC "d", ret)
            break;
#endif
        default:
            HKS_LOG_E("Unsupport aes mode! mode = 0x%" LOG_PUBLIC "x", mode);
            return HKS_ERROR_INVALID_ARGUMENT;
    }

    return HKS_SUCCESS;
}

int32_t HksOpensslAesDecryptInit(void **cryptoCtx, const struct HksBlob *key,
    const struct HksUsageSpec *usageSpec)
{
    int32_t ret;
    switch (usageSpec->mode) {
        case HKS_MODE_GCM:
            ret = OpensslAesAeadCryptInit(key, usageSpec, false, cryptoCtx);
            HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret,
                "OpensslAesAeadDecryptInit for aes fail, ret = %" LOG_PUBLIC "d", ret)
            break;
        case HKS_MODE_CBC:
        case HKS_MODE_CTR:
        case HKS_MODE_ECB:
            ret = OpensslBlockCipherCryptInit(key, usageSpec, false, cryptoCtx, GetAesCipherType);
            HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret,
                "OpensslBlockCipherCryptInit for aes fail, ret = %" LOG_PUBLIC "d", ret)
            break;
        default:
            HKS_LOG_E("Unsupport aes mode! mode = 0x%" LOG_PUBLIC "x", usageSpec->mode);
            return HKS_ERROR_INVALID_ARGUMENT;
    }

    return ret;
}

int32_t HksOpensslAesDecryptUpdate(void *cryptoCtx, const struct HksBlob *message, struct HksBlob *plainText)
{
    struct HksOpensslBlockCipherCtx *contex = (struct HksOpensslBlockCipherCtx *)cryptoCtx;
    uint32_t mode = contex->mode;

    int32_t ret;
    switch (mode) {
        case HKS_MODE_GCM:
            ret = OpensslAesAeadDecryptUpdate(cryptoCtx, message, plainText);
            HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret,
                "OpensslAesAeadDecryptUpdate for aes fail, ret = %" LOG_PUBLIC "d", ret)
            break;
        case HKS_MODE_CBC:
        case HKS_MODE_CTR:
        case HKS_MODE_ECB:
            ret = OpensslBlockCipherDecryptUpdate(cryptoCtx, message, plainText);
            HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret,
                "OpensslBlockCipherDecryptUpdate for aes fail, ret = %" LOG_PUBLIC "d", ret)
            break;
        default:
            HKS_LOG_E("Unsupport aes mode! mode = 0x%" LOG_PUBLIC "x", mode);
            return HKS_ERROR_INVALID_ARGUMENT;
    }

    return ret;
}

int32_t HksOpensslAesDecryptFinal(void **cryptoCtx, const struct HksBlob *message, struct HksBlob *cipherText,
    struct HksBlob *tagAead)
{
    struct HksOpensslBlockCipherCtx *contex = (struct HksOpensslBlockCipherCtx *)*cryptoCtx;
    uint32_t mode = contex->mode;

    int32_t ret;
    switch (mode) {
#ifdef HKS_SUPPORT_AES_GCM
        case HKS_MODE_GCM:
            ret = OpensslAesAeadDecryptFinalGCM(cryptoCtx, message, cipherText, tagAead);
            HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret,
                "OpensslAesAeadDecryptFinalGCM for aes fail, ret = %" LOG_PUBLIC "d", ret)
            break;
#endif
#if defined(HKS_SUPPORT_AES_CBC_NOPADDING) || defined(HKS_SUPPORT_AES_CBC_PKCS7) ||     \
    defined(HKS_SUPPORT_AES_CTR_NOPADDING) || defined(HKS_SUPPORT_AES_ECB_NOPADDING) || \
    defined(HKS_SUPPORT_AES_ECB_PKCS7PADDING)
        case HKS_MODE_CBC:
        case HKS_MODE_CTR:
        case HKS_MODE_ECB:
            ret = OpensslBlockCipherDecryptFinalThree(cryptoCtx, message, cipherText);
            HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret,
                "OpensslBlockCipherDecryptFinalThree for aes fail, ret = %" LOG_PUBLIC "d", ret)
            break;
#endif
        default:
            HKS_LOG_E("Unsupport aes mode! mode = 0x%" LOG_PUBLIC "x", mode);
            return HKS_ERROR_INVALID_ARGUMENT;
    }

    return HKS_SUCCESS;
}

void HksOpensslAesHalFreeCtx(void **cryptoCtx)
{
    if (cryptoCtx == NULL || *cryptoCtx == NULL) {
        HKS_LOG_E("Openssl aes free ctx is null");
        return;
    }

    struct HksOpensslBlockCipherCtx *opensslAesCtx = (struct HksOpensslBlockCipherCtx *)*cryptoCtx;
    switch (opensslAesCtx->mode) {
#ifdef HKS_SUPPORT_AES_GCM
        case HKS_MODE_GCM:
            if ((EVP_CIPHER_CTX *)opensslAesCtx->append != NULL) {
                EVP_CIPHER_CTX_free((EVP_CIPHER_CTX *)opensslAesCtx->append);
                opensslAesCtx->append = NULL;
            }
            break;
#endif
#if defined(HKS_SUPPORT_AES_CBC_NOPADDING) || defined(HKS_SUPPORT_AES_CBC_PKCS7) ||     \
    defined(HKS_SUPPORT_AES_CTR_NOPADDING) || defined(HKS_SUPPORT_AES_ECB_NOPADDING) || \
    defined(HKS_SUPPORT_AES_ECB_PKCS7PADDING)
        case HKS_MODE_CBC:
        case HKS_MODE_CTR:
        case HKS_MODE_ECB:
            if ((EVP_CIPHER_CTX *)opensslAesCtx->append != NULL) {
                EVP_CIPHER_CTX_free((EVP_CIPHER_CTX *)opensslAesCtx->append);
                opensslAesCtx->append = NULL;
            }
            break;
#endif
        default:
            HKS_LOG_E("Unsupport aes mode! mode = 0x%" LOG_PUBLIC "x", opensslAesCtx->mode);
            break;
    }

    HKS_FREE_PTR(*cryptoCtx);
}

int32_t HksOpensslAesEncrypt(const struct HksBlob *key, const struct HksUsageSpec *usageSpec,
    const struct HksBlob *message, struct HksBlob *cipherText, struct HksBlob *tagAead)
{
    EVP_CIPHER_CTX *ctx = NULL;
    struct HksBlob tmpCipherText = *cipherText;

    int32_t ret;
    switch (usageSpec->mode) {
#ifdef HKS_SUPPORT_AES_GCM
        case HKS_MODE_GCM:
            ret = OpensslAesAeadInit(key, usageSpec, true, &ctx);
            HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret,
                "OpensslAesAeadInit for aes fail, ret = %" LOG_PUBLIC "d", ret)

            ret = OpensslAesAeadEncryptFinal(ctx, usageSpec, message, &tmpCipherText, tagAead);
            HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret,
                "OpensslAesAeadEncryptFinal fail, ret = %" LOG_PUBLIC "d", ret)
            break;
#endif
#if defined(HKS_SUPPORT_AES_CBC_NOPADDING) || defined(HKS_SUPPORT_AES_CBC_PKCS7) ||     \
    defined(HKS_SUPPORT_AES_CTR_NOPADDING) || defined(HKS_SUPPORT_AES_ECB_NOPADDING) || \
    defined(HKS_SUPPORT_AES_ECB_PKCS7PADDING)
        case HKS_MODE_CBC:
        case HKS_MODE_CTR:
        case HKS_MODE_ECB:
            ret = OpensslAesCipherInit(key, usageSpec, true, &ctx);
            HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret,
                "OpensslAesCipherInit fail, ret = %" LOG_PUBLIC "d", ret)

            ret = OpensslAesCipherEncryptFinal(ctx, message, &tmpCipherText);
            HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret,
                "OpensslAesCipherEncryptFinal fail, ret = %" LOG_PUBLIC "d", ret)
            break;
#endif
        default:
            HKS_LOG_E("Unsupport aes mode! mode = 0x%" LOG_PUBLIC "x", usageSpec->mode);
            return HKS_ERROR_INVALID_ARGUMENT;
    }

    cipherText->size = tmpCipherText.size;
    return HKS_SUCCESS;
}

int32_t HksOpensslAesDecrypt(const struct HksBlob *key, const struct HksUsageSpec *usageSpec,
    const struct HksBlob *message, struct HksBlob *plainText)
{
    EVP_CIPHER_CTX *ctx = NULL;
    struct HksBlob tmpPlainText = *plainText;

    int32_t ret;
    switch (usageSpec->mode) {
        case HKS_MODE_GCM:
            ret = OpensslAesAeadInit(key, usageSpec, false, &ctx);
            HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret,
                "OpensslAesAeadDecryptInit fail, ret = %" LOG_PUBLIC "d", ret)

            ret = OpensslAesAeadDecryptFinal(ctx, usageSpec, message, &tmpPlainText);
            HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret,
                "OpensslAesAeadDecryptFinal fail, ret = %" LOG_PUBLIC "d", ret)
            break;
        case HKS_MODE_CBC:
        case HKS_MODE_CTR:
        case HKS_MODE_ECB:
            ret = OpensslAesCipherInit(key, usageSpec, false, &ctx);
            HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret,
                "OpensslAesCipherInit fail, ret = %" LOG_PUBLIC "d", ret)

            ret = OpensslAesCipherDecryptFinal(ctx, message, &tmpPlainText);
            HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret,
                "OpensslAesCipherDecryptFinal fail, ret = %" LOG_PUBLIC "d", ret)
            break;
        default:
            HKS_LOG_E("Unsupport aes mode! mode = 0x%" LOG_PUBLIC "x", usageSpec->mode);
            return HKS_ERROR_INVALID_ARGUMENT;
    }

    plainText->size = tmpPlainText.size;
    return ret;
}
#endif
