/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "access_token_helper.h"

#include "common_event_permission_manager.h"
#include "event_log_wrapper.h"
#include "ipc_skeleton.h"
#include "privacy_kit.h"

using namespace OHOS::Security::AccessToken;

namespace OHOS {
namespace EventFwk {
bool AccessTokenHelper::VerifyNativeToken(const AccessTokenID &callerToken)
{
    ATokenTypeEnum tokenType = AccessTokenKit::GetTokenTypeFlag(callerToken);
    return (tokenType == ATokenTypeEnum::TOKEN_NATIVE || tokenType == ATokenTypeEnum::TOKEN_SHELL);
}

bool AccessTokenHelper::VerifyAccessToken(const AccessTokenID &callerToken,
    const std::string &permission)
{
    return (AccessTokenKit::VerifyAccessToken(callerToken, permission) ==
        AccessToken::PermissionState::PERMISSION_GRANTED);
}

void AccessTokenHelper::RecordSensitivePermissionUsage(const AccessTokenID &callerToken,
    const std::string &event)
{
    EVENT_LOGI("enter");
    ATokenTypeEnum tokenType = AccessTokenKit::GetTokenTypeFlag(callerToken);
    if (tokenType != ATokenTypeEnum::TOKEN_HAP) {
        EVENT_LOGE("tokenType != ATokenTypeEnum::TOKEN_HAP");
        return;
    }
    Permission permission = DelayedSingleton<CommonEventPermissionManager>::GetInstance()->GetEventPermission(event);
    if (!permission.isSensitive || permission.names.empty()) {
        EVENT_LOGE("permission denied");
        return;
    }
    for (const auto &permissionName : permission.names) {
        PrivacyKit::AddPermissionUsedRecord(callerToken, permissionName, 1, 0);
    }
}

bool AccessTokenHelper::IsDlpHap(const AccessTokenID &callerToken)
{
    ATokenTypeEnum type = AccessTokenKit::GetTokenTypeFlag(callerToken);
    if (type == ATokenTypeEnum::TOKEN_HAP) {
        HapTokenInfo info;
        AccessTokenKit::GetHapTokenInfo(callerToken, info);
        return (info.dlpType == DlpType::DLP_READ || info.dlpType == DlpType::DLP_FULL_CONTROL);
    }
    return false;
}

AccessTokenID AccessTokenHelper::GetHapTokenID(int userID,
    const std::string& bundleName, int instIndex)
{
    return AccessTokenKit::GetHapTokenID(userID, bundleName, instIndex);
}
}  // namespace EventFwk
}  // namespace OHOS
