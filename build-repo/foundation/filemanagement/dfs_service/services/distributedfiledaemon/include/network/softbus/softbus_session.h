/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef SOFTBUS_SESSION_H
#define SOFTBUS_SESSION_H

#include <array>
#include <cstdint>
#include <string>

#include "network/base_session.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
constexpr int32_t INVALID_SOCKET_FD = -1;
class SoftbusSession final : public BaseSession {
public:
    explicit SoftbusSession(int sessionId);
    ~SoftbusSession() = default;
    bool IsFromServer() const override;
    std::string GetCid() const override;
    int32_t GetHandle() const override;
    std::array<char, KEY_SIZE_MAX> GetKey() const override;
    void Release() const override;
    void DisableSessionListener() const override;

private:
    int sessionId_;
    std::string cid_;
    int32_t socketFd_ {INVALID_SOCKET_FD};
    std::array<char, KEY_SIZE_MAX> key_;
    bool IsServerSide_;
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // SOFTBUS_SESSION_H