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

/*
这段代码定义了一个名为 KvStoreSyncManager 的类，用于管理分布式键值存储的同步操作。该类使用 C++11 的多线程技术，支持添加同步操作、移除同步操作等功能，并使用定时器来管理同步操作的执行时间。

该类的主要成员函数包括：

AddSyncOperation()：添加同步操作，支持指定同步延迟时间、同步操作执行函数、同步完成回调函数等参数。
RemoveSyncOperation()：移除指定的同步操作。
GetExpireTimeRange()：根据指定的同步延迟时间计算同步操作的过期时间范围。
DoRemoveSyncingOp()：根据指定的操作预测函数，移除正在同步中的符合条件的同步操作。
RemoveSyncingOp()：移除指定序列号的正在同步中的同步操作。
AddTimer()：添加定时器，用于在指定时间点调用回调函数。
GetTimeoutSyncOps()：获取已经超时的同步操作。
DoCheckSyncingTimeout()：检查正在同步中的同步操作是否超时。
Schedule()：调度同步操作的执行时间。
该类的主要成员变量包括：

syncOpsMutex_：用于实现线程安全。
realtimeSyncingOps_：用于存储实时同步中的同步操作。
delaySyncingOps_：用于存储延迟同步中的同步操作。
scheduleSyncOps_：用于存储已经调度的同步操作。
syncScheduler_：用于执行同步操作的任务调度器。
nextScheduleTime_：下一次调度同步操作的时间。
syncOpSeq_：用于生成同步操作的序列号。
在实现中，KvStoreSyncManager 类使用了 C++11 中的多线程技术，包括 std::mutex、std::list、std::multimap、std::atomic_uint32_t、std::chrono::steady_clock 等。在执行同步操作时，会根据同步延迟时间将同步操作分为实时同步和延迟同步，并将同步操作添加到相应的同步操作列表中。同时，会根据同步延迟时间计算同步操作的过期时间范围，并添加定时器来管理同步操作的执行时间。在定时器回调函数中，会检查已经超时的同步操作，并将其移除或重新调度执行时间。当同步操作完成后，会调用同步完成回调函数。
*/

#ifndef KVSTORE_SYNC_MANAGER_H
#define KVSTORE_SYNC_MANAGER_H

#include <atomic>
#include <list>
#include <map>

#include "task_scheduler.h"
#include "kv_store_nb_delegate.h"
#include "types.h"

namespace OHOS {
namespace DistributedKv {
class API_EXPORT KvStoreSyncManager {
public:
    static constexpr uint32_t SYNC_DEFAULT_DELAY_MS = 1000;
    static constexpr uint32_t SYNC_MIN_DELAY_MS = 100;
    static constexpr uint32_t SYNC_MAX_DELAY_MS = 1000 * 3600 * 24; // 24hours
    static constexpr uint32_t SYNC_RETRY_MAX_COUNT = 3;
    static KvStoreSyncManager *GetInstance()
    {
        static KvStoreSyncManager syncManager;
        return &syncManager;
    }
    using TimePoint = std::chrono::steady_clock::time_point;
    using SyncEnd = std::function<void(const std::map<std::string, DistributedDB::DBStatus> &)>;
    using SyncFunc = std::function<Status(const SyncEnd &)>;

    struct KvSyncOperation {
        uintptr_t syncId = 0;
        uint32_t opSeq = 0;
        uint32_t delayMs = 0;
        SyncFunc syncFunc;
        SyncEnd syncEnd;
        TimePoint beginTime;
    };
    using OpPred = std::function<bool(KvSyncOperation &)>;
    Status AddSyncOperation(uintptr_t syncId, uint32_t delayMs, const SyncFunc &syncFunc, const SyncEnd &syncEnd);
    Status RemoveSyncOperation(uintptr_t syncId);

private:
    KvStoreSyncManager();
    ~KvStoreSyncManager();

    uint32_t GetExpireTimeRange(uint32_t delayMs) const;
    uint32_t DoRemoveSyncingOp(OpPred pred, std::list<KvSyncOperation> &syncingOps);
    Status RemoveSyncingOp(uint32_t opSeq, std::list<KvSyncOperation> &syncingOps);
    void AddTimer(const TimePoint &expireTime);
    bool GetTimeoutSyncOps(const TimePoint &time, std::list<KvSyncOperation> &syncOps);
    void DoCheckSyncingTimeout(std::list<KvSyncOperation> &syncingOps);
    void Schedule(const TimePoint &expireTime);

    static constexpr uint32_t SYNCING_TIMEOUT_MS = 5000;
    static constexpr uint32_t REALTIME_PRIOR_SYNCING_MS = 300;
    static constexpr uint32_t DELAY_TIME_RANGE_DIVISOR = 4;

    mutable std::mutex syncOpsMutex_;
    std::list<KvSyncOperation> realtimeSyncingOps_;
    std::list<KvSyncOperation> delaySyncingOps_;
    std::multimap<TimePoint, KvSyncOperation> scheduleSyncOps_;

    TaskScheduler syncScheduler_ { "sync_mgr" };
    TimePoint nextScheduleTime_;
    std::atomic_uint32_t syncOpSeq_ = 0;
};
} // namespace DistributedKv
} // namespace OHOS
#endif // KVSTORE_SYNC_MANAGER_H
