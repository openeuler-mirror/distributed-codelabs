/*
 * Copyright (c) 2020-2022 Huawei Device Co., Ltd.
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

#include "mbedtls/compat-2.x.h"
#ifdef HKS_CONFIG_FILE
#include HKS_CONFIG_FILE
#else
#include "hks_config.h"
#endif

#ifdef _CUT_AUTHENTICATE_
#undef HKS_SUPPORT_HASH_C
#endif /* _CUT_AUTHENTICATE_ */

#ifdef HKS_SUPPORT_HASH_C

#include "hks_mbedtls_hash.h"

#include <mbedtls/md5.h>
#include <mbedtls/sha1.h>
#include <mbedtls/sha256.h>
#include <mbedtls/sha512.h>

#include "hks_common_check.h"
#include "hks_log.h"
#include "hks_mem.h"
#include "hks_mbedtls_common.h"
#include "hks_template.h"

struct HksMbedtlsHashCtx {
    uint8_t *append;
    uint32_t mAlg;
} HksMbedtlsHashCtx;

static int32_t HksMbedtlsHashMd5Init(void **ctx, uint32_t alg)
{
    int32_t ret = 0;
    mbedtls_md5_context *context = (mbedtls_md5_context *)HksMalloc(sizeof(mbedtls_md5_context));
    HKS_IF_NULL_LOGE_RETURN(context, HKS_ERROR_MALLOC_FAIL, "malloc fail")

    mbedtls_md5_init(context);

    ret = mbedtls_md5_starts_ret(context);
    if (ret != HKS_MBEDTLS_SUCCESS) {
        HKS_LOG_E("Mbedtls Hash Md5 init starts_ret fail");
        mbedtls_md5_free(context);
        if (context != NULL) {
            HksFree(context);
        }
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    struct HksMbedtlsHashCtx *outCtx = (struct HksMbedtlsHashCtx *)HksMalloc(sizeof(HksMbedtlsHashCtx));
    if (outCtx == NULL) {
        HKS_LOG_E("Mbedtls Hash Md5 init malloc fail");
        mbedtls_md5_free(context);
        if (context != NULL) {
            HksFree(context);
        }
        return HKS_ERROR_MALLOC_FAIL;
    }

    outCtx->append = (void *)context;
    outCtx->mAlg = alg;
    *ctx = (void *)outCtx;
    return HKS_SUCCESS;
}

static int32_t HksMbedtlsHashMd5Update(struct HksMbedtlsHashCtx *ctx, const unsigned char *input, size_t ilen)
{
    int32_t ret = 0;
    mbedtls_md5_context *context = (mbedtls_md5_context *)ctx->append;

    HKS_IF_NULL_RETURN(context, HKS_ERROR_NULL_POINTER)

    ret = mbedtls_md5_update_ret(context, input, ilen);
    if (ret != HKS_MBEDTLS_SUCCESS) {
        HKS_LOG_E("Mbedtls Hash Md5 update fail");
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    return HKS_SUCCESS;
}

static int32_t HksMbedtlsHashMd5Final(struct HksMbedtlsHashCtx *ctx, const struct HksBlob *msg, struct HksBlob *hash)
{
    int32_t ret = 0;
    mbedtls_md5_context *context = (mbedtls_md5_context *)ctx->append;

    HKS_IF_NULL_RETURN(context, HKS_ERROR_NULL_POINTER)

    do {
        if (msg->size != 0) {
            ret = mbedtls_md5_update_ret(context, msg->data, msg->size);
            if (ret != HKS_MBEDTLS_SUCCESS) {
                HKS_LOG_E("Mbedtls Hash Md5 finish last data fail");
                ret = HKS_ERROR_CRYPTO_ENGINE_ERROR;
                break;
            }
        }

        ret = mbedtls_md5_finish_ret(context, hash->data);
        if (ret != HKS_MBEDTLS_SUCCESS) {
            HKS_LOG_E("Mbedtls Hash Md5 finish fail");
            ret = HKS_ERROR_CRYPTO_ENGINE_ERROR;
            break;
        }

        ret = HKS_SUCCESS;
    } while (0);

    mbedtls_md5_free(context);
    HksFree(context);
    ctx->append = NULL;

    return ret;
}

static int32_t HksMbedtlsHashSha1Init(void **ctx, uint32_t alg)
{
    int32_t ret = 0;
    mbedtls_sha1_context *context = (mbedtls_sha1_context *)HksMalloc(sizeof(mbedtls_sha1_context));
    HKS_IF_NULL_LOGE_RETURN(context, HKS_ERROR_MALLOC_FAIL, "malloc fail")
    mbedtls_sha1_init(context);

    ret = mbedtls_sha1_starts_ret(context);
    if (ret != HKS_MBEDTLS_SUCCESS) {
        HKS_LOG_E("Mbedtls Hash Sha1 init start_rat fail");
        mbedtls_sha1_free(context);
        if (context != NULL) {
            HksFree(context);
        }
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    struct HksMbedtlsHashCtx *outCtx = (struct HksMbedtlsHashCtx *)HksMalloc(sizeof(HksMbedtlsHashCtx));
    if (outCtx == NULL) {
        HKS_LOG_E("Mbedtls Hash Sha1 init malloc fail");
        mbedtls_sha1_free(context);
        if (context != NULL) {
            HksFree(context);
        }
        return HKS_ERROR_MALLOC_FAIL;
    }

    outCtx->append = (void *)context;
    outCtx->mAlg = alg;
    *ctx = (void *)outCtx;

    return HKS_SUCCESS;
}

static int32_t HksMbedtlsHashSha1Update(struct HksMbedtlsHashCtx *ctx, const unsigned char *input, size_t ilen)
{
    mbedtls_sha1_context *context = (mbedtls_sha1_context *)ctx->append;
    HKS_IF_NULL_RETURN(context, HKS_ERROR_NULL_POINTER)

    if (ilen == 0 || input == NULL) {
        HKS_LOG_E("Mbedtls Hash sha1 input param error");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    int32_t ret = mbedtls_sha1_update_ret(context, input, ilen);
    if (ret != HKS_MBEDTLS_SUCCESS) {
        HKS_LOG_E("Mbedtls Hash sha1 update fail");
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    return HKS_SUCCESS;
}

static int32_t HksMbedtlsHashSha1Final(struct HksMbedtlsHashCtx *ctx, const struct HksBlob *msg, struct HksBlob *hash)
{
    mbedtls_sha1_context *context = (mbedtls_sha1_context *)ctx->append;
    HKS_IF_NULL_RETURN(context, HKS_ERROR_NULL_POINTER)

    int32_t ret = 0;
    do {
        if (hash->data == NULL) {
            HKS_LOG_E("Mbedtls Hash sha1 output param error");
            ret = HKS_ERROR_INVALID_ARGUMENT;
            break;
        }

        if (msg->size != 0) {
            ret = mbedtls_sha1_update_ret(context, msg->data, msg->size);
            if (ret != HKS_MBEDTLS_SUCCESS) {
                HKS_LOG_E("Mbedtls Hash sha1 finish last data fail");
                ret = HKS_ERROR_CRYPTO_ENGINE_ERROR;
                break;
            }
        }

        ret = mbedtls_sha1_finish_ret(context, hash->data);
        if (ret != HKS_MBEDTLS_SUCCESS) {
            HKS_LOG_E("Mbedtls Hash sha1 finish fail");
            ret = HKS_ERROR_CRYPTO_ENGINE_ERROR;
            break;
        }

        ret = HKS_SUCCESS;
    } while (0);
    mbedtls_sha1_free(context);
    HksFree(context);
    ctx->append = NULL;
    return ret;
}

static int32_t HksMbedtlsHashSha256Init(void **ctx, int is224, uint32_t alg)
{
    mbedtls_sha256_context *context = (mbedtls_sha256_context *)HksMalloc(sizeof(mbedtls_sha256_context));
    HKS_IF_NULL_LOGE_RETURN(context, HKS_ERROR_MALLOC_FAIL, "malloc fail")

    if (is224 != 0 && is224 != 1) {
        HKS_LOG_E("Mbedtls Hash not sha224 & not sha256 ");
        if (context != NULL) {
            HksFree(context);
        }
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    mbedtls_sha256_init(context);

    int32_t ret = mbedtls_sha256_starts_ret(context, is224);
    if (ret != HKS_MBEDTLS_SUCCESS) {
        HKS_LOG_E("Mbedtls Hash sha256 init fail");
        mbedtls_sha256_free(context);
        if (context != NULL) {
            HksFree(context);
        }
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    struct HksMbedtlsHashCtx *outCtx = (struct HksMbedtlsHashCtx *)HksMalloc(sizeof(HksMbedtlsHashCtx));
    if (outCtx == NULL) {
        HKS_LOG_E("Mbedtls Hash sha256 init fail_outCtx malloc fail");
        mbedtls_sha256_free(context);
        if (context != NULL) {
            HksFree(context);
        }
        return HKS_ERROR_MALLOC_FAIL;
    }

    outCtx->append = (void *)context;
    outCtx->mAlg = alg;
    *ctx = (void *)outCtx;
    return HKS_SUCCESS;
}

static int32_t HksMbedtlsHashSha256Update(struct HksMbedtlsHashCtx *ctx, const unsigned char *input, size_t ilen)
{
    mbedtls_sha256_context *context = (mbedtls_sha256_context *)ctx->append;
    HKS_IF_NULL_RETURN(context, HKS_FAILURE)

    if (ilen == 0 || input == NULL) {
        return HKS_FAILURE;
    }

    int32_t ret = mbedtls_sha256_update_ret(context, input, ilen);
    if (ret != HKS_MBEDTLS_SUCCESS) {
        HKS_LOG_E("Mbedtls Hash sha256 update fail");
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    return HKS_SUCCESS;
}

static int32_t HksMbedtlsHashSha256Final(struct HksMbedtlsHashCtx *ctx, const struct HksBlob *msg,
    struct HksBlob *hash)
{
    mbedtls_sha256_context *context = (mbedtls_sha256_context *)ctx->append;
    HKS_IF_NULL_RETURN(context, HKS_ERROR_NULL_POINTER)

    int32_t ret = 0;
    do {
        if (hash->data == NULL) {
            HKS_LOG_E("Mbedtls Hash sha256 output param error");
            ret = HKS_ERROR_INVALID_ARGUMENT;
            break;
        }

        if (msg->size != 0) {
            ret = mbedtls_sha256_update_ret(context, msg->data, msg->size);
            if (ret != HKS_MBEDTLS_SUCCESS) {
                HKS_LOG_E("Mbedtls Hash sha256 update fail");
                ret = HKS_ERROR_CRYPTO_ENGINE_ERROR;
                break;
            }
        }

        ret = mbedtls_sha256_finish_ret(context, hash->data);
        if (ret != HKS_MBEDTLS_SUCCESS) {
            HKS_LOG_E("Mbedtls Hash sha256 finish fail");
            ret = HKS_ERROR_CRYPTO_ENGINE_ERROR;
            break;
        }
        ret = HKS_SUCCESS;
    } while (0);

    mbedtls_sha256_free(context);
    HksFree(context);
    ctx->append = NULL;

    return ret;
}

static int32_t HksMbedtlsHashSha512Init(void **ctx, int is384, uint32_t alg)
{
    mbedtls_sha512_context *context = (mbedtls_sha512_context *)HksMalloc(sizeof(mbedtls_sha512_context));
    HKS_IF_NULL_LOGE_RETURN(context, HKS_ERROR_MALLOC_FAIL, "malloc fail")

    if (is384 != 0 && is384 != 1) {
        HKS_LOG_E("Mbedtls Hash not sha384 & not sha512 ");
        if (context != NULL) {
            HksFree(context);
        }
        return HKS_ERROR_BAD_STATE;
    }

    mbedtls_sha512_init(context);

    int32_t ret = mbedtls_sha512_starts_ret(context, is384);
    if (ret != HKS_MBEDTLS_SUCCESS) {
        HKS_LOG_E("Mbedtls Hash sha512 init fail");
        mbedtls_sha512_free(context);
        if (context != NULL) {
            HksFree(context);
        }
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    struct HksMbedtlsHashCtx *outCtx = (struct HksMbedtlsHashCtx *)HksMalloc(sizeof(HksMbedtlsHashCtx));
    if (outCtx == NULL) {
        HKS_LOG_E("Mbedtls Hash Md5 init fail_outCtx malloc fail");
        mbedtls_sha512_free(context);
        if (context != NULL) {
            HksFree(context);
        }
        return HKS_ERROR_MALLOC_FAIL;
    }

    outCtx->append = (void *)context;
    outCtx->mAlg = alg;
    *ctx = (void *)outCtx;
    return HKS_SUCCESS;
}

static int32_t HksMbedtlsHashSha512Update(struct HksMbedtlsHashCtx *ctx, const unsigned char *input, size_t ilen)
{
    mbedtls_sha512_context *context = (mbedtls_sha512_context *)ctx->append;
    HKS_IF_NULL_RETURN(context, HKS_ERROR_NULL_POINTER)

    if (ilen == 0 || input == NULL) {
        HKS_LOG_E("Mbedtls Hash sha512 input param error");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    int32_t ret = mbedtls_sha512_update_ret(context, input, ilen);
    if (ret != HKS_MBEDTLS_SUCCESS) {
        HKS_LOG_E("Mbedtls Hash sha512 update fail");
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    return HKS_SUCCESS;
}

static int32_t HksMbedtlsHashSha512Final(struct HksMbedtlsHashCtx *ctx, const struct HksBlob *msg,
    struct HksBlob *hash)
{
    mbedtls_sha512_context *context = (mbedtls_sha512_context *)ctx->append;
    HKS_IF_NULL_RETURN(context, HKS_ERROR_NULL_POINTER)

    int32_t ret = 0;
    do {
        if (hash->data == NULL) {
            HKS_LOG_E("Mbedtls Hash sha512 output param error");
            ret = HKS_ERROR_INVALID_ARGUMENT;
            break;
        }

        if (msg->size != 0) {
            ret = mbedtls_sha512_update_ret(context, msg->data, msg->size);
            if (ret != HKS_MBEDTLS_SUCCESS) {
                HKS_LOG_E("Mbedtls Hash sha512 finish last data fail");
                ret = HKS_ERROR_CRYPTO_ENGINE_ERROR;
                break;
            }
        }

        ret = mbedtls_sha512_finish_ret(context, hash->data);
        if (ret != HKS_MBEDTLS_SUCCESS) {
            HKS_LOG_E("Mbedtls Hash sha512 finish fail");
            ret = HKS_ERROR_CRYPTO_ENGINE_ERROR;
            break;
        }

        ret = HKS_SUCCESS;
    } while (0);

    mbedtls_sha512_free(context);
    HksFree(context);
    ctx->append = NULL;
    return ret;
}

static void HksMbedtlsMd5HashFreeCtx(void **cryptoCtx)
{
    if (cryptoCtx == NULL || *cryptoCtx == NULL) {
        HKS_LOG_E("Mbedtls Hash freeCtx param error");
        return;
    }
    struct HksMbedtlsHashCtx *hashCtx = (struct HksMbedtlsHashCtx *)*cryptoCtx;
    if (hashCtx->append != NULL) {
        mbedtls_md5_free((mbedtls_md5_context *)hashCtx->append);
    }
}

static void HksMbedtlsSHA1HashFreeCtx(void **cryptoCtx)
{
    if (cryptoCtx == NULL || *cryptoCtx == NULL) {
        HKS_LOG_E("Mbedtls Hash freeCtx param error");
        return;
    }
    struct HksMbedtlsHashCtx *hashCtx = (struct HksMbedtlsHashCtx *)*cryptoCtx;
    if (hashCtx->append != NULL) {
        mbedtls_sha1_free((mbedtls_sha1_context *)hashCtx->append);
    }
}

static void HksMbedtlsSha224Sha256HashFreeCtx(void **cryptoCtx)
{
    if (cryptoCtx == NULL || *cryptoCtx == NULL) {
        HKS_LOG_E("Mbedtls Hash freeCtx param error");
        return;
    }
    struct HksMbedtlsHashCtx *hashCtx = (struct HksMbedtlsHashCtx *)*cryptoCtx;
    if (hashCtx->append != NULL) {
        mbedtls_sha256_free((mbedtls_sha256_context *)hashCtx->append);
    }
}

static void HksMbedtlsSha384Sha512HashFreeCtx(void **cryptoCtx)
{
    if (cryptoCtx == NULL || *cryptoCtx == NULL) {
        HKS_LOG_E("Mbedtls Hash freeCtx param error");
        return;
    }
    struct HksMbedtlsHashCtx *hashCtx = (struct HksMbedtlsHashCtx *)*cryptoCtx;
    if (hashCtx->append != NULL) {
        mbedtls_sha512_free((mbedtls_sha512_context *)hashCtx->append);
    }
}

int32_t HksMbedtlsHashInit(void **cryptoCtx, uint32_t digestAlg)
{
    int32_t ret = 0;
    switch (digestAlg) {
        case HKS_DIGEST_MD5:
            ret = HksMbedtlsHashMd5Init(cryptoCtx, digestAlg);
            break;
        case HKS_DIGEST_SHA1:
            ret = HksMbedtlsHashSha1Init(cryptoCtx, digestAlg);
            break;
        case HKS_DIGEST_SHA224:
            ret = HksMbedtlsHashSha256Init(cryptoCtx, 1, digestAlg); /* 0 for SHA-224 */
            break;
        case HKS_DIGEST_SHA256:
            ret = HksMbedtlsHashSha256Init(cryptoCtx, 0, digestAlg); /* 0 for SHA-256 */
            break;
        case HKS_DIGEST_SHA384:
            ret = HksMbedtlsHashSha512Init(cryptoCtx, 1, digestAlg); /* 1 for SHA-384 */
            break;
        case HKS_DIGEST_SHA512:
            ret = HksMbedtlsHashSha512Init(cryptoCtx, 0, digestAlg); /* 0 for SHA-512 */
            break;
        default:
            return HKS_ERROR_INVALID_DIGEST;
    }

    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "Mbedtls hash init failed! mbedtls ret = 0x%" LOG_PUBLIC "X", ret)

    return HKS_SUCCESS;
}

int32_t HksMbedtlsHashUpdate(void *cryptoCtx, const struct HksBlob *msg)
{
    int32_t ret = 0;
    struct HksMbedtlsHashCtx *hashCtx = (struct HksMbedtlsHashCtx *)cryptoCtx;

    switch (hashCtx->mAlg) {
        case HKS_DIGEST_MD5:
            ret = HksMbedtlsHashMd5Update(hashCtx, msg->data, msg->size);
            break;
        case HKS_DIGEST_SHA1:
            ret = HksMbedtlsHashSha1Update(hashCtx, msg->data, msg->size);
            break;
        case HKS_DIGEST_SHA224:
        case HKS_DIGEST_SHA256:
            ret = HksMbedtlsHashSha256Update(hashCtx, msg->data, msg->size);
            break;
        case HKS_DIGEST_SHA384:
        case HKS_DIGEST_SHA512:
            ret = HksMbedtlsHashSha512Update(hashCtx, msg->data, msg->size);
            break;
        default:
            return HKS_ERROR_INVALID_DIGEST;
    }

    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "Mbedtls hash update failed! mbedtls ret = 0x%" LOG_PUBLIC "X", ret)

    return HKS_SUCCESS;
}

int32_t HksMbedtlsHashFinal(void **cryptoCtx, const struct HksBlob *msg, struct HksBlob *hash)
{
    int32_t ret = 0;
    struct HksMbedtlsHashCtx *hashCtx = (struct HksMbedtlsHashCtx *)*cryptoCtx;
    HKS_IF_NULL_RETURN(hashCtx, HKS_ERROR_INVALID_ARGUMENT)

    switch (hashCtx->mAlg) {
        case HKS_DIGEST_MD5:
            ret = HksMbedtlsHashMd5Final(hashCtx, msg, hash);
            break;
        case HKS_DIGEST_SHA1:
            ret = HksMbedtlsHashSha1Final(hashCtx, msg, hash);
            break;
        case HKS_DIGEST_SHA224:
        case HKS_DIGEST_SHA256:
            ret = HksMbedtlsHashSha256Final(hashCtx, msg, hash);
            break;
        case HKS_DIGEST_SHA384:
        case HKS_DIGEST_SHA512:
            ret = HksMbedtlsHashSha512Final(hashCtx, msg, hash);
            break;
        default:
            HksMbedtlsHashFreeCtx(cryptoCtx);
            return HKS_ERROR_INVALID_DIGEST;
    }

    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("Mbedtls hash final failed! mbedtls ret = 0x%" LOG_PUBLIC "X", ret);
        HksFree(*cryptoCtx);
        *cryptoCtx = NULL;
        return ret;
    }

    ret = HksGetDigestLen(hashCtx->mAlg, &(hash->size));
    HKS_IF_NOT_SUCC_LOGE(ret, "Get digest len failed!")

    HksMbedtlsHashFreeCtx(cryptoCtx);
    return ret;
}

void HksMbedtlsHashFreeCtx(void **cryptoCtx)
{
    if (cryptoCtx == NULL || *cryptoCtx == NULL) {
        HKS_LOG_E("Mbedtls Hash freeCtx param error");
        return;
    }
    struct HksMbedtlsHashCtx *hashCtx = (struct HksMbedtlsHashCtx *)*cryptoCtx;
    switch (hashCtx->mAlg) {
        case HKS_DIGEST_MD5:
            HksMbedtlsMd5HashFreeCtx(cryptoCtx);
            break;
        case HKS_DIGEST_SHA1:
            HksMbedtlsSHA1HashFreeCtx(cryptoCtx);
            break;
        case HKS_DIGEST_SHA224:
        case HKS_DIGEST_SHA256:
            HksMbedtlsSha224Sha256HashFreeCtx(cryptoCtx);
            break;
        case HKS_DIGEST_SHA384:
        case HKS_DIGEST_SHA512:
            HksMbedtlsSha384Sha512HashFreeCtx(cryptoCtx);
            break;
        default:
            break;
    }

    if (*cryptoCtx != NULL) {
        if (hashCtx->append != NULL) {
            HksFree(hashCtx->append);
            hashCtx->append = NULL;
        }

        HksFree(*cryptoCtx);
        *cryptoCtx = NULL;
    }
}

int32_t HksMbedtlsHash(uint32_t alg, const struct HksBlob *msg, struct HksBlob *hash)
{
    int32_t ret = 0;
    switch (alg) {
#ifdef HKS_SUPPORT_HASH_MD5
        case HKS_DIGEST_MD5:
            ret = mbedtls_md5_ret(msg->data, msg->size, hash->data);
            break;
#endif
#ifdef HKS_SUPPORT_HASH_SHA1
        case HKS_DIGEST_SHA1:
            ret = mbedtls_sha1_ret(msg->data, msg->size, hash->data);
            break;
#endif
#ifdef HKS_SUPPORT_HASH_SHA224
        case HKS_DIGEST_SHA224:
            ret = mbedtls_sha256_ret(msg->data, msg->size, hash->data, 1); /* 0 for SHA-224 */
            break;
#endif
#ifdef HKS_SUPPORT_HASH_SHA256
        case HKS_DIGEST_SHA256:
            ret = mbedtls_sha256_ret(msg->data, msg->size, hash->data, 0); /* 0 for SHA-256 */
            break;
#endif
#ifdef HKS_SUPPORT_HASH_SHA384
        case HKS_DIGEST_SHA384:
            ret = mbedtls_sha512_ret(msg->data, msg->size, hash->data, 1); /* 1 for SHA-384 */
            break;
#endif
#ifdef HKS_SUPPORT_HASH_SHA512
        case HKS_DIGEST_SHA512:
            ret = mbedtls_sha512_ret(msg->data, msg->size, hash->data, 0); /* 0 for SHA-512 */
            break;
#endif
        default:
            return HKS_ERROR_INVALID_DIGEST;
    }

    if (ret != HKS_MBEDTLS_SUCCESS) {
        HKS_LOG_E("Mbedtls hash failed! mbedtls ret = 0x%" LOG_PUBLIC "X", ret);
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    ret = HksGetDigestLen(alg, &(hash->size));
    HKS_IF_NOT_SUCC_LOGE(ret, "Get digest len failed!")

    return ret;
}
#endif /* HKS_SUPPORT_HASH_C */
