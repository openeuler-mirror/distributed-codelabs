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

#include "kvdb_manager.h"
#include "log_print.h"
#include "db_common.h"
#include "runtime_context.h"
#include "schema_object.h"
#include "default_factory.h"
#include "generic_kvdb.h"
#include "db_constant.h"
#include "res_finalizer.h"

namespace DistributedDB {
const std::string KvDBManager::PROCESS_LABEL_CONNECTOR = "-";
std::atomic<KvDBManager *> KvDBManager::instance_{nullptr};
std::mutex KvDBManager::kvDBLock_;
std::mutex KvDBManager::instanceLock_;
std::map<std::string, OS::FileHandle> KvDBManager::locks_;
std::mutex KvDBManager::fileHandleMutex_;

namespace {
    DefaultFactory g_defaultFactory;

    int CreateDataBaseInstance(const KvDBProperties &property, IKvDB *&kvDB)
    {
        IKvDBFactory *factory = IKvDBFactory::GetCurrent();
        if (factory == nullptr) {
            return -E_OUT_OF_MEMORY;
        }
        int errCode = E_OK;
        int databaseType = property.GetIntProp(KvDBProperties::DATABASE_TYPE, KvDBProperties::LOCAL_TYPE);
        if (databaseType == KvDBProperties::LOCAL_TYPE) {
            kvDB = factory->CreateKvDb(LOCAL_KVDB, errCode);
            if (kvDB != nullptr) {
                kvDB->EnableAutonomicUpgrade();
            }
        } else if (databaseType == KvDBProperties::SINGLE_VER_TYPE) {
            kvDB = factory->CreateKvDb(SINGER_VER_KVDB, errCode);
        } else {
            kvDB = factory->CreateKvDb(MULTI_VER_KVDB, errCode);
        }
        return errCode;
    }

    int CreateRemoveStateFlagFile(const KvDBProperties &properties)
    {
        std::string dataDir = properties.GetStringProp(KvDBProperties::DATA_DIR, "");
        std::string identifier = properties.GetStringProp(KvDBProperties::IDENTIFIER_DATA, "");
        std::string identifierName = DBCommon::TransferStringToHex(identifier);
        std::string storeDir = dataDir + "/" + identifierName + DBConstant::DELETE_KVSTORE_REMOVING;
        if (OS::CheckPathExistence(storeDir)) {
            return E_OK;
        }
        // create the pre flag file.
        int errCode = OS::CreateFileByFileName(storeDir);
        if (errCode != E_OK) {
            LOGE("Create remove state flag file failed:%d.", errCode);
        }
        return errCode;
    }
}

int KvDBManager::CheckRemoveStateAndRetry(const KvDBProperties &property)
{
    std::string dataDir = property.GetStringProp(KvDBProperties::DATA_DIR, "");
    std::string identifier = property.GetStringProp(KvDBProperties::IDENTIFIER_DATA, "");
    std::string identifierName = DBCommon::TransferStringToHex(identifier);
    std::string storeDir = dataDir + "/" + identifierName + DBConstant::DELETE_KVSTORE_REMOVING;

    if (OS::CheckPathExistence(storeDir)) {
        KvDBManager::ExecuteRemoveDatabase(property);
    }
    // Re-detection deleted had been finish
    if (OS::CheckPathExistence(storeDir)) {
        LOGD("Deletekvstore unfinished, can not create new same identifier kvstore!");
        return -E_REMOVE_FILE;
    }
    return E_OK;
}

int KvDBManager::ExecuteRemoveDatabase(const KvDBProperties &properties)
{
    int errCode = CheckDatabaseFileStatus(properties);
    if (errCode != E_OK) {
        return errCode;
    }
    IKvDBFactory *factory = IKvDBFactory::GetCurrent();
    if (factory == nullptr) {
        return -E_INVALID_DB;
    }

    errCode = CreateRemoveStateFlagFile(properties);
    if (errCode != E_OK) {
        LOGE("create ctrl file failed:%d.", errCode);
        return errCode;
    }

    errCode = -E_NOT_FOUND;
    for (KvDBType kvDbType = LOCAL_KVDB; kvDbType < UNSUPPORT_KVDB_TYPE; kvDbType = (KvDBType)(kvDbType + 1)) {
        int innerErrCode = E_OK;
        IKvDB *kvdb = factory->CreateKvDb(kvDbType, innerErrCode);
        if (innerErrCode != E_OK) {
            return innerErrCode;
        }
        innerErrCode = kvdb->RemoveKvDB(properties);
        RefObject::DecObjRef(kvdb);
        if (innerErrCode != -E_NOT_FOUND) {
            if (innerErrCode != E_OK) {
                return innerErrCode;
            }
            errCode = E_OK;
        }
    }

    if (errCode == -E_NOT_FOUND) {
        LOGE("DataBase file Not exist! return NOT_FOUND.");
    }

    RemoveDBDirectory(properties);
    return errCode;
}

void KvDBManager::RemoveDBDirectory(const KvDBProperties &properties)
{
    std::string dataDir = properties.GetStringProp(KvDBProperties::DATA_DIR, "");
    std::string identifier = properties.GetStringProp(KvDBProperties::IDENTIFIER_DATA, "");
    std::string identifierName = DBCommon::TransferStringToHex(identifier);
    std::string storeDir = dataDir + "/" + identifierName;
    std::string removingFlag = dataDir + "/" + identifierName + DBConstant::DELETE_KVSTORE_REMOVING;
    (void)OS::RemoveDBDirectory(storeDir);

    std::string storeId = properties.GetStringProp(KvDBProperties::STORE_ID, "");
    identifier = DBCommon::TransferHashString(storeId);
    identifierName = DBCommon::TransferStringToHex(identifier);
    storeDir = dataDir + "/" + identifierName;
    (void)OS::RemoveDBDirectory(storeDir);

    (void)OS::RemoveFile(removingFlag);
}

// Used to open a kvdb with the given property
IKvDB *KvDBManager::OpenDatabase(const KvDBProperties &property, int &errCode)
{
    KvDBManager *manager = GetInstance();
    if (manager == nullptr) {
        errCode = -E_OUT_OF_MEMORY;
        return nullptr;
    }
    return manager->GetDataBase(property, errCode, true);
}

void KvDBManager::EnterDBOpenCloseProcess(const std::string &identifier)
{
    std::unique_lock<std::mutex> lock(kvDBOpenMutex_);
    kvDBOpenCondition_.wait(lock, [this, &identifier]() {
        return this->kvDBOpenSet_.count(identifier) == 0;
    });
    (void)kvDBOpenSet_.insert(identifier);
}

void KvDBManager::ExitDBOpenCloseProcess(const std::string &identifier)
{
    std::unique_lock<std::mutex> lock(kvDBOpenMutex_);
    (void)kvDBOpenSet_.erase(identifier);
    kvDBOpenCondition_.notify_all();
}

// one time 100ms
// In order to prevent long-term blocking of the process, a retry method is used
// The dimensions of the lock by appid-userid-storeid
int KvDBManager::TryLockDB(const KvDBProperties &kvDBProp, int retryTimes)
{
    std::string dataDir = kvDBProp.GetStringProp(KvDBProperties::DATA_DIR, "");
    bool isMemoryDb = kvDBProp.GetBoolProp(KvDBProperties::MEMORY_MODE, false);
    std::string id = KvDBManager::GenerateKvDBIdentifier(kvDBProp);
    if (dataDir.back() != '/') {
        dataDir += "/";
    }

    if (isMemoryDb) {
        LOGI("MemoryDb not need lock!");
        return E_OK;
    }

    {
        std::lock_guard<std::mutex> autoLock(fileHandleMutex_);
        if (locks_.count(id) != 0) {
            LOGI("db has been locked!");
            return E_OK;
        }
    }

    std::string hexHashId = DBCommon::TransferStringToHex((id));
    OS::FileHandle handle;
    int errCode = OS::OpenFile(dataDir + hexHashId + DBConstant::DB_LOCK_POSTFIX, handle);
    if (errCode != E_OK) {
        LOGE("Open lock file fail errCode = [%d], errno:%d", errCode, errno);
        return errCode;
    }

    while (retryTimes-- > 0) {
        errCode = OS::FileLock(handle, false); // not block process
        if (errCode == E_OK) {
            LOGI("[%s]locked!", STR_MASK(DBCommon::TransferStringToHex(KvDBManager::GenerateKvDBIdentifier(kvDBProp))));
            std::lock_guard<std::mutex> autoLock(fileHandleMutex_);
            locks_[id] = handle;
            return errCode;
        } else if (errCode == -E_BUSY) {
            LOGD("DB already held by process lock!");
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // wait for 100ms
            continue;
        } else {
            LOGE("Try lock db failed, errCode = [%d] errno:%d", errCode, errno);
            OS::CloseFile(handle);
            return errCode;
        }
    }
    OS::CloseFile(handle);
    return -E_BUSY;
}

int KvDBManager::UnlockDB(const KvDBProperties &kvDBProp)
{
    bool isMemoryDb = kvDBProp.GetBoolProp(KvDBProperties::MEMORY_MODE, false);
    if (isMemoryDb) {
        return E_OK;
    }
    std::string identifierDir = KvDBManager::GenerateKvDBIdentifier(kvDBProp);
    std::lock_guard<std::mutex> autoLock(fileHandleMutex_);
    if (locks_.count(identifierDir) == 0) {
        return E_OK;
    }
    int errCode = OS::FileUnlock(locks_[identifierDir]);
    LOGI("DB unlocked! errCode = [%d]", errCode);
    if (errCode != E_OK) {
        return errCode;
    }
    locks_.erase(identifierDir);
    return E_OK;
}

bool KvDBManager::CheckOpenDBOptionWithCached(const KvDBProperties &properties, IKvDB *kvDB)
{
    bool isMemoryDb = properties.GetBoolProp(KvDBProperties::MEMORY_MODE, false);
    std::string canonicalDir = properties.GetStringProp(KvDBProperties::DATA_DIR, "");
    if (!isMemoryDb && (canonicalDir.empty() || canonicalDir != kvDB->GetStorePath())) {
        LOGE("Failed to check store path, the input path does not match with cached store.");
        return false;
    }

    bool compressOnSyncUser = properties.GetBoolProp(KvDBProperties::COMPRESS_ON_SYNC, false);
    bool compressOnSyncGet = kvDB->GetMyProperties().GetBoolProp(KvDBProperties::COMPRESS_ON_SYNC, false);
    if (compressOnSyncUser != compressOnSyncGet) {
        LOGE("Failed to check compress option, the input %d not match with cached %d.", compressOnSyncUser,
            compressOnSyncGet);
        return false;
    }
    if (compressOnSyncUser) {
        int compressRateUser = properties.GetIntProp(KvDBProperties::COMPRESSION_RATE, 0);
        int compressRateGet = kvDB->GetMyProperties().GetIntProp(KvDBProperties::COMPRESSION_RATE, 0);
        if (compressRateUser != compressRateGet) {
            LOGE("Failed to check compress rate, the input %d not match with cached %d.", compressRateUser,
                compressRateGet);
            return false;
        }
    }
    return true;
}

// Used to open a kvdb with the given property
IKvDBConnection *KvDBManager::GetDatabaseConnection(const KvDBProperties &properties, int &errCode,
    bool isNeedIfOpened)
{
    auto manager = GetInstance();
    if (manager == nullptr) {
        errCode = -E_OUT_OF_MEMORY;
        return nullptr;
    }
    IKvDBConnection *connection = nullptr;
    std::string identifier = properties.GetStringProp(KvDBProperties::IDENTIFIER_DATA, "");
    LOGD("Begin to get [%s] database connection.", STR_MASK(DBCommon::TransferStringToHex(identifier)));
    manager->EnterDBOpenCloseProcess(identifier);

    IKvDB *kvDB = manager->GetDataBase(properties, errCode, isNeedIfOpened);
    if (kvDB == nullptr) {
        if (isNeedIfOpened) {
            LOGE("Failed to open the db:%d", errCode);
        }
    } else {
        if (!CheckOpenDBOptionWithCached(properties, kvDB)) {
            LOGE("Failed to check open db option");
            errCode = -E_INVALID_ARGS;
        } else {
            connection = kvDB->GetDBConnection(errCode);
            if (connection == nullptr) { // not kill kvdb, Other operations like import may be used concurrently
                LOGE("Failed to get the db connect for delegate:%d", errCode);
            }
        }
        RefObject::DecObjRef(kvDB); // restore the reference increased by the cache.
        kvDB = nullptr;
    }

    manager->ExitDBOpenCloseProcess(identifier);
    if (errCode == -E_INVALID_PASSWD_OR_CORRUPTED_DB) {
        std::string appId = properties.GetStringProp(KvDBProperties::APP_ID, "");
        std::string userId = properties.GetStringProp(KvDBProperties::USER_ID, "");
        std::string storeId = properties.GetStringProp(KvDBProperties::STORE_ID, "");
        manager->DataBaseCorruptNotify(appId, userId, storeId);
        LOGE("Database [%s] is corrupted:%d", STR_MASK(DBCommon::TransferStringToHex(identifier)), errCode);
    }

    return connection;
}

int KvDBManager::ReleaseDatabaseConnection(IKvDBConnection *connection)
{
    if (connection == nullptr) {
        return -E_INVALID_DB;
    }

    std::string identifier = connection->GetIdentifier();
    auto manager = GetInstance();
    if (manager == nullptr) {
        return -E_OUT_OF_MEMORY;
    }
    manager->EnterDBOpenCloseProcess(identifier);
    int errCode = connection->Close();
    manager->ExitDBOpenCloseProcess(identifier);

    if (errCode != E_OK) {
        LOGE("[KvDBManager] Release db connection:%d", errCode);
    }
    LOGI("[Connection] db[%s] conn Close", STR_MASK(DBCommon::TransferStringToHex(identifier)));
    return errCode;
}

IKvDB *KvDBManager::CreateDataBase(const KvDBProperties &property, int &errCode)
{
    IKvDB *kvDB = OpenNewDatabase(property, errCode);
    if (kvDB == nullptr) {
        LOGE("Failed to open the new database.");
        if (errCode == -E_INVALID_PASSWD_OR_CORRUPTED_DB &&
            property.GetBoolProp(KvDBProperties::RM_CORRUPTED_DB, false)) {
            LOGI("Remove the corrupted database while open");
            ExecuteRemoveDatabase(property);
            kvDB = OpenNewDatabase(property, errCode);
        }
        return kvDB;
    }

    if (property.GetBoolProp(KvDBProperties::CHECK_INTEGRITY, false)) {
        int integrityStatus = kvDB->CheckIntegrity();
        if (integrityStatus == -E_INVALID_PASSWD_OR_CORRUPTED_DB) {
            RemoveKvDBFromCache(kvDB);
            RefObject::KillAndDecObjRef(kvDB);
            kvDB = nullptr;
            errCode = -E_INVALID_PASSWD_OR_CORRUPTED_DB;
            if (property.GetBoolProp(KvDBProperties::RM_CORRUPTED_DB, false)) {
                LOGI("Remove the corrupted database for the integrity check");
                ExecuteRemoveDatabase(property);
                kvDB = OpenNewDatabase(property, errCode);
            }
        }
    }
    return kvDB;
}

IKvDB *KvDBManager::GetDataBase(const KvDBProperties &property, int &errCode, bool isNeedIfOpened)
{
    bool isMemoryDb = property.GetBoolProp(KvDBProperties::MEMORY_MODE, false);
    bool isCreateNecessary = property.GetBoolProp(KvDBProperties::CREATE_IF_NECESSARY, true);
    IKvDB *kvDB = FindAndGetKvDBFromCache(property, errCode);
    if (kvDB != nullptr) {
        if (!isNeedIfOpened) {
            LOGI("[KvDBManager] Database has already been opened.");
            RefObject::DecObjRef(kvDB);
            errCode = -E_ALREADY_OPENED;
            kvDB = nullptr;
        }
        return kvDB;
    }
    if (isMemoryDb && !isCreateNecessary) {
        LOGI("IsCreateNecessary is false, Not need create database");
        return nullptr;
    }
    if (errCode != -E_NOT_FOUND) {
        return nullptr;
    }

    // Taking into account the compatibility of version delivery,
    // temporarily use isNeedIntegrityCheck this field to avoid multi-process concurrency
    bool isNeedIntegrityCheck = property.GetBoolProp(KvDBProperties::CHECK_INTEGRITY, false);
    if (isNeedIntegrityCheck) {
        LOGI("db need lock, need check integrity is [%d]", isNeedIntegrityCheck);
        errCode = KvDBManager::TryLockDB(property, 10);  // default 10 times retry
        if (errCode != E_OK) {
            return nullptr;
        }
    }

    ResFinalizer unlock([&errCode, &property, &kvDB]() {
        int err = KvDBManager::UnlockDB(property);
        if (err != E_OK) {
            LOGE("GetDataBase unlock failed! err [%d] errCode [%d]", err, errCode);
            errCode = err;
            RefObject::KillAndDecObjRef(kvDB);
            kvDB = nullptr;
        }
    });

    kvDB = CreateDataBase(property, errCode);
    if (errCode != E_OK) {
        LOGE("Create data base failed, errCode = [%d]", errCode);
    }
    return kvDB;
}

bool KvDBManager::IsOpenMemoryDb(const KvDBProperties &properties, const std::map<std::string, IKvDB *> &cache) const
{
    std::string identifier = GenerateKvDBIdentifier(properties);
    auto iter = cache.find(identifier);
    if (iter != cache.end()) {
        IKvDB *kvDB = iter->second;
        if (kvDB != nullptr && kvDB->GetMyProperties().GetBoolProp(KvDBProperties::MEMORY_MODE, false)) {
            return true;
        }
    }
    return false;
}

// used to get kvdb size with the given property.
int KvDBManager::CalculateKvStoreSize(const KvDBProperties &properties, uint64_t &size)
{
    KvDBManager *manager = GetInstance();
    if (manager == nullptr) {
        LOGE("Failed to get KvDBManager instance!");
        return -E_OUT_OF_MEMORY;
    }

    std::lock_guard<std::mutex> lockGuard(kvDBLock_);
    if (manager->IsOpenMemoryDb(properties, manager->singleVerNaturalStores_)) {
        size = 0;
        return E_OK;
    }

    IKvDBFactory *factory = IKvDBFactory::GetCurrent();
    if (factory == nullptr) {
        return -E_INVALID_DB;
    }

    uint64_t totalSize = 0;
    for (KvDBType kvDbType = LOCAL_KVDB; kvDbType < UNSUPPORT_KVDB_TYPE; kvDbType = (KvDBType)(kvDbType + 1)) {
        int innerErrCode = E_OK;
        IKvDB *kvDB = factory->CreateKvDb(kvDbType, innerErrCode);
        if (innerErrCode != E_OK) {
            return innerErrCode;
        }
        uint64_t dbSize = 0;
        innerErrCode = kvDB->GetKvDBSize(properties, dbSize);
        RefObject::DecObjRef(kvDB);
        if (innerErrCode != E_OK && innerErrCode != -E_NOT_FOUND) {
            return innerErrCode;
        }
        LOGD("DB type [%u], size[%" PRIu64 "]", static_cast<unsigned>(kvDbType), dbSize);
        totalSize = totalSize + dbSize;
    }
    // This represent Db file size(Unit is byte), It is small than max size(max uint64_t represent 2^64B)
    if (totalSize != 0ull) {
        size = totalSize;
        return E_OK;
    }
    return -E_NOT_FOUND;
}

IKvDB *KvDBManager::GetKvDBFromCacheByIdentify(const std::string &identifier,
    const std::map<std::string, IKvDB *> &cache) const
{
    auto iter = cache.find(identifier);
    if (iter != cache.end()) {
        IKvDB *kvDB = iter->second;
        if (kvDB == nullptr) {
            LOGE("Kvstore cache is nullptr, there may be a logic error");
            return nullptr;
        }
        return kvDB;
    }
    return nullptr;
}

int KvDBManager::CheckDatabaseFileStatus(const KvDBProperties &properties)
{
    KvDBManager *manager = GetInstance();
    if (manager == nullptr) {
        LOGE("Failed to get KvDBManager instance!");
        return -E_OUT_OF_MEMORY;
    }

    std::string identifier = GenerateKvDBIdentifier(properties);
    std::lock_guard<std::mutex> lockGuard(kvDBLock_);
    IKvDB *kvDB = manager->GetKvDBFromCacheByIdentify(identifier, manager->localKvDBs_);
    if (kvDB != nullptr) {
        LOGE("The local KvDB is busy!");
        return -E_BUSY;
    }

    kvDB = manager->GetKvDBFromCacheByIdentify(identifier, manager->multiVerNaturalStores_);
    if (kvDB != nullptr) {
        LOGE("The multi ver natural store is busy!");
        return -E_BUSY;
    }

    kvDB = manager->GetKvDBFromCacheByIdentify(identifier, manager->singleVerNaturalStores_);
    if (kvDB != nullptr) {
        LOGE("The single version natural store is busy!");
        return -E_BUSY;
    }
    return E_OK;
}

IKvDB *KvDBManager::OpenNewDatabase(const KvDBProperties &property, int &errCode)
{
    errCode = CheckRemoveStateAndRetry(property);
    if (errCode != E_OK) {
        LOGE("Failed to open IKvDB! Because delete kvstore unfinished err:%d", errCode);
        return nullptr;
    }

    IKvDB *kvDB = nullptr;
    errCode = CreateDataBaseInstance(property, kvDB);
    if (errCode != E_OK) {
        LOGE("Failed to get IKvDB! err:%d", errCode);
        return nullptr;
    }

    errCode = kvDB->Open(property);
    if (errCode != E_OK) {
        LOGE("Failed to open IKvDB! err:%d", errCode);
        RefObject::KillAndDecObjRef(kvDB);
        kvDB = nullptr;
        return nullptr;
    }
    auto identifier = DBCommon::TransferStringToHex(property.GetStringProp(KvDBProperties::IDENTIFIER_DATA, ""));
    auto dbDir = property.GetStringProp(KvDBProperties::IDENTIFIER_DIR, "");
    LOGI("Database identifier:%.6s, dir:%.6s", identifier.c_str(), dbDir.c_str());
    // Register the callback function when the database is closed, triggered when kvdb free
    kvDB->OnClose([kvDB, this]() {
        LOGI("Remove from the cache");
        this->RemoveKvDBFromCache(kvDB);
    });

    IKvDB *kvDBTmp = SaveKvDBToCache(kvDB);
    if (kvDBTmp != kvDB) {
        RefObject::KillAndDecObjRef(kvDB);
        kvDB = nullptr;
        return kvDBTmp;
    }
    return kvDB;
}

// used to delete a kvdb with the given property.
// return BUSY if in use
int KvDBManager::RemoveDatabase(const KvDBProperties &properties)
{
    KvDBManager *manager = GetInstance();
    if (manager == nullptr) {
        LOGE("Failed to get kvdb manager while removing the db!");
        return -E_OUT_OF_MEMORY;
    }
    std::string identifier = GenerateKvDBIdentifier(properties);
    manager->EnterDBOpenCloseProcess(identifier);

    LOGI("KvDBManager::RemoveDatabase begin try lock the database!");
    std::string lockFile = properties.GetStringProp(KvDBProperties::DATA_DIR, "") + "/" +
        DBCommon::TransferStringToHex(identifier) + DBConstant::DB_LOCK_POSTFIX;
    int errCode = E_OK;
    if (OS::CheckPathExistence(lockFile)) {
        errCode = KvDBManager::TryLockDB(properties, 10); // default 10 times retry
        if (errCode != E_OK) {
            manager->ExitDBOpenCloseProcess(identifier);
            return errCode;
        }
    }

    errCode = ExecuteRemoveDatabase(properties);
    if (errCode != E_OK) {
        LOGE("[KvDBManager] Remove database failed:%d", errCode);
    }
    int err = KvDBManager::UnlockDB(properties); // unlock and delete lock file before delete dir
    if (err != E_OK) {
        LOGE("[KvDBManager][RemoveDatabase] UnlockDB failed:%d, errno:%d", err, errno);
        errCode = err;
    }

    manager->ExitDBOpenCloseProcess(identifier);
    return errCode;
}

std::string KvDBManager::GenerateKvDBIdentifier(const KvDBProperties &property)
{
    return property.GetStringProp(KvDBProperties::IDENTIFIER_DATA, "");
}

KvDBManager *KvDBManager::GetInstance()
{
    // For Double-Checked Locking, we need check instance_ twice
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lockGuard(instanceLock_);
        if (instance_ == nullptr) {
            instance_ = new (std::nothrow) KvDBManager();
            if (instance_ == nullptr) {
                LOGE("failed to new KvDBManager!");
                return nullptr;
            }
        }
    }
    if (IKvDBFactory::GetCurrent() == nullptr) {
        IKvDBFactory::Register(&g_defaultFactory);
    }
    return instance_;
}

// Save to IKvDB to the global map
IKvDB *KvDBManager::SaveKvDBToCache(IKvDB *kvDB)
{
    if (kvDB == nullptr) {
        return nullptr;
    }

    {
        KvDBProperties properties = kvDB->GetMyProperties();
        std::string identifier = GenerateKvDBIdentifier(properties);
        int databaseType = properties.GetIntProp(KvDBProperties::DATABASE_TYPE, KvDBProperties::LOCAL_TYPE);
        std::lock_guard<std::mutex> lockGuard(kvDBLock_);
        int errCode = E_OK;
        if (databaseType == KvDBProperties::LOCAL_TYPE) {
            IKvDB *kvDBTmp = FindKvDBFromCache(properties, localKvDBs_, true, errCode);
            if (kvDBTmp != nullptr) {
                kvDBTmp->IncObjRef(kvDBTmp);
                return kvDBTmp;
            }
            localKvDBs_.insert(std::pair<std::string, IKvDB *>(identifier, kvDB));
        } else if (databaseType == KvDBProperties::MULTI_VER_TYPE) {
            IKvDB *kvDBTmp = FindKvDBFromCache(properties, multiVerNaturalStores_, true, errCode);
            if (kvDBTmp != nullptr) {
                kvDBTmp->IncObjRef(kvDBTmp);
                return kvDBTmp;
            }
            kvDB->WakeUpSyncer();
            multiVerNaturalStores_.insert(std::pair<std::string, IKvDB *>(identifier, kvDB));
        } else {
            IKvDB *kvDBTmp = FindKvDBFromCache(properties, singleVerNaturalStores_, true, errCode);
            if (kvDBTmp != nullptr) {
                kvDBTmp->IncObjRef(kvDBTmp);
                return kvDBTmp;
            }
            kvDB->WakeUpSyncer();
            singleVerNaturalStores_.insert(std::pair<std::string, IKvDB *>(identifier, kvDB));
        }
    }
    kvDB->SetCorruptHandler([kvDB, this]() {
        std::string appId = kvDB->GetMyProperties().GetStringProp(KvDBProperties::APP_ID, "");
        std::string userId = kvDB->GetMyProperties().GetStringProp(KvDBProperties::USER_ID, "");
        std::string storeId = kvDB->GetMyProperties().GetStringProp(KvDBProperties::STORE_ID, "");
        this->DataBaseCorruptNotifyAsync(appId, userId, storeId);
    });
    return kvDB;
}

// Save to IKvDB to the global map
void KvDBManager::RemoveKvDBFromCache(const IKvDB *kvDB)
{
    KvDBProperties properties = kvDB->GetMyProperties();
    std::string identifier = GenerateKvDBIdentifier(properties);
    int databaseType = properties.GetIntProp(KvDBProperties::DATABASE_TYPE, KvDBProperties::LOCAL_TYPE);
    std::lock_guard<std::mutex> lockGuard(kvDBLock_);
    if (databaseType == KvDBProperties::LOCAL_TYPE) {
        localKvDBs_.erase(identifier);
    } else if (databaseType == KvDBProperties::MULTI_VER_TYPE) {
        multiVerNaturalStores_.erase(identifier);
    } else {
        singleVerNaturalStores_.erase(identifier);
    }
}

// Get IKvDB from the global map
IKvDB *KvDBManager::FindAndGetKvDBFromCache(const KvDBProperties &properties, int &errCode) const
{
    std::lock_guard<std::mutex> lockGuard(kvDBLock_);

    IKvDB *kvDB = FindKvDBFromCache(properties, localKvDBs_, true, errCode);
    if (kvDB != nullptr) {
        kvDB->IncObjRef(kvDB);
        return kvDB;
    }
    if (errCode != -E_NOT_FOUND) {
        return nullptr;
    }

    kvDB = FindKvDBFromCache(properties, multiVerNaturalStores_, true, errCode);
    if (kvDB != nullptr) {
        kvDB->IncObjRef(kvDB);
        return kvDB;
    }
    if (errCode != -E_NOT_FOUND) {
        return nullptr;
    }

    kvDB = FindKvDBFromCache(properties, singleVerNaturalStores_, true, errCode);
    if (kvDB != nullptr) {
        kvDB->IncObjRef(kvDB);
        return kvDB;
    }
    return nullptr;
}

IKvDB *KvDBManager::FindKvDBFromCache(const KvDBProperties &properties, const std::map<std::string, IKvDB *> &cache,
    bool isNeedCheckPasswd, int &errCode) const
{
    errCode = E_OK;
    std::string identifier = GenerateKvDBIdentifier(properties);
    auto iter = cache.find(identifier);
    if (iter != cache.end()) {
        IKvDB *kvDB = iter->second;
        if (kvDB == nullptr) {
            LOGE("KVSTORE cache is nullptr, there may be a logic error");
            errCode = -E_INTERNAL_ERROR;
            return nullptr;
        }
        int newType = properties.GetIntProp(KvDBProperties::DATABASE_TYPE, KvDBProperties::LOCAL_TYPE);
        int oldType = kvDB->GetMyProperties().GetIntProp(KvDBProperties::DATABASE_TYPE, KvDBProperties::LOCAL_TYPE);
        if (oldType == newType) {
            errCode = CheckKvDBProperties(kvDB, properties, isNeedCheckPasswd);
            if (errCode != E_OK) {
                return nullptr;
            }
            return kvDB;
        } else {
            errCode = -E_INVALID_ARGS;
            LOGE("Database [%s] type not matched, type [%d] vs [%d]",
                STR_MASK(DBCommon::TransferStringToHex(identifier)), newType, oldType);
            return nullptr;
        }
    }

    errCode = -E_NOT_FOUND;
    return nullptr;
}

int KvDBManager::SetProcessLabel(const std::string &appId, const std::string &userId)
{
    std::string label = appId + PROCESS_LABEL_CONNECTOR + userId;
    RuntimeContext::GetInstance()->SetProcessLabel(label);
    return E_OK;
}

void KvDBManager::RestoreSyncableKvStore()
{
    KvDBManager *manager = GetInstance();
    if (manager == nullptr) {
        return;
    }

    manager->RestoreSyncerOfAllKvStore();
}

void KvDBManager::SetDatabaseCorruptionHandler(const KvStoreCorruptionHandler &handler)
{
    KvDBManager *manager = GetInstance();
    if (manager == nullptr) {
        return;
    }

    manager->SetAllDatabaseCorruptionHander(handler);
}

void KvDBManager::SetAllDatabaseCorruptionHander(const KvStoreCorruptionHandler &handler)
{
    {
        std::lock_guard<std::mutex> lock(corruptMutex_);
        corruptHandler_ = handler;
    }
    std::lock_guard<std::mutex> lockGuard(kvDBLock_);
    SetCorruptHandlerForDatabases(singleVerNaturalStores_);
    SetCorruptHandlerForDatabases(localKvDBs_);
    SetCorruptHandlerForDatabases(multiVerNaturalStores_);
}

void KvDBManager::DataBaseCorruptNotify(const std::string &appId, const std::string &userId, const std::string &storeId)
{
    KvStoreCorruptionHandler corruptHandler = nullptr;
    {
        std::lock_guard<std::mutex> lock(corruptMutex_);
        corruptHandler = corruptHandler_;
    }

    if (corruptHandler != nullptr) {
        corruptHandler(appId, userId, storeId);
    }
}

void KvDBManager::DataBaseCorruptNotifyAsync(const std::string &appId, const std::string &userId,
    const std::string &storeId)
{
    int errCode = RuntimeContext::GetInstance()->ScheduleTask(
        std::bind(&KvDBManager::DataBaseCorruptNotify, this, appId, userId, storeId));
    if (errCode != E_OK) {
        LOGE("[KvDBManager][CorruptNotify] ScheduleTask failed, errCode = %d.", errCode);
        return;
    }
}

void KvDBManager::SetCorruptHandlerForDatabases(const std::map<std::string, IKvDB *> &dbMaps)
{
    for (const auto &item : dbMaps) {
        if (item.second == nullptr) {
            continue;
        }

        item.second->SetCorruptHandler([item, this]() {
            std::string appId = item.second->GetMyProperties().GetStringProp(KvDBProperties::APP_ID, "");
            std::string userId = item.second->GetMyProperties().GetStringProp(KvDBProperties::USER_ID, "");
            std::string storeId = item.second->GetMyProperties().GetStringProp(KvDBProperties::STORE_ID, "");
            this->DataBaseCorruptNotifyAsync(appId, userId, storeId);
        });
    }
}

void KvDBManager::RestoreSyncerOfAllKvStore()
{
    std::lock_guard<std::mutex> lockGuard(kvDBLock_);
    for (auto &item : singleVerNaturalStores_) {
        if (item.second != nullptr) {
            item.second->WakeUpSyncer();
        }
    }

    for (auto &item : multiVerNaturalStores_) {
        if (item.second != nullptr) {
            item.second->WakeUpSyncer();
        }
    }
}

bool KvDBManager::CompareSchemaObject(const SchemaObject &newSchema, const SchemaObject &oldSchema)
{
    if (!newSchema.IsSchemaValid() && !oldSchema.IsSchemaValid()) {
        return true;
    }
    if (!newSchema.IsSchemaValid() || !oldSchema.IsSchemaValid()) {
        return false;
    }
    int errCode = oldSchema.CompareAgainstSchemaObject(newSchema);
    if (errCode == -E_SCHEMA_EQUAL_EXACTLY) {
        return true;
    }
    return false;
}

int KvDBManager::CheckSchema(const IKvDB *kvDB, const KvDBProperties &properties)
{
    if (kvDB == nullptr) {
        LOGE("input kvdb is nullptr");
        return -E_INVALID_ARGS;
    }
    SchemaObject inputSchema = properties.GetSchema();
    SchemaObject cacheSchema = kvDB->GetMyProperties().GetSchema();
    bool isFirstOpenReadOnly =
        kvDB->GetMyProperties().GetBoolProp(KvDBProperties::FIRST_OPEN_IS_READ_ONLY, false);
    if (isFirstOpenReadOnly) {
        if (inputSchema.IsSchemaValid()) {
            LOGE("schema not matched");
            return -E_SCHEMA_MISMATCH;
        } else {
            return E_OK;
        }
    }
    if (!CompareSchemaObject(inputSchema, cacheSchema)) {
        LOGE("schema not matched");
        return -E_SCHEMA_MISMATCH;
    }
    return E_OK;
}

namespace {
    bool CheckSecOptions(const KvDBProperties &input, const KvDBProperties &existed)
    {
        // If any has NO_SET, skip the check and using the existed option.
        if (input.GetIntProp(KvDBProperties::SECURITY_LABEL, 0) != 0 &&
            existed.GetIntProp(KvDBProperties::SECURITY_LABEL, 0) != 0) {
            if (existed.GetIntProp(KvDBProperties::SECURITY_LABEL, 0) !=
                input.GetIntProp(KvDBProperties::SECURITY_LABEL, 0)) {
                LOGE("Security label mismatch: existed[%d] vs input[%d]",
                    existed.GetIntProp(KvDBProperties::SECURITY_LABEL, 0),
                    input.GetIntProp(KvDBProperties::SECURITY_LABEL, 0));
                return false;
            }
            if (existed.GetIntProp(KvDBProperties::SECURITY_FLAG, 0) !=
                input.GetIntProp(KvDBProperties::SECURITY_FLAG, 0)) {
                LOGE("Security flag mismatch: existed[%d] vs input[%d]",
                    existed.GetIntProp(KvDBProperties::SECURITY_FLAG, 0),
                    input.GetIntProp(KvDBProperties::SECURITY_FLAG, 0));
                return false;
            }
        }
        return true;
    }
}

int KvDBManager::CheckKvDBProperties(const IKvDB *kvDB, const KvDBProperties &properties,
    bool isNeedCheckPasswd) const
{
    // if get from cache is not memoryDb, do not support open or create memory DB
    bool isMemoryDb = properties.GetBoolProp(KvDBProperties::MEMORY_MODE, false);
    if (isMemoryDb != kvDB->GetMyProperties().GetBoolProp(KvDBProperties::MEMORY_MODE, false)) {
        LOGE("Already open same id physical DB, so do not support open or create memory DB");
        return -E_INVALID_ARGS;
    }

    if (kvDB->GetMyProperties().GetBoolProp(KvDBProperties::CREATE_DIR_BY_STORE_ID_ONLY, false) !=
        properties.GetBoolProp(KvDBProperties::CREATE_DIR_BY_STORE_ID_ONLY, false)) {
        LOGE("Different ways to create dir.");
        return -E_INVALID_ARGS;
    }

    if (kvDB->GetMyProperties().GetIntProp(KvDBProperties::CONFLICT_RESOLVE_POLICY, 0) !=
        properties.GetIntProp(KvDBProperties::CONFLICT_RESOLVE_POLICY, 0)) {
        LOGE("Different conflict resolve policy.");
        return -E_INVALID_ARGS;
    }

    if (kvDB->GetMyProperties().GetBoolProp(KvDBProperties::SYNC_DUAL_TUPLE_MODE, false) !=
        properties.GetBoolProp(KvDBProperties::SYNC_DUAL_TUPLE_MODE, false)) {
            LOGE("Different dual tuple sync mode");
            return -E_MODE_MISMATCH;
    }

    if (kvDB->GetMyProperties().GetBoolProp(KvDBProperties::LOCAL_ONLY, false) !=
        properties.GetBoolProp(KvDBProperties::LOCAL_ONLY, false)) {
        LOGE("Different local only mode");
        return -E_INVALID_ARGS;
    }

    if (!CheckSecOptions(properties, kvDB->GetMyProperties())) {
        return -E_INVALID_ARGS;
    }

    CipherType cacheType;
    CipherType inputType;
    CipherPassword cachePasswd;
    CipherPassword inputPasswd;
    kvDB->GetMyProperties().GetPassword(cacheType, cachePasswd);
    properties.GetPassword(inputType, inputPasswd);
    if (isNeedCheckPasswd && (cachePasswd != inputPasswd || !DBCommon::IsSameCipher(cacheType, inputType))) {
        LOGE("Identification not matched");
        return -E_INVALID_PASSWD_OR_CORRUPTED_DB;
    }

    return CheckSchema(kvDB, properties);
}

// Attention. After call FindKvDB and kvdb is not null, you need to call DecObjRef.
IKvDB* KvDBManager::FindKvDB(const std::string &identifier) const
{
    std::lock_guard<std::mutex> lockGuard(kvDBLock_);
    auto kvdb = singleVerNaturalStores_.find(identifier);
    if (kvdb != singleVerNaturalStores_.end()) {
        // Increase ref counter here.
        RefObject::IncObjRef(kvdb->second);
        return kvdb->second;
    }
    return nullptr;
}

void KvDBManager::Dump(int fd)
{
    std::lock_guard<std::mutex> lockGuard(kvDBLock_);
    for (const auto &entry : singleVerNaturalStores_) {
        RefObject::IncObjRef(entry.second);
        entry.second->Dump(fd);
        RefObject::DecObjRef(entry.second);
    }
}
} // namespace DistributedDB
