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

#ifdef HKS_SUPPORT_SM2_C
#include "hks_openssl_sm2.h"

#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/obj_mac.h>
#include <openssl/ossl_typ.h>
#include <stdbool.h>
#include <stddef.h>

#include "hks_log.h"
#include "hks_mem.h"
#include "hks_openssl_ecc.h"
#include "hks_openssl_engine.h"
#include "hks_template.h"

#ifdef HKS_SUPPORT_SM2_GENERATE_KEY
int32_t HksOpensslSm2GenerateKey(const struct HksKeySpec *spec, struct HksBlob *key)
{
    return HksOpensslEccGenerateKey(spec, key);
}
#endif

#ifdef HKS_SUPPORT_SM2_GET_PUBLIC_KEY
int32_t HksOpensslGetSm2PubKey(const struct HksBlob *input, struct HksBlob *output)
{
    return HksOpensslGetEccPubKey(input, output);
}
#endif

#ifdef HKS_SUPPORT_SM2_SIGN_VERIFY
static int GetSm2Modules(
    const uint8_t *key, uint32_t *keySize, uint32_t *publicXSize, uint32_t *publicYSize, uint32_t *privateXSize)
{
    struct KeyMaterialEcc *keyMaterial = (struct KeyMaterialEcc *)key;
    *keySize = keyMaterial->keySize;
    *publicXSize = keyMaterial->xSize;
    *publicYSize = keyMaterial->ySize;
    *privateXSize = keyMaterial->zSize;
    return HKS_SUCCESS;
}

static int32_t Sm2InitPublicKey(EC_KEY *sm2Key, const uint8_t *keyPair, uint32_t xSize, uint32_t ySize)
{
    const EC_GROUP *ecGroup = EC_KEY_get0_group(sm2Key);
    if (ecGroup == NULL) {
        HksLogOpensslError();
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    int32_t ret = HKS_ERROR_CRYPTO_ENGINE_ERROR;
    uint32_t offset = sizeof(struct KeyMaterialEcc);
    EC_POINT *pub = EC_POINT_new(ecGroup);
    BIGNUM *pubX = BN_bin2bn(keyPair + offset, xSize, NULL);
    offset += xSize;
    BIGNUM *pubY = BN_bin2bn(keyPair + offset, ySize, NULL);
    do {
        if ((pubX == NULL) || (pubY == NULL) || (pub == NULL)) {
            HKS_LOG_E("new big num x or y or pub failed");
            break;
        }

        if (EC_POINT_set_affine_coordinates_GFp(ecGroup, pub, pubX, pubY, NULL) <= 0) {
            HKS_LOG_E("ec point set failed");
            HksLogOpensslError();
            break;
        }

        if (EC_KEY_set_public_key(sm2Key, pub) <= 0) {
            HKS_LOG_E("set public key failed");
            HksLogOpensslError();
            break;
        }
        ret = HKS_SUCCESS;
    } while (0);

    SELF_FREE_PTR(pubX, BN_free);
    SELF_FREE_PTR(pubY, BN_free);
    SELF_FREE_PTR(pub, EC_POINT_free);
    return ret;
}

static EC_KEY *Sm2InitKey(const struct HksBlob *keyBlob, bool private)
{
    /* get ecc pubX,pubY,pri */
    uint8_t *keyPair = keyBlob->data;
    uint32_t publicXSize;
    uint32_t publicYSize;
    uint32_t privateSize;
    uint32_t keySize;

    HKS_IF_NOT_SUCC_LOGE_RETURN(GetSm2Modules(keyPair, &keySize, &publicXSize, &publicYSize, &privateSize),
        NULL, "get sm2 key modules is failed")

    EC_KEY *sm2Key = EC_KEY_new_by_curve_name(NID_sm2);
    if (sm2Key == NULL) {
        HKS_LOG_E("new sm2 key failed");
        HksLogOpensslError();
        return NULL;
    }

    if (!private) {
        if (Sm2InitPublicKey(sm2Key, keyPair, publicXSize, publicYSize) != HKS_SUCCESS) {
            HKS_LOG_E("initialize sm2 public key failed");
            SELF_FREE_PTR(sm2Key, EC_KEY_free);
            return NULL;
        }
    }

    if (private) {
        BIGNUM *pri = BN_bin2bn(keyPair + sizeof(struct KeyMaterialEcc) + publicXSize + publicYSize, privateSize, NULL);
        if (pri == NULL || EC_KEY_set_private_key(sm2Key, pri) <= 0) {
            HKS_LOG_E("build sm2 key failed");
            SELF_FREE_PTR(pri, BN_free);
            SELF_FREE_PTR(sm2Key, EC_KEY_free);
            return NULL;
        }
        SELF_FREE_PTR(pri, BN_clear_free);
    }

    return sm2Key;
}

static EVP_PKEY_CTX *InitSm2Ctx(const struct HksBlob *mainKey, uint32_t digest, bool sign)
{
    EC_KEY *sm2Key = Sm2InitKey(mainKey, sign);
    HKS_IF_NULL_LOGE_RETURN(sm2Key, NULL, "initialize sm2 key failed")

    EVP_PKEY *key = EVP_PKEY_new();
    if (key == NULL) {
        HKS_LOG_E("new evp key failed");
        HksLogOpensslError();
        SELF_FREE_PTR(sm2Key, EC_KEY_free);
        return NULL;
    }

    int32_t ret = HKS_ERROR_CRYPTO_ENGINE_ERROR;
    EVP_PKEY_CTX *ctx = NULL;
    do {
        if (EVP_PKEY_assign_EC_KEY(key, sm2Key) <= 0) {
            HKS_LOG_E("assign ec key failed");
            break;
        }

        if (EVP_PKEY_set_alias_type(key, EVP_PKEY_SM2) != HKS_OPENSSL_SUCCESS) {
            HKS_LOG_E("set alias type failed");
            break;
        }

        ctx = EVP_PKEY_CTX_new(key, NULL);
        HKS_IF_NULL_LOGE_BREAK(ctx, "new ctx failed")

        if (sign) {
            if (EVP_PKEY_sign_init(ctx) != HKS_OPENSSL_SUCCESS) {
                HKS_LOG_E("sign init failed");
                break;
            }
        } else {
            if (EVP_PKEY_verify_init(ctx) != HKS_OPENSSL_SUCCESS) {
                HKS_LOG_E("verify init failed");
                break;
            }
        }
        ret = HKS_SUCCESS;
    } while (0);

    if (ret != HKS_SUCCESS) {
        HksLogOpensslError();
        SELF_FREE_PTR(sm2Key, EC_KEY_free);
        SELF_FREE_PTR(key, EVP_PKEY_free);
        SELF_FREE_PTR(ctx, EVP_PKEY_CTX_free);
        return NULL;
    }

    return ctx;
}

int32_t HksOpensslSm2Verify(const struct HksBlob *key, const struct HksUsageSpec *usageSpec,
    const struct HksBlob *message, const struct HksBlob *signature)
{
    EVP_PKEY_CTX *ctx = InitSm2Ctx(key, usageSpec->digest, false);
    HKS_IF_NULL_LOGE_RETURN(ctx, HKS_ERROR_INVALID_KEY_INFO, "initialize sm2 context failed")

    if (EVP_PKEY_verify(ctx, signature->data, signature->size, message->data, message->size) != HKS_OPENSSL_SUCCESS) {
        HKS_LOG_D("verify data failed");
        HksLogOpensslError();
        SELF_FREE_PTR(ctx, EVP_PKEY_CTX_free);
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    SELF_FREE_PTR(ctx, EVP_PKEY_CTX_free);
    return HKS_SUCCESS;
}

int32_t HksOpensslSm2Sign(const struct HksBlob *key, const struct HksUsageSpec *usageSpec,
    const struct HksBlob *message, struct HksBlob *signature)
{
    EVP_PKEY_CTX *ctx = InitSm2Ctx(key, usageSpec->digest, true);
    HKS_IF_NULL_LOGE_RETURN(ctx, HKS_ERROR_INVALID_KEY_INFO, "initialize sm2 context failed")

    size_t sigSize;
    if (EVP_PKEY_sign(ctx, NULL, &sigSize, message->data, message->size) != HKS_OPENSSL_SUCCESS) {
        HKS_LOG_D("get sigSize failed");
        HksLogOpensslError();
        SELF_FREE_PTR(ctx, EVP_PKEY_CTX_free);
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    if (EVP_PKEY_sign(ctx, signature->data, &sigSize, message->data, message->size) != HKS_OPENSSL_SUCCESS) {
        HKS_LOG_D("sign data failed");
        HksLogOpensslError();
        SELF_FREE_PTR(ctx, EVP_PKEY_CTX_free);
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }
    signature->size = (uint32_t)sigSize;
    SELF_FREE_PTR(ctx, EVP_PKEY_CTX_free);

    return HKS_SUCCESS;
}
#endif

#endif
