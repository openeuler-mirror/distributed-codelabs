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

#ifndef SYNC_STATE_MACHINE_H
#define SYNC_STATE_MACHINE_H

#include <mutex>

#include "isync_interface.h"
#include "isync_state_machine.h"

namespace DistributedDB {
// the 1st uint8_t is event, the 2nd uint8_t is out state
using EventToState = std::map<uint8_t, uint8_t>;

// The StateSwitchTable with the SyncProctolVersion
struct StateSwitchTable {
    uint32_t version = 0;
    std::map<uint8_t, EventToState> switchTable; // the 1st uint8_t is current state
};

struct WatchDogController {
    TimerId feedDogTimerId = 0;
    uint8_t feedDogCnt = 0;
    uint8_t feedDogUpperLimit = 0;
    /* this variable will +1 when call StartFeedDogForSync, -1 when recv one ack,
    when it become <= 0, we stop the watch dog. */
    int refCount = 0;
};

class SyncStateMachine : public ISyncStateMachine {
public:
    SyncStateMachine();
    ~SyncStateMachine() override;

    // Init the SingleVerSyncStateMachine
    int Initialize(ISyncTaskContext *context, ISyncInterface *syncInterface, std::shared_ptr<Metadata> &metadata,
        ICommunicator *communicator) override;

    // start a sync step
    int StartSync() override;

    // call when timeout
    int TimeoutCallback(TimerId timerId) override;

    // Force stop the state machine
    void Abort() override;

    // Force stop the state machine now
    void AbortImmediately() override;

    // Force stop current task with sessionId
    void InnerErrorAbort(uint32_t sessionId) override;

    // start a timer to ResetWatchDog when sync data one (key,value) size bigger than mtu
    bool StartFeedDogForSync(uint32_t time, SyncDirectionFlag flag) override;

    uint8_t GetFeedDogTimeout(int timeoutCount) const;

    // stop timer to ResetWatchDog when sync data one (key,value) size bigger than mtu
    void StopFeedDogForSync(SyncDirectionFlag flag) override;

    // start a timer to ResetWatchDog when get data and send notify ack if need
    void StartFeedDogForGetData(uint32_t sessionId) override;

    // start a timer to ResetWatchDog when get data and stop send notify ack if need
    void StopFeedDogForGetData() override;

    // Notify machine is closing, should release some lock
    void NotifyClosing() override;
protected:

    // SyncOperation is timeout, step to timeout state
    virtual void StepToTimeout(TimerId timerId) = 0;

    // Step the SingleVerSyncStateMachine
    virtual void SyncStep() = 0;

    // Called by SyncStep, Sub class should realize this function to do machine step
    virtual void SyncStepInnerLocked() = 0;

    // Do state machine step with no lock, for inner use
    virtual void SyncStepInner() = 0;

    // Called by StartSync, Sub class should realize this function to start statemachine
    virtual int StartSyncInner() = 0;

    // Called by Abort, Sub class should realize this function to force abort statemachine
    virtual void AbortInner() = 0;

    // while currentstate could not be found, should called, Sub class should realize this function.
    virtual void SetCurStateErrStatus();

    // Used to get instance class' stateSwitchTables
    virtual const std::vector<StateSwitchTable> &GetStateSwitchTables() const = 0;

    // Called by ExecNextTask, Sub class should realize this function to do some thing for run next sync task
    virtual int PrepareNextSyncTask() = 0;

    // Called by StartSaveDataNotifyTimer, Sub class should realize this function to send a heartbeet packet
    virtual void SendNotifyPacket(uint32_t sessionId, uint32_t sequenceId, uint32_t inMsgId) = 0;

    // Used to parse state table to switch machine state, this function must be called in stateMachineLock
    int SwitchMachineState(uint8_t event);

    // Do state switch with the event, and do syncstep
    virtual void SwitchStateAndStep(uint8_t event);

    // To Exec next sync task in context targetQueue
    int ExecNextTask();

    // Start a watchdog used for manual sync, when begin a manual sync
    int StartWatchDog();

    // Reset the watchdog used for manual sync
    int ResetWatchDog();

    // stop a watchdog used for manual sync, call when sync finished,
    void StopWatchDog();

    // Start a timer to send data notify packet to keep remote device not timeout
    bool StartSaveDataNotify(uint32_t sessionId, uint32_t sequenceId, uint32_t inMsgId);

    // Stop send save data notify
    void StopSaveDataNotify();

    // Stop send save data notify without lock
    void StopSaveDataNotifyNoLock();

    // stop a timer to ResetWatchDog when sync data bigger than mtu  without lock
    void StopFeedDogForSyncNoLock(SyncDirectionFlag flag);

    void DecRefCountOfFeedDogTimer(SyncDirectionFlag flag);

    virtual void DoSaveDataNotify(uint32_t sessionId, uint32_t sequenceId, uint32_t inMsgId);

    void DoFeedDogForSync(SyncDirectionFlag flag);

    void DoGetAndSendDataNotify(uint32_t sessionId);

    void StopFeedDogForGetDataInner(TimerId timerId);

    DISABLE_COPY_ASSIGN_MOVE(SyncStateMachine);

    ISyncTaskContext *syncContext_;
    ISyncInterface *storageInterface_;
    ICommunicator *communicator_;
    std::shared_ptr<Metadata> metadata_;
    std::mutex stateMachineLock_;
    uint8_t currentState_;
    bool watchDogStarted_;
    uint32_t currentSyncProctolVersion_;

    // For save data notify
    static const int DATA_NOTIFY_INTERVAL = 2000; // 2s for save/get data notify
    static const int MAX_DATA_NOTIFY_COUNT = 15; // only notify 15 times
    static const int SYNC_DIRECTION_NUM = 2; // send receive
    std::mutex saveDataNotifyLock_;
    TimerId saveDataNotifyTimerId_;
    uint8_t saveDataNotifyCount_;

    std::mutex getDataNotifyLock_;
    TimerId getDataNotifyTimerId_;
    uint8_t getDataNotifyCount_;

    // used for one (key,value) bigger than mtu size, in this case, send packet need more longger time
    std::mutex feedDogLock_[SYNC_DIRECTION_NUM];
    WatchDogController watchDogController_[SYNC_DIRECTION_NUM] = {{0}, {0}};
};
} // namespace DistributedDB
#endif // SYNC_STATE_MACHINE_H
