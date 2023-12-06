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

#ifndef SQLITE_UTILS_H
#define SQLITE_UTILS_H

#include <mutex>
#include <string>
#include <vector>
#include "sqlite_import.h"

#include "db_types.h"
#include "schema_object.h"
#include "store_types.h"
#ifdef RELATIONAL_STORE
#include "relational_schema_object.h"
#endif

namespace DistributedDB {
enum class TransactType {
    DEFERRED,
    IMMEDIATE,
};

struct TransactFunc {
    void (*xFunc)(sqlite3_context*, int, sqlite3_value**) = nullptr;
    void (*xStep)(sqlite3_context*, int, sqlite3_value**) = nullptr;
    void (*xFinal)(sqlite3_context*) = nullptr;
    void (*xDestroy)(void*) = nullptr;
};

namespace TriggerMode {
enum class TriggerModeEnum {
    NONE,
    INSERT,
    UPDATE,
    DELETE
};

std::string GetTriggerModeString(TriggerModeEnum mode);
}

struct OpenDbProperties {
    std::string uri {};
    bool createIfNecessary = true;
    bool isMemDb = false;
    std::vector<std::string> sqls {};
    CipherType cipherType = CipherType::AES_256_GCM;
    CipherPassword passwd {};
    std::string schema {};
    std::string subdir {};
    SecurityOption securityOpt {};
    int conflictReslovePolicy = DEFAULT_LAST_WIN;
    bool createDirByStoreIdOnly = false;
    uint32_t iterTimes = DBConstant::DEFAULT_ITER_TIMES;
};

class SQLiteUtils {
public:
    // Initialize the SQLiteUtils with the given properties.
    static int OpenDatabase(const OpenDbProperties &properties, sqlite3 *&db, bool setWal = true);

    // Check the statement and prepare the new if statement is null
    static int GetStatement(sqlite3 *db, const std::string &sql, sqlite3_stmt *&statement);

    // Bind the Text to the statement
    static int BindTextToStatement(sqlite3_stmt *statement, int index, const std::string &str);

    static int BindInt64ToStatement(sqlite3_stmt *statement, int index, int64_t value);

    // Bind the blob to the statement
    static int BindBlobToStatement(sqlite3_stmt *statement, int index, const std::vector<uint8_t> &value,
        bool permEmpty = true);

    // Reset the statement
    static void ResetStatement(sqlite3_stmt *&statement, bool isNeedFinalize, int &errCode);

    // Step the statement
    static int StepWithRetry(sqlite3_stmt *statement, bool isMemDb = false);

    // Bind the prefix key range
    static int BindPrefixKey(sqlite3_stmt *statement, int index, const Key &keyPrefix);

    static int BeginTransaction(sqlite3 *db, TransactType type = TransactType::DEFERRED);

    static int CommitTransaction(sqlite3 *db);

    static int RollbackTransaction(sqlite3 *db);

    static int ExecuteRawSQL(sqlite3 *db, const std::string &sql);

    static int SetKey(sqlite3 *db, CipherType type, const CipherPassword &passwd, bool setWal,
        uint32_t iterTimes = DBConstant::DEFAULT_ITER_TIMES);

    static int GetColumnBlobValue(sqlite3_stmt *statement, int index, std::vector<uint8_t> &value);

    static int GetColumnTextValue(sqlite3_stmt *statement, int index, std::string &value);

    static int ExportDatabase(sqlite3 *db, CipherType type, const CipherPassword &passwd, const std::string &newDbName);

    static int ExportDatabase(const std::string &srcFile, CipherType type, const CipherPassword &srcPasswd,
        const std::string &targetFile, const CipherPassword &passwd);

    static int Rekey(sqlite3 *db, const CipherPassword &passwd);

    static int GetVersion(const OpenDbProperties &properties, int &version);

    static int GetVersion(sqlite3 *db, int &version);

    static int GetJournalMode(sqlite3 *db, std::string &mode);

    static int SetUserVer(const OpenDbProperties &properties, int version);

    static int SetUserVer(sqlite3 *db, int version);

    static int MapSQLiteErrno(int errCode);

    static int SaveSchema(const OpenDbProperties &properties);

    static int SaveSchema(sqlite3 *db, const std::string &strSchema);

    static int GetSchema(const OpenDbProperties &properties, std::string &strSchema);

    static int GetSchema(sqlite3 *db, std::string &strSchema);

    static int IncreaseIndex(sqlite3 *db, const IndexName &name, const IndexInfo &info, SchemaType type,
        uint32_t skipSize);

    static int ChangeIndex(sqlite3 *db, const IndexName &name, const IndexInfo &info, SchemaType type,
        uint32_t skipSize);

    static int DecreaseIndex(sqlite3 *db, const IndexName &name);

    static int RegisterJsonFunctions(sqlite3 *db);
    // Register the flatBufferExtract function if the schema is of flatBuffer-type(To be refactor)
    static int RegisterFlatBufferFunction(sqlite3 *db, const std::string &inSchema);

    static int RegisterMetaDataUpdateFunction(sqlite3 *db);

    static int GetDbSize(const std::string &dir, const std::string &dbName, uint64_t &size);

    static int ExplainPlan(sqlite3 *db, const std::string &execSql, bool isQueryPlan);

    static int AttachNewDatabase(sqlite3 *db, CipherType type, const CipherPassword &password,
        const std::string &attachDbAbsPath, const std::string &attachAsName = "backup");

    static int AttachNewDatabaseInner(sqlite3 *db, CipherType type, const CipherPassword &password,
        const std::string &attachDbAbsPath, const std::string &attachAsName);

    static int CreateMetaDatabase(const std::string &metaDbPath);

    static int CheckIntegrity(sqlite3 *db, const std::string &sql);
#ifdef RELATIONAL_STORE
    static int RegisterCalcHash(sqlite3 *db);

    static int RegisterGetSysTime(sqlite3 *db);

    static int CreateRelationalLogTable(sqlite3 *db, const std::string &oriTableName);
    static int CreateRelationalMetaTable(sqlite3 *db);

    static int AddRelationalLogTableTrigger(sqlite3 *db, const std::string &identity, const TableInfo &table);
    static int AnalysisSchema(sqlite3 *db, const std::string &tableName, TableInfo &table);

    static int CreateSameStuTable(sqlite3 *db, const TableInfo &baseTbl, const std::string &newTableName);
    static int CloneIndexes(sqlite3 *db, const std::string &oriTableName, const std::string &newTableName);

    static int GetRelationalSchema(sqlite3 *db, std::string &schema);

    static int GetLogTableVersion(sqlite3 *db, std::string &version);
#endif

    static int DropTriggerByName(sqlite3 *db, const std::string &name);

    static int ExpandedSql(sqlite3_stmt *stmt, std::string &basicString);

    static void ExecuteCheckPoint(sqlite3 *db);

    static int CheckTableEmpty(sqlite3 *db, const std::string &tableName, bool &isEmpty);

    static int SetPersistWalMode(sqlite3 *db);

    static int CheckSchemaChanged(sqlite3_stmt *stmt, const TableInfo &table, int offset);

    static int64_t GetLastRowId(sqlite3 *db);

    static std::string GetLastErrorMsg();

    static std::string CalcPrimaryKeyHash(const std::string &references, const TableInfo &table);

    static int SetAuthorizer(sqlite3 *db, int (*xAuth)(void*, int, const char*, const char*, const char*, const char*));

    static void GetSelectCols(sqlite3_stmt *stmt, std::vector<std::string> &colNames);

    static int SetKeyInner(sqlite3 *db, CipherType type, const CipherPassword &passwd, uint32_t iterTimes);

private:

    static int CreateDataBase(const OpenDbProperties &properties, sqlite3 *&dbTemp, bool setWal);

    static int SetBusyTimeout(sqlite3 *db, int timeout);

    static void JsonExtractByPath(sqlite3_context *ctx, int argc, sqlite3_value **argv);

    static void JsonExtractInnerFunc(sqlite3_context *ctx, const ValueObject &inValue, const FieldPath &inPath);

    static void FlatBufferExtractByPath(sqlite3_context *ctx, int argc, sqlite3_value **argv);

    static void FlatBufferExtractInnerFunc(sqlite3_context *ctx, const SchemaObject &schema, const RawValue &inValue,
        RawString inPath);

    static void ExtractReturn(sqlite3_context *ctx, FieldType type, const FieldValue &value);

    static void CalcHashKey(sqlite3_context *ctx, int argc, sqlite3_value **argv);

    static void GetSysTime(sqlite3_context *ctx, int argc, sqlite3_value **argv);

    static int SetDataBaseProperty(sqlite3 *db, const OpenDbProperties &properties, bool setWal,
        const std::vector<std::string> &sqls);

    static int RegisterFunction(sqlite3 *db, const std::string &funcName, int nArg, void *uData, TransactFunc &func);

#ifndef OMIT_ENCRYPT
    static int SetCipherSettings(sqlite3 *db, CipherType type, uint32_t iterTimes);

    static std::string GetCipherName(CipherType type);
#endif

    static void UpdateMetaDataWithinTrigger(sqlite3_context *ctx, int argc, sqlite3_value **argv);

    static void SqliteLogCallback(void *data, int err, const char *msg);

    static int UpdateCipherShaAlgo(sqlite3 *db, bool setWal, CipherType type, const CipherPassword &passwd,
        uint32_t iterTimes);

    static std::mutex logMutex_;
    static std::string lastErrorMsg_;
};
} // namespace DistributedDB

#endif // SQLITE_UTILS_H
