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

#ifdef HKS_SUPPORT_ECC_C
#include "hks_openssl_ecc.h"

#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/obj_mac.h>
#include <openssl/ossl_typ.h>
#include <stdbool.h>
#include <stddef.h>

#include "hks_log.h"
#include "hks_mem.h"
#include "hks_openssl_engine.h"
#include "hks_template.h"
#include "securec.h"

static int32_t HksOpensslEccCheckKeyLen(uint32_t keyLen)
{
    if ((keyLen != HKS_ECC_KEY_SIZE_224) && (keyLen != HKS_ECC_KEY_SIZE_256) && (keyLen != HKS_ECC_KEY_SIZE_384) &&
        (keyLen != HKS_ECC_KEY_SIZE_521)) {
        HKS_LOG_E("invalid param keyLen(0x%" LOG_PUBLIC "x)!", keyLen);
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    return HKS_SUCCESS;
}

static int32_t HksOpensslGetCurveId(uint32_t keyLen, int *nid)
{
    switch (keyLen) {
        case HKS_ECC_KEY_SIZE_224:
            *nid = NID_secp224r1;
            break;
        case HKS_ECC_KEY_SIZE_256:
            *nid = NID_X9_62_prime256v1;
            break;
        case HKS_ECC_KEY_SIZE_384:
            *nid = NID_secp384r1;
            break;
        case HKS_ECC_KEY_SIZE_521:
            *nid = NID_secp521r1;
            break;
        default:
            HKS_LOG_E("invalid key size.");
            return HKS_ERROR_INVALID_AE_TAG;
    }

    return HKS_SUCCESS;
}

#ifdef HKS_SUPPORT_ECC_GENERATE_KEY
static int32_t TransEccKeyToKeyBlob(
    const EC_KEY *eccKey, const struct KeyMaterialEcc *keyMaterial, BIGNUM *pubX, BIGNUM *pubY, uint8_t *rawMaterial)
{
    const EC_GROUP *ecGroup = EC_KEY_get0_group(eccKey);
    int retCode = EC_POINT_get_affine_coordinates_GFp(ecGroup, EC_KEY_get0_public_key(eccKey), pubX, pubY, NULL);
    if (retCode <= 0) {
        HksLogOpensslError();
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    const BIGNUM *priv = EC_KEY_get0_private_key(eccKey);
    uint32_t offset = sizeof(struct KeyMaterialEcc);

    retCode = BN_bn2binpad(pubX, rawMaterial + offset, keyMaterial->xSize);
    if (retCode <= 0) {
        HksLogOpensslError();
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }
    offset += keyMaterial->xSize;

    retCode = BN_bn2binpad(pubY, rawMaterial + offset, keyMaterial->ySize);
    if (retCode <= 0) {
        HksLogOpensslError();
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }
    offset += keyMaterial->ySize;

    retCode = BN_bn2binpad(priv, rawMaterial + offset, keyMaterial->zSize);
    if (retCode <= 0) {
        HksLogOpensslError();
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    return HKS_SUCCESS;
}

static int32_t EccSaveKeyMaterial(const EC_KEY *eccKey, const struct HksKeySpec *spec,
    uint8_t **output, uint32_t *outputSize)
{
    uint32_t keySize = spec->keyLen;
    /* public exponent x and y, and private exponent, so need size is: keySize / 8 * 3 */
    uint32_t rawMaterialLen = sizeof(struct KeyMaterialEcc) + HKS_KEY_BYTES(keySize) * ECC_KEYPAIR_CNT;
    uint8_t *rawMaterial = (uint8_t *)HksMalloc(rawMaterialLen);
    HKS_IF_NULL_LOGE_RETURN(rawMaterial, HKS_ERROR_MALLOC_FAIL, "malloc buffer failed!")

    (void)memset_s(rawMaterial, rawMaterialLen, 0, rawMaterialLen);

    /*
     * ECC key data internal struct:
     * struct KeyMaterialEcc + pubX_data + pubY_data + pri_data
     */
    struct KeyMaterialEcc *keyMaterial = (struct KeyMaterialEcc *)rawMaterial;
    keyMaterial->keyAlg = (enum HksKeyAlg)spec->algType;
    keyMaterial->keySize = keySize;
    keyMaterial->xSize = HKS_KEY_BYTES(keySize);
    keyMaterial->ySize = HKS_KEY_BYTES(keySize);
    keyMaterial->zSize = HKS_KEY_BYTES(keySize);

    BIGNUM *pubX = BN_new();
    BIGNUM *pubY = BN_new();

    int32_t ret;
    do {
        if ((pubX == NULL) || (pubY == NULL)) {
            HKS_LOG_E("BN_new x or y failed");
            ret = HKS_ERROR_NULL_POINTER;
            HksFree(rawMaterial);
            break;
        }
        ret = TransEccKeyToKeyBlob(eccKey, keyMaterial, pubX, pubY, rawMaterial);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("transfer ecc key to key blob failed");
            HksFree(rawMaterial);
            break;
        }
        *output = rawMaterial;
        *outputSize = rawMaterialLen;
    } while (0);

    if (pubX != NULL) {
        BN_free(pubX);
        pubX = NULL;
    }
    if (pubY != NULL) {
        BN_free(pubY);
        pubY = NULL;
    }
    return ret;
}

int32_t HksOpensslEccGenerateKey(const struct HksKeySpec *spec, struct HksBlob *key)
{
    int curveId;
    switch (spec->algType) {
#if defined(HKS_SUPPORT_SM2_C) && defined(HKS_SUPPORT_SM2_GENERATE_KEY)
        case HKS_ALG_SM2:
            if (spec->keyLen != HKS_SM2_KEY_SIZE_256) {
                HKS_LOG_E("Sm2 Invalid Param!");
                return HKS_ERROR_INVALID_ARGUMENT;
            }
            curveId = NID_sm2;
            break;
#endif
        default:
            HKS_IF_NOT_SUCC_LOGE_RETURN(HksOpensslEccCheckKeyLen(spec->keyLen),
                HKS_ERROR_INVALID_ARGUMENT, "Ecc Invalid Param!")

            HKS_IF_NOT_SUCC_LOGE_RETURN(HksOpensslGetCurveId(spec->keyLen, &curveId),
                HKS_ERROR_INVALID_ARGUMENT, "Ecc get curveId failed!")
            break;
    }

    EC_KEY *eccKey = NULL;
    int32_t ret = HKS_ERROR_CRYPTO_ENGINE_ERROR;
    do {
        eccKey = EC_KEY_new_by_curve_name(curveId);
        if (eccKey == NULL) {
            HKS_LOG_E("new ec key failed");
            HksLogOpensslError();
            break;
        }

        if (EC_KEY_generate_key(eccKey) <= 0) {
            HKS_LOG_E("generate ec key failed");
            HksLogOpensslError();
            break;
        }

        ret = EccSaveKeyMaterial(eccKey, spec, &key->data, &key->size);
        HKS_IF_NOT_SUCC_LOGE(ret, "save ec key material failed! ret=0x%" LOG_PUBLIC "x", ret)
    } while (0);

    if (eccKey != NULL) {
        EC_KEY_free(eccKey);
        eccKey = NULL;
    }

    return ret;
}
#endif

#ifdef HKS_SUPPORT_ECC_GET_PUBLIC_KEY
int32_t HksOpensslGetEccPubKey(const struct HksBlob *input, struct HksBlob *output)
{
    struct KeyMaterialEcc *keyMaterial = (struct KeyMaterialEcc *)input->data;

    if ((keyMaterial->xSize == 0) || (keyMaterial->ySize == 0)) {
        HKS_LOG_E("not support get pubkey");
        return HKS_ERROR_NOT_SUPPORTED;
    }

    output->size = sizeof(struct KeyMaterialEcc) + keyMaterial->xSize + keyMaterial->ySize;

    struct KeyMaterialEcc *publickeyMaterial = (struct KeyMaterialEcc *)output->data;
    publickeyMaterial->keyAlg = keyMaterial->keyAlg;
    publickeyMaterial->keySize = keyMaterial->keySize;
    publickeyMaterial->xSize = keyMaterial->xSize;
    publickeyMaterial->ySize = keyMaterial->ySize;
    publickeyMaterial->zSize = 0;

    (void)memcpy_s(output->data + sizeof(struct KeyMaterialEcc), output->size - sizeof(struct KeyMaterialEcc),
        input->data + sizeof(struct KeyMaterialEcc), keyMaterial->xSize + keyMaterial->ySize);

    return HKS_SUCCESS;
}
#endif

static int GetEccModules(
    const uint8_t *key, uint32_t *keySize, uint32_t *publicXSize, uint32_t *publicYSize, uint32_t *privateXSize)
{
    struct KeyMaterialEcc *keyMaterial = (struct KeyMaterialEcc *)key;
    *keySize = keyMaterial->keySize;
    *publicXSize = keyMaterial->xSize;
    *publicYSize = keyMaterial->ySize;
    *privateXSize = keyMaterial->zSize;

    return 0;
}

static int32_t EccInitPublicKey(EC_KEY *eccKey, const uint8_t *keyPair, uint32_t xSize, uint32_t ySize)
{
    const EC_GROUP *ecGroup = EC_KEY_get0_group(eccKey);
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
            HksLogOpensslError();
            break;
        }

        if (EC_KEY_set_public_key(eccKey, pub) <= 0) {
            HksLogOpensslError();
            break;
        }
        ret = HKS_SUCCESS;
    } while (0);

    if (pubX != NULL) {
        BN_free(pubX);
        pubX = NULL;
    }

    if (pubY != NULL) {
        BN_free(pubY);
        pubY = NULL;
    }

    if (pub != NULL) {
        EC_POINT_free(pub);
        pub = NULL;
    }
    return ret;
}

static EC_KEY *EccInitKey(const struct HksBlob *keyBlob, bool private)
{
    /* get ecc pubX,pubY,pri */
    uint8_t *keyPair = keyBlob->data;
    uint32_t publicXSize;
    uint32_t publicYSize;
    uint32_t privateSize;
    uint32_t keySize;

    if (GetEccModules(keyPair, &keySize, &publicXSize, &publicYSize, &privateSize) != 0) {
        HKS_LOG_E("get ecc key modules is failed");
        return NULL;
    }

    int nid;
    HKS_IF_NOT_SUCC_LOGE_RETURN(HksOpensslGetCurveId(keySize, &nid), NULL, "get curve id failed")

    EC_KEY *eccKey = EC_KEY_new_by_curve_name(nid);
    if (eccKey == NULL) {
        HksLogOpensslError();
        return NULL;
    }

    if (!private) {
        if (EccInitPublicKey(eccKey, keyPair, publicXSize, publicYSize) != HKS_SUCCESS) {
            HKS_LOG_E("initialize ecc public key failed");
            EC_KEY_free(eccKey);
            return NULL;
        }
    }

    if (private) {
        BIGNUM *pri = BN_bin2bn(keyPair + sizeof(struct KeyMaterialEcc) + publicXSize + publicYSize, privateSize, NULL);
        if (pri == NULL || EC_KEY_set_private_key(eccKey, pri) <= 0) {
            HKS_LOG_E("build ecc key failed");
            BN_free(pri);
            EC_KEY_free(eccKey);
            return NULL;
        }
        BN_clear_free(pri);
    }

    return eccKey;
}

static int32_t GetEvpKey(const struct HksBlob *keyBlob, EVP_PKEY *key, bool private)
{
    EC_KEY *eccKey = EccInitKey(keyBlob, private);
    HKS_IF_NULL_LOGE_RETURN(eccKey, HKS_ERROR_CRYPTO_ENGINE_ERROR, "initialize ecc key failed\n")

    if (EVP_PKEY_assign_EC_KEY(key, eccKey) <= 0) {
        HksLogOpensslError();
        EC_KEY_free(eccKey);
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }
    return HKS_SUCCESS;
}

static int32_t GetNativePKey(const struct HksBlob *nativeKey, EVP_PKEY *key)
{
    int32_t ret = GetEvpKey(nativeKey, key, true);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get native evp key failed")
    return ret;
}

static int32_t GetPeerKey(const struct HksBlob *pubKey, EVP_PKEY *key)
{
    int32_t ret = GetEvpKey(pubKey, key, false);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_CRYPTO_ENGINE_ERROR, "get peer evp key failed")
    return ret;
}

static int32_t EcdhDerive(EVP_PKEY_CTX *ctx, EVP_PKEY *peerKey, struct HksBlob *sharedKey)
{
    size_t tmpSharedKeySize = (size_t)sharedKey->size;
    if (EVP_PKEY_derive_init(ctx) != 1) {
        HksLogOpensslError();
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }
    if (EVP_PKEY_derive_set_peer(ctx, peerKey) != 1) {
        HksLogOpensslError();
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }
    if (EVP_PKEY_derive(ctx, NULL, &tmpSharedKeySize) != 1) {
        HksLogOpensslError();
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    if (tmpSharedKeySize > sharedKey->size) {
        return HKS_ERROR_BUFFER_TOO_SMALL;
    }

    if (EVP_PKEY_derive(ctx, sharedKey->data, &tmpSharedKeySize) != 1) {
        HksLogOpensslError();
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }
    sharedKey->size = tmpSharedKeySize;

    return HKS_SUCCESS;
}

static int32_t AgreeKeyEcdh(const struct HksBlob *nativeKey, const struct HksBlob *pubKey, struct HksBlob *sharedKey)
{
    int32_t res = HKS_ERROR_CRYPTO_ENGINE_ERROR;
    EVP_PKEY *pKey = EVP_PKEY_new();
    EVP_PKEY *peerKey = EVP_PKEY_new();
    EVP_PKEY_CTX *ctx = NULL;

    do {
        if ((peerKey == NULL) || (pKey == NULL)) {
            HKS_LOG_E("new pkey failed\n");
            break;
        }
        int32_t ret = GetNativePKey(nativeKey, pKey);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "get native pkey failed\n")

        ret = GetPeerKey(pubKey, peerKey);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "get peer pkey failed\n")

        ctx = EVP_PKEY_CTX_new(pKey, NULL);
        if (ctx == NULL) {
            HksLogOpensslError();
            break;
        }

        ret = EcdhDerive(ctx, peerKey, sharedKey);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "derive ecdh key failed\n")

        res = HKS_SUCCESS;
    } while (0);

    EVP_PKEY_CTX_free(ctx);
    if (peerKey != NULL) {
        EVP_PKEY_free(peerKey);
    }
    if (pKey != NULL) {
        EVP_PKEY_free(pKey);
    }

    return res;
}

#ifdef HKS_SUPPORT_ECDH_AGREE_KEY
int32_t HksOpensslEcdhAgreeKey(const struct HksBlob *nativeKey, const struct HksBlob *pubKey,
    const struct HksKeySpec *spec, struct HksBlob *sharedKey)
{
    HKS_IF_NOT_SUCC_LOGE_RETURN(HksOpensslEccCheckKeyLen(spec->keyLen),
        HKS_ERROR_INVALID_ARGUMENT, "invalid param!")
    int32_t ret = AgreeKeyEcdh(nativeKey, pubKey, sharedKey);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "ecdh key agreement failed!")

    return ret;
}
#endif

#ifdef HKS_SUPPORT_ECDSA_SIGN_VERIFY
static EVP_PKEY_CTX *InitEcdsaCtx(const struct HksBlob *mainKey, uint32_t digest, bool sign, uint32_t len)
{
    const EVP_MD *opensslAlg = GetOpensslAlg(digest);
    if (digest == HKS_DIGEST_NONE) {
        opensslAlg = GetOpensslAlgFromLen(len);
    }
    if (opensslAlg == NULL) {
        HKS_LOG_E("get openssl algorithm fail");
        return NULL;
    }

    EC_KEY *eccKey = EccInitKey(mainKey, sign);
    HKS_IF_NULL_LOGE_RETURN(eccKey, NULL, "initialize ecc key failed")

    EVP_PKEY *key = EVP_PKEY_new();
    if (key == NULL) {
        HksLogOpensslError();
        EC_KEY_free(eccKey);
        return NULL;
    }

    if (EVP_PKEY_assign_EC_KEY(key, eccKey) <= 0) {
        EC_KEY_free(eccKey);
        EVP_PKEY_free(key);
        return NULL;
    }

    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(key, NULL);
    if (ctx == NULL) {
        HksLogOpensslError();
        EVP_PKEY_free(key);
        return NULL;
    }
    int32_t ret;
    if (sign) {
        ret = EVP_PKEY_sign_init(ctx);
    } else {
        ret = EVP_PKEY_verify_init(ctx);
    }
    EVP_PKEY_free(key);
    if (ret != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        EVP_PKEY_CTX_free(ctx);
        return NULL;
    }
    if (EVP_PKEY_CTX_set_signature_md(ctx, opensslAlg) != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        EVP_PKEY_CTX_free(ctx);
        return NULL;
    }
    return ctx;
}

int32_t HksOpensslEcdsaVerify(const struct HksBlob *key, const struct HksUsageSpec *usageSpec,
    const struct HksBlob *message, const struct HksBlob *signature)
{
    EVP_PKEY_CTX *ctx = InitEcdsaCtx(key, usageSpec->digest, false, message->size);
    HKS_IF_NULL_LOGE_RETURN(ctx, HKS_ERROR_INVALID_KEY_INFO, "initialize ecc context failed")

    if (EVP_PKEY_verify(ctx, signature->data, signature->size, message->data, message->size) != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        EVP_PKEY_CTX_free(ctx);
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    EVP_PKEY_CTX_free(ctx);
    return HKS_SUCCESS;
}

int32_t HksOpensslEcdsaSign(const struct HksBlob *key, const struct HksUsageSpec *usageSpec,
    const struct HksBlob *message, struct HksBlob *signature)
{
    EVP_PKEY_CTX *ctx = InitEcdsaCtx(key, usageSpec->digest, true, message->size);
    HKS_IF_NULL_LOGE_RETURN(ctx, HKS_ERROR_INVALID_KEY_INFO, "initialize ecc context failed")

    size_t sigSize = (size_t)signature->size;
    if (EVP_PKEY_sign(ctx, signature->data, &sigSize, message->data, message->size) != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        EVP_PKEY_CTX_free(ctx);
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }
    signature->size = (uint32_t)sigSize;
    EVP_PKEY_CTX_free(ctx);
    return HKS_SUCCESS;
}
#endif
#endif