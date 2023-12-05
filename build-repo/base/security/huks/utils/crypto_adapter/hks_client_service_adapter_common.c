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

#include "hks_client_service_adapter_common.h"

#include <stddef.h>

#include "hks_client_service_adapter.h"
#include "hks_log.h"
#include "hks_mem.h"
#include "hks_param.h"
#include "hks_template.h"
#include "securec.h"

int32_t CopyToInnerKey(const struct HksBlob *key, struct HksBlob *outKey)
{
    if ((key->size == 0) || (key->size > MAX_KEY_SIZE)) {
        HKS_LOG_E("invalid input key size: %" LOG_PUBLIC "u", key->size);
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    uint8_t *outData = (uint8_t *)HksMalloc(key->size);
    HKS_IF_NULL_LOGE_RETURN(outData, HKS_ERROR_MALLOC_FAIL, "malloc failed")

    (void)memcpy_s(outData, key->size, key->data, key->size);
    outKey->data = outData;
    outKey->size = key->size;

    return HKS_SUCCESS;
}

#if defined(HKS_SUPPORT_ED25519_C) || defined(HKS_SUPPORT_X25519_C)
static int32_t TranslateToInnerCurve25519Format(const uint32_t alg, const struct HksBlob *key,
    struct HksBlob *publicKey)
{
    if (key->size != HKS_KEY_BYTES(HKS_CURVE25519_KEY_SIZE_256)) {
        HKS_LOG_E("Invalid curve25519 public key size! key size = 0x%" LOG_PUBLIC "X", key->size);
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    uint32_t totalSize = sizeof(struct HksPubKeyInfo) + key->size;
    uint8_t *buffer = (uint8_t *)HksMalloc(totalSize);
    HKS_IF_NULL_LOGE_RETURN(buffer, HKS_ERROR_MALLOC_FAIL, "malloc failed! %" LOG_PUBLIC "u", totalSize)
    (void)memset_s(buffer, totalSize, 0, totalSize);

    struct HksPubKeyInfo *curve25519Key = (struct HksPubKeyInfo *)buffer;
    curve25519Key->keyAlg = (enum HksKeyAlg)alg;
    curve25519Key->keySize = HKS_CURVE25519_KEY_SIZE_256;
    curve25519Key->nOrXSize = key->size; /* curve25519 public key */

    uint32_t offset = sizeof(struct HksPubKeyInfo);
    (void)memcpy_s(buffer + offset, totalSize - offset, key->data, key->size);
    publicKey->data = buffer;
    publicKey->size = totalSize;
    return HKS_SUCCESS;
}
#endif

int32_t GetHksPubKeyInnerFormat(const struct HksParamSet *paramSet,
    const struct HksBlob *key, struct HksBlob *outKey)
{
    if ((CheckBlob(key) != HKS_SUCCESS) || (outKey == NULL)) {
        HKS_LOG_E("invalid key or outKey");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    struct HksParam *algParam = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_ALGORITHM, &algParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_CHECK_GET_ALG_FAIL, "get alg param failed")

    switch (algParam->uint32Param) {
#if defined(HKS_SUPPORT_HMAC_C) || defined(HKS_SUPPORT_SM3_C) || defined(HKS_SUPPORT_SM4_C) || \
    defined(HKS_SUPPORT_AES_C)
        case HKS_ALG_AES:
            if ((key->size != HKS_KEY_BYTES(HKS_AES_KEY_SIZE_128)) &&
                (key->size != HKS_KEY_BYTES(HKS_AES_KEY_SIZE_192)) &&
                (key->size != HKS_KEY_BYTES(HKS_AES_KEY_SIZE_256))) {
                HKS_LOG_E("invalid input key size: %" LOG_PUBLIC "u", key->size);
                return HKS_ERROR_INVALID_KEY_INFO;
            } /* fall-through */
        case HKS_ALG_HMAC:
        case HKS_ALG_SM3:
        case HKS_ALG_SM4:
            return CopyToInnerKey(key, outKey);
#endif
#if defined(HKS_SUPPORT_X25519_C) || defined(HKS_SUPPORT_ED25519_C)
        case HKS_ALG_ED25519:
        case HKS_ALG_X25519:
            return TranslateToInnerCurve25519Format(algParam->uint32Param, key, outKey);
#endif
#if defined(HKS_SUPPORT_RSA_C) || defined(HKS_SUPPORT_ECC_C) || defined(HKS_SUPPORT_DSA_C) || \
    defined(HKS_SUPPORT_DH_C) || defined(HKS_SUPPORT_SM2_C)
        case HKS_ALG_RSA:
        case HKS_ALG_ECC:
        case HKS_ALG_ECDH:
        case HKS_ALG_DSA:
        case HKS_ALG_DH:
        case HKS_ALG_SM2:
            return TranslateFromX509PublicKey(algParam->uint32Param, key, outKey);
#endif
        default:
            return HKS_ERROR_INVALID_ALGORITHM;
    }
}

