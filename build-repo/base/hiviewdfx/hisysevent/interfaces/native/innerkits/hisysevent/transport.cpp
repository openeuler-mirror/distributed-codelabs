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

#include "transport.h"

#include <__mutex_base>
#include <cerrno>
#include <cstddef>
#include <iosfwd>
#include <list>
#include <securec.h>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "def.h"
#include "hilog/log.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, 0xD002D08, "HISYSEVENT" };
constexpr size_t BUF_SIZE = 2000;
char errMsg[BUF_SIZE] = { 0 };
}
Transport Transport::instance_;

Transport& Transport::GetInstance()
{
    return instance_;
}

void Transport::InitRecvBuffer(int socketId)
{
    int oldN = 0;
    socklen_t oldOutSize = static_cast<socklen_t>(sizeof(int));
    if (getsockopt(socketId, SOL_SOCKET, SO_SNDBUF, static_cast<void *>(&oldN), &oldOutSize) < 0) {
        strerror_r(errno, errMsg, BUF_SIZE);
        HiLog::Error(LABEL, "get socket send buffer error=%{public}d, msg=%{public}s", errno, errMsg);
    }

    int sendBuffSize = MAX_DATA_SIZE;
    if (setsockopt(socketId, SOL_SOCKET, SO_SNDBUF, static_cast<void *>(&sendBuffSize), sizeof(int)) < 0) {
        strerror_r(errno, errMsg, BUF_SIZE);
        HiLog::Error(LABEL, "set socket send buffer error=%{public}d, msg=%{public}s", errno, errMsg);
    }

    int newN = 0;
    socklen_t newOutSize = static_cast<socklen_t>(sizeof(int));
    if (getsockopt(socketId, SOL_SOCKET, SO_SNDBUF, static_cast<void *>(&newN), &newOutSize) < 0) {
        strerror_r(errno, errMsg, BUF_SIZE);
        HiLog::Error(LABEL, "get new socket send buffer error=%{public}d, msg=%{public}s", errno, errMsg);
    }
    HiLog::Debug(LABEL, "reset send buffer size old=%{public}d, new=%{public}d", oldN, newN);
}

int Transport::SendToHiSysEventDataSource(const std::string &text)
{
    struct sockaddr_un serverAddr;
    serverAddr.sun_family = AF_UNIX;
    if (strcpy_s(serverAddr.sun_path, sizeof(serverAddr.sun_path), "/dev/unix/socket/hisysevent") != EOK) {
        HiLog::Error(LABEL, "can not assign server path");
        return ERR_DOES_NOT_INIT;
    }
    serverAddr.sun_path[sizeof(serverAddr.sun_path) - 1] = '\0';

    int socketId = TEMP_FAILURE_RETRY(socket(AF_UNIX, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0));
    if (socketId < 0) {
        strerror_r(errno, errMsg, BUF_SIZE);
        HiLog::Error(LABEL, "create hisysevent client socket failed, error=%{public}d, msg=%{public}s",
            errno, errMsg);
        return ERR_DOES_NOT_INIT;
    }
    InitRecvBuffer(socketId);
    auto sendRet = 0;
    auto retryTimes = RETRY_TIMES;
    do {
        sendRet = sendto(socketId, text.c_str(), text.size(), 0,
            reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));
        retryTimes--;
    } while (sendRet < 0 && retryTimes > 0 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR));
    if (sendRet < 0) {
        close(socketId);
        strerror_r(errno, errMsg, BUF_SIZE);
        HiLog::Error(LABEL, "send data to hisysevent server failed, error=%{public}d, msg=%{public}s",
            errno, errMsg);
        return ERR_SEND_FAIL;
    }
    close(socketId);
    HiLog::Debug(LABEL, "HiSysEvent send data successful");
    return SUCCESS;
}

void Transport::AddFailedData(const std::string &text)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (retryDataList_.size() >= RETRY_QUEUE_SIZE) {
        HiLog::Info(LABEL, "dispatch retry sysevent data as reach max size");
        retryDataList_.pop_front();
    }
    retryDataList_.push_back(text);
}

void Transport::RetrySendFailedData()
{
    std::lock_guard<std::mutex> lock(mutex_);
    while (!retryDataList_.empty()) {
        std::string text = retryDataList_.front();
        HiLog::Debug(LABEL, "resend data size=%{public}lu, sysevent=%{public}s",
            static_cast<unsigned long>(text.size()), text.c_str());
        if (SendToHiSysEventDataSource(text) != SUCCESS) {
            return;
        }
        retryDataList_.pop_front();
    }
}

int Transport::SendData(const std::string &text)
{
    if (text.size() > MAX_DATA_SIZE) {
        HiLog::Error(LABEL, "data is too long %{public}lu", static_cast<unsigned long>(text.length()));
        return ERR_OVER_SIZE;
    }
    HiLog::Debug(LABEL, "size=%{public}lu, sysevent=%{public}s",
        static_cast<unsigned long>(text.size()), text.c_str());

    RetrySendFailedData();
    int tryTimes = RETRY_TIMES;
    int retCode = SUCCESS;
    while (tryTimes > 0) {
        tryTimes--;
        retCode = SendToHiSysEventDataSource(text);
        if (retCode == SUCCESS) {
            return retCode;
        }
    }

    AddFailedData(text);
    return retCode;
}
} // namespace HiviewDFX
} // namespace OHOS

