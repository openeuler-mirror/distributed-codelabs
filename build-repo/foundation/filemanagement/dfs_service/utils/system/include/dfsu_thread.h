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
#ifndef DFSU_THREAD_H
#define DFSU_THREAD_H

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Utils {
class DfsuThread {
public:
    DfsuThread() = default;
    DfsuThread(const DfsuThread &) = delete;
    DfsuThread &operator=(const DfsuThread &) = delete;

    ~DfsuThread()
    {
        Stop();
    }

    template<class _Fn, class... _Args>
    bool Run(_Fn &&_Fx, _Args &&..._Ax)
    {
        std::unique_lock<std::mutex> lock(threadMutex_);
        if (thread_ != nullptr) {
            return false;
        }
        running_ = true;
        thread_ = std::make_unique<std::thread>(std::forward<_Fn>(_Fx), std::forward<_Args>(_Ax)...);
        return true;
    }

    bool RunLoop(std::function<bool()> task, uint64_t interval, uint32_t retryTimes = UINT32_MAX)
    {
        std::unique_lock<std::mutex> lock(threadMutex_);
        if (thread_ != nullptr) {
            return false;
        }
        running_ = true;
        thread_ = std::make_unique<std::thread>([this, task, interval, retryTimes] {
            uint32_t times = retryTimes;
            LOGD("DfsThread: entering loop");
            while ((!task()) && (times > 0)) {
                times--;
                std::unique_lock<std::mutex> lock(sleepMutex_);
                bool stop =
                    sleepCv_.wait_for(lock, std::chrono::milliseconds(interval), [this]() { return !this->running_; });
                if (stop) { // is stopped
                    break;
                }
            }
            LOGD("DfsThread: leaving loop");
        });
        return true;
    }

    bool RunLoopFlexible(std::function<bool(uint64_t &)> task, uint64_t interval, uint32_t retryTimes = UINT32_MAX)
    {
        std::unique_lock<std::mutex> lock(threadMutex_);
        if (thread_ != nullptr) {
            return false;
        }
        running_ = true;
        thread_ = std::make_unique<std::thread>([this, task, interval, retryTimes] {
            uint32_t times = retryTimes;
            uint64_t duration = interval;
            LOGD("DfsThread: entering flexible loop");
            while ((!task(duration)) && (times > 0)) {
                times--;
                std::unique_lock<std::mutex> lock(sleepMutex_);
                bool stop =
                    sleepCv_.wait_for(lock, std::chrono::milliseconds(duration), [this]() { return !this->running_; });
                if (stop) { // is stopped
                    break;
                }
            }
            LOGD("DfsThread: leaving flexible loop");
        });
        return true;
    }

    bool Stop()
    {
        std::unique_lock<std::mutex> lockThread(threadMutex_);
        if (thread_ == nullptr) {
            return true;
        }
        {
            std::unique_lock<std::mutex> lockSleep(sleepMutex_);
            running_ = false;
            sleepCv_.notify_one();
        }
        LOGD("wait thread to stop");
        if (thread_->joinable()) {
            thread_->join();
        }
        thread_ = nullptr;
        return true;
    }

    bool operator==(std::thread::id id)
    {
        if (thread_ == nullptr) {
            return false;
        }
        return thread_->get_id() == id;
    }

private:
    std::atomic_bool running_ {false};
    std::mutex threadMutex_ {};
    std::unique_ptr<std::thread> thread_ {nullptr};
    std::mutex sleepMutex_ {};
    std::condition_variable sleepCv_ {};
};
} // namespace Utils
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // DFSU_THREAD_H