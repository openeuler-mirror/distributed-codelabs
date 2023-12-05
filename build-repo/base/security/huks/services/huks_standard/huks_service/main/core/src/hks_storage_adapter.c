/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
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

#include "hks_storage_adapter.h"

#include "hks_log.h"
#include "hks_param.h"
#include "hks_template.h"

bool HksIsKeyInfoLenInvalid(struct HksStoreKeyInfo *keyInfo)
{
    return (keyInfo->aliasSize > HKS_MAX_KEY_ALIAS_LEN) || (keyInfo->aliasSize == 0) ||
        (keyInfo->keySize > HKS_MAX_KEY_MATERIAL_LEN) || (keyInfo->keySize == 0) ||
        (keyInfo->authIdSize > HKS_MAX_KEY_AUTH_ID_LEN) ||
        (keyInfo->keyInfoLen != (sizeof(*keyInfo) + keyInfo->aliasSize + keyInfo->authIdSize + keyInfo->keySize));
}

static int32_t AddStorageFixedParams(const struct HksStoreKeyInfo *keyInfo, struct HksParamSet *paramSet)
{
    struct HksParam params[] = {
        {
            .tag = HKS_TAG_KEY_FLAG,
            .uint32Param = keyInfo->flag
        }, {
            .tag = HKS_TAG_ALGORITHM,
            .uint32Param = keyInfo->keyAlg
        }, {
            .tag = HKS_TAG_PURPOSE,
            .uint32Param = keyInfo->purpose
        }, {
            .tag = HKS_TAG_KEY_SIZE,
            .uint32Param = keyInfo->keyLen
        }, {
            .tag = HKS_TAG_DIGEST,
            .uint32Param = keyInfo->digest
        }, {
            .tag = HKS_TAG_PADDING,
            .uint32Param = keyInfo->padding
        }, {
            .tag = HKS_TAG_BLOCK_MODE,
            .uint32Param = keyInfo->keyMode
        }, {
            .tag = HKS_TAG_KEY_ROLE,
            .uint32Param = keyInfo->role
        }, {
            .tag = HKS_TAG_KEY_DOMAIN,
            .uint32Param = keyInfo->domain
        },
    };

    int32_t ret = HksAddParams(paramSet, params, sizeof(params) / sizeof(params[0]));
    HKS_IF_NOT_SUCC_LOGE(ret, "HksAddParams failed!")

    return ret;
}

static int32_t AddStorageParams(const struct HksBlob *key, const struct HksBlob *keyInfoBlob,
    struct HksParamSet *paramSet)
{
    if (keyInfoBlob->size < sizeof(struct HksStoreKeyInfo)) {
        HKS_LOG_E("invalid keyInfoBlob, size too small, size = %" LOG_PUBLIC "u", keyInfoBlob->size);
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    struct HksStoreKeyInfo *keyInfo = (struct HksStoreKeyInfo *)keyInfoBlob->data;
    if (HksIsKeyInfoLenInvalid(keyInfo)) {
        HKS_LOG_E("invalid keyInfoBlob len");
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    int32_t ret = AddStorageFixedParams(keyInfo, paramSet);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "add storage fixed params failed!")

    if (keyInfo->authIdSize != 0) {
        struct HksBlob keyAuthId = { keyInfo->authIdSize, keyInfoBlob->data + sizeof(*keyInfo) + keyInfo->aliasSize };
        struct HksParam keyAuthIdParam = {
            .tag = HKS_TAG_KEY_AUTH_ID,
            .blob = keyAuthId
        };
        ret = HksAddParams(paramSet, &keyAuthIdParam, 1);
        HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "HksAddParams keyAuthId failed!")
    }

    if (key != NULL) {
        struct HksParam keyParam = {
            .tag = HKS_TAG_KEY,
            .blob = *key
        };
        ret = HksAddParams(paramSet, &keyParam, 1);
        HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "HksAddParams key failed!")
    }
    return ret;
}

int32_t TranslateKeyInfoBlobToParamSet(const struct HksBlob *key, const struct HksBlob *keyInfoBlob,
    struct HksParamSet **paramSet)
{
    struct HksParamSet *outputParamSet = NULL;
    int32_t ret = HksInitParamSet(&outputParamSet);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "HksInitParamSet failed!")

    do {
        ret = AddStorageParams(key, keyInfoBlob, outputParamSet);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "AddParams failed!")

        ret = HksBuildParamSet(&outputParamSet);
        HKS_IF_NOT_SUCC_LOGE(ret, "HksBuildParamSet failed!")
    } while (0);

    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("ConstructParamSet failed.");
        HksFreeParamSet(&outputParamSet);
        return ret;
    }

    *paramSet = outputParamSet;
    return ret;
}

#endif /* _STORAGE_LITE_ */
#endif /* _CUT_AUTHENTICATE_ */
