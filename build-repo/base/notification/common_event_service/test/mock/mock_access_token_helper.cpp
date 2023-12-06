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
#include "mock_constant.h"

namespace OHOS {
namespace EventFwk {
using namespace OHOS::Security;

bool AccessTokenHelper::VerifyNativeToken(const AccessToken::AccessTokenID &callerToken)
{
    return callerToken == PERMISSION_GRANTED;
}

bool AccessTokenHelper::VerifyAccessToken(const AccessToken::AccessTokenID &callerToken, const std::string &permission)
{
    return callerToken == PERMISSION_GRANTED;
}

void AccessTokenHelper::RecordSensitivePermissionUsage(const AccessToken::AccessTokenID &callerToken,
    const std::string &event)
{}

bool AccessTokenHelper::IsDlpHap(const AccessToken::AccessTokenID &callerToken)
{
    return callerToken == DLP_PERMISSION_GRANTED;
}
}  // namespace EventFwk
}  // namespace OHOS
