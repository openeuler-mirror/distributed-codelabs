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

#ifndef BASE_NOTIFICATION_CES_STANDARD_SERVICES_CES_INCLUDE_ACCESS_TOKEN_HELPER_H
#define BASE_NOTIFICATION_CES_STANDARD_SERVICES_CES_INCLUDE_ACCESS_TOKEN_HELPER_H

#include "accesstoken_kit.h"

namespace OHOS {
namespace EventFwk {
using namespace OHOS::Security;
class AccessTokenHelper {
public:
    /**
     * Verifies native token.
     *
     * @param callerToken Indicates the token of caller.
     * @return Returns true if successful; false otherwise.
     */
    static bool VerifyNativeToken(const AccessToken::AccessTokenID &callerToken);
    static bool VerifyAccessToken(const AccessToken::AccessTokenID &callerToken,
        const std::string &permission);
    static void RecordSensitivePermissionUsage(const AccessToken::AccessTokenID &callerToken,
        const std::string &event);
    static bool IsDlpHap(const AccessToken::AccessTokenID &callerToken);
    static AccessToken::AccessTokenID GetHapTokenID(int userID, const std::string& bundleName, int instIndex);
};
}  // namespace EventFwk
}  // namespace OHOS
#endif  // BASE_NOTIFICATION_CES_STANDARD_SERVICES_CES_INCLUDE_ACCESS_TOKEN_HELPER_H