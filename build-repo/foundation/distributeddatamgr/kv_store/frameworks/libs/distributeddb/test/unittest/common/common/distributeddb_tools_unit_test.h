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

#ifndef DISTRIBUTEDDB_TOOLS_UNIT_TEST_H
#define DISTRIBUTEDDB_TOOLS_UNIT_TEST_H

#include <algorithm>
#include <condition_variable>
#include <dirent.h>
#include <mutex>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include "db_types.h"
#include "kv_store_changed_data.h"
#include "kv_store_delegate_impl.h"
#include "kv_store_delegate_manager.h"
#include "kv_store_nb_delegate.h"
#include "kv_store_observer.h"
#include "kv_store_snapshot_delegate_impl.h"
#include "log_print.h"
#include "message.h"
#include "query.h"
#include "relational_store_sqlite_ext.h"
#include "store_observer.h"
#include "store_changed_data.h"
#include "single_ver_kv_entry.h"
#include "sqlite_single_ver_natural_store.h"
#include "sqlite_utils.h"
#include "sync_types.h"
#include "store_types.h"
#include "types_export.h"
namespace DistributedDBUnitTest {
struct DatabaseInfo {
    std::string appId{};
    std::string userId{};
    std::string storeId{};
    std::string dir{};
    int dbUserVersion = 0;
};

struct SyncInputArg {
    uint64_t begin_{};
    uint64_t end_{};
    uint32_t blockSize_{};
    SyncInputArg(uint64_t begin, uint64_t end, uint32_t blockSize)
        : begin_(begin), end_(end), blockSize_(blockSize)
    {}
};

struct DataSyncMessageInfo {
    int messageId_ = DistributedDB::INVALID_MESSAGE_ID;
    uint16_t messageType_ = DistributedDB::TYPE_INVALID;
    uint32_t sequenceId_ = 0;
    uint32_t sessionId_ = 0;
    int sendCode_ = DistributedDB::E_OK;
    uint32_t version_ = 0;
    int32_t mode_ = DistributedDB::PUSH;
    DistributedDB::WaterMark localMark_ = 0;
    DistributedDB::WaterMark peerMark_ = 0;
    DistributedDB::WaterMark deleteMark_ = 0;
    uint64_t packetId_ = 0;
};

class DistributedDBToolsUnitTest final {
public:
    DistributedDBToolsUnitTest() {}
    ~DistributedDBToolsUnitTest() {}

    DistributedDBToolsUnitTest(const DistributedDBToolsUnitTest&) = delete;
    DistributedDBToolsUnitTest& operator=(const DistributedDBToolsUnitTest&) = delete;
    DistributedDBToolsUnitTest(DistributedDBToolsUnitTest&&) = delete;
    DistributedDBToolsUnitTest& operator=(DistributedDBToolsUnitTest&&) = delete;

    // compare whether two vectors are equal.
    template<typename T>
    static bool CompareVector(const std::vector<T>& vec1, const std::vector<T>& vec2)
    {
        if (vec1.size() != vec2.size()) {
            return false;
        }
        for (size_t i = 0; i < vec2.size(); i++) {
            if (vec1[i] != vec2[i]) {
                return false;
            }
        }
        return true;
    }

    // compare whether two vectors are equal.
    template<typename T>
    static bool CompareVectorN(const std::vector<T>& vec1, const std::vector<T>& vec2, uint32_t n)
    {
        if (n > std::min(vec1.size(), vec2.size())) {
            return false;
        }
        for (uint32_t i = 0; i < n; i++) {
            if (vec1[i] != vec2[i]) {
                return false;
            }
        }
        return true;
    }
    // init the test directory of dir.
    static void TestDirInit(std::string &dir);

    // remove the test db files in the test directory of dir.
    static int RemoveTestDbFiles(const std::string &dir);

    // callback function for get a KvStoreDelegate pointer.
    static void KvStoreDelegateCallback(DistributedDB::DBStatus, DistributedDB::KvStoreDelegate*,
        DistributedDB::DBStatus &, DistributedDB::KvStoreDelegate *&);

    // callback function for get a KvStoreDelegate pointer.
    static void KvStoreNbDelegateCallback(DistributedDB::DBStatus, DistributedDB::KvStoreNbDelegate*,
        DistributedDB::DBStatus &, DistributedDB::KvStoreNbDelegate *&);

    // callback function for get a KvStoreSnapshotDelegate pointer.
    static void SnapshotDelegateCallback(DistributedDB::DBStatus, DistributedDB::KvStoreSnapshotDelegate*,
        DistributedDB::DBStatus &, DistributedDB::KvStoreSnapshotDelegate *&);

    // callback function for get the value.
    static void ValueCallback(
        DistributedDB::DBStatus, const DistributedDB::Value &, DistributedDB::DBStatus &, DistributedDB::Value &);

    // callback function for get an entry vector.
    static void EntryVectorCallback(DistributedDB::DBStatus, const std::vector<DistributedDB::Entry> &,
        DistributedDB::DBStatus &, unsigned long &, std::vector<DistributedDB::Entry> &);

    // sync test helper
    DistributedDB::DBStatus SyncTest(DistributedDB::KvStoreNbDelegate* delegate,
        const std::vector<std::string>& devices, DistributedDB::SyncMode mode,
        std::map<std::string, DistributedDB::DBStatus>& statuses, bool wait = false);

    // sync test helper
    DistributedDB::DBStatus SyncTest(DistributedDB::KvStoreNbDelegate* delegate,
        const std::vector<std::string>& devices, DistributedDB::SyncMode mode,
        std::map<std::string, DistributedDB::DBStatus>& statuses, const DistributedDB::Query &query);

    static void GetRandomKeyValue(std::vector<uint8_t> &value, uint32_t defaultSize = 0);

    static bool IsValueEqual(const DistributedDB::Value &read, const DistributedDB::Value &origin);

    static bool IsEntryEqual(const DistributedDB::Entry &entryOrg, const DistributedDB::Entry &entryRet);

    static bool IsEntriesEqual(const std::vector<DistributedDB::Entry> &entriesOrg,
        const std::vector<DistributedDB::Entry> &entriesRet, bool needSort = false);

    static bool CheckObserverResult(const std::vector<DistributedDB::Entry> &orgEntries,
        const std::list<DistributedDB::Entry> &resultLst);

    static bool IsItemValueExist(const DistributedDB::DataItem &item,
        const std::vector<DistributedDB::DataItem> &items);

    static bool IsEntryExist(const DistributedDB::Entry &entry,
        const std::vector<DistributedDB::Entry> &entries);

    static bool IsKvEntryExist(const DistributedDB::Entry &entry,
        const std::vector<DistributedDB::Entry> &entries);

    static void CalcHash(const std::vector<uint8_t> &value, std::vector<uint8_t> &hashValue);

    static int CreateMockSingleDb(DatabaseInfo &dbInfo, DistributedDB::OpenDbProperties &properties);

    static int CreateMockMultiDb(DatabaseInfo &dbInfo, DistributedDB::OpenDbProperties &properties);

    static int ModifyDatabaseFile(const std::string &fileDir, uint64_t modifyPos = 0,
        uint32_t modifyCnt = 256, uint32_t value = 0x1F1F1F1F);

    static int GetSyncDataTest(const SyncInputArg &syncInputArg, DistributedDB::SQLiteSingleVerNaturalStore *store,
        std::vector<DistributedDB::DataItem> &dataItems, DistributedDB::ContinueToken &continueStmtToken);

    static int GetSyncDataNextTest(DistributedDB::SQLiteSingleVerNaturalStore *store, uint32_t blockSize,
        std::vector<DistributedDB::DataItem> &dataItems, DistributedDB::ContinueToken &continueStmtToken);

    static int PutSyncDataTest(DistributedDB::SQLiteSingleVerNaturalStore *store,
        const std::vector<DistributedDB::DataItem> &dataItems, const std::string &deviceName);

    static int PutSyncDataTest(DistributedDB::SQLiteSingleVerNaturalStore *store,
        const std::vector<DistributedDB::DataItem> &dataItems, const std::string &deviceName,
        const DistributedDB::QueryObject &query);

    static int ConvertItemsToSingleVerEntry(const std::vector<DistributedDB::DataItem> &dataItems,
        std::vector<DistributedDB::SingleVerKvEntry *> &entries);

    static void ConvertSingleVerEntryToItems(std::vector<DistributedDB::SingleVerKvEntry *> &entries,
        std::vector<DistributedDB::DataItem> &dataItems);

    static void ReleaseSingleVerEntry(std::vector<DistributedDB::SingleVerKvEntry *> &entries);

    static std::vector<uint8_t> GetRandPrefixKey(const std::vector<uint8_t> &prefixKey, uint32_t size);

    static int GetCurrentDir(std::string& dir);

    static int GetResourceDir(std::string& dir);

    static int GetRandInt(const int randMin, const int randMax);
    static int64_t GetRandInt64(const int64_t randMin, const int64_t randMax);

    static void PrintTestCaseInfo();

    static int BuildMessage(const DataSyncMessageInfo &messageInfo, DistributedDB::Message *&message);

private:
    static int OpenMockMultiDb(DatabaseInfo &dbInfo, DistributedDB::OpenDbProperties &properties);

    std::mutex syncLock_ {};
    std::condition_variable syncCondVar_ {};
};

class KvStoreObserverUnitTest : public DistributedDB::KvStoreObserver {
public:
    KvStoreObserverUnitTest();
    ~KvStoreObserverUnitTest() {}

    KvStoreObserverUnitTest(const KvStoreObserverUnitTest&) = delete;
    KvStoreObserverUnitTest& operator=(const KvStoreObserverUnitTest&) = delete;
    KvStoreObserverUnitTest(KvStoreObserverUnitTest&&) = delete;
    KvStoreObserverUnitTest& operator=(KvStoreObserverUnitTest&&) = delete;

    // callback function will be called when the db data is changed.
    void OnChange(const DistributedDB::KvStoreChangedData&);

    // reset the callCount_ to zero.
    void ResetToZero();

    // get callback results.
    unsigned long GetCallCount() const;
    const std::list<DistributedDB::Entry> &GetEntriesInserted() const;
    const std::list<DistributedDB::Entry> &GetEntriesUpdated() const;
    const std::list<DistributedDB::Entry> &GetEntriesDeleted() const;
    bool IsCleared() const;
private:
    unsigned long callCount_;
    bool isCleared_;
    std::list<DistributedDB::Entry> inserted_;
    std::list<DistributedDB::Entry> updated_;
    std::list<DistributedDB::Entry> deleted_;
};

class RelationalStoreObserverUnitTest : public DistributedDB::StoreObserver {
public:
    RelationalStoreObserverUnitTest();
    ~RelationalStoreObserverUnitTest() {}

    RelationalStoreObserverUnitTest(const RelationalStoreObserverUnitTest&) = delete;
    RelationalStoreObserverUnitTest& operator=(const RelationalStoreObserverUnitTest&) = delete;
    RelationalStoreObserverUnitTest(RelationalStoreObserverUnitTest&&) = delete;
    RelationalStoreObserverUnitTest& operator=(RelationalStoreObserverUnitTest&&) = delete;

    // callback function will be called when the db data is changed.
    void OnChange(const DistributedDB::StoreChangedData &data);

    // reset the callCount_ to zero.
    void ResetToZero();

    // get callback results.
    unsigned long GetCallCount() const;
    const std::string GetDataChangeDevice() const;
    DistributedDB::StoreProperty GetStoreProperty() const;
private:
    unsigned long callCount_;
    std::string changeDevice_;
    DistributedDB::StoreProperty storeProperty_;
};

class KvStoreCorruptInfo {
public:
    KvStoreCorruptInfo() {}
    ~KvStoreCorruptInfo() {}

    KvStoreCorruptInfo(const KvStoreCorruptInfo&) = delete;
    KvStoreCorruptInfo& operator=(const KvStoreCorruptInfo&) = delete;
    KvStoreCorruptInfo(KvStoreCorruptInfo&&) = delete;
    KvStoreCorruptInfo& operator=(KvStoreCorruptInfo&&) = delete;

    // callback function will be called when the db data is changed.
    void CorruptCallBack(const std::string &appId, const std::string &userId, const std::string &storeId);
    size_t GetDatabaseInfoSize() const;
    bool IsDataBaseCorrupted(const std::string &appId, const std::string &userId, const std::string &storeId) const;
    void Reset();
private:
    std::vector<DatabaseInfo> databaseInfoVect_;
};

class RelationalTestUtils {
public:
    static sqlite3 *CreateDataBase(const std::string &dbUri);
    static int ExecSql(sqlite3 *db, const std::string &sql);
    static int ExecSql(sqlite3 *db, const std::string &sql, const std::function<int (sqlite3_stmt *)> &bindCallback,
        const std::function<int (sqlite3_stmt *)> &resultCallback);
    static void CreateDeviceTable(sqlite3 *db, const std::string &table, const std::string &device);
    static int CheckSqlResult(sqlite3 *db, const std::string &sql, bool &result);
    static int CheckTableRecords(sqlite3 *db, const std::string &table);
    static int GetMetaData(sqlite3 *db, const DistributedDB::Key &key, DistributedDB::Value &value);
    static int SetMetaData(sqlite3 *db, const DistributedDB::Key &key, const DistributedDB::Value &value);
};
} // namespace DistributedDBUnitTest

#endif // DISTRIBUTEDDB_TOOLS_UNIT_TEST_H
