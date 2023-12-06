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

#include "hks_keyblob.h"

#include <stdbool.h>
#include <stddef.h>

#include "hks_crypto_adapter.h"
#include "hks_crypto_hal.h"
#include "hks_log.h"
#include "hks_mem.h"
#include "hks_param.h"
#include "hks_template.h"
#include "securec.h"

#ifndef _CUT_AUTHENTICATE_

#define HKS_KEY_BLOB_DUMMY_KEY_VERSION 1
#define HKS_KEY_BLOB_DUMMY_OS_VERSION 1
#define HKS_KEY_BLOB_DUMMY_OS_PATCHLEVEL 1

/* temporarily use default hard-coded AT key by disable HKS_SUPPORT_GET_AT_KEY.
 * while in real scenario,it will generate random only in memory(in TEE)
 * at every start after enable HKS_SUPPORT_GET_AT_KEY
 */
#define HKS_DEFAULT_USER_AT_KEY "huks_default_user_auth_token_key"
#define HKS_DEFAULT_USER_AT_KEY_LEN 32

struct HksKeyBlobInfo {
    uint8_t salt[HKS_KEY_BLOB_DERIVE_SALT_SIZE];
    uint8_t nonce[HKS_KEY_BLOB_NONCE_SIZE];
    uint8_t tag[HKS_KEY_BLOB_TAG_SIZE];
    uint32_t keySize;
};

static void CleanKey(const struct HksParamSet *paramSet)
{
    struct HksParam *keyParam = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_KEY, &keyParam);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("get key param failed!");
        return;
    }
    (void)memset_s(keyParam->blob.data, keyParam->blob.size, 0, keyParam->blob.size);
}

void HksFreeKeyNode(struct HksKeyNode **keyNode)
{
    if ((keyNode == NULL) || (*keyNode == NULL) || ((*keyNode)->refCnt == 0)) {
        return;
    }

    (*keyNode)->refCnt--;
    if (((*keyNode)->status == HKS_KEYNODE_INACTIVE) && ((*keyNode)->refCnt == 0)) {
        CleanKey((*keyNode)->paramSet);
        HksFreeParamSet(&(*keyNode)->paramSet);
        HKS_FREE_PTR(*keyNode);
        *keyNode = NULL;
    }
}

#ifndef _STORAGE_LITE_

static int32_t GetEncryptKey(struct HksBlob *mainKey)
{
    return HksCryptoHalGetMainKey(NULL, mainKey);
}

static int32_t GetSalt(const struct HksParamSet *paramSet, const struct HksKeyBlobInfo *keyBlobInfo,
    struct HksBlob *salt)
{
    struct HksParam *appIdParam = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_PROCESS_NAME, &appIdParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get app id param failed!")

    if (appIdParam->blob.size > HKS_MAX_PROCESS_NAME_LEN) {
        HKS_LOG_E("invalid app id size: %" LOG_PUBLIC "u", appIdParam->blob.size);
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    salt->size = appIdParam->blob.size + HKS_KEY_BLOB_DERIVE_SALT_SIZE;
    salt->data = (uint8_t *)HksMalloc(salt->size);
    HKS_IF_NULL_LOGE_RETURN(salt->data, HKS_ERROR_MALLOC_FAIL, "malloc failed")

    (void)memcpy_s(salt->data, salt->size, appIdParam->blob.data, appIdParam->blob.size);

    (void)memcpy_s(salt->data + appIdParam->blob.size, salt->size - appIdParam->blob.size,
        keyBlobInfo->salt, HKS_KEY_BLOB_DERIVE_SALT_SIZE);
    return ret;
}

static int32_t GetDeriveKey(const struct HksParamSet *paramSet, const struct HksKeyBlobInfo *keyBlobInfo,
    struct HksBlob *derivedKey)
{
    struct HksBlob salt = { 0, NULL };
    int32_t ret = GetSalt(paramSet, keyBlobInfo, &salt);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    struct HksKeyDerivationParam derParam = {
        .salt = salt,
        .iterations = HKS_KEY_BLOB_DERIVE_CNT,
        .digestAlg = HKS_DIGEST_SHA256,
    };

    struct HksKeySpec derivationSpec = { HKS_ALG_PBKDF2, HKS_KEY_BYTES(HKS_AES_KEY_SIZE_256), &derParam };

    uint8_t encryptKeyData[HKS_KEY_BLOB_MAIN_KEY_SIZE] = {0};
    struct HksBlob encryptKey = { HKS_KEY_BLOB_MAIN_KEY_SIZE, encryptKeyData };
    ret = GetEncryptKey(&encryptKey);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("Hks get encrypt key failed! ret = 0x%" LOG_PUBLIC "X", ret);
        HKS_FREE_BLOB(salt);
        return ret;
    }

    derivedKey->size = HKS_KEY_BYTES(HKS_AES_KEY_SIZE_256);
    derivedKey->data = (uint8_t *)HksMalloc(derivedKey->size);
    if (derivedKey->data == NULL) {
        HKS_LOG_E("malloc failed");
        HKS_FREE_BLOB(salt);
        (void)memset_s(encryptKeyData, HKS_KEY_BLOB_MAIN_KEY_SIZE, 0, HKS_KEY_BLOB_MAIN_KEY_SIZE);
        return HKS_ERROR_MALLOC_FAIL;
    }

    ret = HksCryptoHalDeriveKey(&encryptKey, &derivationSpec, derivedKey);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("get keyblob derived key failed!");
        HKS_FREE_PTR(derivedKey->data);
    }

    (void)memset_s(encryptKeyData, HKS_KEY_BLOB_MAIN_KEY_SIZE, 0, HKS_KEY_BLOB_MAIN_KEY_SIZE);
    HKS_FREE_BLOB(salt);

    return ret;
}

static int32_t BuildKeyBlobUsageSpec(const struct HksBlob *aad, const struct HksParam *keyParam,
    bool isEncrypt, struct HksUsageSpec *usageSpec)
{
    usageSpec->mode = HKS_MODE_GCM;
    usageSpec->padding = HKS_PADDING_NONE;
    usageSpec->digest = HKS_DIGEST_NONE;
    usageSpec->algType = HKS_ALG_AES;

    struct HksAeadParam *aeadParam = (struct HksAeadParam *)HksMalloc(sizeof(struct HksAeadParam));
    HKS_IF_NULL_LOGE_RETURN(aeadParam, HKS_ERROR_MALLOC_FAIL, "aeadParam malloc failed!")

    struct HksKeyBlobInfo *keyBlobInfo = (struct HksKeyBlobInfo *)keyParam->blob.data;
    uint32_t keySize;
    (void)memcpy_s(&keySize, sizeof(keySize), &(keyBlobInfo->keySize), sizeof(keyBlobInfo->keySize));
    aeadParam->aad = *aad;
    aeadParam->payloadLen = keySize;
    aeadParam->nonce.data = keyBlobInfo->nonce;
    aeadParam->nonce.size = HKS_KEY_BLOB_NONCE_SIZE;
    if (isEncrypt) {
        aeadParam->tagLenEnc = HKS_AE_TAG_LEN;
    } else {
        aeadParam->tagDec.data = keyBlobInfo->tag;
        aeadParam->tagDec.size = HKS_KEY_BLOB_TAG_SIZE;
    }
    usageSpec->algParam = aeadParam;
    return HKS_SUCCESS;
}

static int32_t EncryptAndDecryptKeyBlob(const struct HksBlob *aad, struct HksParamSet *paramSet, bool isEncrypt)
{
    struct HksParam *keyParam = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_KEY, &keyParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "cipher keyBlob get key param failed!")

    if (keyParam->blob.size <= sizeof(struct HksKeyBlobInfo)) {
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    struct HksUsageSpec *usageSpec = (struct HksUsageSpec *)HksMalloc(sizeof(struct HksUsageSpec));
    HKS_IF_NULL_RETURN(usageSpec, HKS_ERROR_MALLOC_FAIL)

    (void)memset_s(usageSpec, sizeof(struct HksUsageSpec), 0, sizeof(struct HksUsageSpec));
    ret = BuildKeyBlobUsageSpec(aad, keyParam, isEncrypt, usageSpec);
    if (ret != HKS_SUCCESS) {
        HksFreeUsageSpec(&usageSpec);
        return ret;
    }

    struct HksKeyBlobInfo *keyBlobInfo = (struct HksKeyBlobInfo *)keyParam->blob.data;
    uint32_t keySize;
    (void)memcpy_s(&keySize, sizeof(keySize), &(keyBlobInfo->keySize), sizeof(keySize));
    if ((keyParam->blob.size - sizeof(*keyBlobInfo)) != keySize) {
        HKS_LOG_E("invalid key size in keyBlob, keySize: %" LOG_PUBLIC "u, blobSize: %" LOG_PUBLIC "u",
            keySize, keyParam->blob.size);
        HksFreeUsageSpec(&usageSpec);
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    /* encrypt/decrypt will override the srcData, so encKey and decKey point to the same buffer */
    struct HksBlob srcKey = { keySize, keyParam->blob.data + sizeof(*keyBlobInfo) };
    struct HksBlob encKey = srcKey;

    struct HksBlob derivedKey = { 0, NULL };
    ret = GetDeriveKey(paramSet, keyBlobInfo, &derivedKey);
    if (ret != HKS_SUCCESS) {
        HksFreeUsageSpec(&usageSpec);
        return ret;
    }

    if (isEncrypt) {
        struct HksBlob tag = { HKS_KEY_BLOB_TAG_SIZE, keyBlobInfo->tag };
        ret = HksCryptoHalEncrypt(&derivedKey, usageSpec, &srcKey, &encKey, &tag);
    } else {
        ret = HksCryptoHalDecrypt(&derivedKey, usageSpec, &encKey, &srcKey);
    }

    HKS_IF_NOT_SUCC_LOGE(ret, "cipher key[0x%" LOG_PUBLIC "x] failed!", isEncrypt)

    (void)memset_s(derivedKey.data, derivedKey.size, 0, derivedKey.size);
    HKS_FREE_BLOB(derivedKey);
    HksFreeUsageSpec(&usageSpec);
    return ret;
}

/*
 * [input]
 * paramSet: |-inParamSet-|-version-|-osVersion-|-patchLevel-|-struct HksKeyBlobInfo-|-srcKey-|,
 * which use |-inParamSet-|-version-|-osVersion-|-patchLevel-| as aad
 *
 * [output]
 * paramSet: |-inParamSet-|-version-|-osVersion-|-patchLevel-|-struct HksKeyBlobInfo-|-encKey-|
 */
static int32_t EncryptKeyBlob(const struct HksBlob *aad, struct HksParamSet *paramSet)
{
    return EncryptAndDecryptKeyBlob(aad, paramSet, true);
}

/*
 * [input]
 * paramSet: |-inParamSet-|-version-|-osVersion-|-patchLevel-|-struct HksKeyBlobInfo-|-encKey-|,
 * which use |-inParamSet-|-version-|-osVersion-|-patchLevel-| as aad
 *
 * [output]
 * paramSet: |-inParamSet-|-version-|-osVersion-|-patchLevel-|-struct HksKeyBlobInfo-|-srcKey-|
 */
static int32_t DecryptKeyBlob(const struct HksBlob *aad, struct HksParamSet *paramSet)
{
    return EncryptAndDecryptKeyBlob(aad, paramSet, false);
}

static int32_t InitKeyBlobInfo(const struct HksBlob *key, struct HksBlob *keyInfo)
{
    keyInfo->size = key->size + sizeof(struct HksKeyBlobInfo);
    keyInfo->data = (uint8_t *)HksMalloc(keyInfo->size);
    HKS_IF_NULL_LOGE_RETURN(keyInfo->data, HKS_ERROR_MALLOC_FAIL, "malloc failed")

    int32_t ret;
    do {
        struct HksKeyBlobInfo *keyBlobInfo = (struct HksKeyBlobInfo *)keyInfo->data;
        keyBlobInfo->keySize = key->size;

        struct HksBlob salt = { HKS_KEY_BLOB_DERIVE_SALT_SIZE, keyBlobInfo->salt };
        ret = HksCryptoHalFillRandom(&salt);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "get salt randomly failed, ret = %" LOG_PUBLIC "d", ret)

        struct HksBlob nonce = { HKS_KEY_BLOB_NONCE_SIZE, keyBlobInfo->nonce };
        ret = HksCryptoHalFillRandom(&nonce);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "get nonce randomly failed, ret = %" LOG_PUBLIC "d", ret)

        (void)memcpy_s(keyInfo->data + sizeof(*keyBlobInfo), keyInfo->size - sizeof(*keyBlobInfo),
            key->data, key->size);
    } while (0);

    if (ret != HKS_SUCCESS) {
        HKS_FREE_PTR(keyInfo->data);
    }
    return ret;
}

static int32_t AddCoreServiceParams(const struct HksBlob *keyInfo, enum HksKeyFlag keyFlag,
    struct HksParamSet *paramSet)
{
    struct HksParam tmpParam[] = {
        {
            .tag = HKS_TAG_KEY_VERSION,
            .uint32Param = HKS_KEY_BLOB_DUMMY_KEY_VERSION
        }, {
            .tag = HKS_TAG_OS_VERSION,
            .uint32Param = HKS_KEY_BLOB_DUMMY_OS_VERSION
        }, {
            .tag = HKS_TAG_OS_PATCHLEVEL,
            .uint32Param = HKS_KEY_BLOB_DUMMY_OS_PATCHLEVEL
        }, {
            .tag = HKS_TAG_KEY_FLAG,
            .uint32Param = keyFlag
        }, {
            .tag = HKS_TAG_KEY,
            .blob = *keyInfo
        },
    };

    int32_t ret = HksCheckIsTagAlreadyExist(tmpParam, HKS_ARRAY_SIZE(tmpParam), paramSet);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "add in params fail")

    ret = HksAddParams(paramSet, tmpParam, sizeof(tmpParam) / sizeof(tmpParam[0]));
    HKS_IF_NOT_SUCC_LOGE(ret, "add sys params failed")

    return ret;
}

static int32_t BuildClearKeyBlob(const struct HksBlob *key, enum HksKeyFlag keyFlag,
    const struct HksParamSet *inParamSet, struct HksParamSet **outParamSet)
{
    struct HksParamSet *newParamSet = NULL;
    struct HksBlob tmpKey = { 0, NULL };

    int32_t ret = HksInitParamSet(&newParamSet);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "init param set failed")

    do {
        ret = HksAddParams(newParamSet, inParamSet->params, inParamSet->paramsCnt);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "add in params failed")

        ret = InitKeyBlobInfo(key, &tmpKey);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "InitKeyBlobInfo failed")

        ret = AddCoreServiceParams(&tmpKey, keyFlag, newParamSet);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "add Params failed")

        /* need not clean key here */
        ret = HksBuildParamSet(&newParamSet);
        HKS_IF_NOT_SUCC_LOGE(ret, "build paramset failed!")
    } while (0);

    if (tmpKey.data != NULL) {
        (void)memset_s(tmpKey.data, tmpKey.size, 0, tmpKey.size);
        HKS_FREE_PTR(tmpKey.data);
    }
    if (ret != HKS_SUCCESS) {
        HksFreeParamSet(&newParamSet);
        return ret;
    }

    *outParamSet = newParamSet;
    return HKS_SUCCESS;
}

static int32_t GetAadAndParamSet(const struct HksBlob *inData, struct HksBlob *aad, struct HksParamSet **paramSet)
{
    uint8_t *keyBlob = (uint8_t *)HksMalloc(inData->size);
    HKS_IF_NULL_LOGE_RETURN(keyBlob, HKS_ERROR_MALLOC_FAIL, "malloc keyBlob failed")

    (void)memcpy_s(keyBlob, inData->size, inData->data, inData->size);

    struct HksParamSet *keyBlobParamSet = NULL;
    int32_t ret = HksGetParamSet((const struct HksParamSet *)keyBlob, inData->size, &keyBlobParamSet);
    if (ret != HKS_SUCCESS) {
        HKS_FREE_PTR(keyBlob);
        HKS_LOG_E("get keyBlobParamSet failed");
        return ret;
    }

    struct HksParam *keyParam = NULL;
    ret = HksGetParam(keyBlobParamSet, HKS_TAG_KEY, &keyParam);
    if (ret != HKS_SUCCESS) {
        HKS_FREE_PTR(keyBlob);
        HksFreeParamSet(&keyBlobParamSet);
        HKS_LOG_E("aad get key param failed!");
        return ret;
    }

    if (keyParam->blob.data + keyParam->blob.size != (uint8_t *)keyBlobParamSet + keyBlobParamSet->paramSetSize) {
        HKS_FREE_PTR(keyBlob);
        HksFreeParamSet(&keyBlobParamSet);
        HKS_LOG_E("invalid keyblob, keyParam should be the last param!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    *paramSet = keyBlobParamSet;
    /* the aad is the whole keyBlob content without the keyParam blob part */
    aad->data = keyBlob;
    aad->size = keyBlobParamSet->paramSetSize - keyParam->blob.size;
    return HKS_SUCCESS;
}

struct HksKeyNode *HksGenerateKeyNode(const struct HksBlob *key)
{
    if (key->size > MAX_KEY_SIZE) {
        HKS_LOG_E("invalid key blob size %" LOG_PUBLIC "x", key->size);
        return NULL;
    }

    struct HksBlob aad = { 0, NULL };
    struct HksParamSet *keyBlobParamSet = NULL;
    int32_t ret = GetAadAndParamSet(key, &aad, &keyBlobParamSet);
    HKS_IF_NOT_SUCC_RETURN(ret, NULL)

    ret = DecryptKeyBlob(&aad, keyBlobParamSet);
    HKS_FREE_BLOB(aad);
    if (ret != HKS_SUCCESS) {
        HksFreeParamSet(&keyBlobParamSet);
        HKS_LOG_E("decrypt keyBlob failed");
        return NULL;
    }

    struct HksKeyNode *keyNode = (struct HksKeyNode *)HksMalloc(sizeof(struct HksKeyNode));
    if (keyNode == NULL) {
        CleanKey(keyBlobParamSet);
        HksFreeParamSet(&keyBlobParamSet);
        HKS_LOG_E("malloc keynode failed");
        return NULL;
    }

    keyNode->refCnt = 1;
    keyNode->status = HKS_KEYNODE_INACTIVE;
    keyNode->handle = 0;
    keyNode->paramSet = keyBlobParamSet;
    return keyNode;
}

int32_t HksGetRawKey(const struct HksParamSet *paramSet, struct HksBlob *rawKey)
{
    struct HksParam *keyParam = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_KEY, &keyParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get key param failed!")

    if (keyParam->blob.size <= sizeof(struct HksKeyBlobInfo)) {
        HKS_LOG_E("invalid key size in keyBlob!");
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    struct HksKeyBlobInfo *keyBlobInfo = (struct HksKeyBlobInfo *)keyParam->blob.data;
    uint32_t keySize;
    (void)memcpy_s(&keySize, sizeof(keySize), &(keyBlobInfo->keySize), sizeof(keySize));
    if ((keyParam->blob.size - sizeof(*keyBlobInfo)) != keySize) {
        HKS_LOG_E("invalid key size in keyBlob, keySize: %" LOG_PUBLIC "u, blobSize: %" LOG_PUBLIC "u",
            keySize, keyParam->blob.size);
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    uint8_t *data = HksMalloc(keySize);
    HKS_IF_NULL_LOGE_RETURN(data, HKS_ERROR_MALLOC_FAIL, "fail to malloc raw key")

    (void)memcpy_s(data, keySize, keyParam->blob.data + sizeof(*keyBlobInfo), keySize);
    rawKey->size = keySize;
    rawKey->data = data;
    return HKS_SUCCESS;
}

int32_t HksVerifyAuthTokenSign(const struct HksUserAuthToken *authToken)
{
    HKS_IF_NULL_RETURN(authToken, HKS_ERROR_NULL_POINTER)

    uint8_t hmacKey[HKS_KEY_BLOB_AT_HMAC_KEY_BYTES] = {0};
    struct HksBlob macKeyBlob = { HKS_KEY_BLOB_AT_HMAC_KEY_BYTES, hmacKey };

    int32_t ret = HksGetAuthTokenKey(&macKeyBlob);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get authtoken key failed")

    uint32_t authTokenDataSize = sizeof(struct HksUserAuthToken) - SHA256_SIGN_LEN;
    struct HksBlob srcDataBlob = { authTokenDataSize, (uint8_t *)authToken };
    
    uint8_t computedMac[SHA256_SIGN_LEN] = {0};
    struct HksBlob macBlob = { SHA256_SIGN_LEN, computedMac };
    ret = HksCryptoHalHmac(&macKeyBlob, HKS_DIGEST_SHA256, &srcDataBlob, &macBlob);
    (void)memset_s(hmacKey, sizeof(hmacKey), 0, sizeof(hmacKey));
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "compute authtoken data mac failed!")

    ret = HksMemCmp(computedMac, (uint8_t *)authToken + authTokenDataSize, SHA256_SIGN_LEN);
    if (ret != 0) {
        HKS_LOG_E("compute authtoken data mac failed!");
        return HKS_ERROR_KEY_AUTH_VERIFY_FAILED;
    }
    return HKS_SUCCESS;
}

int32_t HksBuildKeyBlob(const struct HksBlob *keyAlias, uint8_t keyFlag, const struct HksBlob *key,
    const struct HksParamSet *paramSet, struct HksBlob *keyOut)
{
    (void)keyAlias;
    struct HksParamSet *keyBlobParamSet = NULL;
    int32_t ret = BuildClearKeyBlob(key, (enum HksKeyFlag)keyFlag, paramSet, &keyBlobParamSet);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    struct HksParam *keyParam = NULL;
    ret = HksGetParam(keyBlobParamSet, HKS_TAG_KEY, &keyParam);
    if (ret != HKS_SUCCESS) {
        HksFreeParamSet(&keyBlobParamSet);
        HKS_LOG_E("get key param when building keyBlob failed!");
        return ret;
    }

    /* the aad is the whole keyBlob content without the keyParam blob part */
    struct HksBlob aad = { keyBlobParamSet->paramSetSize - keyParam->blob.size, (uint8_t *)keyBlobParamSet };
    ret = EncryptKeyBlob(&aad, keyBlobParamSet);
    if (ret != HKS_SUCCESS) {
        /* should clean the clear key if fail to encrypt key */
        (void)memset_s(keyParam->blob.data, keyParam->blob.size, 0, keyParam->blob.size);
        HksFreeParamSet(&keyBlobParamSet);
        return ret;
    }

    if (memcpy_s(keyOut->data, keyOut->size, keyBlobParamSet, keyBlobParamSet->paramSetSize) != EOK) {
        HksFreeParamSet(&keyBlobParamSet);
        HKS_LOG_E("copy keyblob out failed!");
        return HKS_ERROR_MALLOC_FAIL;
    }

    keyOut->size = keyBlobParamSet->paramSetSize;
    HksFreeParamSet(&keyBlobParamSet);
    return HKS_SUCCESS;
}

int32_t HksGetAadAndParamSet(const struct HksBlob *inData, struct HksBlob *aad, struct HksParamSet **paramSet)
{
    return GetAadAndParamSet(inData, aad, paramSet);
}

int32_t HksDecryptKeyBlob(const struct HksBlob *aad, struct HksParamSet *paramSet)
{
    return DecryptKeyBlob(aad, paramSet);
}

#endif /* STORAGE_LITE */

#ifndef HKS_SUPPORT_GET_AT_KEY
static struct HksBlob g_cachedAuthTokenKey = {
    .size = (uint32_t)HKS_DEFAULT_USER_AT_KEY_LEN,
    .data = (uint8_t *)HKS_DEFAULT_USER_AT_KEY
};
int32_t HksCoreInitAuthTokenKey()
{
    HKS_LOG_I("generate At key success!");
    return HKS_SUCCESS;
}

int32_t HksGetAuthTokenKey(struct HksBlob *authTokenKey)
{
    HKS_IF_NOT_SUCC_LOGE_RETURN(CheckBlob(authTokenKey), HKS_ERROR_INVALID_ARGUMENT, "authTokenKey param is invalid!")

    HKS_IF_NOT_SUCC_LOGE_RETURN(CheckBlob(&g_cachedAuthTokenKey), HKS_ERROR_BAD_STATE,
        "cached auth token key is invalid!")

    if (authTokenKey->size < g_cachedAuthTokenKey.size) {
        HKS_LOG_E("buffer is too small");
        return HKS_ERROR_BUFFER_TOO_SMALL;
    }

    (void)memcpy_s(authTokenKey->data, authTokenKey->size, g_cachedAuthTokenKey.data, g_cachedAuthTokenKey.size);
    authTokenKey->size = g_cachedAuthTokenKey.size;

    return HKS_SUCCESS;
}
#else
static HksMutex *g_genAtKeyMutex = NULL;
static struct HksBlob g_cachedAuthTokenKey = { 0, NULL };
static volatile bool g_isInitAuthTokenKey = false;

static int32_t GenerateAuthTokenKey()
{
    struct HksKeySpec spec = { HKS_ALG_HMAC, HKS_KEY_BLOB_AT_HMAC_KEY_SIZE, NULL };
    int32_t ret = HksCryptoHalGenerateKey(&spec, &g_cachedAuthTokenKey);
    HKS_IF_NOT_SUCC_LOGE(ret, "generate hmac key failed!")

    return ret;
}

int32_t HksCoreInitAuthTokenKey()
{
    if (g_isInitAuthTokenKey == false) {
        if (GenerateAuthTokenKey() == HKS_SUCCESS) {
            HKS_LOG_I("generate At key success!");
            g_isInitAuthTokenKey = true;
            return HKS_SUCCESS;
        }
    }

    HKS_LOG_E("generate auth token key failed at core init stage");
    g_isInitAuthTokenKey = false;

    if (g_genAtKeyMutex == NULL) {
        g_genAtKeyMutex = HksMutexCreate();
    }

    HKS_IF_NULL_LOGE_RETURN(g_genAtKeyMutex, HKS_ERROR_BAD_STATE, "create mutex failed!")

    // here we return success for we could generate later at usage stage
    return HKS_SUCCESS;
}

int32_t HksGetAuthTokenKey(struct HksBlob *authTokenKey)
{
    HKS_IF_NOT_SUCC_LOGE_RETURN(CheckBlob(authTokenKey), HKS_ERROR_INVALID_ARGUMENT, "authTokenKey param is invalid!")

    if (g_isInitAuthTokenKey == false) {
        (void)HksMutexLock(g_genAtKeyMutex);

        // double check for avoid duplicate create in multi thread case
        if (g_isInitAuthTokenKey == false) {
            if (GenerateAuthTokenKey() != HKS_SUCCESS) {
                HKS_LOG_E("generate auth token key failed");
                (void)HksMutexUnLock(g_genAtKeyMutex);
                return ret;
            }
            HKS_LOG_I("generate At key success!");
            g_isInitAuthTokenKey = true;
        }
        (void)HksMutexUnLock(g_genAtKeyMutex);
    }

    if (authTokenKey->size < g_cachedAuthTokenKey.size) {
        HKS_LOG_E("buffer is too small");
        return HKS_ERROR_BUFFER_TOO_SMALL;
    }

    (void)memcpy_s(authTokenKey->data, authTokenKey->size, g_cachedAuthTokenKey.data, g_cachedAuthTokenKey.size);
    authTokenKey->size = g_cachedAuthTokenKey.size;
    return HKS_SUCCESS;
}
#endif /* HKS_SUPPORT_GET_AT_KEY */

#endif /* _CUT_AUTHENTICATE_ */
