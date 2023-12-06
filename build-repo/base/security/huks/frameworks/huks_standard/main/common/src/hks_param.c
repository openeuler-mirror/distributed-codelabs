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

#include "hks_param.h"

#include <stddef.h>

#include "hks_log.h"
#include "hks_mem.h"
#include "hks_template.h"
#include "hks_type_inner.h"

#include "securec.h"

static uint32_t g_validTags[] = {
    HKS_TAG_ALGORITHM,
    HKS_TAG_PURPOSE,
    HKS_TAG_KEY_SIZE,
    HKS_TAG_DIGEST,
    HKS_TAG_PADDING,
    HKS_TAG_BLOCK_MODE,
    HKS_TAG_KEY_TYPE,
    HKS_TAG_ASSOCIATED_DATA,
    HKS_TAG_NONCE,
    HKS_TAG_IV,

    HKS_TAG_SALT,
    HKS_TAG_PWD,
    HKS_TAG_INFO,
    HKS_TAG_ITERATION,

    HKS_TAG_KEY_GENERATE_TYPE,
    HKS_TAG_DERIVE_MAIN_KEY,
    HKS_TAG_DERIVE_FACTOR,
    HKS_TAG_DERIVE_ALG,
    HKS_TAG_AGREE_ALG,
    HKS_TAG_AGREE_PUBLIC_KEY_IS_KEY_ALIAS,
    HKS_TAG_AGREE_PRIVATE_KEY_ALIAS,
    HKS_TAG_AGREE_PUBLIC_KEY,
    HKS_TAG_KEY_ALIAS,
    HKS_TAG_DERIVE_KEY_SIZE,
    HKS_TAG_IMPORT_KEY_TYPE,
    HKS_TAG_UNWRAP_ALGORITHM_SUITE,

    HKS_TAG_ACTIVE_DATETIME,
    HKS_TAG_ORIGINATION_EXPIRE_DATETIME,
    HKS_TAG_USAGE_EXPIRE_DATETIME,
    HKS_TAG_CREATION_DATETIME,

    HKS_TAG_ALL_USERS,
    HKS_TAG_USER_ID,
    HKS_TAG_NO_AUTH_REQUIRED,
    HKS_TAG_USER_AUTH_TYPE,
    HKS_TAG_AUTH_TIMEOUT,
    HKS_TAG_AUTH_TOKEN,

    HKS_TAG_OS_VERSION,
    HKS_TAG_OS_PATCHLEVEL,

    HKS_TAG_ATTESTATION_CHALLENGE,
    HKS_TAG_ATTESTATION_APPLICATION_ID,
    HKS_TAG_ATTESTATION_ID_BRAND,
    HKS_TAG_ATTESTATION_ID_DEVICE,
    HKS_TAG_ATTESTATION_ID_PRODUCT,
    HKS_TAG_ATTESTATION_ID_SERIAL,
    HKS_TAG_ATTESTATION_ID_IMEI,
    HKS_TAG_ATTESTATION_ID_MEID,
    HKS_TAG_ATTESTATION_ID_MANUFACTURER,
    HKS_TAG_ATTESTATION_ID_MODEL,
    HKS_TAG_ATTESTATION_ID_ALIAS,
    HKS_TAG_ATTESTATION_ID_SOCID,
    HKS_TAG_ATTESTATION_ID_UDID,
    HKS_TAG_ATTESTATION_ID_SEC_LEVEL_INFO,
    HKS_TAG_ATTESTATION_ID_VERSION_INFO,
    HKS_TAG_ATTESTATION_BASE64,

    HKS_TAG_IS_KEY_ALIAS,
    HKS_TAG_KEY_STORAGE_FLAG,
    HKS_TAG_IS_ALLOWED_WRAP,
    HKS_TAG_KEY_WRAP_TYPE,
    HKS_TAG_KEY_AUTH_ID,
    HKS_TAG_KEY_ROLE,
    HKS_TAG_KEY_FLAG,
    HKS_TAG_KEY_DOMAIN,

    HKS_TAG_KEY_AUTH_ACCESS_TYPE,
    HKS_TAG_KEY_SECURE_SIGN_TYPE,
    HKS_TAG_CHALLENGE_TYPE,
    HKS_TAG_CHALLENGE_POS,

    HKS_TAG_KEY_INIT_CHALLENGE,
    HKS_TAG_IS_USER_AUTH_ACCESS,
    HKS_TAG_USER_AUTH_CHALLENGE,
    HKS_TAG_USER_AUTH_ENROLL_ID_INFO,
    HKS_TAG_USER_AUTH_SECURE_UID,
    HKS_TAG_KEY_AUTH_RESULT,
    HKS_TAG_IF_NEED_APPEND_AUTH_INFO,
    HKS_TAG_VERIFIED_AUTH_TOKEN,
    HKS_TAG_IS_APPEND_UPDATE_DATA,

    HKS_TAG_PROCESS_NAME,
    HKS_TAG_PACKAGE_NAME,
    HKS_TAG_PAYLOAD_LEN,
    HKS_TAG_AE_TAG,
    HKS_TAG_CRYPTO_CTX,
    HKS_TAG_KEY,
    HKS_TAG_KEY_VERSION,
    HKS_TAG_IS_KEY_HANDLE,
    HKS_TAG_SYMMETRIC_KEY_DATA,
    HKS_TAG_ASYMMETRIC_PUBLIC_KEY_DATA,
    HKS_TAG_ASYMMETRIC_PRIVATE_KEY_DATA,
    HKS_TAG_KEY_ACCESS_TIME,

    HKS_TAG_ACCESS_TOKEN_ID,
};

HKS_API_EXPORT enum HksTagType GetTagType(enum HksTag tag)
{
    return (enum HksTagType)((uint32_t)tag & (uint32_t)HKS_TAG_TYPE_MASK);
}

static bool IsValidTag(uint32_t tag)
{
    uint32_t tagSize = HKS_ARRAY_SIZE(g_validTags);
    for (uint32_t i = 0; i < tagSize; ++i) {
        if (tag == g_validTags[i]) {
            return true;
        }
    }
    return false;
}

HKS_API_EXPORT int32_t HksCheckParamSetTag(const struct HksParamSet *paramSet)
{
    HKS_IF_NULL_RETURN(paramSet, HKS_ERROR_NULL_POINTER)

    for (uint32_t i = 0; i < paramSet->paramsCnt; ++i) {
        uint32_t curTag = paramSet->params[i].tag;
        if (!IsValidTag(curTag)) {
            HKS_LOG_E("paramSet contains invalid tag! 0x%" LOG_PUBLIC "x", curTag);
            return HKS_ERROR_INVALID_ARGUMENT;
        }

        for (uint32_t j = i + 1; j < paramSet->paramsCnt; ++j) {
            if (curTag == paramSet->params[j].tag) {
                HKS_LOG_E("paramSet contains multi-tags! 0x%" LOG_PUBLIC "x", curTag);
                return HKS_ERROR_INVALID_ARGUMENT;
            }
        }
    }

    return HKS_SUCCESS;
}

static int32_t CheckBeforeAddParams(const struct HksParamSet *paramSet, const struct HksParam *params,
    uint32_t paramCnt)
{
    if ((params == NULL) || (paramSet == NULL) || (paramSet->paramSetSize > HKS_PARAM_SET_MAX_SIZE) ||
        (paramCnt > HKS_DEFAULT_PARAM_CNT) || (paramSet->paramsCnt > (HKS_DEFAULT_PARAM_CNT - paramCnt))) {
        HKS_LOG_E("invalid params or paramset!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    for (uint32_t i = 0; i < paramCnt; i++) {
        if ((GetTagType((enum HksTag)(params[i].tag)) == HKS_TAG_TYPE_BYTES) &&
            (params[i].blob.data == NULL)) {
            HKS_LOG_E("invalid blob param!");
            return HKS_ERROR_INVALID_ARGUMENT;
        }
    }
    return HKS_SUCCESS;
}

static int32_t BuildParamSet(struct HksParamSet **paramSet)
{
    struct HksParamSet *freshParamSet = *paramSet;
    uint32_t size = freshParamSet->paramSetSize;
    uint32_t offset = sizeof(struct HksParamSet) + sizeof(struct HksParam) * freshParamSet->paramsCnt;

    if (size > HKS_DEFAULT_PARAM_SET_SIZE) {
        freshParamSet = (struct HksParamSet *)HksMalloc(size);
        HKS_IF_NULL_LOGE_RETURN(freshParamSet, HKS_ERROR_MALLOC_FAIL, "malloc params failed!")

        if (memcpy_s(freshParamSet, size, *paramSet, offset) != EOK) {
            HKS_FREE_PTR(freshParamSet);
            HKS_LOG_E("copy params failed!");
            return HKS_ERROR_INSUFFICIENT_MEMORY;
        }
        HKS_FREE_PTR(*paramSet);
        *paramSet = freshParamSet;
    }

    return HksFreshParamSet(freshParamSet, true);
}

HKS_API_EXPORT int32_t HksFreshParamSet(struct HksParamSet *paramSet, bool isCopy)
{
    HKS_IF_NULL_LOGE_RETURN(paramSet, HKS_ERROR_NULL_POINTER, "invalid NULL paramSet")

    int32_t ret = HksCheckParamSet(paramSet, paramSet->paramSetSize);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "invalid fresh paramSet")

    uint32_t size = paramSet->paramSetSize;
    uint32_t offset = sizeof(struct HksParamSet) + sizeof(struct HksParam) * paramSet->paramsCnt;

    for (uint32_t i = 0; i < paramSet->paramsCnt; i++) {
        if (offset > size) {
            HKS_LOG_E("invalid param set offset!");
            return HKS_ERROR_INVALID_ARGUMENT;
        }
        if (GetTagType((enum HksTag)(paramSet->params[i].tag)) == HKS_TAG_TYPE_BYTES) {
            if (IsAdditionOverflow(offset, paramSet->params[i].blob.size)) {
                HKS_LOG_E("blob size overflow!");
                return HKS_ERROR_INVALID_ARGUMENT;
            }

            if (isCopy && (memcpy_s((uint8_t *)paramSet + offset, size - offset,
                paramSet->params[i].blob.data, paramSet->params[i].blob.size) != EOK)) {
                HKS_LOG_E("copy param blob failed!");
                return HKS_ERROR_INSUFFICIENT_MEMORY;
            }
            paramSet->params[i].blob.data = (uint8_t *)paramSet + offset;
            offset += paramSet->params[i].blob.size;
        }
    }

    if (paramSet->paramSetSize != offset) {
        HKS_LOG_E("invalid param set size!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    return HKS_SUCCESS;
}

HKS_API_EXPORT int32_t HksCheckParamSet(const struct HksParamSet *paramSet, uint32_t size)
{
    HKS_IF_NULL_RETURN(paramSet, HKS_ERROR_NULL_POINTER)

    if ((size < sizeof(struct HksParamSet)) || (size > HKS_PARAM_SET_MAX_SIZE) ||
        (paramSet->paramSetSize != size) ||
        (paramSet->paramsCnt > ((size - sizeof(struct HksParamSet)) / sizeof(struct HksParam)))) {
        HKS_LOG_E("invalid param set!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    return HKS_SUCCESS;
}

HKS_API_EXPORT int32_t HksInitParamSet(struct HksParamSet **paramSet)
{
    HKS_IF_NULL_LOGE_RETURN(paramSet, HKS_ERROR_NULL_POINTER, "invalid init params!")

    *paramSet = (struct HksParamSet *)HksMalloc(HKS_DEFAULT_PARAM_SET_SIZE);
    HKS_IF_NULL_LOGE_RETURN(*paramSet, HKS_ERROR_MALLOC_FAIL, "malloc init param set failed!")

    (*paramSet)->paramsCnt = 0;
    (*paramSet)->paramSetSize = sizeof(struct HksParamSet);
    return HKS_SUCCESS;
}

HKS_API_EXPORT int32_t HksAddParams(struct HksParamSet *paramSet,
    const struct HksParam *params, uint32_t paramCnt)
{
    int32_t ret = CheckBeforeAddParams(paramSet, params, paramCnt);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    for (uint32_t i = 0; i < paramCnt; i++) {
        paramSet->paramSetSize += sizeof(struct HksParam);
        if (GetTagType((enum HksTag)(params[i].tag)) == HKS_TAG_TYPE_BYTES) {
            if (IsAdditionOverflow(paramSet->paramSetSize, params[i].blob.size)) {
                HKS_LOG_E("params size overflow!");
                paramSet->paramSetSize -= sizeof(struct HksParam);
                return HKS_ERROR_INVALID_ARGUMENT;
            }
            paramSet->paramSetSize += params[i].blob.size;
        }
        (void)memcpy_s(&paramSet->params[paramSet->paramsCnt++], sizeof(struct HksParam), &params[i],
            sizeof(struct HksParam));
    }
    return HKS_SUCCESS;
}

HKS_API_EXPORT int32_t HksBuildParamSet(struct HksParamSet **paramSet)
{
    if ((paramSet == NULL) || (*paramSet == NULL)) {
        return HKS_ERROR_NULL_POINTER;
    }

    int ret = HksCheckParamSet(*paramSet, (*paramSet)->paramSetSize);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "invalid build params!")

    return BuildParamSet(paramSet);
}

HKS_API_EXPORT void HksFreeParamSet(struct HksParamSet **paramSet)
{
    if (paramSet == NULL) {
        HKS_LOG_E("invalid free paramset!");
        return;
    }
    HKS_FREE_PTR(*paramSet);
}

static int32_t FreshParamSet(struct HksParamSet *paramSet, bool isCopy)
{
    uint32_t size = paramSet->paramSetSize;
    uint32_t offset = sizeof(struct HksParamSet) + sizeof(struct HksParam) * paramSet->paramsCnt;

    for (uint32_t i = 0; i < paramSet->paramsCnt; i++) {
        if (offset > size) {
            HKS_LOG_E("invalid param set offset!");
            return HKS_ERROR_INVALID_ARGUMENT;
        }
        if (GetTagType((enum HksTag)(paramSet->params[i].tag)) == HKS_TAG_TYPE_BYTES) {
            if (IsAdditionOverflow(offset, paramSet->params[i].blob.size)) {
                HKS_LOG_E("blob size overflow!");
                return HKS_ERROR_INVALID_ARGUMENT;
            }
            if (isCopy && memcpy_s((uint8_t *)paramSet + offset, size - offset,
                paramSet->params[i].blob.data, paramSet->params[i].blob.size) != EOK) {
                HKS_LOG_E("copy param blob failed!");
                return HKS_ERROR_INSUFFICIENT_MEMORY;
            }
            paramSet->params[i].blob.data = (uint8_t *)paramSet + offset;
            offset += paramSet->params[i].blob.size;
        }
    }

    if (paramSet->paramSetSize != offset) {
        HKS_LOG_E("invalid param set size!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    return HKS_SUCCESS;
}

HKS_API_EXPORT int32_t HksGetParam(const struct HksParamSet *paramSet, uint32_t tag, struct HksParam **param)
{
    if ((paramSet == NULL) || (param == NULL)) {
        HKS_LOG_E("invalid params!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    HKS_IF_NOT_SUCC_LOGE_RETURN(HksCheckParamSet(paramSet, paramSet->paramSetSize),
        HKS_ERROR_INVALID_ARGUMENT, "invalid paramSet!")

    for (uint32_t i = 0; i < paramSet->paramsCnt; i++) {
        if (tag == paramSet->params[i].tag) {
            *param = (struct HksParam *)&paramSet->params[i];
            return HKS_SUCCESS;
        }
    }

    return HKS_ERROR_PARAM_NOT_EXIST;
}

HKS_API_EXPORT int32_t HksGetParamSet(const struct HksParamSet *inParamSet,
    uint32_t inParamSetSize, struct HksParamSet **outParamSet)
{
    int32_t ret = HksCheckParamSet(inParamSet, inParamSetSize);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    HKS_IF_NULL_RETURN(outParamSet, HKS_ERROR_NULL_POINTER)

    uint32_t size = inParamSet->paramSetSize;
    struct HksParamSet *buf = (struct HksParamSet *)HksMalloc(size);
    HKS_IF_NULL_LOGE_RETURN(buf, HKS_ERROR_MALLOC_FAIL, "malloc from param set failed!")

    (void)memcpy_s(buf, size, inParamSet, size);

    ret = FreshParamSet(buf, false);
    if (ret != HKS_SUCCESS) {
        HKS_FREE_PTR(buf);
        return ret;
    }
    *outParamSet = buf;
    return HKS_SUCCESS;
}

HKS_API_EXPORT int32_t HksCheckParamMatch(const struct HksParam *baseParam, const struct HksParam *param)
{
    if (baseParam == NULL || param == NULL) {
        return HKS_ERROR_NULL_POINTER;
    }

    if (baseParam->tag != param->tag) {
        HKS_LOG_E("unmatch param type!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    switch (GetTagType((enum HksTag)(baseParam->tag))) {
        case HKS_TAG_TYPE_INT:
            return (baseParam->int32Param == param->int32Param) ? HKS_SUCCESS : HKS_ERROR_INVALID_ARGUMENT;
        case HKS_TAG_TYPE_UINT:
            return (baseParam->uint32Param == param->uint32Param) ? HKS_SUCCESS : HKS_ERROR_INVALID_ARGUMENT;
        case HKS_TAG_TYPE_ULONG:
            return (baseParam->uint64Param == param->uint64Param) ? HKS_SUCCESS : HKS_ERROR_INVALID_ARGUMENT;
        case HKS_TAG_TYPE_BOOL:
            return (baseParam->boolParam == param->boolParam) ? HKS_SUCCESS : HKS_ERROR_INVALID_ARGUMENT;
        case HKS_TAG_TYPE_BYTES:
            if (baseParam->blob.size != param->blob.size ||
                baseParam->blob.data == NULL ||(param->blob.data == NULL)) {
                HKS_LOG_E("unmatch byte type len!");
                return HKS_ERROR_INVALID_ARGUMENT;
            }
            if (HksMemCmp(baseParam->blob.data, param->blob.data, baseParam->blob.size)) {
                HKS_LOG_E("unmatch byte type content!");
                return HKS_ERROR_INVALID_ARGUMENT;
            }
            return HKS_SUCCESS;
        default:
            HKS_LOG_E("invalid tag type:%" LOG_PUBLIC "x", GetTagType((enum HksTag)(baseParam->tag)));
            return HKS_ERROR_INVALID_ARGUMENT;
    }
}

HKS_API_EXPORT int32_t HksCheckIsTagAlreadyExist(const struct HksParam *params, uint32_t paramsCnt,
    const struct HksParamSet *targetParamSet)
{
    if (params == NULL || targetParamSet == NULL) {
        return HKS_ERROR_NULL_POINTER;
    }

    int32_t ret = HksCheckParamSet(targetParamSet, targetParamSet->paramSetSize);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)
    
    for (uint32_t i = 0; i < targetParamSet->paramsCnt; ++i) {
        for (uint32_t j = 0; j < paramsCnt; ++j) {
            if (params[j].tag == targetParamSet->params[i].tag) {
                return HKS_ERROR_INVALID_ARGUMENT;
            }
        }
    }

    return HKS_SUCCESS;
}
