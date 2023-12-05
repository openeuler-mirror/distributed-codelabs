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

#ifndef DFSU_THREAD_SAFE_QUEUE_H
#define DFSU_THREAD_SAFE_QUEUE_H

#include <algorithm>
#include <deque>

#include "dfsu_exception.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
/**
 * @brief  A Thread-safe Queue.
 *
 * Design choices:
 * 1) unlimited capacity
 * 2) throw exception to indicate failues
 * 3) blocking pop interface
 * 3) hatlable
 *
 * @tparam T Any type. Aggregate data type is prefered
 *
 */
template<typename T>
class DfsuThreadSafeQueue {
public:
    void Push(std::unique_ptr<T> pt)
    {
        if (!pt) {
            ThrowException(ERR_UTILS_ACTOR_INVALID_CMD, "Push an empty cmd");
        }
        std::unique_lock<std::mutex> lock(mutex_);
        queue_.emplace_back(std::move(pt));
        cv_.notify_one();
    }

    void PushFront(std::unique_ptr<T> pt)
    {
        if (!pt) {
            ThrowException(ERR_UTILS_ACTOR_INVALID_CMD, "Push an empty cmd");
        }
        std::unique_lock<std::mutex> lock(mutex_);
        queue_.emplace_front(std::move(pt));
        cv_.notify_one();
    }

    std::unique_ptr<T> WaitAndPop()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [&] { return !queue_.empty() || halted; });
        if (halted && queue_.empty()) {
            ThrowException(ERR_UTILS_ACTOR_QUEUE_STOP, "Queue was halted");
        }

        auto res = std::move(queue_.front());
        queue_.pop_front();
        return std::move(res);
    }

    void ForEach(std::function<void(const std::unique_ptr<T> &)> executor)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        std::for_each(queue_.begin(), queue_.end(), executor);
    }

    void Halt()
    {
        halted = true;
        cv_.notify_all();
    }

private:
    std::deque<std::unique_ptr<T>> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;

    bool halted {false};
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // DFSU_THREAD_SAFE_QUEUE_H