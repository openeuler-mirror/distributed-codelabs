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

#include "hks_local_engine.h"

#include <stdbool.h>
#include <stddef.h>

#include "hks_ability.h"
#include "hks_base_check.h"
#include "hks_check_paramset.h"
#include "hks_cmd_id.h"
#include "hks_common_check.h"
#include "hks_crypto_adapter.h"
#include "hks_crypto_hal.h"
#include "hks_log.h"
#include "hks_mem.h"
#include "hks_param.h"
#include "hks_template.h"
#include "securec.h"

#define MAX_DEGIST_SIZE 64
#define X25519_KEY_BYTE_SIZE 32

static void HksLocalCryptoAbilityInit(void)
{
    int32_t ret = HksCryptoAbilityInit();
    HKS_IF_NOT_SUCC_LOGE(ret, "Hks local init crypto ability failed, ret = %" LOG_PUBLIC "d", ret)
}

#ifndef _CUT_AUTHENTICATE_
int32_t HksLocalHash(const struct HksParamSet *paramSet, const struct HksBlob *srcData, struct HksBlob *hash)
{
    HKS_IF_NOT_SUCC_RETURN(HksCheckBlob2AndParamSet(srcData, hash, paramSet), HKS_ERROR_INVALID_ARGUMENT)

    struct HksParam *digestAlg = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_DIGEST, &digestAlg);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_CHECK_GET_DIGEST_FAIL, "get param digest failed")

    uint32_t digestLen;
    ret = HksGetDigestLen(digestAlg->uint32Param, &digestLen);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get digest len failed, ret = %" LOG_PUBLIC "d", ret)

    if (hash->size < digestLen) {
        HKS_LOG_E("hash len too small, size = %" LOG_PUBLIC "u", hash->size);
        return HKS_ERROR_BUFFER_TOO_SMALL;
    }
    HksLocalCryptoAbilityInit();
    return HksCryptoHalHash(digestAlg->uint32Param, srcData, hash);
}
#endif

int32_t HksLocalMac(const struct HksBlob *key, const struct HksParamSet *paramSet,
    const struct HksBlob *srcData, struct HksBlob *mac)
{
    HKS_IF_NOT_SUCC_RETURN(HksCheckBlob3AndParamSet(key, srcData, mac, paramSet), HKS_ERROR_INVALID_ARGUMENT)
    int32_t ret = HksCoreCheckMacParams(key, paramSet, srcData, mac, true);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    struct HksParam *digestAlg = NULL;
    ret = HksGetParam(paramSet, HKS_TAG_DIGEST, &digestAlg);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_CHECK_GET_DIGEST_FAIL, "get param digest failed")
    HksLocalCryptoAbilityInit();
    return HksCryptoHalHmac(key, digestAlg->uint32Param, srcData, mac);
}

int32_t HksLocalBnExpMod(struct HksBlob *x, const struct HksBlob *a, const struct HksBlob *e, const struct HksBlob *n)
{
    int32_t ret = HksCheckBlob4(x, a, e, n);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)
    HksLocalCryptoAbilityInit();
    return HksCryptoHalBnExpMod(x, a, e, n);
}

#ifndef _CUT_AUTHENTICATE_
static int32_t CheckLocalGenerateKeyParams(const struct HksParamSet *paramSetIn, const struct HksParamSet *paramSetOut)
{
    int32_t ret;
    if ((HksCheckParamSetValidity(paramSetIn) != HKS_SUCCESS) || (paramSetOut == NULL)) {
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    ret = HksCoreCheckGenKeyParams(NULL, paramSetIn, NULL, NULL);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_INVALID_ARGUMENT,
        "check generate key parameter failed ret = %" LOG_PUBLIC "x.", ret)

    return HKS_SUCCESS;
}

int32_t HksLocalGenerateKey(const struct HksParamSet *paramSetIn, struct HksParamSet *paramSetOut)
{
    int32_t ret = CheckLocalGenerateKeyParams(paramSetIn, paramSetOut);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    struct HksKeySpec spec = {0};
    HksFillKeySpec(paramSetIn, &spec);
    struct HksBlob key = { 0, NULL };

    HksLocalCryptoAbilityInit();
    ret = HksCryptoHalGenerateKey(&spec, &key);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "local engine generate key failed, ret:%" LOG_PUBLIC "x!", ret)

    ret = HksFormatKeyFromMaterial(spec.algType, &key, paramSetOut);
    (void)memset_s(key.data, key.size, 0, key.size);
    HKS_FREE_PTR(key.data);
    return ret;
}

int32_t HksLocalAgreeKey(const struct HksParamSet *paramSet, const struct HksBlob *privateKey,
    const struct HksBlob *peerPublicKey, struct HksBlob *agreedKey)
{
    HKS_IF_NOT_SUCC_RETURN(HksCheckBlob3AndParamSet(privateKey, peerPublicKey, agreedKey, paramSet),
        HKS_ERROR_INVALID_ARGUMENT)

    int32_t ret = HksCoreCheckAgreeKeyParams(paramSet, privateKey, peerPublicKey, agreedKey, true);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    struct HksKeySpec spec = {0};
    HksFillKeySpec(paramSet, &spec);

    struct HksBlob privateKeyMaterial = { 0, NULL };
    ret = HksSetKeyToMaterial(spec.algType, false, privateKey, &privateKeyMaterial);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "set prikey to materail failed, ret:%" LOG_PUBLIC "x!", ret)

    struct HksBlob publicKeyMaterial = { 0, NULL };
    ret = HksSetKeyToMaterial(spec.algType, true, peerPublicKey, &publicKeyMaterial);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("set pubkey to materail failed, ret:%" LOG_PUBLIC "x!", ret);
        (void)memset_s(privateKeyMaterial.data, privateKeyMaterial.size, 0, privateKeyMaterial.size);
        HKS_FREE_PTR(privateKeyMaterial.data);
        return ret;
    }

    HksLocalCryptoAbilityInit();
    ret = HksCryptoHalAgreeKey(&privateKeyMaterial, &publicKeyMaterial, &spec, agreedKey);
    HKS_IF_NOT_SUCC_LOGE(ret, "local engine agree key failed, ret:%" LOG_PUBLIC "x!", ret)

    (void)memset_s(privateKeyMaterial.data, privateKeyMaterial.size, 0, privateKeyMaterial.size);
    HKS_FREE_PTR(privateKeyMaterial.data);
    (void)memset_s(publicKeyMaterial.data, publicKeyMaterial.size, 0, publicKeyMaterial.size);
    HKS_FREE_PTR(publicKeyMaterial.data);
    return ret;
}
#endif

static int32_t CipherEncrypt(const struct HksBlob *key, const struct HksParamSet *paramSet,
    const struct HksUsageSpec *usageSpec, const struct HksBlob *inData, struct HksBlob *outData)
{
    struct HksBlob tag = { 0, NULL };
    int32_t ret = HksGetEncryptAeTag(paramSet, inData, outData, &tag);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "cipher encrypt get ae tag failed!")

    ret = HksCryptoHalEncrypt(key, usageSpec, inData, outData, &tag);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "cipher encrypt failed!")

    outData->size += tag.size;
    return HKS_SUCCESS;
}

static int32_t CheckLocalCipherParams(uint32_t cmdId, const struct HksBlob *key, const struct HksParamSet *paramSet,
    const struct HksBlob *inputText, struct HksBlob *outputText)
{
    HKS_IF_NOT_SUCC_RETURN(HksCheckBlob3AndParamSet(key, inputText, outputText, paramSet), HKS_ERROR_INVALID_ARGUMENT)

    struct HksParam *outParam = NULL;
    HKS_IF_NOT_SUCC_LOGE_RETURN(HksGetParam(paramSet, HKS_TAG_ALGORITHM, &outParam),
        HKS_ERROR_CHECK_GET_ALG_FAIL, "get tag algorithm failed.")

    uint32_t keySize = 0;
    if (outParam->uint32Param == HKS_ALG_AES) {
        keySize = key->size * HKS_BITS_PER_BYTE;
    } else if (outParam->uint32Param == HKS_ALG_RSA) {
        const struct KeyMaterialRsa *keyMaterial = (struct KeyMaterialRsa *)key->data;
        keySize = keyMaterial->keySize;
    } else {
        return HKS_ERROR_INVALID_ALGORITHM;
    }

    return HksLocalCheckCipherParams(cmdId, keySize, paramSet, inputText, outputText);
}

static int32_t EncryptAndDecrypt(uint32_t cmdId, const struct HksBlob *key, const struct HksParamSet *paramSet,
    const struct HksBlob *inputText, struct HksBlob *outputText)
{
    int32_t ret = CheckLocalCipherParams(cmdId, key, paramSet, inputText, outputText);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    struct HksUsageSpec *usageSpec = NULL;
    bool isEncrypt = (cmdId == HKS_CMD_ID_ENCRYPT);
    struct HksBlob tmpInputText = { inputText->size, inputText->data };
    ret = HksBuildCipherUsageSpec(paramSet, isEncrypt, &tmpInputText, &usageSpec);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "build cipher usagespec failed, ret:%" LOG_PUBLIC "x!", ret)

    if (cmdId == HKS_CMD_ID_ENCRYPT) {
        ret = CipherEncrypt(key, paramSet, usageSpec, &tmpInputText, outputText);
    } else {
        ret = HksCryptoHalDecrypt(key, usageSpec, &tmpInputText, outputText);
    }
    HksFreeUsageSpec(&usageSpec);

    HKS_IF_NOT_SUCC_LOGE(ret,
        "local engine EncryptDecrypt cmdId = %" LOG_PUBLIC "u failed, ret:%" LOG_PUBLIC "x!", cmdId, ret)
    return ret;
}

int32_t HksLocalEncrypt(const struct HksBlob *key, const struct HksParamSet *paramSet,
    const struct HksBlob *plainText, struct HksBlob *cipherText)
{
    HksLocalCryptoAbilityInit();
    return EncryptAndDecrypt(HKS_CMD_ID_ENCRYPT, key, paramSet, plainText, cipherText);
}

int32_t HksLocalDecrypt(const struct HksBlob *key, const struct HksParamSet *paramSet,
    const struct HksBlob *cipherText, struct HksBlob *plainText)
{
    HksLocalCryptoAbilityInit();
    return EncryptAndDecrypt(HKS_CMD_ID_DECRYPT, key, paramSet, cipherText, plainText);
}

int32_t HksLocalDeriveKey(const struct HksParamSet *paramSet, const struct HksBlob *mainKey,
    struct HksBlob *derivedKey)
{
    HksLocalCryptoAbilityInit();
    HKS_IF_NOT_SUCC_RETURN(HksCheckBlob2AndParamSet(mainKey, derivedKey, paramSet), HKS_ERROR_INVALID_ARGUMENT)

    int32_t ret = HksCoreCheckDeriveKeyParams(paramSet, mainKey, derivedKey, true);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    struct HksKeyDerivationParam derivationParam = { { 0, NULL }, { 0, NULL }, 0, 0 };
    struct HksKeySpec derivationSpec = { 0, 0, &derivationParam };

    HksFillKeySpec(paramSet, &derivationSpec);
    HksFillKeyDerivationParam(paramSet, &derivationParam);

    return HksCryptoHalDeriveKey(mainKey, &derivationSpec, derivedKey);
}

#ifndef _CUT_AUTHENTICATE_
static int32_t CheckLocalSignVerifyParams(uint32_t cmdId, const struct HksBlob *key, const struct HksParamSet *paramSet,
    const struct HksBlob *srcData, const struct HksBlob *signature)
{
    HksLocalCryptoAbilityInit();
    HKS_IF_NOT_SUCC_RETURN(HksCheckBlob3AndParamSet(key, srcData, signature, paramSet), HKS_ERROR_INVALID_ARGUMENT)

    struct HksParam *algParam = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_ALGORITHM, &algParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_CHECK_GET_ALG_FAIL, "get param algorithm failed")

    uint32_t keySize = 0;
    if (algParam->uint32Param == HKS_ALG_RSA) {
        keySize = ((struct KeyMaterialRsa *)key->data)->keySize;
    } else if (algParam->uint32Param == HKS_ALG_DSA) {
        keySize = ((struct KeyMaterialDsa *)key->data)->keySize;
    } else if (algParam->uint32Param == HKS_ALG_ECC) {
        keySize = ((struct KeyMaterialEcc *)key->data)->keySize;
    } else if (algParam->uint32Param == HKS_ALG_ED25519) {
        keySize = key->size * HKS_BITS_PER_BYTE;
    }

    ret = HksLocalCheckSignVerifyParams(cmdId, keySize, paramSet, srcData, signature);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    if (algParam->uint32Param == HKS_ALG_ED25519) {
        if (key->size != (HKS_CURVE25519_KEY_SIZE_256 / HKS_BITS_PER_BYTE)) {
            HKS_LOG_E("invalid key size: %" LOG_PUBLIC "u", key->size);
            return HKS_ERROR_INVALID_KEY_SIZE;
        }

        if (signature->size < HKS_SIGNATURE_MIN_SIZE) {
            HKS_LOG_E("signature size too small, size: %" LOG_PUBLIC "u", signature->size);
            return HKS_ERROR_INVALID_SIGNATURE_SIZE;
        }
    }

    return HKS_SUCCESS;
}

static int32_t GetSignVerifyMessage(struct HksUsageSpec *usageSpec, const struct HksBlob *srcData,
    struct HksBlob *message, bool *needFree)
{
    if (HksCheckNeedCache(usageSpec->algType, usageSpec->digest) == HKS_SUCCESS) {
        message->size = srcData->size;
        message->data = srcData->data;
        *needFree = false;
    } else {
        message->size = MAX_DEGIST_SIZE;
        message->data = (uint8_t *)HksMalloc(MAX_DEGIST_SIZE);
        HKS_IF_NULL_LOGE_RETURN(message->data, HKS_ERROR_MALLOC_FAIL, "SignVerify malloc message data failed!")

        int32_t ret = HksCryptoHalHash(usageSpec->digest, srcData, message);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("SignVerify calc hash failed!");
            HKS_FREE_PTR(message->data);
            return ret;
        }

        *needFree = true;
    }
    return HKS_SUCCESS;
}

int32_t HksLocalSign(const struct HksBlob *key, const struct HksParamSet *paramSet, const struct HksBlob *srcData,
    struct HksBlob *signature)
{
    int32_t ret = CheckLocalSignVerifyParams(HKS_CMD_ID_SIGN, key, paramSet, srcData, signature);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    struct HksUsageSpec usageSpec = {0};
    HksFillUsageSpec(paramSet, &usageSpec);
    bool needFree = true;
    struct HksBlob message = { 0, NULL };
    struct HksBlob keyMaterial = { 0, NULL };
    do {
        ret = HksSetKeyToMaterial(usageSpec.algType, false, key, &keyMaterial);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "set key to material failed, ret:%" LOG_PUBLIC "x!", ret)
        ret = GetSignVerifyMessage(&usageSpec, srcData, &message, &needFree);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "SignVerify calc hash failed!")

        HksLocalCryptoAbilityInit();
        ret = HksCryptoHalSign(&keyMaterial, &usageSpec, &message, signature);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "local engine verify failed, ret:%" LOG_PUBLIC "x!", ret)
    } while (0);

    if (needFree) {
        HKS_FREE_PTR(message.data);
    }
    if (keyMaterial.data != NULL) {
        (void)memset_s(keyMaterial.data, keyMaterial.size, 0, keyMaterial.size);
        HKS_FREE_PTR(keyMaterial.data);
    }
    return ret;
}

int32_t HksLocalVerify(const struct HksBlob *key, const struct HksParamSet *paramSet, const struct HksBlob *srcData,
    const struct HksBlob *signature)
{
    int32_t ret = CheckLocalSignVerifyParams(HKS_CMD_ID_VERIFY, key, paramSet, srcData, signature);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    struct HksUsageSpec usageSpec = {0};
    HksFillUsageSpec(paramSet, &usageSpec);

    bool needFree = true;
    struct HksBlob message = { 0, NULL };
    struct HksBlob keyMaterial = { 0, NULL };
    do {
        ret = HksSetKeyToMaterial(usageSpec.algType, true, key, &keyMaterial);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "set key to material failed, ret:%" LOG_PUBLIC "x!", ret)

        ret = GetSignVerifyMessage(&usageSpec, srcData, &message, &needFree);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "SignVerify calc hash failed!")

        HksLocalCryptoAbilityInit();
        ret = HksCryptoHalVerify(&keyMaterial, &usageSpec, &message, signature);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "local engine verify failed, ret:%" LOG_PUBLIC "x!", ret)
    } while (0);

    if (needFree) {
        HKS_FREE_PTR(message.data);
    }
    if (keyMaterial.data != NULL) {
        (void)memset_s(keyMaterial.data, keyMaterial.size, 0, keyMaterial.size);
        HKS_FREE_PTR(keyMaterial.data);
    }
    return ret;
}
#endif