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

#include "network/softbus/softbus_session.h"

#include "dfs_session.h"
#include "session.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace std;

constexpr int32_t SOFTBUS_OK = 0;
constexpr int32_t DEVICE_ID_SIZE_MAX = 65;
constexpr int32_t IS_SERVER = 0;

SoftbusSession::SoftbusSession(int sessionId) : sessionId_(sessionId)
{
    char perDevId[DEVICE_ID_SIZE_MAX] = "";
    int ret = ::GetPeerDeviceId(sessionId_, perDevId, sizeof(perDevId));
    if (ret != SOFTBUS_OK) {
        LOGE("get my peer device id failed, errno:%{public}d, sessionId:%{public}d", ret, sessionId_);
        cid_ = "";
    } else {
        cid_ = string(perDevId);
    }

    int32_t socket_fd;
    ret = ::GetSessionHandle(sessionId_, &socket_fd);
    if (ret != SOFTBUS_OK) {
        LOGE("get session socket fd failed, errno:%{public}d, sessionId:%{public}d", ret, sessionId_);
        socketFd_ = INVALID_SOCKET_FD;
    } else {
        socketFd_ = socket_fd;
    }

    array<char, KEY_SIZE_MAX> key;
    ret = ::GetSessionKey(sessionId_, key.data(), key.size());
    if (ret != SOFTBUS_OK) {
        LOGE("get session key failed, errno:%{public}d, sessionId:%{public}d", ret, sessionId_);
        key_ = {};
    } else {
        key_ = key;
    }

    IsServerSide_ = (::GetSessionSide(sessionId_) == IS_SERVER) ? true : false;
}

bool SoftbusSession::IsFromServer() const
{
    return IsServerSide_;
}

string SoftbusSession::GetCid() const
{
    return cid_;
}

int32_t SoftbusSession::GetHandle() const
{
    return socketFd_;
}

array<char, KEY_SIZE_MAX> SoftbusSession::GetKey() const
{
    return key_;
}

void SoftbusSession::Release() const
{
    ::CloseSession(sessionId_);
    LOGI("session closed, sessionId:%{public}d", sessionId_);
}

void SoftbusSession::DisableSessionListener() const
{
    int32_t ret = ::DisableSessionListener(sessionId_);
    if (ret != SOFTBUS_OK) {
        LOGE("disableSessionlistener failed, errno:%{public}d, sessionId:%{public}d", ret, sessionId_);
        return;
    }
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
