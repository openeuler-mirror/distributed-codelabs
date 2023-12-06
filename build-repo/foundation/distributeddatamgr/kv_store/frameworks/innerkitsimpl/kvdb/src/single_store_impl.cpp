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
#define LOG_TAG "SingleStoreImpl"
#include "single_store_impl.h"
#include "auto_sync_timer.h"
#include "backup_manager.h"
#include "dds_trace.h"
#include "dev_manager.h"
#include "kvdb_service_client.h"
#include "log_print.h"
#include "store_result_set.h"
#include "store_util.h"
namespace OHOS::DistributedKv {
using namespace OHOS::DistributedDataDfx;
using namespace std::chrono;
SingleStoreImpl::SingleStoreImpl(std::shared_ptr<DBStore> dbStore, const AppId &appId, const Options &options,
    const Convertor &cvt)
    : convertor_(cvt), dbStore_(std::move(dbStore))
{
    appId_ = appId.appId;
    storeId_ = dbStore_->GetStoreId();
    autoSync_ = options.autoSync;
    syncObserver_ = std::make_shared<SyncObserver>();
    if (options.backup) {
        BackupManager::GetInstance().Prepare(options.baseDir, storeId_);
    }

    for (auto &policy : options.policies) {
        if (policy.type != TERM_OF_SYNC_VALIDITY) {
            continue;
        }
        auto exist = std::get_if<uint32_t>(&policy.value);
        if (exist == nullptr || *exist <= 0) {
            break;
        }
        interval_ = *exist;
        DevManager::GetInstance().Register(this);
    }
}

SingleStoreImpl::~SingleStoreImpl()
{
    if (interval_ > 0) {
        DevManager::GetInstance().Unregister(this);
    }
}

StoreId SingleStoreImpl::GetStoreId() const
{
    return { storeId_ };
}

Status SingleStoreImpl::Put(const Key &key, const Value &value)
{
    // // DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));
    std::shared_lock<decltype(rwMutex_)> lock(rwMutex_);
    if (dbStore_ == nullptr) {
        ZLOGE("db:%{public}s already closed!", storeId_.c_str());
        return ALREADY_CLOSED;
    }

    DBKey dbKey = convertor_.ToLocalDBKey(key);
    if (dbKey.empty() || value.Size() > MAX_VALUE_LENGTH) {
        ZLOGE("invalid key:%{public}s size:[k:%{public}zu v:%{public}zu]",
            StoreUtil::Anonymous(key.ToString()).c_str(), key.Size(), value.Size());
        return INVALID_ARGUMENT;
    }

    auto dbStatus = dbStore_->Put(dbKey, value);
    auto status = StoreUtil::ConvertStatus(dbStatus);
    if (status != SUCCESS) {
        ZLOGE("status:0x%{public}x key:%{public}s, value size:%{public}zu", status,
            StoreUtil::Anonymous(key.ToString()).c_str(), value.Size());
    }
    DoAutoSync();
    return status;
}

Status SingleStoreImpl::PutBatch(const std::vector<Entry> &entries)
{
    // DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));
    std::shared_lock<decltype(rwMutex_)> lock(rwMutex_);
    if (dbStore_ == nullptr) {
        ZLOGE("db:%{public}s already closed!", storeId_.c_str());
        return ALREADY_CLOSED;
    }

    std::vector<DBEntry> dbEntries;
    for (const auto &entry : entries) {
        DBEntry dbEntry;
        dbEntry.key = convertor_.ToLocalDBKey(entry.key);
        if (dbEntry.key.empty() || entry.value.Size() > MAX_VALUE_LENGTH) {
            ZLOGE("invalid key:%{public}s size:[k:%{public}zu v:%{public}zu]",
                StoreUtil::Anonymous(entry.key.ToString()).c_str(), entry.key.Size(), entry.value.Size());
            return INVALID_ARGUMENT;
        }
        dbEntry.value = entry.value;
        dbEntries.push_back(std::move(dbEntry));
    }
    auto dbStatus = dbStore_->PutBatch(dbEntries);
    auto status = StoreUtil::ConvertStatus(dbStatus);
    if (status != SUCCESS) {
        ZLOGE("status:0x%{public}x entries size:%{public}zu", status, entries.size());
    }
    DoAutoSync();
    return status;
}

Status SingleStoreImpl::Delete(const Key &key)
{
    // DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));
    std::shared_lock<decltype(rwMutex_)> lock(rwMutex_);
    if (dbStore_ == nullptr) {
        ZLOGE("db:%{public}s already closed!", storeId_.c_str());
        return ALREADY_CLOSED;
    }

    DBKey dbKey = convertor_.ToLocalDBKey(key);
    if (dbKey.empty()) {
        ZLOGE("invalid key:%{public}s size:%{public}zu", StoreUtil::Anonymous(key.ToString()).c_str(), key.Size());
        return INVALID_ARGUMENT;
    }

    auto dbStatus = dbStore_->Delete(dbKey);
    auto status = StoreUtil::ConvertStatus(dbStatus);
    if (status != SUCCESS) {
        ZLOGE("status:0x%{public}x key:%{public}s", status, StoreUtil::Anonymous(key.ToString()).c_str());
    }
    DoAutoSync();
    return status;
}

Status SingleStoreImpl::DeleteBatch(const std::vector<Key> &keys)
{
    // DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));
    std::shared_lock<decltype(rwMutex_)> lock(rwMutex_);
    if (dbStore_ == nullptr) {
        ZLOGE("db:%{public}s already closed!", storeId_.c_str());
        return ALREADY_CLOSED;
    }

    std::vector<DBKey> dbKeys;
    for (const auto &key : keys) {
        DBKey dbKey = convertor_.ToLocalDBKey(key);
        if (dbKey.empty()) {
            ZLOGE("invalid key:%{public}s size:%{public}zu", StoreUtil::Anonymous(key.ToString()).c_str(), key.Size());
            return INVALID_ARGUMENT;
        }
        dbKeys.push_back(std::move(dbKey));
    }

    auto dbStatus = dbStore_->DeleteBatch(dbKeys);
    auto status = StoreUtil::ConvertStatus(dbStatus);
    if (status != SUCCESS) {
        ZLOGE("status:0x%{public}x keys size:%{public}zu", status, keys.size());
    }
    DoAutoSync();
    return status;
}

Status SingleStoreImpl::StartTransaction()
{
    // DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));
    std::shared_lock<decltype(rwMutex_)> lock(rwMutex_);
    if (dbStore_ == nullptr) {
        ZLOGE("db:%{public}s already closed!", storeId_.c_str());
        return ALREADY_CLOSED;
    }

    auto dbStatus = dbStore_->StartTransaction();
    auto status = StoreUtil::ConvertStatus(dbStatus);
    if (status != SUCCESS) {
        ZLOGE("status:0x%{public}x storeId:%{public}s", status, storeId_.c_str());
    }
    return status;
}

Status SingleStoreImpl::Commit()
{
    // DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));
    std::shared_lock<decltype(rwMutex_)> lock(rwMutex_);
    if (dbStore_ == nullptr) {
        ZLOGE("db:%{public}s already closed!", storeId_.c_str());
        return ALREADY_CLOSED;
    }

    auto dbStatus = dbStore_->Commit();
    auto status = StoreUtil::ConvertStatus(dbStatus);
    if (status != SUCCESS) {
        ZLOGE("status:0x%{public}x storeId:%{public}s", status, dbStore_->GetStoreId().c_str());
    }
    return status;
}

Status SingleStoreImpl::Rollback()
{
    // DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));
    std::shared_lock<decltype(rwMutex_)> lock(rwMutex_);
    if (dbStore_ == nullptr) {
        ZLOGE("db:%{public}s already closed!", storeId_.c_str());
        return ALREADY_CLOSED;
    }

    auto dbStatus = dbStore_->Rollback();
    auto status = StoreUtil::ConvertStatus(dbStatus);
    if (status != SUCCESS) {
        ZLOGE("status:0x%{public}x storeId:%{public}s", status, storeId_.c_str());
    }
    return status;
}

Status SingleStoreImpl::SubscribeKvStore(SubscribeType type, std::shared_ptr<Observer> observer)
{
    // DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));
    std::shared_lock<decltype(rwMutex_)> lock(rwMutex_);
    if (dbStore_ == nullptr) {
        ZLOGE("db:%{public}s already closed!", storeId_.c_str());
        return ALREADY_CLOSED;
    }

    if (observer == nullptr) {
        ZLOGE("invalid observer is null");
        return INVALID_ARGUMENT;
    }

    uint32_t realType = type;
    std::shared_ptr<ObserverBridge> bridge = PutIn(realType, observer);
    if (bridge == nullptr) {
        return (realType == type) ? OVER_MAX_SUBSCRIBE_LIMITS : STORE_ALREADY_SUBSCRIBE;
    }

    Status status = SUCCESS;
    if ((realType & SUBSCRIBE_TYPE_LOCAL) == SUBSCRIBE_TYPE_LOCAL) {
        auto dbStatus = dbStore_->RegisterObserver({}, DistributedDB::OBSERVER_CHANGES_NATIVE, bridge.get());
        status = StoreUtil::ConvertStatus(dbStatus);
    }

    if (((realType & SUBSCRIBE_TYPE_REMOTE) == SUBSCRIBE_TYPE_REMOTE) && status == SUCCESS) {
        realType &= ~SUBSCRIBE_TYPE_LOCAL;
        status = bridge->RegisterRemoteObserver();
    }

    if (status != SUCCESS) {
        ZLOGE("status:0x%{public}x type:%{public}d->%{public}d observer:0x%{public}x", status, type, realType,
            StoreUtil::Anonymous(bridge.get()));
        TakeOut(realType, observer);
    }
    return status;
}

Status SingleStoreImpl::UnSubscribeKvStore(SubscribeType type, std::shared_ptr<Observer> observer)
{
    // DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));
    std::shared_lock<decltype(rwMutex_)> lock(rwMutex_);
    if (dbStore_ == nullptr) {
        ZLOGE("db:%{public}s already closed!", storeId_.c_str());
        return ALREADY_CLOSED;
    }

    if (observer == nullptr) {
        ZLOGE("invalid observer is null");
        return INVALID_ARGUMENT;
    }

    uint32_t realType = type;
    std::shared_ptr<ObserverBridge> bridge = TakeOut(realType, observer);
    if (bridge == nullptr) {
        return STORE_NOT_SUBSCRIBE;
    }

    Status status = SUCCESS;
    if ((realType & SUBSCRIBE_TYPE_LOCAL) == SUBSCRIBE_TYPE_LOCAL) {
        auto dbStatus = dbStore_->UnRegisterObserver(bridge.get());
        status = StoreUtil::ConvertStatus(dbStatus);
    }

    if (((realType & SUBSCRIBE_TYPE_REMOTE) == SUBSCRIBE_TYPE_REMOTE) && status == SUCCESS) {
        realType &= ~SUBSCRIBE_TYPE_LOCAL;
        status = bridge->UnregisterRemoteObserver();
    }

    if (status != SUCCESS) {
        ZLOGE("status:0x%{public}x type:%{public}d->%{public}d observer:0x%{public}x", status, type, realType,
            StoreUtil::Anonymous(bridge.get()));
    }
    return status;
}

Status SingleStoreImpl::Get(const Key &key, Value &value)
{
    // DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));
    std::shared_lock<decltype(rwMutex_)> lock(rwMutex_);
    if (dbStore_ == nullptr) {
        ZLOGE("db:%{public}s already closed!", storeId_.c_str());
        return ALREADY_CLOSED;
    }

    DBKey dbKey = convertor_.ToWholeDBKey(key);
    if (dbKey.empty()) {
        ZLOGE("invalid key:%{public}s size:%{public}zu", StoreUtil::Anonymous(key.ToString()).c_str(), key.Size());
        return INVALID_ARGUMENT;
    }

    DBValue dbValue;
    auto dbStatus = dbStore_->Get(dbKey, dbValue);
    value = std::move(dbValue);
    auto status = StoreUtil::ConvertStatus(dbStatus);
    if (status != SUCCESS) {
        ZLOGE("status:0x%{public}x key:%{public}s", status, StoreUtil::Anonymous(key.ToString()).c_str());
    }
    return status;
}

Status SingleStoreImpl::GetEntries(const Key &prefix, std::vector<Entry> &entries) const
{
    // DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));
    DBKey dbPrefix = convertor_.GetPrefix(prefix);
    if (dbPrefix.empty() && !prefix.Empty()) {
        ZLOGE("invalid prefix:%{public}s size:%{public}zu", StoreUtil::Anonymous(prefix.ToString()).c_str(),
            prefix.Size());
        return INVALID_ARGUMENT;
    }

    DBQuery dbQuery = DBQuery::Select();
    dbQuery.PrefixKey(dbPrefix);
    auto status = GetEntries(dbQuery, entries);
    if (status != SUCCESS) {
        ZLOGE("status:0x%{public}x prefix:%{public}s", status, StoreUtil::Anonymous(prefix.ToString()).c_str());
    }
    return status;
}

Status SingleStoreImpl::GetEntries(const DataQuery &query, std::vector<Entry> &entries) const
{
    // DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));
    DBQuery dbQuery = convertor_.GetDBQuery(query);
    auto status = GetEntries(dbQuery, entries);
    if (status != SUCCESS) {
        ZLOGE("status:0x%{public}x query:%{public}s", status, StoreUtil::Anonymous(query.ToString()).c_str());
    }
    return status;
}

Status SingleStoreImpl::GetResultSet(const Key &prefix, std::shared_ptr<ResultSet> &resultSet) const
{
    // DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));
    DBKey dbPrefix = convertor_.GetPrefix(prefix);
    if (dbPrefix.empty() && !prefix.Empty()) {
        ZLOGE("invalid prefix:%{public}s size:%{public}zu", StoreUtil::Anonymous(prefix.ToString()).c_str(),
            prefix.Size());
        return INVALID_ARGUMENT;
    }

    DBQuery dbQuery = DistributedDB::Query::Select();
    dbQuery.PrefixKey(dbPrefix);
    auto status = GetResultSet(dbQuery, resultSet);
    if (status != SUCCESS) {
        ZLOGE("status:0x%{public}x prefix:%{public}s", status, StoreUtil::Anonymous(prefix.ToString()).c_str());
    }
    return status;
}

Status SingleStoreImpl::GetResultSet(const DataQuery &query, std::shared_ptr<ResultSet> &resultSet) const
{
    // DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));
    DBQuery dbQuery = convertor_.GetDBQuery(query);
    auto status = GetResultSet(dbQuery, resultSet);
    if (status != SUCCESS) {
        ZLOGE("status:0x%{public}x query:%{public}s", status, StoreUtil::Anonymous(query.ToString()).c_str());
    }
    return status;
}

Status SingleStoreImpl::CloseResultSet(std::shared_ptr<ResultSet> &resultSet)
{
    // DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));
    if (resultSet == nullptr) {
        ZLOGE("input is nullptr");
        return INVALID_ARGUMENT;
    }

    auto status = resultSet->Close();
    if (status != SUCCESS) {
        ZLOGE("status:0x%{public}x storeId:%{public}s", status, storeId_.c_str());
    }
    resultSet = nullptr;
    return status;
}

Status SingleStoreImpl::GetCount(const DataQuery &query, int &result) const
{
    // DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));
    std::shared_lock<decltype(rwMutex_)> lock(rwMutex_);
    if (dbStore_ == nullptr) {
        ZLOGE("db:%{public}s already closed!", storeId_.c_str());
        return ALREADY_CLOSED;
    }

    DBQuery dbQuery = convertor_.GetDBQuery(query);
    auto dbStatus = dbStore_->GetCount(dbQuery, result);
    auto status = StoreUtil::ConvertStatus(dbStatus);
    if (status != SUCCESS) {
        ZLOGE("status:0x%{public}x query:%{public}s", status, StoreUtil::Anonymous(query.ToString()).c_str());
    }
    return status;
}

Status SingleStoreImpl::GetSecurityLevel(SecurityLevel &secLevel) const
{
    // DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));
    std::shared_lock<decltype(rwMutex_)> lock(rwMutex_);
    if (dbStore_ == nullptr) {
        ZLOGE("db:%{public}s already closed!", storeId_.c_str());
        return ALREADY_CLOSED;
    }

    DistributedDB::SecurityOption option;
    auto dbStatus = dbStore_->GetSecurityOption(option);
    secLevel = static_cast<SecurityLevel>(StoreUtil::GetSecLevel(option));
    auto status = StoreUtil::ConvertStatus(dbStatus);
    if (status != SUCCESS) {
        ZLOGE("status:0x%{public}x security:[%{public}d]", status, option.securityLabel);
    }
    return status;
}

Status SingleStoreImpl::RemoveDeviceData(const std::string &device)
{
    // DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));
    std::shared_lock<decltype(rwMutex_)> lock(rwMutex_);
    if (dbStore_ == nullptr) {
        ZLOGE("db:%{public}s already closed!", storeId_.c_str());
        return ALREADY_CLOSED;
    }

    if (device.empty()) {
        auto dbStatus = dbStore_->RemoveDeviceData();
        auto status = StoreUtil::ConvertStatus(dbStatus);
        if (status != SUCCESS) {
            ZLOGE("status:0x%{public}x device:all others", status);
        }
        return status;
    }

    auto dbStatus = dbStore_->RemoveDeviceData(DevManager::GetInstance().ToUUID(device));
    auto status = StoreUtil::ConvertStatus(dbStatus);
    if (status != SUCCESS) {
        ZLOGE("status:0x%{public}x device:%{public}s", status, StoreUtil::Anonymous(device).c_str());
    }
    return status;
}

Status SingleStoreImpl::Sync(const std::vector<std::string> &devices, SyncMode mode, uint32_t delay)
{
    // DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));
    KVDBService::SyncInfo syncInfo;
    syncInfo.seqId = StoreUtil::GenSequenceId();
    syncInfo.mode = mode;
    syncInfo.delay = delay;
    syncInfo.devices = devices;
    return DoSync(syncInfo, syncObserver_);
}

Status SingleStoreImpl::Sync(const std::vector<std::string> &devices, SyncMode mode, const DataQuery &query,
    std::shared_ptr<SyncCallback> syncCallback)
{
    // DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));
    KVDBService::SyncInfo syncInfo;
    syncInfo.seqId = StoreUtil::GenSequenceId();
    syncInfo.mode = mode;
    syncInfo.devices = devices;
    syncInfo.query = query.ToString();
    return DoSync(syncInfo, syncCallback);
}

Status SingleStoreImpl::RegisterSyncCallback(std::shared_ptr<SyncCallback> callback)
{
    // DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__), TraceSwitch::BYTRACE_ON);
    if (callback == nullptr) {
        ZLOGW("INVALID_ARGUMENT.");
        return INVALID_ARGUMENT;
    }
    syncObserver_->Add(callback);
    return SUCCESS;
}

Status SingleStoreImpl::UnRegisterSyncCallback()
{
    // DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__), TraceSwitch::BYTRACE_ON);
    syncObserver_->Clean();
    return SUCCESS;
}

Status SingleStoreImpl::SetSyncParam(const KvSyncParam &syncParam)
{
    // DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__), true);
    auto service = KVDBServiceClient::GetInstance();
    if (service == nullptr) {
        return SERVER_UNAVAILABLE;
    }
    return service->SetSyncParam({ appId_ }, { storeId_ }, syncParam);
}

Status SingleStoreImpl::GetSyncParam(KvSyncParam &syncParam)
{
    // DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__), true);
    auto service = KVDBServiceClient::GetInstance();
    if (service == nullptr) {
        return SERVER_UNAVAILABLE;
    }
    return service->GetSyncParam({ appId_ }, { storeId_ }, syncParam);
}

Status SingleStoreImpl::SetCapabilityEnabled(bool enabled) const
{
    // DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__), true);
    auto service = KVDBServiceClient::GetInstance();
    if (service == nullptr) {
        return SERVER_UNAVAILABLE;
    }
    if (enabled) {
        return service->EnableCapability({ appId_ }, { storeId_ });
    }
    return service->DisableCapability({ appId_ }, { storeId_ });
}

Status SingleStoreImpl::SetCapabilityRange(const std::vector<std::string> &localLabels,
    const std::vector<std::string> &remoteLabels) const
{
    // DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__), true);
    auto service = KVDBServiceClient::GetInstance();
    if (service == nullptr) {
        return SERVER_UNAVAILABLE;
    }
    return service->SetCapability({ appId_ }, { storeId_ }, localLabels, remoteLabels);
}

Status SingleStoreImpl::SubscribeWithQuery(const std::vector<std::string> &devices, const DataQuery &query)
{
    // DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__), true);
    auto service = KVDBServiceClient::GetInstance();
    if (service == nullptr) {
        return SERVER_UNAVAILABLE;
    }
    SyncInfo syncInfo;
    syncInfo.seqId = StoreUtil::GenSequenceId();
    syncInfo.devices = devices;
    syncInfo.query = query.ToString();
    auto syncAgent = service->GetSyncAgent({ appId_ });
    if (syncAgent == nullptr) {
        ZLOGE("failed! invalid agent app:%{public}s, store:%{public}s!", appId_.c_str(), storeId_.c_str());
        return ILLEGAL_STATE;
    }

    syncAgent->AddSyncCallback(syncObserver_, syncInfo.seqId);
    return service->AddSubscribeInfo({ appId_ }, { storeId_ }, syncInfo);
}

Status SingleStoreImpl::UnsubscribeWithQuery(const std::vector<std::string> &devices, const DataQuery &query)
{
    // DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__), true);
    auto service = KVDBServiceClient::GetInstance();
    if (service == nullptr) {
        return SERVER_UNAVAILABLE;
    }
    SyncInfo syncInfo;
    syncInfo.seqId = StoreUtil::GenSequenceId();
    syncInfo.devices = devices;
    syncInfo.query = query.ToString();
    auto syncAgent = service->GetSyncAgent({ appId_ });
    if (syncAgent == nullptr) {
        ZLOGE("failed! invalid agent app:%{public}s, store:%{public}s!", appId_.c_str(), storeId_.c_str());
        return ILLEGAL_STATE;
    }

    syncAgent->AddSyncCallback(syncObserver_, syncInfo.seqId);
    return service->RmvSubscribeInfo({ appId_ }, { storeId_ }, syncInfo);
}

int32_t SingleStoreImpl::AddRef()
{
    ref_++;
    return ref_;
}

int32_t SingleStoreImpl::Close(bool isForce)
{
    if (isForce) {
        ref_ = 1;
    }
    ref_--;
    if (ref_ != 0) {
        return ref_;
    }

    observers_.Clear();
    syncObserver_->Clean();
    std::unique_lock<decltype(rwMutex_)> lock(rwMutex_);
    dbStore_ = nullptr;
    return ref_;
}

Status SingleStoreImpl::Backup(const std::string &file, const std::string &baseDir)
{
    // DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));
    auto status = BackupManager::GetInstance().Backup(file, baseDir, storeId_, dbStore_);
    if (status != SUCCESS) {
        ZLOGE("status:0x%{public}x storeId:%{public}s backup:%{public}s ", status, storeId_.c_str(), file.c_str());
    }
    return status;
}

Status SingleStoreImpl::Restore(const std::string &file, const std::string &baseDir)
{
    // DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));
    auto status = BackupManager::GetInstance().Restore(file, baseDir, appId_, storeId_, dbStore_);
    if (status != SUCCESS) {
        ZLOGE("status:0x%{public}x storeId:%{public}s backup:%{public}s ", status, storeId_.c_str(), file.c_str());
    }
    return status;
}

Status SingleStoreImpl::DeleteBackup(const std::vector<std::string> &files, const std::string &baseDir,
    std::map<std::string, DistributedKv::Status> &results)
{
    // DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));
    for (auto &file : files) {
        results.emplace(file, DEVICE_NOT_FOUND);
    }
    auto status = BackupManager::GetInstance().DeleteBackup(results, baseDir, storeId_);
    if (status != SUCCESS) {
        ZLOGE("status:0x%{public}x storeId:%{public}s", status, storeId_.c_str());
    }
    return status;
}

std::function<void(ObserverBridge *)> SingleStoreImpl::BridgeReleaser()
{
    return [this](ObserverBridge *obj) {
        if (obj == nullptr) {
            return;
        }
        Status status = ALREADY_CLOSED;
        {
            std::shared_lock<decltype(rwMutex_)> lock(rwMutex_);
            if (dbStore_ != nullptr) {
                auto dbStatus = dbStore_->UnRegisterObserver(obj);
                status = StoreUtil::ConvertStatus(dbStatus);
            }
        }
        Status remote = obj->UnregisterRemoteObserver();
        if (status != SUCCESS || remote != SUCCESS) {
            ZLOGE("status:0x%{public}x remote:0x%{public}x observer:0x%{public}x", status, remote,
                StoreUtil::Anonymous(obj));
        }

        delete obj;
    };
}

std::shared_ptr<ObserverBridge> SingleStoreImpl::PutIn(uint32_t &realType, std::shared_ptr<Observer> observer)
{
    std::shared_ptr<ObserverBridge> bridge = nullptr;
    observers_.Compute(uintptr_t(observer.get()),
        [this, &realType, observer, &bridge](const auto &, std::pair<uint32_t, std::shared_ptr<ObserverBridge>> &pair) {
            if ((pair.first & realType) == realType) {
                realType = (realType & (~pair.first));
                return (pair.first != 0);
            }

            if (observers_.Size() > MAX_OBSERVER_SIZE) {
                return false;
            }

            if (pair.first == 0) {
                auto release = BridgeReleaser();
                StoreId storeId{ storeId_ };
                AppId appId{ appId_ };
                pair.second = { new ObserverBridge(appId, storeId, observer, convertor_), release };
            }
            bridge = pair.second;
            realType = (realType & (~pair.first));
            pair.first = pair.first | realType;
            return (pair.first != 0);
        });
    return bridge;
}

std::shared_ptr<ObserverBridge> SingleStoreImpl::TakeOut(uint32_t &realType, std::shared_ptr<Observer> observer)
{
    std::shared_ptr<ObserverBridge> bridge = nullptr;
    observers_.ComputeIfPresent(uintptr_t(observer.get()),
        [&realType, observer, &bridge](const auto &, std::pair<uint32_t, std::shared_ptr<ObserverBridge>> &pair) {
            if ((pair.first & realType) == 0) {
                return (pair.first != 0);
            }
            realType = (realType & pair.first);
            pair.first = (pair.first & (~realType));
            bridge = pair.second;
            return (pair.first != 0);
        });
    return bridge;
}

Status SingleStoreImpl::GetResultSet(const DBQuery &query, std::shared_ptr<ResultSet> &resultSet) const
{
    std::shared_lock<decltype(rwMutex_)> lock(rwMutex_);
    if (dbStore_ == nullptr) {
        ZLOGE("db:%{public}s already closed!", storeId_.c_str());
        return ALREADY_CLOSED;
    }

    DistributedDB::KvStoreResultSet *dbResultSet = nullptr;
    auto status = dbStore_->GetEntries(query, dbResultSet);
    if (dbResultSet == nullptr) {
        return StoreUtil::ConvertStatus(status);
    }
    resultSet = std::make_shared<StoreResultSet>(dbResultSet, dbStore_, convertor_);
    return SUCCESS;
}

Status SingleStoreImpl::GetEntries(const DBQuery &query, std::vector<Entry> &entries) const
{
    std::shared_lock<decltype(rwMutex_)> lock(rwMutex_);
    if (dbStore_ == nullptr) {
        ZLOGE("db:%{public}s already closed!", storeId_.c_str());
        return ALREADY_CLOSED;
    }

    std::vector<DBEntry> dbEntries;
    std::string deviceId;
    auto dbStatus = dbStore_->GetEntries(query, dbEntries);
    entries.resize(dbEntries.size());
    auto it = entries.begin();
    for (auto &dbEntry : dbEntries) {
        auto &entry = *it;
        entry.key = convertor_.ToKey(std::move(dbEntry.key), deviceId);
        entry.value = std::move(dbEntry.value);
        ++it;
    }

    auto status = StoreUtil::ConvertStatus(dbStatus);
    if (status == NOT_FOUND) {
        status = SUCCESS;
    }
    return status;
}

Status SingleStoreImpl::DoSync(const SyncInfo &syncInfo, std::shared_ptr<SyncCallback> observer)
{
    auto service = KVDBServiceClient::GetInstance();
    if (service == nullptr) {
        return SERVER_UNAVAILABLE;
    }

    auto syncAgent = service->GetSyncAgent({ appId_ });
    if (syncAgent == nullptr) {
        ZLOGE("failed! invalid agent app:%{public}s store:%{public}s!", appId_.c_str(), storeId_.c_str());
        return ILLEGAL_STATE;
    }

    syncAgent->AddSyncCallback(observer, syncInfo.seqId);
    auto status = service->Sync({ appId_ }, { storeId_ }, syncInfo);
    if (status != Status::SUCCESS) {
        syncAgent->DeleteSyncCallback(syncInfo.seqId);
    }
    return status;
}

void SingleStoreImpl::DoAutoSync()
{
    if (!autoSync_) {
        ZLOGD("[HP_DEBUG]DoAutoSync disable!");
        return;
    }
    ZLOGD("[HP_DEBUG]DoAutoSync start...");
    ZLOGD("app:%{public}s store:%{public}s!", appId_.c_str(), storeId_.c_str());
    AutoSyncTimer::GetInstance().DoAutoSync(appId_, { { storeId_ } });
    expiration_ = steady_clock::now() + seconds(interval_);
}

void SingleStoreImpl::Online(const std::string &device)
{
    if (!autoSync_ || steady_clock::now() >= expiration_) {
        return;
    }

    ZLOGI("device:%{public}s online app:%{public}s store:%{public}s Sync!", StoreUtil::Anonymous(device).c_str(),
        appId_.c_str(), storeId_.c_str());
    SyncInfo syncInfo;
    syncInfo.devices = { device };
    DoSync(syncInfo, nullptr);
}

void SingleStoreImpl::Offline(const std::string &device)
{
}
} // namespace OHOS::DistributedKv