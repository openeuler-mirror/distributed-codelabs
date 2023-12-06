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

#ifndef TIME_TICK_MONITOR_H
#define TIME_TICK_MONITOR_H

#include "db_types.h"
#include "macro_utils.h"
#include "runtime_context.h"
#include "platform_specific.h"


namespace DistributedDB {
class TimeTickMonitor final {
public:
    TimeTickMonitor();
    ~TimeTickMonitor();

    DISABLE_COPY_ASSIGN_MOVE(TimeTickMonitor);

    // Start the TimeTickMonitor
    int StartTimeTickMonitor();

    // Stop the TimeTickMonitor
    void StopTimeTickMonitor();

    // Register a time changed lister, it will be callback when local time changed.
    NotificationChain::Listener *RegisterTimeChangedLister(const TimeChangedAction &action, int &errCode);

    // Notify TIME_CHANGE_EVENT.
    void NotifyTimeChange(TimeOffset offset) const;

    bool EmptyListener() const;
private:
    static constexpr  uint64_t MONITOR_INTERVAL = 1 * 1000; // 1s
    static constexpr int64_t MAX_NOISE = 9 * 100 * 1000; // 900ms
    static const EventType TIME_CHANGE_EVENT = 1;
    static const uint64_t INVALID_TIMESTAMP = 0;

    // Get the current system time
    static Timestamp GetSysCurrentTime();

    // Get the Monotonic time
    static Timestamp GetMonotonicTime();

    // prepare notifier chain
    int PrepareNotifierChain();

    // Callback for the Timer
    int TimeTick(TimerId timerId);

    mutable std::mutex timeTickMonitorLock_;
    NotificationChain *timeChangedNotifier_;
    RuntimeContext *runtimeCxt_;
    TimerId monitorTimerId_ = 0;
    TimerAction monitorCallback_;
    Timestamp lastMonotonicTime_ = 0;
    Timestamp lastSystemTime_ = 0;
    bool isStarted_ = false;
};
} // namespace DistributedDB

#endif // TIME_TICK_MONITOR_H