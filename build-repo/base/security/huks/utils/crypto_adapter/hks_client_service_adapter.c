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

#include "hks_client_service_adapter.h"

#include <openssl/bn.h>
#include <openssl/dh.h>
#include <openssl/dsa.h>
#include <openssl/ec.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/obj_mac.h>
#include <openssl/ossl_typ.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>
#include <stddef.h>
#include <stdint.h>

#include "hks_crypto_hal.h"
#include "hks_log.h"
#include "hks_mem.h"
#include "hks_template.h"
#include "securec.h"

#if defined(HKS_SUPPORT_DSA_C)
typedef const BIGNUM* (*GetDsaInfoFunc)(const DSA *d);
#endif

#if defined(HKS_SUPPORT_RSA_C) || defined(HKS_SUPPORT_ECC_C) || defined(HKS_SUPPORT_DSA_C) || \
    defined(HKS_SUPPORT_DH_C) || defined(HKS_SUPPORT_SM2_C)
static int32_t EvpKeyToX509Format(EVP_PKEY *pkey, struct HksBlob *x509Key)
{
    int32_t length = i2d_PUBKEY(pkey, NULL);
    if (length <= 0 || length > MAX_OUT_BLOB_SIZE) {
        HKS_LOG_E("i2d_PUBKEY error %" LOG_PUBLIC "s", ERR_reason_error_string(ERR_get_error()));
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    uint32_t keyLength = (uint32_t)length;
    uint8_t *key = (uint8_t *)HksMalloc(keyLength);
    HKS_IF_NULL_LOGE_RETURN(key, HKS_ERROR_MALLOC_FAIL, "malloc key fail")

    /* tmp will be modified in i2d_PUBKEY */
    uint8_t *tmp = key;
    if ((uint32_t)i2d_PUBKEY(pkey, &tmp) != keyLength) {
        HKS_LOG_E("i2d_PUBKEY error %" LOG_PUBLIC "s", ERR_reason_error_string(ERR_get_error()));
        HksFree(key);
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    x509Key->size = keyLength;
    x509Key->data = key;
    return HKS_SUCCESS;
}

#if defined(HKS_SUPPORT_RSA_C) && defined(HKS_SUPPORT_RSA_GET_PUBLIC_KEY)
static int32_t RsaToX509PublicKey(const struct HksBlob *mod, const struct HksBlob *e, struct HksBlob *x509Key)
{
    RSA *rsa = NULL;
    BIGNUM *rsaN = NULL;
    BIGNUM *rsaE = NULL;
    EVP_PKEY *pkey = NULL;
    int32_t result;

    do {
        result = HKS_ERROR_CRYPTO_ENGINE_ERROR;
        rsa = RSA_new();
        HKS_IF_NULL_LOGE_BREAK(rsa, "rsa is null")
        rsaN = BN_bin2bn(mod->data, mod->size, NULL);
        rsaE = BN_bin2bn(e->data, e->size, NULL);
        if (rsaN == NULL || rsaE == NULL) {
            HKS_LOG_E("BN_bin2bn error %" LOG_PUBLIC "s", ERR_reason_error_string(ERR_get_error()));
            break;
        }
        if (RSA_set0_key(rsa, rsaN, rsaE, NULL) == 0) {
            HKS_LOG_E("RSA_set0_key error %" LOG_PUBLIC "s", ERR_reason_error_string(ERR_get_error()));
            break;
        }
        rsaN = NULL;
        rsaE = NULL;
        pkey = EVP_PKEY_new();
        HKS_IF_NULL_LOGE_BREAK(pkey, "pkey is null")
        if (EVP_PKEY_set1_RSA(pkey, rsa) == 0) {
            HKS_LOG_E("EVP_PKEY_set1_RSA error %" LOG_PUBLIC "s", ERR_reason_error_string(ERR_get_error()));
            break;
        }
        result = EvpKeyToX509Format(pkey, x509Key);
    } while (0);

    SELF_FREE_PTR(rsa, RSA_free);
    SELF_FREE_PTR(rsaN, BN_free);
    SELF_FREE_PTR(rsaE, BN_free);
    SELF_FREE_PTR(pkey, EVP_PKEY_free);
    return result;
}
#endif

#if defined(HKS_SUPPORT_ECC_C) && defined(HKS_SUPPORT_ECC_GET_PUBLIC_KEY)
static int32_t GetEccNid(uint32_t keySize, int32_t *nid)
{
    int32_t nids[][2] = {
        /* 2 is size */
        { 224, NID_secp224r1 },
        { 256, NID_X9_62_prime256v1 },
        { 384, NID_secp384r1 },
        { 521, NID_secp521r1 },
    };

    uint32_t nidCount = sizeof(nids) / sizeof(nids[0]);

    for (uint32_t i = 0; i < nidCount; i++) {
        if (keySize == (uint32_t)nids[i][0]) {
            *nid = nids[i][1];
            return HKS_SUCCESS;
        }
    }

    HKS_LOG_E("not found nid!");
    return HKS_ERROR_INVALID_ARGUMENT;
}

static int32_t EccToX509PublicKey(
    const uint32_t alg, uint32_t keySize, const struct HksBlob *x, const struct HksBlob *y, struct HksBlob *x509Key)
{
    int32_t nid;
    if (alg == HKS_ALG_SM2) {
        nid = NID_sm2;
    } else {
        HKS_IF_NOT_SUCC_LOGE_RETURN(GetEccNid(keySize, &nid), HKS_ERROR_INVALID_ARGUMENT, "GetNidFromKeySize fail")
    }

    EC_KEY *ecKey = NULL;
    BIGNUM *ecX = NULL;
    BIGNUM *ecY = NULL;
    EVP_PKEY *pkey = NULL;
    int32_t ret = HKS_ERROR_CRYPTO_ENGINE_ERROR;
    do {
        ecKey = EC_KEY_new_by_curve_name(nid);
        HKS_IF_NULL_LOGE_BREAK(ecKey,
            "EC_KEY_new_by_curve_name error %" LOG_PUBLIC "s", ERR_reason_error_string(ERR_get_error()))

        ecX = BN_bin2bn(x->data, x->size, NULL);
        ecY = BN_bin2bn(y->data, y->size, NULL);
        if (ecX == NULL || ecY == NULL) {
            HKS_LOG_E("x y point is null");
            break;
        }

        if (EC_KEY_set_public_key_affine_coordinates(ecKey, ecX, ecY) == 0) {
            HKS_LOG_E("EC_KEY_set_public_key_affine_coordinates error %" LOG_PUBLIC "s",
                ERR_reason_error_string(ERR_get_error()));
            break;
        }

        EC_KEY_set_conv_form(ecKey, POINT_CONVERSION_UNCOMPRESSED);
        pkey = EVP_PKEY_new();
        HKS_IF_NULL_LOGE_BREAK(pkey, "pkey is null")

        if (EVP_PKEY_set1_EC_KEY(pkey, ecKey) == 0) {
            HKS_LOG_E("EVP_PKEY_set1_EC_KEY error %" LOG_PUBLIC "s", ERR_reason_error_string(ERR_get_error()));
            break;
        }

        ret = EvpKeyToX509Format(pkey, x509Key);
    } while (0);

    SELF_FREE_PTR(ecKey, EC_KEY_free);
    SELF_FREE_PTR(ecX, BN_free);
    SELF_FREE_PTR(ecY, BN_free);
    SELF_FREE_PTR(pkey, EVP_PKEY_free);
    return ret;
}
#endif

#if defined(HKS_SUPPORT_DSA_C) && defined(HKS_SUPPORT_DSA_GET_PUBLIC_KEY)
static int32_t GetDsaPubKeyParam(
    const struct HksBlob *publicKey, struct HksBlob *y, struct HksBlob *p, struct HksBlob *q, struct HksBlob *g)
{
    if (publicKey->size < sizeof(struct KeyMaterialDsa)) {
        HKS_LOG_E("Invaild dsa key material size!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    struct KeyMaterialDsa *keyMaterial = (struct KeyMaterialDsa *)publicKey->data;
    uint32_t keyMaterialSize = sizeof(struct KeyMaterialDsa) + keyMaterial->xSize + keyMaterial->ySize +
        keyMaterial->pSize + keyMaterial->qSize + keyMaterial->gSize;
    if (publicKey->size < keyMaterialSize) {
        HKS_LOG_E("translate to x509 public key invalid size");
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    uint32_t offset = sizeof(struct KeyMaterialDsa) + keyMaterial->xSize;
    y->size = keyMaterial->ySize;
    y->data = publicKey->data + offset;
    offset += keyMaterial->ySize;
    p->size = keyMaterial->pSize;
    p->data = publicKey->data + offset;
    offset += keyMaterial->pSize;
    q->size = keyMaterial->qSize;
    q->data = publicKey->data + offset;
    offset += keyMaterial->qSize;
    g->size = keyMaterial->gSize;
    g->data = publicKey->data + offset;
    return HKS_SUCCESS;
}

static int32_t DsaToX509PublicKey(const struct HksBlob *y, const struct HksBlob *p, const struct HksBlob *q,
    const struct HksBlob *g, struct HksBlob *x509Key)
{
    int32_t ret = HKS_ERROR_CRYPTO_ENGINE_ERROR;
    DSA *dsa = NULL;
    BIGNUM *dsaY = BN_bin2bn(y->data, y->size, NULL);
    BIGNUM *dsaP = BN_bin2bn(p->data, p->size, NULL);
    BIGNUM *dsaQ = BN_bin2bn(q->data, q->size, NULL);
    BIGNUM *dsaG = BN_bin2bn(g->data, g->size, NULL);
    EVP_PKEY *pkey = NULL;
    do {
        dsa = DSA_new();
        HKS_IF_NULL_LOGE_BREAK(dsa, "DSA_new error %" LOG_PUBLIC "s", ERR_reason_error_string(ERR_get_error()))

        if (dsaY == NULL || dsaP == NULL || dsaQ == NULL || dsaG == NULL) {
            HKS_LOG_E("DSA parameter is null.");
            break;
        }

        if (DSA_set0_key(dsa, dsaY, NULL) != 1) {
            HKS_LOG_E("DSA_set0_key error %" LOG_PUBLIC "s", ERR_reason_error_string(ERR_get_error()));
            break;
        }
        dsaY = NULL;
        if (DSA_set0_pqg(dsa, dsaP, dsaQ, dsaG) != 1) {
            HKS_LOG_E("DSA_set0_pqg error %" LOG_PUBLIC "s", ERR_reason_error_string(ERR_get_error()));
            break;
        }
        dsaP = NULL;
        dsaQ = NULL;
        dsaG = NULL;

        pkey = EVP_PKEY_new();
        HKS_IF_NULL_LOGE_BREAK(pkey, "pkey is null")

        if (EVP_PKEY_set1_DSA(pkey, dsa) == 0) {
            HKS_LOG_E("EVP_PKEY_set1_DSA error %" LOG_PUBLIC "s", ERR_reason_error_string(ERR_get_error()));
            break;
        }

        ret = EvpKeyToX509Format(pkey, x509Key);
    } while (0);

    SELF_FREE_PTR(dsa, DSA_free);
    SELF_FREE_PTR(dsaY, BN_free);
    SELF_FREE_PTR(dsaP, BN_free);
    SELF_FREE_PTR(dsaQ, BN_free);
    SELF_FREE_PTR(dsaG, BN_free);
    SELF_FREE_PTR(pkey, EVP_PKEY_free);
    return ret;
}

static int32_t DsaPublicKeyToX509(const struct HksBlob *publicKey, struct HksBlob *x509Key)
{
    struct HksBlob y = {0};
    struct HksBlob p = {0};
    struct HksBlob q = {0};
    struct HksBlob g = {0};
    int32_t ret = GetDsaPubKeyParam(publicKey, &y, &p, &q, &g);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    return DsaToX509PublicKey(&y, &p, &q, &g, x509Key);
}

#endif

#if defined(HKS_SUPPORT_DH_C) && defined(HKS_SUPPORT_DH_GET_PUBLIC_KEY)
static int32_t GetDhNid(uint32_t keySize, int32_t *nid)
{
    switch (keySize) {
        case HKS_DH_KEY_SIZE_2048:
            *nid = NID_ffdhe2048;
            return HKS_SUCCESS;
        case HKS_DH_KEY_SIZE_3072:
            *nid = NID_ffdhe3072;
            return HKS_SUCCESS;
        case HKS_DH_KEY_SIZE_4096:
            *nid = NID_ffdhe4096;
            return HKS_SUCCESS;
        default:
            return HKS_ERROR_INVALID_ARGUMENT;
    }
}

static int32_t DhToX509PublicKey(
    uint32_t keySize, const struct HksBlob *pubKey, const struct HksBlob *privKey, struct HksBlob *x509Key)
{
    int32_t nid;
    int32_t ret = GetDhNid(keySize, &nid);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "GetNidFromKeySize fail")

    BIGNUM *pub = NULL;
    DH *dh = NULL;
    EVP_PKEY *pkey = NULL;
    do {
        dh = DH_new_by_nid(nid);
        HKS_IF_NULL_LOGE_BREAK(dh, "DH_new_by_nid error %" LOG_PUBLIC "s", ERR_reason_error_string(ERR_get_error()))

        pub = BN_bin2bn(pubKey->data, pubKey->size, NULL);
        HKS_IF_NULL_LOGE_BREAK(pub, "BN_bin2bn error %" LOG_PUBLIC "s", ERR_reason_error_string(ERR_get_error()))

        if (DH_set0_key(dh, pub, NULL) != 1) {
            HKS_LOG_E("DH_set0_key error:%" LOG_PUBLIC "s", ERR_reason_error_string(ERR_get_error()));
            break;
        }
        pub = NULL;

        pkey = EVP_PKEY_new();
        HKS_IF_NULL_LOGE_BREAK(pkey, "pkey is null")

        if (EVP_PKEY_set1_DH(pkey, dh) == 0) {
            HKS_LOG_E("EVP_PKEY_set1_DH error %" LOG_PUBLIC "s", ERR_reason_error_string(ERR_get_error()));
            break;
        }

        ret = EvpKeyToX509Format(pkey, x509Key);
    } while (0);

    SELF_FREE_PTR(dh, DH_free);
    SELF_FREE_PTR(pub, BN_free);
    SELF_FREE_PTR(pkey, EVP_PKEY_free);
    return ret;
}
#endif
#endif

#if defined(HKS_SUPPORT_X25519_C) || defined(HKS_SUPPORT_ED25519_C)
static int32_t Curve25519ToX509PublicKey(const struct HksBlob *publicKey, struct HksBlob *x509Key)
{
    if (publicKey->size != HKS_KEY_BYTES(HKS_CURVE25519_KEY_SIZE_256)) {
        HKS_LOG_E("Invalid public key size! key size = 0x%" LOG_PUBLIC "X", publicKey->size);
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    x509Key->data = (uint8_t *)HksMalloc(publicKey->size);
    HKS_IF_NULL_LOGE_RETURN(x509Key->data, HKS_ERROR_MALLOC_FAIL,
        "X25519/Ed25519 to x509 public key malloc x509 key data failed!")

    (void)memcpy_s(x509Key->data, publicKey->size, publicKey->data, publicKey->size);
    x509Key->size = publicKey->size;

    return HKS_SUCCESS;
}
#endif

int32_t TranslateToX509PublicKey(const struct HksBlob *publicKey, struct HksBlob *x509Key)
{
    if ((publicKey == NULL) || (publicKey->data == NULL) || (publicKey->size == 0) || (x509Key == NULL)) {
        HKS_LOG_E("translate to x509 public key invalid args");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    if (publicKey->size < sizeof(struct HksPubKeyInfo)) {
        HKS_LOG_E("translate to x509 public key invalid publicKey size");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    struct HksPubKeyInfo *publicKeyInfo = (struct HksPubKeyInfo *)publicKey->data;
    uint32_t offset = sizeof(struct HksPubKeyInfo);
    if ((publicKey->size - offset) < publicKeyInfo->nOrXSize) {
        HKS_LOG_E("translate to x509 public key invalid nOrXSize size");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    struct HksBlob material1 = { publicKeyInfo->nOrXSize, publicKey->data + offset };
    offset += publicKeyInfo->nOrXSize;
    if ((publicKey->size - offset) < publicKeyInfo->eOrYSize) {
        HKS_LOG_E("translate to x509 public key invalid eOrYSize size");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    struct HksBlob material2 = { publicKeyInfo->eOrYSize, publicKey->data + offset };
    switch (publicKeyInfo->keyAlg) {
#if defined(HKS_SUPPORT_RSA_C) && defined(HKS_SUPPORT_RSA_GET_PUBLIC_KEY)
        case HKS_ALG_RSA:
            return RsaToX509PublicKey(&material1, &material2, x509Key);
#endif
#if (defined(HKS_SUPPORT_ECC_C) && defined(HKS_SUPPORT_ECC_GET_PUBLIC_KEY)) || \
        (defined(HKS_SUPPORT_SM2_C) && defined(HKS_SUPPORT_SM2_GET_PUBLIC_KEY))
        case HKS_ALG_ECC:
        case HKS_ALG_SM2:
            return EccToX509PublicKey(publicKeyInfo->keyAlg, publicKeyInfo->keySize, &material1, &material2, x509Key);
#endif
#if defined(HKS_SUPPORT_DSA_C) && defined(HKS_SUPPORT_DSA_GET_PUBLIC_KEY)
        case HKS_ALG_DSA:
            return DsaPublicKeyToX509(publicKey, x509Key);
#endif
#if defined(HKS_SUPPORT_X25519_C) || defined(HKS_SUPPORT_ED25519_C)
        case HKS_ALG_X25519:
        case HKS_ALG_ED25519:
            return Curve25519ToX509PublicKey(&material1, x509Key);
#endif
#if defined(HKS_SUPPORT_DH_C) && defined(HKS_SUPPORT_DH_GET_PUBLIC_KEY)
        case HKS_ALG_DH:
            return DhToX509PublicKey(publicKeyInfo->keySize, &material1, NULL, x509Key);
#endif
        default:
            HKS_LOG_E("Unsupport alg type! type = 0x%" LOG_PUBLIC "X", publicKeyInfo->keyAlg);
            return HKS_ERROR_INVALID_ARGUMENT;
    }
}

#if defined(HKS_SUPPORT_RSA_C) || defined(HKS_SUPPORT_ECC_C) || defined(HKS_SUPPORT_DSA_C) || \
    defined(HKS_SUPPORT_DH_C)
#ifdef HKS_SUPPORT_RSA_C
static int32_t X509PublicKeyToRsa(EVP_PKEY *pkey, struct HksBlob *rsaPublicKey)
{
    RSA *rsa = EVP_PKEY_get0_RSA(pkey);
    HKS_IF_NULL_LOGE_RETURN(rsa, HKS_ERROR_NULL_POINTER,
        "EVP_PKEY_get1_RSA error %" LOG_PUBLIC "s", ERR_reason_error_string(ERR_get_error()))

    int nSize = BN_num_bytes(RSA_get0_n(rsa));
    int eSize = BN_num_bytes(RSA_get0_e(rsa));
    if (nSize <= 0 || eSize <= 0) {
        HKS_LOG_E("X509PublicKeyToRsa BN_num_bytes failed");
        return HKS_ERROR_INTERNAL_ERROR;
    }

    /* n and e in RSA algorithm is small, will never overflow. */
    uint32_t totalSize = (uint32_t)nSize + (uint32_t)eSize + sizeof(struct HksPubKeyInfo);
    uint8_t *keyBuffer = (uint8_t *)HksMalloc(totalSize);
    HKS_IF_NULL_LOGE_RETURN(keyBuffer, HKS_ERROR_MALLOC_FAIL, "X509PublicKeyToRsa keyBuffer failed")

    struct HksPubKeyInfo *pubKeyInfo = (struct HksPubKeyInfo *)keyBuffer;
    pubKeyInfo->keyAlg = HKS_ALG_RSA;
    pubKeyInfo->keySize = ((uint32_t)RSA_size(rsa)) * HKS_BITS_PER_BYTE;
    pubKeyInfo->nOrXSize = (uint32_t)nSize;
    pubKeyInfo->eOrYSize = (uint32_t)eSize;
    pubKeyInfo->placeHolder = 0;
    if (BN_bn2bin(RSA_get0_n(rsa), keyBuffer + sizeof(struct HksPubKeyInfo)) == 0 ||
        BN_bn2bin(RSA_get0_e(rsa), keyBuffer + sizeof(struct HksPubKeyInfo) + (uint32_t)nSize) == 0) {
        HKS_LOG_E("BN_bn2bin error %" LOG_PUBLIC "s", ERR_reason_error_string(ERR_get_error()));
        HKS_FREE_PTR(keyBuffer);
        return HKS_ERROR_INTERNAL_ERROR;
    }

    rsaPublicKey->data = keyBuffer;
    rsaPublicKey->size = totalSize;
    return HKS_SUCCESS;
}
#endif

#ifdef HKS_SUPPORT_ECC_C
static int32_t EcKeyToPublicKey(const uint32_t alg, EC_KEY *ecKey, struct HksBlob *eccPublicKey)
{
    BIGNUM *x = BN_new();
    BIGNUM *y = BN_new();
    int32_t ret;
    do {
        ret = HKS_ERROR_CRYPTO_ENGINE_ERROR;
        if (x == NULL || y == NULL) {
            HKS_LOG_E("X509PublicKeyToEcc BN_new failed");
            break;
        }

        if (EC_POINT_get_affine_coordinates_GFp(EC_KEY_get0_group(ecKey), EC_KEY_get0_public_key(ecKey), x, y, NULL) ==
            0) {
            HKS_LOG_E("EC_POINT_get_affine_coordinates_GFp error %" LOG_PUBLIC "s",
                ERR_reason_error_string(ERR_get_error()));
            break;
        }

        uint32_t keyLen = (uint32_t)EC_GROUP_order_bits(EC_KEY_get0_group(ecKey));
        uint32_t xSize = HKS_KEY_BYTES(keyLen);
        uint32_t ySize = HKS_KEY_BYTES(keyLen);

        if ((keyLen == 0) || (keyLen > HKS_ECC_KEY_SIZE_521)) {
            HKS_LOG_E("invalid ecc key length");
            break;
        }

        uint32_t totalSize = xSize + ySize + sizeof(struct HksPubKeyInfo);
        uint8_t *keyBuffer = (uint8_t *)HksMalloc(totalSize);
        HKS_IF_NULL_LOGE_BREAK(keyBuffer, "X509PublicKeyToRsa keyBuffer failed")

        struct HksPubKeyInfo *pubKeyInfo = (struct HksPubKeyInfo *)keyBuffer;
        pubKeyInfo->keyAlg = alg;
        pubKeyInfo->keySize = keyLen;
        pubKeyInfo->nOrXSize = xSize;
        pubKeyInfo->eOrYSize = ySize;
        pubKeyInfo->placeHolder = 0;
        if (BN_bn2binpad(x, keyBuffer + sizeof(struct HksPubKeyInfo), xSize) == 0 ||
            BN_bn2binpad(y, keyBuffer + sizeof(struct HksPubKeyInfo) + xSize, ySize) == 0) {
            HKS_LOG_E("BN_bn2binpad error %" LOG_PUBLIC "s", ERR_reason_error_string(ERR_get_error()));
            HKS_FREE_PTR(keyBuffer);
            break;
        }

        ret = HKS_SUCCESS;
        eccPublicKey->data = keyBuffer;
        eccPublicKey->size = totalSize;
    } while (0);

    SELF_FREE_PTR(x, BN_free);
    SELF_FREE_PTR(y, BN_free);
    return ret;
}

static int32_t X509PublicKeyToEcc(const uint32_t alg, EVP_PKEY *pkey, struct HksBlob *eccPublicKey)
{
    EC_KEY *ecKey = EVP_PKEY_get0_EC_KEY(pkey);
    HKS_IF_NULL_LOGE_RETURN(ecKey, HKS_ERROR_NULL_POINTER,
        "EVP_PKEY_get1_EC_KEY error %" LOG_PUBLIC "s", ERR_reason_error_string(ERR_get_error()))

    return EcKeyToPublicKey(alg, ecKey, eccPublicKey);
}
#endif

#ifdef HKS_SUPPORT_DSA_C

static int32_t GetDsaKeyInfo(const DSA *dsa, const BIGNUM **info, uint32_t *infoSize, GetDsaInfoFunc func)
{
    *info = func(dsa);
    HKS_IF_NULL_RETURN(*info, HKS_ERROR_NULL_POINTER)

    int size = BN_num_bytes(*info);
    if (size <= 0) {
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    *infoSize = (uint32_t)size;
    return HKS_SUCCESS;
}

static int32_t X509PublicKeyToDsa(EVP_PKEY *pkey, struct HksBlob *dsaPublicKey)
{
    DSA *dsa = EVP_PKEY_get0_DSA(pkey);
    HKS_IF_NULL_LOGE_RETURN(dsa, HKS_ERROR_NULL_POINTER,
        "EVP_PKEY_get1_DSA error %" LOG_PUBLIC "s", ERR_reason_error_string(ERR_get_error()))

    const BIGNUM *y = NULL;
    const BIGNUM *p = NULL;
    const BIGNUM *q = NULL;
    const BIGNUM *g = NULL;
    uint32_t ySize = 0;
    uint32_t pSize = 0;
    uint32_t qSize = 0;
    uint32_t gSize = 0;

    if (GetDsaKeyInfo(dsa, &y, &ySize, DSA_get0_pub_key) != HKS_SUCCESS ||
        GetDsaKeyInfo(dsa, &p, &pSize, DSA_get0_p) != HKS_SUCCESS ||
        GetDsaKeyInfo(dsa, &q, &qSize, DSA_get0_q) != HKS_SUCCESS ||
        GetDsaKeyInfo(dsa, &g, &gSize, DSA_get0_g) != HKS_SUCCESS) {
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    uint32_t totalSize = sizeof(struct KeyMaterialDsa) + ySize + pSize + qSize + gSize;
    uint8_t *keyBuffer = (uint8_t *)HksMalloc(totalSize);
    HKS_IF_NULL_RETURN(keyBuffer, HKS_ERROR_MALLOC_FAIL)

    if ((ySize > UINT32_MAX - HKS_BITS_PER_BYTE) ||
        ((ySize + HKS_BITS_PER_BYTE - 1) / HKS_BITS_PER_BYTE > UINT32_MAX / (HKS_BITS_PER_BYTE * HKS_BITS_PER_BYTE))) {
        HKS_FREE_PTR(keyBuffer);
        return HKS_ERROR_BAD_STATE;
    }

    struct KeyMaterialDsa *keyMaterial = (struct KeyMaterialDsa *)keyBuffer;
    keyMaterial->keyAlg = HKS_ALG_DSA;
    keyMaterial->keySize = (ySize + HKS_BITS_PER_BYTE - 1) / HKS_BITS_PER_BYTE * HKS_BITS_PER_BYTE * HKS_BITS_PER_BYTE;
    keyMaterial->xSize = 0;
    keyMaterial->ySize = ySize;
    keyMaterial->pSize = pSize;
    keyMaterial->qSize = qSize;
    keyMaterial->gSize = gSize;

    if ((BN_bn2bin(y, keyBuffer + sizeof(struct KeyMaterialDsa) + keyMaterial->xSize) == 0) ||
        (BN_bn2bin(p, keyBuffer + sizeof(struct KeyMaterialDsa) + keyMaterial->xSize + ySize) == 0) ||
        (BN_bn2bin(q, keyBuffer + sizeof(struct KeyMaterialDsa) + keyMaterial->xSize + ySize + pSize) == 0) ||
        (BN_bn2bin(g, keyBuffer + sizeof(struct KeyMaterialDsa) + keyMaterial->xSize + ySize + pSize + qSize) == 0)) {
        HKS_LOG_E("BN_bn2bin error %" LOG_PUBLIC "s", ERR_reason_error_string(ERR_get_error()));
        HKS_FREE_PTR(keyBuffer);
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }
    dsaPublicKey->size = totalSize;
    dsaPublicKey->data = keyBuffer;

    return HKS_SUCCESS;
}
#endif

#ifdef HKS_SUPPORT_DH_C
static int32_t X509PublicKeyToDh(EVP_PKEY *pkey, struct HksBlob *dhPublicKey)
{
    DH *dh = EVP_PKEY_get0_DH(pkey);
    HKS_IF_NULL_LOGE_RETURN(dh, HKS_ERROR_NULL_POINTER,
        "EVP_PKEY_get0_DH error %" LOG_PUBLIC "s", ERR_reason_error_string(ERR_get_error()))

    const BIGNUM *pubKey = DH_get0_pub_key(dh);
    HKS_IF_NULL_LOGE_RETURN(pubKey, HKS_ERROR_NULL_POINTER,
        "DH_get0_pub_key error %" LOG_PUBLIC "s", ERR_reason_error_string(ERR_get_error()))

    uint32_t pubKeySize = (uint32_t)BN_num_bytes(pubKey);
    if (pubKeySize > UINT32_MAX - sizeof(struct KeyMaterialDh)) {
        HKS_LOG_E("the size is too long, failed");
        return HKS_ERROR_BAD_STATE;
    }

    uint32_t totalSize = sizeof(struct KeyMaterialDh) + pubKeySize;
    uint8_t *keyBuffer = (uint8_t *)HksMalloc(totalSize);
    HKS_IF_NULL_LOGE_RETURN(keyBuffer, HKS_ERROR_MALLOC_FAIL, "alloc keyBuffer failed")
    struct KeyMaterialDh *keyMaterial = (struct KeyMaterialDh *)keyBuffer;
    keyMaterial->keyAlg = HKS_ALG_DH;
    keyMaterial->keySize = (uint32_t)DH_bits(dh);
    keyMaterial->pubKeySize = pubKeySize;
    keyMaterial->priKeySize = 0;
    keyMaterial->reserved = 0;

    BN_bn2bin(pubKey, keyBuffer + sizeof(struct KeyMaterialDh));

    dhPublicKey->size = totalSize;
    dhPublicKey->data = keyBuffer;

    return HKS_SUCCESS;
}
#endif

int32_t TranslateFromX509PublicKey(const uint32_t alg, const struct HksBlob *x509Key, struct HksBlob *publicKey)
{
    if (x509Key == NULL || x509Key->data == NULL || x509Key->size == 0 || publicKey == NULL) {
        HKS_LOG_E("TranslateFromX509PublicKey invalid args");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    uint8_t *data = x509Key->data;

    EVP_PKEY *pkey = d2i_PUBKEY(NULL, (const unsigned char **)&data, x509Key->size);
    HKS_IF_NULL_RETURN(pkey, HKS_ERROR_INVALID_ARGUMENT)

    int32_t ret;
    int32_t keyType = EVP_PKEY_base_id(pkey);
    if (keyType == EVP_PKEY_RSA) {
#ifdef HKS_SUPPORT_RSA_C
        ret = X509PublicKeyToRsa(pkey, publicKey);
#else
        ret = HKS_ERROR_INVALID_ALGORITHM;
#endif
    } else if (keyType == EVP_PKEY_EC) {
#if defined(HKS_SUPPORT_ECC_C) || defined(HKS_SUPPORT_SM2_C)
        ret = X509PublicKeyToEcc(alg, pkey, publicKey);
#else
        ret = HKS_ERROR_INVALID_ALGORITHM;
#endif
    } else if (keyType == EVP_PKEY_DSA) {
#ifdef HKS_SUPPORT_DSA_C
        ret = X509PublicKeyToDsa(pkey, publicKey);
#else
        ret = HKS_ERROR_INVALID_ALGORITHM;
#endif
    } else if (keyType == EVP_PKEY_DH) {
#ifdef HKS_SUPPORT_DH_C
        ret = X509PublicKeyToDh(pkey, publicKey);
#else
        ret = HKS_ERROR_INVALID_ALGORITHM;
#endif
    } else {
        HKS_LOG_E("Unsupport alg type!");
        ret = HKS_ERROR_INVALID_ARGUMENT;
    }

    SELF_FREE_PTR(pkey, EVP_PKEY_free);
    return ret;
}
#endif

