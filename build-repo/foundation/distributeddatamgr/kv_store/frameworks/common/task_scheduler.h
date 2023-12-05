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

/*
这段代码定义了一个名为 TaskScheduler 的类，用于执行定时任务。该类使用 C++11 的多线程技术，支持在指定的时间点执行任务、周期性地执行任务、在指定时间间隔内重复执行任务等功能。

该类的主要成员函数包括：

At()：在指定时间点执行任务，并返回该任务的 ID。
Reset()：重设指定任务的执行时间，并返回该任务的 ID。
Every()：周期性地执行任务，支持指定时间间隔、延迟执行、执行次数等参数。
Remove()：移除指定的任务。
Execute()：立即执行任务，并返回该任务的 ID。
Clean()：清空所有任务。
该类的主要成员变量包括：

isRunning_：标识任务调度器是否正在运行。
capacity_：任务调度器可以容纳的最大任务数量。
tasks_：用于存储所有待执行的任务，并按照执行时间排序。
indexes_：用于存储每个任务的 ID 与其在 tasks_ 中的迭代器之间的映射关系。
mutex_：用于实现线程安全。
thread_：用于执行任务的线程。
condition_：用于实现条件变量，用于线程等待任务执行的通知。
taskId_：用于生成任务的唯一 ID。
在实现中，TaskScheduler 类使用了 C++11 中的多线程技术，包括 std::thread、std::mutex、std::unique_lock、std::condition_variable 等。在任务调度器的主循环中，使用 condition_.wait() 等待有任务需要执行，当有任务需要执行时，通过 tasks_.begin() 来获取最近需要执行的任务，并在执行完毕后将其从 tasks_ 中删除。同时，通过 indexes_ 来记录每个任务在 tasks_ 中的位置，以便于修改任务的执行时间、移除任务等操作。
*/

#ifndef OHOS_DISTRIBUTED_DATA_FRAMEWORKS_COMMON_TASK_SCHEDULER_H
#define OHOS_DISTRIBUTED_DATA_FRAMEWORKS_COMMON_TASK_SCHEDULER_H
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <thread>

#include "visibility.h"
namespace OHOS {
class API_LOCAL TaskScheduler {
public:
    using TaskId = uint64_t;
    using Time = std::chrono::steady_clock::time_point;
    using Duration = std::chrono::steady_clock::duration;
    using Clock = std::chrono::steady_clock;
    using Task = std::function<void()>;
    inline static constexpr TaskId INVALID_TASK_ID = static_cast<uint64_t>(0l);
    TaskScheduler(size_t capacity, const std::string &name)
    {
        capacity_ = capacity;
        isRunning_ = true;
        taskId_ = INVALID_TASK_ID;
        thread_ = std::make_unique<std::thread>([this, name]() {
            auto realName = std::string("scheduler_") + name;
            pthread_setname_np(pthread_self(), realName.c_str());
            Loop();
        });
    }
    TaskScheduler(const std::string &name) : TaskScheduler(std::numeric_limits<size_t>::max(), name) {}
    TaskScheduler(size_t capacity = std::numeric_limits<size_t>::max()) : TaskScheduler(capacity, "") {}

    ~TaskScheduler()
    {
        isRunning_ = false;
        Clean();
        At(std::chrono::steady_clock::now(), []() {});
        thread_->join();
    }

    // execute task at specific time
    TaskId At(const Time &time, Task task)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (tasks_.size() >= capacity_) {
            return INVALID_TASK_ID;
        }
        auto taskId = GenTaskId();
        auto it = tasks_.insert({ time, std::pair{ task, taskId } });
        if (it == tasks_.begin()) {
            condition_.notify_one();
        }
        indexes_[taskId] = it;
        return taskId;
    }

    TaskId Reset(TaskId taskId, const Duration &interval)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        auto index = indexes_.find(taskId);
        if (index == indexes_.end()) {
            return INVALID_TASK_ID;
        }

        auto it = tasks_.insert({ std::chrono::steady_clock::now() + interval, std::move(index->second->second) });
        if (it == tasks_.begin() || index->second == tasks_.begin()) {
            condition_.notify_one();
        }
        tasks_.erase(index->second);
        indexes_[taskId] = it;
        return taskId;
    }

    void Clean()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        indexes_.clear();
        tasks_.clear();
    }

    // execute task periodically with duration
    void Every(Duration interval, Task task)
    {
        std::function<void()> waitFunc = [this, interval, task]() {
            task();
            this->Every(interval, task);
        };
        At(std::chrono::steady_clock::now() + interval, waitFunc);
    }

    // remove task in SchedulerTask
    void Remove(TaskId taskId)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        auto index = indexes_.find(taskId);
        if (index == indexes_.end()) {
            return;
        }
        tasks_.erase(index->second);
        indexes_.erase(index);
        condition_.notify_one();
    }

    // execute task periodically with duration after delay
    void Every(Duration delay, Duration interval, Task task)
    {
        std::function<void()> waitFunc = [this, interval, task]() {
            task();
            Every(interval, task);
        };
        At(std::chrono::steady_clock::now() + delay, waitFunc);
    }

    // execute task for some times periodically with duration after delay
    void Every(int32_t times, Duration delay, Duration interval, Task task)
    {
        std::function<void()> waitFunc = [this, times, interval, task]() {
            task();
            int count = times;
            count--;
            if (times > 1) {
                Every(count, interval, interval, task);
            }
        };

        At(std::chrono::steady_clock::now() + delay, waitFunc);
    }

    TaskId Execute(Task task)
    {
        return At(std::chrono::steady_clock::now(), std::move(task));
    }

private:
    using InnerTask = std::pair<std::function<void()>, uint64_t>;
    void Loop()
    {
        while (isRunning_) {
            std::function<void()> exec;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                condition_.wait(lock, [this] {
                    return !tasks_.empty();
                });
                if (tasks_.begin()->first > std::chrono::steady_clock::now()) {
                    auto time = tasks_.begin()->first;
                    condition_.wait_until(lock, time);
                    continue;
                }
                auto it = tasks_.begin();
                exec = it->second.first;
                indexes_.erase(it->second.second);
                tasks_.erase(it);
            }

            if (exec) {
                exec();
            }
        }
    }

    TaskId GenTaskId()
    {
        auto taskId = ++taskId_;
        if (taskId == INVALID_TASK_ID) {
            return ++taskId_;
        }
        return taskId;
    }

    volatile bool isRunning_;
    size_t capacity_;
    std::multimap<Time, InnerTask> tasks_;
    std::map<TaskId, decltype(tasks_)::iterator> indexes_;
    std::mutex mutex_;
    std::unique_ptr<std::thread> thread_;
    std::condition_variable condition_;
    std::atomic<uint64_t> taskId_;
};
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_DATA_FRAMEWORKS_COMMON_TASK_SCHEDULER_H
