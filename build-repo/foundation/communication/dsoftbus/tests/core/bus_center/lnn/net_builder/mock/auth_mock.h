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

#ifndef AUTH_MOCK_H
#define AUTH_MOCK_H

#include <gmock/gmock.h>

#include "auth_interface.h"

namespace OHOS {
class AuthInterface {
public:
    AuthInterface() {};
    virtual ~AuthInterface() {};

    virtual void AuthHandleLeaveLNN(int64_t authId) = 0;
    virtual uint32_t AuthGenRequestId(void) = 0;
    virtual int32_t AuthStartVerify(const AuthConnInfo *connInfo,
        uint32_t requestId, const AuthVerifyCallback *callback) = 0;
    virtual int32_t AuthGetVersion(int64_t authId, SoftBusVersion *version) = 0;
    virtual int32_t AuthGetDeviceUuid(int64_t authId, char *uuid, uint16_t size) = 0;
};
class AuthInterfaceMock : public AuthInterface {
public:
    AuthInterfaceMock();
    ~AuthInterfaceMock() override;
    MOCK_METHOD(void, AuthHandleLeaveLNN, (int64_t), (override));
    MOCK_METHOD(uint32_t, AuthGenRequestId, (), (override));
    MOCK_METHOD(int32_t, AuthStartVerify, (const AuthConnInfo *,
        uint32_t, const AuthVerifyCallback *), (override));
    MOCK_METHOD(int32_t, AuthGetVersion, (int64_t, SoftBusVersion *), (override));
    MOCK_METHOD(int32_t, AuthGetDeviceUuid, (int64_t, char *, uint16_t), (override));
};
} // namespace OHOS
#endif // AUTH_MOCK_H