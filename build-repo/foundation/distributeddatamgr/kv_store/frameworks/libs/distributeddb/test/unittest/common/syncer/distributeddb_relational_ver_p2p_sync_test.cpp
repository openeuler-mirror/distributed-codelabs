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
#include <gtest/gtest.h>

#include "db_common.h"
#include "db_constant.h"
#include "distributeddb_data_generate_unit_test.h"
#include "distributeddb_tools_unit_test.h"
#include "isyncer.h"
#include "kv_virtual_device.h"
#include "platform_specific.h"
#include "process_system_api_adapter_impl.h"
#include "relational_schema_object.h"
#include "relational_store_manager.h"
#include "relational_virtual_device.h"
#include "runtime_config.h"
#include "virtual_relational_ver_sync_db_interface.h"

using namespace testing::ext;
using namespace DistributedDB;
using namespace DistributedDBUnitTest;

namespace {
    const std::string DEVICE_A = "real_device";
    const std::string DEVICE_B = "deviceB";
    const std::string DEVICE_C = "deviceC";
    const std::string DEVICE_D = "deviceD";
    const std::string g_tableName = "TEST_TABLE";

#ifndef OMIT_ENCRYPT
    bool g_isAfterRekey = false;
    const string CORRECT_KEY = "a correct key";
    CipherPassword g_correctPasswd;
    const string REKEY_KEY = "a key after rekey";
    CipherPassword g_rekeyPasswd;
    const string INCORRECT_KEY = "a incorrect key";
    CipherPassword g_incorrectPasswd;
    const int DEFAULT_ITER = 5000;
#endif

    const int ONE_HUNDERED = 100;
    const char DEFAULT_CHAR = 'D';
    const std::string DEFAULT_TEXT = "This is a text";
    const std::vector<uint8_t> DEFAULT_BLOB(ONE_HUNDERED, DEFAULT_CHAR);
    const std::string SHA1_ALGO_SQL = "PRAGMA codec_hmac_algo=SHA1";
    const std::string SHA256_ALGO_SQL = "PRAGMA codec_hmac_algo=SHA256";
    const std::string SHA256_ALGO_REKEY_SQL = "PRAGMA codec_rekey_hmac_algo=SHA256";

    RelationalStoreManager g_mgr(APP_ID, USER_ID);
    std::string g_testDir;
    std::string g_dbDir;
    std::string g_id;
    std::vector<StorageType> g_storageType = {
        StorageType::STORAGE_TYPE_INTEGER, StorageType::STORAGE_TYPE_REAL,
        StorageType::STORAGE_TYPE_TEXT, StorageType::STORAGE_TYPE_BLOB
    };
    DistributedDBToolsUnitTest g_tool;
    RelationalStoreDelegate* g_rdbDelegatePtr = nullptr;
    VirtualCommunicatorAggregator* g_communicatorAggregator = nullptr;
    RelationalVirtualDevice *g_deviceB = nullptr;
    RelationalVirtualDevice *g_deviceC = nullptr;
    KvVirtualDevice *g_deviceD = nullptr;
    std::vector<FieldInfo> g_fieldInfoList;
    RelationalStoreObserverUnitTest *g_observer = nullptr;
    std::string GetDeviceTableName(const std::string &tableName)
    {
        return "naturalbase_rdb_aux_" +
            tableName + "_" + DBCommon::TransferStringToHex(DBCommon::TransferHashString(DEVICE_B));
    }

    void OpenStore()
    {
        if (g_observer == nullptr) {
            g_observer = new (std::nothrow) RelationalStoreObserverUnitTest();
        }
        RelationalStoreDelegate::Option option;
        option.observer = g_observer;
#ifndef OMIT_ENCRYPT
        option.isEncryptedDb = true;
        option.iterateTimes = DEFAULT_ITER;
        option.passwd = g_isAfterRekey ? g_rekeyPasswd : g_correctPasswd;
        option.cipher = CipherType::DEFAULT;
#endif
        g_mgr.OpenStore(g_dbDir, STORE_ID_1, option, g_rdbDelegatePtr);
        ASSERT_TRUE(g_rdbDelegatePtr != nullptr);
    }

    int GetDB(sqlite3 *&db, bool isSha256Algo = true)
    {
        int flag = SQLITE_OPEN_URI | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
        const auto &dbPath = g_dbDir;
        int rc = sqlite3_open_v2(dbPath.c_str(), &db, flag, nullptr);
        if (rc != SQLITE_OK) {
            return rc;
        }
#ifndef OMIT_ENCRYPT
        string sql =
            "PRAGMA key='" + (g_isAfterRekey ? REKEY_KEY : CORRECT_KEY) + "';"
            "PRAGMA codec_kdf_iter=" + std::to_string(DEFAULT_ITER) + ";";
        EXPECT_EQ(sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr), SQLITE_OK);
        if (isSha256Algo) {
            EXPECT_EQ(SQLiteUtils::ExecuteRawSQL(db, SHA256_ALGO_SQL), E_OK);
        } else {
            EXPECT_EQ(SQLiteUtils::ExecuteRawSQL(db, SHA1_ALGO_SQL), E_OK);
        }
        EXPECT_EQ(SQLiteUtils::ExecuteRawSQL(db, SHA256_ALGO_REKEY_SQL), E_OK);
#endif
        EXPECT_EQ(SQLiteUtils::RegisterCalcHash(db), E_OK);
        EXPECT_EQ(SQLiteUtils::RegisterGetSysTime(db), E_OK);
        EXPECT_EQ(sqlite3_exec(db, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr), SQLITE_OK);
        return rc;
    }

    std::string GetType(StorageType type)
    {
        static std::map<StorageType, std::string> typeMap = {
            {StorageType::STORAGE_TYPE_INTEGER, "INT"},
            {StorageType::STORAGE_TYPE_REAL, "DOUBLE"},
            {StorageType::STORAGE_TYPE_TEXT, "TEXT"},
            {StorageType::STORAGE_TYPE_BLOB, "BLOB"}
        };
        if (typeMap.find(type) == typeMap.end()) {
            type = StorageType::STORAGE_TYPE_INTEGER;
        }
        return typeMap[type];
    }

    int DropTable(sqlite3 *db, const std::string &tableName)
    {
        std::string sql = "DROP TABLE " + tableName + ";";
        return sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);
    }

    int CreateTable(sqlite3 *db, const std::vector<FieldInfo> &fieldInfoList, const std::string &tableName)
    {
        std::string sql = "CREATE TABLE " + tableName + "(";
        int index = 0;
        for (const auto &field : fieldInfoList) {
            if (index != 0) {
                sql += ",";
            }
            sql += field.GetFieldName() + " ";
            std::string type = GetType(field.GetStorageType());
            sql += type + " ";
            if (index == 0) {
                sql += "PRIMARY KEY NOT NULL ";
            }
            index++;
        }
        sql += ");";
        int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);
        return rc;
    }

    int PrepareInsert(sqlite3 *db, sqlite3_stmt *&statement,
        std::vector<FieldInfo> fieldInfoList, const std::string &tableName)
    {
        std::string sql = "INSERT OR REPLACE INTO " + tableName + "(";
        int index = 0;
        for (const auto &fieldInfo : fieldInfoList) {
            if (index != 0) {
                sql += ",";
            }
            sql += fieldInfo.GetFieldName();
            index++;
        }
        sql += ") VALUES (";
        while (index > 0) {
            sql += "?";
            if (index != 1) {
                sql += ", ";
            }
            index--;
        }
        sql += ");";
        return sqlite3_prepare_v2(db, sql.c_str(), -1, &statement, nullptr);
    }

    int SimulateCommitData(sqlite3 *db, sqlite3_stmt *&statement)
    {
        sqlite3_exec(db, "BEGIN IMMEDIATE TRANSACTION", nullptr, nullptr, nullptr);

        int rc = sqlite3_step(statement);

        sqlite3_exec(db, "COMMIT TRANSACTION", nullptr, nullptr, nullptr);
        return rc;
    }

    void BindValue(const DataValue &item, sqlite3_stmt *stmt, int col)
    {
        switch (item.GetType()) {
            case StorageType::STORAGE_TYPE_INTEGER: {
                int64_t intData = 0;
                (void)item.GetInt64(intData);
                EXPECT_EQ(sqlite3_bind_int64(stmt, col, intData), SQLITE_OK);
                break;
            }

            case StorageType::STORAGE_TYPE_REAL: {
                double doubleData = 0;
                (void)item.GetDouble(doubleData);
                EXPECT_EQ(sqlite3_bind_double(stmt, col, doubleData), SQLITE_OK);
                break;
            }

            case StorageType::STORAGE_TYPE_TEXT: {
                std::string strData;
                (void)item.GetText(strData);
                EXPECT_EQ(SQLiteUtils::BindTextToStatement(stmt, col, strData), E_OK);
                break;
            }

            case StorageType::STORAGE_TYPE_BLOB: {
                Blob blob;
                (void)item.GetBlob(blob);
                std::vector<uint8_t> blobData(blob.GetData(), blob.GetData() + blob.GetSize());
                EXPECT_EQ(SQLiteUtils::BindBlobToStatement(stmt, col, blobData, true), E_OK);
                break;
            }

            case StorageType::STORAGE_TYPE_NULL: {
                EXPECT_EQ(SQLiteUtils::MapSQLiteErrno(sqlite3_bind_null(stmt, col)), E_OK);
                break;
            }

            default:
                break;
        }
    }

    void InsertValue(sqlite3 *db, std::map<std::string, DataValue> &dataMap,
        const std::vector<FieldInfo> &fieldInfoList, const std::string &tableName)
    {
        sqlite3_stmt *stmt = nullptr;
        EXPECT_EQ(PrepareInsert(db, stmt, fieldInfoList, tableName), SQLITE_OK);
        for (int i = 0; i < static_cast<int>(fieldInfoList.size()); ++i) {
            const auto &fieldName = fieldInfoList[i].GetFieldName();
            ASSERT_TRUE(dataMap.find(fieldName) != dataMap.end());
            const auto &item = dataMap[fieldName];
            const int index = i + 1;
            BindValue(item, stmt, index);
        }
        EXPECT_EQ(SimulateCommitData(db, stmt), SQLITE_DONE);
        sqlite3_finalize(stmt);
    }

    void InsertValue(sqlite3 *db, std::map<std::string, DataValue> &dataMap)
    {
        InsertValue(db, dataMap, g_fieldInfoList, g_tableName);
    }

    void SetNull(DataValue &dataValue)
    {
        dataValue.ResetValue();
    }

    void SetInt64(DataValue &dataValue)
    {
        dataValue = INT64_MAX;
    }

    void SetDouble(DataValue &dataValue)
    {
        dataValue = 1.0;
    }

    void SetText(DataValue &dataValue)
    {
        dataValue.SetText(DEFAULT_TEXT);
    }

    void SetBlob(DataValue &dataValue)
    {
        Blob blob;
        blob.WriteBlob(DEFAULT_BLOB.data(), DEFAULT_BLOB.size());
        dataValue.SetBlob(blob);
    }

    void GenerateValue(std::map<std::string, DataValue> &dataMap, std::vector<FieldInfo> &fieldInfoList)
    {
        static std::map<StorageType, void(*)(DataValue&)> typeMapFunction = {
            {StorageType::STORAGE_TYPE_NULL,    &SetNull},
            {StorageType::STORAGE_TYPE_INTEGER, &SetInt64},
            {StorageType::STORAGE_TYPE_REAL,    &SetDouble},
            {StorageType::STORAGE_TYPE_TEXT,    &SetText},
            {StorageType::STORAGE_TYPE_BLOB,    &SetBlob}
        };
        for (auto &fieldInfo : fieldInfoList) {
            DataValue dataValue;
            if (typeMapFunction.find(fieldInfo.GetStorageType()) == typeMapFunction.end()) {
                fieldInfo.SetStorageType(StorageType::STORAGE_TYPE_NULL);
            }
            typeMapFunction[fieldInfo.GetStorageType()](dataValue);
            dataMap[fieldInfo.GetFieldName()] = std::move(dataValue);
        }
    }

    void InsertFieldInfo(std::vector<FieldInfo> &fieldInfoList)
    {
        fieldInfoList.clear();
        FieldInfo columnFirst;
        columnFirst.SetFieldName("ID");
        columnFirst.SetStorageType(StorageType::STORAGE_TYPE_INTEGER);
        columnFirst.SetColumnId(0); // the first column
        FieldInfo columnSecond;
        columnSecond.SetFieldName("NAME");
        columnSecond.SetStorageType(StorageType::STORAGE_TYPE_TEXT);
        columnSecond.SetColumnId(1); // the 2nd column
        FieldInfo columnThird;
        columnThird.SetFieldName("AGE");
        columnThird.SetStorageType(StorageType::STORAGE_TYPE_INTEGER);
        columnThird.SetColumnId(2); // the 3rd column(index 2 base 0)
        fieldInfoList.push_back(columnFirst);
        fieldInfoList.push_back(columnSecond);
        fieldInfoList.push_back(columnThird);
    }

    void BlockSync(const Query &query, SyncMode syncMode, DBStatus exceptStatus,
        const std::vector<std::string> &devices)
    {
        std::map<std::string, std::vector<TableStatus>> statusMap;
        SyncStatusCallback callBack = [&statusMap](
            const std::map<std::string, std::vector<TableStatus>> &devicesMap) {
            statusMap = devicesMap;
        };
        DBStatus callStatus = g_rdbDelegatePtr->Sync(devices, syncMode, query, callBack, true);
        EXPECT_EQ(callStatus, OK);
        for (const auto &tablesRes : statusMap) {
            for (const auto &tableStatus : tablesRes.second) {
                EXPECT_EQ(tableStatus.status, exceptStatus);
            }
        }
    }

    void BlockSync(const std::string &tableName, SyncMode syncMode, DBStatus exceptStatus,
        const std::vector<std::string> &devices)
    {
        Query query = Query::Select(tableName);
        BlockSync(query, syncMode, exceptStatus, devices);
    }

    void BlockSync(SyncMode syncMode, DBStatus exceptStatus, const std::vector<std::string> &devices)
    {
        BlockSync(g_tableName, syncMode, exceptStatus, devices);
    }

    int PrepareSelect(sqlite3 *db, sqlite3_stmt *&statement, const std::string &table)
    {
        const std::string sql = "SELECT * FROM " + table;
        return sqlite3_prepare_v2(db, sql.c_str(), -1, &statement, nullptr);
    }

    void GetDataValue(sqlite3_stmt *statement, int col, DataValue &dataValue)
    {
        int type = sqlite3_column_type(statement, col);
        switch (type) {
            case SQLITE_INTEGER: {
                dataValue = static_cast<int64_t>(sqlite3_column_int64(statement, col));
                break;
            }
            case SQLITE_FLOAT: {
                dataValue = sqlite3_column_double(statement, col);
                break;
            }
            case SQLITE_TEXT: {
                std::string str;
                SQLiteUtils::GetColumnTextValue(statement, col, str);
                dataValue.SetText(str);
                break;
            }
            case SQLITE_BLOB: {
                std::vector<uint8_t> blobValue;
                (void)SQLiteUtils::GetColumnBlobValue(statement, col, blobValue);
                Blob blob;
                blob.WriteBlob(blobValue.data(), static_cast<uint32_t>(blobValue.size()));
                dataValue.SetBlob(blob);
                break;
            }
            case SQLITE_NULL:
                break;
            default:
                LOGW("unknown type[%d] column[%d] ignore", type, col);
        }
    }

    void GetSyncDataStep(std::map<std::string, DataValue> &dataMap, sqlite3_stmt *statement,
        const std::vector<FieldInfo> &fieldInfoList)
    {
        int columnCount = sqlite3_column_count(statement);
        ASSERT_EQ(static_cast<size_t>(columnCount), fieldInfoList.size());
        for (int col = 0; col < columnCount; ++col) {
            DataValue dataValue;
            GetDataValue(statement, col, dataValue);
            dataMap[fieldInfoList.at(col).GetFieldName()] = std::move(dataValue);
        }
    }

    void GetSyncData(sqlite3 *db, std::map<std::string, DataValue> &dataMap, const std::string &tableName,
        const std::vector<FieldInfo> &fieldInfoList)
    {
        sqlite3_stmt *statement = nullptr;
        EXPECT_EQ(PrepareSelect(db, statement, GetDeviceTableName(tableName)), SQLITE_OK);
        while (true) {
            int rc = sqlite3_step(statement);
            if (rc != SQLITE_ROW) {
                LOGD("GetSyncData Exist by code[%d]", rc);
                break;
            }
            GetSyncDataStep(dataMap, statement, fieldInfoList);
        }
        sqlite3_finalize(statement);
    }

    void InsertValueToDB(std::map<std::string, DataValue> &dataMap)
    {
        sqlite3 *db = nullptr;
        EXPECT_EQ(GetDB(db), SQLITE_OK);
        InsertValue(db, dataMap);
        sqlite3_close(db);
    }

    void PrepareBasicTable(const std::string &tableName, std::vector<FieldInfo> &fieldInfoList,
        const std::vector<RelationalVirtualDevice *> &remoteDeviceVec, bool createDistributedTable = true)
    {
        sqlite3 *db = nullptr;
        EXPECT_EQ(GetDB(db), SQLITE_OK);
        if (fieldInfoList.empty()) {
            InsertFieldInfo(fieldInfoList);
        }
        for (auto &dev : remoteDeviceVec) {
            dev->SetLocalFieldInfo(fieldInfoList);
        }
        EXPECT_EQ(CreateTable(db, fieldInfoList, tableName), SQLITE_OK);
        TableInfo tableInfo;
        SQLiteUtils::AnalysisSchema(db, tableName, tableInfo);
        for (auto &dev : remoteDeviceVec) {
            dev->SetTableInfo(tableInfo);
        }
        if (createDistributedTable) {
            EXPECT_EQ(g_rdbDelegatePtr->CreateDistributedTable(tableName), OK);
        }

        sqlite3_close(db);
    }

    void PrepareEnvironment(std::map<std::string, DataValue> &dataMap,
        std::vector<RelationalVirtualDevice *> remoteDeviceVec)
    {
        PrepareBasicTable(g_tableName, g_fieldInfoList, remoteDeviceVec);
        GenerateValue(dataMap, g_fieldInfoList);
        InsertValueToDB(dataMap);
    }

    void PrepareVirtualEnvironment(std::map<std::string, DataValue> &dataMap, const std::string &tableName,
        std::vector<FieldInfo> &fieldInfoList, const std::vector<RelationalVirtualDevice *> remoteDeviceVec,
        bool createDistributedTable = true)
    {
        PrepareBasicTable(tableName, fieldInfoList, remoteDeviceVec, createDistributedTable);
        GenerateValue(dataMap, fieldInfoList);
        VirtualRowData virtualRowData;
        for (const auto &item : dataMap) {
            virtualRowData.objectData.PutDataValue(item.first, item.second);
        }
        virtualRowData.logInfo.timestamp = 1;
        g_deviceB->PutData(tableName, {virtualRowData});
    }

    void PrepareVirtualEnvironment(std::map<std::string, DataValue> &dataMap,
        const std::vector<RelationalVirtualDevice *> remoteDeviceVec, bool createDistributedTable = true)
    {
        PrepareVirtualEnvironment(dataMap, g_tableName, g_fieldInfoList, remoteDeviceVec, createDistributedTable);
    }

    void CheckData(const std::map<std::string, DataValue> &targetMap, const std::string &tableName,
        const std::vector<FieldInfo> &fieldInfoList)
    {
        std::map<std::string, DataValue> dataMap;
        sqlite3 *db = nullptr;
        EXPECT_EQ(GetDB(db), SQLITE_OK);
        GetSyncData(db, dataMap, tableName, fieldInfoList);
        sqlite3_close(db);

        for (const auto &[fieldName, dataValue] : targetMap) {
            ASSERT_TRUE(dataMap.find(fieldName) != dataMap.end());
            EXPECT_TRUE(dataMap[fieldName] == dataValue);
        }
    }

    void CheckData(const std::map<std::string, DataValue> &targetMap)
    {
        CheckData(targetMap, g_tableName, g_fieldInfoList);
    }

    void CheckVirtualData(const std::string &tableName, std::map<std::string, DataValue> &data)
    {
        std::vector<VirtualRowData> targetData;
        g_deviceB->GetAllSyncData(tableName, targetData);

        ASSERT_EQ(targetData.size(), 1u);
        for (auto &[field, value] : data) {
            DataValue target;
            EXPECT_EQ(targetData[0].objectData.GetDataValue(field, target), E_OK);
            LOGD("field %s actual_val[%s] except_val[%s]", field.c_str(), target.ToString().c_str(),
                value.ToString().c_str());
            EXPECT_TRUE(target == value);
        }
    }

    void CheckVirtualData(std::map<std::string, DataValue> &data)
    {
        CheckVirtualData(g_tableName, data);
    }

    void GetFieldInfo(std::vector<FieldInfo> &fieldInfoList, std::vector<StorageType> typeList)
    {
        fieldInfoList.clear();
        for (size_t index = 0; index < typeList.size(); index++) {
            const auto &type = typeList[index];
            FieldInfo fieldInfo;
            fieldInfo.SetFieldName("field_" + std::to_string(index));
            fieldInfo.SetColumnId(index);
            fieldInfo.SetStorageType(type);
            fieldInfoList.push_back(fieldInfo);
        }
    }

    void InsertValueToDB(std::map<std::string, DataValue> &dataMap,
        std::vector<FieldInfo> fieldInfoList, const std::string &tableName)
    {
        sqlite3 *db = nullptr;
        EXPECT_EQ(GetDB(db), SQLITE_OK);
        InsertValue(db, dataMap, fieldInfoList, tableName);
        sqlite3_close(db);
    }

    void PrepareEnvironment(std::map<std::string, DataValue> &dataMap, const std::string &tableName,
        std::vector<FieldInfo> &localFieldInfoList, std::vector<FieldInfo> &remoteFieldInfoList,
        std::vector<RelationalVirtualDevice *> remoteDeviceVec)
    {
        sqlite3 *db = nullptr;
        EXPECT_EQ(GetDB(db), SQLITE_OK);

        EXPECT_EQ(CreateTable(db, remoteFieldInfoList, tableName), SQLITE_OK);
        TableInfo tableInfo;
        SQLiteUtils::AnalysisSchema(db, tableName, tableInfo);
        for (auto &dev : remoteDeviceVec) {
            dev->SetTableInfo(tableInfo);
        }

        EXPECT_EQ(DropTable(db, tableName), SQLITE_OK);
        EXPECT_EQ(CreateTable(db, localFieldInfoList, tableName), SQLITE_OK);
        EXPECT_EQ(g_rdbDelegatePtr->CreateDistributedTable(tableName), OK);

        sqlite3_close(db);

        GenerateValue(dataMap, localFieldInfoList);
        InsertValueToDB(dataMap, localFieldInfoList, tableName);
        for (auto &dev : remoteDeviceVec) {
            dev->SetLocalFieldInfo(remoteFieldInfoList);
        }
    }

    void PrepareEnvironment(std::map<std::string, DataValue> &dataMap,
        std::vector<FieldInfo> &localFieldInfoList, std::vector<FieldInfo> &remoteFieldInfoList,
        const std::vector<RelationalVirtualDevice *> remoteDeviceVec)
    {
        PrepareEnvironment(dataMap, g_tableName, localFieldInfoList, remoteFieldInfoList, remoteDeviceVec);
    }

    void CheckIdentify(RelationalStoreObserverUnitTest *observer)
    {
        ASSERT_NE(observer, nullptr);
        StoreProperty property = observer->GetStoreProperty();
        EXPECT_EQ(property.appId, APP_ID);
        EXPECT_EQ(property.storeId, STORE_ID_1);
        EXPECT_EQ(property.userId, USER_ID);
    }

    void CheckSearchData(std::shared_ptr<ResultSet> result, std::map<std::string, DataValue> &dataMap)
    {
        ASSERT_NE(result, nullptr);
        EXPECT_EQ(result->GetCount(), 1);
        ASSERT_TRUE(result->MoveToFirst());
        std::vector<string> columnNames;
        result->GetColumnNames(columnNames);
        ASSERT_EQ(columnNames.size(), dataMap.size());
        int index = 0;
        for (auto &column : columnNames) {
            ASSERT_TRUE(dataMap.find(column) != dataMap.end());
            LOGD("now check %s", column.c_str());
            if (dataMap[column].GetType() == StorageType::STORAGE_TYPE_INTEGER) {
                int64_t expectVal = 0;
                dataMap[column].GetInt64(expectVal);
                int64_t actualVal = 0;
                ASSERT_EQ(result->Get(index, actualVal), OK);
                EXPECT_EQ(expectVal, actualVal);
            } else if (dataMap[column].GetType() == StorageType::STORAGE_TYPE_TEXT) {
                std::string expectVal = "";
                dataMap[column].GetText(expectVal);
                std::string actualVal = "";
                ASSERT_EQ(result->Get(index, actualVal), OK);
                EXPECT_EQ(expectVal, actualVal);
            } else if (dataMap[column].GetType() == StorageType::STORAGE_TYPE_REAL) {
                double expectVal = 0;
                dataMap[column].GetDouble(expectVal);
                double actualVal = 0;
                ASSERT_EQ(result->Get(index, actualVal), OK);
                EXPECT_EQ(expectVal, actualVal);
            }
            index++;
        }
    }

    int GetCount(sqlite3 *db, const string &sql, size_t &count)
    {
        sqlite3_stmt *stmt = nullptr;
        int errCode = SQLiteUtils::GetStatement(db, sql, stmt);
        if (errCode != E_OK) {
            return errCode;
        }
        errCode = SQLiteUtils::StepWithRetry(stmt, false);
        if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
            count = static_cast<size_t>(sqlite3_column_int64(stmt, 0));
            errCode = E_OK;
        }
        SQLiteUtils::ResetStatement(stmt, true, errCode);
        return errCode;
    }

    void GenerateSecurityData(std::vector<SecurityLabel> &labelList, std::vector<SecurityFlag> &flagList)
    {
        labelList = {
            SecurityLabel::NOT_SET, SecurityLabel::S0,
            SecurityLabel::S1, SecurityLabel::S2,
            SecurityLabel::S3, SecurityLabel::S4
        };
        flagList = {
            SecurityFlag::ECE, SecurityFlag::SECE
        };
    }

    SecurityOption SelectSecurityOption(int &labelIndex, int &flagIndex,
        const std::vector<SecurityLabel> &labelList, const std::vector<SecurityFlag> &flagList)
    {
        SecurityOption option;
        if (labelIndex >= static_cast<int>(labelList.size()) || flagIndex >= static_cast<int>(flagList.size())) {
            return option;
        }
        option.securityLabel = labelList[labelIndex];
        option.securityFlag = flagList[flagIndex];
        labelIndex++;
        if (labelIndex >= static_cast<int>(labelList.size())) {
            labelIndex = 0;
            flagIndex++;
        }
        return option;
    }

    bool SelectSecurityEnd(int flagIndex, const std::vector<SecurityFlag> &flagList)
    {
        return flagIndex >= static_cast<int>(flagList.size());
    }

    DBStatus GetSecurityRes(const SecurityOption &localOption, const SecurityOption &remoteOption,
        bool checkDeviceResult)
    {
        if (!checkDeviceResult) {
            return SECURITY_OPTION_CHECK_ERROR;
        }
        if (localOption.securityLabel == static_cast<int>(SecurityLabel::NOT_SET) ||
            remoteOption.securityLabel == static_cast<int>(SecurityLabel::NOT_SET)) {
            return OK;
        }
        if (localOption.securityLabel != remoteOption.securityLabel) {
            return SECURITY_OPTION_CHECK_ERROR;
        }
        return OK;
    }

    void SyncWithSecurityCheck(const SecurityOption &localOption, const SecurityOption &remoteOption,
        bool remoteQuery, bool checkDeviceResult)
    {
        std::shared_ptr<ProcessSystemApiAdapterImpl> adapter = std::make_shared<ProcessSystemApiAdapterImpl>();
        adapter->ForkCheckDeviceSecurityAbility(
            [&localOption, &remoteOption, checkDeviceResult, remoteQuery](const std::string &devId,
                const SecurityOption &option) {
            if (remoteQuery) {
                EXPECT_TRUE(remoteOption == option);
                EXPECT_EQ(devId, DEVICE_A);
            } else {
                EXPECT_TRUE(localOption == option);
            }
            return checkDeviceResult;
        });
        adapter->ForkGetSecurityOption(
            [&localOption, &remoteOption](const std::string &filePath, SecurityOption &option) {
            if (filePath.empty()) {
                option = remoteOption;
            } else {
                option = localOption;
            }
            return OK;
        });
        RuntimeConfig::SetProcessSystemAPIAdapter(adapter);
        DBStatus resStatus = GetSecurityRes(localOption, remoteOption, checkDeviceResult);
        if (remoteQuery) {
            RemoteCondition condition;
            condition.sql = "SELECT * FROM " + g_tableName;
            std::shared_ptr<ResultSet> result = nullptr;
            ASSERT_NE(g_rdbDelegatePtr, nullptr);
            EXPECT_EQ(g_rdbDelegatePtr->RemoteQuery(DEVICE_B, condition, DBConstant::MIN_TIMEOUT, result), resStatus);
        } else {
            BlockSync(SYNC_MODE_PUSH_ONLY, resStatus, {DEVICE_B});
        }
        RuntimeConfig::SetProcessSystemAPIAdapter(nullptr);
        if (g_rdbDelegatePtr != nullptr) {
            LOGD("CloseStore Start");
            ASSERT_EQ(g_mgr.CloseStore(g_rdbDelegatePtr), OK);
            g_rdbDelegatePtr = nullptr;
        }
        OpenStore();
    }

    void TestWithSecurityCheck(bool remoteQuery)
    {
        /**
         * @tc.steps: step1. create table and open store
         * @tc.expected: step1. open store ok
         */
        std::map<std::string, DataValue> dataMap;
        if (remoteQuery) {
            PrepareEnvironment(dataMap, {g_deviceB});
        } else {
            PrepareVirtualEnvironment(dataMap, {g_deviceB});
        }
        ASSERT_NE(g_rdbDelegatePtr, nullptr);

        /**
         * @tc.steps: step2. generate test data
         */
        std::vector<SecurityLabel> labelList;
        std::vector<SecurityFlag> flagList;
        int localLabelIndex = 0;
        int localFlagIndex = 0;
        GenerateSecurityData(labelList, flagList);

        // loop local
        while (!SelectSecurityEnd(localFlagIndex, flagList)) {
            SecurityOption localOption = SelectSecurityOption(localLabelIndex, localFlagIndex, labelList, flagList);
            // loop remote
            int remoteLabelIndex = 0;
            int remoteFlagIndex = 0;
            while (!SelectSecurityEnd(remoteFlagIndex, flagList)) {
                SecurityOption remoteOption = SelectSecurityOption(remoteLabelIndex, remoteFlagIndex,
                    labelList, flagList);
                /**
                 * @tc.steps: step3. call sync
                 * @tc.expected: step3. sync result is based on security option
                 */
                SyncWithSecurityCheck(localOption, remoteOption, remoteQuery, true);
                SyncWithSecurityCheck(localOption, remoteOption, remoteQuery, false);
            }
        }
    }
}

class DistributedDBRelationalVerP2PSyncTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void DistributedDBRelationalVerP2PSyncTest::SetUpTestCase()
{
    /**
    * @tc.setup: Init datadir and Virtual Communicator.
    */
    DistributedDBToolsUnitTest::TestDirInit(g_testDir);
    g_dbDir = g_testDir + "/test.db";
    sqlite3 *db = nullptr;
    ASSERT_EQ(GetDB(db), SQLITE_OK);
    sqlite3_close(db);

    g_communicatorAggregator = new (std::nothrow) VirtualCommunicatorAggregator();
    ASSERT_TRUE(g_communicatorAggregator != nullptr);
    RuntimeContext::GetInstance()->SetCommunicatorAggregator(g_communicatorAggregator);

    g_id = g_mgr.GetRelationalStoreIdentifier(USER_ID, APP_ID, STORE_ID_1);

#ifndef OMIT_ENCRYPT
    g_correctPasswd.SetValue(reinterpret_cast<const uint8_t *>(CORRECT_KEY.data()), CORRECT_KEY.size());
    g_rekeyPasswd.SetValue(reinterpret_cast<const uint8_t *>(REKEY_KEY.data()), REKEY_KEY.size());
    g_incorrectPasswd.SetValue(reinterpret_cast<const uint8_t *>(INCORRECT_KEY.data()), INCORRECT_KEY.size());
#endif
}

void DistributedDBRelationalVerP2PSyncTest::TearDownTestCase()
{
    /**
    * @tc.teardown: Release virtual Communicator and clear data dir.
    */
    if (DistributedDBToolsUnitTest::RemoveTestDbFiles(g_testDir) != 0) {
        LOGE("rm test db files error!");
    }
    RuntimeContext::GetInstance()->SetCommunicatorAggregator(nullptr);
    LOGD("TearDownTestCase FINISH");
}

void DistributedDBRelationalVerP2PSyncTest::SetUp(void)
{
    DistributedDBToolsUnitTest::PrintTestCaseInfo();
    g_fieldInfoList.clear();
    /**
    * @tc.setup: create virtual device B, and get a KvStoreNbDelegate as deviceA
    */
    sqlite3 *db = nullptr;
    ASSERT_EQ(GetDB(db), SQLITE_OK);
    sqlite3_close(db);
    OpenStore();
    g_deviceB = new (std::nothrow) RelationalVirtualDevice(DEVICE_B);
    ASSERT_TRUE(g_deviceB != nullptr);
    g_deviceC = new (std::nothrow) RelationalVirtualDevice(DEVICE_C);
    ASSERT_TRUE(g_deviceC != nullptr);
    g_deviceD = new (std::nothrow) KvVirtualDevice(DEVICE_D);
    ASSERT_TRUE(g_deviceD != nullptr);
    auto *syncInterfaceB = new (std::nothrow) VirtualRelationalVerSyncDBInterface();
    auto *syncInterfaceC = new (std::nothrow) VirtualRelationalVerSyncDBInterface();
    auto *syncInterfaceD = new (std::nothrow) VirtualSingleVerSyncDBInterface();
    ASSERT_TRUE(syncInterfaceB != nullptr);
    ASSERT_TRUE(syncInterfaceC != nullptr);
    ASSERT_TRUE(syncInterfaceD != nullptr);
    ASSERT_EQ(g_deviceB->Initialize(g_communicatorAggregator, syncInterfaceB), E_OK);
    ASSERT_EQ(g_deviceC->Initialize(g_communicatorAggregator, syncInterfaceC), E_OK);
    ASSERT_EQ(g_deviceD->Initialize(g_communicatorAggregator, syncInterfaceD), E_OK);

    auto permissionCheckCallback = [] (const std::string &userId, const std::string &appId, const std::string &storeId,
        const std::string &deviceId, uint8_t flag) -> bool {
        return true;
    };
    EXPECT_EQ(RuntimeConfig::SetPermissionCheckCallback(permissionCheckCallback), OK);
}

void DistributedDBRelationalVerP2PSyncTest::TearDown(void)
{
    /**
    * @tc.teardown: Release device A, B, C
    */
    if (g_rdbDelegatePtr != nullptr) {
        LOGD("CloseStore Start");
        ASSERT_EQ(g_mgr.CloseStore(g_rdbDelegatePtr), OK);
        g_rdbDelegatePtr = nullptr;
    }
    if (g_deviceB != nullptr) {
        delete g_deviceB;
        g_deviceB = nullptr;
    }
    if (g_deviceC != nullptr) {
        delete g_deviceC;
        g_deviceC = nullptr;
    }
    if (g_deviceD != nullptr) {
        delete g_deviceD;
        g_deviceD = nullptr;
    }
    if (g_observer != nullptr) {
        delete g_observer;
        g_observer = nullptr;
    }
    PermissionCheckCallbackV2 nullCallback;
    EXPECT_EQ(RuntimeConfig::SetPermissionCheckCallback(nullCallback), OK);
    EXPECT_EQ(DistributedDBToolsUnitTest::RemoveTestDbFiles(g_testDir), OK);
    if (g_communicatorAggregator != nullptr) {
        g_communicatorAggregator->RegOnDispatch(nullptr);
    }
    g_isAfterRekey = false;
    LOGD("TearDown FINISH");
}

/**
* @tc.name: Normal Sync 001
* @tc.desc: Test normal push sync for add data.
* @tc.type: FUNC
* @tc.require: AR000GK58N
* @tc.author: zhangqiquan
*/
HWTEST_F(DistributedDBRelationalVerP2PSyncTest, NormalSync001, TestSize.Level0)
{
    std::map<std::string, DataValue> dataMap;
    PrepareEnvironment(dataMap, {g_deviceB});
    BlockSync(SyncMode::SYNC_MODE_PUSH_ONLY, OK, {DEVICE_B});

    CheckVirtualData(dataMap);
}

/**
* @tc.name: Normal Sync 002
* @tc.desc: Test normal pull sync for add data.
* @tc.type: FUNC
* @tc.require: AR000GK58N
* @tc.author: zhangqiquan
*/
HWTEST_F(DistributedDBRelationalVerP2PSyncTest, NormalSync002, TestSize.Level0)
{
    std::map<std::string, DataValue> dataMap;
    PrepareEnvironment(dataMap, {g_deviceB});

    Query query = Query::Select(g_tableName);
    g_deviceB->GenericVirtualDevice::Sync(DistributedDB::SYNC_MODE_PULL_ONLY, query, true);

    CheckVirtualData(dataMap);
}

/**
* @tc.name: Normal Sync 003
* @tc.desc: Test normal push sync for update data.
* @tc.type: FUNC
* @tc.require: AR000GK58N
* @tc.author: zhangqiquan
*/
HWTEST_F(DistributedDBRelationalVerP2PSyncTest, NormalSync003, TestSize.Level1)
{
    std::map<std::string, DataValue> dataMap;
    PrepareEnvironment(dataMap, {g_deviceB});

    BlockSync(SyncMode::SYNC_MODE_PUSH_ONLY, OK, {DEVICE_B});

    CheckVirtualData(dataMap);

    GenerateValue(dataMap, g_fieldInfoList);
    dataMap["AGE"] = static_cast<int64_t>(1);
    InsertValueToDB(dataMap);
    BlockSync(SyncMode::SYNC_MODE_PUSH_ONLY, OK, {DEVICE_B});

    CheckVirtualData(dataMap);
}

/**
* @tc.name: Normal Sync 004
* @tc.desc: Test normal push sync for delete data.
* @tc.type: FUNC
* @tc.require: AR000GK58N
* @tc.author: zhangqiquan
*/
HWTEST_F(DistributedDBRelationalVerP2PSyncTest, NormalSync004, TestSize.Level1)
{
    std::map<std::string, DataValue> dataMap;
    PrepareEnvironment(dataMap, {g_deviceB});

    BlockSync(SyncMode::SYNC_MODE_PUSH_ONLY, OK, {DEVICE_B});

    CheckVirtualData(dataMap);

    sqlite3 *db = nullptr;
    EXPECT_EQ(GetDB(db), SQLITE_OK);
    std::string sql = "DELETE FROM TEST_TABLE WHERE 1 = 1";
    EXPECT_EQ(SQLiteUtils::ExecuteRawSQL(db, sql), E_OK);
    sqlite3_close(db);

    BlockSync(SyncMode::SYNC_MODE_PUSH_ONLY, OK, {DEVICE_B});

    std::vector<VirtualRowData> dataList;
    EXPECT_EQ(g_deviceB->GetAllSyncData(g_tableName, dataList), E_OK);
    EXPECT_EQ(static_cast<int>(dataList.size()), 1);
    for (const auto &item : dataList) {
        EXPECT_EQ(item.logInfo.flag, DataItem::DELETE_FLAG);
    }
}

/**
* @tc.name: Normal Sync 005
* @tc.desc: Test normal push sync for add data.
* @tc.type: FUNC
* @tc.require: AR000GK58N
* @tc.author: zhangqiquan
*/
HWTEST_F(DistributedDBRelationalVerP2PSyncTest, NormalSync005, TestSize.Level0)
{
    std::map<std::string, DataValue> dataMap;
    PrepareVirtualEnvironment(dataMap, {g_deviceB});

    Query query = Query::Select(g_tableName);
    g_deviceB->GenericVirtualDevice::Sync(SYNC_MODE_PUSH_ONLY, query, true);

    CheckData(dataMap);

    g_rdbDelegatePtr->RemoveDeviceData(DEVICE_B);

    g_deviceB->GenericVirtualDevice::Sync(SYNC_MODE_PUSH_ONLY, query, true);

    CheckData(dataMap);
}

/**
* @tc.name: Normal Sync 007
* @tc.desc: Test normal sync for miss query data.
* @tc.type: FUNC
* @tc.require: AR000GK58N
* @tc.author: zhangqiquan
*/
HWTEST_F(DistributedDBRelationalVerP2PSyncTest, NormalSync007, TestSize.Level1)
{
    std::map<std::string, DataValue> dataMap;
    PrepareEnvironment(dataMap, {g_deviceB});

    Query query = Query::Select(g_tableName).EqualTo("NAME", DEFAULT_TEXT);
    BlockSync(query, SyncMode::SYNC_MODE_PUSH_ONLY, OK, {DEVICE_B});

    CheckVirtualData(dataMap);

    sqlite3 *db = nullptr;
    EXPECT_EQ(GetDB(db), SQLITE_OK);
    std::string sql = "UPDATE TEST_TABLE SET NAME = '' WHERE 1 = 1";
    EXPECT_EQ(SQLiteUtils::ExecuteRawSQL(db, sql), E_OK);
    sqlite3_close(db);

    BlockSync(query, SyncMode::SYNC_MODE_PUSH_ONLY, OK, {DEVICE_B});

    std::vector<VirtualRowData> dataList;
    EXPECT_EQ(g_deviceB->GetAllSyncData(g_tableName, dataList), E_OK);
    EXPECT_EQ(static_cast<int>(dataList.size()), 1);
    for (const auto &item : dataList) {
        EXPECT_EQ(item.logInfo.flag, DataItem::REMOTE_DEVICE_DATA_MISS_QUERY);
    }
}

/**
* @tc.name: Normal Sync 006
* @tc.desc: Test normal pull sync for add data.
* @tc.type: FUNC
* @tc.require: AR000GK58N
* @tc.author: zhangqiquan
*/
HWTEST_F(DistributedDBRelationalVerP2PSyncTest, NormalSync006, TestSize.Level1)
{
    std::map<std::string, DataValue> dataMap;
    PrepareVirtualEnvironment(dataMap, {g_deviceB});

    BlockSync(SYNC_MODE_PULL_ONLY, OK, {DEVICE_B});

    CheckData(dataMap);
}

/**
* @tc.name: AutoLaunchSync 001
* @tc.desc: Test rdb autoLaunch success when callback return true.
* @tc.type: FUNC
* @tc.require: AR000GK58N
* @tc.author: zhangqiquan
*/
HWTEST_F(DistributedDBRelationalVerP2PSyncTest, AutoLaunchSync001, TestSize.Level3)
{
    /**
     * @tc.steps: step1. open rdb store, create distribute table and insert data
     */
    std::map<std::string, DataValue> dataMap;
    PrepareVirtualEnvironment(dataMap, {g_deviceB});

    /**
     * @tc.steps: step2. set auto launch callBack
     */
    int currentStatus = 0;
    const AutoLaunchNotifier notifier = [&currentStatus](const std::string &userId,
        const std::string &appId, const std::string &storeId, AutoLaunchStatus status) {
        currentStatus = static_cast<int>(status);
    };
    const AutoLaunchRequestCallback callback = [&notifier](const std::string &identifier, AutoLaunchParam &param) {
        if (g_id != identifier) {
            return false;
        }
        param.path    = g_dbDir;
        param.appId   = APP_ID;
        param.userId  = USER_ID;
        param.storeId = STORE_ID_1;
        param.notifier = notifier;
#ifndef OMIT_ENCRYPT
        param.option.isEncryptedDb = true;
        param.option.cipher = CipherType::DEFAULT;
        param.option.passwd = g_correctPasswd;
        param.option.iterateTimes = DEFAULT_ITER;
#endif
        return true;
    };
    g_mgr.SetAutoLaunchRequestCallback(callback);
    /**
     * @tc.steps: step3. close store ensure communicator has closed
     */
    g_mgr.CloseStore(g_rdbDelegatePtr);
    g_rdbDelegatePtr = nullptr;
    /**
     * @tc.steps: step4. RunCommunicatorLackCallback to autolaunch store
     */
    LabelType labelType(g_id.begin(), g_id.end());
    g_communicatorAggregator->RunCommunicatorLackCallback(labelType);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(currentStatus, 0);
    /**
     * @tc.steps: step5. Call sync expect sync successful
     */
    Query query = Query::Select(g_tableName);
    EXPECT_EQ(g_deviceB->GenericVirtualDevice::Sync(SYNC_MODE_PUSH_ONLY, query, true), E_OK);
    EXPECT_EQ(currentStatus, AutoLaunchStatus::WRITE_OPENED);
    /**
     * @tc.steps: step6. check sync data ensure sync successful
     */
    CheckData(dataMap);

    OpenStore();
    std::this_thread::sleep_for(std::chrono::seconds(61)); // sleep 61s
    EXPECT_EQ(currentStatus, AutoLaunchStatus::WRITE_CLOSED);
}

/**
* @tc.name: AutoLaunchSyncAfterRekey_001
* @tc.desc: Test auto launch sync ok after rekey.
* @tc.type: FUNC
* @tc.require: AR000H68LL
* @tc.author: lidongwei
*/
#ifndef OMIT_ENCRYPT
HWTEST_F(DistributedDBRelationalVerP2PSyncTest, AutoLaunchSyncAfterRekey_001, TestSize.Level3)
{
    /**
     * @tc.steps: step1. open rdb store, create distribute table and insert data
     */
    std::map<std::string, DataValue> dataMap;
    PrepareVirtualEnvironment(dataMap, {g_deviceB});

    /**
     * @tc.steps: step2. set auto launch callBack
     */
    AutoLaunchParam encryptedParam { USER_ID, APP_ID, STORE_ID_1, AutoLaunchOption {}, nullptr, g_dbDir };
    encryptedParam.option.isEncryptedDb = true;
    encryptedParam.option.cipher = CipherType::DEFAULT;
    encryptedParam.option.passwd = g_correctPasswd;
    encryptedParam.option.iterateTimes = DEFAULT_ITER;
    AutoLaunchRequestCallback callback = [&encryptedParam](const std::string &identifier, AutoLaunchParam &param) {
        if (g_id != identifier) {
            return false;
        }
        param = encryptedParam;
        return true;
    };
    g_mgr.SetAutoLaunchRequestCallback(callback);
    /**
     * @tc.steps: step3. close store ensure communicator has closed
     */
    g_mgr.CloseStore(g_rdbDelegatePtr);
    g_rdbDelegatePtr = nullptr;
    /**
     * @tc.steps: step4. RunCommunicatorLackCallback to autolaunch store
     */
    LabelType labelType(g_id.begin(), g_id.end());
    g_communicatorAggregator->RunCommunicatorLackCallback(labelType);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    /**
     * @tc.steps: step5. Rekey
     */
    sqlite3 *db = nullptr;
    EXPECT_EQ(GetDB(db), SQLITE_OK);
    std::thread t1([&db] {
        std::string sql = "PARGMA rekey=" + REKEY_KEY;
        EXPECT_EQ(sqlite3_rekey(db, REKEY_KEY.data(), REKEY_KEY.size()), SQLITE_OK);
    });
    t1.join();
    g_isAfterRekey = true;

    /**
     * @tc.steps: step5. Call sync expect sync failed
     */
    Query query = Query::Select(g_tableName);
    EXPECT_EQ(g_deviceB->GenericVirtualDevice::Sync(SYNC_MODE_PUSH_ONLY, query, true), E_OK);
    size_t count = 0;
    GetCount(db, "SELECT count(*) FROM sqlite_master WHERE name='" + GetDeviceTableName(g_tableName) + "';", count);
    EXPECT_EQ(count, 0u);

    /**
     * @tc.steps: step6. Set callback.
     */
    encryptedParam.option.passwd = g_rekeyPasswd;
    g_mgr.SetAutoLaunchRequestCallback(callback);
    g_communicatorAggregator->RunCommunicatorLackCallback(labelType);
    std::this_thread::sleep_for(std::chrono::seconds(2));

    /**
     * @tc.steps: step5. Call sync expect sync success
     */
    EXPECT_EQ(g_deviceB->GenericVirtualDevice::Sync(SYNC_MODE_PUSH_ONLY, query, true), E_OK);
    GetCount(db, "SELECT count(*) FROM sqlite_master WHERE name='" + GetDeviceTableName(g_tableName) + "';", count);
    EXPECT_EQ(count, 1u);
    GetSyncData(db, dataMap, g_tableName, g_fieldInfoList);
    EXPECT_EQ(dataMap.size(), 3u);
    OpenStore();
    std::this_thread::sleep_for(std::chrono::minutes(1));
    sqlite3_close(db);
    db = nullptr;
}
#endif

/**
* @tc.name: AutoLaunchSync 002
* @tc.desc: Test rdb autoLaunch failed when callback return false.
* @tc.type: FUNC
* @tc.require: AR000GK58N
* @tc.author: zhangqiquan
*/
HWTEST_F(DistributedDBRelationalVerP2PSyncTest, AutoLaunchSync002, TestSize.Level3)
{
    /**
     * @tc.steps: step1. open rdb store, create distribute table and insert data
     */
    std::map<std::string, DataValue> dataMap;
    PrepareVirtualEnvironment(dataMap, {g_deviceB});

    /**
     * @tc.steps: step2. set auto launch callBack
     */
    const AutoLaunchRequestCallback callback = [](const std::string &identifier, AutoLaunchParam &param) {
        return false;
    };
    g_mgr.SetAutoLaunchRequestCallback(callback);
    /**
     * @tc.steps: step2. close store ensure communicator has closed
     */
    g_mgr.CloseStore(g_rdbDelegatePtr);
    g_rdbDelegatePtr = nullptr;
    /**
     * @tc.steps: step3. store can't autoLaunch because callback return false
     */
    LabelType labelType(g_id.begin(), g_id.end());
    g_communicatorAggregator->RunCommunicatorLackCallback(labelType);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    /**
     * @tc.steps: step4. Call sync expect sync fail
     */
    Query query = Query::Select(g_tableName);
    SyncOperation::UserCallback callBack = [](const std::map<std::string, int> &statusMap) {
        for (const auto &entry : statusMap) {
            EXPECT_EQ(entry.second, static_cast<int>(SyncOperation::OP_COMM_ABNORMAL));
        }
    };
    EXPECT_EQ(g_deviceB->GenericVirtualDevice::Sync(SYNC_MODE_PUSH_ONLY, query, callBack, true), E_OK);

    OpenStore();
    std::this_thread::sleep_for(std::chrono::minutes(1));
}

/**
* @tc.name: AutoLaunchSync 003
* @tc.desc: Test rdb autoLaunch failed when callback is nullptr.
* @tc.type: FUNC
* @tc.require: AR000GK58N
* @tc.author: zhangqiquan
*/
HWTEST_F(DistributedDBRelationalVerP2PSyncTest, AutoLaunchSync003, TestSize.Level3)
{
    /**
     * @tc.steps: step1. open rdb store, create distribute table and insert data
     */
    std::map<std::string, DataValue> dataMap;
    PrepareVirtualEnvironment(dataMap, {g_deviceB});

    g_mgr.SetAutoLaunchRequestCallback(nullptr);
    /**
     * @tc.steps: step2. close store ensure communicator has closed
     */
    g_mgr.CloseStore(g_rdbDelegatePtr);
    g_rdbDelegatePtr = nullptr;
    /**
     * @tc.steps: step3. store can't autoLaunch because callback is nullptr
     */
    LabelType labelType(g_id.begin(), g_id.end());
    g_communicatorAggregator->RunCommunicatorLackCallback(labelType);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    /**
     * @tc.steps: step4. Call sync expect sync fail
     */
    Query query = Query::Select(g_tableName);
    SyncOperation::UserCallback callBack = [](const std::map<std::string, int> &statusMap) {
        for (const auto &entry : statusMap) {
            EXPECT_EQ(entry.second, static_cast<int>(SyncOperation::OP_COMM_ABNORMAL));
        }
    };
    EXPECT_EQ(g_deviceB->GenericVirtualDevice::Sync(SYNC_MODE_PUSH_ONLY, query, callBack, true), E_OK);

    OpenStore();
    std::this_thread::sleep_for(std::chrono::minutes(1));
}

/**
* @tc.name: Ability Sync 001
* @tc.desc: Test ability sync success when has same schema.
* @tc.type: FUNC
* @tc.require: AR000GK58N
* @tc.author: zhangqiquan
*/
HWTEST_F(DistributedDBRelationalVerP2PSyncTest, AbilitySync001, TestSize.Level1)
{
    std::map<std::string, DataValue> dataMap;
    std::vector<FieldInfo> localFieldInfo;
    GetFieldInfo(localFieldInfo, g_storageType);

    PrepareEnvironment(dataMap, localFieldInfo, localFieldInfo, {g_deviceB});
    BlockSync(SyncMode::SYNC_MODE_PUSH_ONLY, OK, {DEVICE_B});

    CheckVirtualData(dataMap);
}

/**
* @tc.name: Ability Sync 002
* @tc.desc: Test ability sync failed when has different schema.
* @tc.type: FUNC
* @tc.require: AR000GK58N
* @tc.author: zhangqiquan
*/
HWTEST_F(DistributedDBRelationalVerP2PSyncTest, AbilitySync002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. set local schema is (BOOL, INTEGER, REAL, TEXT, BLOB, INTEGER)
     */
    std::map<std::string, DataValue> dataMap;
    std::vector<FieldInfo> localFieldInfo;
    std::vector<StorageType> localStorageType = g_storageType;
    localStorageType.push_back(StorageType::STORAGE_TYPE_INTEGER);
    GetFieldInfo(localFieldInfo, localStorageType);

    /**
     * @tc.steps: step2. set remote schema is (BOOL, INTEGER, REAL, TEXT, BLOB, TEXT)
     */
    std::vector<FieldInfo> remoteFieldInfo;
    std::vector<StorageType> remoteStorageType = g_storageType;
    remoteStorageType.push_back(StorageType::STORAGE_TYPE_TEXT);
    GetFieldInfo(remoteFieldInfo, remoteStorageType);

    /**
     * @tc.steps: step3. call sync
     * @tc.expected: sync fail when abilitySync
     */
    PrepareEnvironment(dataMap, localFieldInfo, remoteFieldInfo, {g_deviceB});
    BlockSync(SyncMode::SYNC_MODE_PUSH_ONLY, SCHEMA_MISMATCH, {DEVICE_B});
}

/**
* @tc.name: Ability Sync 003
* @tc.desc: Test ability sync failed when has different schema.
* @tc.type: FUNC
* @tc.require: AR000GK58N
* @tc.author: zhangqiquan
*/
HWTEST_F(DistributedDBRelationalVerP2PSyncTest, AbilitySync003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. set local and remote schema is (BOOL, INTEGER, REAL, TEXT, BLOB)
     */
    std::map<std::string, DataValue> dataMap;
    std::vector<FieldInfo> schema;
    std::vector<StorageType> localStorageType = g_storageType;
    GetFieldInfo(schema, localStorageType);

    /**
     * @tc.steps: step2. create table and insert data
     */
    PrepareEnvironment(dataMap, schema, schema, {g_deviceB});

    /**
     * @tc.steps: step3. change local table to (BOOL, INTEGER, REAL, TEXT, BLOB)
     * @tc.expected: sync fail
     */
    g_communicatorAggregator->RegOnDispatch([](const std::string &target, Message *inMsg) {
        if (target != "real_device") {
            return;
        }
        if (inMsg->GetMessageType() != TYPE_NOTIFY || inMsg->GetMessageId() != ABILITY_SYNC_MESSAGE) {
            return;
        }
        sqlite3 *db = nullptr;
        EXPECT_EQ(GetDB(db), SQLITE_OK);
        ASSERT_NE(db, nullptr);
        std::string alterSql = "ALTER TABLE " + g_tableName + " ADD COLUMN NEW_COLUMN TEXT DEFAULT 'DEFAULT_TEXT'";
        EXPECT_EQ(sqlite3_exec(db, alterSql.c_str(), nullptr, nullptr, nullptr), SQLITE_OK);
        EXPECT_EQ(sqlite3_close(db), SQLITE_OK);
        EXPECT_EQ(g_rdbDelegatePtr->CreateDistributedTable(g_tableName), OK);
    });

    BlockSync(SyncMode::SYNC_MODE_PUSH_ONLY, OK, {DEVICE_B});

    g_communicatorAggregator->RegOnDispatch(nullptr);
}

/**
* @tc.name: Ability Sync 004
* @tc.desc: Test ability sync failed when one device hasn't distributed table.
* @tc.type: FUNC
* @tc.require: AR000GK58N
* @tc.author: zhangqiquan
*/
HWTEST_F(DistributedDBRelationalVerP2PSyncTest, AbilitySync004, TestSize.Level1)
{
    std::map<std::string, DataValue> dataMap;
    PrepareVirtualEnvironment(dataMap, {g_deviceB}, false);

    Query query = Query::Select(g_tableName);
    int res = DB_ERROR;
    auto callBack = [&res](std::map<std::string, int> resMap) {
        if (resMap.find("real_device") != resMap.end()) {
            res = resMap["real_device"];
        }
    };
    EXPECT_EQ(g_deviceB->GenericVirtualDevice::Sync(DistributedDB::SYNC_MODE_PULL_ONLY, query, callBack, true), E_OK);
    EXPECT_EQ(res, static_cast<int>(SyncOperation::Status::OP_SCHEMA_INCOMPATIBLE));
}

/**
* @tc.name: WaterMark 001
* @tc.desc: Test sync success after erase waterMark.
* @tc.type: FUNC
* @tc.require: AR000GK58N
* @tc.author: zhangqiquan
*/
HWTEST_F(DistributedDBRelationalVerP2PSyncTest, WaterMark001, TestSize.Level1)
{
    std::map<std::string, DataValue> dataMap;
    PrepareEnvironment(dataMap, {g_deviceB});
    BlockSync(SyncMode::SYNC_MODE_PUSH_ONLY, OK, {DEVICE_B});

    CheckVirtualData(dataMap);

    EXPECT_EQ(g_rdbDelegatePtr->RemoveDeviceData(g_deviceB->GetDeviceId(), g_tableName), OK);
    g_deviceB->EraseSyncData(g_tableName);

    BlockSync(SyncMode::SYNC_MODE_PUSH_ONLY, OK, {DEVICE_B});

    CheckVirtualData(dataMap);
}

/*
* @tc.name: pressure sync 001
* @tc.desc: Test rdb sync different table at same time
* @tc.type: FUNC
* @tc.require: AR000GK58N
* @tc.author: zhangqiquan
*/
HWTEST_F(DistributedDBRelationalVerP2PSyncTest, PressureSync001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create table A and device A push data to device B
     * @tc.expected: step1. all is ok
     */
    std::map<std::string, DataValue> tableADataMap;
    std::vector<FieldInfo> tableAFieldInfo;
    std::vector<StorageType> localStorageType = g_storageType;
    localStorageType.push_back(StorageType::STORAGE_TYPE_INTEGER);
    GetFieldInfo(tableAFieldInfo, localStorageType);
    const std::string tableNameA = "TABLE_A";
    PrepareEnvironment(tableADataMap, tableNameA, tableAFieldInfo, tableAFieldInfo, {g_deviceB});

    /**
     * @tc.steps: step2. create table B and device B push data to device A
     * @tc.expected: step2. all is ok
     */
    std::map<std::string, DataValue> tableBDataMap;
    std::vector<FieldInfo> tableBFieldInfo;
    localStorageType = g_storageType;
    localStorageType.push_back(StorageType::STORAGE_TYPE_REAL);
    GetFieldInfo(tableBFieldInfo, localStorageType);
    const std::string tableNameB = "TABLE_B";
    PrepareVirtualEnvironment(tableBDataMap, tableNameB, tableBFieldInfo, {g_deviceB});

    std::condition_variable cv;
    bool subFinish = false;
    std::thread subThread = std::thread([&subFinish, &cv, &tableNameA, &tableADataMap]() {
        BlockSync(tableNameA, SyncMode::SYNC_MODE_PUSH_ONLY, OK, {DEVICE_B});

        CheckVirtualData(tableNameA, tableADataMap);
        subFinish = true;
        cv.notify_all();
    });
    subThread.detach();

    Query query = Query::Select(tableNameB);
    g_deviceB->GenericVirtualDevice::Sync(SYNC_MODE_PUSH_ONLY, query, true);
    CheckData(tableBDataMap, tableNameB, tableBFieldInfo);

    std::mutex mutex;
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, [&subFinish] { return subFinish; });
}

/*
* @tc.name: relation observer 001
* @tc.desc: Test relation observer while normal pull sync
* @tc.type: FUNC
* @tc.require:
* @tc.author: zhuwentao
*/
HWTEST_F(DistributedDBRelationalVerP2PSyncTest, Observer001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. device A create table and device B insert data and device C don't insert data
     * @tc.expected: step1. create and insert ok
     */
    g_observer->ResetToZero();
    std::map<std::string, DataValue> dataMap;
    PrepareVirtualEnvironment(dataMap, {g_deviceB, g_deviceC});
    /**
     * @tc.steps: step2. device A pull sync mode
     * @tc.expected: step2. sync ok
     */
    BlockSync(SyncMode::SYNC_MODE_PULL_ONLY, OK, {DEVICE_B, DEVICE_C});
    /**
     * @tc.steps: step3. device A check observer
     * @tc.expected: step2. data change device is deviceB
     */
    EXPECT_EQ(g_observer->GetCallCount(), 1u);
    EXPECT_EQ(g_observer->GetDataChangeDevice(), DEVICE_B);
    CheckIdentify(g_observer);
}

/**
* @tc.name: relation observer 002
* @tc.desc: Test rdb observer ok in autolauchCallback scene
* @tc.type: FUNC
* @tc.require: AR000GK58N
* @tc.author: zhuwentao
*/
HWTEST_F(DistributedDBRelationalVerP2PSyncTest, observer002, TestSize.Level3)
{
    /**
     * @tc.steps: step1. open rdb store, create distribute table and insert data
     */
    g_observer->ResetToZero();
    std::map<std::string, DataValue> dataMap;
    PrepareVirtualEnvironment(dataMap, {g_deviceB});

    /**
     * @tc.steps: step2. set auto launch callBack
     */
    RelationalStoreObserverUnitTest *observer = new (std::nothrow) RelationalStoreObserverUnitTest();
    const AutoLaunchRequestCallback callback = [observer](const std::string &identifier, AutoLaunchParam &param) {
        if (g_id != identifier) {
            return false;
        }
        param.path    = g_dbDir;
        param.appId   = APP_ID;
        param.userId  = USER_ID;
        param.storeId = STORE_ID_1;
        param.option.storeObserver = observer;
#ifndef OMIT_ENCRYPT
        param.option.isEncryptedDb = true;
        param.option.cipher = CipherType::DEFAULT;
        param.option.passwd = g_correctPasswd;
        param.option.iterateTimes = DEFAULT_ITER;
#endif
        return true;
    };
    g_mgr.SetAutoLaunchRequestCallback(callback);
    /**
     * @tc.steps: step3. close store ensure communicator has closed
     */
    g_mgr.CloseStore(g_rdbDelegatePtr);
    g_rdbDelegatePtr = nullptr;
    /**
     * @tc.steps: step4. RunCommunicatorLackCallback to autolaunch store
     */
    LabelType labelType(g_id.begin(), g_id.end());
    g_communicatorAggregator->RunCommunicatorLackCallback(labelType);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    /**
     * @tc.steps: step5. Call sync expect sync successful and device A check observer
     */
    Query query = Query::Select(g_tableName);
    EXPECT_EQ(g_deviceB->GenericVirtualDevice::Sync(SYNC_MODE_PUSH_ONLY, query, true), E_OK);
    EXPECT_EQ(observer->GetCallCount(), 1u);
    EXPECT_EQ(observer->GetDataChangeDevice(), DEVICE_B);
    CheckIdentify(observer);
    std::this_thread::sleep_for(std::chrono::minutes(1));
    delete observer;
}

/**
* @tc.name: remote query 001
* @tc.desc: Test rdb remote query
* @tc.type: FUNC
* @tc.require: AR000GK58G
* @tc.author: zhangqiquan
*/
HWTEST_F(DistributedDBRelationalVerP2PSyncTest, RemoteQuery001, TestSize.Level1)
{
    std::map<std::string, DataValue> dataMap;
    PrepareEnvironment(dataMap, {g_deviceB});
    ASSERT_NE(g_rdbDelegatePtr, nullptr);
    RemoteCondition condition;
    condition.sql = "SELECT * FROM " + g_tableName;
    std::shared_ptr<ResultSet> result = nullptr;
    EXPECT_EQ(g_rdbDelegatePtr->RemoteQuery(DEVICE_B, condition, DBConstant::MIN_TIMEOUT, result), OK);

    EXPECT_NE(result, nullptr);
}

/**
* @tc.name: remote query 002
* @tc.desc: Test rdb remote query
* @tc.type: FUNC
* @tc.require: AR000GK58G
* @tc.author: zhangqiquan
*/
HWTEST_F(DistributedDBRelationalVerP2PSyncTest, RemoteQuery002, TestSize.Level1)
{
    std::map<std::string, DataValue> dataMap;
    PrepareEnvironment(dataMap, {g_deviceB});
    ASSERT_NE(g_deviceB, nullptr);
    ASSERT_NE(g_rdbDelegatePtr, nullptr);
    RemoteCondition condition;
    condition.sql = "SELECT * FROM " + g_tableName;
    std::shared_ptr<ResultSet> result = nullptr;
    EXPECT_EQ(g_deviceB->RemoteQuery(DEVICE_A, condition, DBConstant::MIN_TIMEOUT, result), OK);
    CheckSearchData(result, dataMap);
}

/**
* @tc.name: remote query 003
* @tc.desc: Test rdb remote query but query not data
* @tc.type: FUNC
* @tc.require: AR000GK58G
* @tc.author: zhangqiquan
*/
HWTEST_F(DistributedDBRelationalVerP2PSyncTest, RemoteQuery003, TestSize.Level1)
{
    std::map<std::string, DataValue> dataMap;
    PrepareEnvironment(dataMap, {g_deviceB});
    ASSERT_NE(g_deviceB, nullptr);
    ASSERT_NE(g_rdbDelegatePtr, nullptr);
    RemoteCondition condition;
    condition.sql = "SELECT * FROM " + g_tableName + " WHERE 1=0";
    std::shared_ptr<ResultSet> result = nullptr;
    EXPECT_EQ(g_deviceB->RemoteQuery(DEVICE_A, condition, DBConstant::MIN_TIMEOUT, result), OK);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->GetCount(), 0);
}

/**
* @tc.name: remote query 004
* @tc.desc: Test rdb queue size
* @tc.type: FUNC
* @tc.require: AR000GK58G
* @tc.author: zhangqiquan
*/
HWTEST_F(DistributedDBRelationalVerP2PSyncTest, RemoteQuery004, TestSize.Level3)
{
    std::map<std::string, DataValue> dataMap;
    PrepareEnvironment(dataMap, {g_deviceB});
    ASSERT_NE(g_rdbDelegatePtr, nullptr);
    RemoteCondition condition;
    condition.sql = "SELECT * FROM " + g_tableName;
    std::vector<std::string> deviceMap = {DEVICE_B, DEVICE_C};
    g_communicatorAggregator->RegOnDispatch([](const std::string &device, Message *inMsg) {
        ASSERT_NE(inMsg, nullptr);
        inMsg->SetMessageId(INVALID_MESSAGE_ID);
    });
    const size_t MAX_SIZE = 7;
    std::vector<std::thread *> threadList;
    for (size_t j = 0; j < deviceMap.size(); j++) {
        for (size_t i = 0; i < MAX_SIZE; i++) {
            std::string device = deviceMap[j];
            threadList.push_back(new std::thread([&, device]() {
                std::shared_ptr<ResultSet> result = nullptr;
                EXPECT_EQ(g_rdbDelegatePtr->RemoteQuery(device, condition,
                    DBConstant::MIN_TIMEOUT, result), TIME_OUT);
                EXPECT_EQ(result, nullptr);
            }));
        }
    }
    for (size_t i = 0; i < threadList.size(); i++) {
        threadList[i]->join();
        delete threadList[i];
        threadList[i] = nullptr;
    }
}

/**
* @tc.name: remote query 005
* @tc.desc: Test rdb remote query timeout by invalid message
* @tc.type: FUNC
* @tc.require: AR000GK58G
* @tc.author: zhangqiquan
*/
HWTEST_F(DistributedDBRelationalVerP2PSyncTest, RemoteQuery005, TestSize.Level1)
{
    std::map<std::string, DataValue> dataMap;
    PrepareEnvironment(dataMap, {g_deviceB});
    ASSERT_NE(g_deviceB, nullptr);
    ASSERT_NE(g_rdbDelegatePtr, nullptr);
    g_communicatorAggregator->RegOnDispatch([](const std::string &device, Message *inMsg) {
        ASSERT_NE(inMsg, nullptr);
        inMsg->SetMessageId(INVALID_MESSAGE_ID);
    });
    RemoteCondition condition;
    condition.sql = "SELECT * FROM " + g_tableName + " WHERE 1=0";
    std::shared_ptr<ResultSet> result = nullptr;
    EXPECT_EQ(g_rdbDelegatePtr->RemoteQuery(DEVICE_B, condition, DBConstant::MIN_TIMEOUT, result), TIME_OUT);
    ASSERT_EQ(result, nullptr);
}

/**
* @tc.name: remote query 006
* @tc.desc: Test rdb remote query commfailure by offline
* @tc.type: FUNC
* @tc.require: AR000GK58G
* @tc.author: zhangqiquan
*/
HWTEST_F(DistributedDBRelationalVerP2PSyncTest, RemoteQuery006, TestSize.Level1)
{
    std::map<std::string, DataValue> dataMap;
    PrepareEnvironment(dataMap, {g_deviceB});
    ASSERT_NE(g_deviceB, nullptr);
    ASSERT_NE(g_rdbDelegatePtr, nullptr);
    std::thread *offlineThread = nullptr;
    g_communicatorAggregator->RegOnDispatch([&offlineThread](const std::string &device, Message *inMsg) {
        ASSERT_NE(inMsg, nullptr);
        inMsg->SetMessageId(INVALID_MESSAGE_ID);
        offlineThread = new std::thread([]() {
            g_deviceB->Offline();
        });
    });
    RemoteCondition condition;
    condition.sql = "SELECT * FROM " + g_tableName + " WHERE 1=0";
    std::shared_ptr<ResultSet> result = nullptr;
    EXPECT_EQ(g_rdbDelegatePtr->RemoteQuery(DEVICE_B, condition, DBConstant::MIN_TIMEOUT, result), COMM_FAILURE);
    ASSERT_EQ(result, nullptr);
    if (offlineThread != nullptr) {
        offlineThread->join();
        delete offlineThread;
        offlineThread = nullptr;
    }
}

/**
* @tc.name: remote query 007
* @tc.desc: Test rdb remote query failed by permission check
* @tc.type: FUNC
* @tc.require: AR000GK58G
* @tc.author: zhangqiquan
*/
HWTEST_F(DistributedDBRelationalVerP2PSyncTest, RemoteQuery007, TestSize.Level1)
{
    std::map<std::string, DataValue> dataMap;
    PrepareEnvironment(dataMap, {g_deviceB});
    ASSERT_NE(g_deviceB, nullptr);
    ASSERT_NE(g_rdbDelegatePtr, nullptr);
    PermissionCheckCallbackV2 callback = [](const std::string &userId, const std::string &appId,
        const std::string &storeId, const std::string &deviceId, uint8_t flag) -> bool {
        return false;
    };
    EXPECT_EQ(RuntimeConfig::SetPermissionCheckCallback(callback), OK);
    RemoteCondition condition;
    condition.sql = "SELECT * FROM " + g_tableName + " WHERE 1=0";
    std::shared_ptr<ResultSet> result = nullptr;
    EXPECT_EQ(g_deviceB->RemoteQuery(DEVICE_A, condition, DBConstant::MIN_TIMEOUT, result),
        PERMISSION_CHECK_FORBID_SYNC);
    ASSERT_EQ(result, nullptr);
    callback = nullptr;
    EXPECT_EQ(RuntimeConfig::SetPermissionCheckCallback(callback), OK);
}

/**
* @tc.name: remote query 008
* @tc.desc: Test rdb remote query timeout but not effected by invalid message
* @tc.type: FUNC
* @tc.require: AR000GK58G
* @tc.author: zhangqiquan
*/
HWTEST_F(DistributedDBRelationalVerP2PSyncTest, RemoteQuery008, TestSize.Level1)
{
    std::map<std::string, DataValue> dataMap;
    PrepareEnvironment(dataMap, {g_deviceB});
    ASSERT_NE(g_deviceB, nullptr);
    ASSERT_NE(g_rdbDelegatePtr, nullptr);
    g_communicatorAggregator->RegOnDispatch([](const std::string &device, Message *inMsg) {
        ASSERT_NE(inMsg, nullptr);
        if (device != DEVICE_B) {
            return;
        }
        inMsg->SetMessageId(INVALID_MESSAGE_ID);
        std::thread t([]() {
            auto *msg = new (std::nothrow) Message(REMOTE_EXECUTE_MESSAGE);
            ASSERT_NE(msg, nullptr);
            auto *packet = new (std::nothrow) RemoteExecutorAckPacket();
            if (packet != nullptr) {
                packet->SetAckCode(-E_FEEDBACK_COMMUNICATOR_NOT_FOUND);
            }
            if (msg->SetExternalObject(packet) != E_OK) {
                delete packet;
                packet = nullptr;
            }
            msg->SetMessageType(TYPE_RESPONSE);
            msg->SetErrorNo(E_FEEDBACK_COMMUNICATOR_NOT_FOUND);
            msg->SetSessionId(0u);
            msg->SetSequenceId(1u);
            g_communicatorAggregator->DispatchMessage(DEVICE_B, DEVICE_A, msg, nullptr);
            LOGD("DispatchMessage Finish");
        });
        t.detach();
    });
    RemoteCondition condition;
    condition.sql = "SELECT * FROM " + g_tableName + " WHERE 1=0";
    std::shared_ptr<ResultSet> result = nullptr;
    EXPECT_EQ(g_rdbDelegatePtr->RemoteQuery(DEVICE_B, condition, DBConstant::MIN_TIMEOUT, result), TIME_OUT);
    ASSERT_EQ(result, nullptr);
}

/**
* @tc.name: remote query 009
* @tc.desc: Test rdb remote query busy before timeout
* @tc.type: FUNC
* @tc.require: AR000GK58G
* @tc.author: zhangqiquan
*/
HWTEST_F(DistributedDBRelationalVerP2PSyncTest, RemoteQuery009, TestSize.Level1)
{
    std::map<std::string, DataValue> dataMap;
    PrepareEnvironment(dataMap, {g_deviceB});
    ASSERT_NE(g_deviceB, nullptr);
    ASSERT_NE(g_rdbDelegatePtr, nullptr);
    g_communicatorAggregator->RegOnDispatch([](const std::string &device, Message *inMsg) {
        ASSERT_NE(inMsg, nullptr);
        inMsg->SetMessageId(INVALID_MESSAGE_ID);
    });
    RemoteCondition condition;
    condition.sql = "SELECT * FROM " + g_tableName + " WHERE 1=0";
    std::shared_ptr<ResultSet> result = nullptr;
    std::thread t([]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if (g_rdbDelegatePtr != nullptr) {
            LOGD("CloseStore Start");
            ASSERT_EQ(g_mgr.CloseStore(g_rdbDelegatePtr), OK);
            g_rdbDelegatePtr = nullptr;
        }
    });
    EXPECT_EQ(g_rdbDelegatePtr->RemoteQuery(DEVICE_B, condition, DBConstant::MIN_TIMEOUT, result), BUSY);
    ASSERT_EQ(result, nullptr);
    t.join();
}

/**
* @tc.name: remote query 010
* @tc.desc: Test rdb remote query with kv db
* @tc.type: FUNC
* @tc.require: AR000GK58G
* @tc.author: zhangqiquan
*/
HWTEST_F(DistributedDBRelationalVerP2PSyncTest, RemoteQuery010, TestSize.Level1)
{
    std::map<std::string, DataValue> dataMap;
    PrepareEnvironment(dataMap, {g_deviceB});
    ASSERT_NE(g_rdbDelegatePtr, nullptr);
    RemoteCondition condition;
    condition.sql = "SELECT * FROM " + g_tableName;
    std::shared_ptr<ResultSet> result = nullptr;
    EXPECT_EQ(g_rdbDelegatePtr->RemoteQuery(DEVICE_D, condition, DBConstant::MIN_TIMEOUT, result), NOT_SUPPORT);

    EXPECT_EQ(result, nullptr);
}

/**
* @tc.name: remote query 010
* @tc.desc: Test rdb remote query with
* @tc.type: FUNC
* @tc.require: AR000GK58G
* @tc.author: zhangqiquan
*/
HWTEST_F(DistributedDBRelationalVerP2PSyncTest, RemoteQuery011, TestSize.Level1)
{
    std::map<std::string, DataValue> dataMap;
    PrepareEnvironment(dataMap, {g_deviceB});
    ASSERT_NE(g_deviceB, nullptr);
    ASSERT_NE(g_rdbDelegatePtr, nullptr);
    RemoteCondition condition;
    condition.sql = "This is error sql";
    std::shared_ptr<ResultSet> result = nullptr;
    EXPECT_EQ(g_deviceB->RemoteQuery(DEVICE_A, condition, DBConstant::MIN_TIMEOUT, result), DB_ERROR);
    EXPECT_EQ(result, nullptr);
}

/**
* @tc.name: RelationalPemissionTest001
* @tc.desc: deviceB PermissionCheck not pass test, SYNC_MODE_PUSH_ONLY
* @tc.type: FUNC
* @tc.require: AR000GK58N
* @tc.author: zhangshijie
*/
HWTEST_F(DistributedDBRelationalVerP2PSyncTest, RelationalPemissionTest001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. SetPermissionCheckCallback
     * @tc.expected: step1. return OK.
     */
    auto permissionCheckCallback = [] (const std::string &userId, const std::string &appId,
        const std::string &storeId, const std::string &deviceId, uint8_t flag) -> bool {
            LOGE("u: %s, a: %s, s: %s", userId.c_str(), appId.c_str(), storeId.c_str());
            bool empty = userId.empty() || appId.empty() || storeId.empty();
            EXPECT_TRUE(empty == false);
            if (flag & (CHECK_FLAG_SEND)) {
                LOGD("in RunPermissionCheck callback, check not pass, flag:%d", flag);
                return false;
            } else {
                LOGD("in RunPermissionCheck callback, check pass, flag:%d", flag);
                return true;
            }
        };
    EXPECT_EQ(RuntimeConfig::SetPermissionCheckCallback(permissionCheckCallback), OK);

    /**
     * @tc.steps: step2. sync with deviceB
     */
    std::map<std::string, DataValue> dataMap;
    PrepareEnvironment(dataMap, { g_deviceB });
    BlockSync(SyncMode::SYNC_MODE_PUSH_ONLY, PERMISSION_CHECK_FORBID_SYNC, { DEVICE_B });

    /**
     * @tc.steps: step3. check data in deviceB
     * @tc.expected: step3. deviceB has no data
     */
    std::vector<VirtualRowData> targetData;
    g_deviceB->GetAllSyncData(g_tableName, targetData);

    ASSERT_EQ(targetData.size(), 0u);
}

/**
* @tc.name: RelationalPemissionTest002
* @tc.desc: deviceB PermissionCheck not pass test, SYNC_MODE_PULL_ONLY
* @tc.type: FUNC
* @tc.require: AR000GK58N
* @tc.author: zhangshijie
*/
HWTEST_F(DistributedDBRelationalVerP2PSyncTest, RelationalPemissionTest002, TestSize.Level0)
{
    /**
     * @tc.steps: step1. SetPermissionCheckCallback
     * @tc.expected: step1. return OK.
     */
    auto permissionCheckCallback = [] (const std::string &userId, const std::string &appId,
        const std::string &storeId, const std::string &deviceId, uint8_t flag) -> bool {
            LOGE("u: %s, a: %s, s: %s", userId.c_str(), appId.c_str(), storeId.c_str());
            bool empty = userId.empty() || appId.empty() || storeId.empty();
            EXPECT_TRUE(empty == false);
            if (flag & (CHECK_FLAG_RECEIVE)) {
                LOGD("in RunPermissionCheck callback, check not pass, flag:%d", flag);
                return false;
            } else {
                LOGD("in RunPermissionCheck callback, check pass, flag:%d", flag);
                return true;
            }
        };
    EXPECT_EQ(RuntimeConfig::SetPermissionCheckCallback(permissionCheckCallback), OK);

    /**
     * @tc.steps: step2. sync with deviceB
     */
    std::map<std::string, DataValue> dataMap;
    PrepareEnvironment(dataMap, { g_deviceB });
    BlockSync(SyncMode::SYNC_MODE_PULL_ONLY, PERMISSION_CHECK_FORBID_SYNC, { DEVICE_B });

    /**
     * @tc.steps: step3. check data in deviceB
     * @tc.expected: step3. deviceB has no data
     */
    std::vector<VirtualRowData> targetData;
    g_deviceB->GetAllSyncData(g_tableName, targetData);

    ASSERT_EQ(targetData.size(), 0u);
}

/**
* @tc.name: RelationalPemissionTest003
* @tc.desc: deviceB PermissionCheck not pass test, flag CHECK_FLAG_SPONSOR
* @tc.type: FUNC
* @tc.require: AR000GK58N
* @tc.author: zhangshijie
*/
HWTEST_F(DistributedDBRelationalVerP2PSyncTest, RelationalPemissionTest003, TestSize.Level0)
{
    /**
     * @tc.steps: step1. SetPermissionCheckCallback
     * @tc.expected: step1. return OK.
     */
    auto permissionCheckCallback = [] (const std::string &userId, const std::string &appId,
        const std::string &storeId, const std::string &deviceId, uint8_t flag) -> bool {
            LOGE("u: %s, a: %s, s: %s", userId.c_str(), appId.c_str(), storeId.c_str());
            bool empty = userId.empty() || appId.empty() || storeId.empty();
            EXPECT_TRUE(empty == false);
            if (flag &  CHECK_FLAG_SPONSOR) {
                LOGD("in RunPermissionCheck callback, check not pass, flag:%d", flag);
                return false;
            } else {
                LOGD("in RunPermissionCheck callback, check pass, flag:%d", flag);
                return true;
            }
        };
    EXPECT_EQ(RuntimeConfig::SetPermissionCheckCallback(permissionCheckCallback), OK);

    /**
     * @tc.steps: step2. sync with deviceB
     */
    std::map<std::string, DataValue> dataMap;
    PrepareEnvironment(dataMap, { g_deviceB });
    BlockSync(SyncMode::SYNC_MODE_PULL_ONLY, PERMISSION_CHECK_FORBID_SYNC, { DEVICE_B });

    /**
     * @tc.steps: step3. check data in deviceB
     * @tc.expected: step3. deviceB has no data
     */
    std::vector<VirtualRowData> targetData;
    g_deviceB->GetAllSyncData(g_tableName, targetData);

    ASSERT_EQ(targetData.size(), 0u);
}

/**
* @tc.name: RelationalPemissionTest004
* @tc.desc: deviceB PermissionCheck pass test. deviceC not pass, SYNC_MODE_PUSH_ONLY
* @tc.type: FUNC
* @tc.require: AR000GK58N
* @tc.author: zhangshijie
*/
HWTEST_F(DistributedDBRelationalVerP2PSyncTest, RelationalPemissionTest004, TestSize.Level0)
{
    /**
     * @tc.steps: step1. SetPermissionCheckCallback
     * @tc.expected: step1. return OK.
     */
    auto permissionCheckCallback = [] (const std::string &userId, const std::string &appId,
        const std::string &storeId, const std::string &deviceId, uint8_t flag) -> bool {
            LOGE("u: %s, a: %s, s: %s", userId.c_str(), appId.c_str(), storeId.c_str());
            if (deviceId == g_deviceC->GetDeviceId()) {
                LOGE("in RunPermissionCheck callback, check pass, device:%s", deviceId.c_str());
                return false;
            } else {
                LOGE("in RunPermissionCheck callback, check not pass, device:%s", deviceId.c_str());
                return true;
            }
        };
    EXPECT_EQ(RuntimeConfig::SetPermissionCheckCallback(permissionCheckCallback), OK);

    std::map<std::string, DataValue> dataMap;
    PrepareEnvironment(dataMap, { g_deviceB, g_deviceC });

    /**
     * @tc.steps: step2. sync with deviceB
     */
    BlockSync(SyncMode::SYNC_MODE_PUSH_ONLY, OK, { DEVICE_B });

    /**
     * @tc.steps: step3. check data in deviceB
     * @tc.expected: step3. deviceB has data
     */
    std::vector<VirtualRowData> targetData;
    g_deviceB->GetAllSyncData(g_tableName, targetData);
    ASSERT_EQ(targetData.size(), 1u);

    /**
     * @tc.steps: step4. sync with deviceC
     */
    BlockSync(SyncMode::SYNC_MODE_PUSH_ONLY, PERMISSION_CHECK_FORBID_SYNC, { DEVICE_C });

    /**
     * @tc.steps: step5. check data in deviceC
     * @tc.expected: step5. deviceC has no data
     */
    targetData.clear();
    g_deviceC->GetAllSyncData(g_tableName, targetData);
    ASSERT_EQ(targetData.size(), 0u);
}

/**
* @tc.name: SecurityOptionCheck001
* @tc.desc: Test sync failed when getSecurityOption return error
* @tc.type: FUNC
* @tc.require: AR000GK58N
* @tc.author: zhangqiquan
*/
HWTEST_F(DistributedDBRelationalVerP2PSyncTest, SecurityOptionCheck001, TestSize.Level1)
{
    std::vector<std::string> devices;
    devices.push_back(g_deviceB->GetDeviceId());

    /**
     * @tc.steps: step1. make getSecurityOption return -1
     */
    std::shared_ptr<ProcessSystemApiAdapterImpl> adapter = std::make_shared<ProcessSystemApiAdapterImpl>();
    adapter->ForkGetSecurityOption(
        [](const std::string &filePath, SecurityOption &option) {
        (void)filePath;
        (void)option;
        return DB_ERROR;
    });
    RuntimeConfig::SetProcessSystemAPIAdapter(adapter);

    std::map<std::string, DataValue> dataMap;
    PrepareEnvironment(dataMap, { g_deviceB });

    /**
     * @tc.steps: step2. sync with deviceB
     */
    BlockSync(SyncMode::SYNC_MODE_PUSH_ONLY, SECURITY_OPTION_CHECK_ERROR, { DEVICE_B });
    RuntimeConfig::SetProcessSystemAPIAdapter(nullptr);
}

/**
* @tc.name: SecurityOptionCheck002
* @tc.desc: Test remote query failed when getSecurityOption return error
* @tc.type: FUNC
* @tc.require: AR000GK58G
* @tc.author: zhangqiquan
*/
HWTEST_F(DistributedDBRelationalVerP2PSyncTest, SecurityOptionCheck002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. make getSecurityOption return -1
     */
    std::shared_ptr<ProcessSystemApiAdapterImpl> adapter = std::make_shared<ProcessSystemApiAdapterImpl>();
    adapter->ForkGetSecurityOption(
        [](const std::string &filePath, SecurityOption &option) {
        (void)filePath;
        (void)option;
        return DB_ERROR;
    });
    RuntimeConfig::SetProcessSystemAPIAdapter(adapter);

    /**
     * @tc.steps: step2. remote query with deviceB
     */
    std::map<std::string, DataValue> dataMap;
    PrepareEnvironment(dataMap, {g_deviceB});
    ASSERT_NE(g_deviceB, nullptr);
    ASSERT_NE(g_rdbDelegatePtr, nullptr);
    RemoteCondition condition;
    condition.sql = "SELECT * FROM " + g_tableName + " WHERE 1=0";
    std::shared_ptr<ResultSet> result = nullptr;
    EXPECT_EQ(g_deviceB->RemoteQuery(DEVICE_A, condition, DBConstant::MIN_TIMEOUT, result),
        SECURITY_OPTION_CHECK_ERROR);
    EXPECT_EQ(result, nullptr);
    RuntimeConfig::SetProcessSystemAPIAdapter(nullptr);
}

/**
* @tc.name: OrderbyWriteTimeSync001
* @tc.desc: sync query with order by writeTime
* @tc.type: FUNC
* @tc.require: AR000H5VLO
* @tc.author: zhuwentao
*/
HWTEST_F(DistributedDBRelationalVerP2PSyncTest, OrderbyWriteTimeSync001, TestSize.Level0)
{
    std::map<std::string, DataValue> dataMap;
    PrepareEnvironment(dataMap, {g_deviceB});
    Query query = Query::Select(g_tableName).OrderByWriteTime(true);;
    EXPECT_EQ(g_rdbDelegatePtr->Sync({DEVICE_B}, DistributedDB::SYNC_MODE_PUSH_ONLY, query, nullptr, false),
        NOT_SUPPORT);
}

/**
* @tc.name: RDBSecurityOptionCheck001
* @tc.desc: Test sync with security option.
* @tc.type: FUNC
* @tc.require: AR000GK58N
* @tc.author: zhangqiquan
*/
HWTEST_F(DistributedDBRelationalVerP2PSyncTest, RDBSecurityOptionCheck001, TestSize.Level3)
{
    TestWithSecurityCheck(false);
}

/**
* @tc.name: RDBSecurityOptionCheck002
* @tc.desc: Test remote query with security option.
* @tc.type: FUNC
* @tc.require: AR000GK58N
* @tc.author: zhangqiquan
*/
HWTEST_F(DistributedDBRelationalVerP2PSyncTest, RDBSecurityOptionCheck002, TestSize.Level1)
{
    TestWithSecurityCheck(true);
}

/**
* @tc.name: EncryptedAlgoUpgrade001
  * @tc.desc: Test upgrade encrypted db can sync normally
  * @tc.type: FUNC
  * @tc.require: AR000HI2JS
  * @tc.author: zhuwentao
*/
HWTEST_F(DistributedDBRelationalVerP2PSyncTest, EncryptedAlgoUpgrade001, TestSize.Level0)
{
    if (g_rdbDelegatePtr != nullptr) {
        LOGD("CloseStore Start");
        ASSERT_EQ(g_mgr.CloseStore(g_rdbDelegatePtr), OK);
        g_rdbDelegatePtr = nullptr;
    }
    EXPECT_EQ(OS::RemoveFile(g_dbDir), E_OK);
    /**
     * @tc.steps: step1. open old db use sha1 algo and insert some data
     * @tc.expected: step1. interface return ok
     */
    sqlite3 *db = nullptr;
    EXPECT_EQ(GetDB(db, false), SQLITE_OK);
    const std::string user_version_sql = "PRAGMA user_version=101;";
    EXPECT_EQ(SQLiteUtils::ExecuteRawSQL(db, user_version_sql), E_OK);
    sqlite3_close(db);

    /**
     * @tc.steps: step2. open db by OpenStore
     * @tc.expected: step2. interface return ok
     */
    OpenStore();
    /**
     * @tc.steps: step3. sync with push
     * @tc.expected: step3. interface return ok
     */
    std::map<std::string, DataValue> dataMap;
    PrepareEnvironment(dataMap, {g_deviceB});
    BlockSync(SyncMode::SYNC_MODE_PUSH_ONLY, OK, {DEVICE_B});
    CheckVirtualData(dataMap);
    dataMap.clear();

    /**
     * @tc.steps: step4. sync with pull
     * @tc.expected: step4. interface return ok
     */

    Query query = Query::Select(g_tableName);
    g_deviceB->GenericVirtualDevice::Sync(DistributedDB::SYNC_MODE_PULL_ONLY, query, true);
    CheckVirtualData(dataMap);
}
#endif