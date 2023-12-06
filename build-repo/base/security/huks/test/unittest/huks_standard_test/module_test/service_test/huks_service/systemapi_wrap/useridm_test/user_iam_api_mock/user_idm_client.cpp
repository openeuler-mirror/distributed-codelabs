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

#include "user_idm_client.h"

#include "hks_log.h"

#include <thread>
#include <unistd.h>

namespace OHOS {
namespace UserIam {
namespace UserAuth {
static const uint32_t g_sleepTime = 1;
static const uint32_t g_enrolledForPin = 1;
static const uint32_t g_enrolledForFace = 2;
static const uint32_t g_enrolledForFingerPrint = 4;

static volatile bool isCredentialInfoValid = true;
static volatile bool isSecUserInfoValid = true;

// only for tdd test, no need for lock
static std::vector<CredentialInfo> g_credentialInfos;
static SecUserInfo g_userInfo;

void RunOnCredentialInfo(const std::shared_ptr<GetCredentialInfoCallback>& callback)
{
    sleep(g_sleepTime);
    (*callback).OnCredentialInfo(g_credentialInfos);
}

static void ConstructCredentialInfo()
{
    if (isCredentialInfoValid) {
        // value is not needed
        struct CredentialInfo cre = {0};
        g_credentialInfos.push_back(cre);
    } else {
        g_credentialInfos.clear();
    }
}

int32_t UserIdmClient::GetCredentialInfo(int32_t userId, AuthType authType,
    const std::shared_ptr<GetCredentialInfoCallback>& callback)
{
    if (userId == 0) {
        return FAIL;
    }
    ConstructCredentialInfo();
    std::thread thObj(RunOnCredentialInfo, std::ref(callback));
    thObj.detach();
    return SUCCESS;
}

void RunOnSecUserInfo(const std::shared_ptr<GetSecUserInfoCallback>& callback)
{
    sleep(g_sleepTime);
    (*callback).OnSecUserInfo(g_userInfo);
}

static void ConstructSecUserInfo()
{
    if (isSecUserInfoValid) {
        struct EnrolledInfo pinInfo = { PIN, g_enrolledForPin };
        struct EnrolledInfo faceInfo = { FACE, g_enrolledForFace };
        struct EnrolledInfo fingetPrintInfo = { FINGERPRINT, g_enrolledForFingerPrint };
        g_userInfo.enrolledInfo.push_back(pinInfo);
        g_userInfo.enrolledInfo.push_back(faceInfo);
        g_userInfo.enrolledInfo.push_back(fingetPrintInfo);
    } else {
        g_userInfo.enrolledInfo.clear();
    }
}

int32_t UserIdmClient::GetSecUserInfo(int32_t userId, const std::shared_ptr<GetSecUserInfoCallback>& callback)
{
    if (userId == 0) {
        return FAIL;
    }
    
    ConstructSecUserInfo();
    std::thread thObj(RunOnSecUserInfo, std::ref(callback));
    thObj.detach();
    return SUCCESS;
}

UserIdmClient &UserIdmClient::GetInstance()
{
    static UserIdmClient impl;
    return impl;
}

void ChangeGetCredentialInfoReturn(bool isValid)
{
    isCredentialInfoValid = isValid;
}

void ChangeGetSecUserInfoReturn(bool isValid)
{
    isSecUserInfoValid = isValid;
}
} // namespace UserAuth
} // namespace UserIam
} // namespace OHOS