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

#include "hks_useridm_api_wrap.h"

#include "hks_condition.h"
#include "hks_log.h"
#include "hks_mem.h"
#include "hks_template.h"

#ifdef HKS_SUPPORT_USER_AUTH_ACCESS_CONTROL
#include "user_idm_client.h"

#define USER_IAM OHOS::UserIam::UserAuth

static constexpr const uint32_t g_maxEnrolledLen = 256;

static int32_t ConvertFromHksAuthType(enum HksUserAuthType hksAuthType, enum USER_IAM::AuthType *authType)
{
    switch (hksAuthType) {
        case HKS_USER_AUTH_TYPE_FACE:
            *authType = USER_IAM::AuthType::FACE;
            break;
        case HKS_USER_AUTH_TYPE_PIN:
            *authType = USER_IAM::AuthType::PIN;
            break;
        case HKS_USER_AUTH_TYPE_FINGERPRINT:
            *authType = USER_IAM::AuthType::FINGERPRINT;
            break;
        default:
            HKS_LOG_E("Invalid hksAuthType!");
            return HKS_ERROR_NOT_SUPPORTED;
    }
    return HKS_SUCCESS;
}

static int32_t ConvertToHksAuthType(enum USER_IAM::AuthType authType, enum HksUserAuthType *hksAuthType)
{
    switch (authType) {
        case USER_IAM::AuthType::FACE:
            *hksAuthType = HKS_USER_AUTH_TYPE_FACE;
            break;
        case USER_IAM::AuthType::PIN:
            *hksAuthType =  HKS_USER_AUTH_TYPE_PIN;
            break;
        case USER_IAM::AuthType::FINGERPRINT:
            *hksAuthType =  HKS_USER_AUTH_TYPE_FINGERPRINT;
            break;
        default:
            HKS_LOG_E("Invalid authType!");
            return HKS_ERROR_NOT_SUPPORTED;
    }
    return HKS_SUCCESS;
}

class GetSecUserInfoCallbackImplHuks : public USER_IAM::GetSecUserInfoCallback {
public:
    GetSecUserInfoCallbackImplHuks(struct SecInfoWrap **mOutSecInfo, HksCondition *mCondition):outSecInfo(mOutSecInfo),
        condition(mCondition) {}
    virtual ~GetSecUserInfoCallbackImplHuks() = default;
    void OnSecUserInfo(const USER_IAM::SecUserInfo &info) override;
    volatile bool isCallbacked = false;

private:
    struct SecInfoWrap **outSecInfo;
    HksCondition *condition;
};

void GetSecUserInfoCallbackImplHuks::OnSecUserInfo(const USER_IAM::SecUserInfo &info)
{
    int32_t ret = HKS_SUCCESS;
    do {
        if (info.enrolledInfo.size() > g_maxEnrolledLen) {
            HKS_LOG_E("invalid num of enrolledInfo");
            ret = HKS_ERROR_GET_USERIAM_SECINFO_FAILED;
            break;
        }
        uint32_t secInfoSize = sizeof(struct SecInfoWrap) + ((info.enrolledInfo.size() >
            DEFAULT_ENROLLED_INFO_LEN) ? sizeof(struct EnrolledInfoWrap) * (info.enrolledInfo.size() -
            DEFAULT_ENROLLED_INFO_LEN) : 0);
        *outSecInfo = static_cast<struct SecInfoWrap *>(HksMalloc(secInfoSize));
        if (*outSecInfo == NULL) {
            HKS_LOG_E("Malloc enrolledInfo failed!");
            ret = HKS_ERROR_MALLOC_FAIL;
            break;
        }

        (**outSecInfo).secureUid = info.secureUid;
        (**outSecInfo).enrolledInfoLen = info.enrolledInfo.size();
        for (uint32_t i = 0; i < (**outSecInfo).enrolledInfoLen; ++i) {
            enum HksUserAuthType authType;
            ret = ConvertToHksAuthType(info.enrolledInfo[i].authType, &authType);
            HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "ConvertToHksAuthType failed :%" LOG_PUBLIC "d!", ret)

            (**outSecInfo).enrolledInfo[i].authType = authType;
                
            (**outSecInfo).enrolledInfo[i].enrolledId = info.enrolledInfo[i].enrolledId;
        }
    } while (0);
    if (ret != HKS_SUCCESS) {
        HksFree(*outSecInfo);
        *outSecInfo = NULL;
    }
    isCallbacked = true;
    HksConditionNotify(condition);
}

int32_t HksUserIdmGetSecInfo(int32_t userId, struct SecInfoWrap **outSecInfo)
{
    HKS_IF_NULL_LOGE_RETURN(outSecInfo, HKS_ERROR_INVALID_ARGUMENT, "HksUserIdmGetSecInfo arguments invalid!")

    HksCondition *condition = HksConditionCreate();
    HKS_IF_NULL_LOGE_RETURN(condition, HKS_FAILURE, "create condition failed!")

    auto mCallback = std::make_shared<GetSecUserInfoCallbackImplHuks>(outSecInfo, condition);

    std::shared_ptr<USER_IAM::GetSecUserInfoCallback> callback = mCallback;

    int32_t ret = USER_IAM::UserIdmClient::GetInstance().GetSecUserInfo(userId, callback);
    if (ret == USER_IAM::ResultCode::SUCCESS) {
        while (!(*mCallback).isCallbacked) {
            HksConditionWait(condition);
        }
    } else {
        HKS_LOG_E("GetSecInfoCallback failed: %" LOG_PUBLIC "d!", ret);
    }
    
    HksConditionDestroy(condition);
    
    if (ret != USER_IAM::ResultCode::SUCCESS || *outSecInfo == NULL) {
        HKS_LOG_E("get sec info failed!");
        return HKS_ERROR_GET_USERIAM_SECINFO_FAILED;
    }
    return HKS_SUCCESS;
}

class GetCredentialInfoCallbackImplHuks : public USER_IAM::GetCredentialInfoCallback {
public:
    GetCredentialInfoCallbackImplHuks(uint32_t *mNumOfAuthInfo, HksCondition *mCondition)
        :numOfAuthInfo(mNumOfAuthInfo), condition(mCondition)
    {
    }
    virtual ~GetCredentialInfoCallbackImplHuks() {}

    volatile bool isCallbacked = false;

    void OnCredentialInfo(const std::vector<USER_IAM::CredentialInfo> &infoList) override;
private:
    uint32_t *numOfAuthInfo;
    HksCondition *condition;
};

void GetCredentialInfoCallbackImplHuks::OnCredentialInfo(const std::vector<USER_IAM::CredentialInfo> &infoList)
{
    *numOfAuthInfo = infoList.size();
    isCallbacked = true;
    HksConditionNotify(condition);
}

int32_t HksUserIdmGetAuthInfoNum(int32_t userId, enum HksUserAuthType hksAuthType, uint32_t *numOfAuthInfo)
{
    HKS_IF_NULL_LOGE_RETURN(numOfAuthInfo, HKS_ERROR_INVALID_ARGUMENT, "HksGetAuthInfo arguments invalid!")

    HksCondition *condition = HksConditionCreate();
    HKS_IF_NULL_LOGE_RETURN(condition, HKS_ERROR_BAD_STATE, "create condition failed!")

    auto huksCallback = std::make_shared<GetCredentialInfoCallbackImplHuks>(numOfAuthInfo, condition);
    std::shared_ptr<USER_IAM::GetCredentialInfoCallback> callback = huksCallback;

    enum USER_IAM::AuthType authType;

    int32_t ret = ConvertFromHksAuthType(hksAuthType, &authType);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("ConvertFromHksAuthType failed: %" LOG_PUBLIC "d!", ret);
        HksConditionDestroy(condition);
        return ret;
    }

    ret = USER_IAM::UserIdmClient::GetInstance().GetCredentialInfo(userId, authType, callback);
    if (ret == USER_IAM::ResultCode::SUCCESS) {
        while (!(*huksCallback).isCallbacked) {
            HksConditionWait(condition);
        }
        ret = HKS_SUCCESS;
    } else if (ret == USER_IAM::ResultCode::NOT_ENROLLED) { // follow userIam errorCode
        HKS_LOG_E("no credential enrolled");
        ret = HKS_ERROR_CREDENTIAL_NOT_EXIST;
    } else {
        HKS_LOG_E("GetAuthInfo failed: %" LOG_PUBLIC "d!", ret);
        ret = HKS_ERROR_GET_USERIAM_AUTHINFO_FAILED;
    }
    
    HksConditionDestroy(condition);
    return ret;
}

int32_t HksConvertUserIamTypeToHksType(enum HksUserIamType type, uint32_t userIamValue, uint32_t *hksValue)
{
    HKS_IF_NULL_RETURN(hksValue, HKS_ERROR_NULL_POINTER)

    switch (type) {
        case HKS_AUTH_TYPE:
            return ConvertToHksAuthType(static_cast<enum USER_IAM::AuthType>(userIamValue),
                reinterpret_cast<enum HksUserAuthType *>(hksValue));
        default:
            break;
    }
    return HKS_ERROR_NOT_SUPPORTED;
}
#else
int32_t HksUserIdmGetSecInfo(int32_t userId, struct SecInfoWrap **outSecInfo)
{
    (void)userId;
    (void)outSecInfo;
    return HKS_ERROR_NOT_SUPPORTED;
}

int32_t HksUserIdmGetAuthInfoNum(int32_t userId, enum HksUserAuthType hksAuthType, uint32_t *numOfAuthInfo)
{
    (void)userId;
    (void)hksAuthType;
    (void)numOfAuthInfo;
    return HKS_ERROR_NOT_SUPPORTED;
}

int32_t HksConvertUserIamTypeToHksType(enum HksUserIamType type, uint32_t userIamValue, uint32_t *hksValue)
{
    (void)type;
    (void)userIamValue;
    (void)hksValue;
    return HKS_ERROR_NOT_SUPPORTED;
}
#endif
