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
#ifndef KERNEL_TALKER_H
#define KERNEL_TALKER_H

#include <atomic>
#include <fcntl.h>
#include <functional>
#include <memory>
#include <mutex>
#include <poll.h>
#include <thread>
#include <unistd.h>
#include <unordered_set>

#include "mountpoint/mount_point.h"
#include "network/base_session.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
constexpr int CID_MAX_LEN = 64;
struct NotifyParam {
    int32_t notify;
    int32_t fd;
    uint16_t udpPort;
    uint8_t deviceType;
    int32_t flag;
    int32_t reserved;
    char remoteCid[CID_MAX_LEN];
} __attribute__((packed));

class KernelTalker final : protected NoCopyable, public std::enable_shared_from_this<KernelTalker> {
public:
    explicit KernelTalker(std::weak_ptr<MountPoint> mountPoint,
                          std::function<void(NotifyParam &)> getSessionCallback,
                          std::function<void(const std::string &)> closeSessionCallback)
        : mountPoint_(mountPoint), GetSessionCallback_(getSessionCallback), CloseSessionCallback_(closeSessionCallback)
    {
    }
    KernelTalker() = default;
    ~KernelTalker() = default;

    void SinkSessionTokernel(std::shared_ptr<BaseSession> session);
    void SinkDevslTokernel(const std::string &cid, uint32_t devsl);
    void SinkOfflineCmdToKernel(std::string cid);

    void CreatePollThread();
    void WaitForPollThreadExited();

private:
    template<typename T>
    void SetCmd(T &cmd)
    {
        auto spt = mountPoint_.lock();
        if (spt == nullptr) {
            LOGE("mountPoint is not exist! bad weak_ptr");
            return;
        }
        std::string ctrlPath = spt->GetMountArgument().GetCtrlPath();
        LOGI("cmd path:%{public}s", ctrlPath.c_str());
        std::lock_guard<std::mutex> lock(cmdMutex_);

        char *realPath = realpath(ctrlPath.c_str(), nullptr);
        if (realPath == nullptr) {
            return;
        }

        int file = open(realPath, O_RDWR);
        free(realPath);
        if (file < 0) {
            LOGE("Open node file error. %{public}d", errno);
            return;
        }
        int err = write(file, &cmd, sizeof(T));
        if (err < 0) {
            LOGE("write return err. %{public}d", errno);
        }
        close(file);
    }

    void PollRun();
    void HandleAllNotify(int fd);
    void NotifyHandler(NotifyParam &param);

    std::weak_ptr<MountPoint> mountPoint_;
    std::mutex cmdMutex_;
    std::atomic<bool> isRunning_ {true};
    std::unique_ptr<std::thread> pollThread_ {nullptr};
    std::function<void(NotifyParam &)> GetSessionCallback_ {nullptr};
    std::function<void(const std::string &)> CloseSessionCallback_ {nullptr};
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // KERNEL_TALKER_H