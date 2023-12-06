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

#include "sync_task_context.h"

#include <algorithm>
#include <cmath>

#include "db_constant.h"
#include "db_dump_helper.h"
#include "db_dfx_adapter.h"
#include "db_errno.h"
#include "hash.h"
#include "isync_state_machine.h"
#include "log_print.h"
#include "time_helper.h"

namespace DistributedDB {
std::mutex SyncTaskContext::synTaskContextSetLock_;
std::set<ISyncTaskContext *> SyncTaskContext::synTaskContextSet_;

namespace {
    const int NEGOTIATION_LIMIT = 2;
}

SyncTaskContext::SyncTaskContext()
    : syncOperation_(nullptr),
      syncId_(0),
      mode_(0),
      isAutoSync_(false),
      status_(0),
      taskExecStatus_(0),
      syncInterface_(nullptr),
      communicator_(nullptr),
      stateMachine_(nullptr),
      requestSessionId_(0),
      lastRequestSessionId_(0),
      timeHelper_(nullptr),
      remoteSoftwareVersion_(0),
      remoteSoftwareVersionId_(0),
      isCommNormal_(true),
      taskErrCode_(E_OK),
      syncTaskRetryStatus_(false),
      isSyncRetry_(false),
      negotiationCount_(0),
      isAutoSubscribe_(false),
      isNeedResetAbilitySync_(false)
{
}

SyncTaskContext::~SyncTaskContext()
{
    if (stateMachine_ != nullptr) {
        delete stateMachine_;
        stateMachine_ = nullptr;
    }
    ClearSyncOperation();
    ClearSyncTarget();
    syncInterface_ = nullptr;
    communicator_ = nullptr;
}

int SyncTaskContext::AddSyncTarget(ISyncTarget *target)
{
    if (target == nullptr) {
        return -E_INVALID_ARGS;
    }
    int targetMode = target->GetMode();
    {
        std::lock_guard<std::mutex> lock(targetQueueLock_);
        if (target->GetTaskType() == ISyncTarget::REQUEST) {
            requestTargetQueue_.push_back(target);
        } else if (target->GetTaskType() == ISyncTarget::RESPONSE) {
            responseTargetQueue_.push_back(target);
        } else {
            return -E_INVALID_ARGS;
        }
    }
    CancelCurrentSyncRetryIfNeed(targetMode);
    if (taskExecStatus_ == RUNNING) {
        return E_OK;
    }
    if (onSyncTaskAdd_) {
        RefObject::IncObjRef(this);
        int errCode = RuntimeContext::GetInstance()->ScheduleTask([this]() {
            onSyncTaskAdd_();
            RefObject::DecObjRef(this);
        });
        if (errCode != E_OK) {
            RefObject::DecObjRef(this);
        }
    }
    return E_OK;
}

void SyncTaskContext::SetOperationStatus(int status)
{
    std::lock_guard<std::mutex> lock(operationLock_);
    if (syncOperation_ == nullptr) {
        LOGD("[SyncTaskContext][SetStatus] syncOperation is null");
        return;
    }
    int finalStatus = status;

    int operationStatus = syncOperation_->GetStatus(deviceId_);
    if (status == SyncOperation::OP_SEND_FINISHED && operationStatus == SyncOperation::OP_RECV_FINISHED) {
        if (GetTaskErrCode() == -E_EKEYREVOKED) {
            finalStatus = SyncOperation::OP_EKEYREVOKED_FAILURE;
        } else {
            finalStatus = SyncOperation::OP_FINISHED_ALL;
        }
    } else if (status == SyncOperation::OP_RECV_FINISHED && operationStatus == SyncOperation::OP_SEND_FINISHED) {
        if (GetTaskErrCode() == -E_EKEYREVOKED) {
            finalStatus = SyncOperation::OP_EKEYREVOKED_FAILURE;
        } else {
            finalStatus = SyncOperation::OP_FINISHED_ALL;
        }
    }
    syncOperation_->SetStatus(deviceId_, finalStatus);
    if (finalStatus >= SyncOperation::OP_FINISHED_ALL) {
        SaveLastPushTaskExecStatus(finalStatus);
    }
    if (syncOperation_->CheckIsAllFinished()) {
        syncOperation_->Finished();
    }
}

void SyncTaskContext::SaveLastPushTaskExecStatus(int finalStatus)
{
    (void)finalStatus;
}

void SyncTaskContext::Clear()
{
    StopTimer();
    retryTime_ = 0;
    sequenceId_ = 1;
    syncId_ = 0;
    isAutoSync_ = false;
    requestSessionId_ = 0;
    isNeedRetry_ = NO_NEED_RETRY;
    mode_ = SyncModeType::INVALID_MODE;
    status_ = SyncOperation::OP_WAITING;
    taskErrCode_ = E_OK;
    packetId_ = 0;
    isAutoSubscribe_ = false;
}

int SyncTaskContext::RemoveSyncOperation(int syncId)
{
    std::lock_guard<std::mutex> lock(targetQueueLock_);
    auto iter = std::find_if(requestTargetQueue_.begin(), requestTargetQueue_.end(),
        [syncId](const ISyncTarget *target) {
            if (target == nullptr) {
                return false;
            }
            return target->GetSyncId() == syncId;
        });
    if (iter != requestTargetQueue_.end()) {
        if (*iter != nullptr) {
            delete *iter;
            *iter = nullptr;
        }
        requestTargetQueue_.erase(iter);
        return E_OK;
    }
    return -E_INVALID_ARGS;
}

void SyncTaskContext::ClearSyncTarget()
{
    std::lock_guard<std::mutex> lock(targetQueueLock_);
    for (auto &requestTarget : requestTargetQueue_) {
        if (requestTarget != nullptr) {
            delete requestTarget;
            requestTarget = nullptr;
        }
    }
    requestTargetQueue_.clear();

    for (auto &responseTarget : responseTargetQueue_) {
        if (responseTarget != nullptr) {
            delete responseTarget;
            responseTarget = nullptr;
        }
    }
    responseTargetQueue_.clear();
}

bool SyncTaskContext::IsTargetQueueEmpty() const
{
    std::lock_guard<std::mutex> lock(targetQueueLock_);
    return requestTargetQueue_.empty() && responseTargetQueue_.empty();
}

int SyncTaskContext::GetOperationStatus() const
{
    std::lock_guard<std::mutex> lock(operationLock_);
    if (syncOperation_ == nullptr) {
        return SyncOperation::OP_FINISHED_ALL;
    }
    return syncOperation_->GetStatus(deviceId_);
}

void SyncTaskContext::SetMode(int mode)
{
    mode_ = mode;
}

int SyncTaskContext::GetMode() const
{
    return mode_;
}

void SyncTaskContext::MoveToNextTarget()
{
    ClearSyncOperation();
    TaskParam param;
    // call other system api without lock
    param.timeout = communicator_->GetTimeout(deviceId_);
    std::lock_guard<std::mutex> lock(targetQueueLock_);
    while (!requestTargetQueue_.empty() || !responseTargetQueue_.empty()) {
        ISyncTarget *tmpTarget = nullptr;
        if (!requestTargetQueue_.empty()) {
            tmpTarget = requestTargetQueue_.front();
            requestTargetQueue_.pop_front();
        } else {
            tmpTarget = responseTargetQueue_.front();
            responseTargetQueue_.pop_front();
        }
        if (tmpTarget == nullptr) {
            LOGE("[SyncTaskContext][MoveToNextTarget] currentTarget is null skip!");
            continue;
        }
        SyncOperation *tmpOperation = nullptr;
        tmpTarget->GetSyncOperation(tmpOperation);
        if ((tmpOperation != nullptr) && tmpOperation->IsKilled()) {
            // if killed skip this syncOperation_.
            delete tmpTarget;
            tmpTarget = nullptr;
            continue;
        }
        CopyTargetData(tmpTarget, param);
        delete tmpTarget;
        tmpTarget = nullptr;
        break;
    }
}

int SyncTaskContext::GetNextTarget(bool isNeedSetFinished)
{
    MoveToNextTarget();
    int checkErrCode = RunPermissionCheck(GetPermissionCheckFlag(IsAutoSync(), GetMode()));
    if (checkErrCode != E_OK) {
        SetOperationStatus(SyncOperation::OP_PERMISSION_CHECK_FAILED);
        if (isNeedSetFinished) {
            SetTaskExecStatus(ISyncTaskContext::FINISHED);
        }
        return checkErrCode;
    }
    return E_OK;
}

uint32_t SyncTaskContext::GetSyncId() const
{
    return syncId_;
}

// Get the current task deviceId.
std::string SyncTaskContext::GetDeviceId() const
{
    return deviceId_;
}

void SyncTaskContext::SetTaskExecStatus(int status)
{
    taskExecStatus_ = status;
}

int SyncTaskContext::GetTaskExecStatus() const
{
    return taskExecStatus_;
}

bool SyncTaskContext::IsAutoSync() const
{
    return isAutoSync_;
}

int SyncTaskContext::StartTimer()
{
    std::lock_guard<std::mutex> lockGuard(timerLock_);
    if (timerId_ > 0) {
        return -E_UNEXPECTED_DATA;
    }
    TimerId timerId = 0;
    RefObject::IncObjRef(this);
    TimerAction timeOutCallback = std::bind(&SyncTaskContext::TimeOut, this, std::placeholders::_1);
    int errCode = RuntimeContext::GetInstance()->SetTimer(timeout_, timeOutCallback,
        [this]() {
            int ret = RuntimeContext::GetInstance()->ScheduleTask([this](){ RefObject::DecObjRef(this); });
            if (ret != E_OK) {
                LOGE("[SyncTaskContext] timer finalizer ScheduleTask, errCode %d", ret);
            }
        }, timerId);
    if (errCode != E_OK) {
        RefObject::DecObjRef(this);
        return errCode;
    }
    timerId_ = timerId;
    return errCode;
}

void SyncTaskContext::StopTimer()
{
    TimerId timerId;
    {
        std::lock_guard<std::mutex> lockGuard(timerLock_);
        timerId = timerId_;
        if (timerId_ == 0) {
            return;
        }
        timerId_ = 0;
    }
    RuntimeContext::GetInstance()->RemoveTimer(timerId);
}

int SyncTaskContext::ModifyTimer(int milliSeconds)
{
    std::lock_guard<std::mutex> lockGuard(timerLock_);
    if (timerId_ == 0) {
        return -E_UNEXPECTED_DATA;
    }
    return RuntimeContext::GetInstance()->ModifyTimer(timerId_, milliSeconds);
}

void SyncTaskContext::SetRetryTime(int retryTime)
{
    retryTime_ = retryTime;
}

int SyncTaskContext::GetRetryTime() const
{
    return retryTime_;
}

void SyncTaskContext::SetRetryStatus(int isNeedRetry)
{
    isNeedRetry_ = isNeedRetry;
}

int SyncTaskContext::GetRetryStatus() const
{
    return isNeedRetry_;
}

TimerId SyncTaskContext::GetTimerId() const
{
    return timerId_;
}

uint32_t SyncTaskContext::GetRequestSessionId() const
{
    return requestSessionId_;
}

void SyncTaskContext::IncSequenceId()
{
    sequenceId_++;
}

uint32_t SyncTaskContext::GetSequenceId() const
{
    return sequenceId_;
}

void SyncTaskContext::ReSetSequenceId()
{
    sequenceId_ = 1;
}

void SyncTaskContext::IncPacketId()
{
    packetId_++;
}

uint64_t SyncTaskContext::GetPacketId() const
{
    return packetId_;
}

int SyncTaskContext::GetTimeoutTime() const
{
    return timeout_;
}

void SyncTaskContext::SetTimeoutCallback(const TimerAction &timeOutCallback)
{
    timeOutCallback_ = timeOutCallback;
}

void SyncTaskContext::SetTimeOffset(TimeOffset offset)
{
    timeOffset_ = offset;
}

TimeOffset SyncTaskContext::GetTimeOffset() const
{
    return timeOffset_;
}

int SyncTaskContext::StartStateMachine()
{
    return stateMachine_->StartSync();
}

int SyncTaskContext::ReceiveMessageCallback(Message *inMsg)
{
    int errCode = E_OK;
    if (IncUsedCount() == E_OK) {
        errCode = stateMachine_->ReceiveMessageCallback(inMsg);
        SafeExit();
    }
    return errCode;
}

void SyncTaskContext::RegOnSyncTask(const std::function<int(void)> &callback)
{
    onSyncTaskAdd_ = callback;
}

int SyncTaskContext::IncUsedCount()
{
    AutoLock lock(this);
    if (IsKilled()) {
        LOGI("[SyncTaskContext] IncUsedCount isKilled");
        return -E_OBJ_IS_KILLED;
    }
    usedCount_++;
    return E_OK;
}

void SyncTaskContext::SafeExit()
{
    AutoLock lock(this);
    usedCount_--;
    if (usedCount_ < 1) {
        safeKill_.notify_one();
    }
}

Timestamp SyncTaskContext::GetCurrentLocalTime() const
{
    if (timeHelper_ == nullptr) {
        return TimeHelper::INVALID_TIMESTAMP;
    }
    return timeHelper_->GetTime();
}

void SyncTaskContext::Abort(int status)
{
    (void)status;
    Clear();
}

void SyncTaskContext::CommErrHandlerFunc(int errCode, ISyncTaskContext *context, int32_t sessionId)
{
    {
        std::lock_guard<std::mutex> lock(synTaskContextSetLock_);
        if (synTaskContextSet_.count(context) == 0) {
            LOGI("[SyncTaskContext][CommErrHandle] context has been killed");
            return;
        }

        // IncObjRef to maker sure context not been killed. after the lock_guard
        RefObject::IncObjRef(context);
    }

    static_cast<SyncTaskContext *>(context)->CommErrHandlerFuncInner(errCode, static_cast<uint32_t>(sessionId));
    RefObject::DecObjRef(context);
}

void SyncTaskContext::SetRemoteSoftwareVersion(uint32_t version)
{
    std::lock_guard<std::mutex> lock(remoteSoftwareVersionLock_);
    remoteSoftwareVersion_ = version;
    remoteSoftwareVersionId_++;
}

uint32_t SyncTaskContext::GetRemoteSoftwareVersion() const
{
    std::lock_guard<std::mutex> lock(remoteSoftwareVersionLock_);
    return remoteSoftwareVersion_;
}

uint64_t SyncTaskContext::GetRemoteSoftwareVersionId() const
{
    std::lock_guard<std::mutex> lock(remoteSoftwareVersionLock_);
    return remoteSoftwareVersionId_;
}

bool SyncTaskContext::IsCommNormal() const
{
    return isCommNormal_;
}

void SyncTaskContext::CommErrHandlerFuncInner(int errCode, uint32_t sessionId)
{
    {
        RefObject::AutoLock lock(this);
        if ((sessionId != requestSessionId_) || (requestSessionId_ == 0)) {
            return;
        }

        if (errCode == E_OK) {
            // when communicator sent message failed, the state machine will get the error and exit this sync task
            // it seems unnecessary to change isCommNormal_ value, so just return here
            return;
        }
    }
    LOGE("[SyncTaskContext][CommErr] errCode %d", errCode);
    stateMachine_->CommErrAbort(sessionId);
}

int SyncTaskContext::TimeOut(TimerId id)
{
    if (!timeOutCallback_) {
        return E_OK;
    }
    int errCode = IncUsedCount();
    if (errCode != E_OK) {
        LOGW("[SyncTaskContext][TimeOut] IncUsedCount failed! errCode=", errCode);
        // if return is not E_OK, the timer will be removed
        // we removed timer when context call StopTimer
        return E_OK;
    }
    IncObjRef(this);
    errCode = RuntimeContext::GetInstance()->ScheduleTask([this, id]() {
        timeOutCallback_(id);
        SafeExit();
        DecObjRef(this);
    });
    if (errCode != E_OK) {
        LOGW("[SyncTaskContext][Timeout] Trigger Timeout Async Failed! TimerId=" PRIu64 " errCode=%d", id, errCode);
        SafeExit();
        DecObjRef(this);
    }
    return E_OK;
}

void SyncTaskContext::CopyTargetData(const ISyncTarget *target, const TaskParam &taskParam)
{
    mode_ = target->GetMode();
    status_ = SyncOperation::OP_SYNCING;
    isNeedRetry_ = SyncTaskContext::NO_NEED_RETRY;
    taskErrCode_ = E_OK;
    packetId_ = 0;
    isCommNormal_ = true; // reset comm status here
    syncTaskRetryStatus_ = isSyncRetry_;
    timeout_ = static_cast<int>(taskParam.timeout);
    negotiationCount_ = 0;
    target->GetSyncOperation(syncOperation_);
    ReSetSequenceId();

    if (syncOperation_ != nullptr) {
        // IncRef for syncOperation_ to make sure syncOperation_ is valid, when setStatus
        RefObject::IncObjRef(syncOperation_);
        syncId_ = syncOperation_->GetSyncId();
        isAutoSync_ = syncOperation_->IsAutoSync();
        isAutoSubscribe_ = syncOperation_->IsAutoControlCmd();
        if (isAutoSync_ || mode_ == SUBSCRIBE_QUERY || mode_ == UNSUBSCRIBE_QUERY) {
            syncTaskRetryStatus_ = true;
        }
        requestSessionId_ = Hash::Hash32Func(deviceId_ + std::to_string(syncId_) +
            std::to_string(TimeHelper::GetSysCurrentTime()));
        if (lastRequestSessionId_ == requestSessionId_) {
            // Hash32Func max is 0x7fffffff and UINT32_MAX is 0xffffffff
            requestSessionId_++;
            LOGI("last sessionId is equal to this sessionId!");
        }
        LOGI("[SyncTaskContext][copyTarget] mode=%d,syncId=%d,isAutoSync=%d,isRetry=%d,dev=%s{private}",
            mode_, syncId_, isAutoSync_, syncTaskRetryStatus_, deviceId_.c_str());
        lastRequestSessionId_ = requestSessionId_;
        DBDfxAdapter::StartAsyncTrace(syncActionName_, static_cast<int>(syncId_));
    } else {
        isAutoSync_ = false;
        LOGI("[SyncTaskContext][copyTarget] for response data dev %s{private},isRetry=%d", deviceId_.c_str(),
            syncTaskRetryStatus_);
    }
}

void SyncTaskContext::KillWait()
{
    StopTimer();
    stateMachine_->NotifyClosing();
    UnlockObj();
    stateMachine_->AbortImmediately();
    LockObj();
    LOGW("[SyncTaskContext] Try to kill a context, now wait.");
    bool noDeadLock = WaitLockedUntil(
        safeKill_,
        [this]() {
            if (usedCount_ < 1) {
                return true;
            }
            return false;
        },
        KILL_WAIT_SECONDS);
    if (!noDeadLock) {
        LOGE("[SyncTaskContext] Dead lock may happen, we stop waiting the task exit.");
    } else {
        LOGW("[SyncTaskContext] Wait the task exit ok.");
    }
    std::lock_guard<std::mutex> lock(synTaskContextSetLock_);
    synTaskContextSet_.erase(this);
}

void SyncTaskContext::ClearSyncOperation()
{
    std::lock_guard<std::mutex> lock(operationLock_);
    if (syncOperation_ != nullptr) {
        DBDfxAdapter::FinishAsyncTrace(syncActionName_, static_cast<int>(syncId_));
        RefObject::DecObjRef(syncOperation_);
        syncOperation_ = nullptr;
    }
}

void SyncTaskContext::CancelCurrentSyncRetryIfNeed(int newTargetMode)
{
    AutoLock(this);
    if (!isAutoSync_) {
        return;
    }
    int mode = SyncOperation::TransferSyncMode(newTargetMode);
    if (newTargetMode == mode_ || mode == SyncModeType::PUSH_AND_PULL) {
        SetRetryTime(AUTO_RETRY_TIMES);
        ModifyTimer(timeout_);
    }
}

int SyncTaskContext::GetTaskErrCode() const
{
    return taskErrCode_;
}

void SyncTaskContext::SetTaskErrCode(int errCode)
{
    taskErrCode_ = errCode;
}

bool SyncTaskContext::IsSyncTaskNeedRetry() const
{
    return syncTaskRetryStatus_;
}

void SyncTaskContext::SetSyncRetry(bool isRetry)
{
    isSyncRetry_ = isRetry;
}

int SyncTaskContext::GetSyncRetryTimes() const
{
    if (IsAutoSync() || mode_ == SUBSCRIBE_QUERY || mode_ == UNSUBSCRIBE_QUERY) {
        return AUTO_RETRY_TIMES;
    }
    return MANUAL_RETRY_TIMES;
}

int SyncTaskContext::GetSyncRetryTimeout(int retryTime) const
{
    int timeoutTime = GetTimeoutTime();
    if (IsAutoSync()) {
        // set the new timeout value with 2 raised to the power of retryTime.
        return timeoutTime * static_cast<int>(pow(2, retryTime));
    }
    return timeoutTime;
}

void SyncTaskContext::ClearAllSyncTask()
{
}

bool SyncTaskContext::IsAutoLiftWaterMark() const
{
    return negotiationCount_ < NEGOTIATION_LIMIT;
}

void SyncTaskContext::IncNegotiationCount()
{
    negotiationCount_++;
}

bool SyncTaskContext::IsNeedTriggerQueryAutoSync(Message *inMsg, QuerySyncObject &query)
{
    return stateMachine_->IsNeedTriggerQueryAutoSync(inMsg, query);
}

bool SyncTaskContext::IsAutoSubscribe() const
{
    return isAutoSubscribe_;
}

bool SyncTaskContext::GetIsNeedResetAbilitySync() const
{
    return isNeedResetAbilitySync_;
}

void SyncTaskContext::SetIsNeedResetAbilitySync(bool isNeedReset)
{
    isNeedResetAbilitySync_ = isNeedReset;
}

bool SyncTaskContext::IsCurrentSyncTaskCanBeSkipped() const
{
    return false;
}

void SyncTaskContext::ResetLastPushTaskStatus()
{
}

void SyncTaskContext::SchemaChange()
{
    SetIsNeedResetAbilitySync(true);
}

void SyncTaskContext::Dump(int fd)
{
    size_t totalSyncTaskCount = 0u;
    size_t autoSyncTaskCount = 0u;
    size_t reponseTaskCount = 0u;
    {
        std::lock_guard<std::mutex> lock(targetQueueLock_);
        totalSyncTaskCount = requestTargetQueue_.size() + responseTargetQueue_.size();
        for (const auto &target : requestTargetQueue_) {
            if (target->IsAutoSync()) {
                autoSyncTaskCount++;
            }
        }
        reponseTaskCount = responseTargetQueue_.size();
    }
    DBDumpHelper::Dump(fd, "\t\ttarget = %s, total sync task count = %zu, auto sync task count = %zu,"
        " response task count = %zu\n",
        deviceId_.c_str(), totalSyncTaskCount, autoSyncTaskCount, reponseTaskCount);
}

int SyncTaskContext::RunPermissionCheck(uint8_t flag) const
{
    std::string appId = syncInterface_->GetDbProperties().GetStringProp(DBProperties::APP_ID, "");
    std::string userId = syncInterface_->GetDbProperties().GetStringProp(DBProperties::USER_ID, "");
    std::string storeId = syncInterface_->GetDbProperties().GetStringProp(DBProperties::STORE_ID, "");
    int32_t instanceId = syncInterface_->GetDbProperties().GetIntProp(DBProperties::INSTANCE_ID, 0);
    int errCode = RuntimeContext::GetInstance()->RunPermissionCheck(
        { userId, appId, storeId, deviceId_, instanceId }, flag);
    if (errCode != E_OK) {
        LOGE("[SyncTaskContext] RunPermissionCheck not pass errCode:%d, flag:%d, %s{private}",
            errCode, flag, deviceId_.c_str());
    }
    return errCode;
}

uint8_t SyncTaskContext::GetPermissionCheckFlag(bool isAutoSync, int syncMode)
{
    uint8_t flag = 0;
    int mode = SyncOperation::TransferSyncMode(syncMode);
    if (mode == SyncModeType::PUSH || mode == SyncModeType::RESPONSE_PULL) {
        flag = CHECK_FLAG_SEND;
    } else if (mode == SyncModeType::PULL) {
        flag = CHECK_FLAG_RECEIVE;
    } else if (mode == SyncModeType::PUSH_AND_PULL) {
        flag = CHECK_FLAG_SEND | CHECK_FLAG_RECEIVE;
    }
    if (isAutoSync) {
        flag = flag | CHECK_FLAG_AUTOSYNC;
    }
    if (mode != SyncModeType::RESPONSE_PULL) {
        // it means this sync is started by local
        flag = flag | CHECK_FLAG_SPONSOR;
    }
    return flag;
}

void SyncTaskContext::AbortMachineIfNeed(uint32_t syncId)
{
    uint32_t sessionId = 0u;
    {
        RefObject::AutoLock autoLock(this);
        if (syncId_ != syncId) {
            return;
        }
        sessionId = requestSessionId_;
    }
    stateMachine_->InnerErrorAbort(sessionId);
}

SyncOperation *SyncTaskContext::GetAndIncSyncOperation() const
{
    std::lock_guard<std::mutex> lock(operationLock_);
    if (syncOperation_ == nullptr) {
        return nullptr;
    }
    RefObject::IncObjRef(syncOperation_);
    return syncOperation_;
}
} // namespace DistributedDB
