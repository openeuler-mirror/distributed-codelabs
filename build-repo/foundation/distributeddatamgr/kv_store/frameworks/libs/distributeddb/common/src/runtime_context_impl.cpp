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

#include "runtime_context_impl.h"
#include "db_errno.h"
#include "db_dfx_adapter.h"
#include "log_print.h"
#include "communicator_aggregator.h"
#include "network_adapter.h"

namespace DistributedDB {
RuntimeContextImpl::RuntimeContextImpl()
    : adapter_(nullptr),
      communicatorAggregator_(nullptr),
      mainLoop_(nullptr),
      currentTimerId_(0),
      taskPool_(nullptr),
      taskPoolReportsTimerId_(0),
      timeTickMonitor_(nullptr),
      systemApiAdapter_(nullptr),
      lockStatusObserver_(nullptr),
      currentSessionId_(1)
{
}

// Destruct the object.
RuntimeContextImpl::~RuntimeContextImpl()
{
    if (taskPoolReportsTimerId_ > 0) {
        RemoveTimer(taskPoolReportsTimerId_, true);
        taskPoolReportsTimerId_ = 0;
    }
    if (taskPool_ != nullptr) {
        taskPool_->Stop();
        taskPool_->Release(taskPool_);
        taskPool_ = nullptr;
    }
    if (mainLoop_ != nullptr) {
        mainLoop_->KillAndDecObjRef(mainLoop_);
        mainLoop_ = nullptr;
    }
    SetCommunicatorAggregator(nullptr);
    (void)SetCommunicatorAdapter(nullptr);
    systemApiAdapter_ = nullptr;
    delete lockStatusObserver_;
    lockStatusObserver_ = nullptr;
    userChangeMonitor_ = nullptr;
}

// Set the label of this process.
void RuntimeContextImpl::SetProcessLabel(const std::string &label)
{
    std::lock_guard<std::mutex> labelLock(labelMutex_);
    processLabel_ = label;
}

std::string RuntimeContextImpl::GetProcessLabel() const
{
    std::lock_guard<std::mutex> labelLock(labelMutex_);
    return processLabel_;
}

int RuntimeContextImpl::SetCommunicatorAdapter(IAdapter *adapter)
{
    {
        std::lock_guard<std::mutex> autoLock(communicatorLock_);
        if (adapter_ != nullptr) {
            if (communicatorAggregator_ != nullptr) {
                return -E_NOT_SUPPORT;
            }
            delete adapter_;
        }
        adapter_ = adapter;
    }
    ICommunicatorAggregator *communicatorAggregator = nullptr;
    GetCommunicatorAggregator(communicatorAggregator);
    autoLaunch_.SetCommunicatorAggregator(communicatorAggregator);
    return E_OK;
}

int RuntimeContextImpl::GetCommunicatorAggregator(ICommunicatorAggregator *&outAggregator)
{
    outAggregator = nullptr;
    std::lock_guard<std::mutex> lock(communicatorLock_);
    if (communicatorAggregator_ != nullptr) {
        outAggregator = communicatorAggregator_;
        return E_OK;
    }

    if (adapter_ == nullptr) {
        LOGE("Adapter has not set!");
        return -E_NOT_INIT;
    }

    communicatorAggregator_ = new (std::nothrow) CommunicatorAggregator;
    if (communicatorAggregator_ == nullptr) {
        LOGE("CommunicatorAggregator create failed, may be no available memory!");
        return -E_OUT_OF_MEMORY;
    }

    int errCode = communicatorAggregator_->Initialize(adapter_);
    if (errCode != E_OK) {
        LOGE("CommunicatorAggregator init failed, err = %d!", errCode);
        RefObject::KillAndDecObjRef(communicatorAggregator_);
        communicatorAggregator_ = nullptr;
    }
    outAggregator = communicatorAggregator_;
    return errCode;
}

void RuntimeContextImpl::SetCommunicatorAggregator(ICommunicatorAggregator *inAggregator)
{
    std::lock_guard<std::mutex> autoLock(communicatorLock_);
    if (communicatorAggregator_ != nullptr) {
        autoLaunch_.SetCommunicatorAggregator(nullptr);
        communicatorAggregator_->Finalize();
        RefObject::KillAndDecObjRef(communicatorAggregator_);
    }
    communicatorAggregator_ = inAggregator;
    autoLaunch_.SetCommunicatorAggregator(communicatorAggregator_);
}

int RuntimeContextImpl::GetLocalIdentity(std::string &outTarget)
{
    std::lock_guard<std::mutex> autoLock(communicatorLock_);
    if (communicatorAggregator_ != nullptr) {
        return communicatorAggregator_->GetLocalIdentity(outTarget);
    }
    return -E_NOT_INIT;
}

// Add and start a timer.
int RuntimeContextImpl::SetTimer(int milliSeconds, const TimerAction &action,
    const TimerFinalizer &finalizer, TimerId &timerId)
{
    timerId = 0;
    if ((milliSeconds < 0) || !action) {
        return -E_INVALID_ARGS;
    }

    IEventLoop *loop = nullptr;
    int errCode = PrepareLoop(loop);
    if (errCode != E_OK) {
        LOGE("SetTimer(), prepare loop failed.");
        return errCode;
    }

    IEvent *evTimer = IEvent::CreateEvent(milliSeconds, errCode);
    if (evTimer == nullptr) {
        loop->DecObjRef(loop);
        loop = nullptr;
        return errCode;
    }

    errCode = AllocTimerId(evTimer, timerId);
    if (errCode != E_OK) {
        evTimer->DecObjRef(evTimer);
        evTimer = nullptr;
        loop->DecObjRef(loop);
        loop = nullptr;
        return errCode;
    }

    evTimer->SetAction([this, timerId, action](EventsMask revents) -> int {
            int errCodeInner = action(timerId);
            if (errCodeInner != E_OK) {
                RemoveTimer(timerId, false);
            }
            return errCodeInner;
        },
        finalizer);

    errCode = loop->Add(evTimer);
    if (errCode != E_OK) {
        evTimer->IgnoreFinalizer();
        RemoveTimer(timerId, false);
        timerId = 0;
    }

    loop->DecObjRef(loop);
    loop = nullptr;
    return errCode;
}

// Modify the interval of the timer.
int RuntimeContextImpl::ModifyTimer(TimerId timerId, int milliSeconds)
{
    if (milliSeconds < 0) {
        return -E_INVALID_ARGS;
    }

    std::lock_guard<std::mutex> autoLock(timersLock_);
    auto iter = timers_.find(timerId);
    if (iter == timers_.end()) {
        return -E_NO_SUCH_ENTRY;
    }

    IEvent *evTimer = iter->second;
    if (evTimer == nullptr) {
        return -E_INTERNAL_ERROR;
    }
    return evTimer->SetTimeout(milliSeconds);
}

// Remove the timer.
void RuntimeContextImpl::RemoveTimer(TimerId timerId, bool wait)
{
    IEvent *evTimer = nullptr;
    {
        std::lock_guard<std::mutex> autoLock(timersLock_);
        auto iter = timers_.find(timerId);
        if (iter == timers_.end()) {
            return;
        }
        evTimer = iter->second;
        timers_.erase(iter);
    }

    if (evTimer != nullptr) {
        evTimer->Detach(wait);
        evTimer->DecObjRef(evTimer);
        evTimer = nullptr;
    }
}

// Task interfaces.
int RuntimeContextImpl::ScheduleTask(const TaskAction &task)
{
    std::lock_guard<std::mutex> autoLock(taskLock_);
    int errCode = PrepareTaskPool();
    if (errCode != E_OK) {
        LOGE("Schedule task failed, fail to prepare task pool.");
        return errCode;
    }
    return taskPool_->Schedule(task);
}

int RuntimeContextImpl::ScheduleQueuedTask(const std::string &queueTag,
    const TaskAction &task)
{
    std::lock_guard<std::mutex> autoLock(taskLock_);
    int errCode = PrepareTaskPool();
    if (errCode != E_OK) {
        LOGE("Schedule queued task failed, fail to prepare task pool.");
        return errCode;
    }
    return taskPool_->Schedule(queueTag, task);
}

void RuntimeContextImpl::ShrinkMemory(const std::string &description)
{
    std::lock_guard<std::mutex> autoLock(taskLock_);
    if (taskPool_ != nullptr) {
        taskPool_->ShrinkMemory(description);
    }
}

NotificationChain::Listener *RuntimeContextImpl::RegisterTimeChangedLister(const TimeChangedAction &action,
    int &errCode)
{
    std::lock_guard<std::mutex> autoLock(timeTickMonitorLock_);
    if (timeTickMonitor_ == nullptr) {
        timeTickMonitor_ = std::make_unique<TimeTickMonitor>();
        errCode = timeTickMonitor_->StartTimeTickMonitor();
        if (errCode != E_OK) {
            LOGE("TimeTickMonitor start failed!");
            timeTickMonitor_ = nullptr;
            return nullptr;
        }
        LOGD("[RuntimeContext] TimeTickMonitor start success");
    }
    return timeTickMonitor_->RegisterTimeChangedLister(action, errCode);
}

int RuntimeContextImpl::PrepareLoop(IEventLoop *&loop)
{
    std::lock_guard<std::mutex> autoLock(loopLock_);
    if (mainLoop_ != nullptr) {
        loop = mainLoop_;
        loop->IncObjRef(loop); // ref 1 returned to caller.
        return E_OK;
    }

    int errCode = E_OK;
    loop = IEventLoop::CreateEventLoop(errCode);
    if (loop == nullptr) {
        return errCode;
    }

    loop->IncObjRef(loop); // ref 1 owned by thread.
    std::thread loopThread([loop]() {
            loop->Run();
            loop->DecObjRef(loop); // ref 1 dropped by thread.
        });
    loopThread.detach();

    mainLoop_ = loop;
    loop->IncObjRef(loop); // ref 1 returned to caller.
    return E_OK;
}

int RuntimeContextImpl::PrepareTaskPool()
{
    if (taskPool_ != nullptr) {
        return E_OK;
    }

    int errCode = E_OK;
    TaskPool *taskPool = TaskPool::Create(MAX_TP_THREADS, MIN_TP_THREADS, errCode);
    if (taskPool == nullptr) {
        return errCode;
    }

    errCode = taskPool->Start();
    if (errCode != E_OK) {
        taskPool->Release(taskPool);
        return errCode;
    }

    taskPool_ = taskPool;
    return E_OK;
}

int RuntimeContextImpl::AllocTimerId(IEvent *evTimer, TimerId &timerId)
{
    if (evTimer == nullptr) {
        return -E_INVALID_ARGS;
    }

    std::lock_guard<std::mutex> autoLock(timersLock_);
    TimerId startId = currentTimerId_;
    while (++currentTimerId_ != startId) {
        if (currentTimerId_ == 0) {
            continue;
        }
        if (timers_.find(currentTimerId_) == timers_.end()) {
            timerId = currentTimerId_;
            timers_[timerId] = evTimer;
            return E_OK;
        }
    }
    return -E_OUT_OF_IDS;
}

int RuntimeContextImpl::SetPermissionCheckCallback(const PermissionCheckCallback &callback)
{
    std::unique_lock<std::shared_mutex> writeLock(permissionCheckCallbackMutex_);
    permissionCheckCallback_ = callback;
    LOGI("SetPermissionCheckCallback ok");
    return E_OK;
}

int RuntimeContextImpl::SetPermissionCheckCallback(const PermissionCheckCallbackV2 &callback)
{
    std::unique_lock<std::shared_mutex> writeLock(permissionCheckCallbackMutex_);
    permissionCheckCallbackV2_ = callback;
    LOGI("SetPermissionCheckCallback V2 ok");
    return E_OK;
}

int RuntimeContextImpl::SetPermissionCheckCallback(const PermissionCheckCallbackV3 &callback)
{
    std::unique_lock<std::shared_mutex> writeLock(permissionCheckCallbackMutex_);
    permissionCheckCallbackV3_ = callback;
    LOGI("SetPermissionCheckCallback V3 ok");
    return E_OK;
}

int RuntimeContextImpl::RunPermissionCheck(const PermissionCheckParam &param, uint8_t flag) const
{
    bool checkResult = false;
    std::shared_lock<std::shared_mutex> autoLock(permissionCheckCallbackMutex_);
    if (permissionCheckCallbackV3_) {
        checkResult = permissionCheckCallbackV3_(param, flag);
    } else if (permissionCheckCallbackV2_) {
        checkResult = permissionCheckCallbackV2_(param.userId, param.appId, param.storeId, param.deviceId, flag);
    } else if (permissionCheckCallback_) {
        checkResult = permissionCheckCallback_(param.userId, param.appId, param.storeId, flag);
    } else {
        return E_OK;
    }
    if (checkResult) {
        return E_OK;
    } else {
        return -E_NOT_PERMIT;
    }
}

int RuntimeContextImpl::EnableKvStoreAutoLaunch(const KvDBProperties &properties, AutoLaunchNotifier notifier,
    const AutoLaunchOption &option)
{
    return autoLaunch_.EnableKvStoreAutoLaunch(properties, notifier, option);
}

int RuntimeContextImpl::DisableKvStoreAutoLaunch(const std::string &normalIdentifier,
    const std::string &dualTupleIdentifier, const std::string &userId)
{
    return autoLaunch_.DisableKvStoreAutoLaunch(normalIdentifier, dualTupleIdentifier, userId);
}

void RuntimeContextImpl::GetAutoLaunchSyncDevices(const std::string &identifier,
    std::vector<std::string> &devices) const
{
    return autoLaunch_.GetAutoLaunchSyncDevices(identifier, devices);
}

void RuntimeContextImpl::SetAutoLaunchRequestCallback(const AutoLaunchRequestCallback &callback, DBType type)
{
    autoLaunch_.SetAutoLaunchRequestCallback(callback, type);
}

NotificationChain::Listener *RuntimeContextImpl::RegisterLockStatusLister(const LockStatusNotifier &action,
    int &errCode)
{
    std::lock(lockStatusLock_, systemApiAdapterLock_);
    std::lock_guard<std::mutex> lockStatusLock(lockStatusLock_, std::adopt_lock);
    std::lock_guard<std::recursive_mutex> systemApiAdapterLock(systemApiAdapterLock_, std::adopt_lock);
    if (lockStatusObserver_ == nullptr) {
        lockStatusObserver_ = new (std::nothrow) LockStatusObserver();
        if (lockStatusObserver_ == nullptr) {
            LOGE("lockStatusObserver_ is nullptr");
            errCode = -E_OUT_OF_MEMORY;
            return nullptr;
        }
    }

    if (!lockStatusObserver_->IsStarted()) {
        errCode = lockStatusObserver_->Start();
        if (errCode != E_OK) {
            LOGE("lockStatusObserver start failed, err = %d", errCode);
            delete lockStatusObserver_;
            lockStatusObserver_ = nullptr;
            return nullptr;
        }

        if (systemApiAdapter_ != nullptr) {
            auto callback = std::bind(&LockStatusObserver::OnStatusChange,
                lockStatusObserver_, std::placeholders::_1);
            errCode = systemApiAdapter_->RegOnAccessControlledEvent(callback);
            if (errCode != OK) {
                LOGE("Register access control event change failed, err = %d", errCode);
                delete lockStatusObserver_;
                lockStatusObserver_ = nullptr;
                return nullptr;
            }
        }
    }

    NotificationChain::Listener *listener = lockStatusObserver_->RegisterLockStatusChangedLister(action, errCode);
    if ((listener == nullptr) || (errCode != E_OK)) {
        LOGE("Register lock status changed listener failed, err = %d", errCode);
        delete lockStatusObserver_;
        lockStatusObserver_ = nullptr;
        return nullptr;
    }
    return listener;
}

bool RuntimeContextImpl::IsAccessControlled() const
{
    std::lock_guard<std::recursive_mutex> autoLock(systemApiAdapterLock_);
    if (systemApiAdapter_ == nullptr) {
        return false;
    }
    return systemApiAdapter_->IsAccessControlled();
}

int RuntimeContextImpl::SetSecurityOption(const std::string &filePath, const SecurityOption &option) const
{
    std::lock_guard<std::recursive_mutex> autoLock(systemApiAdapterLock_);
    if (systemApiAdapter_ == nullptr || !OS::CheckPathExistence(filePath)) {
        LOGI("Adapter is not set, or path not existed, not support set security option!");
        return -E_NOT_SUPPORT;
    }

    if (option == SecurityOption()) {
        LOGD("SecurityOption is NOT_SET,Not need to set security option!");
        return E_OK;
    }

    std::string fileRealPath;
    int errCode = OS::GetRealPath(filePath, fileRealPath);
    if (errCode != E_OK) {
        LOGE("Get real path failed when set security option!");
        return errCode;
    }

    errCode = systemApiAdapter_->SetSecurityOption(fileRealPath, option);
    if (errCode != OK) {
        if (errCode == NOT_SUPPORT) {
            return -E_NOT_SUPPORT;
        }
        LOGE("SetSecurityOption failed, errCode = %d", errCode);
        return -E_SYSTEM_API_ADAPTER_CALL_FAILED;
    }
    return E_OK;
}

int RuntimeContextImpl::GetSecurityOption(const std::string &filePath, SecurityOption &option) const
{
    std::lock_guard<std::recursive_mutex> autoLock(systemApiAdapterLock_);
    if (systemApiAdapter_ == nullptr) {
        LOGI("Get Security option, but not set system api adapter!");
        return -E_NOT_SUPPORT;
    }
    LOGI("[HP_DEBUG] GetSecurityOption filePath = %s", filePath.c_str());
    int errCode = systemApiAdapter_->GetSecurityOption(filePath, option);
    if (errCode != OK) {
        if (errCode == NOT_SUPPORT) {
            return -E_NOT_SUPPORT;
        }
        LOGE("GetSecurityOption failed, errCode = %d", errCode);
        return -E_SYSTEM_API_ADAPTER_CALL_FAILED;
    }

    LOGD("Get security option from system adapter [%d, %d]", option.securityLabel, option.securityFlag);
    // This interface may return success but failed to obtain the flag and modified it to -1
    if (option.securityFlag == INVALID_SEC_FLAG) {
        // Currently ignoring the failure to obtain flags -1 other than S3, modify the flag to the default value
        if (option.securityLabel == S3) {
            LOGE("GetSecurityOption failed, SecurityOption is invalid [3, -1]!");
            return -E_SYSTEM_API_ADAPTER_CALL_FAILED;
        }
        option.securityFlag = 0; // 0 is default value
    }
    return E_OK;
}

bool RuntimeContextImpl::CheckDeviceSecurityAbility(const std::string &devId, const SecurityOption &option) const
{
    std::lock_guard<std::recursive_mutex> autoLock(systemApiAdapterLock_);
    if (systemApiAdapter_ == nullptr) {
        return true;
    }
    return systemApiAdapter_->CheckDeviceSecurityAbility(devId, option);
}

int RuntimeContextImpl::SetProcessSystemApiAdapter(const std::shared_ptr<IProcessSystemApiAdapter> &adapter)
{
    std::lock(lockStatusLock_, systemApiAdapterLock_);
    std::lock_guard<std::mutex> lockStatusLock(lockStatusLock_, std::adopt_lock);
    std::lock_guard<std::recursive_mutex> systemApiAdapterLock(systemApiAdapterLock_, std::adopt_lock);
    systemApiAdapter_ = adapter;
    if (systemApiAdapter_ != nullptr && lockStatusObserver_ != nullptr && lockStatusObserver_->IsStarted()) {
        auto callback = std::bind(&LockStatusObserver::OnStatusChange,
            lockStatusObserver_, std::placeholders::_1);
        int errCode = systemApiAdapter_->RegOnAccessControlledEvent(callback);
        if (errCode != OK) {
            LOGE("Register access controlled event failed while setting adapter, err = %d", errCode);
            delete lockStatusObserver_;
            lockStatusObserver_ = nullptr;
            return -E_SYSTEM_API_ADAPTER_CALL_FAILED;
        }
    }
    return E_OK;
}

bool RuntimeContextImpl::IsProcessSystemApiAdapterValid() const
{
    std::lock_guard<std::recursive_mutex> autoLock(systemApiAdapterLock_);
    return (systemApiAdapter_ != nullptr);
}

void RuntimeContextImpl::NotifyTimestampChanged(TimeOffset offset) const
{
    std::lock_guard<std::mutex> autoLock(timeTickMonitorLock_);
    if (timeTickMonitor_ == nullptr) {
        LOGD("NotifyTimestampChanged fail, timeTickMonitor_ is null.");
        return;
    }
    timeTickMonitor_->NotifyTimeChange(offset);
}

bool RuntimeContextImpl::IsCommunicatorAggregatorValid() const
{
    std::lock_guard<std::mutex> autoLock(communicatorLock_);
    if (communicatorAggregator_ == nullptr && adapter_ == nullptr) {
        return false;
    }
    return true;
}

void RuntimeContextImpl::SetStoreStatusNotifier(const StoreStatusNotifier &notifier)
{
    std::unique_lock<std::shared_mutex> writeLock(databaseStatusCallbackMutex_);
    databaseStatusNotifyCallback_ = notifier;
    LOGI("SetStoreStatusNotifier ok");
}

void RuntimeContextImpl::NotifyDatabaseStatusChange(const std::string &userId, const std::string &appId,
    const std::string &storeId, const std::string &deviceId, bool onlineStatus)
{
    ScheduleTask([this, userId, appId, storeId, deviceId, onlineStatus] {
        std::shared_lock<std::shared_mutex> autoLock(databaseStatusCallbackMutex_);
        if (databaseStatusNotifyCallback_) {
            LOGI("start notify database status:%d", onlineStatus);
            databaseStatusNotifyCallback_(userId, appId, storeId, deviceId, onlineStatus);
        }
    });
}

int RuntimeContextImpl::SetSyncActivationCheckCallback(const SyncActivationCheckCallback &callback)
{
    std::unique_lock<std::shared_mutex> writeLock(syncActivationCheckCallbackMutex_);
    syncActivationCheckCallback_ = callback;
    LOGI("SetSyncActivationCheckCallback ok");
    return E_OK;
}

int RuntimeContextImpl::SetSyncActivationCheckCallback(const SyncActivationCheckCallbackV2 &callback)
{
    std::unique_lock<std::shared_mutex> writeLock(syncActivationCheckCallbackMutex_);
    syncActivationCheckCallbackV2_ = callback;
    LOGI("SetSyncActivationCheckCallbackV2 ok");
    return E_OK;
}

bool RuntimeContextImpl::IsSyncerNeedActive(const DBProperties &properties) const
{
    ActivationCheckParam param = {
        properties.GetStringProp(DBProperties::USER_ID, ""),
        properties.GetStringProp(DBProperties::APP_ID, ""),
        properties.GetStringProp(DBProperties::STORE_ID, ""),
        properties.GetIntProp(DBProperties::INSTANCE_ID, 0)
    };
    std::shared_lock<std::shared_mutex> autoLock(syncActivationCheckCallbackMutex_);
    if (syncActivationCheckCallbackV2_) {
        return syncActivationCheckCallbackV2_(param);
    } else if (syncActivationCheckCallback_) {
        return syncActivationCheckCallback_(param.userId, param.appId, param.storeId);
    }
    return true;
}

NotificationChain::Listener *RuntimeContextImpl::RegisterUserChangedListener(const UserChangedAction &action,
    EventType event)
{
    int errCode;
    std::lock_guard<std::mutex> autoLock(userChangeMonitorLock_);
    if (userChangeMonitor_ == nullptr) {
        userChangeMonitor_ = std::make_unique<UserChangeMonitor>();
        errCode = userChangeMonitor_->Start();
        if (errCode != E_OK) {
            LOGE("UserChangeMonitor start failed!");
            userChangeMonitor_ = nullptr;
            return nullptr;
        }
    }
    NotificationChain::Listener *listener = userChangeMonitor_->RegisterUserChangedListener(action, event, errCode);
    if ((listener == nullptr) || (errCode != E_OK)) {
        LOGE("Register user status changed listener failed, err = %d", errCode);
        return nullptr;
    }
    return listener;
}

int RuntimeContextImpl::NotifyUserChanged() const
{
    {
        std::lock_guard<std::mutex> autoLock(userChangeMonitorLock_);
        if (userChangeMonitor_ == nullptr) {
            LOGD("userChangeMonitor is null, all db is in normal sync mode");
            return E_OK;
        }
    }
    userChangeMonitor_->NotifyUserChanged();
    return E_OK;
}

uint32_t RuntimeContextImpl::GenerateSessionId()
{
    uint32_t sessionId = currentSessionId_++;
    if (sessionId == 0) {
        sessionId = currentSessionId_++;
    }
    return sessionId;
}

void RuntimeContextImpl::DumpCommonInfo(int fd)
{
    autoLaunch_.Dump(fd);
}

void RuntimeContextImpl::CloseAutoLaunchConnection(DBType type, const DBProperties &properties)
{
    autoLaunch_.CloseConnection(type, properties);
}

int RuntimeContextImpl::SetPermissionConditionCallback(const PermissionConditionCallback &callback)
{
    std::unique_lock<std::shared_mutex> autoLock(permissionConditionLock_);
    permissionConditionCallback_ = callback;
    return E_OK;
}

std::map<std::string, std::string> RuntimeContextImpl::GetPermissionCheckParam(const DBProperties &properties)
{
    PermissionConditionParam param = {
        properties.GetStringProp(DBProperties::USER_ID, ""),
        properties.GetStringProp(DBProperties::APP_ID, ""),
        properties.GetStringProp(DBProperties::STORE_ID, ""),
        properties.GetIntProp(DBProperties::INSTANCE_ID, 0)
    };
    std::shared_lock<std::shared_mutex> autoLock(permissionConditionLock_);
    if (permissionConditionCallback_ == nullptr) {
        return {};
    }
    return permissionConditionCallback_(param);
}

void RuntimeContextImpl::StopTaskPool()
{
    std::lock_guard<std::mutex> autoLock(taskLock_);
    if (taskPool_ != nullptr) {
        taskPool_->Stop();
        TaskPool::Release(taskPool_);
        taskPool_ = nullptr;
    }
}

void RuntimeContextImpl::StopTimeTickMonitorIfNeed()
{
    std::lock_guard<std::mutex> autoLock(timeTickMonitorLock_);
    if (timeTickMonitor_ == nullptr) {
        return;
    }
    if (timeTickMonitor_->EmptyListener()) {
        LOGD("[RuntimeContext] TimeTickMonitor exist because no listener");
        timeTickMonitor_ = nullptr;
    }
}
} // namespace DistributedDB
