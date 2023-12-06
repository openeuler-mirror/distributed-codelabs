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

#include "sync_state_machine.h"

#include <algorithm>

#include "log_print.h"
#include "version.h"

namespace DistributedDB {
SyncStateMachine::SyncStateMachine()
    : syncContext_(nullptr),
      storageInterface_(nullptr),
      communicator_(nullptr),
      metadata_(nullptr),
      currentState_(0),
      watchDogStarted_(false),
      currentSyncProctolVersion_(SINGLE_VER_SYNC_PROCTOL_V3),
      saveDataNotifyTimerId_(0),
      saveDataNotifyCount_(0),
      getDataNotifyTimerId_(0),
      getDataNotifyCount_(0)
{
}

SyncStateMachine::~SyncStateMachine()
{
    syncContext_ = nullptr;
    storageInterface_ = nullptr;
    watchDogStarted_ = false;
    metadata_ = nullptr;
    if (communicator_ != nullptr) {
        RefObject::DecObjRef(communicator_);
        communicator_ = nullptr;
    }
}

int SyncStateMachine::Initialize(ISyncTaskContext *context, ISyncInterface *syncInterface,
    std::shared_ptr<Metadata> &metadata, ICommunicator *communicator)
{
    if ((context == nullptr) || (syncInterface == nullptr) || (metadata == nullptr) || (communicator == nullptr)) {
        return -E_INVALID_ARGS;
    }
    syncContext_ = context;
    storageInterface_ = syncInterface;
    metadata_ = metadata;
    RefObject::IncObjRef(communicator);
    communicator_ = communicator;
    return E_OK;
}

int SyncStateMachine::StartSync()
{
    int errCode = syncContext_->IncUsedCount();
    if (errCode != E_OK) {
        return errCode;
    }
    std::lock_guard<std::mutex> lock(stateMachineLock_);
    errCode = StartSyncInner();
    syncContext_->SafeExit();
    return errCode;
}

int SyncStateMachine::TimeoutCallback(TimerId timerId)
{
    RefObject::AutoLock lock(syncContext_);
    if (syncContext_->IsKilled()) {
        return -E_OBJ_IS_KILLED;
    }
    TimerId timer = syncContext_->GetTimerId();
    if (timer != timerId) {
        return -E_UNEXPECTED_DATA;
    }

    int retryTime = syncContext_->GetRetryTime();
    if (retryTime >= syncContext_->GetSyncRetryTimes() || !syncContext_->IsSyncTaskNeedRetry()) {
        LOGI("[SyncStateMachine][Timeout] TimeoutCallback retryTime:%d", retryTime);
        syncContext_->UnlockObj();
        StepToTimeout(timerId);
        syncContext_->LockObj();
        return E_OK;
    }
    retryTime++;
    syncContext_->SetRetryTime(retryTime);
    // the sequenceid will be managed by dataSync slide windows.
    syncContext_->SetRetryStatus(SyncTaskContext::NEED_RETRY);
    int timeoutTime = syncContext_->GetSyncRetryTimeout(retryTime);
    syncContext_->ModifyTimer(timeoutTime);
    LOGI("[SyncStateMachine][Timeout] Schedule task, timeoutTime = %d, retryTime = %d", timeoutTime, retryTime);
    SyncStep();
    return E_OK;
}

void SyncStateMachine::Abort()
{
    RefObject::IncObjRef(syncContext_);
    int errCode = RuntimeContext::GetInstance()->ScheduleTask([this]() {
        this->AbortImmediately();
        RefObject::DecObjRef(this->syncContext_);
    });
    if (errCode != E_OK) {
        LOGE("[SyncStateMachine][Abort] Abort failed, errCode %d", errCode);
        RefObject::DecObjRef(syncContext_);
    }
}

void SyncStateMachine::AbortImmediately()
{
    std::lock_guard<std::mutex> lock(stateMachineLock_);
    AbortInner();
    StopWatchDog();
    currentState_ = 0;
}

int SyncStateMachine::SwitchMachineState(uint8_t event)
{
    const std::vector<StateSwitchTable> &tables = GetStateSwitchTables();
    auto tableIter = std::find_if(tables.begin(), tables.end(),
        [this](const StateSwitchTable &table) {
            return table.version <= currentSyncProctolVersion_;
        });
    if (tableIter == tables.end()) {
        LOGE("[SyncStateMachine][SwitchState] Can't find a compatible version by version %u",
            currentSyncProctolVersion_);
        return -E_NOT_FOUND;
    }

    const std::map<uint8_t, EventToState> &table = (*tableIter).switchTable;
    auto eventToStateIter = table.find(currentState_);
    if (eventToStateIter == table.end()) {
        LOGE("[SyncStateMachine][SwitchState] tableVer:%d, Can't find EventToState with currentSate %u",
            (*tableIter).version, currentState_);
        SetCurStateErrStatus();
        return E_OK;
    }

    const EventToState &eventToState = eventToStateIter->second;
    auto stateIter = eventToState.find(event);
    if (stateIter == eventToState.end()) {
        LOGD("[SyncStateMachine][SwitchState] tableVer:%d, Can't find event %u int currentSate %u ignore",
            (*tableIter).version, event, currentState_);
        return -E_NOT_FOUND;
    }

    currentState_ = stateIter->second;
    LOGD("[SyncStateMachine][SwitchState] tableVer:%d, from state %u move to state %u with event %u dev %s{private}",
        (*tableIter).version, eventToStateIter->first, currentState_, event, syncContext_->GetDeviceId().c_str());
    return E_OK;
}

void SyncStateMachine::SwitchStateAndStep(uint8_t event)
{
    if (SwitchMachineState(event) == E_OK) {
        SyncStepInner();
    }
}

int SyncStateMachine::ExecNextTask()
{
    syncContext_->Clear();
    while (!syncContext_->IsTargetQueueEmpty()) {
        int errCode = syncContext_->GetNextTarget(false);
        if (errCode != E_OK) {
            continue;
        }
        if (syncContext_->IsCurrentSyncTaskCanBeSkipped()) {
            syncContext_->SetOperationStatus(SyncOperation::OP_FINISHED_ALL);
            continue;
        }
        errCode = PrepareNextSyncTask();
        if (errCode != E_OK) {
            LOGE("[SyncStateMachine] PrepareSync failed");
            syncContext_->SetOperationStatus(SyncOperation::OP_FAILED);
        }
        return errCode;
    }
    syncContext_->SetTaskExecStatus(ISyncTaskContext::FINISHED);
    // no task left
    LOGD("[SyncStateMachine] All sync task finished!");
    return -E_NO_SYNC_TASK;
}

int SyncStateMachine::StartWatchDog()
{
    int errCode = syncContext_->StartTimer();
    if (errCode == E_OK) {
        watchDogStarted_ = true;
    }
    return errCode;
}

int SyncStateMachine::ResetWatchDog()
{
    if (!watchDogStarted_) {
        return E_OK;
    }
    LOGD("[SyncStateMachine][WatchDog] ResetWatchDog.");
    syncContext_->StopTimer();
    syncContext_->SetRetryTime(0);
    return syncContext_->StartTimer();
}

void SyncStateMachine::StopWatchDog()
{
    watchDogStarted_ = false;
    LOGD("[SyncStateMachine][WatchDog] StopWatchDog.");
    syncContext_->StopTimer();
}

bool SyncStateMachine::StartSaveDataNotify(uint32_t sessionId, uint32_t sequenceId, uint32_t inMsgId)
{
    std::lock_guard<std::mutex> lockGuard(saveDataNotifyLock_);
    if (saveDataNotifyTimerId_ > 0) {
        saveDataNotifyCount_ = 0;
        LOGW("[SyncStateMachine][SaveDataNotify] timer has been started!");
        return false;
    }

    // Incref to make sure context still alive before timer stopped.
    RefObject::IncObjRef(syncContext_);
    int errCode = RuntimeContext::GetInstance()->SetTimer(
        DATA_NOTIFY_INTERVAL,
        [this, sessionId, sequenceId, inMsgId](TimerId timerId) {
            RefObject::IncObjRef(syncContext_);
            int ret = RuntimeContext::GetInstance()->ScheduleTask([this, sessionId, sequenceId, inMsgId]() {
                DoSaveDataNotify(sessionId, sequenceId, inMsgId);
                RefObject::DecObjRef(syncContext_);
            });
            if (ret != E_OK) {
                LOGE("[SyncStateMachine] [DoSaveDataNotify] ScheduleTask failed errCode %d", ret);
                RefObject::DecObjRef(syncContext_);
            }
            return ret;
        },
        [this]() { RefObject::DecObjRef(syncContext_); },
        saveDataNotifyTimerId_);
    if (errCode != E_OK) {
        LOGW("[SyncStateMachine][SaveDataNotify] start timer failed err %d !", errCode);
        return false;
    }
    return true;
}

void SyncStateMachine::StopSaveDataNotify()
{
    std::lock_guard<std::mutex> lockGuard(saveDataNotifyLock_);
    StopSaveDataNotifyNoLock();
}

void SyncStateMachine::StopSaveDataNotifyNoLock()
{
    if (saveDataNotifyTimerId_ == 0) {
        LOGI("[SyncStateMachine][SaveDataNotify] timer is not started!");
        return;
    }
    RuntimeContext::GetInstance()->RemoveTimer(saveDataNotifyTimerId_);
    saveDataNotifyTimerId_ = 0;
    saveDataNotifyCount_ = 0;
}

bool SyncStateMachine::StartFeedDogForSync(uint32_t time, SyncDirectionFlag flag)
{
    if (flag != SyncDirectionFlag::SEND && flag != SyncDirectionFlag::RECEIVE) {
        LOGE("[SyncStateMachine][feedDog] start wrong flag:%d", flag);
        return false;
    }

    uint8_t cnt = GetFeedDogTimeout(time / DATA_NOTIFY_INTERVAL);
    LOGI("[SyncStateMachine][feedDog] start cnt:%d, flag:%d", cnt, flag);

    std::lock_guard<std::mutex> lockGuard(feedDogLock_[flag]);
    watchDogController_[flag].refCount++;
    LOGD("af incr refCount = %d", watchDogController_[flag].refCount);

    if (watchDogController_[flag].feedDogTimerId > 0) {
        // update the upperLimit, if the new cnt is bigger then last upperLimit
        if (cnt > watchDogController_[flag].feedDogUpperLimit) {
            LOGD("update feedDogUpperLimit = %d", cnt);
            watchDogController_[flag].feedDogUpperLimit = cnt;
        }
        watchDogController_[flag].feedDogCnt = 0u;
        LOGW("[SyncStateMachine][feedDog] timer has been started!, flag:%d", flag);
        return false;
    }

    // Incref to make sure context still alive before timer stopped.
    RefObject::IncObjRef(syncContext_);
    watchDogController_[flag].feedDogUpperLimit = cnt;
    int errCode = RuntimeContext::GetInstance()->SetTimer(
        DATA_NOTIFY_INTERVAL,
        [this, flag](TimerId timerId) {
            RefObject::IncObjRef(syncContext_);
            int ret = RuntimeContext::GetInstance()->ScheduleTask([this, flag]() {
                DoFeedDogForSync(flag);
                RefObject::DecObjRef(syncContext_);
            });
            if (ret != E_OK) {
                LOGE("[SyncStateMachine] [DoFeedDogForSync] ScheduleTask failed errCode %d", ret);
                RefObject::DecObjRef(syncContext_);
            }
            return ret;
        },
        [this]() { RefObject::DecObjRef(syncContext_); },
        watchDogController_[flag].feedDogTimerId);
    if (errCode != E_OK) {
        LOGW("[SyncStateMachine][feedDog] start timer failed err %d !", errCode);
        return false;
    }
    return true;
}

uint8_t SyncStateMachine::GetFeedDogTimeout(int timeoutCount) const
{
    if (timeoutCount > UINT8_MAX) {
        return UINT8_MAX;
    }
    return timeoutCount;
}

void SyncStateMachine::StopFeedDogForSync(SyncDirectionFlag flag)
{
    if (flag != SyncDirectionFlag::SEND && flag != SyncDirectionFlag::RECEIVE) {
        LOGE("[SyncStateMachine][feedDog] stop wrong flag:%d", flag);
        return;
    }
    std::lock_guard<std::mutex> lockGuard(feedDogLock_[flag]);
    StopFeedDogForSyncNoLock(flag);
}

void SyncStateMachine::StopFeedDogForSyncNoLock(SyncDirectionFlag flag)
{
    if (flag != SyncDirectionFlag::SEND && flag != SyncDirectionFlag::RECEIVE) {
        LOGE("[SyncStateMachine][feedDog] stop wrong flag:%d", flag);
        return;
    }
    if (watchDogController_[flag].feedDogTimerId == 0) {
        return;
    }
    LOGI("[SyncStateMachine][feedDog] stop flag:%d", flag);
    RuntimeContext::GetInstance()->RemoveTimer(watchDogController_[flag].feedDogTimerId);
    watchDogController_[flag].feedDogTimerId = 0;
    watchDogController_[flag].feedDogCnt = 0;
    watchDogController_[flag].refCount = 0;
}

void SyncStateMachine::SetCurStateErrStatus()
{
}

void SyncStateMachine::DecRefCountOfFeedDogTimer(SyncDirectionFlag flag)
{
    std::lock_guard<std::mutex> lockGuard(feedDogLock_[flag]);
    if (watchDogController_[flag].feedDogTimerId == 0) {
        return;
    }
    if (--watchDogController_[flag].refCount <= 0) {
        LOGD("stop feed dog timer, refcount = %d", watchDogController_[flag].refCount);
        StopFeedDogForSyncNoLock(flag);
    }
    LOGD("af dec refcount = %d", watchDogController_[flag].refCount);
}

void SyncStateMachine::DoSaveDataNotify(uint32_t sessionId, uint32_t sequenceId, uint32_t inMsgId)
{
    {
        std::lock_guard<std::mutex> lock(stateMachineLock_);
        (void)ResetWatchDog();
    }
    std::lock_guard<std::mutex> innerLock(saveDataNotifyLock_);
    if (saveDataNotifyCount_ >= MAX_DATA_NOTIFY_COUNT) {
        StopSaveDataNotifyNoLock();
        return;
    }
    SendNotifyPacket(sessionId, sequenceId, inMsgId);
    saveDataNotifyCount_++;
}

void SyncStateMachine::DoFeedDogForSync(SyncDirectionFlag flag)
{
    {
        std::lock_guard<std::mutex> lock(stateMachineLock_);
        (void)ResetWatchDog();
    }
    std::lock_guard<std::mutex> innerLock(feedDogLock_[flag]);
    if (watchDogController_[flag].feedDogCnt >= watchDogController_[flag].feedDogUpperLimit) {
        StopFeedDogForSyncNoLock(flag);
        return;
    }
    watchDogController_[flag].feedDogCnt++;
}

void SyncStateMachine::InnerErrorAbort(uint32_t sessionId)
{
    // do nothing
    (void) sessionId;
}

void SyncStateMachine::NotifyClosing()
{
    // do nothing
}

void SyncStateMachine::StartFeedDogForGetData(uint32_t sessionId)
{
    std::lock_guard<std::mutex> lockGuard(getDataNotifyLock_);
    if (getDataNotifyTimerId_ > 0) {
        getDataNotifyCount_ = 0;
        LOGW("[SyncStateMachine][StartFeedDogForGetData] timer has been started!");
    }

    // Incref to make sure context still alive before timer stopped.
    RefObject::IncObjRef(syncContext_);
    int errCode = RuntimeContext::GetInstance()->SetTimer(
        DATA_NOTIFY_INTERVAL,
        [this, sessionId](TimerId timerId) {
            RefObject::IncObjRef(syncContext_);
            int ret = RuntimeContext::GetInstance()->ScheduleTask([this, sessionId, timerId]() {
                DoGetAndSendDataNotify(sessionId);
                int getDataNotifyCount = 0;
                {
                    std::lock_guard<std::mutex> autoLock(getDataNotifyLock_);
                    getDataNotifyCount = getDataNotifyCount_;
                }
                if (getDataNotifyCount >= MAX_DATA_NOTIFY_COUNT) {
                    StopFeedDogForGetDataInner(timerId);
                }
                RefObject::DecObjRef(syncContext_);
            });
            if (ret != E_OK) {
                LOGE("[SyncStateMachine] [StartFeedDogForGetData] ScheduleTask failed errCode %d", ret);
                RefObject::DecObjRef(syncContext_);
            }
            return ret;
        },
        [this]() { RefObject::DecObjRef(syncContext_); },
        getDataNotifyTimerId_);
    if (errCode != E_OK) {
        LOGW("[SyncStateMachine][StartFeedDogForGetData] start timer failed err %d !", errCode);
    }
}

void SyncStateMachine::StopFeedDogForGetData()
{
    TimerId timerId = 0;
    {
        std::lock_guard<std::mutex> lockGuard(getDataNotifyLock_);
        timerId = getDataNotifyTimerId_;
    }
    if (timerId == 0) {
        return;
    }
    StopFeedDogForGetDataInner(timerId);
}

void SyncStateMachine::DoGetAndSendDataNotify(uint32_t sessionId)
{
    (void)ResetWatchDog();
    std::lock_guard<std::mutex> autoLock(getDataNotifyLock_);
    if (getDataNotifyCount_ >= MAX_DATA_NOTIFY_COUNT) {
        return;
    }
    if (sessionId != 0) {
        SendNotifyPacket(sessionId, 0, DATA_SYNC_MESSAGE);
    }
    getDataNotifyCount_++;
}

void SyncStateMachine::StopFeedDogForGetDataInner(TimerId timerId)
{
    std::lock_guard<std::mutex> lockGuard(getDataNotifyLock_);
    if (getDataNotifyTimerId_ == 0 || getDataNotifyTimerId_ != timerId) {
        return;
    }
    RuntimeContext::GetInstance()->RemoveTimer(timerId);
    getDataNotifyTimerId_ = 0;
    getDataNotifyCount_ = 0;
}
} // namespace DistributedDB
