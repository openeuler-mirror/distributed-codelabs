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

#include "hks_ipc_serialization.h"

#include "hks_log.h"
#include "hks_mem.h"
#include "hks_template.h"

static int32_t CopyUint32ToBuffer(uint32_t value, const struct HksBlob *destBlob, uint32_t *destOffset)
{
    if (*destOffset > destBlob->size) {
        return HKS_ERROR_BUFFER_TOO_SMALL;
    }

    if (memcpy_s(destBlob->data + *destOffset, destBlob->size - *destOffset, &(value), sizeof(value)) != EOK) {
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }

    *destOffset += sizeof(value);
    return HKS_SUCCESS;
}

static int32_t CopyBlobToBuffer(const struct HksBlob *blob, const struct HksBlob *destBlob, uint32_t *destOffset)
{
    HKS_IF_NOT_SUCC_RETURN(CheckBlob(blob), HKS_ERROR_INVALID_ARGUMENT)

    if ((*destOffset > destBlob->size) ||
        ((destBlob->size - *destOffset) < (sizeof(blob->size) + ALIGN_SIZE(blob->size)))) {
        return HKS_ERROR_BUFFER_TOO_SMALL;
    }

    (void)memcpy_s(destBlob->data + *destOffset, destBlob->size - *destOffset, &(blob->size), sizeof(blob->size));

    *destOffset += sizeof(blob->size);

    (void)memcpy_s(destBlob->data + *destOffset, destBlob->size - *destOffset, blob->data, blob->size);

    *destOffset += ALIGN_SIZE(blob->size);
    return HKS_SUCCESS;
}

static int32_t CopyParamSetToBuffer(const struct HksParamSet *paramSet,
    const struct HksBlob *destBlob, uint32_t *destOffset)
{
    HKS_IF_NULL_RETURN(paramSet, HKS_ERROR_INVALID_ARGUMENT)

    if ((*destOffset > destBlob->size) || (destBlob->size - *destOffset < ALIGN_SIZE(paramSet->paramSetSize))) {
        return HKS_ERROR_BUFFER_TOO_SMALL;
    }

    (void)memcpy_s(destBlob->data + *destOffset, destBlob->size - *destOffset, paramSet, paramSet->paramSetSize);

    *destOffset += ALIGN_SIZE(paramSet->paramSetSize);
    return HKS_SUCCESS;
}

static int32_t GetUint32FromBuffer(uint32_t *value, const struct HksBlob *srcBlob, uint32_t *srcOffset)
{
    if ((*srcOffset > srcBlob->size) || (srcBlob->size - *srcOffset < sizeof(uint32_t))) {
        return HKS_ERROR_BUFFER_TOO_SMALL;
    }

    (void)memcpy_s(value, sizeof(*value), srcBlob->data + *srcOffset, sizeof(uint32_t));

    *srcOffset += sizeof(uint32_t);
    return HKS_SUCCESS;
}

int32_t GetBlobFromBuffer(struct HksBlob *blob, const struct HksBlob *srcBlob, uint32_t *srcOffset)
{
    if ((*srcOffset > srcBlob->size) || ((srcBlob->size - *srcOffset) < sizeof(uint32_t))) {
        return HKS_ERROR_BUFFER_TOO_SMALL;
    }

    uint32_t size = *((uint32_t *)(srcBlob->data + *srcOffset));
    if (ALIGN_SIZE(size) > srcBlob->size - *srcOffset - sizeof(uint32_t)) {
        return HKS_ERROR_BUFFER_TOO_SMALL;
    }

    blob->size = size;
    *srcOffset += sizeof(blob->size);
    blob->data = (uint8_t *)(srcBlob->data + *srcOffset);
    *srcOffset += ALIGN_SIZE(blob->size);
    return HKS_SUCCESS;
}

static int32_t GetParamSetFromBuffer(struct HksParamSet **paramSet,
    const struct HksBlob *srcBlob, uint32_t *srcOffset)
{
    if (*srcOffset > srcBlob->size || ((srcBlob->size - *srcOffset) < sizeof(uint32_t))) {
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    uint32_t size = *((uint32_t *)(srcBlob->data + *srcOffset));
    if (ALIGN_SIZE(size) > srcBlob->size - *srcOffset) {
        return HKS_ERROR_BUFFER_TOO_SMALL;
    }

    *paramSet = (struct HksParamSet*)(srcBlob->data + *srcOffset);
    *srcOffset += ALIGN_SIZE((*paramSet)->paramSetSize);
    return HKS_SUCCESS;
}

static int32_t GetKeyAndParamSetFromBuffer(const struct HksBlob *srcData, struct HksBlob *keyAlias,
    struct HksParamSet **paramSet, uint32_t *offset)
{
    int32_t ret = GetBlobFromBuffer(keyAlias, srcData, offset);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get keyAlias failed")

    ret = GetParamSetFromBuffer(paramSet, srcData, offset);
    HKS_IF_NOT_SUCC_LOGE(ret, "get paramSet failed")

    return ret;
}

static int32_t MallocBlobFromBuffer(const struct HksBlob *srcData, struct HksBlob *blob, uint32_t *offset)
{
    uint32_t blobSize = 0;
    int32_t ret = GetUint32FromBuffer(&blobSize, srcData, offset);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get blobSize failed")

    if (IsInvalidLength(blobSize)) {
        HKS_LOG_E("get blobSize failed");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    uint8_t *blobData = (uint8_t *)HksMalloc(blobSize);
    HKS_IF_NULL_RETURN(blobData, HKS_ERROR_MALLOC_FAIL)

    blob->data = blobData;
    blob->size = blobSize;
    return HKS_SUCCESS;
}

static int32_t MallocParamSetFromBuffer(const struct HksBlob *srcData, struct HksParamSet **paramSet, uint32_t *offset)
{
    uint32_t paramSetOutSize = 0;
    int32_t ret = GetUint32FromBuffer(&paramSetOutSize, srcData, offset);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get paramSetOutSize failed")

    if (IsInvalidLength(paramSetOutSize) || paramSetOutSize < sizeof(struct HksParamSet)) {
        HKS_LOG_E("get paramSetOutSize failed");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    *paramSet = (struct HksParamSet *)HksMalloc(paramSetOutSize);
    HKS_IF_NULL_RETURN(*paramSet, HKS_ERROR_MALLOC_FAIL)

    (*paramSet)->paramSetSize = paramSetOutSize;
    return HKS_SUCCESS;
}

int32_t HksGenerateKeyUnpack(const struct HksBlob *srcData, struct HksBlob *keyAlias,
    struct HksParamSet **paramSetIn, struct HksBlob *keyOut)
{
    uint32_t offset = 0;
    int32_t ret = GetKeyAndParamSetFromBuffer(srcData, keyAlias, paramSetIn, &offset);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "GetKeyAndParamSetFromBuffer failed")

    uint32_t keyOutSize = 0;
    ret = GetUint32FromBuffer(&keyOutSize, srcData, &offset);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get keyOutSize failed")

    if (keyOutSize > MAX_OUT_BLOB_SIZE) {
        HKS_LOG_E("keyOutSize out of range %" LOG_PUBLIC "u", keyOutSize);
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    /* no allocate memory when keyOutSize is 0 */
    if (keyOutSize > 0) {
        uint8_t *keyData = (uint8_t *)HksMalloc(keyOutSize);
        HKS_IF_NULL_RETURN(keyData, HKS_ERROR_MALLOC_FAIL)

        keyOut->data = keyData;
        keyOut->size = keyOutSize;
    }

    return HKS_SUCCESS;
}

int32_t HksImportKeyUnpack(const struct HksBlob *srcData, struct HksBlob *keyAlias, struct HksParamSet **paramSet,
    struct HksBlob *key)
{
    uint32_t offset = 0;
    int32_t ret = GetKeyAndParamSetFromBuffer(srcData, keyAlias, paramSet, &offset);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "GetKeyAndParamSetFromBuffer failed")

    return GetBlobFromBuffer(key, srcData, &offset);
}

int32_t HksImportWrappedKeyUnpack(const struct HksBlob *srcData, struct HksBlob *keyAlias,
    struct HksBlob *wrappingKeyAlias, struct HksParamSet **paramSet, struct HksBlob *wrappedKeyData)
{
    uint32_t offset = 0;
    int32_t ret = GetBlobFromBuffer(keyAlias, srcData, &offset);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get keyAlias failed")

    ret = GetBlobFromBuffer(wrappingKeyAlias, srcData, &offset);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get wrappingKeyAlias failed")

    ret = GetParamSetFromBuffer(paramSet, srcData, &offset);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get paramSet failed")

    return GetBlobFromBuffer(wrappedKeyData, srcData, &offset);
}

int32_t HksExportPublicKeyUnpack(const struct HksBlob *srcData, struct HksBlob *keyAlias, struct HksBlob *key)
{
    uint32_t offset = 0;
    int32_t ret = GetBlobFromBuffer(keyAlias, srcData, &offset);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get keyAlias failed")

    ret = MallocBlobFromBuffer(srcData, key, &offset);
    HKS_IF_NOT_SUCC_LOGE(ret, "malloc key data failed")

    return ret;
}

int32_t HksGetKeyParamSetUnpack(const struct HksBlob *srcData, struct HksBlob *keyAlias,
    struct HksParamSet **paramSet)
{
    uint32_t offset = 0;
    int32_t ret = GetBlobFromBuffer(keyAlias, srcData, &offset);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get keyAlias failed")

    ret = MallocParamSetFromBuffer(srcData, paramSet, &offset);
    HKS_IF_NOT_SUCC_LOGE(ret, "malloc paramSet failed")

    return ret;
}

static int32_t SignVerifyMacUnpack(const struct HksBlob *srcData, struct HksBlob *key, struct HksParamSet **paramSet,
    struct HksBlob *inputData, uint32_t *offset)
{
    int32_t ret = GetKeyAndParamSetFromBuffer(srcData, key, paramSet, offset);
    HKS_IF_NOT_SUCC_LOGE(ret, "GetKeyAndParamSetFromBuffer failed")

    ret = GetBlobFromBuffer(inputData, srcData, offset);
    HKS_IF_NOT_SUCC_LOGE(ret, "get unsignedData failed")

    return ret;
}

int32_t HksSignUnpack(const struct HksBlob *srcData, struct HksBlob *key, struct HksParamSet **paramSet,
    struct HksBlob *unsignedData, struct HksBlob *signature)
{
    uint32_t offset = 0;
    int32_t ret = SignVerifyMacUnpack(srcData, key, paramSet, unsignedData, &offset);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "SignVerifyMacUnpack failed")

    ret = MallocBlobFromBuffer(srcData, signature, &offset);
    HKS_IF_NOT_SUCC_LOGE(ret, "malloc signature data failed")

    return ret;
}

int32_t HksVerifyUnpack(const struct HksBlob *srcData, struct HksBlob *key, struct HksParamSet **paramSet,
    struct HksBlob *unsignedData, struct HksBlob *signature)
{
    uint32_t offset = 0;
    int32_t ret = SignVerifyMacUnpack(srcData, key, paramSet, unsignedData, &offset);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "SignVerifyMacUnpack failed")

    return GetBlobFromBuffer(signature, srcData, &offset);
}

int32_t HksEncryptDecryptUnpack(const struct HksBlob *srcData, struct HksBlob *key,
    struct HksParamSet **paramSet, struct HksBlob *inputText, struct HksBlob *outputText)
{
    uint32_t offset = 0;
    int32_t ret = GetKeyAndParamSetFromBuffer(srcData, key, paramSet, &offset);
    HKS_IF_NOT_SUCC_LOGE(ret, "getKeyAndParamSetFromBuffer failed")

    ret = GetBlobFromBuffer(inputText, srcData, &offset);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get inputText failed")

    ret = MallocBlobFromBuffer(srcData, outputText, &offset);
    HKS_IF_NOT_SUCC_LOGE(ret, "malloc outputText data failed")

    return ret;
}

int32_t HksAgreeKeyUnpack(const struct HksBlob *srcData, struct HksParamSet **paramSet, struct HksBlob *privateKey,
    struct HksBlob *peerPublicKey, struct HksBlob *agreedKey)
{
    uint32_t offset = 0;
    int32_t ret = GetParamSetFromBuffer(paramSet, srcData, &offset);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get paramSet failed")

    ret = GetBlobFromBuffer(privateKey, srcData, &offset);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get privateKey failed")

    ret = GetBlobFromBuffer(peerPublicKey, srcData, &offset);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get peerPublicKey failed")

    ret = MallocBlobFromBuffer(srcData, agreedKey, &offset);
    HKS_IF_NOT_SUCC_LOGE(ret, "malloc agreedKey data failed")

    return ret;
}

int32_t HksDeriveKeyUnpack(const struct HksBlob *srcData, struct HksParamSet **paramSet, struct HksBlob *kdfKey,
    struct HksBlob *derivedKey)
{
    uint32_t offset = 0;
    int32_t ret = GetParamSetFromBuffer(paramSet, srcData, &offset);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get paramSet failed")

    ret = GetBlobFromBuffer(kdfKey, srcData, &offset);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get kdfKey failed")

    ret = MallocBlobFromBuffer(srcData, derivedKey, &offset);
    HKS_IF_NOT_SUCC_LOGE(ret, "malloc derivedKey data failed")

    return ret;
}

int32_t HksHmacUnpack(const struct HksBlob *srcData, struct HksBlob *key, struct HksParamSet **paramSet,
    struct HksBlob *inputData, struct HksBlob *mac)
{
    uint32_t offset = 0;
    int32_t ret = SignVerifyMacUnpack(srcData, key, paramSet, inputData, &offset);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "SignVerifyMacUnpack failed")

    ret = MallocBlobFromBuffer(srcData, mac, &offset);
    HKS_IF_NOT_SUCC_LOGE(ret, "malloc mac data failed")

    return ret;
}

static int32_t KeyInfoListInit(struct HksKeyInfo *keyInfoList, uint32_t listCount,
    const struct HksBlob *srcData, uint32_t *offset)
{
    uint32_t i = 0;
    int32_t ret = HKS_SUCCESS;
    for (; i < listCount; ++i) {
        ret = MallocBlobFromBuffer(srcData, &keyInfoList[i].alias, offset);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "malloc keyInfoList alias failed")

        ret = MallocParamSetFromBuffer(srcData, &keyInfoList[i].paramSet, offset);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "malloc keyInfoList paramSetSize failed")
    }

    if (ret != HKS_SUCCESS) {
        for (uint32_t j = 0; j < i; ++j) {
            HKS_FREE_BLOB(keyInfoList[j].alias);
            HKS_FREE_PTR(keyInfoList[j].paramSet);
        }
    }
    return ret;
}

int32_t HksGetKeyInfoListUnpack(const struct HksBlob *srcData, uint32_t *listCount, struct HksKeyInfo **keyInfoList)
{
    uint32_t offset = 0;
    int32_t ret = GetUint32FromBuffer(listCount, srcData, &offset);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get count failed")

    if ((UINT32_MAX / sizeof(struct HksKeyInfo)) < *listCount) {
        HKS_LOG_E("listCount too big %" LOG_PUBLIC "u", *listCount);
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }

    uint32_t keyInfoListSize = (*listCount) * sizeof(struct HksKeyInfo);
    if (IsInvalidLength(keyInfoListSize)) {
        HKS_LOG_E("keyInfoListSize too big %" LOG_PUBLIC "u", keyInfoListSize);
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }

    *keyInfoList = (struct HksKeyInfo *)HksMalloc(keyInfoListSize);
    HKS_IF_NULL_LOGE_RETURN(*keyInfoList, HKS_ERROR_MALLOC_FAIL, "*keyInfoList is NULL")

    (void)memset_s(*keyInfoList, keyInfoListSize, 0, keyInfoListSize);

    ret = KeyInfoListInit(*keyInfoList, *listCount, srcData, &offset);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("KeyInfoListInit failed");
        HKS_FREE_PTR(*keyInfoList);
    }

    return ret;
}

int32_t HksGetKeyInfoListPackFromService(struct HksBlob *destData, uint32_t listCount,
    const struct HksKeyInfo *keyInfoList)
{
    uint32_t offset = 0;
    int32_t ret = CopyUint32ToBuffer(listCount, destData, &offset);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "copy CopyUint32ToBuffer failed")

    for (uint32_t i = 0; i < listCount; ++i) {
        ret = CopyBlobToBuffer(&keyInfoList[i].alias, destData, &offset);
        HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "copy alias failed")

        ret = CopyParamSetToBuffer(keyInfoList[i].paramSet, destData, &offset);
        HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "copy paramSet failed")
    }

    return HKS_SUCCESS;
}

int32_t HksCertificateChainUnpack(const struct HksBlob *srcData, struct HksBlob *keyAlias,
    struct HksParamSet **paramSet, struct HksBlob *certChainBlob)
{
    uint32_t offset = 0;
    int32_t ret = GetKeyAndParamSetFromBuffer(srcData, keyAlias, paramSet, &offset);
    HKS_IF_NOT_SUCC_LOGE(ret, "GetKeyAndParamSetFromBuffer failed")

    ret = MallocBlobFromBuffer(srcData, certChainBlob, &offset);
    HKS_IF_NOT_SUCC_LOGE(ret, "malloc certChainBlob data failed")

    return ret;
}

static int32_t GetNullBlobParam(const struct HksParamSet *paramSet, struct HksParamOut *outParams)
{
    if (GetTagType(outParams->tag) != HKS_TAG_TYPE_BYTES) {
        HKS_LOG_E("get param tag[0x%" LOG_PUBLIC "x] from ipc buffer failed", outParams->tag);
        return HKS_ERROR_PARAM_NOT_EXIST;
    }

    struct HksParam *param = NULL;
    int32_t ret = HksGetParam(paramSet, outParams->tag + HKS_PARAM_BUFFER_NULL_INTERVAL, &param);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get param tag[0x%" LOG_PUBLIC "x] from ipc buffer failed",
            outParams->tag + HKS_PARAM_BUFFER_NULL_INTERVAL)

    outParams->blob->data = NULL;
    outParams->blob->size = 0;
    return HKS_SUCCESS;
}

static int32_t GetNormalParam(const struct HksParam *param, struct HksParamOut *outParams)
{
    switch (GetTagType(outParams->tag)) {
        case HKS_TAG_TYPE_INT:
            *(outParams->int32Param) = param->int32Param;
            break;
        case HKS_TAG_TYPE_UINT:
            *(outParams->uint32Param) = param->uint32Param;
            break;
        case HKS_TAG_TYPE_ULONG:
            *(outParams->uint64Param) = param->uint64Param;
            break;
        case HKS_TAG_TYPE_BOOL:
            *(outParams->boolParam) = param->boolParam;
            break;
        case HKS_TAG_TYPE_BYTES:
            *(outParams->blob) = param->blob;
            break;
        default:
            HKS_LOG_I("invalid tag type:%" LOG_PUBLIC "x", GetTagType(outParams->tag));
            return HKS_ERROR_INVALID_ARGUMENT;
    }
    return HKS_SUCCESS;
}

int32_t HksParamSetToParams(const struct HksParamSet *paramSet, struct HksParamOut *outParams, uint32_t cnt)
{
    struct HksParam *param = NULL;
    for (uint32_t i = 0; i < cnt; i++) {
        int32_t ret = HksGetParam(paramSet, outParams[i].tag, &param);
        if (ret == HKS_SUCCESS) {
            ret = GetNormalParam(param, &outParams[i]);
        } else {
            ret = GetNullBlobParam(paramSet, &outParams[i]);
        }
        HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get param failed, ret = %" LOG_PUBLIC "d", ret)
    }
    return HKS_SUCCESS;
}

