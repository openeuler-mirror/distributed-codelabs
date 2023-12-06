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


#ifndef _CUT_AUTHENTICATE_
#ifdef _STORAGE_LITE_

#include "hks_keyblob.h"
#include "hks_crypto_hal.h"
#include "hks_log.h"
#include "hks_mem.h"
#include "hks_param.h"
#include "hks_storage_adapter.h"
#include "hks_template.h"

static const char g_deriveKekTag[] = "derive_key";
static const char g_deriveNonceTag[] = "derive_nonce";

enum DeriveType {
    DERIVE_KEK = 0,
    DERIVE_NONCE = 1,
};

static int32_t HksBlobInit(struct HksBlob *blob, uint32_t size)
{
    blob->data = (uint8_t *)HksMalloc(size);
    HKS_IF_NULL_LOGE_RETURN(blob->data, HKS_ERROR_MALLOC_FAIL, "malloc failed")

    blob->size = size;
    return HKS_SUCCESS;
}

static int32_t GetSalt(enum DeriveType type, const struct HksBlob *random, struct HksBlob *salt)
{
    struct HksBlob tag = { 0, NULL };
    if (type == DERIVE_KEK) {
        tag.size = strlen(g_deriveKekTag);
        tag.data = (uint8_t *)g_deriveKekTag;
    } else if (type == DERIVE_NONCE) {
        tag.size = strlen(g_deriveNonceTag);
        tag.data = (uint8_t *)g_deriveNonceTag;
    }

    int32_t ret = HksBlobInit(salt, random->size + tag.size);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    if ((memcpy_s(salt->data, salt->size, random->data, random->size) != EOK) ||
        (memcpy_s(salt->data + random->size, salt->size - random->size, tag.data, tag.size) != EOK)) {
        HKS_FREE_PTR(salt->data);
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    return ret;
}

static int32_t GetDeriveMaterial(enum DeriveType type, const struct HksBlob *random, struct HksBlob *derivedMaterial)
{
    uint8_t keyBuf[HKS_KEY_BYTES(HKS_AES_KEY_SIZE_256)] = {0};
    struct HksBlob mk = { HKS_KEY_BYTES(HKS_AES_KEY_SIZE_256), keyBuf };

    int32_t ret = HksCryptoHalGetMainKey(NULL, &mk);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get kek failed, ret = %" LOG_PUBLIC "d", ret)

    struct HksBlob salt = { 0, NULL };
    ret = GetSalt(type, random, &salt);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("get salt failed, ret = %" LOG_PUBLIC "d", ret);
        (void)memset_s(mk.data, mk.size, 0, mk.size);
        return ret;
    }

    struct HksKeyDerivationParam derParam = {
        .salt = salt,
        .iterations = HKS_KEY_BLOB_DERIVE_CNT,
        .digestAlg = HKS_DIGEST_SHA256,
    };
    struct HksKeySpec derivationSpec = { HKS_ALG_PBKDF2, HKS_KEY_BYTES(HKS_AES_KEY_SIZE_256), &derParam };
    ret = HksCryptoHalDeriveKey(&mk, &derivationSpec, derivedMaterial);
    HKS_IF_NOT_SUCC_LOGE(ret, "get keyblob derive material failed, type = %" LOG_PUBLIC "u", type)

    HKS_FREE_BLOB(salt);
    (void)memset_s(mk.data, mk.size, 0, mk.size);
    return ret;
}

static int32_t BuildKeyBlobUsageSpec(const struct HksBlob *cipherKey, const struct HksBlob *random,
    bool isEncrypt, struct HksUsageSpec *usageSpec)
{
    usageSpec->mode = HKS_MODE_GCM;
    usageSpec->padding = HKS_PADDING_NONE;
    usageSpec->digest = HKS_DIGEST_NONE;
    usageSpec->algType = HKS_ALG_AES;

    struct HksStoreKeyInfo *keyInfo = (struct HksStoreKeyInfo *)cipherKey->data;

    /* get nonce, derive from random + tag("derive_nonce") */
    struct HksBlob nonce = { 0, NULL };
    int32_t ret = HksBlobInit(&nonce, HKS_KEY_BLOB_NONCE_SIZE); /* need free by caller function */
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    ret = GetDeriveMaterial(DERIVE_NONCE, random, &nonce);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("get derive material nonce failed, ret = %" LOG_PUBLIC "d", ret);
        HKS_FREE_PTR(nonce.data);
        return ret;
    }

    /* aad: from keyInfo->keySize to authId */
    struct HksBlob aad = {
        .size = sizeof(*keyInfo) - sizeof(keyInfo->keyInfoLen) + keyInfo->aliasSize + keyInfo->authIdSize,
        .data = cipherKey->data + sizeof(keyInfo->keyInfoLen)
    };

    struct HksAeadParam *aeadParam = (struct HksAeadParam *)usageSpec->algParam;
    aeadParam->nonce = nonce;
    aeadParam->aad = aad;
    aeadParam->payloadLen = keyInfo->keySize - HKS_AE_TAG_LEN;

    if (isEncrypt) {
        aeadParam->tagLenEnc = HKS_AE_TAG_LEN;
    } else {
        aeadParam->tagDec.data = cipherKey->data + keyInfo->keyInfoLen - HKS_AE_TAG_LEN; /* the last 16 bytes */
        aeadParam->tagDec.size = HKS_AE_TAG_LEN;
    }
    return HKS_SUCCESS;
}

static int32_t EncryptAndDecryptKeyBlob(struct HksBlob *rawKey, struct HksBlob *cipherKey, bool isEncrypt)
{
    struct HksStoreKeyInfo *keyInfo = (struct HksStoreKeyInfo *)cipherKey->data;
    int32_t ret;

    /* 1. generate random */
    struct HksBlob random = { HKS_DEFAULT_RANDOM_LEN, keyInfo->random };
    if (isEncrypt) {
        ret = HksCryptoHalFillRandom(&random);
        HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get random failed")
    }

    /* 2. get kek, derive from random + tag("derive_kek") */
    uint8_t kekBuf[HKS_KEY_BYTES(HKS_AES_KEY_SIZE_256)] = {0};
    struct HksBlob kek = { HKS_KEY_BYTES(HKS_AES_KEY_SIZE_256), kekBuf };
    ret = GetDeriveMaterial(DERIVE_KEK, &random, &kek);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    /* 3. get usage spec */
    struct HksAeadParam aeadParam = {0};
    struct HksUsageSpec usageSpec = { .algParam = (void *)&aeadParam };
    ret = BuildKeyBlobUsageSpec(cipherKey, &random, isEncrypt, &usageSpec);
    if (ret != HKS_SUCCESS) {
        (void)memset_s(kekBuf, HKS_KEY_BYTES(HKS_AES_KEY_SIZE_256), 0, HKS_KEY_BYTES(HKS_AES_KEY_SIZE_256));
        return ret;
    }

    /* 4. get encrypted/decrypted key */
    struct HksBlob encKey = { keyInfo->keySize, cipherKey->data + keyInfo->keyInfoLen - keyInfo->keySize };
    if (isEncrypt) {
        struct HksBlob tag = { HKS_AE_TAG_LEN, cipherKey->data + keyInfo->keyInfoLen - HKS_AE_TAG_LEN };
        ret = HksCryptoHalEncrypt(&kek, &usageSpec, rawKey, &encKey, &tag);
    } else {
        encKey.size -= HKS_AE_TAG_LEN; /* the decrypt len should remove the tag len */
        ret = HksCryptoHalDecrypt(&kek, &usageSpec, &encKey, rawKey);
    }
    HKS_IF_NOT_SUCC_LOGE(ret, "cipher key[0x%" LOG_PUBLIC "x] failed, ret = %" LOG_PUBLIC "d", isEncrypt, ret)

    /* need clean kek buf */
    (void)memset_s(kekBuf, HKS_KEY_BYTES(HKS_AES_KEY_SIZE_256), 0, HKS_KEY_BYTES(HKS_AES_KEY_SIZE_256));
    HKS_FREE_BLOB(aeadParam.nonce);
    return ret;
}

static int32_t EncryptKeyBlob(const struct HksBlob *rawKey, struct HksBlob *cipherKey)
{
    return EncryptAndDecryptKeyBlob((struct HksBlob *)rawKey, cipherKey, true);
}

static int32_t DecryptKeyBlob(const struct HksBlob *cipherKey, struct HksBlob *rawKey)
{
    return EncryptAndDecryptKeyBlob(rawKey, (struct HksBlob *)cipherKey, false);
}

static int32_t CopyKey(const struct HksBlob *key, struct HksBlob *adjustedKey)
{
    int32_t ret  = HksBlobInit(adjustedKey, key->size);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    if (memcpy_s(adjustedKey->data, adjustedKey->size, key->data, key->size) != EOK) {
        HKS_FREE_PTR(adjustedKey->data);
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    return ret;
}

static int32_t Ed25519BlobToKeyMaterial(const struct HksBlob *key, struct HksBlob *adjustedKey)
{
    if (key->size != (HKS_KEY_BYTES(HKS_CURVE25519_KEY_SIZE_256) << 1)) {
        HKS_LOG_E("invalid keySize %" LOG_PUBLIC "u", key->size);
        return HKS_ERROR_INVALID_KEY_FILE;
    }

    int32_t ret  = HksBlobInit(adjustedKey, sizeof(struct KeyMaterial25519) +
        (HKS_KEY_BYTES(HKS_CURVE25519_KEY_SIZE_256) << 1));
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    (void)memset_s(adjustedKey->data, adjustedKey->size, 0, adjustedKey->size);

    struct KeyMaterial25519 *keyMaterial = (struct KeyMaterial25519 *)adjustedKey->data;
    keyMaterial->keyAlg = HKS_ALG_ED25519;
    keyMaterial->keySize = HKS_CURVE25519_KEY_SIZE_256;
    keyMaterial->pubKeySize = HKS_KEY_BYTES(HKS_CURVE25519_KEY_SIZE_256);
    keyMaterial->priKeySize = HKS_KEY_BYTES(HKS_CURVE25519_KEY_SIZE_256);

    uint32_t offset = sizeof(*keyMaterial);
    /* copy public key: the first 32 bytes of input key value; then private key: next 32 bytes */
    if (memcpy_s(adjustedKey->data + offset, adjustedKey->size - offset,
        key->data, (HKS_KEY_BYTES(HKS_CURVE25519_KEY_SIZE_256) << 1)) != EOK) {
        HKS_LOG_E("copy ed25519 public and private value failed");
        (void)memset_s(adjustedKey->data, adjustedKey->size, 0, adjustedKey->size);
        HKS_FREE_PTR(adjustedKey->data);
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    return ret;
}

static int32_t Ed25519KeyMaterialToBlob(const struct HksBlob *key, struct HksBlob *adjustedKey)
{
    if (key->size < sizeof(struct KeyMaterial25519)) {
        HKS_LOG_E("key size invalid, size = %" LOG_PUBLIC "u smaller than struct size", key->size);
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    struct KeyMaterial25519 *keyMaterial = (struct KeyMaterial25519 *)key->data;
    if ((keyMaterial->pubKeySize != HKS_KEY_BYTES(HKS_CURVE25519_KEY_SIZE_256)) ||
        (keyMaterial->priKeySize != HKS_KEY_BYTES(HKS_CURVE25519_KEY_SIZE_256)) ||
        (key->size < (sizeof(*keyMaterial) + keyMaterial->pubKeySize + keyMaterial->priKeySize))) {
        HKS_LOG_E("key size invalid, keySize = %" LOG_PUBLIC "u, pubSize %" LOG_PUBLIC "u, priSize %" LOG_PUBLIC "u",
            key->size, keyMaterial->pubKeySize, keyMaterial->priKeySize);
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    int32_t ret = HksBlobInit(adjustedKey, keyMaterial->priKeySize + keyMaterial->pubKeySize);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    /* 32 bytes pubkey first, then 32 bytes private key */
    if (memcpy_s(adjustedKey->data, adjustedKey->size, key->data + sizeof(*keyMaterial),
        keyMaterial->pubKeySize + keyMaterial->priKeySize) != EOK) {
        HKS_LOG_E("copy pubKey and private key failed.");
        HKS_FREE_PTR(adjustedKey->data);
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    return ret;
}

static int32_t GetRawKeyMaterial(const struct HksBlob *key, struct HksBlob *rawKey)
{
    if (key->size < sizeof(struct HksStoreKeyInfo)) {
        HKS_LOG_E("invalid key, size too small, size = %" LOG_PUBLIC "u", key->size);
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    struct HksStoreKeyInfo *keyInfo = (struct HksStoreKeyInfo *)key->data;
    if (HksIsKeyInfoLenInvalid(keyInfo)) {
        HKS_LOG_E("invalid keyInfoBlob len");
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    struct HksBlob tmpKey = { 0, NULL };
    int32_t ret = HksBlobInit(&tmpKey, keyInfo->keySize);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    ret = DecryptKeyBlob(key, &tmpKey);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("decrypt key blob failed, ret = %" LOG_PUBLIC "d", ret);
        HKS_FREE_BLOB(tmpKey);
        return ret;
    }

    if ((keyInfo->keyAlg == HKS_ALG_ED25519) && (keyInfo->flag == HKS_KEY_FLAG_GENERATE_KEY)) {
        ret = Ed25519BlobToKeyMaterial(&tmpKey, rawKey);
    } else {
        ret = CopyKey(&tmpKey, rawKey);
    }
    HKS_IF_NOT_SUCC_LOGE(ret, "operate failed, alg:%" LOG_PUBLIC "u, ret = %" LOG_PUBLIC "d", keyInfo->keyAlg, ret)

    (void)memset_s(tmpKey.data, tmpKey.size, 0, tmpKey.size);
    HKS_FREE_BLOB(tmpKey);
    return HKS_SUCCESS;
}

struct HksKeyNode *HksGenerateKeyNode(const struct HksBlob *key)
{
    if (key->size > MAX_KEY_SIZE) {
        HKS_LOG_E("invalid key blob size %" LOG_PUBLIC "x", key->size);
        return NULL;
    }
    struct HksKeyNode *keyNode = (struct HksKeyNode *)HksMalloc(sizeof(struct HksKeyNode));
    HKS_IF_NULL_LOGE_RETURN(keyNode, NULL, "malloc keynode failed")

    keyNode->refCnt = 1;
    keyNode->status = HKS_KEYNODE_INACTIVE;
    keyNode->handle = 0;

    int32_t ret;
    do {
        struct HksBlob rawKey = { 0, NULL };
        ret = GetRawKeyMaterial(key, &rawKey);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "get raw key material failed, ret = %" LOG_PUBLIC "d", ret)

        struct HksParamSet *keyBlobParamSet = NULL;
        ret = TranslateKeyInfoBlobToParamSet(&rawKey, key, &keyBlobParamSet);
        (void)memset_s(rawKey.data, rawKey.size, 0, rawKey.size);
        HKS_FREE_BLOB(rawKey);

        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "translate key info to paramset failed, ret = %" LOG_PUBLIC "d", ret)

        keyNode->paramSet = keyBlobParamSet;
    } while (0);

    if (ret != HKS_SUCCESS) {
        HKS_FREE_PTR(keyNode);
        return NULL;
    }

    return keyNode;
}

static int32_t FillBaseInfo(const struct HksParamSet *paramSet, struct HksBlob *keyOut)
{
    struct HksStoreKeyInfo *keyInfo = (struct HksStoreKeyInfo *)keyOut->data;
    for (uint32_t i = 0; i < paramSet->paramsCnt; i++) {
        switch (paramSet->params[i].tag) {
            case HKS_TAG_ALGORITHM:
                keyInfo->keyAlg = paramSet->params[i].uint32Param;
                break;
            case HKS_TAG_PADDING:
                keyInfo->padding = paramSet->params[i].uint32Param;
                break;
            case HKS_TAG_DIGEST:
                keyInfo->digest = paramSet->params[i].uint32Param;
                break;
            case HKS_TAG_BLOCK_MODE:
                keyInfo->keyMode = paramSet->params[i].uint32Param;
                break;
            case HKS_TAG_PURPOSE:
                keyInfo->purpose = paramSet->params[i].uint32Param;
                break;
            case HKS_TAG_KEY_SIZE:
                keyInfo->keyLen = paramSet->params[i].uint32Param;
                break;
            case HKS_TAG_KEY_ROLE:
                keyInfo->role = paramSet->params[i].uint32Param;
                break;
            case HKS_TAG_KEY_DOMAIN:
                keyInfo->domain = paramSet->params[i].uint32Param;
                break;
            case HKS_TAG_KEY_AUTH_ID:
                if (paramSet->params[i].blob.size > HKS_MAX_KEY_AUTH_ID_LEN) {
                    HKS_LOG_E("invlaid authId size %" LOG_PUBLIC "u", paramSet->params[i].blob.size);
                    return HKS_ERROR_INVALID_ARGUMENT;
                }
                if (memcpy_s(keyOut->data + sizeof(*keyInfo) + keyInfo->aliasSize, HKS_MAX_KEY_AUTH_ID_LEN,
                    paramSet->params[i].blob.data, paramSet->params[i].blob.size) != EOK) {
                    HKS_LOG_E("memcpy key auth id failed");
                    return HKS_ERROR_INSUFFICIENT_MEMORY;
                }
                keyInfo->authIdSize = paramSet->params[i].blob.size;
                break;
            default:
                break;
        }
    }
    return HKS_SUCCESS;
}

static int32_t FillStoreKeyInfo(const struct HksBlob *keyAlias, uint8_t keyFlag, const struct HksBlob *key,
    const struct HksParamSet *paramSet, struct HksBlob *keyOut)
{
    if ((keyAlias->size > HKS_MAX_KEY_ALIAS_LEN) || (key->size > HKS_MAX_KEY_LEN)) {
        HKS_LOG_E("invalid keyAlias size %" LOG_PUBLIC "u, or key size %" LOG_PUBLIC "u", keyAlias->size, key->size);
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    struct HksStoreKeyInfo *keyInfo = (struct HksStoreKeyInfo *)keyOut->data;
    keyInfo->flag = keyFlag;
    keyInfo->keySize = key->size + HKS_AE_TAG_LEN;

    /* 1. copy keyAlias */
    if (memcpy_s(keyOut->data + sizeof(*keyInfo), HKS_MAX_KEY_ALIAS_LEN, keyAlias->data, keyAlias->size) != EOK) {
        HKS_LOG_E("memcpy keyAlias failed");
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    keyInfo->aliasSize = keyAlias->size;

    /* 2. copy keyAuthId, keyAlg, purpose ect. */
    int32_t ret = FillBaseInfo(paramSet, keyOut);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    keyInfo->keyInfoLen = sizeof(*keyInfo) + keyInfo->aliasSize + keyInfo->authIdSize + keyInfo->keySize;

    /* 3. encrypt key */
    ret = EncryptKeyBlob(key, keyOut);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "encrypt key blob failed, ret = %" LOG_PUBLIC "d", ret)

    keyOut->size = keyInfo->keyInfoLen;

    return ret;
}

static int32_t AdjustKey(uint8_t keyFlag, const struct HksParamSet *paramSet,
    const struct HksBlob *key, struct HksBlob *adjustedKey)
{
    struct HksParam *algParam = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_ALGORITHM, &algParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get alg param failed")

    /* for storage-restricted products, when generate ed25519 key, only 64-byte private key can be stored */
    if ((algParam->uint32Param == HKS_ALG_ED25519) && (keyFlag == HKS_KEY_FLAG_GENERATE_KEY)) {
        ret = Ed25519KeyMaterialToBlob(key, adjustedKey);
    } else {
        ret = CopyKey(key, adjustedKey);
    }
    HKS_IF_NOT_SUCC_LOGE(ret,
        "operate failed, alg = %" LOG_PUBLIC "u, ret = %" LOG_PUBLIC "d", algParam->uint32Param, ret)

    return ret;
}

int32_t HksGetRawKey(const struct HksParamSet *paramSet, struct HksBlob *rawKey)
{
    struct HksParam *keyParam = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_KEY, &keyParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get key param failed!")

    uint8_t *data = HksMalloc(keyParam->blob.size);
    HKS_IF_NULL_LOGE_RETURN(data, HKS_ERROR_MALLOC_FAIL, "fail to malloc raw key")

    (void)memcpy_s(data, keyParam->blob.size, keyParam->blob.data, keyParam->blob.size);

    rawKey->size = keyParam->blob.size;
    rawKey->data = data;
    return HKS_SUCCESS;
}

int32_t HksBuildKeyBlob(const struct HksBlob *keyAlias, uint8_t keyFlag, const struct HksBlob *key,
    const struct HksParamSet *paramSet, struct HksBlob *keyOut)
{
    struct HksBlob adjustedKey = { 0, NULL };
    int32_t ret = AdjustKey(keyFlag, paramSet, key, &adjustedKey);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "adjust key failed, ret = %" LOG_PUBLIC "d", ret)

    struct HksBlob tmpOut = { 0, NULL };
    do {
        uint32_t totalLen = sizeof(struct HksStoreKeyInfo) + HKS_MAX_KEY_ALIAS_LEN + HKS_MAX_KEY_AUTH_ID_LEN +
            HKS_MAX_KEY_MATERIAL_LEN;
        ret = HksBlobInit(&tmpOut, totalLen);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "hks blob init failed, ret = %" LOG_PUBLIC "d", ret)

        (void)memset_s(tmpOut.data, tmpOut.size, 0, tmpOut.size); /* need init 0 */

        ret = FillStoreKeyInfo(keyAlias, keyFlag, &adjustedKey, paramSet, &tmpOut);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "fill storage key info failed, ret = %" LOG_PUBLIC "d", ret)

        if (memcpy_s(keyOut->data, keyOut->size, tmpOut.data, tmpOut.size) != EOK) {
            HKS_LOG_E("copy keyblob out failed!");
            ret = HKS_ERROR_INSUFFICIENT_MEMORY;
            break;
        }
        keyOut->size = tmpOut.size;
    } while (0);

    (void)memset_s(adjustedKey.data, adjustedKey.size, 0, adjustedKey.size); /* need clean key */
    HKS_FREE_BLOB(adjustedKey);
    HKS_FREE_BLOB(tmpOut);
    return ret;
}

int32_t HksGetRawKeyMaterial(const struct HksBlob *key, struct HksBlob *rawKey)
{
    return GetRawKeyMaterial(key, rawKey);
}

int32_t HksTranslateKeyInfoBlobToParamSet(const struct HksBlob *key, const struct HksBlob *keyInfoBlob,
    struct HksParamSet **paramSet)
{
    return TranslateKeyInfoBlobToParamSet(key, keyInfoBlob, paramSet);
}

#endif /* _STORAGE_LITE_ */
#endif /* _CUT_AUTHENTICATE_ */
