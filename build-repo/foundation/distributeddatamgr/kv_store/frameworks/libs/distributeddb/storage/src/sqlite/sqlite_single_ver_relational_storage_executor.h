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
#ifndef SQLITE_SINGLE_VER_RELATIONAL_STORAGE_EXECUTOR_H
#define SQLITE_SINGLE_VER_RELATIONAL_STORAGE_EXECUTOR_H
#ifdef RELATIONAL_STORE

#include "data_transformer.h"
#include "db_types.h"
#include "macro_utils.h"
#include "sqlite_utils.h"
#include "sqlite_storage_executor.h"
#include "relational_row_data.h"
#include "relational_store_delegate.h"
#include "query_object.h"

namespace DistributedDB {
class SQLiteSingleVerRelationalStorageExecutor : public SQLiteStorageExecutor {
public:
    SQLiteSingleVerRelationalStorageExecutor(sqlite3 *dbHandle, bool writable, DistributedTableMode mode);
    ~SQLiteSingleVerRelationalStorageExecutor() override = default;

    // Delete the copy and assign constructors
    DISABLE_COPY_ASSIGN_MOVE(SQLiteSingleVerRelationalStorageExecutor);

    // The parameter "identity" is a hash string that identifies a device
    int CreateDistributedTable(const std::string &tableName, DistributedTableMode mode, bool isUpgraded,
        const std::string &identity, TableInfo &table);

    int UpgradeDistributedTable(const std::string &tableName, DistributedTableMode mode, bool &schemaChanged,
        RelationalSchemaObject &schema);

    int StartTransaction(TransactType type);
    int Commit();
    int Rollback();

    // For Get sync data
    int GetSyncDataByQuery(std::vector<DataItem> &dataItems, size_t appendLength, const DataSizeSpecInfo &sizeInfo,
        std::function<int(sqlite3 *, sqlite3_stmt *&, sqlite3_stmt *&, bool &)> getStmt, const TableInfo &tableInfo);

    // operation of meta data
    int GetKvData(const Key &key, Value &value) const;
    int PutKvData(const Key &key, const Value &value) const;
    int DeleteMetaData(const std::vector<Key> &keys) const;
    int DeleteMetaDataByPrefixKey(const Key &keyPrefix) const;
    int GetAllMetaKeys(std::vector<Key> &keys) const;

    // For Put sync data
    int SaveSyncItems(const QueryObject &object, std::vector<DataItem> &dataItems,
        const std::string &deviceName, const TableInfo &table, bool useTrans = true);

    int AnalysisRelationalSchema(const std::string &tableName, TableInfo &tableInfo);

    int CheckDBModeForRelational();

    int DeleteDistributedDeviceTable(const std::string &device, const std::string &tableName);

    int DeleteDistributedDeviceTableLog(const std::string &device, const std::string &tableName,
        const std::map<std::string, TableInfo> &tables);

    int DeleteDistributedLogTable(const std::string &tableName);

    int CheckAndCleanDistributedTable(const std::vector<std::string> &tableNames,
        std::vector<std::string> &missingTables);

    int CreateDistributedDeviceTable(const std::string &device, const TableInfo &baseTbl);

    int CheckQueryObjectLegal(const TableInfo &table, QueryObject &query, const std::string &schemaVersion);

    int GetMaxTimestamp(const std::vector<std::string> &tablesName, Timestamp &maxTimestamp) const;

    int ExecuteQueryBySqlStmt(const std::string &sql, const std::vector<std::string> &bindArgs, int packetSize,
        std::vector<std::string> &colNames, std::vector<RelationalRowData *> &data);

    int CheckEncryptedOrCorrupted() const;

private:
    struct SaveSyncDataStmt {
        sqlite3_stmt *saveDataStmt = nullptr;
        sqlite3_stmt *saveLogStmt = nullptr;
        sqlite3_stmt *queryStmt = nullptr;
        sqlite3_stmt *rmDataStmt = nullptr;
        sqlite3_stmt *rmLogStmt = nullptr;

        int ResetStatements(bool isNeedFinalize);
    };

    int PrepareForSyncDataByTime(Timestamp begin, Timestamp end,
        sqlite3_stmt *&statement, bool getDeletedData) const;

    int GetDataItemForSync(sqlite3_stmt *statement, DataItem &dataItem, bool isGettingDeletedData) const;

    int GetSyncDataPre(const DataItem &dataItem, DataItem &itemGet);

    int CheckDataConflictDefeated(const DataItem &item, bool &isDefeated);

    int SaveSyncDataItem(const std::vector<FieldInfo> &fieldInfos, const std::string &deviceName, DataItem &item);

    int SaveSyncDataItems(const QueryObject &object, std::vector<DataItem> &dataItems, const std::string &deviceName);
    int SaveSyncDataItem(const DataItem &dataItem, sqlite3_stmt *&saveDataStmt, sqlite3_stmt *&rmDataStmt,
        const std::vector<FieldInfo> &fieldInfos, int64_t &rowid);

    int DeleteSyncDataItem(const DataItem &dataItem, sqlite3_stmt *&rmDataStmt);

    int SaveSyncLog(sqlite3_stmt *statement, sqlite3_stmt *queryStmt, const DataItem &dataItem, int64_t rowid);
    int PrepareForSavingData(const QueryObject &object, sqlite3_stmt *&statement) const;
    int PrepareForSavingLog(const QueryObject &object, const std::string &deviceName,
        sqlite3_stmt *&statement,  sqlite3_stmt *&queryStmt) const;

    int AlterAuxTableForUpgrade(const TableInfo &oldTableInfo, const TableInfo &newTableInfo);

    int DeleteSyncLog(const DataItem &item, sqlite3_stmt *&rmLogStmt);
    int ProcessMissQueryData(const DataItem &item, sqlite3_stmt *&rmDataStmt, sqlite3_stmt *&rmLogStmt);
    int GetMissQueryData(sqlite3_stmt *fullStmt, DataItem &item);
    int GetQueryDataAndStepNext(bool isFirstTime, bool isGettingDeletedData, sqlite3_stmt *queryStmt, DataItem &item,
        Timestamp &queryTime);
    int GetMissQueryDataAndStepNext(sqlite3_stmt *fullStmt, DataItem &item, Timestamp &missQueryTime);

    int SetLogTriggerStatus(bool status);

    void SetTableInfo(const TableInfo &tableInfo);  // When put or get sync data, must call the func first.

    int GeneLogInfoForExistedData(sqlite3 *db, const std::string &tableName, const TableInfo &table,
        const std::string &calPrimaryKeyHash);

    std::string baseTblName_;
    TableInfo table_;  // Always operating table, user table when get, device table when put.

    SaveSyncDataStmt saveStmt_;

    DistributedTableMode mode_;
};
} // namespace DistributedDB
#endif
#endif // SQLITE_SINGLE_VER_RELATIONAL_STORAGE_EXECUTOR_H