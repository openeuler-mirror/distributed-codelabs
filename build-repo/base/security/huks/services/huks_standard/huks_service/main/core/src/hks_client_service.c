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

#ifdef HKS_CONFIG_FILE
#include HKS_CONFIG_FILE
#else
#include "hks_config.h"
#endif

#include "hks_client_service.h"

#include <stdbool.h>
#include <stddef.h>

#include "hks_client_check.h"
#include "hks_hitrace.h"
#include "hks_log.h"
#include "hks_mem.h"
#include "hks_param.h"
#include "hks_report.h"
#include "hks_session_manager.h"
#include "hks_storage.h"
#include "hks_template.h"
#include "huks_access.h"
#include "securec.h"

#ifdef _STORAGE_LITE_
#include "hks_storage_adapter.h"
#endif

#ifdef HKS_SUPPORT_UPGRADE_STORAGE_DATA
#include "hks_upgrade_storage_data.h"
#endif

#ifdef HKS_SUPPORT_USER_AUTH_ACCESS_CONTROL

#include "hks_useridm_api_wrap.h"

#endif

#define USER_ID_ROOT_DEFAULT          "0"

#define MAX_STORAGE_SIZE (2 * 1024 * 1024)

#ifndef _CUT_AUTHENTICATE_
#ifdef _STORAGE_LITE_
static int32_t GetKeyData(const struct HksProcessInfo *processInfo, const struct HksBlob *keyAlias,
    struct HksBlob *key, int32_t mode)
{
    int32_t ret = HksStoreGetKeyBlob(processInfo, keyAlias, mode, key);
    HKS_IF_NOT_SUCC_LOGE(ret, "get key blob from storage failed, ret = %" LOG_PUBLIC "d", ret)
    return ret;
}

static int32_t CheckKeyCondition(const struct HksProcessInfo *processInfo, const struct HksBlob *keyAlias)
{
    /* check is enough buffer to store */
    uint32_t size = 0;
    int32_t ret = HksStoreGetToatalSize(&size);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get total size from storage failed, ret = %" LOG_PUBLIC "d", ret)

    if (size >= MAX_STORAGE_SIZE) {
        /* is key exist */
        ret = HksStoreIsKeyBlobExist(processInfo, keyAlias, HKS_STORAGE_TYPE_KEY);
        HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_STORAGE_FAILURE, "buffer exceeds limit")
    }

    return HKS_SUCCESS;
}

static int32_t GetKeyParamSet(const struct HksBlob *key, struct HksParamSet *paramSet)
{
    struct HksParamSet *tmpParamSet = NULL;
    int32_t ret = TranslateKeyInfoBlobToParamSet(NULL, key, &tmpParamSet);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    if (paramSet->paramSetSize < tmpParamSet->paramSetSize) {
        HksFreeParamSet(&tmpParamSet);
        return HKS_ERROR_BUFFER_TOO_SMALL;
    }
    if (memcpy_s(paramSet, paramSet->paramSetSize, tmpParamSet, tmpParamSet->paramSetSize) != EOK) {
        HKS_LOG_E("memcpy paramSet failed");
        ret = HKS_ERROR_INSUFFICIENT_MEMORY;
    }

    HksFreeParamSet(&tmpParamSet);
    return ret;
}

int32_t HksServiceGetKeyInfoList(const struct HksProcessInfo *processInfo, struct HksKeyInfo *keyInfoList,
    uint32_t *listCount)
{
    int32_t ret = HksCheckGetKeyInfoListParams(&(processInfo->processName), keyInfoList, listCount);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    ret = HksStoreGetKeyInfoList(keyInfoList, listCount);

    HksReport(__func__, processInfo, NULL, ret);
    return ret;
}
#else

static int32_t GetKeyData(const struct HksProcessInfo *processInfo, const struct HksBlob *keyAlias,
    struct HksBlob *key, int32_t mode)
{
    uint32_t size;
    int32_t ret = HksStoreGetKeyBlobSize(processInfo, keyAlias, mode, &size);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get keyblob size from storage failed, ret = %" LOG_PUBLIC "d.", ret)

    if (size > MAX_STORAGE_SIZE) {
        HKS_LOG_E("invalid storage size, size = %" LOG_PUBLIC "u", size);
        return HKS_ERROR_INVALID_KEY_FILE;
    }

    key->data = (uint8_t *)HksMalloc(size);
    HKS_IF_NULL_LOGE_RETURN(key->data, HKS_ERROR_MALLOC_FAIL, "get key data: malloc failed")

    key->size = size;
    ret = HksStoreGetKeyBlob(processInfo, keyAlias, mode, key);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("get keyblob from storage failed, ret = %" LOG_PUBLIC "d", ret);
        HKS_FREE_BLOB(*key);
    }

    return ret;
}

static int32_t CheckKeyCondition(const struct HksProcessInfo *processInfo, const struct HksBlob *keyAlias)
{
    /* delete old key and certchain before obtaining the number of keys */
    int32_t ret = HksStoreDeleteKeyBlob(processInfo, keyAlias, HKS_STORAGE_TYPE_KEY);
    if ((ret != HKS_SUCCESS) && (ret != HKS_ERROR_NOT_EXIST)) {
        HKS_LOG_E("delete keyblob from storage failed, ret = %" LOG_PUBLIC "d", ret);
        return ret;
    }

    ret = HksStoreDeleteKeyBlob(processInfo, keyAlias, HKS_STORAGE_TYPE_CERTCHAIN);
    if ((ret != HKS_SUCCESS) && (ret != HKS_ERROR_NOT_EXIST)) {
        HKS_LOG_E("delete certchain from storage failed, ret = %" LOG_PUBLIC "d", ret);
        return ret;
    }

    uint32_t fileCount;
    ret = HksGetKeyCountByProcessName(processInfo, &fileCount);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    return ret;
}

static int32_t DeleteTagFromParamSet(const uint32_t *tag, uint32_t tagCount, const struct HksParamSet *paramSet,
    struct HksParamSet **outParamSet)
{
    int32_t ret = HksFreshParamSet((struct HksParamSet *)paramSet, false);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "fresh paramset failed")

    struct HksParamSet *newParamSet = NULL;
    ret = HksInitParamSet(&newParamSet);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "init param set failed")

    for (uint32_t i = 0; i < paramSet->paramsCnt; ++i) {
        bool isDeleteTag = false;
        for (uint32_t j = 0; j < tagCount; ++j) {
            if (paramSet->params[i].tag == tag[j]) {
                isDeleteTag = true;
                break;
            }
        }
        if (!isDeleteTag) {
            ret = HksAddParams(newParamSet, &paramSet->params[i], 1);
            if (ret != HKS_SUCCESS) {
                HKS_LOG_E("add in params failed");
                HKS_FREE_PTR(newParamSet);
                return ret;
            }
        }
    }

    ret = HksBuildParamSet(&newParamSet);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("build paramset failed");
        HksFreeParamSet(&newParamSet);
        return ret;
    }

    *outParamSet = newParamSet;
    return HKS_SUCCESS;
}

static const uint32_t g_deleteTag[] = { HKS_TAG_KEY, HKS_TAG_ACCESS_TOKEN_ID };

static int32_t GetKeyParamSet(const struct HksBlob *key, struct HksParamSet *paramSet)
{
    if (key->size < sizeof(struct HksParamSet)) {
        HKS_LOG_E("get key paramset: invalid key size: %" LOG_PUBLIC "u", key->size);
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    const struct HksParamSet *tmpParamSet = (const struct HksParamSet *)key->data;
    struct HksParamSet *outParamSet = NULL;
    int32_t ret = DeleteTagFromParamSet(g_deleteTag, sizeof(g_deleteTag) / sizeof(g_deleteTag[0]), tmpParamSet,
        &outParamSet);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "delete tag from paramSet failed, ret = %" LOG_PUBLIC "d.", ret)

    if (paramSet->paramSetSize < outParamSet->paramSetSize) {
        HksFreeParamSet(&outParamSet);
        return HKS_ERROR_BUFFER_TOO_SMALL;
    }
    (void)memcpy_s(paramSet, paramSet->paramSetSize, outParamSet, outParamSet->paramSetSize);

    HksFreeParamSet(&outParamSet);
    return ret;
}

int32_t HksServiceGetKeyInfoList(const struct HksProcessInfo *processInfo, struct HksKeyInfo *keyInfoList,
    uint32_t *listCount)
{
    int32_t ret;

    do {
        ret = HksCheckGetKeyInfoListParams(&processInfo->processName, keyInfoList, listCount);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "get key data failed, ret = %" LOG_PUBLIC "d", ret)

        ret = HksGetKeyAliasByProcessName(processInfo, keyInfoList, listCount);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "get key alias list from storage failed, ret = %" LOG_PUBLIC "d", ret)

        for (uint32_t i = 0; i < *listCount; ++i) {
            struct HksBlob keyFromFile = { 0, NULL };
            ret = GetKeyData(processInfo, &(keyInfoList[i].alias), &keyFromFile, HKS_STORAGE_TYPE_KEY);
            HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "get key data failed, ret = %" LOG_PUBLIC "d", ret)

            ret = GetKeyParamSet(&keyFromFile, keyInfoList[i].paramSet);
            HKS_FREE_BLOB(keyFromFile);
            HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "get key paramSet failed, ret = %" LOG_PUBLIC "d", ret)
        }
    } while (0);

    HksReport(__func__, processInfo, NULL, ret);
    return ret;
}
#endif
#endif /* _CUT_AUTHENTICATE_ */

static int32_t AppendToNewParamSet(const struct HksParamSet *paramSet, struct HksParamSet **outParamSet)
{
    int32_t ret;
    struct HksParamSet *newParamSet = NULL;

    do {
        ret = HksCheckParamSet(paramSet, paramSet->paramSetSize);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "check paramSet failed")

        ret = HksFreshParamSet((struct HksParamSet *)paramSet, false);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "append fresh paramset failed")

        ret = HksInitParamSet(&newParamSet);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "append init operation param set failed")

        ret = HksAddParams(newParamSet, paramSet->params, paramSet->paramsCnt);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "append params failed")

        *outParamSet = newParamSet;
        return ret;
    } while (0);

    HksFreeParamSet(&newParamSet);
    return ret;
}

static bool CheckProcessNameTagExist(const struct HksParamSet *paramSet)
{
    for (uint32_t i = 0; i < paramSet->paramsCnt; ++i) {
        if (paramSet->params[i].tag == HKS_TAG_PROCESS_NAME) {
            return true;
        }
    }

    return false;
}

static int32_t AppendProcessInfo(const struct HksParamSet *paramSet, const struct HksBlob *processName,
    uint64_t accessTokenId, struct HksParamSet **outParamSet)
{
    int32_t ret;
    struct HksParamSet *newParamSet = NULL;

    do {
        if (paramSet != NULL) {
            ret = AppendToNewParamSet(paramSet, &newParamSet);
        } else {
            ret = HksInitParamSet(&newParamSet);
        }

        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "append client service tag failed")

        // process name only can be inserted by service
        if (CheckProcessNameTagExist(newParamSet)) {
            ret = HKS_ERROR_INVALID_ARGUMENT;
            break;
        }

        struct HksParam tmpParam;
        tmpParam.tag = HKS_TAG_PROCESS_NAME;
        tmpParam.blob = *processName;

        ret = HksAddParams(newParamSet, &tmpParam, 1);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "add param failed")

#ifdef HKS_SUPPORT_ACCESS_TOKEN
        struct HksParam accessTokenIdParam;
        accessTokenIdParam.tag = HKS_TAG_ACCESS_TOKEN_ID;
        accessTokenIdParam.uint64Param = accessTokenId;
        ret = HksAddParams(newParamSet, &accessTokenIdParam, 1);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "add access token id failed")
#else
        (void)accessTokenId;
#endif

        ret = HksBuildParamSet(&newParamSet);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "build paramset failed")

        *outParamSet = newParamSet;
        return ret;
    } while (0);

    HksFreeParamSet(&newParamSet);
    return ret;
}

#ifndef _CUT_AUTHENTICATE_
#ifdef HKS_SUPPORT_USER_AUTH_ACCESS_CONTROL
static int32_t AppendSecUid(struct HksParamSet *newParamSet, struct SecInfoWrap *secInfo)
{
    struct HksParam secureUid;
    secureUid.tag = HKS_TAG_USER_AUTH_SECURE_UID;
    secureUid.blob.size = sizeof(uint64_t);
    secureUid.blob.data = (uint8_t *)&secInfo->secureUid;
    return HksAddParams(newParamSet, &secureUid, 1);
}

static int32_t ConstructEnrolledInfoBlob(struct SecInfoWrap *secInfo, struct HksBlob *enrolledInfo,
    struct HksParam *outParam)
{
    (void)memset_s(enrolledInfo->data, enrolledInfo->size, 0, enrolledInfo->size);

    uint32_t index = 0;
    if (memcpy_s(enrolledInfo->data, enrolledInfo->size, &secInfo->enrolledInfoLen,
        sizeof(uint32_t)) != EOK) {
        HKS_LOG_E("copy enrolledInfoLen failed!");
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    index += sizeof(secInfo->enrolledInfoLen);
    for (uint32_t i = 0; i < secInfo->enrolledInfoLen; ++i) {
        uint32_t authTypeInt = (uint32_t)secInfo->enrolledInfo[i].authType;
        if (memcpy_s(enrolledInfo->data + index, enrolledInfo->size - index, &authTypeInt,
            sizeof(uint32_t)) != EOK) {
            HKS_LOG_E("copy authType failed!");
            return HKS_ERROR_INSUFFICIENT_MEMORY;
        }
        index += sizeof(authTypeInt);
        if (memcpy_s(enrolledInfo->data + index, enrolledInfo->size - index,
            &((secInfo->enrolledInfo[i]).enrolledId), sizeof(uint64_t)) != EOK) {
            HKS_LOG_E("copy enrolledId failed!");
            return HKS_ERROR_INSUFFICIENT_MEMORY;
        }
        index += sizeof(secInfo->enrolledInfo[i].enrolledId);
    }

    outParam->tag = HKS_TAG_USER_AUTH_ENROLL_ID_INFO;
    outParam->blob = *enrolledInfo;
    return HKS_SUCCESS;
}

static uint32_t ComputeEnrolledInfoLen(uint32_t enrolledInfoLen)
{
    return sizeof(uint32_t) + (sizeof(uint32_t) + sizeof(uint64_t)) * enrolledInfoLen;
}

static int32_t AddEnrolledInfoInParamSet(struct SecInfoWrap *secInfo, struct HksBlob *enrolledInfo,
    struct HksParamSet *paramSet)
{
    int32_t ret;
    do {
        struct HksParam tmpParam;
        enrolledInfo->size = ComputeEnrolledInfoLen(secInfo->enrolledInfoLen);
        enrolledInfo->data = (uint8_t *)HksMalloc(enrolledInfo->size);
        if (enrolledInfo->data == NULL) {
            HKS_LOG_E("malloc enrolledInfo blob failed");
            ret = HKS_ERROR_MALLOC_FAIL;
            break;
        }

        ret = ConstructEnrolledInfoBlob(secInfo, enrolledInfo, &tmpParam);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "ConstructEnrolledInfoBlob failed!")

        ret = HksAddParams(paramSet, &tmpParam, 1);
        return ret;
    } while (0);
    HKS_FREE_PTR(enrolledInfo->data);
    return ret;
}

static int32_t AppendUserAuthInfo(const struct HksParamSet *paramSet, int32_t userId, uint32_t authAccessType,
    struct HksParamSet **outParamSet)
{
    struct SecInfoWrap *secInfo = NULL;
    struct HksParamSet *newParamSet = NULL;
    int32_t ret;

    struct HksBlob enrolledInfo = { 0, NULL };
    do {
        ret = HksUserIdmGetSecInfo(userId, &secInfo);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("get useriam sec info failed ret=%" LOG_PUBLIC "d", ret);
            ret = HKS_ERROR_GET_USERIAM_SECINFO_FAILED;
            break;
        }
        
        if (paramSet != NULL) {
            ret = AppendToNewParamSet(paramSet, &newParamSet);
        } else {
            ret = HksInitParamSet(&newParamSet);
        }
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "init param set failed")

        ret = AppendSecUid(newParamSet, secInfo);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "append sec uid failed")

        ret = AddEnrolledInfoInParamSet(secInfo, &enrolledInfo, newParamSet);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "AddEnrolledInfoInParamSet failed!")

        ret = HksBuildParamSet(&newParamSet);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "build append info failed")

        *outParamSet = newParamSet;
    } while (0);
    HKS_FREE_PTR(enrolledInfo.data);
    HKS_FREE_PTR(secInfo);
    if (ret != HKS_SUCCESS) {
        HksFreeParamSet(&newParamSet);
        *outParamSet = NULL;
    }
    return ret;
}

static int32_t CheckIfEnrollAuthInfo(int32_t userId, enum HksUserAuthType authType)
{
    uint32_t numOfAuthInfo = 0;
    int32_t ret = HksUserIdmGetAuthInfoNum(userId, authType, &numOfAuthInfo);
    if (ret == HKS_ERROR_CREDENTIAL_NOT_EXIST) {
        HKS_LOG_E("have not enroll the auth info.");
        return HKS_ERROR_CREDENTIAL_NOT_EXIST;
    }

    return ret;
}

static int32_t CheckIfUserIamSupportCurType(int32_t userId, uint32_t userAuthType)
{
    const enum HksUserAuthType userAuthTypes[] = {
        HKS_USER_AUTH_TYPE_FINGERPRINT,
        HKS_USER_AUTH_TYPE_FACE,
        HKS_USER_AUTH_TYPE_PIN
    };
    int32_t ret;
    for (uint32_t i = 0; i < HKS_ARRAY_SIZE(userAuthTypes); ++i) {
        if ((userAuthType & userAuthTypes[i]) == 0) {
            continue;
        }
        ret = CheckIfEnrollAuthInfo(userId, userAuthTypes[i]);
        HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret,
            "no enrolled info of the user auth type: %" LOG_PUBLIC "d.", userAuthTypes[i])
    }
    return HKS_SUCCESS;
}

static int32_t AppendNewInfoForGenKeyInService(const struct HksProcessInfo *processInfo,
    const struct HksParamSet *paramSet, struct HksParamSet **outParamSet)
{
    uint32_t userAuthType = 0;
    uint32_t authAccessType = 0;
    int32_t ret = HksCheckAndGetUserAuthInfo(paramSet, &userAuthType, &authAccessType);
    if (ret == HKS_ERROR_NOT_SUPPORTED) {
        struct HksParamSet *newParamSet = NULL;
        ret = AppendProcessInfo(paramSet, &processInfo->processName, processInfo->accessTokenId, &newParamSet);
        HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "append tag processName failed, ret = %" LOG_PUBLIC "d", ret)

        *outParamSet = newParamSet;
        return HKS_SUCCESS;
    }

    if (ret == HKS_SUCCESS) {
        HKS_LOG_I("support secure access");
        ret = CheckIfUserIamSupportCurType(processInfo->userIdInt, userAuthType);
        HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "UserIAM do not support current user auth or not enrolled cur auth info")

        struct HksParamSet *userAuthParamSet = NULL;
        ret = AppendUserAuthInfo(paramSet, processInfo->userIdInt, authAccessType, &userAuthParamSet);
        HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "append secure access info failed!")

        struct HksParamSet *newInfoParamSet = NULL;
        ret = AppendProcessInfo(userAuthParamSet, &processInfo->processName, processInfo->accessTokenId,
            &newInfoParamSet);
        if (ret != HKS_SUCCESS) {
            HksFreeParamSet(&userAuthParamSet);
            HKS_LOG_E("append tag processName failed, ret = %" LOG_PUBLIC "d", ret);
            return ret;
        }
        HksFreeParamSet(&userAuthParamSet);
        *outParamSet = newInfoParamSet;
        return HKS_SUCCESS;
    }
    return ret;
}
#else
static int32_t AppendNewInfoForGenKeyInService(const struct HksProcessInfo *processInfo,
    const struct HksParamSet *paramSet, struct HksParamSet **outParamSet)
{
    return AppendProcessInfo(paramSet, &processInfo->processName, processInfo->accessTokenId, outParamSet);
}
#endif

static int32_t GetKeyAndNewParamSet(const struct HksProcessInfo *processInfo, const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSet, struct HksBlob *key, struct HksParamSet **outParamSet)
{
    int32_t ret = AppendProcessInfo(paramSet, &processInfo->processName, processInfo->accessTokenId, outParamSet);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "append tag processName failed, ret = %" LOG_PUBLIC "d", ret)

    ret = GetKeyData(processInfo, keyAlias, key, HKS_STORAGE_TYPE_KEY);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("get key data failed, ret = %" LOG_PUBLIC "d.", ret);
        HksFreeParamSet(outParamSet);
    }

    return ret;
}

#ifdef HKS_SUPPORT_ED25519_TO_X25519
static int32_t GetAgreeStoreKey(uint32_t keyAliasTag, const struct HksProcessInfo *processInfo,
    const struct HksParamSet *paramSet, struct HksBlob *key)
{
    struct HksParam *keyAliasParam = NULL;
    int32_t ret = HksGetParam(paramSet, keyAliasTag, &keyAliasParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get agree key alias tag failed")

    if (keyAliasParam->blob.size > HKS_MAX_KEY_ALIAS_LEN) {
        HKS_LOG_E("invalid main key size: %" LOG_PUBLIC "u", keyAliasParam->blob.size);
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    return GetKeyData(processInfo, &(keyAliasParam->blob), key, HKS_STORAGE_TYPE_KEY);
}

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

static int32_t GetAgreePublicKey(const uint32_t alg, const struct HksProcessInfo *processInfo,
    const struct HksParamSet *paramSet, struct HksBlob *key)
{
    struct HksParam *isKeyAliasParam = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_AGREE_PUBLIC_KEY_IS_KEY_ALIAS, &isKeyAliasParam);
    if ((ret == HKS_SUCCESS) && (!(isKeyAliasParam->boolParam))) {
        struct HksParam *keyParam = NULL;
        ret = HksGetParam(paramSet, HKS_TAG_AGREE_PUBLIC_KEY, &keyParam);
        HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get agree public key tag fail")

        return TranslateToInnerCurve25519Format(alg, &(keyParam->blob), key);
    }

    return GetAgreeStoreKey(HKS_TAG_AGREE_PUBLIC_KEY, processInfo, paramSet, key);
}

static int32_t GetAgreePrivateKey(const struct HksProcessInfo *processInfo,
    const struct HksParamSet *paramSet, struct HksBlob *key)
{
    return GetAgreeStoreKey(HKS_TAG_AGREE_PRIVATE_KEY_ALIAS, processInfo, paramSet, key);
}

static int32_t ConbineIntoKeyPair(const struct HksBlob *privateKey,
    const struct HksBlob *publicKey, struct HksBlob *keyPair)
{
    uint32_t size = sizeof(struct Hks25519KeyPair) + privateKey->size + publicKey->size; /* size has been checked */
    uint8_t *buffer = (uint8_t *)HksMalloc(size);
    HKS_IF_NULL_RETURN(buffer, HKS_ERROR_MALLOC_FAIL)

    (void)memset_s(buffer, size, 0, size);

    struct Hks25519KeyPair keyPairStruct = { publicKey->size, privateKey->size };
    uint32_t offset = 0;

    (void)memcpy_s(buffer + offset, size, &keyPairStruct, sizeof(keyPairStruct));
    offset += sizeof(keyPairStruct);

    (void)memcpy_s(buffer  + offset, size - offset, publicKey->data, publicKey->size);
    offset += publicKey->size;

    (void)memcpy_s(buffer  + offset, size - offset, privateKey->data, privateKey->size) ;

    keyPair->data = buffer;
    keyPair->size = size;
    return HKS_SUCCESS;
}

static int32_t GetAgreeKeyPair(const uint32_t alg, const struct HksProcessInfo *processInfo,
    const struct HksParamSet *paramSet, struct HksBlob *key)
{
    int32_t ret;
    struct HksBlob privateKey = { 0, NULL };
    struct HksBlob publicKey = { 0, NULL };
    do {
        ret = GetAgreePublicKey(alg, processInfo, paramSet, &publicKey);
        HKS_IF_NOT_SUCC_BREAK(ret)

        ret = GetAgreePrivateKey(processInfo, paramSet, &privateKey);
        HKS_IF_NOT_SUCC_BREAK(ret)

        ret = ConbineIntoKeyPair(&privateKey, &publicKey, key);
    } while (0);

    HKS_FREE_BLOB(privateKey);
    HKS_FREE_BLOB(publicKey);
    return ret;
}
#endif

static int32_t GetAgreeBaseKey(const struct HksProcessInfo *processInfo, const struct HksParamSet *paramSet,
    struct HksBlob *key)
{
    (void)key;
    struct HksParam *keyAlgParam = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_ALGORITHM, &keyAlgParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_CHECK_GET_ALG_FAIL, "get alg tag fail")

    if (keyAlgParam->uint32Param != HKS_ALG_AES) {
        HKS_LOG_I("not an aes key, no need check main key and derive factor");
        return HKS_SUCCESS;
    }

#ifdef HKS_SUPPORT_ED25519_TO_X25519
    struct HksParam *agreeAlgParam = NULL;
    ret = HksGetParam(paramSet, HKS_TAG_AGREE_ALG, &agreeAlgParam);
    HKS_IF_NOT_SUCC_RETURN(ret, HKS_ERROR_CHECK_GET_ALG_FAIL)

    if ((agreeAlgParam->uint32Param != HKS_ALG_X25519) && (agreeAlgParam->uint32Param != HKS_ALG_ED25519)) {
        return HKS_ERROR_INVALID_ALGORITHM;
    }

    return GetAgreeKeyPair(agreeAlgParam->uint32Param, processInfo, paramSet, key);
#else
    (void)processInfo;
    return HKS_ERROR_INVALID_ARGUMENT; /* if aes generated by agree but x25519/ed25519 is ot support, return error */
#endif
}

static int32_t GetDeriveMainKey(const struct HksProcessInfo *processInfo, const struct HksParamSet *paramSet,
    struct HksBlob *key)
{
    struct HksParam *keyGenTypeParam = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_KEY_GENERATE_TYPE, &keyGenTypeParam);
    /* not set tag KEY_GENERATE_TYPE, gen key by default type */
    HKS_IF_NOT_SUCC_RETURN(ret, HKS_SUCCESS)

    if (keyGenTypeParam->uint32Param == HKS_KEY_GENERATE_TYPE_AGREE) {
        return GetAgreeBaseKey(processInfo, paramSet, key);
    } else if (keyGenTypeParam->uint32Param == HKS_KEY_GENERATE_TYPE_DEFAULT) {
        return HKS_SUCCESS;
    }
    return HKS_ERROR_INVALID_ARGUMENT;
}

static int32_t GetKeyIn(const struct HksProcessInfo *processInfo, const struct HksParamSet *paramSet,
    struct HksBlob *key)
{
    int32_t ret = GetDeriveMainKey(processInfo, paramSet, key);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    /* if not generate by derive, init keyIn by default value(ca to ta not accept null pointer) */
    if (key->data == NULL) {
        key->size = 1; /* malloc least buffer as keyIn buffer */
        key->data = (uint8_t *)HksMalloc(key->size);
        HKS_IF_NULL_LOGE_RETURN(key->data, HKS_ERROR_MALLOC_FAIL, "malloc failed")

        key->data[0] = 0;
    }
    return HKS_SUCCESS;
}

static int32_t StoreOrCopyKeyBlob(const struct HksParamSet *paramSet, const struct HksProcessInfo *processInfo,
    struct HksBlob *output, struct HksBlob *outData, bool isStorage)
{
    if (!isStorage) {
        if ((outData->size != 0) && (memcpy_s(outData->data, outData->size, output->data, output->size) != EOK)) {
            HKS_LOG_E("copy keyblob data fail");
            return HKS_ERROR_INSUFFICIENT_MEMORY;
        }
        outData->size = output->size;
        return HKS_SUCCESS;
    }

    struct HksParam *keyAliasParam = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_KEY_ALIAS, &keyAliasParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get key alias fail, ret = %" LOG_PUBLIC "d", ret)

    if (keyAliasParam->blob.size > HKS_MAX_KEY_ALIAS_LEN) {
        HKS_LOG_E("key alias size is too long, size is %" LOG_PUBLIC "u", keyAliasParam->blob.size);
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    ret = CheckKeyCondition(processInfo, &keyAliasParam->blob);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "CheckKeyCondition fail, ret = %" LOG_PUBLIC "d", ret)

    ret = HksStoreKeyBlob(processInfo, &keyAliasParam->blob, HKS_STORAGE_TYPE_KEY, output);
    HKS_IF_NOT_SUCC_LOGE(ret, "HksStoreKeyBlob fail, ret = %" LOG_PUBLIC "d", ret)

    return ret;
}

int32_t HksServiceGenerateKey(const struct HksProcessInfo *processInfo, const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSetIn, struct HksBlob *keyOut)
{
    struct HksParamSet *newParamSet = NULL;
    uint8_t *keyOutBuffer = (uint8_t *)HksMalloc(MAX_KEY_SIZE);
    HKS_IF_NULL_RETURN(keyOutBuffer, HKS_ERROR_MALLOC_FAIL)

    struct HksHitraceId traceId = HksHitraceBegin(__func__, HKS_HITRACE_FLAG_DEFAULT);

    struct HksBlob output = { MAX_KEY_SIZE, keyOutBuffer };
    struct HksBlob keyIn = { 0, NULL };
    int32_t ret;
    do {
        /* if user don't pass the key out buffer, we will use a tmp key out buffer */
        if ((keyOut != NULL) && (keyOut->data != NULL) && (keyOut->size != 0)) {
            output = *keyOut;
        }

        ret = HksCheckGenAndImportKeyParams(&processInfo->processName, keyAlias, paramSetIn, &output);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "check generate key params failed, ret = %" LOG_PUBLIC "d", ret)

        ret = CheckKeyCondition(processInfo, keyAlias);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "check key condition failed, ret = %" LOG_PUBLIC "d", ret)

        ret = AppendNewInfoForGenKeyInService(processInfo, paramSetIn, &newParamSet);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "append processName tag failed, ret = %" LOG_PUBLIC "d", ret)

        ret = GetKeyIn(processInfo, newParamSet, &keyIn);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "get keyIn failed, ret = %" LOG_PUBLIC "d", ret)

        ret = HuksAccessGenerateKey(keyAlias, newParamSet, &keyIn, &output);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "access level generate key failed, ret = %" LOG_PUBLIC "d", ret)

        ret = HksStoreKeyBlob(processInfo, keyAlias, HKS_STORAGE_TYPE_KEY, &output);
        HKS_IF_NOT_SUCC_LOGE(ret, "store keyblob to storage failed, ret = %" LOG_PUBLIC "d", ret)
    } while (0);

    HKS_FREE_PTR(keyOutBuffer);
    HKS_FREE_PTR(keyIn.data);
    HksFreeParamSet(&newParamSet);
    HksHitraceEnd(&traceId);

    HksReport(__func__, processInfo, paramSetIn, ret);

    return ret;
}

int32_t HksServiceSign(const struct HksProcessInfo *processInfo, const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSet, const struct HksBlob *srcData, struct HksBlob *signature)
{
    int32_t ret;
    struct HksParamSet *newParamSet = NULL;
    struct HksBlob keyFromFile = { 0, NULL };

    struct HksHitraceId traceId = HksHitraceBegin(__func__, HKS_HITRACE_FLAG_DEFAULT);

    do {
        ret = HksCheckAllParams(&processInfo->processName, keyAlias, paramSet, srcData, signature);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "check sign params failed, ret = %" LOG_PUBLIC "d", ret)

        ret = GetKeyAndNewParamSet(processInfo, keyAlias, paramSet, &keyFromFile, &newParamSet);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "sign: get key and new paramSet failed, ret = %" LOG_PUBLIC "d", ret)

        ret = HuksAccessSign(&keyFromFile, newParamSet, srcData, signature);
    } while (0);

    HKS_FREE_BLOB(keyFromFile);
    HksFreeParamSet(&newParamSet);

    HksHitraceEnd(&traceId);

    HksReport(__func__, processInfo, paramSet, ret);

    return ret;
}

int32_t HksServiceVerify(const struct HksProcessInfo *processInfo, const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSet, const struct HksBlob *srcData, const struct HksBlob *signature)
{
    int32_t ret;
    struct HksParamSet *newParamSet = NULL;
    struct HksBlob keyFromFile = { 0, NULL };
    struct HksHitraceId traceId = HksHitraceBegin(__func__, HKS_HITRACE_FLAG_DEFAULT);

    do {
        ret = HksCheckAllParams(&processInfo->processName, keyAlias, paramSet, srcData, signature);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "check verify params failed, ret = %" LOG_PUBLIC "d", ret)

        ret = GetKeyAndNewParamSet(processInfo, keyAlias, paramSet, &keyFromFile, &newParamSet);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "verify: get key and new paramSet failed, ret = %" LOG_PUBLIC "d", ret)

        ret = HuksAccessVerify(&keyFromFile, newParamSet, srcData, signature);
    } while (0);

    HKS_FREE_BLOB(keyFromFile);
    HksFreeParamSet(&newParamSet);

    HksHitraceEnd(&traceId);

    HksReport(__func__, processInfo, paramSet, ret);

    return ret;
}

int32_t HksServiceEncrypt(const struct HksProcessInfo *processInfo, const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSet, const struct HksBlob *plainText, struct HksBlob *cipherText)
{
    int32_t ret;
    struct HksParamSet *newParamSet = NULL;
    struct HksBlob keyFromFile = { 0, NULL };

    do {
        ret = HksCheckAllParams(&processInfo->processName, keyAlias, paramSet, plainText, cipherText);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "check encrypt failed, ret = %" LOG_PUBLIC "d", ret)

        ret = GetKeyAndNewParamSet(processInfo, keyAlias, paramSet, &keyFromFile, &newParamSet);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "encrypt: get key and new paramSet failed, ret = %" LOG_PUBLIC "d", ret)

        ret = HuksAccessEncrypt(&keyFromFile, newParamSet, plainText, cipherText);
    } while (0);

    HKS_FREE_BLOB(keyFromFile);
    HksFreeParamSet(&newParamSet);

    HksReport(__func__, processInfo, paramSet, ret);

    return ret;
}

int32_t HksServiceDecrypt(const struct HksProcessInfo *processInfo, const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSet, const struct HksBlob *cipherText, struct HksBlob *plainText)
{
    int32_t ret;
    struct HksParamSet *newParamSet = NULL;
    struct HksBlob keyFromFile = { 0, NULL };
    struct HksHitraceId traceId = HksHitraceBegin(__func__, HKS_HITRACE_FLAG_DEFAULT);

    do {
        ret = HksCheckAllParams(&processInfo->processName, keyAlias, paramSet, cipherText, plainText);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "check decrypt failed, ret = %" LOG_PUBLIC "d", ret)

        ret = GetKeyAndNewParamSet(processInfo, keyAlias, paramSet, &keyFromFile, &newParamSet);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "decrypt: get key and new paramSet failed, ret = %" LOG_PUBLIC "d", ret)

        ret = HuksAccessDecrypt(&keyFromFile, newParamSet, cipherText, plainText);
    } while (0);

    HKS_FREE_BLOB(keyFromFile);
    HksFreeParamSet(&newParamSet);
    HksHitraceEnd(&traceId);

    HksReport(__func__, processInfo, paramSet, ret);

    return ret;
}

int32_t HksServiceDeleteKey(const struct HksProcessInfo *processInfo, const struct HksBlob *keyAlias)
{
    int32_t ret = HksCheckProcessNameAndKeyAlias(&processInfo->processName, keyAlias);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    /*
     * Detele key first, record log if failed; then delete cert chain, return error if failed;
     * Return error code of deleteKey in the end.
     */
    ret = HksStoreDeleteKeyBlob(processInfo, keyAlias, HKS_STORAGE_TYPE_KEY);
    if ((ret != HKS_SUCCESS) && (ret != HKS_ERROR_NOT_EXIST)) {
        HKS_LOG_E("service delete main key failed, ret = %" LOG_PUBLIC "d", ret);
    }

    int32_t deleteCertRet = HksStoreDeleteKeyBlob(processInfo, keyAlias, HKS_STORAGE_TYPE_CERTCHAIN);
    if ((deleteCertRet != HKS_SUCCESS) && (deleteCertRet != HKS_ERROR_NOT_EXIST)) {
        HKS_LOG_E("service delete cert chain failed, ret = %" LOG_PUBLIC "d", ret);
        ret = deleteCertRet;
    }

    HksReport(__func__, processInfo, NULL, ret);

    return ret;
}

int32_t HksServiceKeyExist(const struct HksProcessInfo *processInfo, const struct HksBlob *keyAlias)
{
    int32_t ret = HksCheckProcessNameAndKeyAlias(&processInfo->processName, keyAlias);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    ret = HksStoreIsKeyBlobExist(processInfo, keyAlias, HKS_STORAGE_TYPE_KEY);

    HksReport(__func__, processInfo, NULL, ret);
    return ret;
}

int32_t HksServiceGetKeyParamSet(const struct HksProcessInfo *processInfo, const struct HksBlob *keyAlias,
    struct HksParamSet *paramSet)
{
    int32_t ret;
    struct HksParamSet *newParamSet = NULL;
    struct HksBlob keyFromFile = { 0, NULL };

    do {
        ret = HksCheckGetKeyParamSetParams(&processInfo->processName, keyAlias, paramSet);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "check get key paramSet params failed, ret = %" LOG_PUBLIC "d", ret)

        ret = GetKeyAndNewParamSet(processInfo, keyAlias, NULL, &keyFromFile, &newParamSet);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret,
            "get key paramSet: get key and new paramSet failed, ret = %" LOG_PUBLIC "d", ret)

        ret = HuksAccessGetKeyProperties(newParamSet, &keyFromFile);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "access level check key validity failed, ret = %" LOG_PUBLIC "d", ret)

        ret = GetKeyParamSet(&keyFromFile, paramSet);
        HKS_IF_NOT_SUCC_LOGE(ret, "get Key paramSet failed, ret = %" LOG_PUBLIC "d", ret)
    } while (0);

    HKS_FREE_BLOB(keyFromFile);
    HksFreeParamSet(&newParamSet);

    HksReport(__func__, processInfo, paramSet, ret);

    return ret;
}

int32_t HksServiceImportKey(const struct HksProcessInfo *processInfo, const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSet, const struct HksBlob *key)
{
    int32_t ret;
    struct HksParamSet *newParamSet = NULL;

    do {
        ret = HksCheckGenAndImportKeyParams(&processInfo->processName, keyAlias, paramSet, key);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "check import key params failed, ret = %" LOG_PUBLIC "d", ret)

        ret = CheckKeyCondition(processInfo, keyAlias);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "import: check key condition failed, ret = %" LOG_PUBLIC "d", ret)

        ret = AppendNewInfoForGenKeyInService(processInfo, paramSet, &newParamSet);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "append processName tag failed, ret = %" LOG_PUBLIC "d", ret)

        uint8_t *keyOutBuffer = (uint8_t *)HksMalloc(MAX_KEY_SIZE);
        if (keyOutBuffer == NULL) {
            ret = HKS_ERROR_MALLOC_FAIL;
            break;
        }

        struct HksBlob keyOut = { MAX_KEY_SIZE, keyOutBuffer };
        ret = HuksAccessImportKey(keyAlias, key, newParamSet, &keyOut);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("access level import public key failed, ret = %" LOG_PUBLIC "d", ret);
            HKS_FREE_PTR(keyOutBuffer);
            break;
        }

        ret = HksStoreKeyBlob(processInfo, keyAlias, HKS_STORAGE_TYPE_KEY, &keyOut);
        HKS_FREE_PTR(keyOutBuffer);
    } while (0);

    HksFreeParamSet(&newParamSet);
    HksReport(__func__, processInfo, paramSet, ret);
    return ret;
}

static int32_t GetKeyAndNewParamSetInForGenKeyInService(const struct HksProcessInfo *processInfo,
    const struct HksBlob *keyAlias, const struct HksParamSet *paramSet, struct HksBlob *key,
    struct HksParamSet **outParamSet)
{
    int32_t ret = AppendNewInfoForGenKeyInService(processInfo, paramSet, outParamSet);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "append new info failed, ret = %" LOG_PUBLIC "d", ret)

    ret = GetKeyData(processInfo, keyAlias, key, HKS_STORAGE_TYPE_KEY);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("get key data failed, ret = %" LOG_PUBLIC "d.", ret);
        HksFreeParamSet(outParamSet);
    }

    return ret;
}

int32_t HksServiceImportWrappedKey(const struct HksProcessInfo *processInfo, const struct HksBlob *keyAlias,
    const struct HksBlob *wrappingKeyAlias, const struct HksParamSet *paramSet, const struct HksBlob *wrappedKeyData)
{
    int32_t ret;
    struct HksParamSet *newParamSet = NULL;
    struct HksBlob wrappingKeyFromFile = { 0, NULL };
    struct HksHitraceId traceId = HksHitraceBegin(__func__, HKS_HITRACE_FLAG_DEFAULT);

    do {
        ret = HksCheckImportWrappedKeyParams(&processInfo->processName, keyAlias,
            wrappingKeyAlias, paramSet, wrappedKeyData);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "check import params failed, ret = %" LOG_PUBLIC "d", ret)

        ret = HksServiceKeyExist(processInfo, wrappingKeyAlias);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "wrapping key is not exist, ret = %" LOG_PUBLIC "d", ret)

        ret = CheckKeyCondition(processInfo, keyAlias);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "check condition failed, ret = %" LOG_PUBLIC "d", ret)

        ret = GetKeyAndNewParamSetInForGenKeyInService(processInfo, wrappingKeyAlias, paramSet, &wrappingKeyFromFile,
            &newParamSet);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "get wrapping key and new paramSet failed, ret = %" LOG_PUBLIC "d", ret)

        uint8_t *keyOutBuffer = (uint8_t *)HksMalloc(MAX_KEY_SIZE);
        if (keyOutBuffer == NULL) {
            ret = HKS_ERROR_MALLOC_FAIL;
            break;
        }
        struct HksBlob keyOut = { MAX_KEY_SIZE, keyOutBuffer };
        ret = HuksAccessImportWrappedKey(wrappingKeyAlias, &wrappingKeyFromFile, wrappedKeyData, newParamSet, &keyOut);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("access level import wrapped key failed, ret = %" LOG_PUBLIC "d", ret);
            HKS_FREE_PTR(keyOutBuffer);
            break;
        }

        ret = HksStoreKeyBlob(processInfo, keyAlias, HKS_STORAGE_TYPE_KEY, &keyOut);
        HKS_FREE_PTR(keyOutBuffer);
    } while (0);

    HKS_FREE_BLOB(wrappingKeyFromFile);
    HksFreeParamSet(&newParamSet);
    HksHitraceEnd(&traceId);

    HksReport(__func__, processInfo, paramSet, ret);

    return ret;
}

int32_t HksServiceExportPublicKey(const struct HksProcessInfo *processInfo, const struct HksBlob *keyAlias,
    struct HksBlob *key)
{
    int32_t ret;
    struct HksParamSet *newParamSet = NULL;
    struct HksBlob keyFromFile = { 0, NULL };

    do {
        ret = HksCheckExportPublicKeyParams(&processInfo->processName, keyAlias, key);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "check export public key params failed, ret = %" LOG_PUBLIC "d", ret)

        ret = GetKeyAndNewParamSet(processInfo, keyAlias, NULL, &keyFromFile, &newParamSet);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "export public: get key and new paramSet failed, ret = %" LOG_PUBLIC "d", ret)

        ret = HuksAccessExportPublicKey(&keyFromFile, newParamSet, key);
    } while (0);

    HKS_FREE_BLOB(keyFromFile);
    HksFreeParamSet(&newParamSet);

    HksReport(__func__, processInfo, NULL, ret);
    return ret;
}

int32_t HksServiceAgreeKey(const struct HksProcessInfo *processInfo, const struct HksParamSet *paramSet,
    const struct HksBlob *privateKey, const struct HksBlob *peerPublicKey, struct HksBlob *agreedKey)
{
    int32_t ret;
    struct HksParamSet *newParamSet = NULL;
    struct HksBlob keyFromFile = { 0, NULL };
    struct HksHitraceId traceId = HksHitraceBegin(__func__, HKS_HITRACE_FLAG_DEFAULT);

    do {
        ret = HksCheckAllParams(&processInfo->processName, privateKey, paramSet, peerPublicKey, agreedKey);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "check agree key params failed, ret = %" LOG_PUBLIC "d", ret)

        ret = GetKeyAndNewParamSet(processInfo, privateKey, paramSet, &keyFromFile, &newParamSet);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "agree: get key and new paramSet failed, ret = %" LOG_PUBLIC "d", ret)

        ret = HuksAccessAgreeKey(newParamSet, &keyFromFile, peerPublicKey, agreedKey);
    } while (0);

    HKS_FREE_BLOB(keyFromFile);
    HksFreeParamSet(&newParamSet);
    HksHitraceEnd(&traceId);

    HksReport(__func__, processInfo, paramSet, ret);

    return ret;
}

int32_t HksServiceDeriveKey(const struct HksProcessInfo *processInfo, const struct HksParamSet *paramSet,
    const struct HksBlob *mainKey, struct HksBlob *derivedKey)
{
    int32_t ret;
    struct HksParamSet *newParamSet = NULL;
    struct HksBlob keyFromFile = { 0, NULL };
    struct HksHitraceId traceId = HksHitraceBegin(__func__, HKS_HITRACE_FLAG_DEFAULT);

    do {
        ret = HksCheckDeriveKeyParams(&processInfo->processName, paramSet, mainKey, derivedKey);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "check derive key params failed, ret = %" LOG_PUBLIC "d", ret)

        ret = GetKeyAndNewParamSet(processInfo, mainKey, paramSet, &keyFromFile, &newParamSet);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "derive: get key and new paramSet failed, ret = %" LOG_PUBLIC "d", ret)

        ret = HuksAccessDeriveKey(newParamSet, &keyFromFile, derivedKey);
    } while (0);

    HKS_FREE_BLOB(keyFromFile);
    HksFreeParamSet(&newParamSet);
    HksHitraceEnd(&traceId);

    HksReport(__func__, processInfo, paramSet, ret);

    return ret;
}

int32_t HksServiceMac(const struct HksProcessInfo *processInfo, const struct HksBlob *key,
    const struct HksParamSet *paramSet, const struct HksBlob *srcData, struct HksBlob *mac)
{
    int32_t ret;
    struct HksParamSet *newParamSet = NULL;
    struct HksBlob keyFromFile = { 0, NULL };
    struct HksHitraceId traceId = HksHitraceBegin(__func__, HKS_HITRACE_FLAG_DEFAULT);

    do {
        ret = HksCheckAllParams(&processInfo->processName, key, paramSet, srcData, mac);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "check mac params failed, ret = %" LOG_PUBLIC "d", ret)

        ret = GetKeyAndNewParamSet(processInfo, key, paramSet, &keyFromFile, &newParamSet);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "mac: get key and new paramSet failed, ret = %" LOG_PUBLIC "d", ret)

        ret = HuksAccessMac(&keyFromFile, newParamSet, srcData, mac);
    } while (0);

    HKS_FREE_BLOB(keyFromFile);
    HksFreeParamSet(&newParamSet);
    HksHitraceEnd(&traceId);

    HksReport(__func__, processInfo, paramSet, ret);

    return ret;
}

int32_t HksServiceInitialize(void)
{
    int32_t ret;
    do {
#ifdef HKS_SUPPORT_UPGRADE_STORAGE_DATA
        ret = HksUpgradeStorageData();
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "hks update storage data failed, ret = %" LOG_PUBLIC "d", ret)
#endif
        ret = HuksAccessModuleInit();
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "hks core service initialize failed! ret = %" LOG_PUBLIC "d", ret)

#ifdef _STORAGE_LITE_
        ret = HksLoadFileToBuffer();
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "load file to buffer failed, ret = %" LOG_PUBLIC "d", ret)
#endif
    } while (0);

    HksReport(__func__, NULL, NULL, ret);

    return ret;
}

int32_t HksServiceRefreshKeyInfo(const struct HksBlob *processName)
{
    int32_t ret = HksStoreDestroy(processName);
    HKS_LOG_I("destroy storage files ret = 0x%" LOG_PUBLIC "X", ret); /* only recode log */

#ifdef HKS_SUPPORT_UPGRADE_STORAGE_DATA
    ret = HksDestroyOldVersionFiles();
    HKS_LOG_I("destroy old version files ret = 0x%" LOG_PUBLIC "X", ret); /* only recode log */
#endif
    do {
#ifndef _HARDWARE_ROOT_KEY_
        ret = HuksAccessRefresh();
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "Hks core service refresh info failed! ret = 0x%" LOG_PUBLIC "X", ret)
#endif

#ifdef _STORAGE_LITE_
        ret = HksFileBufferRefresh();
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "load file to buffer failed, ret = %" LOG_PUBLIC "d", ret)
#endif
    } while (0);

#ifdef L2_STANDARD
    int userId = 0;
    struct HksBlob userIdBlob = { sizeof(int), (uint8_t *)&userId };
    struct HksProcessInfo processInfo = {userIdBlob, *processName};
    HksReport(__func__, &processInfo, NULL, ret);
#endif
    
    return ret;
}

int32_t HksServiceAttestKey(const struct HksProcessInfo *processInfo, const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSet, struct HksBlob *certChain)
{
#ifdef HKS_SUPPORT_API_ATTEST_KEY
    struct HksHitraceId traceId = HksHitraceBegin(__func__, HKS_HITRACE_FLAG_DEFAULT);
    struct HksBlob keyFromFile = { 0, NULL };
    struct HksParamSet *newParamSet = NULL;
    int32_t ret;
    do {
        ret = HksCheckAttestKeyParams(&processInfo->processName, keyAlias, paramSet, certChain);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "check attest key param fail")

        ret = GetKeyAndNewParamSet(processInfo, keyAlias, paramSet, &keyFromFile, &newParamSet);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "GetKeyAndNewParamSet failed, ret = %" LOG_PUBLIC "d.", ret)

        ret = HuksAccessAttestKey(&keyFromFile, newParamSet, certChain);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HuksAccessAttestKey fail, ret = %" LOG_PUBLIC "d.", ret)

        ret = HksStoreKeyBlob(processInfo, keyAlias, HKS_STORAGE_TYPE_CERTCHAIN, certChain);
        HKS_IF_NOT_SUCC_LOGE(ret, "store attest cert chain failed")
    } while (0);

    HKS_FREE_BLOB(keyFromFile);
    HksFreeParamSet(&newParamSet);
    HksHitraceEnd(&traceId);

    HksReport(__func__, processInfo, paramSet, ret);

    return ret;
#else
    (void)processInfo;
    (void)keyAlias;
    (void)paramSet;
    (void)certChain;
    return HKS_ERROR_NOT_SUPPORTED;
#endif
}

int32_t HksServiceInit(const struct HksProcessInfo *processInfo, const struct HksBlob *key,
    const struct HksParamSet *paramSet, struct HksBlob *handle, struct HksBlob *token)
{
    int32_t ret;
    struct HksParamSet *newParamSet = NULL;
    struct HksBlob keyFromFile = { 0, NULL };
    struct HksHitraceId traceId = HksHitraceBegin(__func__, HKS_HITRACE_FLAG_DEFAULT);

    do {
        ret = HksCheckServiceInitParams(&processInfo->processName, key, paramSet);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "check ServiceInit params failed, ret = %" LOG_PUBLIC "d", ret)

        ret = GetKeyAndNewParamSet(processInfo, key, paramSet, &keyFromFile, &newParamSet);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "GetKeyAndNewParamSet failed, ret = %" LOG_PUBLIC "d", ret)

        ret = HuksAccessInit(&keyFromFile, newParamSet, handle, token);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HuksAccessInit failed, ret = %" LOG_PUBLIC "d", ret)

        ret = CreateOperation(processInfo, handle, true);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "create operation failed, ret = %" LOG_PUBLIC "d", ret)
    } while (0);

    HKS_FREE_BLOB(keyFromFile);
    HksFreeParamSet(&newParamSet);

    HksHitraceEnd(&traceId);

    HksReport(__func__, processInfo, paramSet, ret);

    return ret;
}

int32_t HksServiceUpdate(const struct HksBlob *handle, const struct HksProcessInfo *processInfo,
    const struct HksParamSet *paramSet, const struct HksBlob *inData, struct HksBlob *outData)
{
    struct HksHitraceId traceId = HksHitraceBegin(__func__, HKS_HITRACE_FLAG_DEFAULT);
    int32_t ret;
    do {
        struct HksOperation *operation = QueryOperation(processInfo, handle);
        if (operation == NULL) {
            HKS_LOG_E("operationHandle is not exist");
            ret = HKS_ERROR_NOT_EXIST;
            break;
        }
        if (operation->accessTokenId != processInfo->accessTokenId) {
            HKS_LOG_E("compare access token id failed, unauthorized calling");
            ret = HKS_ERROR_BAD_STATE;
            break;
        }

        ret = HuksAccessUpdate(handle, paramSet, inData, outData);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HuksAccessUpdate fail, ret = %" LOG_PUBLIC "d", ret);
            DeleteOperation(handle);
            break;
        }
    } while (0);
    
    HksHitraceEnd(&traceId);

    HksReport(__func__, processInfo, paramSet, ret);

    return ret;
}

static int32_t AppendAndQueryInFinish(const struct HksBlob *handle, const struct HksProcessInfo *processInfo,
    const struct HksParamSet *paramSet, struct HksParamSet **newParamSet)
{
    int32_t ret;
    do {
        ret = AppendProcessInfo(paramSet, &processInfo->processName, processInfo->accessTokenId, newParamSet);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "append tag processName failed, ret = %" LOG_PUBLIC "d", ret)

        struct HksOperation *operation = QueryOperation(processInfo, handle);
        if (operation == NULL) {
            HKS_LOG_E("operationHandle is not exist");
            ret = HKS_ERROR_NOT_EXIST;
            break;
        }
        if (operation->accessTokenId != processInfo->accessTokenId) {
            HKS_LOG_E("compare access token id failed, unauthorized calling");
            ret = HKS_ERROR_BAD_STATE;
            break;
        }
    } while (0);
    return ret;
}

static int32_t InitOutputDataForFinish(struct HksBlob *output, const struct HksBlob *outData, bool isStorage)
{
    output->data = (uint8_t *)HksMalloc(output->size);
    HKS_IF_NULL_RETURN(output->data, HKS_ERROR_MALLOC_FAIL)

    (void)memset_s(output->data, output->size, 0, output->size);
    if (!isStorage) {
        if ((memcpy_s(output->data, output->size, outData->data, outData->size) != EOK)) {
            HKS_FREE_PTR(output->data);
            return HKS_ERROR_INSUFFICIENT_MEMORY;
        }
    }
    return HKS_SUCCESS;
}

int32_t HksServiceFinish(const struct HksBlob *handle, const struct HksProcessInfo *processInfo,
    const struct HksParamSet *paramSet, const struct HksBlob *inData, struct HksBlob *outData)
{
    struct HksHitraceId traceId = HksHitraceBegin(__func__, HKS_HITRACE_FLAG_DEFAULT);
    struct HksParamSet *newParamSet = NULL;
    bool isStorage = false;
    uint32_t outSize = outData->size;
    struct HksParam *storageFlag = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_KEY_STORAGE_FLAG, &storageFlag);
    if ((ret == HKS_SUCCESS) && (storageFlag->uint32Param == HKS_STORAGE_PERSISTENT)) {
        isStorage = true;
        outSize = MAX_KEY_SIZE;
    }
    struct HksBlob output = { outSize, NULL };
    do {
        if (outSize != 0) {
            ret = InitOutputDataForFinish(&output, outData, isStorage);
            HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "init output data failed")
        }
        ret = AppendAndQueryInFinish(handle, processInfo, paramSet, &newParamSet);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "AppendAndQueryInFinish fail, ret = %" LOG_PUBLIC "d", ret)

        ret = HuksAccessFinish(handle, newParamSet, inData, &output);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HuksAccessFinish fail, ret = %" LOG_PUBLIC "d", ret)

        ret = StoreOrCopyKeyBlob(paramSet, processInfo, &output, outData, isStorage);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "StoreOrCopyKeyBlob fail, ret = %" LOG_PUBLIC "d", ret)
    } while (0);
    if (output.data != NULL) {
        (void)memset_s(output.data, output.size, 0, output.size);
    }
    HKS_FREE_BLOB(output);
    DeleteOperation(handle);
    HksFreeParamSet(&newParamSet);
    HksHitraceEnd(&traceId);
    HksReport(__func__, processInfo, paramSet, ret);
    return ret;
}

int32_t HksServiceAbort(const struct HksBlob *handle, const struct HksProcessInfo *processInfo,
    const struct HksParamSet *paramSet)
{
    struct HksHitraceId traceId = HksHitraceBegin(__func__, HKS_HITRACE_FLAG_DEFAULT);
    int32_t ret;
    do {
        if (QueryOperation(processInfo, handle) == NULL) {
            HKS_LOG_E("operationHandle is not exist");
            ret = HKS_SUCCESS; /* return success if the handle is not found */
            break;
        }

        ret = HuksAccessAbort(handle, paramSet);
        HKS_IF_NOT_SUCC_LOGE(ret, "HuksAccessAbort fail, ret = %" LOG_PUBLIC "d", ret)

        DeleteOperation(handle);
    } while (0);

    HksHitraceEnd(&traceId);
    
    HksReport(__func__, processInfo, paramSet, ret);

    return ret;
}

void HksServiceDeleteProcessInfo(const struct HksProcessInfo *processInfo)
{
#ifndef __LITEOS_M__
    HKS_LOG_I("remove session");
    DeleteSessionByProcessInfo(processInfo);

    if (processInfo->processName.size == 0) {
        HksServiceDeleteUserIDKeyAliasFile(processInfo->userId);
    } else {
        HksServiceDeleteUIDKeyAliasFile(*processInfo);
    }
#else
    (void)processInfo;
#endif
}

#endif

int32_t HksServiceGenerateRandom(const struct HksBlob *processName, struct HksBlob *random)
{
    int32_t ret;
    struct HksParamSet *newParamSet = NULL;

    do {
        ret = HksCheckGenerateRandomParams(processName, random);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "check generate random params failed, ret = %" LOG_PUBLIC "d", ret)

        ret = AppendProcessInfo(NULL, processName, 0, &newParamSet);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "append processName tag failed, ret = %" LOG_PUBLIC "d", ret)

        ret = HuksAccessGenerateRandom(newParamSet, random);
    } while (0);

    HksFreeParamSet(&newParamSet);
    
#ifdef L2_STANDARD
    int userId = 0;
    struct HksBlob userIdBlob = { sizeof(int), (uint8_t *)&userId };
    struct HksProcessInfo processInfo = {userIdBlob, *processName};
    
    HksReport(__func__, &processInfo, NULL, ret);
#endif

    return ret;
}
