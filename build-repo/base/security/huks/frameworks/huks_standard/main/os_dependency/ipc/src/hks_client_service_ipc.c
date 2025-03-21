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

#include "hks_client_ipc.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "hks_common_check.h"
#include "hks_ipc_check.h"
#include "hks_ipc_serialization.h"
#include "hks_ipc_slice.h"
#include "hks_log.h"
#include "hks_mem.h"
#include "hks_param.h"
#include "hks_request.h"
#include "hks_template.h"
#include "hks_type.h"
#include "hks_type_inner.h"
#include "securec.h"

int32_t HksClientInitialize(void)
{
    return HKS_SUCCESS;
}

int32_t HksClientRefreshKeyInfo(void)
{
    return HKS_SUCCESS;
}

int32_t HksClientGenerateKey(const struct HksBlob *keyAlias, const struct HksParamSet *paramSetIn,
    struct HksParamSet *paramSetOut)
{
    int32_t ret = HksCheckIpcGenerateKey(keyAlias, paramSetIn);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "HksCheckIpcGenerateKey fail")

    struct HksBlob inBlob = { 0, NULL };
    struct HksBlob outBlob = { 0, NULL };
    inBlob.size = sizeof(keyAlias->size) + ALIGN_SIZE(keyAlias->size) + ALIGN_SIZE(paramSetIn->paramSetSize) +
        sizeof(outBlob.size);
    inBlob.data = (uint8_t *)HksMalloc(inBlob.size);
    HKS_IF_NULL_RETURN(inBlob.data, HKS_ERROR_MALLOC_FAIL)
    if (paramSetOut != NULL) {
        outBlob.size = paramSetOut->paramSetSize;
        outBlob.data = (uint8_t *)paramSetOut;
    }

    do {
        ret = HksGenerateKeyPack(&inBlob, keyAlias, paramSetIn, &outBlob);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksGenerateKeyPack fail")

        ret = HksSendRequest(HKS_MSG_GEN_KEY, &inBlob, &outBlob, paramSetIn);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksSendRequest fail, ret = %" LOG_PUBLIC "d", ret)

        if (paramSetOut != NULL) {
            ret = HksFreshParamSet(paramSetOut, false);
            HKS_IF_NOT_SUCC_LOGE(ret, "FreshParamSet fail, ret = %" LOG_PUBLIC "d", ret)
        }
    } while (0);

    HKS_FREE_BLOB(inBlob);
    return ret;
}

int32_t HksClientImportKey(const struct HksBlob *keyAlias, const struct HksParamSet *paramSet,
    const struct HksBlob *key)
{
    int32_t ret = HksCheckIpcImportKey(keyAlias, paramSet, key);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "HksCheckIpcImportKey fail")

    struct HksBlob inBlob = { 0, NULL };
    inBlob.size = sizeof(keyAlias->size) + ALIGN_SIZE(keyAlias->size) + ALIGN_SIZE(paramSet->paramSetSize) +
        sizeof(key->size) + ALIGN_SIZE(key->size);
    inBlob.data = (uint8_t *)HksMalloc(inBlob.size);
    HKS_IF_NULL_RETURN(inBlob.data, HKS_ERROR_MALLOC_FAIL)

    do {
        ret = HksImportKeyPack(&inBlob, keyAlias, paramSet, key);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksImportKeyPack fail")

        ret = HksSendRequest(HKS_MSG_IMPORT_KEY, &inBlob, NULL, paramSet);
    } while (0);

    HKS_FREE_BLOB(inBlob);
    return ret;
}

int32_t HksClientExportPublicKey(const struct HksBlob *keyAlias, const struct HksParamSet *paramSet,
    struct HksBlob *key)
{
    int32_t ret = HksCheckIpcExportPublicKey(keyAlias, key);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "HksCheckIpcExportPublicKey fail")

    struct HksBlob inBlob = { 0, NULL };
    inBlob.size = sizeof(keyAlias->size) + ALIGN_SIZE(keyAlias->size) + sizeof(key->size);
    inBlob.data = (uint8_t *)HksMalloc(inBlob.size);
    HKS_IF_NULL_RETURN(inBlob.data, HKS_ERROR_MALLOC_FAIL)

    do {
        ret = HksExportPublicKeyPack(&inBlob, keyAlias, key);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksExportPublicKeyPack fail")

        ret = HksSendRequest(HKS_MSG_EXPORT_PUBLIC_KEY, &inBlob, key, paramSet);
    } while (0);

    HKS_FREE_BLOB(inBlob);
    return ret;
}

int32_t HksClientImportWrappedKey(const struct HksBlob *keyAlias, const struct HksBlob *wrappingKeyAlias,
    const struct HksParamSet *paramSet, const struct HksBlob *wrappedKeyData)
{
    int32_t ret = HksCheckIpcImportWrappedKey(keyAlias, wrappingKeyAlias, paramSet, wrappedKeyData);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "HksClientImportWrappedKey fail")

    struct HksBlob inBlob = { 0, NULL };
    inBlob.size = sizeof(keyAlias->size) + ALIGN_SIZE(keyAlias->size) +
                  sizeof(wrappingKeyAlias->size) + ALIGN_SIZE(wrappingKeyAlias->size) +
                  ALIGN_SIZE(paramSet->paramSetSize) +
                  sizeof(wrappedKeyData->size) + ALIGN_SIZE(wrappedKeyData->size);
    inBlob.data = (uint8_t *)HksMalloc(inBlob.size);
    HKS_IF_NULL_RETURN(inBlob.data, HKS_ERROR_MALLOC_FAIL)

    do {
        ret = HksImportWrappedKeyPack(&inBlob, keyAlias, wrappingKeyAlias, paramSet, wrappedKeyData);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksImportWrappedKeyPack fail")

        ret = HksSendRequest(HKS_MSG_IMPORT_WRAPPED_KEY, &inBlob, NULL, paramSet);
    } while (0);

    HKS_FREE_BLOB(inBlob);
    return ret;
}

int32_t HksClientDeleteKey(const struct HksBlob *keyAlias, const struct HksParamSet *paramSet)
{
    HKS_IF_NOT_SUCC_RETURN(CheckBlob(keyAlias), HKS_ERROR_INVALID_ARGUMENT)
    if (keyAlias->size > MAX_PROCESS_SIZE) {
        HKS_LOG_E("CheckDeleteKeyParam fail");
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    return HksSendRequest(HKS_MSG_DELETE_KEY, keyAlias, NULL, paramSet);
}

int32_t HksClientGetKeyParamSet(const struct HksBlob *keyAlias, struct HksParamSet *paramSetOut)
{
    int32_t ret = HksCheckIpcGetKeyParamSet(keyAlias, paramSetOut);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    struct HksBlob inBlob = { 0, NULL };
    struct HksBlob outBlob = { paramSetOut->paramSetSize, (uint8_t *)paramSetOut };
    inBlob.size = sizeof(keyAlias->size) + ALIGN_SIZE(keyAlias->size) + sizeof(paramSetOut->paramSetSize);
    inBlob.data = (uint8_t *)HksMalloc(inBlob.size);
    HKS_IF_NULL_RETURN(inBlob.data, HKS_ERROR_MALLOC_FAIL)

    do {
        ret = HksGetKeyParamSetPack(&inBlob, keyAlias, &outBlob);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksGenerateKeyPack fail")

        ret = HksSendRequest(HKS_MSG_GET_KEY_PARAMSET, &inBlob, &outBlob, NULL);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksSendRequest fail, ret = %" LOG_PUBLIC "d", ret)

        ret = HksFreshParamSet(paramSetOut, false);
        HKS_IF_NOT_SUCC_LOGE(ret, "FreshParamSet fail, ret = %" LOG_PUBLIC "d", ret)
    } while (0);

    HKS_FREE_BLOB(inBlob);
    return ret;
}

int32_t HksClientKeyExist(const struct HksBlob *keyAlias, const struct HksParamSet *paramSet)
{
    HKS_IF_NOT_SUCC_RETURN(CheckBlob(keyAlias), HKS_ERROR_INVALID_ARGUMENT)
    if (keyAlias->size > MAX_PROCESS_SIZE) {
        HKS_LOG_E("CheckKeyExistParam fail");
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    return HksSendRequest(HKS_MSG_KEY_EXIST, keyAlias, NULL, paramSet);
}

int32_t HksClientGenerateRandom(struct HksBlob *random, const struct HksParamSet *paramSet)
{
    HKS_IF_NOT_SUCC_RETURN(CheckBlob(random), HKS_ERROR_INVALID_ARGUMENT)
    struct HksBlob inBlob = { sizeof(random->size), (uint8_t *)&(random->size) };
    return HksSendRequest(HKS_MSG_GENERATE_RANDOM, &inBlob, random, paramSet);
}

int32_t HksClientSign(const struct HksBlob *key, const struct HksParamSet *paramSet,
    const struct HksBlob *srcData, struct HksBlob *signature)
{
    int32_t ret = HksCheckBlob3AndParamSet(key, srcData, signature, paramSet);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "check in and out data failed")

    struct HksBlob tmpInData = *srcData;
    struct HksBlob tmpOutData = *signature;
    ret = HksSliceDataEntry(HKS_MSG_SIGN, key, paramSet, &tmpInData, &tmpOutData);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("HksClientSign fail");
    } else {
        signature->size = tmpOutData.size;
    }
    return ret;
}

int32_t HksClientVerify(const struct HksBlob *key, const struct HksParamSet *paramSet,
    const struct HksBlob *srcData, const struct HksBlob *signature)
{
    int32_t ret = HksCheckBlob3AndParamSet(key, srcData, signature, paramSet);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "check in and out data failed")

    struct HksBlob tmpInData = *srcData;
    struct HksBlob tmpOutData = *signature;
    ret = HksSliceDataEntry(HKS_MSG_VERIFY, key, paramSet, &tmpInData, &tmpOutData);
    HKS_IF_NOT_SUCC_LOGE(ret, "HksClientVerify fail")
    return ret;
}

static int32_t AddAeTag(struct HksParamSet *paramSet, const struct HksBlob *inText, bool isEncrypt)
{
    int32_t ret;
    if (!isEncrypt) {
        if (inText->size <= HKS_AE_TAG_LEN) {
            HKS_LOG_E("too small inText size");
            return HKS_ERROR_INVALID_ARGUMENT;
        }

        struct HksParam aeParam;
        aeParam.tag = HKS_TAG_AE_TAG;
        aeParam.blob.data = inText->data + inText->size - HKS_AE_TAG_LEN;
        aeParam.blob.size = HKS_AE_TAG_LEN;
        ret = HksAddParams(paramSet, &aeParam, 1);
        HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "decrypt add ae params failed")
    }

    struct HksParam payloadParam;
    payloadParam.tag = HKS_TAG_PAYLOAD_LEN;
    payloadParam.uint32Param = inText->size;
    ret = HksAddParams(paramSet, &payloadParam, 1);
    HKS_IF_NOT_SUCC_LOGE(ret, "add payload param failed")
    return ret;
}

static int32_t AddAesTag(const struct HksParamSet *paramSet, struct HksParamSet *newParamSet,
    struct HksBlob *inText, bool isEncrypt)
{
    bool isAeMode = false;
    bool isAes = false;
    int32_t ret = HksCheckAesAeMode(paramSet, &isAes, &isAeMode);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    /* Except for AES GCM and CCM mode, no need add tag, return success */
    if ((!isAes) || (!isAeMode)) {
        HKS_LOG_I("Not AES CCM or GCM mode!");
        return HKS_SUCCESS;
    }
    return AddAeTag(newParamSet, inText, isEncrypt);
}

static int32_t AppendToNewParamSet(const struct HksParamSet *paramSet, struct HksParamSet **outParamSet)
{
    struct HksParamSet *newParamSet = NULL;
    int32_t ret = HksInitParamSet(&newParamSet);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "append init operation param set fail")

    ret = HksAddParams(newParamSet, paramSet->params, paramSet->paramsCnt);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("append add in params fail");
        HksFreeParamSet(&newParamSet);
        return ret;
    }

    *outParamSet = newParamSet;
    return HKS_SUCCESS;
}

static int32_t AppendCipherTag(const struct HksParamSet *paramSet, const struct HksBlob *inText, bool isEncrypt,
    struct HksParamSet **outParamSet)
{
    struct HksParamSet *newParamSet = NULL;
    int32_t ret = AppendToNewParamSet(paramSet, &newParamSet);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "append cipher client service tag fail")

    do {
        ret = AddAesTag(paramSet, newParamSet, (struct HksBlob *)inText, isEncrypt);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "append add Aes Tag fail")

        ret = HksBuildParamSet(&newParamSet);
        HKS_IF_NOT_SUCC_LOGE(ret, "append build paramset fail")
    } while (0);
    if (ret != HKS_SUCCESS) {
        HksFreeParamSet(&newParamSet);
        return ret;
    }

    *outParamSet = newParamSet;
    return HKS_SUCCESS;
}

int32_t HksClientEncrypt(const struct HksBlob *key, const struct HksParamSet *paramSet,
    const struct HksBlob *plainText, struct HksBlob *cipherText)
{
    int32_t ret = HksCheckBlob3AndParamSet(key, plainText, cipherText, paramSet);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "check in and out data failed")

    struct HksParamSet *newParamSet = NULL;
    ret = AppendCipherTag(paramSet, plainText, true, &newParamSet);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "AppendCipherTag fail, ret = %" LOG_PUBLIC "d", ret)

    struct HksBlob tmpInData = *plainText;
    struct HksBlob tmpOutData = *cipherText;
    ret = HksSliceDataEntry(HKS_MSG_ENCRYPT, key, newParamSet, &tmpInData, &tmpOutData);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("HksClientEncrypt fail");
    } else {
        cipherText->size = tmpOutData.size;
    }

    HksFreeParamSet(&newParamSet);
    return ret;
}

int32_t HksClientDecrypt(const struct HksBlob *key, const struct HksParamSet *paramSet,
    const struct HksBlob *cipherText, struct HksBlob *plainText)
{
    int32_t ret = HksCheckBlob3AndParamSet(key, plainText, cipherText, paramSet);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "check in and out data failed")

    struct HksParamSet *newParamSet = NULL;
    struct HksBlob tmpCipherText = *cipherText;
    ret = AppendCipherTag(paramSet, &tmpCipherText, false, &newParamSet);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "AppendCipherTag fail, ret = %" LOG_PUBLIC "d", ret)

    struct HksBlob tmpOutData = *plainText;
    ret = HksSliceDataEntry(HKS_MSG_DECRYPT, key, newParamSet, &tmpCipherText, &tmpOutData);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("HksClientDecrypt fail");
    } else {
        plainText->size = tmpOutData.size;
    }

    HksFreeParamSet(&newParamSet);
    return ret;
}

int32_t HksClientAgreeKey(const struct HksParamSet *paramSet, const struct HksBlob *privateKey,
    const struct HksBlob *peerPublicKey, struct HksBlob *agreedKey)
{
    int32_t ret = HksCheckIpcAgreeKey(paramSet, privateKey, peerPublicKey, agreedKey);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "HksCheckIpcAgreeKey fail")

    struct HksBlob inBlob = { 0, NULL };
    inBlob.size = ALIGN_SIZE(paramSet->paramSetSize) + sizeof(privateKey->size) + ALIGN_SIZE(privateKey->size) +
        sizeof(peerPublicKey->size) + ALIGN_SIZE(peerPublicKey->size) + sizeof(agreedKey->size);
    inBlob.data = (uint8_t *)HksMalloc(inBlob.size);
    HKS_IF_NULL_RETURN(inBlob.data, HKS_ERROR_MALLOC_FAIL)

    do {
        ret = HksAgreeKeyPack(&inBlob, paramSet, privateKey, peerPublicKey, agreedKey);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksAgreeKeyPack fail")

        ret = HksSendRequest(HKS_MSG_AGREE_KEY, &inBlob, agreedKey, paramSet);
    } while (0);

    HKS_FREE_BLOB(inBlob);
    return ret;
}

int32_t HksClientDeriveKey(const struct HksParamSet *paramSet, const struct HksBlob *mainKey,
    struct HksBlob *derivedKey)
{
    int32_t ret = HksCheckIpcDeriveKey(paramSet, mainKey, derivedKey);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "HksCheckIpcDeriveKey fail")

    struct HksBlob inBlob = { 0, NULL };
    inBlob.size = ALIGN_SIZE(paramSet->paramSetSize) + sizeof(mainKey->size) + ALIGN_SIZE(mainKey->size) +
        sizeof(derivedKey->size);
    inBlob.data = (uint8_t *)HksMalloc(inBlob.size);
    HKS_IF_NULL_RETURN(inBlob.data, HKS_ERROR_MALLOC_FAIL)

    do {
        ret = HksDeriveKeyPack(&inBlob, paramSet, mainKey, derivedKey);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksDeriveKeyPack fail")

        ret = HksSendRequest(HKS_MSG_DERIVE_KEY, &inBlob, derivedKey, paramSet);
    } while (0);

    HKS_FREE_BLOB(inBlob);
    return ret;
}

int32_t HksClientMac(const struct HksBlob *key, const struct HksParamSet *paramSet, const struct HksBlob *srcData,
    struct HksBlob *mac)
{
    int32_t ret = HksCheckBlob3AndParamSet(key, srcData, mac, paramSet);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "check in and out data failed")

    struct HksBlob tmpInData = *srcData;
    struct HksBlob tmpOutData = *mac;
    ret = HksSliceDataEntry(HKS_MSG_MAC, key, paramSet, &tmpInData, &tmpOutData);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("HksClientMac fail");
    } else {
        mac->size = tmpOutData.size;
    }
    return ret;
}

int32_t HksClientGetKeyInfoList(struct HksKeyInfo *keyInfoList, uint32_t *listCount)
{
    int32_t ret = HksCheckIpcGetKeyInfoList(keyInfoList, *listCount);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "HksCheckIpcGetKeyInfoList fail")

    struct HksBlob inBlob = { 0, NULL };
    inBlob.size = sizeof(*listCount) + (sizeof(keyInfoList->alias.size) +
        sizeof(keyInfoList->paramSet->paramSetSize)) * (*listCount);
    inBlob.data = (uint8_t *)HksMalloc(inBlob.size);
    HKS_IF_NULL_RETURN(inBlob.data, HKS_ERROR_MALLOC_FAIL)

    struct HksBlob outBlob = { 0, NULL };
    outBlob.size += sizeof(*listCount);
    for (uint32_t i = 0; i < *listCount; ++i) {
        outBlob.size += sizeof(keyInfoList[i].alias.size) + ALIGN_SIZE(keyInfoList[i].alias.size) +
            ALIGN_SIZE(keyInfoList[i].paramSet->paramSetSize);
    }

    outBlob.data = (uint8_t *)HksMalloc(outBlob.size);
    if (outBlob.data == NULL) {
        HKS_FREE_BLOB(inBlob);
        return HKS_ERROR_MALLOC_FAIL;
    }

    do {
        ret = HksGetKeyInfoListPack(&inBlob, *listCount, keyInfoList);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksGetKeyInfoListPack fail")

        ret = HksSendRequest(HKS_MSG_GET_KEY_INFO_LIST, &inBlob, &outBlob, NULL);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksSendRequest result is fail")

        ret = HksGetKeyInfoListUnpackFromService(&outBlob, listCount, keyInfoList);
    } while (0);

    HKS_FREE_BLOB(inBlob);
    HKS_FREE_BLOB(outBlob);
    return ret;
}

static int32_t CertificateChainInitBlob(struct HksBlob *inBlob, struct HksBlob *outBlob, const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSet, const struct HksCertChain *certChain)
{
    int32_t ret = HksCheckIpcCertificateChain(keyAlias, paramSet, certChain);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "HksCheckIpcCertificateChain fail")

    uint32_t certBufSize = sizeof(certChain->certsCount);
    for (uint32_t i = 0; i < certChain->certsCount; ++i) {
        certBufSize += sizeof(certChain->certs[i].size) + ALIGN_SIZE(certChain->certs[i].size);
    }

    inBlob->size = sizeof(keyAlias->size) + ALIGN_SIZE(keyAlias->size) + ALIGN_SIZE(paramSet->paramSetSize) +
        sizeof(certBufSize);
    inBlob->data = (uint8_t *)HksMalloc(inBlob->size);
    HKS_IF_NULL_RETURN(inBlob->data, HKS_ERROR_MALLOC_FAIL)

    outBlob->size = certBufSize;
    outBlob->data = (uint8_t *)HksMalloc(certBufSize);
    if (outBlob->data == NULL) {
        HKS_FREE_BLOB(*inBlob);
        return HKS_ERROR_MALLOC_FAIL;
    }

    return HKS_SUCCESS;
}

static int32_t CertificateChainGetOrAttest(enum HksMessage type, const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSet, struct HksCertChain *certChain)
{
    struct HksBlob inBlob = { 0, NULL };
    struct HksBlob outBlob = { 0, NULL };
    
    int32_t ret = CertificateChainInitBlob(&inBlob, &outBlob, keyAlias, paramSet, certChain);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "CertificateChainInitBlob fail")

    do {
        struct HksParam *isBase64Param = NULL;
        bool isBase64 = false;
        ret = HksGetParam(paramSet, HKS_TAG_ATTESTATION_BASE64, &isBase64Param);
        if (ret == HKS_SUCCESS) {
            isBase64 = isBase64Param->boolParam;
        }
        ret = HksCertificateChainPack(&inBlob, keyAlias, paramSet, &outBlob);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksCertificateChainPack fail")

        ret = HksSendRequest(type, &inBlob, &outBlob, paramSet);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "CertificateChainGetOrAttest request fail")
        ret = HksCertificateChainUnpackFromService(&outBlob, isBase64, certChain);
    } while (0);

    HKS_FREE_BLOB(inBlob);
    HKS_FREE_BLOB(outBlob);
    return ret;
}

int32_t HksClientAttestKey(const struct HksBlob *keyAlias, const struct HksParamSet *paramSet,
    struct HksCertChain *certChain)
{
    return CertificateChainGetOrAttest(HKS_MSG_ATTEST_KEY, keyAlias, paramSet, certChain);
}

static int32_t CopyData(const uint8_t *data, const uint32_t size, struct HksBlob *out)
{
    if (size == 0) {
        out->size = 0;
        return HKS_SUCCESS;
    }

    if (out->size < size) {
        HKS_LOG_E("out size[%" LOG_PUBLIC "u] smaller than [%" LOG_PUBLIC "u]", out->size, size);
        return HKS_ERROR_BUFFER_TOO_SMALL;
    }
    (void)memcpy_s(out->data, out->size, data, size);
    out->size = size;
    return HKS_SUCCESS;
}

static int32_t ClientInit(const struct HksBlob *inData, const struct HksParamSet *paramSet,
    struct HksBlob *handle, struct HksBlob *token)
{
    uint8_t *tmpOut = (uint8_t *)HksMalloc(HANDLE_SIZE + TOKEN_SIZE);
    HKS_IF_NULL_LOGE_RETURN(tmpOut, HKS_ERROR_MALLOC_FAIL, "malloc ipc tmp out failed")
    struct HksBlob outBlob = { HANDLE_SIZE + TOKEN_SIZE, tmpOut };

    int32_t ret;
    do {
        ret = HksSendRequest(HKS_MSG_INIT, inData, &outBlob, paramSet);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "client init send fail")

        if (outBlob.size < HANDLE_SIZE) {
            HKS_LOG_E("invalid out size[%" LOG_PUBLIC "u]", outBlob.size);
            ret = HKS_ERROR_INSUFFICIENT_MEMORY;
            break;
        }
        ret = CopyData(outBlob.data, HANDLE_SIZE, handle);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "copy handle failed")

        if (token != NULL) {
            if (outBlob.size < (HANDLE_SIZE + TOKEN_SIZE)) {
                HKS_LOG_W("client init success without out token");
                token->size = 0;
                break;
            }
            if (token->size < TOKEN_SIZE) {
                HKS_LOG_E("copy token failed");
                ret = HKS_ERROR_BUFFER_TOO_SMALL;
                break;
            }

            ret = CopyData(outBlob.data + HANDLE_SIZE, TOKEN_SIZE, token);
            HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "copy token failed")
        }
    } while (0);

    HKS_FREE_PTR(tmpOut);
    return ret;
}

int32_t HksClientInit(const struct HksBlob *keyAlias, const struct HksParamSet *paramSet,
    struct HksBlob *handle, struct HksBlob *token)
{
    struct HksParamSet *sendParamSet = NULL;

    struct HksParam params[] = {
        { .tag = HKS_TAG_PARAM0_BUFFER,
          .blob = *keyAlias },
        { .tag = HKS_TAG_PARAM1_BUFFER,
          .blob = { paramSet->paramSetSize,
                    (uint8_t *)paramSet } },
    };

    int32_t ret = HksParamsToParamSet(params, HKS_ARRAY_SIZE(params), &sendParamSet);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "HksParamsToParamSet fail")

    struct HksBlob parcelBlob = {
        .size = sendParamSet->paramSetSize,
        .data = (uint8_t *)sendParamSet
    };

    ret = ClientInit(&parcelBlob, paramSet, handle, token);
    HksFreeParamSet(&sendParamSet);
    return ret;
}

int32_t HksClientUpdate(const struct HksBlob *handle, const struct HksParamSet *paramSet,
    const struct HksBlob *inData, struct HksBlob *outData)
{
    struct HksParamSet *sendParamSet = NULL;

    struct HksParam params[] = {
        { .tag = HKS_TAG_PARAM0_BUFFER,
          .blob = { paramSet->paramSetSize,
                    (uint8_t *)paramSet } },
        { .tag = HKS_TAG_PARAM1_BUFFER,
          .blob = *handle },
        { .tag = HKS_TAG_PARAM2_BUFFER,
          .blob = *inData },
    };

    int32_t ret = HksParamsToParamSet(params, HKS_ARRAY_SIZE(params), &sendParamSet);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "HksParamSetPack fail")

    struct HksBlob parcelBlob = {
        .size = sendParamSet->paramSetSize,
        .data = (uint8_t *)sendParamSet
    };
    ret = HksSendRequest(HKS_MSG_UPDATE, &parcelBlob, outData, paramSet);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("HksParamSet send fail");
        HksFreeParamSet(&sendParamSet);
        return ret;
    }

    HksFreeParamSet(&sendParamSet);
    return ret;
}

int32_t HksClientFinish(const struct HksBlob *handle, const struct HksParamSet *paramSet,
    const struct HksBlob *inData, struct HksBlob *outData)
{
    struct HksParamSet *sendParamSet = NULL;
    struct HksParam params[] = {
        { .tag = HKS_TAG_PARAM0_BUFFER,
          .blob = { paramSet->paramSetSize,
                    (uint8_t *)paramSet } },
        { .tag = HKS_TAG_PARAM1_BUFFER,
          .blob = *handle },
        { .tag = HKS_TAG_PARAM2_BUFFER,
          .blob = *inData },
        { .tag = HKS_TAG_PARAM3_BUFFER,
          .blob = *outData },
    };

    int32_t ret = HksParamsToParamSet(params, HKS_ARRAY_SIZE(params), &sendParamSet);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "HksParamSetPack fail")

    struct HksBlob parcelBlob = {
        .size = sendParamSet->paramSetSize,
        .data = (uint8_t *)sendParamSet
    };
    ret = HksSendRequest(HKS_MSG_FINISH, &parcelBlob, outData, paramSet);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("HksParamSet send fail");
        HksFreeParamSet(&sendParamSet);
        return ret;
    }

    HksFreeParamSet(&sendParamSet);
    return ret;
}

int32_t HksClientAbort(const struct HksBlob *handle, const struct HksParamSet *paramSet)
{
    struct HksParamSet *sendParamSet = NULL;
    struct HksParam params[] = {
        { .tag = HKS_TAG_PARAM0_BUFFER,
          .blob = { paramSet->paramSetSize,
                    (uint8_t *)paramSet } },
        { .tag = HKS_TAG_PARAM1_BUFFER,
          .blob = *handle },
    };

    int32_t ret = HksParamsToParamSet(params, HKS_ARRAY_SIZE(params), &sendParamSet);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "HksParamSetPack fail")

    struct HksBlob parcelBlob = {
        .size = sendParamSet->paramSetSize,
        .data = (uint8_t *)sendParamSet
    };
    ret = HksSendRequest(HKS_MSG_ABORT, &parcelBlob, NULL, paramSet);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("HksParamSet send fail");
        HksFreeParamSet(&sendParamSet);
        return ret;
    }

    HksFreeParamSet(&sendParamSet);
    return ret;
}
