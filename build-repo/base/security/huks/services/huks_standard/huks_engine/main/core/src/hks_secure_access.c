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

#include "hks_secure_access.h"

#include "hks_base_check.h"
#include "hks_keyblob.h"
#include "hks_keynode.h"
#include "hks_log.h"
#include "hks_mem.h"
#include "hks_param.h"
#include "hks_type_inner.h"
#include "hks_template.h"

#include "securec.h"

#ifdef HKS_SUPPORT_USER_AUTH_ACCESS_CONTROL

#include "hks_crypto_hal.h"
#include "hks_core_hal_api.h"
#include "hks_useridm_api_wrap.h"

#define BYTES_PER_POS 8
#define S_TO_MS 1000
#define DEFAULT_TIME_OUT 3
#define AUTH_INFO_LEN (sizeof(uint32_t) + sizeof(uint64_t) + sizeof(uint64_t))

struct HksSecureAccessInnerParams {
    const struct HksParamSet *initParamSet;
    uint32_t challengePos;
    bool isUserAuthAccess;
    bool isSecureSign;
    struct HksBlob *outToken;
};

struct HksAppendDataInnerParams {
    struct HuksKeyNode *keyNode;
    const struct HksParamSet *inParamSet;
    const struct HksBlob *inData;
};

static int32_t CheckChallengeTypeValidity(const struct HksParam *blobChallengeType,
    struct HksSecureAccessInnerParams *innerParams)
{
    if (blobChallengeType->uint32Param == HKS_CHALLENGE_TYPE_CUSTOM) {
        struct HksParam *challengePosParam = NULL;
        int32_t ret = HksGetParam(innerParams->initParamSet, HKS_TAG_CHALLENGE_POS, &challengePosParam);
        HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get init paramSet's challenge pos failed!")

        if (challengePosParam->uint32Param > HKS_CHALLENGE_POS_3) {
            HKS_LOG_E("challenge position should in range of 0~3!");
            return HKS_ERROR_INVALID_ARGUMENT;
        }
        innerParams->challengePos = challengePosParam->uint32Param;
    } else {
        innerParams->challengePos = 0;
    }

    if (blobChallengeType->uint32Param == HKS_CHALLENGE_TYPE_NONE) {
        // must set zero for ClientInit judgement
        innerParams->outToken->size = 0;
        return HKS_SUCCESS;
    }

    if (innerParams->outToken->size < TOKEN_SIZE) {
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    return HKS_SUCCESS;
}

static int32_t CheckInitParamSetValidityAndGet(const struct HksParamSet *keyBlobParamSet,
    struct HksSecureAccessInnerParams *innerParams)
{
    struct HksParam *blobUserAuthType = NULL;
    int32_t ret = HksGetParam(keyBlobParamSet, HKS_TAG_USER_AUTH_TYPE, &blobUserAuthType);
    if (ret == HKS_ERROR_PARAM_NOT_EXIST) {
        innerParams->isUserAuthAccess = false;
        innerParams->isSecureSign = false;
        // must set zero for ClientInit judgement
        innerParams->outToken->size = 0;
        return HKS_SUCCESS;
    }

    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get blob user auth type failed!")

    struct HksParam *blobChallengeType = NULL;
    ret = HksGetParam(keyBlobParamSet, HKS_TAG_CHALLENGE_TYPE, &blobChallengeType);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get blob challenge type failed!")

    ret = CheckChallengeTypeValidity(blobChallengeType, innerParams);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "check init paramSet's challenge type related params failed!")

    struct HksParam *secureSignTag = NULL;
    ret = HksGetParam(keyBlobParamSet, HKS_TAG_KEY_SECURE_SIGN_TYPE, &secureSignTag);
    if (ret == HKS_SUCCESS) {
        ret = HksCheckSecureSignParams(secureSignTag->uint32Param);
        HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_BAD_STATE, "invalid key blob secure sign type!")

        innerParams->isSecureSign = true;
    } else {
        innerParams->isSecureSign = false;
    }

    innerParams->isUserAuthAccess = true;
    return HKS_SUCCESS;
}

static int32_t AddChallengeParams(struct HksParamSet *paramSet, struct HksBlob *challengeBlob)
{
    int32_t ret = HksCryptoHalFillRandom(challengeBlob);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_BAD_STATE, "generate challenge failed!")

    struct HksParam challengeParam;
    challengeParam.tag = HKS_TAG_KEY_INIT_CHALLENGE;
    challengeParam.blob = *challengeBlob;
    ret = HksAddParams(paramSet, &challengeParam, 1);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "add challenge params fail")

    return HKS_SUCCESS;
}

static int32_t AddKeyAccessTimeParams(struct HksParamSet *paramSet)
{
    uint64_t curTime = 0;
    int32_t ret = HksCoreHalElapsedRealTime(&curTime);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get elapsed real time failed!")

    struct HksParam accessTimeParam;
    accessTimeParam.tag = HKS_TAG_KEY_ACCESS_TIME;
    accessTimeParam.uint64Param = curTime / S_TO_MS;
    ret = HksAddParams(paramSet, &accessTimeParam, 1);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "add access time param fail")

    return HKS_SUCCESS;
}

static int32_t AssignToken(struct HksBlob *token, const struct HksBlob *challenge)
{
    if (token->size >= TOKEN_SIZE) {
        if (memcpy_s(token->data, token->size, challenge->data, challenge->size) != EOK) {
            HKS_LOG_E("copy token failed");
            return HKS_ERROR_INSUFFICIENT_MEMORY;
        }
        token->size = challenge->size;
        return HKS_SUCCESS;
    } else if (token->size == 0) {
        return HKS_SUCCESS;
    } else {
        HKS_LOG_E("token size is too small");
        return HKS_ERROR_INVALID_ARGUMENT;
    }
}

static int32_t AddAppendDataPlaceholder(struct HksParamSet *paramSet, uint8_t *appendDataPlaceholder,
    uint32_t placeholderSize)
{
    struct HksParam signAuthParam = {
        .tag = HKS_TAG_APPENDED_DATA_PREFIX,
        .blob = {
            .size = placeholderSize,
            .data = appendDataPlaceholder
        }
    };

    int32_t ret = HksAddParams(paramSet, &signAuthParam, 1);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "add sign auth info params fail")

    return HKS_SUCCESS;
}

static int32_t AddDefaultAuthRuntimeParams(struct HksParamSet *paramSet,
    struct HksSecureAccessInnerParams *innerParams, bool isNeedAppendAuthInfo)
{
    struct HksParam defineParams[] = {
        { .tag = HKS_TAG_IS_USER_AUTH_ACCESS, .boolParam = innerParams->isUserAuthAccess },
        { .tag = HKS_TAG_IF_NEED_APPEND_AUTH_INFO, .boolParam = isNeedAppendAuthInfo },
        { .tag = HKS_TAG_IS_APPEND_UPDATE_DATA, .boolParam = false },
        { .tag = HKS_TAG_KEY_AUTH_RESULT, .int32Param = HKS_AUTH_RESULT_INIT },
        { .tag = HKS_TAG_CHALLENGE_POS, .uint32Param = innerParams->challengePos }
    };

    int32_t ret = HksAddParams(paramSet, defineParams, sizeof(defineParams) / sizeof(defineParams[0]));
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "add runtime defineParams fail")

    return HKS_SUCCESS;
}

static int32_t BuildAuthRuntimeParamSet(struct HksSecureAccessInnerParams *innerParams, bool isNeedAppendAuthInfo,
    struct HksParamSet **outParamSet)
{
    struct HksParamSet *paramSet = NULL;
    int32_t ret = HksInitParamSet(&paramSet);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "init keyNode auth runtime param set fail")

    do {
        ret = AddDefaultAuthRuntimeParams(paramSet, innerParams, isNeedAppendAuthInfo);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "add auth runtime default params fail")

        ret = AddKeyAccessTimeParams(paramSet);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "add key access time params failed!")

        uint8_t challenge[TOKEN_SIZE] = {0};
        struct HksBlob challengeBlob = { TOKEN_SIZE, challenge };
        ret = AddChallengeParams(paramSet, &challengeBlob);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "add challenge params failed!")

        if (isNeedAppendAuthInfo) {
            uint8_t appendPlaceholder[sizeof(struct HksSecureSignAuthInfo)] = {0};
            ret = AddAppendDataPlaceholder(paramSet, appendPlaceholder, sizeof(appendPlaceholder));
            HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "add append data info params fail")
        }

        ret = HksBuildParamSet(&paramSet);
        HKS_IF_NOT_SUCC_BREAK(ret)

        ret = AssignToken(innerParams->outToken, &challengeBlob);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "assign out token failed")

        *outParamSet = paramSet;
        return HKS_SUCCESS;
    } while (0);

    HksFreeParamSet(&paramSet);
    return ret;
}

static int32_t HksVerifyKeyChallenge(const struct HuksKeyNode *keyNode, const struct HksUserAuthToken *token,
    uint32_t challengePos, uint32_t checkLen)
{
    struct HksParam *challenge = NULL;
    int32_t ret = HksGetParam(keyNode->authRuntimeParamSet, HKS_TAG_KEY_INIT_CHALLENGE, &challenge);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_BAD_STATE, "get init challenge failed!")

    if (checkLen + challengePos * BYTES_PER_POS > challenge->blob.size) {
        HKS_LOG_E("check challenge too long!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    if (HksMemCmp(challenge->blob.data + challengePos * BYTES_PER_POS, token->challenge + challengePos * BYTES_PER_POS,
        checkLen) != 0) {
        HKS_LOG_E("verify challenge failed!");
        return HKS_ERROR_KEY_AUTH_FAILED;
    }
    return HKS_SUCCESS;
}

static int32_t HksVerifyKeyTimestamp(const struct HuksKeyNode *keyNode, const struct HksUserAuthToken *token)
{
    uint32_t timeOutInt = DEFAULT_TIME_OUT;
    struct HksParam *timeOut = NULL;
    int32_t ret = HksGetParam(keyNode->keyBlobParamSet, HKS_TAG_AUTH_TIMEOUT, &timeOut);
    if (ret == HKS_SUCCESS) {
        timeOutInt = timeOut->uint32Param;
    }

    struct HksParam *accessTime = NULL;
    ret = HksGetParam(keyNode->authRuntimeParamSet, HKS_TAG_KEY_ACCESS_TIME, &accessTime);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_BAD_STATE, "get access time failed!")

    // ms to s
    uint64_t authTokenTime = token->time / S_TO_MS;
    if ((accessTime->uint64Param > authTokenTime && accessTime->uint64Param - authTokenTime > timeOutInt) ||
        (authTokenTime > accessTime->uint64Param && authTokenTime - accessTime->uint64Param > timeOutInt)) {
        HKS_LOG_E("auth token time out!");
        return HKS_ERROR_KEY_AUTH_TIME_OUT;
    }
    return HKS_SUCCESS;
}

static int32_t CheckAuthToken(const struct HksBlob *authTokenParam)
{
    if (authTokenParam->size != sizeof(struct HksUserAuthToken)) {
        HKS_LOG_E("size of authTokenParam not match HksUserAuthToken!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    return HKS_SUCCESS;
}

static int32_t ParseAuthToken(const struct HksBlob *inAuthTokenParam, struct HksUserAuthToken **outAuthToken)
{
    int32_t ret = CheckAuthToken(inAuthTokenParam);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    struct HksUserAuthToken *authToken = NULL;
    do {
        authToken = (struct HksUserAuthToken *)HksMalloc(sizeof(struct HksUserAuthToken));
        if (authToken == NULL) {
            HKS_LOG_E("malloc for authToken failed!");
            ret = HKS_ERROR_MALLOC_FAIL;
            break;
        }

        (void)memcpy_s(authToken, sizeof(struct HksUserAuthToken), inAuthTokenParam->data, inAuthTokenParam->size);
        *outAuthToken = authToken;
        return HKS_SUCCESS;
    } while (0);

    HKS_FREE_PTR(authToken);
    return ret;
}

static int32_t GetAuthToken(const struct HksParamSet *paramSet, struct HksUserAuthToken **authToken)
{
    struct HksParam *authTokenParam = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_AUTH_TOKEN, &authTokenParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_CHECK_GET_AUTH_TOKEN_FAILED, "get auth token param failed!")

    ret = ParseAuthToken(&authTokenParam->blob, authToken);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_INVALID_AUTH_TOKEN, "parse auth token failed!")

    return HKS_SUCCESS;
}

static int32_t GetChallengePos(const struct HksParamSet *paramSet, uint32_t *pos)
{
    struct HksParam *posParam = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_CHALLENGE_POS, &posParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_INVALID_ARGUMENT, "get challenge pos failed!")

    *pos = posParam->uint32Param;
    return ret;
}

static int32_t GetChallengeType(const struct HksParamSet *paramSet, uint32_t *type)
{
    struct HksParam *typeParam = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_CHALLENGE_TYPE, &typeParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_BAD_STATE, "get challenge type failed!")

    *type = typeParam->uint32Param;
    return HKS_SUCCESS;
}

static int32_t VerifyCustomChallenge(const struct HuksKeyNode *keyNode, const struct HksUserAuthToken *authToken)
{
    uint32_t pos = 0;
    int32_t ret = GetChallengePos(keyNode->authRuntimeParamSet, &pos);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_INVALID_ARGUMENT, "get challenge pos failed!")

    return HksVerifyKeyChallenge(keyNode, authToken, pos, BYTES_PER_POS);
}

static int32_t VerifyNormalChallenge(const struct HuksKeyNode *keyNode, const struct HksUserAuthToken *authToken)
{
    return HksVerifyKeyChallenge(keyNode, authToken, 0, TOKEN_SIZE);
}

static int32_t VerifyChallengeOrTimeStamp(const struct HuksKeyNode *keyNode, const struct HksUserAuthToken *authToken)
{
    uint32_t blobChallengeType;
    int32_t ret = GetChallengeType(keyNode->keyBlobParamSet, &blobChallengeType);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get challenge type failed!")

    switch (blobChallengeType) {
        case HKS_CHALLENGE_TYPE_NORMAL:
            ret = VerifyNormalChallenge(keyNode, authToken);
            break;
        case HKS_CHALLENGE_TYPE_CUSTOM:
            ret = VerifyCustomChallenge(keyNode, authToken);
            break;
        case HKS_CHALLENGE_TYPE_NONE:
            ret = HksVerifyKeyTimestamp(keyNode, authToken);
            break;
        default:
            ret = HKS_ERROR_BAD_STATE;
            break;
    }
    return ret;
}

static int32_t VerifySecureUidIfNeed(const struct HksParamSet *keyBlobParamSet,
    const struct HksUserAuthToken *authToken, uint32_t authAccessType)
{
    if ((authAccessType & HKS_AUTH_ACCESS_INVALID_CLEAR_PASSWORD) == 0) {
        return HKS_SUCCESS;
    }

    struct HksParam *secUid = NULL;
    int32_t ret = HksGetParam(keyBlobParamSet, HKS_TAG_USER_AUTH_SECURE_UID, &secUid);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_BAD_STATE, "get sec uid failed!")

    if (secUid->blob.size != sizeof(uint64_t)) {
        HKS_LOG_E("invalid sec uid param!");
        return HKS_ERROR_BAD_STATE;
    }

    if (HksMemCmp(secUid->blob.data, &authToken->secureUid, sizeof(uint64_t)) != 0) {
        HKS_LOG_E("check sec uid failed!");
        return HKS_ERROR_KEY_AUTH_PERMANENTLY_INVALIDATED;
    }
    return HKS_SUCCESS;
}

static int32_t VerifyEnrolledIdInfoIfNeed(const struct HksParamSet *keyBlobParamSet,
    const struct HksUserAuthToken *authToken, uint32_t blobAuthType, uint32_t authAccessType,
    uint32_t authTokenAuthType)
{
    if ((blobAuthType & (HKS_USER_AUTH_TYPE_FACE | HKS_USER_AUTH_TYPE_FINGERPRINT)) == 0 ||
        (authAccessType & HKS_AUTH_ACCESS_INVALID_NEW_BIO_ENROLL) == 0) {
        return HKS_SUCCESS;
    }

    struct HksParam *enrolledIdInfo = NULL;
    int32_t ret = HksGetParam(keyBlobParamSet, HKS_TAG_USER_AUTH_ENROLL_ID_INFO, &enrolledIdInfo);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_BAD_STATE, "get enrolled info param failed!")

    struct HksBlob enrolledIdInfoBlob = enrolledIdInfo->blob;
    if (enrolledIdInfoBlob.size < ENROLLED_ID_INFO_MIN_LEN) {
        HKS_LOG_E("get enrolled info param invalid!");
        return HKS_ERROR_BAD_STATE;
    }

    uint32_t enrolledIdNum = 0;
    (void)memcpy_s(&enrolledIdNum, sizeof(uint32_t), enrolledIdInfoBlob.data, sizeof(uint32_t));
    uint32_t index = sizeof(uint32_t);

    for (uint32_t i = 0; i < enrolledIdNum && index < enrolledIdInfoBlob.size; ++i) {
        uint32_t authType = 0;
        (void)memcpy_s(&authType, sizeof(uint32_t), enrolledIdInfoBlob.data + index, sizeof(uint32_t));
        index += sizeof(uint32_t);

        uint64_t enrolledId = 0;
        (void)memcpy_s(&enrolledId, sizeof(uint64_t), enrolledIdInfoBlob.data + index, sizeof(uint64_t));
        index += sizeof(uint64_t);
        if (authType == authTokenAuthType && enrolledId == authToken->enrolledId) {
            return HKS_SUCCESS;
        }
    }
    HKS_LOG_E("match enrolled id failed!");
    return HKS_ERROR_KEY_AUTH_PERMANENTLY_INVALIDATED;
}

static int32_t VerifyAuthTokenInfo(const struct HuksKeyNode *keyNode, const struct HksUserAuthToken *authToken)
{
    struct HksParamSet *keyBlobParamSet = keyNode->keyBlobParamSet;
    struct HksParam *userAuthType = NULL;
    int32_t ret = HksGetParam(keyBlobParamSet, HKS_TAG_USER_AUTH_TYPE, &userAuthType);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_BAD_STATE, "get userAuthType type failed!")

    struct HksParam *authAccessType = NULL;
    ret = HksGetParam(keyBlobParamSet, HKS_TAG_KEY_AUTH_ACCESS_TYPE, &authAccessType);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_BAD_STATE, "get auth access type failed!")

    uint32_t authTokenAuthType = 0;
    ret = HksConvertUserIamTypeToHksType(HKS_AUTH_TYPE, authToken->authType, &authTokenAuthType);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_NOT_SUPPORTED, "invalid user iam auth type:not support!")

    if ((authTokenAuthType & userAuthType->uint32Param) == 0) {
        HKS_LOG_E("current keyblob auth do not support current auth token auth type!");
        return HKS_ERROR_KEY_AUTH_VERIFY_FAILED;
    }

    ret = VerifySecureUidIfNeed(keyBlobParamSet, authToken, authAccessType->uint32Param);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "verify sec uid failed!")

    ret = VerifyEnrolledIdInfoIfNeed(keyBlobParamSet, authToken, userAuthType->uint32Param,
        authAccessType->uint32Param, authTokenAuthType);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "verify enrolled id info failed!")

    return ret;
}

static int32_t HksAddVerifiedAuthTokenIfNeed(struct HuksKeyNode *keyNode,
    const struct HksUserAuthToken *verifiedAuthToken)
{
    struct HksParam *isNeedSecureSignInfo = NULL;
    int32_t ret = HksGetParam(keyNode->authRuntimeParamSet, HKS_TAG_IF_NEED_APPEND_AUTH_INFO, &isNeedSecureSignInfo);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_BAD_STATE, "get is secure sign failed!")

    if (isNeedSecureSignInfo->boolParam == false) {
        return HKS_SUCCESS;
    }

    struct HksParamSet *newAuthRuntimeParamSet = NULL;
    ret = HksInitParamSet(&newAuthRuntimeParamSet);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "init new auth param set fail")

    struct HksParamSet *authRuntimeParamSet = keyNode->authRuntimeParamSet;
    if (authRuntimeParamSet != NULL) {
        ret = HksAddParams(newAuthRuntimeParamSet, authRuntimeParamSet->params, authRuntimeParamSet->paramsCnt);
        if (ret != HKS_SUCCESS) {
            HksFreeParamSet(&newAuthRuntimeParamSet);
            HKS_LOG_E("add old auth runtime param set fail");
            return ret;
        }
    }

    struct HksParam verifiedAuthTokenParam = {
        .tag = HKS_TAG_VERIFIED_AUTH_TOKEN,
        .blob = {
            .size = sizeof(struct HksUserAuthToken),
            .data = (uint8_t *)verifiedAuthToken
        }
    };

    ret = HksAddParams(newAuthRuntimeParamSet, &verifiedAuthTokenParam, 1);
    if (ret != HKS_SUCCESS) {
        HksFreeParamSet(&newAuthRuntimeParamSet);
        HKS_LOG_E("add verified authtoken to auth runtime param set fail");
        return ret;
    }

    ret = HksBuildParamSet(&newAuthRuntimeParamSet);
    if (ret != HKS_SUCCESS) {
        HksFreeParamSet(&newAuthRuntimeParamSet);
        HKS_LOG_E("build paramSet fail");
        return ret;
    }
    HksFreeParamSet(&authRuntimeParamSet);
    keyNode->authRuntimeParamSet = newAuthRuntimeParamSet;
    return HKS_SUCCESS;
}

static int32_t CheckIfNeedVerifyParams(const struct HuksKeyNode *keyNode, bool *isNeedVerify,
    struct HksParam **outAuthResult)
{
    struct HksParam *isNeedSecureAccess = NULL;
    int32_t ret = HksGetParam(keyNode->authRuntimeParamSet, HKS_TAG_IS_USER_AUTH_ACCESS, &isNeedSecureAccess);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_BAD_STATE, "get isSecureAccess failed!")

    struct HksParam *authResult = NULL;
    ret = HksGetParam(keyNode->authRuntimeParamSet, HKS_TAG_KEY_AUTH_RESULT, &authResult);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_BAD_STATE, "get authResult failed!")

    *outAuthResult = authResult;
    if (isNeedSecureAccess->boolParam == false) {
        *isNeedVerify = false;
        return HKS_SUCCESS;
    }

    if (authResult->uint32Param == HKS_AUTH_RESULT_SUCCESS) {
        *isNeedVerify = false;
        return HKS_SUCCESS;
    }

    *isNeedVerify = true;
    return HKS_SUCCESS;
}

static int32_t AssignVerifyResultAndFree(int32_t outRet, struct HksParam *authResult, struct HuksKeyNode *keyNode,
    struct HksUserAuthToken *authToken)
{
    int32_t ret = outRet;
    if (ret == HKS_SUCCESS) {
        authResult->uint32Param = HKS_AUTH_RESULT_SUCCESS;
        ret = HksAddVerifiedAuthTokenIfNeed(keyNode, authToken);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("add verified auth token failed!");
            HKS_FREE_PTR(authToken);
            return HKS_ERROR_BAD_STATE;
        }
    } else {
        authResult->uint32Param = HKS_AUTH_RESULT_FAILED;
        HKS_FREE_PTR(authToken);
    }
    HKS_FREE_PTR(authToken);
    return ret;
}

static int32_t GetUserAuthResult(const struct HuksKeyNode *keyNode, int32_t *authResult)
{
    struct HksParam *isSecureAccess = NULL;
    int32_t ret = HksGetParam(keyNode->authRuntimeParamSet, HKS_TAG_IS_USER_AUTH_ACCESS, &isSecureAccess);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_BAD_STATE, "get isSecureAccess failed!")

    if (isSecureAccess->boolParam == false) {
        *authResult = HKS_AUTH_RESULT_NONE;
        return HKS_SUCCESS;
    }

    struct HksParam *authResultParam = NULL;
    ret = HksGetParam(keyNode->authRuntimeParamSet, HKS_TAG_KEY_AUTH_RESULT, &authResultParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_BAD_STATE, "get authResult failed!")

    *authResult = authResultParam->int32Param;
    return HKS_SUCCESS;
}

static int32_t CheckParamsAndGetAppendState(const struct HuksKeyNode *keyNode, struct HksParam **isAppendDataParam)
{
    HKS_IF_NULL_LOGE_RETURN(keyNode, HKS_ERROR_NULL_POINTER, "the pointer param is invalid")

    struct HksParam *isAppendData = NULL;
    int32_t ret = HksGetParam(keyNode->authRuntimeParamSet, HKS_TAG_IS_APPEND_UPDATE_DATA, &isAppendData);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_BAD_STATE, "get is append update param failed")

    *isAppendDataParam = isAppendData;
    return HKS_SUCCESS;
}

static int32_t GetSupportAppendAuthInfoParams(const struct HuksKeyNode *keyNode, bool *isNeedAppendAuthInfo,
    int32_t *authResultOut)
{
    struct HksParam *isNeedAppendParam = NULL;
    int32_t ret = HksGetParam(keyNode->authRuntimeParamSet, HKS_TAG_IF_NEED_APPEND_AUTH_INFO, &isNeedAppendParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_BAD_STATE, "get is need append param failed")

    int32_t authResult = (int32_t) HKS_AUTH_RESULT_NONE;
    ret = GetUserAuthResult(keyNode, &authResult);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_BAD_STATE, "get auth result failed")

    *isNeedAppendAuthInfo = isNeedAppendParam->boolParam;
    *authResultOut = authResult;
    return HKS_SUCCESS;
}

static int32_t CheckIfNeedAppendUpdateData(const struct HksAppendDataInnerParams *innerParams, bool *outIsNeedAppend,
    int32_t *outAuthResult, const struct HksBlob *appendedData, struct HksParam **isAppendDataParam)
{
    bool isNeedAppend = false;
    int32_t authResult = HKS_AUTH_RESULT_NONE;
    int32_t ret = GetSupportAppendAuthInfoParams(innerParams->keyNode, &isNeedAppend, &authResult);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get append auth support params failed")

    *outAuthResult = authResult;
    if (isNeedAppend == false) {
        *outIsNeedAppend = false;
        return HKS_SUCCESS;
    }

    struct HksParam *isAlreadyAppendData = NULL;
    ret = CheckParamsAndGetAppendState(innerParams->keyNode, &isAlreadyAppendData);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "check is append data params failed")

    if (isAlreadyAppendData->boolParam == true) {
        *outIsNeedAppend = false;
        return HKS_SUCCESS;
    }

    if (innerParams->inData == NULL || innerParams->inData->size == 0 || appendedData == NULL) {
        HKS_LOG_E("the in data is invalid");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    if (UINT32_MAX - innerParams->inData->size < sizeof(struct HksSecureSignAuthInfo)) {
        HKS_LOG_E("inData size is too large");
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    *outIsNeedAppend = true;
    *isAppendDataParam = isAlreadyAppendData;
    return HKS_SUCCESS;
}

static int32_t GetSecureSignAuthInfo(const struct HuksKeyNode *keyNode, struct HksSecureSignAuthInfo *secureSignInfo)
{
    struct HksParam *authTokenParam = NULL;
    int32_t ret = HksGetParam(keyNode->authRuntimeParamSet, HKS_TAG_VERIFIED_AUTH_TOKEN, &authTokenParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_BAD_STATE, "get verified auth token failed")

    if (authTokenParam->blob.size != sizeof(struct HksUserAuthToken)) {
        return HKS_ERROR_BAD_STATE;
    }

    struct HksUserAuthToken *authToken = (struct HksUserAuthToken *)authTokenParam->blob.data;
    uint32_t hksAuthType;
    ret = HksConvertUserIamTypeToHksType(HKS_AUTH_TYPE, authToken->authType, &hksAuthType);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_BAD_STATE, "invalid user iam auth type")

    secureSignInfo->userAuthType = hksAuthType;
    secureSignInfo->credentialId = authToken->credentialId;
    secureSignInfo->authenticatorId = authToken->enrolledId;
    return HKS_SUCCESS;
}

static int32_t DoAppendPrefixAuthInfoToUpdateInData(const struct HuksKeyNode *keyNode,
    struct HksSecureSignAuthInfo *secureSignInfo, const struct HksBlob *inData, struct HksBlob *outDataBlob)
{
    uint32_t outDataSize = sizeof(uint32_t) + sizeof(struct HksSecureSignAuthInfo) + inData->size;
    uint8_t *outData = (uint8_t *)HksMalloc(outDataSize);
    HKS_IF_NULL_LOGE_RETURN(outData, HKS_ERROR_MALLOC_FAIL, "malloc outData failed!")

    uint32_t version = SECURE_SIGN_VERSION;
    (void)memcpy_s(outData, outDataSize, (uint8_t *)&version, sizeof(uint32_t));

    (void)memcpy_s(outData + sizeof(uint32_t), outDataSize - sizeof(uint32_t), secureSignInfo,
        sizeof(struct HksSecureSignAuthInfo));

    (void)memcpy_s(outData + sizeof(uint32_t) + sizeof(struct HksSecureSignAuthInfo),
        outDataSize - sizeof(uint32_t) - sizeof(struct HksSecureSignAuthInfo), inData->data, inData->size);

    struct HksParam *appendDataPrefixParam = NULL;
    int32_t ret = HksGetParam(keyNode->authRuntimeParamSet, HKS_TAG_APPENDED_DATA_PREFIX, &appendDataPrefixParam);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("get append prefix data failed");
        HKS_FREE_PTR(outData);
        return HKS_ERROR_BAD_STATE;
    }

    if (memcpy_s(appendDataPrefixParam->blob.data, appendDataPrefixParam->blob.size, secureSignInfo,
        sizeof(struct HksSecureSignAuthInfo)) != EOK) {
        HKS_LOG_E("get append prefix data failed");
        HKS_FREE_PTR(outData);
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    outDataBlob->data = outData;
    outDataBlob->size = outDataSize;
    return HKS_SUCCESS;
}

static int32_t CheckIfNeedAppendFinishData(const struct HksAppendDataInnerParams *innerParams, bool *outIsNeedAppend,
    int32_t *outAuthResult, uint32_t inOutDataOriginSize)
{
    bool isNeedAppend = false;
    int32_t authResult = HKS_AUTH_RESULT_NONE;
    int32_t ret = GetSupportAppendAuthInfoParams(innerParams->keyNode, &isNeedAppend, &authResult);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get append auth support params failed")

    *outAuthResult = authResult;
    if (isNeedAppend == false) {
        *outIsNeedAppend = false;
        return HKS_SUCCESS;
    }

    if (authResult != HKS_AUTH_RESULT_SUCCESS) {
        HKS_LOG_E("key auth failed");
        return HKS_ERROR_KEY_AUTH_FAILED;
    }

    struct HksParam *isAlreadyAppendUpdateData = NULL;
    ret = CheckParamsAndGetAppendState(innerParams->keyNode, &isAlreadyAppendUpdateData);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "check is already append update data params failed")

    if (isAlreadyAppendUpdateData->boolParam == false) {
        HKS_LOG_E("did not append update data");
        return HKS_ERROR_BAD_STATE;
    }

    if (innerParams->inData == NULL || innerParams->inData->size == 0) {
        HKS_LOG_E("the in data is invalid");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    if (inOutDataOriginSize < innerParams->inData->size ||
        inOutDataOriginSize - innerParams->inData->size < sizeof(struct HksSecureSignAuthInfo)) {
        HKS_LOG_E("outData origin buffer size is too small to append auth info");
        return HKS_ERROR_BUFFER_TOO_SMALL;
    }

    *outIsNeedAppend = true;
    return HKS_SUCCESS;
}

static int32_t DoAppendPrefixDataToFinishData(const struct HuksKeyNode *keyNode,
    struct HksAppendDataInnerParams *innerParams, struct HksBlob *inOutData, uint32_t inOutDataOriginSize)
{
    struct HksParam *appendDataPrefixParam = NULL;
    int32_t ret = HksGetParam(keyNode->authRuntimeParamSet, HKS_TAG_APPENDED_DATA_PREFIX, &appendDataPrefixParam);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_BAD_STATE, "get append prefix data failed")

    uint32_t cacheOutDataSize = sizeof(uint32_t) + sizeof(struct HksSecureSignAuthInfo) + innerParams->inData->size;
    uint8_t *cacheOutData = (uint8_t *)HksMalloc(cacheOutDataSize);
    HKS_IF_NULL_LOGE_RETURN(cacheOutData, HKS_ERROR_MALLOC_FAIL, "malloc cacheOutData failed!")

    const uint32_t version = SECURE_SIGN_VERSION;
    (void)memcpy_s(cacheOutData, cacheOutDataSize, &version, sizeof(uint32_t));

    (void)memcpy_s(cacheOutData + sizeof(uint32_t), cacheOutDataSize - sizeof(uint32_t),
        appendDataPrefixParam->blob.data, appendDataPrefixParam->blob.size);

    (void)memcpy_s(cacheOutData + sizeof(uint32_t) + appendDataPrefixParam->blob.size,
        cacheOutDataSize - appendDataPrefixParam->blob.size - sizeof(uint32_t), innerParams->inData->data,
        innerParams->inData->size);

    if (memcpy_s(inOutData->data, inOutDataOriginSize, cacheOutData, cacheOutDataSize) != 0) {
        HKS_LOG_E("memcpy cacheOutData to inOutData failed!");
        HKS_FREE_PTR(cacheOutData);
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    inOutData->size = cacheOutDataSize;
    HKS_FREE_PTR(cacheOutData);
    return HKS_SUCCESS;
}

int32_t HksCoreSecureAccessInitParams(struct HuksKeyNode *keyNode, const struct HksParamSet *initParamSet,
    struct HksBlob *token)
{
    if (keyNode == NULL || initParamSet == NULL || token == NULL) {
        HKS_LOG_E("the pointer param is invalid");
        return HKS_ERROR_NULL_POINTER;
    }
    struct HksSecureAccessInnerParams innerParams;
    (void)memset_s(&innerParams, sizeof(innerParams), 0, sizeof(innerParams));

    innerParams.initParamSet = initParamSet;
    innerParams.outToken = token;

    int32_t ret = CheckInitParamSetValidityAndGet(keyNode->keyBlobParamSet, &innerParams);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "check init params failed")

    struct HksParamSet *authRuntimeParamSet = NULL;
    ret = BuildAuthRuntimeParamSet(&innerParams, innerParams.isSecureSign, &authRuntimeParamSet);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "build auth run time params failed")

    keyNode->authRuntimeParamSet = authRuntimeParamSet;
    return HKS_SUCCESS;
}

int32_t HksCoreSecureAccessVerifyParams(struct HuksKeyNode *keyNode, const struct HksParamSet *paramSet)
{
    if (keyNode == NULL || paramSet == NULL) {
        HKS_LOG_E("the pointer param is invalid");
        return HKS_ERROR_NULL_POINTER;
    }

    struct HksParam *authResult = NULL;
    bool isNeedSecureAccess = true;
    
    int32_t ret = CheckIfNeedVerifyParams(keyNode, &isNeedSecureAccess, &authResult);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_BAD_STATE, "check if need verify params failed!")

    if (isNeedSecureAccess == false) {
        return HKS_SUCCESS;
    }

    if (authResult->uint32Param == HKS_AUTH_RESULT_FAILED) {
        HKS_LOG_E("check key auth failed");
        return HKS_ERROR_KEY_AUTH_FAILED;
    }

    struct HksUserAuthToken *authToken = NULL;
    do {
        ret = GetAuthToken(paramSet, &authToken);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "get auth token failed!")

        ret = HksVerifyAuthTokenSign(authToken);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "verify the auth token sign failed!")

        ret = VerifyChallengeOrTimeStamp(keyNode, authToken);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "verify challenge failed!")

        ret = VerifyAuthTokenInfo(keyNode, authToken);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "verify auth token info failed!")
    } while (0);

    return AssignVerifyResultAndFree(ret, authResult, keyNode, authToken);
}

int32_t HksCoreAppendAuthInfoBeforeUpdate(struct HuksKeyNode *keyNode, uint32_t pur,
    const struct HksParamSet *inParamSet, const struct HksBlob *inData, struct HksBlob *appendedData)
{
    // current only support append secure sign
    if (pur != HKS_KEY_PURPOSE_SIGN) {
        return HKS_SUCCESS;
    }

    bool isNeedAppend = false;
    int32_t authResult = HKS_AUTH_RESULT_NONE;
    struct HksParam *isAppendedData = NULL;
    struct HksAppendDataInnerParams innerParams = {
        .keyNode = keyNode,
        .inParamSet = inParamSet,
        .inData = inData
    };

    int32_t ret = CheckIfNeedAppendUpdateData(&innerParams, &isNeedAppend, &authResult, appendedData, &isAppendedData);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get if need append update data params failed")

    if (isNeedAppend == false) {
        return HKS_SUCCESS;
    }

    if (authResult != HKS_AUTH_RESULT_SUCCESS) {
        HKS_LOG_E("should do user auth success before update");
        return HKS_ERROR_KEY_AUTH_FAILED;
    }

    struct HksSecureSignAuthInfo secureSignInfo;
    ret = GetSecureSignAuthInfo(keyNode, &secureSignInfo);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get secure sign auth info failed")

    struct HksBlob outDataBlob = { 0, NULL };
    ret = DoAppendPrefixAuthInfoToUpdateInData(keyNode, &secureSignInfo, inData, &outDataBlob);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "do append prefix auth info to update in data failed")

    isAppendedData->boolParam = true;
    *appendedData = outDataBlob;
    return HKS_SUCCESS;
}

int32_t HksCoreAppendAuthInfoBeforeFinish(struct HuksKeyNode *keyNode, uint32_t pur,
    const struct HksParamSet *inParamSet, const struct HksBlob *inData, struct HksBlob *appendedData)
{
    return HksCoreAppendAuthInfoBeforeUpdate(keyNode, pur, inParamSet, inData, appendedData);
}

int32_t HksCoreAppendAuthInfoAfterFinish(struct HuksKeyNode *keyNode, uint32_t pur,
    const struct HksParamSet *inParamSet, uint32_t inOutDataOriginSize, struct HksBlob *inOutData)
{
    if (pur != HKS_KEY_PURPOSE_SIGN) {
        return HKS_SUCCESS;
    }

    bool isNeedAppend = false;
    int32_t authResult = HKS_AUTH_RESULT_NONE;
    const struct HksBlob *inDataConst = (const struct HksBlob *)inOutData;
    struct HksAppendDataInnerParams innerParams = {
        .keyNode = keyNode,
        .inParamSet = inParamSet,
        .inData = inDataConst
    };

    int32_t ret = CheckIfNeedAppendFinishData(&innerParams, &isNeedAppend, &authResult, inOutDataOriginSize);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get if need append finish data params failed")

    if (isNeedAppend == false) {
        return HKS_SUCCESS;
    }

    return DoAppendPrefixDataToFinishData(keyNode, &innerParams, inOutData, inOutDataOriginSize);
}
#else
int32_t HksCoreSecureAccessInitParams(struct HuksKeyNode *keyNode, const struct HksParamSet *initParamSet,
    struct HksBlob *token)
{
    (void)keyNode;
    (void)initParamSet;
    (void)token;
    return HKS_SUCCESS;
}

int32_t HksCoreSecureAccessVerifyParams(struct HuksKeyNode *keyNode, const struct HksParamSet *inParamSet)
{
    (void)keyNode;
    (void)inParamSet;
    return HKS_SUCCESS;
}

int32_t HksCoreAppendAuthInfoBeforeUpdate(struct HuksKeyNode *keyNode, uint32_t pur,
    const struct HksParamSet *inParamSet, const struct HksBlob *inData, struct HksBlob *appendedData)
{
    (void)keyNode;
    (void)pur;
    (void)inParamSet;
    (void)inData;
    (void)appendedData;
    return HKS_SUCCESS;
}

int32_t HksCoreAppendAuthInfoBeforeFinish(struct HuksKeyNode *keyNode, uint32_t pur,
    const struct HksParamSet *inParamSet, const struct HksBlob *inData, struct HksBlob *appendedData)
{
    (void)keyNode;
    (void)pur;
    (void)inParamSet;
    (void)inData;
    (void)appendedData;
    return HKS_SUCCESS;
}

int32_t HksCoreAppendAuthInfoAfterFinish(struct HuksKeyNode *keyNode, uint32_t pur,
    const struct HksParamSet *inParamSet, uint32_t inOutDataBufferSize, struct HksBlob *inOutData)
{
    (void)keyNode;
    (void)pur;
    (void)inParamSet;
    (void)inOutDataBufferSize;
    (void)inOutData;
    return HKS_SUCCESS;
}
#endif

#ifndef _STORAGE_LITE_
#ifdef HKS_SUPPORT_ACCESS_TOKEN
static int32_t HksCheckCompareAccessTokenId(const struct HksParamSet *blobParamSet,
    const struct HksParamSet *runtimeParamSet)
{
    struct HksParam *blobAccessTokenId = NULL;
    int32_t ret = HksGetParam(blobParamSet, HKS_TAG_ACCESS_TOKEN_ID, &blobAccessTokenId);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_SUCCESS, "no access token id in keyblob")

    struct HksParam *runtimeAccessTokenId = NULL;
    ret = HksGetParam(runtimeParamSet, HKS_TAG_ACCESS_TOKEN_ID, &runtimeAccessTokenId);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_BAD_STATE, "get access token id form runtime paramSet failed")

    ret = (blobAccessTokenId->uint64Param == runtimeAccessTokenId->uint64Param) ? HKS_SUCCESS : HKS_ERROR_BAD_STATE;
    return ret;
}

#else
static int32_t HksCheckCompareAccessTokenId(const struct HksParamSet *blobParamSet,
    const struct HksParamSet *runtimeParamSet)
{
    (void)blobParamSet;
    (void)runtimeParamSet;
    return HKS_SUCCESS;
}
#endif

static int32_t HksCheckCompareProcessName(const struct HksParamSet *blobParamSet,
    const struct HksParamSet *runtimeParamSet)
{
    struct HksParam *blobProcessName = NULL;
    int32_t ret = HksGetParam(blobParamSet, HKS_TAG_PROCESS_NAME, &blobProcessName);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_BAD_STATE, "no process name in keyblob")

    struct HksParam *runtimeProcessName = NULL;
    ret = HksGetParam(runtimeParamSet, HKS_TAG_PROCESS_NAME, &runtimeProcessName);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_BAD_STATE, "get process name form runtime paramSet failed")

    if (blobProcessName->blob.size == runtimeProcessName->blob.size &&
        HksMemCmp(blobProcessName->blob.data, runtimeProcessName->blob.data,
            blobProcessName->blob.size) == HKS_SUCCESS) {
        return HKS_SUCCESS;
    }
    return HKS_ERROR_BAD_STATE;
}
#endif /** _STORAGE_LITE_ */

int32_t HksProcessIdentityVerify(const struct HksParamSet *blobParamSet, const struct HksParamSet *runtimeParamSet)
{
    int32_t ret = HKS_SUCCESS;
    
#ifndef _STORAGE_LITE_
    ret = HksCheckCompareAccessTokenId(blobParamSet, runtimeParamSet);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "access token compare failed")

    ret = HksCheckCompareProcessName(blobParamSet, runtimeParamSet);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "process name compare failed")
#else
    (void)blobParamSet;
    (void)runtimeParamSet;
#endif

    return ret;
}
