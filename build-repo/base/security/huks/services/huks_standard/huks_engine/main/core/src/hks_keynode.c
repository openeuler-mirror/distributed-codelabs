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

#ifndef _CUT_AUTHENTICATE_

#include "hks_keynode.h"

#include <stddef.h>

#include "hks_core_service.h"
#include "hks_crypto_hal.h"
#include "hks_keyblob.h"
#include "hks_log.h"
#include "hks_mem.h"
#include "hks_mutex.h"
#include "hks_param.h"
#include "hks_template.h"
#include "securec.h"

#define S_TO_MS 1000
#ifdef _SUPPORT_HKS_TEE_
#define MAX_KEYNODE_COUNT 20
#else
#define MAX_KEYNODE_COUNT 100
#endif

static struct DoubleList g_keyNodeList = { &g_keyNodeList, &g_keyNodeList };
static uint32_t g_keyNodeCount = 0;

static int32_t BuildRuntimeParamSet(const struct HksParamSet *inParamSet, struct HksParamSet **outParamSet)
{
    struct HksParamSet *paramSet = NULL;
    int32_t ret = HksInitParamSet(&paramSet);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "init keyNode param set fail")

    struct HksParam params[] = {
        {
            .tag = HKS_TAG_CRYPTO_CTX,
            .uint64Param = 0
        },
    };

    if (inParamSet != NULL) {
        ret = HksCheckIsTagAlreadyExist(params, HKS_ARRAY_SIZE(params), inParamSet);
        if (ret != HKS_SUCCESS) {
            HksFreeParamSet(&paramSet);
            HKS_LOG_E("check params fail");
            return ret;
        }

        ret = HksAddParams(paramSet, inParamSet->params, inParamSet->paramsCnt);
        if (ret != HKS_SUCCESS) {
            HksFreeParamSet(&paramSet);
            HKS_LOG_E("add in params fail");
            return ret;
        }
    }

    ret = HksAddParams(paramSet, params, sizeof(params) / sizeof(params[0]));
    if (ret != HKS_SUCCESS) {
        HksFreeParamSet(&paramSet);
        HKS_LOG_E("add runtime params fail");
        return ret;
    }

    ret = HksBuildParamSet(&paramSet);
    if (ret != HKS_SUCCESS) {
        HksFreeParamSet(&paramSet);
        HKS_LOG_E("build paramSet fail");
        return ret;
    }

    *outParamSet = paramSet;
    return HKS_SUCCESS;
}

static int32_t GenerateKeyNodeHandle(uint64_t *handle)
{
    uint32_t handleData = 0;
    struct HksBlob opHandle = {
        .size = sizeof(uint32_t),
        .data = (uint8_t *)&handleData
    };

    int32_t ret = HksCryptoHalFillRandom(&opHandle);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "fill keyNode handle failed")

    *handle = handleData; /* Temporarily only use 32 bit handle */
    return HKS_SUCCESS;
}

static void FreeKeyBlobParamSet(struct HksParamSet **paramSet)
{
    if ((paramSet == NULL) || (*paramSet == NULL)) {
        HKS_LOG_E("invalid keyblob paramset");
        return;
    }
    struct HksParam *keyParam = NULL;
    int32_t ret = HksGetParam(*paramSet, HKS_TAG_KEY, &keyParam);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("get key param failed!");
        HksFreeParamSet(paramSet);
        return;
    }
    (void)memset_s(keyParam->blob.data, keyParam->blob.size, 0, keyParam->blob.size);
    HksFreeParamSet(paramSet);
}

static int32_t AddKeyNode(struct HuksKeyNode *keyNode)
{
    int32_t ret = HKS_SUCCESS;
    HksMutexLock(HksCoreGetHuksMutex());
    do {
        if (g_keyNodeCount >= MAX_KEYNODE_COUNT) {
            HKS_LOG_E("maximum number of keyNode reached");
            ret = HKS_ERROR_SESSION_REACHED_LIMIT;
            break;
        }

        AddNodeAfterDoubleListHead(&g_keyNodeList, &keyNode->listHead);
        ++g_keyNodeCount;
        HKS_LOG_I("add keynode count:%" LOG_PUBLIC "u", g_keyNodeCount);
    } while (0);

    HksMutexUnlock(HksCoreGetHuksMutex());
    return ret;
}

#ifdef _STORAGE_LITE_
struct HuksKeyNode *HksCreateKeyNode(const struct HksBlob *key, const struct HksParamSet *paramSet)
{
    struct HuksKeyNode *keyNode = (struct HuksKeyNode *)HksMalloc(sizeof(struct HuksKeyNode));
    HKS_IF_NULL_LOGE_RETURN(keyNode, NULL, "malloc hks keyNode failed")

    int32_t ret = GenerateKeyNodeHandle(&keyNode->handle);
    if (ret != HKS_SUCCESS) {
        HksFree(keyNode);
        HKS_LOG_E("get keynode handle failed");
        return NULL;
    }

    struct HksParamSet *runtimeParamSet = NULL;
    ret = BuildRuntimeParamSet(paramSet, &runtimeParamSet);
    if (ret != HKS_SUCCESS) {
        HksFree(keyNode);
        HKS_LOG_E("get runtime paramSet failed");
        return NULL;
    }

    struct HksBlob rawKey = { 0, NULL };
    ret = HksGetRawKeyMaterial(key, &rawKey);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("get raw key material failed, ret = %" LOG_PUBLIC "d", ret);
        HksFreeParamSet(&runtimeParamSet);
        HksFree(keyNode);
        return NULL;
    }

    struct HksParamSet *keyBlobParamSet = NULL;
    ret = HksTranslateKeyInfoBlobToParamSet(&rawKey, key, &keyBlobParamSet);
    (void)memset_s(rawKey.data, rawKey.size, 0, rawKey.size);
    HKS_FREE_BLOB(rawKey);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("translate key info to paramset failed, ret = %" LOG_PUBLIC "d", ret);
        HksFreeParamSet(&runtimeParamSet);
        HksFree(keyNode);
        return NULL;
    }

    ret = AddKeyNode(keyNode);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("add keyNode failed");
        HksFreeParamSet(&runtimeParamSet);
        HksFree(keyNode);
        return NULL;
    }

    keyNode->keyBlobParamSet = keyBlobParamSet;
    keyNode->runtimeParamSet = runtimeParamSet;
    return keyNode;
}
#else // _STORAGE_LITE_
static void FreeParamsForBuildKeyNode(struct HksBlob *aad, struct HksParamSet **runtimeParamSet,
    struct HksParamSet **keyblobParamSet, struct HuksKeyNode *keyNode)
{
    if (aad != NULL && aad->data != NULL) {
        HKS_FREE_BLOB(*aad);
    }

    if (runtimeParamSet != NULL && *runtimeParamSet != NULL) {
        HksFreeParamSet(runtimeParamSet);
    }

    if (keyblobParamSet != NULL && *keyblobParamSet != NULL) {
        FreeKeyBlobParamSet(keyblobParamSet);
    }
    
    if (keyNode != NULL) {
        HksFree(keyNode);
    }
}

struct HuksKeyNode *HksCreateKeyNode(const struct HksBlob *key, const struct HksParamSet *paramSet)
{
    struct HuksKeyNode *keyNode = (struct HuksKeyNode *)HksMalloc(sizeof(struct HuksKeyNode));
    HKS_IF_NULL_LOGE_RETURN(keyNode, NULL, "malloc hks keyNode failed")

    int32_t ret;
    struct HksBlob aad = { 0, NULL };
    struct HksParamSet *runtimeParamSet = NULL;
    struct HksParamSet *keyBlobParamSet = NULL;
    do {
        ret = GenerateKeyNodeHandle(&keyNode->handle);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "get keynode handle failed")

        ret = BuildRuntimeParamSet(paramSet, &runtimeParamSet);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "get runtime paramSet failed")

        ret = HksGetAadAndParamSet(key, &aad, &keyBlobParamSet);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "get aad and paramSet failed")

        ret = HksDecryptKeyBlob(&aad, keyBlobParamSet);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "decrypt keyBlob failed")

        ret = AddKeyNode(keyNode);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "add keyNode failed")
    } while (0);

    if (ret != HKS_SUCCESS) {
        FreeParamsForBuildKeyNode(&aad, &runtimeParamSet, &keyBlobParamSet, keyNode);
        return NULL;
    }

    keyNode->keyBlobParamSet = keyBlobParamSet;
    keyNode->runtimeParamSet = runtimeParamSet;
    keyNode->authRuntimeParamSet = NULL;

    HKS_FREE_BLOB(aad);
    return keyNode;
}
#endif // _STORAGE_LITE_

struct HuksKeyNode *HksQueryKeyNode(uint64_t handle)
{
    struct HuksKeyNode *keyNode = NULL;
    HksMutexLock(HksCoreGetHuksMutex());
    HKS_DLIST_ITER(keyNode, &g_keyNodeList) {
        if (keyNode != NULL && keyNode->handle == handle) {
            HksMutexUnlock(HksCoreGetHuksMutex());
            return keyNode;
        }
    }
    HksMutexUnlock(HksCoreGetHuksMutex());
    return NULL;
}

static void FreeCachedData(void **ctx)
{
    struct HksBlob *cachedData = (struct HksBlob *)*ctx;
    if (cachedData == NULL) {
        return;
    }
    if (cachedData->data != NULL) {
        (void)memset_s(cachedData->data, cachedData->size, 0, cachedData->size);
        HKS_FREE_PTR(cachedData->data);
    }
    HKS_FREE_PTR(*ctx);
}

static void KeyNodeFreeCtx(uint32_t purpose, uint32_t alg, bool hasCalcHash, void **ctx)
{
    switch (purpose) {
        case HKS_KEY_PURPOSE_AGREE:
        case HKS_KEY_PURPOSE_DERIVE:
            FreeCachedData(ctx);
            break;
        case HKS_KEY_PURPOSE_SIGN:
        case HKS_KEY_PURPOSE_VERIFY:
            if (hasCalcHash) {
                HksCryptoHalHashFreeCtx(ctx);
            } else {
                FreeCachedData(ctx);
            }
            break;
        case HKS_KEY_PURPOSE_ENCRYPT:
        case HKS_KEY_PURPOSE_DECRYPT:
            if (alg != HKS_ALG_RSA) {
                HksCryptoHalEncryptFreeCtx(ctx, alg);
            } else {
                FreeCachedData(ctx);
            }
            break;
        case HKS_KEY_PURPOSE_MAC:
            HksCryptoHalHmacFreeCtx(ctx);
            break;
        default:
            return;
    }
}

static void FreeRuntimeParamSet(struct HksParamSet **paramSet)
{
    if ((paramSet == NULL) || (*paramSet == NULL)) {
        HKS_LOG_E("invalid keyblob paramset");
        return;
    }

    struct HksParam *ctxParam = NULL;
    int32_t ret = HksGetParam(*paramSet, HKS_TAG_CRYPTO_CTX, &ctxParam);
    if (ret != HKS_SUCCESS) {
        HksFreeParamSet(paramSet);
        HKS_LOG_E("get ctx from keyNode failed!");
        return;
    }

    if (ctxParam->uint64Param != 0) {
        void *ctx = (void *)(uintptr_t)ctxParam->uint64Param;
        struct HksParam *param1 = NULL;
        ret = HksGetParam(*paramSet, HKS_TAG_PURPOSE, &param1);
        if (ret != HKS_SUCCESS) {
            HksFreeParamSet(paramSet);
            return;
        }
        struct HksParam *param2 = NULL;
        ret = HksGetParam(*paramSet, HKS_TAG_ALGORITHM, &param2);
        if (ret != HKS_SUCCESS) {
            HksFreeParamSet(paramSet);
            return;
        }
        struct HksParam *param3 = NULL;
        ret = HksGetParam(*paramSet, HKS_TAG_DIGEST, &param3);
        if (ret == HKS_ERROR_INVALID_ARGUMENT) {
            HksFreeParamSet(paramSet);
            return;
        }
        bool hasCalcHash = true;
        /* If the algorithm is ed25519, the plaintext is directly cached, and if the digest is HKS_DIGEST_NONE, the
           hash value has been passed in by the user. So the hash value does not need to be free.
        */
        if (ret == HKS_SUCCESS) {
            hasCalcHash = param3->uint32Param != HKS_DIGEST_NONE;
        }
        hasCalcHash &= (param2->uint32Param != HKS_ALG_ED25519);
        KeyNodeFreeCtx(param1->uint32Param, param2->uint32Param, hasCalcHash, &ctx);
        ctxParam->uint64Param = 0; /* clear ctx to NULL */
    }
    HksFreeParamSet(paramSet);
}

void HksDeleteKeyNode(uint64_t handle)
{
    struct HuksKeyNode *keyNode = NULL;
    HksMutexLock(HksCoreGetHuksMutex());
    HKS_DLIST_ITER(keyNode, &g_keyNodeList) {
        if (keyNode != NULL && keyNode->handle == handle) {
            RemoveDoubleListNode(&keyNode->listHead);
            FreeKeyBlobParamSet(&keyNode->keyBlobParamSet);
            FreeRuntimeParamSet(&keyNode->runtimeParamSet);
            FreeRuntimeParamSet(&keyNode->authRuntimeParamSet);
            HKS_FREE_PTR(keyNode);
            --g_keyNodeCount;
            HKS_LOG_I("delete keynode count:%" LOG_PUBLIC "u", g_keyNodeCount);
            HksMutexUnlock(HksCoreGetHuksMutex());
            return;
        }
    }
    HksMutexUnlock(HksCoreGetHuksMutex());
}
#endif /* _CUT_AUTHENTICATE_ */
