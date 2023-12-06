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
#ifdef RELATIONAL_STORE
#include "sqlite_single_ver_relational_storage_executor.h"

#include <algorithm>
#include <optional>

#include "data_transformer.h"
#include "db_common.h"
#include "log_table_manager_factory.h"
#include "relational_row_data_impl.h"
#include "res_finalizer.h"
#include "sqlite_relational_utils.h"

namespace DistributedDB {
namespace {
int PermitSelect(void *a, int b, const char *c, const char *d, const char *e, const char *f)
{
    if (b != SQLITE_SELECT && b != SQLITE_READ && b != SQLITE_FUNCTION) {
        return SQLITE_DENY;
    }
    return SQLITE_OK;
}
}
SQLiteSingleVerRelationalStorageExecutor::SQLiteSingleVerRelationalStorageExecutor(sqlite3 *dbHandle, bool writable,
    DistributedTableMode mode)
    : SQLiteStorageExecutor(dbHandle, writable, false), mode_(mode)
{}


int CheckTableConstraint(const TableInfo &table, DistributedTableMode mode)
{
    std::string trimedSql = DBCommon::TrimSpace(table.GetCreateTableSql());
    if (DBCommon::HasConstraint(trimedSql, "WITHOUT ROWID", " ),", " ,;")) {
        LOGE("[CreateDistributedTable] Not support create distributed table without rowid.");
        return -E_NOT_SUPPORT;
    }

    if (mode == DistributedTableMode::COLLABORATION) {
        if (DBCommon::HasConstraint(trimedSql, "CHECK", " ,", " (")) {
            LOGE("[CreateDistributedTable] Not support create distributed table with 'CHECK' constraint.");
            return -E_NOT_SUPPORT;
        }

        if (DBCommon::HasConstraint(trimedSql, "ON CONFLICT", " )", " ")) {
            LOGE("[CreateDistributedTable] Not support create distributed table with 'ON CONFLICT' constraint.");
            return -E_NOT_SUPPORT;
        }

        if (DBCommon::HasConstraint(trimedSql, "REFERENCES", " )", " ")) {
            LOGE("[CreateDistributedTable] Not support create distributed table with 'FOREIGN KEY' constraint.");
            return -E_NOT_SUPPORT;
        }
    }

    if (mode == DistributedTableMode::SPLIT_BY_DEVICE) {
        if (table.GetPrimaryKey().size() > 1) {
            LOGE("[CreateDistributedTable] Not support create distributed table with composite primary keys.");
            return -E_NOT_SUPPORT;
        }
    }

    return E_OK;
}

namespace {
int GetExistedDataTimeOffset(sqlite3 *db, const std::string &tableName, bool isMem, int64_t &timeOffset)
{
    std::string sql = "SELECT get_sys_time(0) - max(rowid) - 1 FROM " + tableName;
    sqlite3_stmt *stmt = nullptr;
    int errCode = SQLiteUtils::GetStatement(db, sql, stmt);
    if (errCode != E_OK) {
        return errCode;
    }
    errCode = SQLiteUtils::StepWithRetry(stmt, isMem);
    if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
        timeOffset = static_cast<int64_t>(sqlite3_column_int64(stmt, 0));
        errCode = E_OK;
    }
    SQLiteUtils::ResetStatement(stmt, true, errCode);
    return errCode;
}
}

int SQLiteSingleVerRelationalStorageExecutor::GeneLogInfoForExistedData(sqlite3 *db, const std::string &tableName,
    const TableInfo &table, const std::string &calPrimaryKeyHash)
{
    int64_t timeOffset = 0;
    int errCode = GetExistedDataTimeOffset(db, tableName, isMemDb_, timeOffset);
    if (errCode != E_OK) {
        return errCode;
    }
    std::string timeOffsetStr = std::to_string(timeOffset);
    std::string logTable = DBConstant::RELATIONAL_PREFIX + tableName + "_log";
    std::string sql = "INSERT INTO " + logTable +
        " SELECT rowid, '', '', " + timeOffsetStr + " + rowid, " + timeOffsetStr + " + rowid, 0x2, " +
        calPrimaryKeyHash + " FROM " + tableName + " WHERE 1=1;";
    return SQLiteUtils::ExecuteRawSQL(db, sql);
}

int SQLiteSingleVerRelationalStorageExecutor::CreateDistributedTable(const std::string &tableName,
    DistributedTableMode mode, bool isUpgraded, const std::string &identity, TableInfo &table)
{
    if (dbHandle_ == nullptr) {
        return -E_INVALID_DB;
    }

    if (mode == DistributedTableMode::SPLIT_BY_DEVICE && !isUpgraded) {
        bool isEmpty = false;
        int errCode = SQLiteUtils::CheckTableEmpty(dbHandle_, tableName, isEmpty);
        if (errCode != E_OK || !isEmpty) {
            LOGE("[CreateDistributedTable] check table empty failed. error=%d, isEmpty=%d", errCode, isEmpty);
            return -E_NOT_SUPPORT;
        }
    }

    int errCode = SQLiteUtils::AnalysisSchema(dbHandle_, tableName, table);
    if (errCode != E_OK) {
        LOGE("[CreateDistributedTable] analysis table schema failed. %d", errCode);
        return errCode;
    }

    errCode = CheckTableConstraint(table, mode);
    if (errCode != E_OK) {
        LOGE("[CreateDistributedTable] check table constraint failed.");
        return errCode;
    }

    // create log table
    auto tableManager = LogTableManagerFactory::GetTableManager(mode);
    errCode = tableManager->CreateRelationalLogTable(dbHandle_, table);
    if (errCode != E_OK) {
        LOGE("[CreateDistributedTable] create log table failed");
        return errCode;
    }

    if (!isUpgraded) {
        std::string calPrimaryKeyHash = tableManager->CalcPrimaryKeyHash("", table, identity);
        errCode = GeneLogInfoForExistedData(dbHandle_, tableName, table, calPrimaryKeyHash);
        if (errCode != E_OK) {
            return errCode;
        }
    }

    // add trigger
    errCode = tableManager->AddRelationalLogTableTrigger(dbHandle_, table, identity);
    if (errCode != E_OK) {
        LOGE("[CreateDistributedTable] Add relational log table trigger failed.");
        return errCode;
    }
    return SetLogTriggerStatus(true);
}

int SQLiteSingleVerRelationalStorageExecutor::UpgradeDistributedTable(const std::string &tableName,
    DistributedTableMode mode, bool &schemaChanged, RelationalSchemaObject &schema)
{
    if (dbHandle_ == nullptr) {
        return -E_INVALID_DB;
    }
    TableInfo newTableInfo;
    int errCode = SQLiteUtils::AnalysisSchema(dbHandle_, tableName, newTableInfo);
    if (errCode != E_OK) {
        LOGE("[UpgradeDistributedTable] analysis table schema failed. %d", errCode);
        return errCode;
    }

    if (CheckTableConstraint(newTableInfo, mode)) {
        LOGE("[UpgradeDistributedTable] Not support create distributed table without rowid.");
        return -E_NOT_SUPPORT;
    }

    // new table should has same or compatible upgrade
    TableInfo tableInfo = schema.GetTable(tableName);
    errCode = tableInfo.CompareWithTable(newTableInfo, schema.GetSchemaVersion());
    if (errCode == -E_RELATIONAL_TABLE_INCOMPATIBLE) {
        LOGE("[UpgradeDistributedTable] Not support with incompatible upgrade.");
        return -E_SCHEMA_MISMATCH;
    } else if (errCode == -E_RELATIONAL_TABLE_EQUAL) {
        LOGD("[UpgradeDistributedTable] schema has not changed.");
        return E_OK;
    }

    schemaChanged = true;
    errCode = AlterAuxTableForUpgrade(tableInfo, newTableInfo);
    if (errCode != E_OK) {
        LOGE("[UpgradeDistributedTable] Alter aux table for upgrade failed. %d", errCode);
    }

    schema.AddRelationalTable(newTableInfo);
    return errCode;
}

namespace {
int GetDeviceTableName(sqlite3 *handle, const std::string &tableName, const std::string &device,
    std::vector<std::string> &deviceTables)
{
    if (device.empty() && tableName.empty()) { // device and table name should not both be empty
        return -E_INVALID_ARGS;
    }
    std::string deviceHash = DBCommon::TransferStringToHex(DBCommon::TransferHashString(device));
    std::string devicePattern = device.empty() ? "%" : deviceHash;
    std::string tablePattern = tableName.empty() ? "%" : tableName;
    std::string deviceTableName = DBConstant::RELATIONAL_PREFIX + tablePattern + "_" + devicePattern;

    const std::string checkSql = "SELECT name FROM sqlite_master WHERE type='table' AND name LIKE '" +
        deviceTableName + "';";
    sqlite3_stmt *stmt = nullptr;
    int errCode = SQLiteUtils::GetStatement(handle, checkSql, stmt);
    if (errCode != E_OK) {
        SQLiteUtils::ResetStatement(stmt, true, errCode);
        return errCode;
    }

    do {
        errCode = SQLiteUtils::StepWithRetry(stmt, false);
        if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
            errCode = E_OK;
            break;
        } else if (errCode != SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
            LOGE("Get table name failed. %d", errCode);
            break;
        }
        std::string realTableName;
        errCode = SQLiteUtils::GetColumnTextValue(stmt, 0, realTableName); // 0: table name result column index
        if (errCode != E_OK || realTableName.empty()) { // sqlite might return a row with NULL
            continue;
        }
        if (realTableName.rfind("_log") == (realTableName.length() - 4)) { // 4:suffix length of "_log"
            continue;
        }
        deviceTables.emplace_back(realTableName);
    } while (true);

    SQLiteUtils::ResetStatement(stmt, true, errCode);
    return errCode;
}

std::vector<FieldInfo> GetUpgradeFields(const TableInfo &oldTableInfo, const TableInfo &newTableInfo)
{
    std::vector<FieldInfo> fields;
    auto itOld = oldTableInfo.GetFields().begin();
    auto itNew = newTableInfo.GetFields().begin();
    for (; itNew != newTableInfo.GetFields().end(); itNew++) {
        if (itOld == oldTableInfo.GetFields().end() || itOld->first != itNew->first) {
            fields.emplace_back(itNew->second);
            continue;
        }
        itOld++;
    }
    return fields;
}

int UpgradeFields(sqlite3 *db, const std::vector<std::string> &tables, std::vector<FieldInfo> &fields)
{
    if (db == nullptr) {
        return -E_INVALID_ARGS;
    }

    std::sort(fields.begin(), fields.end(), [] (const FieldInfo &a, const FieldInfo &b) {
        return a.GetColumnId()< b.GetColumnId();
    });
    int errCode = E_OK;
    for (const auto &table : tables) {
        for (const auto &field : fields) {
            std::string alterSql = "ALTER TABLE " + table + " ADD " + field.GetFieldName() + " " + field.GetDataType();
            alterSql += field.IsNotNull() ? " NOT NULL" : "";
            alterSql += field.HasDefaultValue() ? " DEFAULT " + field.GetDefaultValue() : "";
            alterSql += ";";
            errCode = SQLiteUtils::ExecuteRawSQL(db, alterSql);
            if (errCode != E_OK) {
                LOGE("Alter table failed. %d", errCode);
                break;
            }
        }
    }
    return errCode;
}

std::map<std::string, CompositeFields> GetChangedIndexes(const TableInfo &oldTableInfo, const TableInfo &newTableInfo)
{
    std::map<std::string, CompositeFields> indexes;
    auto itOld = oldTableInfo.GetIndexDefine().begin();
    auto itNew = newTableInfo.GetIndexDefine().begin();
    auto itOldEnd = oldTableInfo.GetIndexDefine().end();
    auto itNewEnd = newTableInfo.GetIndexDefine().end();

    while (itOld != itOldEnd && itNew != itNewEnd) {
        if (itOld->first == itNew->first) {
            if (itOld->second != itNew->second) {
                indexes.insert({itNew->first, itNew->second});
            }
            itOld++;
            itNew++;
        } else if (itOld->first < itNew->first) {
            indexes.insert({itOld->first, {}});
            itOld++;
        } else if (itOld->first > itNew->first) {
            indexes.insert({itNew->first, itNew->second});
            itNew++;
        }
    }

    while (itOld != itOldEnd) {
        indexes.insert({itOld->first, {}});
        itOld++;
    }

    while (itNew != itNewEnd) {
        indexes.insert({itNew->first, itNew->second});
        itNew++;
    }

    return indexes;
}

int Upgradeindexes(sqlite3 *db, const std::vector<std::string> &tables,
    const std::map<std::string, CompositeFields> &indexes)
{
    if (db == nullptr) {
        return -E_INVALID_ARGS;
    }

    int errCode = E_OK;
    for (const auto &table : tables) {
        for (const auto &index : indexes) {
            if (index.first.empty()) {
                continue;
            }
            std::string realIndexName = table + "_" + index.first;
            std::string deleteIndexSql = "DROP INDEX IF EXISTS " + realIndexName;
            errCode = SQLiteUtils::ExecuteRawSQL(db, deleteIndexSql);
            if (errCode != E_OK) {
                LOGE("Drop index failed. %d", errCode);
                return errCode;
            }

            if (index.second.empty()) { // empty means drop index only
                continue;
            }

            auto it = index.second.begin();
            std::string indexDefine = *it++;
            while (it != index.second.end()) {
                indexDefine += ", " + *it++;
            }
            std::string createIndexSql = "CREATE INDEX IF NOT EXISTS " + realIndexName + " ON " + table +
                "(" + indexDefine + ");";
            errCode = SQLiteUtils::ExecuteRawSQL(db, createIndexSql);
            if (errCode != E_OK) {
                LOGE("Create index failed. %d", errCode);
                break;
            }
        }
    }
    return errCode;
}
}

int SQLiteSingleVerRelationalStorageExecutor::AlterAuxTableForUpgrade(const TableInfo &oldTableInfo,
    const TableInfo &newTableInfo)
{
    std::vector<FieldInfo> upgradeFields = GetUpgradeFields(oldTableInfo, newTableInfo);
    std::map<std::string, CompositeFields> upgradeIndexces = GetChangedIndexes(oldTableInfo, newTableInfo);
    std::vector<std::string> deviceTables;
    int errCode = GetDeviceTableName(dbHandle_, oldTableInfo.GetTableName(), {}, deviceTables);
    if (errCode != E_OK) {
        LOGE("Get device table name for alter table failed. %d", errCode);
        return errCode;
    }

    LOGD("Begin to alter table: upgrade fields[%zu], indexces[%zu], deviceTable[%zu]", upgradeFields.size(),
        upgradeIndexces.size(), deviceTables.size());
    errCode = UpgradeFields(dbHandle_, deviceTables, upgradeFields);
    if (errCode != E_OK) {
        LOGE("upgrade fields failed. %d", errCode);
        return errCode;
    }

    errCode = Upgradeindexes(dbHandle_, deviceTables, upgradeIndexces);
    if (errCode != E_OK) {
        LOGE("upgrade indexes failed. %d", errCode);
    }

    return E_OK;
}

int SQLiteSingleVerRelationalStorageExecutor::StartTransaction(TransactType type)
{
    if (dbHandle_ == nullptr) {
        LOGE("Begin transaction failed, dbHandle is null.");
        return -E_INVALID_DB;
    }
    int errCode = SQLiteUtils::BeginTransaction(dbHandle_, type);
    if (errCode != E_OK) {
        LOGE("Begin transaction failed, errCode = %d", errCode);
    }
    return errCode;
}

int SQLiteSingleVerRelationalStorageExecutor::Commit()
{
    if (dbHandle_ == nullptr) {
        return -E_INVALID_DB;
    }

    return SQLiteUtils::CommitTransaction(dbHandle_);
}

int SQLiteSingleVerRelationalStorageExecutor::Rollback()
{
    if (dbHandle_ == nullptr) {
        return -E_INVALID_DB;
    }
    int errCode = SQLiteUtils::RollbackTransaction(dbHandle_);
    if (errCode != E_OK) {
        LOGE("sqlite single ver storage executor rollback fail! errCode = [%d]", errCode);
    }
    return errCode;
}

void SQLiteSingleVerRelationalStorageExecutor::SetTableInfo(const TableInfo &tableInfo)
{
    table_ = tableInfo;
}

static int BindDataValueByType(sqlite3_stmt *statement, const std::optional<DataValue> &data, int cid)
{
    int errCode = E_OK;
    StorageType type = data.value().GetType();
    switch (type) {
        case StorageType::STORAGE_TYPE_INTEGER: {
            int64_t intData = 0;
            (void)data.value().GetInt64(intData);
            errCode = SQLiteUtils::MapSQLiteErrno(sqlite3_bind_int64(statement, cid, intData));
            break;
        }

        case StorageType::STORAGE_TYPE_REAL: {
            double doubleData = 0;
            (void)data.value().GetDouble(doubleData);
            errCode = SQLiteUtils::MapSQLiteErrno(sqlite3_bind_double(statement, cid, doubleData));
            break;
        }

        case StorageType::STORAGE_TYPE_TEXT: {
            std::string strData;
            (void)data.value().GetText(strData);
            errCode = SQLiteUtils::BindTextToStatement(statement, cid, strData);
            break;
        }

        case StorageType::STORAGE_TYPE_BLOB: {
            Blob blob;
            (void)data.value().GetBlob(blob);
            std::vector<uint8_t> blobData(blob.GetData(), blob.GetData() + blob.GetSize());
            errCode = SQLiteUtils::BindBlobToStatement(statement, cid, blobData, true);
            break;
        }

        case StorageType::STORAGE_TYPE_NULL: {
            errCode = SQLiteUtils::MapSQLiteErrno(sqlite3_bind_null(statement, cid));
            break;
        }

        default:
            break;
    }
    return errCode;
}

static int GetLogData(sqlite3_stmt *logStatement, LogInfo &logInfo)
{
    logInfo.dataKey = sqlite3_column_int64(logStatement, 0);  // 0 means dataKey index

    std::vector<uint8_t> dev;
    int errCode = SQLiteUtils::GetColumnBlobValue(logStatement, 1, dev);  // 1 means dev index
    if (errCode != E_OK) {
        return errCode;
    }
    logInfo.device = std::string(dev.begin(), dev.end());

    std::vector<uint8_t> oriDev;
    errCode = SQLiteUtils::GetColumnBlobValue(logStatement, 2, oriDev);  // 2 means ori_dev index
    if (errCode != E_OK) {
        return errCode;
    }
    logInfo.originDev = std::string(oriDev.begin(), oriDev.end());
    logInfo.timestamp = static_cast<uint64_t>(sqlite3_column_int64(logStatement, 3));  // 3 means timestamp index
    logInfo.wTimestamp = static_cast<uint64_t>(sqlite3_column_int64(logStatement, 4));  // 4 means w_timestamp index
    logInfo.flag = static_cast<uint64_t>(sqlite3_column_int64(logStatement, 5));  // 5 means flag index
    logInfo.flag &= (~DataItem::LOCAL_FLAG);
    logInfo.flag &= (~DataItem::UPDATE_FLAG);
    return SQLiteUtils::GetColumnBlobValue(logStatement, 6, logInfo.hashKey);  // 6 means hashKey index
}

static size_t GetDataItemSerialSize(DataItem &item, size_t appendLen)
{
    // timestamp and local flag: 3 * uint64_t, version(uint32_t), key, value, origin dev and the padding size.
    // the size would not be very large.
    static const size_t maxOrigDevLength = 40;
    size_t devLength = std::max(maxOrigDevLength, item.origDev.size());
    size_t dataSize = (Parcel::GetUInt64Len() * 3 + Parcel::GetUInt32Len() + Parcel::GetVectorCharLen(item.key) +
        Parcel::GetVectorCharLen(item.value) + devLength + appendLen);
    return dataSize;
}

int SQLiteSingleVerRelationalStorageExecutor::GetKvData(const Key &key, Value &value) const
{
    static const std::string SELECT_META_VALUE_SQL = "SELECT value FROM " + DBConstant::RELATIONAL_PREFIX +
        "metadata WHERE key=?;";
    sqlite3_stmt *statement = nullptr;
    int errCode = SQLiteUtils::GetStatement(dbHandle_, SELECT_META_VALUE_SQL, statement);
    if (errCode != E_OK) {
        goto END;
    }

    errCode = SQLiteUtils::BindBlobToStatement(statement, 1, key, false); // first arg.
    if (errCode != E_OK) {
        goto END;
    }

    errCode = SQLiteUtils::StepWithRetry(statement, isMemDb_);
    if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
        errCode = -E_NOT_FOUND;
        goto END;
    } else if (errCode != SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
        goto END;
    }

    errCode = SQLiteUtils::GetColumnBlobValue(statement, 0, value); // only one result.
    END:
    SQLiteUtils::ResetStatement(statement, true, errCode);
    return errCode;
}

int SQLiteSingleVerRelationalStorageExecutor::PutKvData(const Key &key, const Value &value) const
{
    static const std::string INSERT_META_SQL = "INSERT OR REPLACE INTO " + DBConstant::RELATIONAL_PREFIX +
        "metadata VALUES(?,?);";
    sqlite3_stmt *statement = nullptr;
    int errCode = SQLiteUtils::GetStatement(dbHandle_, INSERT_META_SQL, statement);
    if (errCode != E_OK) {
        goto ERROR;
    }

    errCode = SQLiteUtils::BindBlobToStatement(statement, 1, key, false);  // 1 means key index
    if (errCode != E_OK) {
        LOGE("[SingleVerExe][BindPutKv]Bind key error:%d", errCode);
        goto ERROR;
    }

    errCode = SQLiteUtils::BindBlobToStatement(statement, 2, value, true);  // 2 means value index
    if (errCode != E_OK) {
        LOGE("[SingleVerExe][BindPutKv]Bind value error:%d", errCode);
        goto ERROR;
    }
    errCode = SQLiteUtils::StepWithRetry(statement, isMemDb_);
    if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
        errCode = E_OK;
    }
ERROR:
    SQLiteUtils::ResetStatement(statement, true, errCode);
    return errCode;
}

int SQLiteSingleVerRelationalStorageExecutor::DeleteMetaData(const std::vector<Key> &keys) const
{
    static const std::string REMOVE_META_VALUE_SQL = "DELETE FROM " + DBConstant::RELATIONAL_PREFIX +
        "metadata WHERE key=?;";
    sqlite3_stmt *statement = nullptr;
    int errCode = SQLiteUtils::GetStatement(dbHandle_, REMOVE_META_VALUE_SQL, statement);
    if (errCode != E_OK) {
        return errCode;
    }

    for (const auto &key : keys) {
        errCode = SQLiteUtils::BindBlobToStatement(statement, 1, key, false); // first arg.
        if (errCode != E_OK) {
            break;
        }

        errCode = SQLiteUtils::StepWithRetry(statement, isMemDb_);
        if (errCode != SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
            break;
        }
        errCode = E_OK;
        SQLiteUtils::ResetStatement(statement, false, errCode);
    }
    SQLiteUtils::ResetStatement(statement, true, errCode);
    return CheckCorruptedStatus(errCode);
}

int SQLiteSingleVerRelationalStorageExecutor::DeleteMetaDataByPrefixKey(const Key &keyPrefix) const
{
    static const std::string REMOVE_META_VALUE_BY_KEY_PREFIX_SQL = "DELETE FROM " + DBConstant::RELATIONAL_PREFIX +
        "metadata WHERE key>=? AND key<=?;";
    sqlite3_stmt *statement = nullptr;
    int errCode = SQLiteUtils::GetStatement(dbHandle_, REMOVE_META_VALUE_BY_KEY_PREFIX_SQL, statement);
    if (errCode != E_OK) {
        return errCode;
    }

    errCode = SQLiteUtils::BindPrefixKey(statement, 1, keyPrefix); // 1 is first arg.
    if (errCode == E_OK) {
        errCode = SQLiteUtils::StepWithRetry(statement, isMemDb_);
        if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
            errCode = E_OK;
        }
    }
    SQLiteUtils::ResetStatement(statement, true, errCode);
    return CheckCorruptedStatus(errCode);
}

static int GetAllKeys(sqlite3_stmt *statement, std::vector<Key> &keys)
{
    if (statement == nullptr) {
        return -E_INVALID_DB;
    }
    int errCode;
    do {
        errCode = SQLiteUtils::StepWithRetry(statement, false);
        if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
            Key key;
            errCode = SQLiteUtils::GetColumnBlobValue(statement, 0, key);
            if (errCode != E_OK) {
                break;
            }

            keys.push_back(std::move(key));
        } else if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
            errCode = E_OK;
            break;
        } else {
            LOGE("SQLite step for getting all keys failed:%d", errCode);
            break;
        }
    } while (true);
    return errCode;
}

int SQLiteSingleVerRelationalStorageExecutor::GetAllMetaKeys(std::vector<Key> &keys) const
{
    static const std::string SELECT_ALL_META_KEYS = "SELECT key FROM " + DBConstant::RELATIONAL_PREFIX + "metadata;";
    sqlite3_stmt *statement = nullptr;
    int errCode = SQLiteUtils::GetStatement(dbHandle_, SELECT_ALL_META_KEYS, statement);
    if (errCode != E_OK) {
        LOGE("[Relational][GetAllKey] Get statement failed:%d", errCode);
        return errCode;
    }
    errCode = GetAllKeys(statement, keys);
    SQLiteUtils::ResetStatement(statement, true, errCode);
    return errCode;
}

int SQLiteSingleVerRelationalStorageExecutor::PrepareForSavingLog(const QueryObject &object,
    const std::string &deviceName, sqlite3_stmt *&logStmt, sqlite3_stmt *&queryStmt) const
{
    std::string devName = DBCommon::TransferHashString(deviceName);
    const std::string tableName = DBConstant::RELATIONAL_PREFIX + object.GetTableName() + "_log";
    std::string dataFormat = "?, '" + deviceName + "', ?, ?, ?, ?, ?";
    std::string columnList = "data_key, device, ori_device, timestamp, wtimestamp, flag, hash_key";
    std::string sql = "INSERT OR REPLACE INTO " + tableName +
        " (" + columnList + ") VALUES (" + dataFormat + ");";
    int errCode = SQLiteUtils::GetStatement(dbHandle_, sql, logStmt);
    if (errCode != E_OK) {
        LOGE("[info statement] Get log statement fail! errCode:%d", errCode);
        return errCode;
    }
    std::string selectSql = "select " + columnList + " from " + tableName;
    if (mode_ == DistributedTableMode::COLLABORATION) {
        selectSql += " where hash_key = ?;";
    } else {
        selectSql += " where hash_key = ? and device = ?;";
    }
    errCode = SQLiteUtils::GetStatement(dbHandle_, selectSql, queryStmt);
    if (errCode != E_OK) {
        SQLiteUtils::ResetStatement(logStmt, true, errCode);
        LOGE("[info statement] Get query statement fail! errCode:%d", errCode);
    }
    return errCode;
}

int SQLiteSingleVerRelationalStorageExecutor::PrepareForSavingData(const QueryObject &object,
    sqlite3_stmt *&statement) const
{
    std::string colName;
    std::string dataFormat;
    for (size_t colId = 0; colId < table_.GetFields().size(); ++colId) {
        colName += table_.GetFieldName(colId) + ",";
        dataFormat += "?,";
    }
    colName.pop_back();
    dataFormat.pop_back();

    const std::string sql = "INSERT OR REPLACE INTO " + table_.GetTableName() +
        " (" + colName + ") VALUES (" + dataFormat + ");";
    int errCode = SQLiteUtils::GetStatement(dbHandle_, sql, statement);
    if (errCode != E_OK) {
        LOGE("[info statement] Get saving data statement fail! errCode:%d", errCode);
    }
    return errCode;
}

int SQLiteSingleVerRelationalStorageExecutor::SaveSyncLog(sqlite3_stmt *statement, sqlite3_stmt *queryStmt,
    const DataItem &dataItem, int64_t rowid)
{
    int errCode = SQLiteUtils::BindBlobToStatement(queryStmt, 1, dataItem.hashKey);  // 1 means hashkey index.
    if (errCode != E_OK) {
        return errCode;
    }
    if (mode_ != DistributedTableMode::COLLABORATION) {
        errCode = SQLiteUtils::BindTextToStatement(queryStmt, 2, dataItem.dev);  // 2 means device index.
        if (errCode != E_OK) {
            return errCode;
        }
    }
    LogInfo logInfoGet;
    errCode = SQLiteUtils::StepWithRetry(queryStmt, isMemDb_);
    if (errCode != SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
        errCode = -E_NOT_FOUND;
    } else {
        errCode = GetLogData(queryStmt, logInfoGet);
    }

    LogInfo logInfoBind;
    logInfoBind.hashKey = dataItem.hashKey;
    logInfoBind.device = dataItem.dev;
    logInfoBind.timestamp = dataItem.timestamp;
    logInfoBind.flag = dataItem.flag;

    if (errCode == -E_NOT_FOUND) { // insert
        logInfoBind.wTimestamp = dataItem.writeTimestamp;
        logInfoBind.originDev = dataItem.dev;
    } else if (errCode == E_OK) { // update
        logInfoBind.wTimestamp = logInfoGet.wTimestamp;
        logInfoBind.originDev = logInfoGet.originDev;
    } else {
        return errCode;
    }

    // bind
    SQLiteUtils::BindInt64ToStatement(statement, 1, rowid);  // 1 means dataKey index
    std::vector<uint8_t> originDev(logInfoBind.originDev.begin(), logInfoBind.originDev.end());
    SQLiteUtils::BindBlobToStatement(statement, 2, originDev);  // 2 means ori_dev index
    SQLiteUtils::BindInt64ToStatement(statement, 3, logInfoBind.timestamp);  // 3 means timestamp index
    SQLiteUtils::BindInt64ToStatement(statement, 4, logInfoBind.wTimestamp);  // 4 means w_timestamp index
    SQLiteUtils::BindInt64ToStatement(statement, 5, logInfoBind.flag);  // 5 means flag index
    SQLiteUtils::BindBlobToStatement(statement, 6, logInfoBind.hashKey);  // 6 means hashKey index
    errCode = SQLiteUtils::StepWithRetry(statement, isMemDb_);
    if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
        return E_OK;
    }
    return errCode;
}

int SQLiteSingleVerRelationalStorageExecutor::DeleteSyncDataItem(const DataItem &dataItem, sqlite3_stmt *&stmt)
{
    if (stmt == nullptr) {
        std::string sql = "DELETE FROM " + table_.GetTableName() + " WHERE rowid IN ("
            "SELECT data_key FROM " + DBConstant::RELATIONAL_PREFIX + baseTblName_ + "_log ";
        if (mode_ == DistributedTableMode::COLLABORATION) {
            sql += "WHERE hash_key=?);";
        } else {
            sql += "WHERE hash_key=? AND device=? AND flag&0x01=0);";
        }
        int errCode = SQLiteUtils::GetStatement(dbHandle_, sql, stmt);
        if (errCode != E_OK) {
            LOGE("[DeleteSyncDataItem] Get statement fail!, errCode:%d", errCode);
            return errCode;
        }
    }

    int errCode = SQLiteUtils::BindBlobToStatement(stmt, 1, dataItem.hashKey); // 1 means hash_key index
    if (errCode != E_OK) {
        SQLiteUtils::ResetStatement(stmt, true, errCode);
        return errCode;
    }
    if (mode_ != DistributedTableMode::COLLABORATION) {
        errCode = SQLiteUtils::BindTextToStatement(stmt, 2, dataItem.dev); // 2 means device index
        if (errCode != E_OK) {
            SQLiteUtils::ResetStatement(stmt, true, errCode);
            return errCode;
        }
    }
    errCode = SQLiteUtils::StepWithRetry(stmt, isMemDb_);
    if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
        errCode = E_OK;
    }
    SQLiteUtils::ResetStatement(stmt, false, errCode);  // Finalize outside.
    return errCode;
}

int SQLiteSingleVerRelationalStorageExecutor::SaveSyncDataItem(const DataItem &dataItem, sqlite3_stmt *&saveDataStmt,
    sqlite3_stmt *&rmDataStmt, const std::vector<FieldInfo> &fieldInfos, int64_t &rowid)
{
    if ((dataItem.flag & DataItem::DELETE_FLAG) != 0) {
        return DeleteSyncDataItem(dataItem, rmDataStmt);
    }
    if ((mode_ == DistributedTableMode::COLLABORATION &&
         table_.GetIdentifyKey().size() == 1u && table_.GetIdentifyKey().at(0) == "rowid") ||
        (mode_ == DistributedTableMode::SPLIT_BY_DEVICE &&
         table_.GetPrimaryKey().size() == 1u && table_.GetPrimaryKey().at(0) == "rowid") ||
        table_.GetAutoIncrement()) {  // No primary key of auto increment
        int errCode = DeleteSyncDataItem(dataItem, rmDataStmt);
        if (errCode != E_OK) {
            LOGE("Delete no pk data before insert failed, errCode=%d.", errCode);
            return errCode;
        }
    }
    OptRowDataWithLog data;
    int errCode = DataTransformer::DeSerializeDataItem(dataItem, data, fieldInfos);
    if (errCode != E_OK) {
        LOGE("[RelationalStorageExecutor] DeSerialize dataItem failed! errCode = [%d]", errCode);
        return errCode;
    }

    if (data.optionalData.size() != table_.GetFields().size()) {
        LOGW("Remote data has different fields with local data. Remote size:%zu, local size:%zu",
            data.optionalData.size(), table_.GetFields().size());
    }

    auto putSize = std::min(data.optionalData.size(), table_.GetFields().size());
    for (size_t cid = 0; cid < putSize; ++cid) {
        const auto &fieldData = data.optionalData[cid];
        errCode = BindDataValueByType(saveDataStmt, fieldData, cid + 1);
        if (errCode != E_OK) {
            LOGE("Bind data failed, errCode:%d, cid:%zu.", errCode, cid + 1);
            return errCode;
        }
    }

    errCode = SQLiteUtils::StepWithRetry(saveDataStmt, isMemDb_);
    if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
        rowid = SQLiteUtils::GetLastRowId(dbHandle_);
        errCode = E_OK;
    }
    return errCode;
}

int SQLiteSingleVerRelationalStorageExecutor::DeleteSyncLog(const DataItem &dataItem, sqlite3_stmt *&stmt)
{
    if (stmt == nullptr) {
        std::string sql = "DELETE FROM " + DBConstant::RELATIONAL_PREFIX + baseTblName_ + "_log ";
        if (mode_ == DistributedTableMode::COLLABORATION) {
            sql += "WHERE hash_key=?";
        } else {
            sql += "WHERE hash_key=? AND device=?";
        }

        int errCode = SQLiteUtils::GetStatement(dbHandle_, sql, stmt);
        if (errCode != E_OK) {
            LOGE("[DeleteSyncLog] Get statement fail!");
            return errCode;
        }
    }

    int errCode = SQLiteUtils::BindBlobToStatement(stmt, 1, dataItem.hashKey); // 1 means hashkey index
    if (errCode != E_OK) {
        SQLiteUtils::ResetStatement(stmt, true, errCode);
        return errCode;
    }
    if (mode_ != DistributedTableMode::COLLABORATION) {
        errCode = SQLiteUtils::BindTextToStatement(stmt, 2, dataItem.dev); // 2 means device index
        if (errCode != E_OK) {
            SQLiteUtils::ResetStatement(stmt, true, errCode);
            return errCode;
        }
    }
    errCode = SQLiteUtils::StepWithRetry(stmt, isMemDb_);
    if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
        errCode = E_OK;
    }
    SQLiteUtils::ResetStatement(stmt, false, errCode);  // Finalize outside.
    return errCode;
}

int SQLiteSingleVerRelationalStorageExecutor::ProcessMissQueryData(const DataItem &item, sqlite3_stmt *&rmDataStmt,
    sqlite3_stmt *&rmLogStmt)
{
    int errCode = DeleteSyncDataItem(item, rmDataStmt);
    if (errCode != E_OK) {
        return errCode;
    }
    return DeleteSyncLog(item, rmLogStmt);
}

int SQLiteSingleVerRelationalStorageExecutor::GetSyncDataPre(const DataItem &dataItem, DataItem &itemGet)
{
    if (saveStmt_.queryStmt == nullptr) {
        return -E_INVALID_ARGS;
    }
    int errCode = SQLiteUtils::BindBlobToStatement(saveStmt_.queryStmt, 1, dataItem.hashKey); // 1 index for hashkey
    if (errCode != E_OK) {
        return errCode;
    }
    if (mode_ != DistributedTableMode::COLLABORATION) {
        errCode = SQLiteUtils::BindTextToStatement(saveStmt_.queryStmt, 2, dataItem.dev); // 2 index for devices
        if (errCode != E_OK) {
            return errCode;
        }
    }

    LogInfo logInfoGet;
    errCode = SQLiteUtils::StepWithRetry(saveStmt_.queryStmt, isMemDb_);
    if (errCode != SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
        errCode = -E_NOT_FOUND;
    } else {
        errCode = GetLogData(saveStmt_.queryStmt, logInfoGet);
    }
    itemGet.timestamp = logInfoGet.timestamp;
    SQLiteUtils::ResetStatement(saveStmt_.queryStmt, false, errCode);
    return errCode;
}

int SQLiteSingleVerRelationalStorageExecutor::CheckDataConflictDefeated(const DataItem &dataItem, bool &isDefeated)
{
    if ((dataItem.flag & DataItem::REMOTE_DEVICE_DATA_MISS_QUERY) != DataItem::REMOTE_DEVICE_DATA_MISS_QUERY &&
        mode_ == DistributedTableMode::SPLIT_BY_DEVICE) {
        isDefeated = false; // no need to solve conflict except miss query data
        return E_OK;
    }

    DataItem itemGet;
    int errCode = GetSyncDataPre(dataItem, itemGet);
    if (errCode != E_OK && errCode != -E_NOT_FOUND) {
        LOGE("Failed to get raw data. %d", errCode);
        return errCode;
    }
    isDefeated = (dataItem.timestamp <= itemGet.timestamp); // defeated if item timestamp is earlier then raw data
    return E_OK;
}

int SQLiteSingleVerRelationalStorageExecutor::SaveSyncDataItem(const std::vector<FieldInfo> &fieldInfos,
    const std::string &deviceName, DataItem &item)
{
    item.dev = deviceName;
    bool isDefeated = false;
    int errCode = CheckDataConflictDefeated(item, isDefeated);
    if (errCode != E_OK) {
        LOGE("check data conflict failed. %d", errCode);
        return errCode;
    }

    if (isDefeated) {
        LOGD("Data was defeated.");
        return E_OK;
    }
    if ((item.flag & DataItem::REMOTE_DEVICE_DATA_MISS_QUERY) != 0) {
        return ProcessMissQueryData(item, saveStmt_.rmDataStmt, saveStmt_.rmLogStmt);
    }
    int64_t rowid = -1;
    errCode = SaveSyncDataItem(item, saveStmt_.saveDataStmt, saveStmt_.rmDataStmt, fieldInfos, rowid);
    if (errCode == E_OK || errCode == -E_NOT_FOUND) {
        errCode = SaveSyncLog(saveStmt_.saveLogStmt, saveStmt_.queryStmt, item, rowid);
    }
    return errCode;
}

int SQLiteSingleVerRelationalStorageExecutor::SaveSyncDataItems(const QueryObject &object,
    std::vector<DataItem> &dataItems, const std::string &deviceName)
{
    int errCode = PrepareForSavingData(object, saveStmt_.saveDataStmt);
    if (errCode != E_OK) {
        return errCode;
    }
    errCode = PrepareForSavingLog(object, deviceName, saveStmt_.saveLogStmt, saveStmt_.queryStmt);
    if (errCode != E_OK) {
        SQLiteUtils::ResetStatement(saveStmt_.saveDataStmt, true, errCode);
        return errCode;
    }
    std::vector<FieldInfo> fieldInfos;
    for (const auto &col: table_.GetFields()) {
        fieldInfos.push_back(col.second);
    }

    for (auto &item : dataItems) {
        if (item.neglect) { // Do not save this record if it is neglected
            continue;
        }
        errCode = SaveSyncDataItem(fieldInfos, deviceName, item);
        if (errCode != E_OK) {
            break;
        }
        // Need not reset rmDataStmt and rmLogStmt here.
        saveStmt_.ResetStatements(false);
    }
    if (errCode == -E_NOT_FOUND) {
        errCode = E_OK;
    }
    saveStmt_.ResetStatements(true);
    return errCode;
}

int SQLiteSingleVerRelationalStorageExecutor::SaveSyncItems(const QueryObject &object, std::vector<DataItem> &dataItems,
    const std::string &deviceName, const TableInfo &table, bool useTrans)
{
    if (useTrans) {
        int errCode = StartTransaction(TransactType::IMMEDIATE);
        if (errCode != E_OK) {
            return errCode;
        }
    }

    int errCode = SetLogTriggerStatus(false);
    if (errCode != E_OK) {
        goto END;
    }

    baseTblName_ = object.GetTableName();
    SetTableInfo(table);
    errCode = SaveSyncDataItems(object, dataItems, deviceName);
    if (errCode != E_OK) {
        LOGE("Save sync data items failed. errCode=%d", errCode);
        goto END;
    }

    errCode = SetLogTriggerStatus(true);
END:
    if (useTrans) {
        if (errCode == E_OK) {
            errCode = Commit();
        } else {
            (void)Rollback(); // Keep the error code of the first scene
        }
    }
    return errCode;
}

int SQLiteSingleVerRelationalStorageExecutor::GetDataItemForSync(sqlite3_stmt *stmt, DataItem &dataItem,
    bool isGettingDeletedData) const
{
    RowDataWithLog data;
    int errCode = GetLogData(stmt, data.logInfo);
    if (errCode != E_OK) {
        LOGE("relational data value transfer to kv fail");
        return errCode;
    }

    if (!isGettingDeletedData) {
        for (size_t cid = 0; cid < table_.GetFields().size(); ++cid) {
            DataValue value;
            errCode = SQLiteRelationalUtils::GetDataValueByType(stmt, cid + DBConstant::RELATIONAL_LOG_TABLE_FIELD_NUM,
                value);
            if (errCode != E_OK) {
                return errCode;
            }
            data.rowData.push_back(std::move(value));
        }
    }

    errCode = DataTransformer::SerializeDataItem(data,
        isGettingDeletedData ? std::vector<FieldInfo>() : table_.GetFieldInfos(), dataItem);
    if (errCode != E_OK) {
        LOGE("relational data value transfer to kv fail");
    }
    return errCode;
}

int SQLiteSingleVerRelationalStorageExecutor::GetMissQueryData(sqlite3_stmt *fullStmt, DataItem &item)
{
    int errCode = GetDataItemForSync(fullStmt, item, false);
    if (errCode != E_OK) {
        return errCode;
    }
    item.value = {};
    item.flag |= DataItem::REMOTE_DEVICE_DATA_MISS_QUERY;
    return errCode;
}

namespace {
int StepNext(bool isMemDB, sqlite3_stmt *stmt, Timestamp &timestamp)
{
    if (stmt == nullptr) {
        return -E_INVALID_ARGS;
    }
    int errCode = SQLiteUtils::StepWithRetry(stmt, isMemDB);
    if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
        timestamp = INT64_MAX;
        errCode = E_OK;
    } else if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
        timestamp = static_cast<uint64_t>(sqlite3_column_int64(stmt, 3));  // 3 means timestamp index
        errCode = E_OK;
    }
    return errCode;
}

int AppendData(const DataSizeSpecInfo &sizeInfo, size_t appendLength, size_t &overLongSize, size_t &dataTotalSize,
    std::vector<DataItem> &dataItems, DataItem &&item)
{
    // If one record is over 4M, ignore it.
    if (item.value.size() > DBConstant::MAX_VALUE_SIZE) {
        overLongSize++;
    } else {
        // If dataTotalSize value is bigger than blockSize value , reserve the surplus data item.
        dataTotalSize += GetDataItemSerialSize(item, appendLength);
        if ((dataTotalSize > sizeInfo.blockSize && !dataItems.empty()) || dataItems.size() >= sizeInfo.packetSize) {
            return -E_UNFINISHED;
        } else {
            dataItems.push_back(item);
        }
    }
    return E_OK;
}
}

int SQLiteSingleVerRelationalStorageExecutor::GetQueryDataAndStepNext(bool isFirstTime, bool isGettingDeletedData,
    sqlite3_stmt *queryStmt, DataItem &item, Timestamp &queryTime)
{
    if (!isFirstTime) { // For the first time, never step before, can get nothing
        int errCode = GetDataItemForSync(queryStmt, item, isGettingDeletedData);
        if (errCode != E_OK) {
            return errCode;
        }
    }
    return StepNext(isMemDb_, queryStmt, queryTime);
}

int SQLiteSingleVerRelationalStorageExecutor::GetMissQueryDataAndStepNext(sqlite3_stmt *fullStmt, DataItem &item,
    Timestamp &missQueryTime)
{
    int errCode = GetMissQueryData(fullStmt, item);
    if (errCode != E_OK) {
        return errCode;
    }
    return StepNext(isMemDb_, fullStmt, missQueryTime);
}

int SQLiteSingleVerRelationalStorageExecutor::GetSyncDataByQuery(std::vector<DataItem> &dataItems, size_t appendLength,
    const DataSizeSpecInfo &sizeInfo, std::function<int(sqlite3 *, sqlite3_stmt *&, sqlite3_stmt *&, bool &)> getStmt,
    const TableInfo &tableInfo)
{
    baseTblName_ = tableInfo.GetTableName();
    SetTableInfo(tableInfo);
    sqlite3_stmt *queryStmt = nullptr;
    sqlite3_stmt *fullStmt = nullptr;
    bool isGettingDeletedData = false;
    int errCode = getStmt(dbHandle_, queryStmt, fullStmt, isGettingDeletedData);
    if (errCode != E_OK) {
        return errCode;
    }

    Timestamp queryTime = 0;
    Timestamp missQueryTime = (fullStmt == nullptr ? INT64_MAX : 0);

    bool isFirstTime = true;
    size_t dataTotalSize = 0;
    size_t overLongSize = 0;
    do {
        DataItem item;
        if (queryTime < missQueryTime) {
            errCode = GetQueryDataAndStepNext(isFirstTime, isGettingDeletedData, queryStmt, item, queryTime);
        } else if (queryTime == missQueryTime) {
            errCode = GetQueryDataAndStepNext(isFirstTime, isGettingDeletedData, queryStmt, item, queryTime);
            if (errCode != E_OK) {
                break;
            }
            errCode = StepNext(isMemDb_, fullStmt, missQueryTime);
        } else {
            errCode = GetMissQueryDataAndStepNext(fullStmt, item, missQueryTime);
        }

        if (errCode == E_OK && !isFirstTime) {
            errCode = AppendData(sizeInfo, appendLength, overLongSize, dataTotalSize, dataItems, std::move(item));
        }

        if (errCode != E_OK) {
            break;
        }

        isFirstTime = false;
        if (queryTime == INT64_MAX && missQueryTime == INT64_MAX) {
            errCode = -E_FINISHED;
            break;
        }
    } while (true);
    LOGI("Get sync data finished, rc:%d, record size:%zu, overlong size:%zu, isDeleted:%d",
        errCode, dataItems.size(), overLongSize, isGettingDeletedData);
    SQLiteUtils::ResetStatement(queryStmt, true, errCode);
    SQLiteUtils::ResetStatement(fullStmt, true, errCode);
    return errCode;
}

int SQLiteSingleVerRelationalStorageExecutor::CheckDBModeForRelational()
{
    std::string journalMode;
    int errCode = SQLiteUtils::GetJournalMode(dbHandle_, journalMode);

    for (auto &c : journalMode) { // convert to lowercase
        c = static_cast<char>(std::tolower(c));
    }

    if (errCode == E_OK && journalMode != "wal") {
        LOGE("Not support journal mode %s for relational db, expect wal mode.", journalMode.c_str());
        return -E_NOT_SUPPORT;
    }
    return errCode;
}

int SQLiteSingleVerRelationalStorageExecutor::DeleteDistributedDeviceTable(const std::string &device,
    const std::string &tableName)
{
    std::vector<std::string> deviceTables;
    int errCode = GetDeviceTableName(dbHandle_, tableName, device, deviceTables);
    if (errCode != E_OK) {
        LOGE("Get device table name for alter table failed. %d", errCode);
        return errCode;
    }

    LOGD("Begin to delete device table: deviceTable[%zu]", deviceTables.size());
    for (const auto &table : deviceTables) {
        std::string deleteSql = "DROP TABLE IF EXISTS " + table + ";"; // drop the found table
        errCode = SQLiteUtils::ExecuteRawSQL(dbHandle_, deleteSql);
        if (errCode != E_OK) {
            LOGE("Delete device data failed. %d", errCode);
            break;
        }
    }
    return errCode;
}

int SQLiteSingleVerRelationalStorageExecutor::DeleteDistributedDeviceTableLog(const std::string &device,
    const std::string &tableName, const std::map<std::string, TableInfo> &tables)
{
    int errCode = E_OK;
    for (const auto &it : tables) {
        if (!tableName.empty() && it.second.GetTableName() != tableName) {
            continue; // Table name is specified, skip others
        }
        std::string deleteLogSql = "DELETE FROM " + DBConstant::RELATIONAL_PREFIX + it.second.GetTableName() +
            "_log WHERE device = ?";
        sqlite3_stmt *deleteLogStmt = nullptr;
        errCode = SQLiteUtils::GetStatement(dbHandle_, deleteLogSql, deleteLogStmt);
        if (errCode != E_OK) {
            LOGE("Get delete device data log statement failed. %d", errCode);
            return errCode;
        }

        errCode = SQLiteUtils::BindTextToStatement(deleteLogStmt, 1, device);
        if (errCode != E_OK) {
            LOGE("Bind device to delete data log statement failed. %d", errCode);
            SQLiteUtils::ResetStatement(deleteLogStmt, true, errCode);
            return errCode;
        }

        errCode = SQLiteUtils::StepWithRetry(deleteLogStmt);
        if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
            errCode = E_OK;
        } else {
            LOGE("Delete data log failed. %d", errCode);
        }

        SQLiteUtils::ResetStatement(deleteLogStmt, true, errCode);
    }

    return errCode;
}

int SQLiteSingleVerRelationalStorageExecutor::DeleteDistributedLogTable(const std::string &tableName)
{
    if (tableName.empty()) {
        return -E_INVALID_ARGS;
    }
    std::string logTableName = DBConstant::RELATIONAL_PREFIX + tableName + "_log";
    std::string deleteSql = "DROP TABLE IF EXISTS " + logTableName + ";";
    int errCode = SQLiteUtils::ExecuteRawSQL(dbHandle_, deleteSql);
    if (errCode != E_OK) {
        LOGE("Delete distributed log table failed. %d", errCode);
    }
    return errCode;
}

int SQLiteSingleVerRelationalStorageExecutor::CheckAndCleanDistributedTable(const std::vector<std::string> &tableNames,
    std::vector<std::string> &missingTables)
{
    if (tableNames.empty()) {
        return E_OK;
    }
    const std::string checkSql = "SELECT name FROM sqlite_master WHERE type='table' AND name=?;";
    sqlite3_stmt *stmt = nullptr;
    int errCode = SQLiteUtils::GetStatement(dbHandle_, checkSql, stmt);
    if (errCode != E_OK) {
        SQLiteUtils::ResetStatement(stmt, true, errCode);
        return errCode;
    }
    for (const auto &tableName : tableNames) {
        errCode = SQLiteUtils::BindTextToStatement(stmt, 1, tableName); // 1: tablename bind index
        if (errCode != E_OK) {
            LOGE("Bind table name to check distributed table statement failed. %d", errCode);
            break;
        }

        errCode = SQLiteUtils::StepWithRetry(stmt, false);
        if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) { // The table in schema was dropped
            errCode = DeleteDistributedDeviceTable({}, tableName); // Clean the auxiliary tables for the dropped table
            if (errCode != E_OK) {
                LOGE("Delete device tables for missing distributed table failed. %d", errCode);
                break;
            }
            errCode = DeleteDistributedLogTable(tableName);
            if (errCode != E_OK) {
                LOGE("Delete log tables for missing distributed table failed. %d", errCode);
                break;
            }
            missingTables.emplace_back(tableName);
        } else if (errCode != SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
            LOGE("Check distributed table failed. %d", errCode);
            break;
        }
        errCode = E_OK; // Check result ok for distributed table is still exists
        SQLiteUtils::ResetStatement(stmt, false, errCode);
    }
    SQLiteUtils::ResetStatement(stmt, true, errCode);
    return CheckCorruptedStatus(errCode);
}

int SQLiteSingleVerRelationalStorageExecutor::CreateDistributedDeviceTable(const std::string &device,
    const TableInfo &baseTbl)
{
    if (dbHandle_ == nullptr) {
        return -E_INVALID_DB;
    }

    if (device.empty() || !baseTbl.IsValid()) {
        return -E_INVALID_ARGS;
    }

    std::string deviceTableName = DBCommon::GetDistributedTableName(device, baseTbl.GetTableName());
    int errCode = SQLiteUtils::CreateSameStuTable(dbHandle_, baseTbl, deviceTableName);
    if (errCode != E_OK) {
        LOGE("Create device table failed. %d", errCode);
        return errCode;
    }

    errCode = SQLiteUtils::CloneIndexes(dbHandle_, baseTbl.GetTableName(), deviceTableName);
    if (errCode != E_OK) {
        LOGE("Copy index to device table failed. %d", errCode);
    }
    return errCode;
}

int SQLiteSingleVerRelationalStorageExecutor::CheckQueryObjectLegal(const TableInfo &table, QueryObject &query,
    const std::string &schemaVersion)
{
    if (dbHandle_ == nullptr) {
        return -E_INVALID_DB;
    }

    TableInfo newTable;
    int errCode = SQLiteUtils::AnalysisSchema(dbHandle_, table.GetTableName(), newTable);
    if (errCode != E_OK && errCode != -E_NOT_FOUND) {
        LOGE("Check new schema failed. %d", errCode);
        return errCode;
    } else {
        errCode = table.CompareWithTable(newTable, schemaVersion);
        if (errCode != -E_RELATIONAL_TABLE_EQUAL && errCode != -E_RELATIONAL_TABLE_COMPATIBLE) {
            LOGE("Check schema failed, schema was changed. %d", errCode);
            return -E_DISTRIBUTED_SCHEMA_CHANGED;
        } else {
            errCode = E_OK;
        }
    }

    SqliteQueryHelper helper = query.GetQueryHelper(errCode);
    if (errCode != E_OK) {
        LOGE("Get query helper for check query failed. %d", errCode);
        return errCode;
    }

    if (!query.IsQueryForRelationalDB()) {
        LOGE("Not support for this query type.");
        return -E_NOT_SUPPORT;
    }

    SyncTimeRange defaultTimeRange;
    sqlite3_stmt *stmt = nullptr;
    errCode = helper.GetRelationalQueryStatement(dbHandle_, defaultTimeRange.beginTime, defaultTimeRange.endTime, {},
        stmt);
    if (errCode != E_OK) {
        LOGE("Get query statement for check query failed. %d", errCode);
    }

    SQLiteUtils::ResetStatement(stmt, true, errCode);
    return errCode;
}

int SQLiteSingleVerRelationalStorageExecutor::SaveSyncDataStmt::ResetStatements(bool isNeedFinalize)
{
    int errCode = E_OK;
    if (saveDataStmt != nullptr) {
        SQLiteUtils::ResetStatement(saveDataStmt, isNeedFinalize, errCode);
    }
    if (saveLogStmt != nullptr) {
        SQLiteUtils::ResetStatement(saveLogStmt, isNeedFinalize, errCode);
    }
    if (queryStmt != nullptr) {
        SQLiteUtils::ResetStatement(queryStmt, isNeedFinalize, errCode);
    }
    if (rmDataStmt != nullptr) {
        SQLiteUtils::ResetStatement(rmDataStmt, isNeedFinalize, errCode);
    }
    if (rmLogStmt != nullptr) {
        SQLiteUtils::ResetStatement(rmLogStmt, isNeedFinalize, errCode);
    }
    return errCode;
}

int SQLiteSingleVerRelationalStorageExecutor::GetMaxTimestamp(const std::vector<std::string> &tableNames,
    Timestamp &maxTimestamp) const
{
    maxTimestamp = 0;
    for (const auto &tableName : tableNames) {
        const std::string sql = "SELECT max(timestamp) from " + DBConstant::RELATIONAL_PREFIX + tableName + "_log;";
        sqlite3_stmt *stmt = nullptr;
        int errCode = SQLiteUtils::GetStatement(dbHandle_, sql, stmt);
        if (errCode != E_OK) {
            return errCode;
        }
        errCode = SQLiteUtils::StepWithRetry(stmt, isMemDb_);
        if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
            maxTimestamp = std::max(maxTimestamp, static_cast<Timestamp>(sqlite3_column_int64(stmt, 0))); // 0 is index
            errCode = E_OK;
        }
        SQLiteUtils::ResetStatement(stmt, true, errCode);
        if (errCode != E_OK) {
            maxTimestamp = 0;
            return errCode;
        }
    }
    return E_OK;
}

int SQLiteSingleVerRelationalStorageExecutor::SetLogTriggerStatus(bool status)
{
    const std::string key = "log_trigger_switch";
    std::string val = status ? "true" : "false";
    std::string sql = "INSERT OR REPLACE INTO " + DBConstant::RELATIONAL_PREFIX + "metadata" +
        " VALUES ('" + key + "', '" + val + "')";
    int errCode = SQLiteUtils::ExecuteRawSQL(dbHandle_, sql);
    if (errCode != E_OK) {
        LOGE("Set log trigger to %s failed. errCode=%d", val.c_str(), errCode);
    }
    return errCode;
}

namespace {
int GetRowDatas(sqlite3_stmt *stmt, bool isMemDb, std::vector<std::string> &colNames,
    std::vector<RelationalRowData *> &data)
{
    size_t totalLength = 0;
    do {
        int errCode = SQLiteUtils::StepWithRetry(stmt, isMemDb);
        if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
            return E_OK;
        } else if (errCode != SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
            LOGE("Get data by bind sql failed:%d", errCode);
            return errCode;
        }

        if (colNames.empty()) {
            SQLiteUtils::GetSelectCols(stmt, colNames);  // Get column names.
        }
        auto relaRowData = new (std::nothrow) RelationalRowDataImpl(SQLiteRelationalUtils::GetSelectValues(stmt));
        if (relaRowData == nullptr) {
            LOGE("ExecuteQueryBySqlStmt OOM");
            return -E_OUT_OF_MEMORY;
        }

        auto dataSz = relaRowData->CalcLength();
        if (dataSz == 0) {  // invalid data
            delete relaRowData;
            relaRowData = nullptr;
            continue;
        }

        totalLength += static_cast<size_t>(dataSz);
        if (totalLength > static_cast<uint32_t>(DBConstant::MAX_REMOTEDATA_SIZE)) {  // the set has been full
            delete relaRowData;
            relaRowData = nullptr;
            LOGE("ExecuteQueryBySqlStmt OVERSIZE");
            return -E_REMOTE_OVER_SIZE;
        }
        data.push_back(relaRowData);
    } while (true);
    return E_OK;
}
}

// sql must not be empty, colNames and data must be empty
int SQLiteSingleVerRelationalStorageExecutor::ExecuteQueryBySqlStmt(const std::string &sql,
    const std::vector<std::string> &bindArgs, int packetSize, std::vector<std::string> &colNames,
    std::vector<RelationalRowData *> &data)
{
    int errCode = SQLiteUtils::SetAuthorizer(dbHandle_, &PermitSelect);
    if (errCode != E_OK) {
        return errCode;
    }

    sqlite3_stmt *stmt = nullptr;
    errCode = SQLiteUtils::GetStatement(dbHandle_, sql, stmt);
    if (errCode != E_OK) {
        (void)SQLiteUtils::SetAuthorizer(dbHandle_, nullptr);
        return errCode;
    }
    ResFinalizer finalizer([this, &stmt, &errCode] {
        (void)SQLiteUtils::SetAuthorizer(this->dbHandle_, nullptr);
        SQLiteUtils::ResetStatement(stmt, true, errCode);
    });
    for (size_t i = 0; i < bindArgs.size(); ++i) {
        errCode = SQLiteUtils::BindTextToStatement(stmt, i + 1, bindArgs.at(i));
        if (errCode != E_OK) {
            return errCode;
        }
    }
    return GetRowDatas(stmt, isMemDb_, colNames, data);
}

int SQLiteSingleVerRelationalStorageExecutor::CheckEncryptedOrCorrupted() const
{
    if (dbHandle_ == nullptr) {
        return -E_INVALID_DB;
    }

    int errCode = SQLiteUtils::ExecuteRawSQL(dbHandle_, "SELECT count(*) FROM sqlite_master;");
    if (errCode != E_OK) {
        LOGE("[SingVerRelaExec] CheckEncryptedOrCorrupted failed:%d", errCode);
    }
    return errCode;
}
} // namespace DistributedDB
#endif
