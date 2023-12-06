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

#ifndef USER_IDM_CLIENT_H
#define USER_IDM_CLIENT_H

#include "user_iam_define.h"

namespace OHOS {
namespace UserIam {
namespace UserAuth {
class UserIdmClient {
public:
    static UserIdmClient &GetInstance();
    ~UserIdmClient() = default;
    int32_t GetCredentialInfo(int32_t userId, AuthType authType,
        const std::shared_ptr<GetCredentialInfoCallback> &callback);
    int32_t GetSecUserInfo(int32_t userId, const std::shared_ptr<GetSecUserInfoCallback> &callback);
};

void ChangeGetCredentialInfoReturn(bool isValid);

void ChangeGetSecUserInfoReturn(bool isValid);
} // namespace UserAuth
} // namespace UserIam
} // namespace OHOS
#endif // USER_IDM_CLIENT_H