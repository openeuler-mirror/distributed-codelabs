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

#include <mbedtls/bignum.h>
#include <mbedtls/ecp.h>
#include <mbedtls/pk.h>
#include <mbedtls/rsa.h>

#include "hks_log.h"
#include "hks_mbedtls_common.h"
#include "hks_mbedtls_ecc.h"
#include "hks_mem.h"
#include "hks_template.h"

#ifdef _CUT_AUTHENTICATE_
#undef HKS_SUPPORT_HASH_C
#undef HKS_SUPPORT_RSA_C
#undef HKS_SUPPORT_ECC_C
#undef HKS_SUPPORT_X25519_C
#undef HKS_SUPPORT_ED25519_C
#undef HKS_SUPPORT_KDF_PBKDF2
#endif /* _CUT_AUTHENTICATE_ */

#if defined(HKS_SUPPORT_RSA_C) || defined(HKS_SUPPORT_ECC_C)
static int32_t PkCtxToX509(mbedtls_pk_context *ctx, struct HksBlob *x509Key)
{
    uint8_t *tmpBuf = (uint8_t *)HksMalloc(MAX_KEY_SIZE);
    HKS_IF_NULL_RETURN(tmpBuf, HKS_ERROR_MALLOC_FAIL)

    int32_t ret = HKS_SUCCESS;
    do {
        int32_t x509Size = mbedtls_pk_write_pubkey_der(ctx, tmpBuf, MAX_KEY_SIZE);
        if (x509Size < HKS_MBEDTLS_SUCCESS) {
            HKS_LOG_E("public key to x509 write der failed! mbedtls ret = 0x%" LOG_PUBLIC "X", x509Size);
            ret = x509Size;
            break;
        }

        uint8_t *x509KeyData = (uint8_t *)HksMalloc(x509Size);
        if (x509KeyData == NULL) {
            HKS_LOG_E("Malloc x509 key data failed!");
            ret = HKS_ERROR_MALLOC_FAIL;
            break;
        }

        /* mbedtls_pk_write_pubkey_der use little-endian for storage */
        if (memcpy_s(x509KeyData, x509Size, tmpBuf + MAX_KEY_SIZE - x509Size, x509Size) != EOK) {
            HKS_LOG_E("public key to x509 memcpy to x509key failed!");
            HKS_FREE_PTR(x509KeyData);
            ret = HKS_ERROR_INSUFFICIENT_MEMORY;
            break;
        }

        x509Key->size = x509Size;
        x509Key->data = x509KeyData;
    } while (0);

    HKS_FREE_PTR(tmpBuf);
    return ret;
}

#ifdef HKS_SUPPORT_RSA_C
static int32_t InitRsaPkCtx(const struct HksBlob *mod, const struct HksBlob *e, mbedtls_pk_context *ctx)
{
    int32_t ret = mbedtls_pk_setup(ctx, mbedtls_pk_info_from_type(MBEDTLS_PK_RSA));
    if (ret != HKS_MBEDTLS_SUCCESS) {
        HKS_LOG_E("Mbedtls setup pk context failed! mbedtls ret = 0x%" LOG_PUBLIC "X", ret);
        return ret;
    }

    mbedtls_mpi mpiN;
    mbedtls_mpi mpiE;

    mbedtls_mpi_init(&mpiN);
    mbedtls_mpi_init(&mpiE);

    do {
        ret = mbedtls_mpi_read_binary(&mpiN, mod->data, mod->size);
        if (ret != HKS_MBEDTLS_SUCCESS) {
            HKS_LOG_E("Hks init rsa pk context read N failed! mbedtls ret = 0x%" LOG_PUBLIC "X", ret);
            break;
        }

        ret = mbedtls_mpi_read_binary(&mpiE, e->data, e->size);
        if (ret != HKS_MBEDTLS_SUCCESS) {
            HKS_LOG_E("Hks init rsa pk context read E failed! mbedtls ret = 0x%" LOG_PUBLIC "X", ret);
            break;
        }

        mbedtls_rsa_context *rsaCtx = mbedtls_pk_rsa(*ctx);
        ret = mbedtls_rsa_import(rsaCtx, &mpiN, NULL, NULL, NULL, &mpiE);
        if (ret != HKS_MBEDTLS_SUCCESS) {
            HKS_LOG_E("Hks init rsa pk context import rsa context failed! mbedtls ret = 0x%" LOG_PUBLIC "X", ret);
            break;
        }

        ret = mbedtls_rsa_complete(rsaCtx);
        if (ret != HKS_MBEDTLS_SUCCESS) {
            HKS_LOG_E("Hks init rsa pk context complete rsa context failed! mbedtls ret = 0x%" LOG_PUBLIC "X", ret);
        }
    } while (0);

    mbedtls_mpi_free(&mpiN);
    mbedtls_mpi_free(&mpiE);
    return ret;
}

static int32_t RsaToX509PublicKey(const struct HksBlob *mod, const struct HksBlob *e, struct HksBlob *x509Key)
{
    mbedtls_pk_context ctx;
    mbedtls_pk_init(&ctx);

    int32_t ret;
    do {
        ret = InitRsaPkCtx(mod, e, &ctx);
        HKS_IF_NOT_SUCC_BREAK(ret)

        ret = PkCtxToX509(&ctx, x509Key);
        HKS_IF_NOT_SUCC_LOGE(ret, "Pk context to rsa x509 failed! ret = 0x%" LOG_PUBLIC "X", ret)
    } while (0);

    mbedtls_pk_free(&ctx);
    return ret;
}
#endif

#ifdef HKS_SUPPORT_ECC_C
static int32_t InitEccPkCtx(uint32_t keySize, const struct HksBlob *x, const struct HksBlob *y,
    mbedtls_pk_context *ctx)
{
    int32_t ret = mbedtls_pk_setup(ctx, mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY));
    if (ret != HKS_MBEDTLS_SUCCESS) {
        HKS_LOG_E("Mbedtls setup pk context failed! mbedtls ret = 0x%" LOG_PUBLIC "X", ret);
        return ret;
    }

    mbedtls_ecp_group_id grp_id;
    ret = GetEccGroupId(keySize, &grp_id);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "Get ecc group id failed! ret = 0x%" LOG_PUBLIC "X", ret)

    mbedtls_ecp_keypair *pubKey = mbedtls_pk_ec(*ctx);
    ret = mbedtls_ecp_group_load(&(pubKey->MBEDTLS_PRIVATE(grp)), grp_id);
    if (ret != HKS_MBEDTLS_SUCCESS) {
        HKS_LOG_E("Hks init ecc pk context load group failed! mbedtls ret = 0x%" LOG_PUBLIC "X", ret);
        return ret;
    }

    ret = mbedtls_mpi_read_binary(&(pubKey->MBEDTLS_PRIVATE(Q).MBEDTLS_PRIVATE(X)), x->data, x->size);
    if (ret != HKS_MBEDTLS_SUCCESS) {
        HKS_LOG_E("Hks init ecc pk context read X failed! mbedtls ret = 0x%" LOG_PUBLIC "X", ret);
        return ret;
    }

    ret = mbedtls_mpi_read_binary(&(pubKey->MBEDTLS_PRIVATE(Q).MBEDTLS_PRIVATE(Y)), y->data, y->size);
    if (ret != HKS_MBEDTLS_SUCCESS) {
        HKS_LOG_E("Hks init ecc pk context read Y failed! mbedtls ret = 0x%" LOG_PUBLIC "X", ret);
        return ret;
    }

    /* Z = 1, X and Y are its standard (affine) coordinates */
    ret = mbedtls_mpi_lset(&(pubKey->MBEDTLS_PRIVATE(Q).MBEDTLS_PRIVATE(Z)), 1);
    if (ret != HKS_MBEDTLS_SUCCESS) {
        HKS_LOG_E("Hks init ecc pk context set Z failed! mbedtls ret = 0x%" LOG_PUBLIC "X", ret);
        return ret;
    }

    return HKS_SUCCESS;
}

static int32_t EccToX509PublicKey(uint32_t keySize, const struct HksBlob *x, const struct HksBlob *y,
    struct HksBlob *x509Key)
{
    mbedtls_pk_context ctx;
    mbedtls_pk_init(&ctx);

    int32_t ret;
    do {
        ret = InitEccPkCtx(keySize, x, y, &ctx);
        HKS_IF_NOT_SUCC_BREAK(ret)

        ret = PkCtxToX509(&ctx, x509Key);
        HKS_IF_NOT_SUCC_LOGE(ret, "Pk context to ecc x509 failed! ret = 0x%" LOG_PUBLIC "X", ret)
    } while (0);

    mbedtls_pk_free(&ctx);
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

    if (memcpy_s(x509Key->data, publicKey->size, publicKey->data, publicKey->size) != EOK) {
        HKS_LOG_E("X25519/Ed25519 to x509 public key memcpy to x509 key data failed!");
        HKS_FREE_PTR(x509Key->data);
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    x509Key->size = publicKey->size;

    return HKS_SUCCESS;
}
#endif

int32_t TranslateToX509PublicKey(const struct HksBlob *publicKey, struct HksBlob *x509Key)
{
#if defined(HKS_SUPPORT_RSA_C) || defined(HKS_SUPPORT_ECC_C) || defined(HKS_SUPPORT_X25519_C) || \
    defined(HKS_SUPPORT_ED25519_C)
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

#if defined(HKS_SUPPORT_RSA_C) || defined(HKS_SUPPORT_ECC_C)
    struct HksBlob material2 = { publicKeyInfo->eOrYSize, publicKey->data + offset };
#endif
    switch (publicKeyInfo->keyAlg) {
#ifdef HKS_SUPPORT_RSA_C
        case HKS_ALG_RSA:
            return RsaToX509PublicKey(&material1, &material2, x509Key);
#endif
#ifdef HKS_SUPPORT_ECC_C
        case HKS_ALG_ECC:
            return EccToX509PublicKey(publicKeyInfo->keySize, &material1, &material2, x509Key);
#endif
#if defined(HKS_SUPPORT_X25519_C) || defined(HKS_SUPPORT_ED25519_C)
        case HKS_ALG_X25519:
        case HKS_ALG_ED25519:
            return Curve25519ToX509PublicKey(&material1, x509Key);
#endif
        default:
            HKS_LOG_E("Unsupport alg type! type = 0x%" LOG_PUBLIC "X", publicKeyInfo->keyAlg);
            return HKS_ERROR_INVALID_ARGUMENT;
    }
#else
    (void)publicKey;
    (void)x509Key;
    return HKS_ERROR_NOT_SUPPORTED;
#endif
}

#if defined(HKS_SUPPORT_RSA_C) || defined(HKS_SUPPORT_ECC_C)
#ifdef HKS_SUPPORT_RSA_C
static int32_t CheckRsaCtx(const mbedtls_rsa_context *rsaCtx)
{
    uint32_t maxKeyByteLen = HKS_RSA_KEY_SIZE_4096 / HKS_BITS_PER_BYTE;
    if (rsaCtx->MBEDTLS_PRIVATE(len) > maxKeyByteLen) {
        HKS_LOG_E("Invalid mbedtls rsa context's len! len = 0x%" LOG_PUBLIC "X", rsaCtx->MBEDTLS_PRIVATE(len));
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    return HKS_SUCCESS;
}

static int32_t X509PublicKeyToRsa(mbedtls_rsa_context *rsaCtx, struct HksBlob *rsaPublicKey)
{
    int32_t ret = CheckRsaCtx(rsaCtx);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "Check rsa ctx failed! ret = 0x%" LOG_PUBLIC "X", ret)

    uint32_t nSize = rsaCtx->MBEDTLS_PRIVATE(len);
    uint32_t eSize = rsaCtx->MBEDTLS_PRIVATE(len);

    uint32_t totalSize = sizeof(struct HksPubKeyInfo) + nSize + eSize;
    uint8_t *keyBuffer = (uint8_t *)HksMalloc(totalSize);
    HKS_IF_NULL_LOGE_RETURN(keyBuffer, HKS_ERROR_MALLOC_FAIL,
        "X509 public key to rsa malloc keyBuffer failed!")

    struct HksPubKeyInfo *pubKeyInfo = (struct HksPubKeyInfo *)keyBuffer;
    pubKeyInfo->keyAlg = HKS_ALG_RSA;
    pubKeyInfo->keySize = rsaCtx->MBEDTLS_PRIVATE(len) * HKS_BITS_PER_BYTE;
    pubKeyInfo->nOrXSize = nSize;
    pubKeyInfo->eOrYSize = eSize;
    pubKeyInfo->placeHolder = 0;

    ret = mbedtls_mpi_write_binary(&rsaCtx->MBEDTLS_PRIVATE(N), keyBuffer + sizeof(struct HksPubKeyInfo), nSize);
    if (ret != HKS_MBEDTLS_SUCCESS) {
        HKS_LOG_E("X509 public key to rsa write N failed! mbedtls ret = 0x%" LOG_PUBLIC "X", ret);
        HKS_FREE_PTR(keyBuffer);
        return ret;
    }

    ret = mbedtls_mpi_write_binary(&rsaCtx->MBEDTLS_PRIVATE(E),
        keyBuffer + sizeof(struct HksPubKeyInfo) + nSize, eSize);
    if (ret != HKS_MBEDTLS_SUCCESS) {
        HKS_LOG_E("X509 public key to rsa write E failed! mbedtls ret = 0x%" LOG_PUBLIC "X", ret);
        HKS_FREE_PTR(keyBuffer);
        return ret;
    }

    rsaPublicKey->data = keyBuffer;
    rsaPublicKey->size = totalSize;

    return HKS_SUCCESS;
}
#endif

#ifdef HKS_SUPPORT_ECC_C
static int32_t CheckEccXySize(const uint32_t xSize, const uint32_t ySize)
{
    uint32_t maxKeyByteLen = HKS_ECC_KEY_SIZE_521 / HKS_BITS_PER_BYTE;
    if ((xSize > maxKeyByteLen) || (ySize > maxKeyByteLen)) {
        HKS_LOG_E("Invalid ecc public key size! xSize = 0x%" LOG_PUBLIC "X, ySize = 0x%" LOG_PUBLIC "X", xSize, ySize);
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    return HKS_SUCCESS;
}

static int32_t X509PublicKeyToEcc(mbedtls_ecp_keypair *pubKey, struct HksBlob *eccPublicKey)
{
    /* When converting from X509 to internal format, the first byte needs to be filled with 0 */
    uint32_t xSize = mbedtls_mpi_size(&(pubKey->MBEDTLS_PRIVATE(grp).P));
    uint32_t ySize = mbedtls_mpi_size(&(pubKey->MBEDTLS_PRIVATE(grp).P));

    int32_t ret = CheckEccXySize(xSize, ySize);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "Check ecc public key size failed! ret = 0x%" LOG_PUBLIC "X", ret)

    uint32_t totalSize = sizeof(struct HksPubKeyInfo) + xSize + ySize;
    uint8_t *keyBuffer = (uint8_t *)HksMalloc(totalSize);
    HKS_IF_NULL_LOGE_RETURN(keyBuffer, HKS_ERROR_MALLOC_FAIL,
        "X509 public key to ecc malloc keyBuffer failed!")

    if (mbedtls_mpi_size(&(pubKey->MBEDTLS_PRIVATE(grp).P)) > UINT32_MAX / HKS_BITS_PER_BYTE) {
        HKS_FREE_PTR(keyBuffer);
        HKS_LOG_E("invalid param, the size is :%" LOG_PUBLIC "u", mbedtls_mpi_size(&(pubKey->MBEDTLS_PRIVATE(grp).P)));
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    struct HksPubKeyInfo *pubKeyInfo = (struct HksPubKeyInfo *)keyBuffer;
    pubKeyInfo->keyAlg = HKS_ALG_ECC;
    pubKeyInfo->keySize = mbedtls_mpi_size(&(pubKey->MBEDTLS_PRIVATE(grp).P)) * HKS_BITS_PER_BYTE;
    pubKeyInfo->nOrXSize = xSize;
    pubKeyInfo->eOrYSize = ySize;
    pubKeyInfo->placeHolder = 0;

    ret = mbedtls_mpi_write_binary(&(pubKey->MBEDTLS_PRIVATE(Q).MBEDTLS_PRIVATE(X)),
                                   keyBuffer + sizeof(struct HksPubKeyInfo), xSize);
    if (ret != HKS_MBEDTLS_SUCCESS) {
        HKS_LOG_E("X509 public key to ecc write X failed! mbedtls ret = 0x%" LOG_PUBLIC "X", ret);
        HKS_FREE_PTR(keyBuffer);
        return ret;
    }

    ret = mbedtls_mpi_write_binary(&(pubKey->MBEDTLS_PRIVATE(Q).MBEDTLS_PRIVATE(Y)),
                                    keyBuffer + sizeof(struct HksPubKeyInfo) + xSize, ySize);
    if (ret != HKS_MBEDTLS_SUCCESS) {
        HKS_LOG_E("X509 public key to ecc write Y failed! mbedtls ret = 0x%" LOG_PUBLIC "X", ret);
        HKS_FREE_PTR(keyBuffer);
        return ret;
    }

    eccPublicKey->data = keyBuffer;
    eccPublicKey->size = totalSize;

    return HKS_SUCCESS;
}
#endif
#endif

int32_t TranslateFromX509PublicKey(const uint32_t alg, const struct HksBlob *x509Key, struct HksBlob *publicKey)
{
    (void)alg;
#if defined(HKS_SUPPORT_RSA_C) || defined(HKS_SUPPORT_ECC_C)
    mbedtls_pk_context ctx;
    mbedtls_pk_init(&ctx);

    int32_t ret = mbedtls_pk_parse_public_key(&ctx, x509Key->data, x509Key->size);
    if (ret != HKS_MBEDTLS_SUCCESS) {
        mbedtls_pk_free(&ctx);
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    /* 1: if the context can do operations on the given type. */
    if (mbedtls_pk_can_do(&ctx, MBEDTLS_PK_RSA) == 1) {
#ifdef HKS_SUPPORT_RSA_C
        ret = X509PublicKeyToRsa(mbedtls_pk_rsa(ctx), publicKey);
#else
        ret = HKS_ERROR_INVALID_ALGORITHM;
#endif
    } else if (mbedtls_pk_can_do(&ctx, MBEDTLS_PK_ECKEY) == 1) {
#ifdef HKS_SUPPORT_ECC_C
        ret = X509PublicKeyToEcc(mbedtls_pk_ec(ctx), publicKey);
#else
        ret = HKS_ERROR_INVALID_ALGORITHM;
#endif
    } else {
        HKS_LOG_E("Unsupport alg type!");
        ret = HKS_ERROR_INVALID_ARGUMENT;
    }

    mbedtls_pk_free(&ctx);
    return ret;
#else
    (void)publicKey;
    (void)x509Key;
    return HKS_ERROR_NOT_SUPPORTED;
#endif
}

