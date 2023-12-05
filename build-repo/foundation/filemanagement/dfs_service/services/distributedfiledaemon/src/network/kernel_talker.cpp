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

#include "network/kernel_talker.h"
#include "device/device_manager_agent.h"
#include "network/devsl_dispatcher.h"
#include "securec.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace std;

constexpr int KEY_MAX_LEN = 32;
constexpr int POLL_TIMEOUT_MS = 200;
constexpr int NONE_EVENT = -1;
constexpr int READ_EVENT = 1;
constexpr int TIME_OUT_EVENT = 0;

struct UpdateSocketParam {
    int32_t cmd;
    int32_t newfd;
    uint32_t devsl;
    uint8_t status;
    uint8_t masterKey[KEY_MAX_LEN];
    uint8_t cid[CID_MAX_LEN];
} __attribute__((packed));

struct UpdateDevslParam {
    int32_t cmd;
    uint32_t devsl;
    uint8_t cid[CID_MAX_LEN];
} __attribute__((packed));

struct OfflineParam {
    int32_t cmd;
    uint8_t remoteCid[CID_MAX_LEN];
} __attribute__((packed));

enum CmdCode {
    CMD_UPDATE_SOCKET = 0,
    CMD_UPDATE_DEVSL,
    CMD_OFF_LINE,
    CMD_OFF_LINE_ALL,
    CMD_CNT,
};

enum SocketStat {
    SOCKET_STAT_ACCEPT = 0,
    SOCKET_STAT_OPEN,
};

enum Notify {
    NOTIFY_GET_SESSION = 0,
    NOTIFY_OFFLINE,
    NOTIFY_NONE,
    NOTIFY_CNT,
};

void KernelTalker::SinkSessionTokernel(shared_ptr<BaseSession> session)
{
    int socketFd = session->GetHandle();
    auto masterkey = session->GetKey();
    auto cid = session->GetCid();

    uint8_t status = (session->IsFromServer() ? SOCKET_STAT_ACCEPT : SOCKET_STAT_OPEN);

    UpdateSocketParam cmd = {
        .cmd = CMD_UPDATE_SOCKET,
        .newfd = socketFd,
        .devsl = 4,
        .status = status,
    };
    if (memcpy_s(cmd.masterKey, KEY_MAX_LEN, masterkey.data(), KEY_MAX_LEN) != EOK) {
        return;
    }

    if (memcpy_s(cmd.cid, CID_MAX_LEN, cid.c_str(), cid.size())) {
        return;
    }
    SetCmd(cmd);
    LOGD("sink session to kernel success, cid:%{public}s, socketFd:%{public}d, key[0]:%{public}x, devsl:%{public}d",
         cid.c_str(), socketFd, *(uint32_t *)masterkey.data(), cmd.devsl);

    // DevslDispatcher::DevslGetRegister(cid, shared_from_this());
}

void KernelTalker::SinkDevslTokernel(const std::string &cid, uint32_t devsl)
{
    LOGD("sink dsl to kernel success, cid:%{public}s, devsl:%{public}d", cid.c_str(), devsl);
    UpdateDevslParam cmd = {
        .cmd = CMD_UPDATE_DEVSL,
        .devsl = devsl,
    };

    if (memcpy_s(cmd.cid, CID_MAX_LEN, cid.c_str(), CID_MAX_LEN)) {
        return;
    }
    SetCmd(cmd);
}

void KernelTalker::SinkOfflineCmdToKernel(string cid)
{
    OfflineParam cmd = {
        .cmd = CMD_OFF_LINE,
    };

    if (cid.length() < CID_MAX_LEN) {
        LOGE("cid lengh err, cid:%{public}s, length:%{public}zu", cid.c_str(), cid.length());
        return;
    }

    if (memcpy_s(cmd.remoteCid, CID_MAX_LEN, cid.c_str(), CID_MAX_LEN) != EOK) {
        return;
    }
    SetCmd(cmd);
}

void KernelTalker::CreatePollThread()
{
    isRunning_ = true;
    if (pollThread_ != nullptr) {
        LOGE("pollTread is not null");
        return;
    }
    pollThread_ = make_unique<thread>(&KernelTalker::PollRun, this);
    LOGI("Create pollThread OK");
}

void KernelTalker::WaitForPollThreadExited()
{
    isRunning_ = false;
    if (pollThread_ == nullptr) {
        LOGE("pollTread is null");
        return;
    }

    if (pollThread_->joinable()) {
        LOGI("pollThread->joinable is true");
        pollThread_->join();
    }
    pollThread_ = nullptr;
    LOGI("pollTread exit ok");
}

void KernelTalker::PollRun()
{
    struct pollfd fileFd;
    int cmdFd = -1;

    LOGI("entry");
    auto spt = mountPoint_.lock();
    if (spt == nullptr) {
        LOGE("mountPoint is not exist! bad weak_ptr");
        return;
    }
    string ctrlPath = spt->GetMountArgument().GetCtrlPath();
    LOGI("Open node file ctrl path %{public}s", ctrlPath.c_str());
    char *realPath = realpath(ctrlPath.c_str(), nullptr);
    if (realPath == nullptr) {
        return;
    }
    cmdFd = open(realPath, O_RDWR);
    free(realPath);
    if (cmdFd < 0) {
        LOGE("Open node file error %{public}d, ctrl path %{public}s", errno, ctrlPath.c_str());
        return;
    }

    LOGI("Open node file success");

    while (isRunning_) {
        fileFd.fd = cmdFd;
        fileFd.events = POLLPRI;
        fileFd.revents = 0;
        int ret = poll(&fileFd, 1, POLL_TIMEOUT_MS);
        switch (ret) {
            case NONE_EVENT:
                LOGI("none event, poll exit");
                break;
            case TIME_OUT_EVENT:
                break;
            case READ_EVENT:
                HandleAllNotify(cmdFd);
                break;
            default:
                LOGI("poll exit");
        }
    }
    close(cmdFd);
    LOGI("exit");
    return;
}

void KernelTalker::HandleAllNotify(int fd)
{
    NotifyParam param;

    while (isRunning_) {
        lseek(fd, 0, SEEK_SET);
        param.notify = NOTIFY_NONE;
        int readSize = read(fd, &param, sizeof(NotifyParam));
        if ((readSize < (int)sizeof(NotifyParam)) || (param.notify == NOTIFY_NONE)) {
            return;
        }
        NotifyHandler(param);
    }
}

void KernelTalker::NotifyHandler(NotifyParam &param)
{
    int cmd = param.notify;
    string cidStr(param.remoteCid, CID_MAX_LEN);
    switch (cmd) {
        case NOTIFY_GET_SESSION:
            GetSessionCallback_(param);
            break;
        case NOTIFY_OFFLINE:
            LOGI("NOTIFY_OFFLINE, remote cid %{public}s", cidStr.c_str());
            CloseSessionCallback_(cidStr);
            break;
        default:
            LOGI("cmd %{public}d not support now", cmd);
            break;
    }
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
