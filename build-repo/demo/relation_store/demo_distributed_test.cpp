#define LOG_TAG "[RDB_DEMO]"

// 使用逻辑参考：foundation/distributeddatamgr/relational_store/test/native/rdb/distributedtest/rdb_store_impl_test/distributed_test.cpp
// 参考：rdb_distributed_test.cpp
// 测试关系型数据库的跨设备同步功能
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>
#include <iomanip>

#include "rdb_errno.h"
#include "rdb_helper.h"
#include "rdb_open_callback.h"
// #include "rdb_store_impl.h"
#include "rdb_types.h"
#include "rdb_predicates.h"
#include "distributed_kv_data_manager.h"
#include "types.h"
#include "hilog/log.h"

using namespace OHOS::NativeRdb;
using namespace OHOS::DistributedRdb;
using DeviceInfo = OHOS::DistributedKv::DeviceInfo;
using DistributedKvDataManager = OHOS::DistributedKv::DistributedKvDataManager;
using DeviceFilterStrategy = OHOS::DistributedKv::DeviceFilterStrategy;
using SyncResult = std::map<std::string, int>;

// const int MSG_LENGTH = 100;

// hilog日志打印使用
static inline OHOS::HiviewDFX::HiLogLabel LogLabel() {
    return {LOG_CORE, 0xD009999, "RDB_DEMO"};
}

#define ZLOGD(fmt, ...) \
        OHOS::HiviewDFX::HiLog::Debug(LogLabel(), LOG_TAG "::%{public}s: " fmt, __FUNCTION__, ##__VA_ARGS__)

#define ZLOGI(fmt, ...) \
        OHOS::HiviewDFX::HiLog::Info(LogLabel(), LOG_TAG "::%{public}s: " fmt, __FUNCTION__, ##__VA_ARGS__)

#define ZLOGW(fmt, ...) \
        OHOS::HiviewDFX::HiLog::Warn(LogLabel(), LOG_TAG "::%{public}s: " fmt, __FUNCTION__, ##__VA_ARGS__)

#define ZLOGE(fmt, ...) \
        OHOS::HiviewDFX::HiLog::Error(LogLabel(), LOG_TAG "::%{public}s: " fmt, __FUNCTION__, ##__VA_ARGS__)

#define LOG_RESULT(ret) \
    do { \
        if (ret == E_OK) { \
            ZLOGI("Succeed to %s", __func__); \
        } else { \
            ZLOGE("Failed to %s, ret = %d", __func__, ret); \
        } \
    } while (0)

// 通过异步回调的方式创建数据库表
class RDBTestOpenCallback : public RdbOpenCallback {
   public:
    int OnCreate(RdbStore& rdbStore) override {
        ZLOGI("Create table.");
        return rdbStore.ExecuteSql(CREATE_TABLE_TEST);
    }

    int OnUpgrade(RdbStore& rdbStore, int oldVersion, int newVersion) override {
        ZLOGI("rdb is upgraded, from %d to %d.", oldVersion, newVersion);
        return E_OK;
    }

    static const std::string CREATE_TABLE_TEST;
};

const std::string RDBTestOpenCallback::CREATE_TABLE_TEST = std::string(
    "CREATE TABLE IF NOT EXISTS test1 (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL)");


class RDBTestStoreObserver : public RdbStoreObserver {
public:
    void OnChange(const std::vector<std::string> &devices) override {
        ZLOGI("Rdb is changed.");
    }
};


class RDBTest {
public:
    static const std::string DATABASE_NAME;
    std::vector <DeviceInfo> deviceInfos_;
    DistributedKvDataManager manager_;

    RDBTest() {
        int errCode = E_OK;
        RdbStoreConfig config(RDBTest::DATABASE_NAME);
        config.SetBundleName("com.example.distributed.rdb");
        config.SetName("distributed_rdb.db");
        RDBTestOpenCallback helper;
        rdbStore_ = RdbHelper::GetRdbStore(config, 1, helper, errCode);
        if (rdbStore_ == nullptr) {
            ZLOGE("Failed to open database file: %s, err: %d", RDBTest::DATABASE_NAME.c_str(),
                  errCode);
        }
        // 每次程序拉起先将test1表删除
        rdbStore_->ExecuteSql("DELETE FROM test1");

        // 获取设备列表
        manager_.GetDeviceList(deviceInfos_, DeviceFilterStrategy::NO_FILTER);
        
        // PrintDevicesInfo();

        // 将表设置为分布式表，可跨设备同步
        errCode = rdbStore_->SetDistributedTables(std::vector<std::string>{"test1"});
        if (errCode != E_OK) {
            ZLOGE("Failed to set distributed table, err: %d.", errCode);
        } else {
            ZLOGI("Succeed to set distributed table.");
        }
    }

    ~RDBTest() {
        RdbHelper::DeleteRdbStore(RDBTest::DATABASE_NAME);
        rdbStore_ = nullptr;
        ZLOGI("Deleteed rdb store.");
    }

    int Sync(uint8_t syncMode) {
        manager_.GetDeviceList(deviceInfos_, DeviceFilterStrategy::NO_FILTER);
        if (deviceInfos_.empty()) {
            ZLOGE("no remote device.");
            return -1;
        }

        if (syncMode != 0 && syncMode != 1) {
            ZLOGE("invalid sync mode. only support 0 or 1.");
            return -1;
        }
        int errCode = E_OK;
        // std::string remoteTable = rdbStore_->ObtainDistributedTableName(deviceInfos_[0].deviceId, "test1", errCode);
        
        SyncOption syncOption;
        syncOption.mode = (OHOS::DistributedRdb::SyncMode)(syncMode);
        syncOption.isBlock = true;
        OHOS::NativeRdb::RdbPredicates rdbPredicates("test1");

        errCode = rdbStore_->Sync(syncOption, rdbPredicates, [](const SyncResult &res) {
            ZLOGI("Sync callback result.");
        });

        if (errCode != EOK) {
            ZLOGE("Failed to Sync, err: %d.", errCode);
        }
        ZLOGI("Succeed to Sync.");
        return errCode;
    }

    void RemoteQuery(const std::string& tableName) {
        manager_.GetDeviceList(deviceInfos_, DeviceFilterStrategy::NO_FILTER);
        int errCode = E_ERROR;
        // std::string remoteTable = rdbStore_->ObtainDistributedTableName(deviceInfos_[0].deviceId, tableName, errCode);
        // ZLOGI("remote[%s] table name: %s", deviceInfos_[0].deviceId.c_str(), remoteTable.c_str());

        // PrintTable(remoteTable);

        AbsRdbPredicates predicate("test1");
        // predicate.EqualTo("name", "zhangsan1");
        std::vector<std::string> columns;
        std::shared_ptr<ResultSet> resultSet = rdbStore_->RemoteQuery(deviceInfos_[0].deviceId, predicate, columns, errCode);
        if (resultSet != nullptr) {
            int count = 0;
            resultSet->GetRowCount(count);
            ZLOGI("remote query get %d rows.", count);
        }
    }

    bool CreateTable(const std::string& table_name, const std::vector<std::string>& fields) {
        std::string fields_str = "";
        for (auto& field : fields) {
            fields_str += field + ", ";
        }
        fields_str = fields_str.substr(0, fields_str.length() - 2);
        std::string sql = "CREATE TABLE IF NOT EXISTS " + table_name + " (" + fields_str + ")";
        ZLOGI("sql is %s", sql.c_str());
        int ret = rdbStore_->ExecuteSql(sql);
        return (ret == E_OK);
    }

    bool DeleteTable(const std::string& table_name) {
        std::string sql = "DELETE FROM " + table_name;
        int ret = rdbStore_->ExecuteSql(sql);
        return (ret == E_OK);
    }

    bool Insert(const std::string& table_name,
                const std::vector<std::pair<std::string, std::string>>& values) {
        ValuesBucket valuesBucket;
        for (auto& value : values) {
            ZLOGI("value: %s->%s", value.first.c_str(), value.second.c_str());
            if (value.first == "id") {
                valuesBucket.PutInt(value.first, std::stoi(value.second));
            } else {
                valuesBucket.PutString(value.first, value.second);
            }
        }
        int64_t id = 0;
        int ret = rdbStore_->Insert(id, table_name, valuesBucket);
        LOG_RESULT(ret);
        return (ret == EOK);
    }

    // 暂时写死插入内容
    bool BatchInsert(const std::string& table_name) {
        ValuesBucket values;
        values.PutString("name", "zhangsan");
        std::vector<ValuesBucket> valuesBuckets;
        for (int i = 0; i < 10; i++) {
            valuesBuckets.push_back(values);
        }

        int64_t insertNum = 0;
        int ret = rdbStore_->BatchInsert(insertNum, table_name, valuesBuckets);
        LOG_RESULT(ret);
        return (ret == E_OK);
    }

    bool Replace(const std::string& table_name) {
        int64_t id;
        ValuesBucket values;
        values.PutInt("id", 1);
        values.PutString("name", "wangwu");
        int ret = rdbStore_->Replace(id, table_name, values);
        LOG_RESULT(ret);
        return (ret == EOK);
    }

    std::unique_ptr<ResultSet> SelectAll(const std::string& table_name) {
        std::unique_ptr<ResultSet> resultSet = rdbStore_->QuerySql("SELECT * FROM " + table_name);
        return resultSet;
    }

    std::unique_ptr<ResultSet> SelectWhere(const std::string& table_name,
                                            const std::string& where_clause,
                                            const std::vector<std::string>& args) {
        std::unique_ptr<ResultSet> resultSet =
            rdbStore_->QuerySql("SELECT * FROM " + table_name + " WHERE " + where_clause, args);
        return resultSet;
    }

    bool Update(const std::string& table_name,
                const std::vector<std::pair<std::string, std::string>>& values,
                const std::string& where_clause, const std::vector<std::string>& args) {
        ValuesBucket valuesBucket;
        for (auto& value : values) {
            if (value.first == "id") {
                valuesBucket.PutInt(value.first, std::stoi(value.second));
            } else {
                valuesBucket.PutString(value.first, value.second);
            }
        }
        int changeRows = 0;
        int ret = rdbStore_->Update(changeRows, table_name, valuesBucket, where_clause, args);
        LOG_RESULT(ret);
        ZLOGI("change %d rows.", changeRows);
        return (ret == E_OK);
    }

    bool DeleteWhere(const std::string& table_name, const std::string& where_clause,
                      const std::vector<std::string>& args) {
        int deletedRows = 0;
        int ret = rdbStore_->Delete(deletedRows, table_name, where_clause, args);
        LOG_RESULT(ret);
        return (ret == E_OK);
    }

    bool Backup(const std::string path) {
        int ret = rdbStore_->Backup(path, {});
        LOG_RESULT(ret);
        return (ret == E_OK);
    }

    int GetVersion() {
        int version = 0;
        int ret = rdbStore_->GetVersion(version);
        LOG_RESULT(ret);
        return version;
    }

    void BeginTransaction() {
        int ret = rdbStore_->BeginTransaction();
        LOG_RESULT(ret);
    }

    void RoolBack() {
        int ret = rdbStore_->RollBack();
        LOG_RESULT(ret);
    }

    void Commit() {
        int ret = rdbStore_->Commit();
        LOG_RESULT(ret);
    }

    void MarkAsCommit() {
        int ret = rdbStore_->MarkAsCommit();
        LOG_RESULT(ret);
    }

    void EndTransaction() {
        int ret = rdbStore_->EndTransaction();
        LOG_RESULT(ret);
    }

    bool IsInTransaction() {
        return rdbStore_->IsInTransaction();
    }

    bool IsHoldingConnection() {
        return rdbStore_->IsHoldingConnection();
    }

    bool IsOpen() {
        return rdbStore_->IsOpen();
    }

    bool IsReadOnly() {
        return rdbStore_->IsReadOnly();
    }

    bool IsMemoryRdb() {
        return rdbStore_->IsMemoryRdb();
    }

    void SetVersion(int version) {
        int ret = rdbStore_->SetVersion(version);
        LOG_RESULT(ret);
    }

    std::string GetPath() {
        return rdbStore_->GetPath();
    }

    void PrintTable(ResultSet *resultSet, const std::string &table_name) {
        int count = 0;
        resultSet->GetRowCount(count);
        if (count == 0) {
            ZLOGI("Table %s is empty.", table_name.c_str());
            return;
        }
        ZLOGI("Table %s has %d rows.", table_name.c_str(), count);

        // get column names
        std::vector<std::string> columnNames;
        resultSet->GetAllColumnNames(columnNames);

        // calculate column widths
        std::vector<size_t> columnWidths(columnNames.size());
        for (size_t i = 0; i < columnNames.size(); i++) {
            columnWidths[i] = columnNames[i].length();
            // ZLOGI("column %lu: %s.", i, columnNames[i].c_str());
        }
        resultSet->GoToFirstRow();
        bool isEnd = false;
        while (!resultSet->IsEnded(isEnd) && !isEnd) {
            for (size_t i = 0; i < columnNames.size(); i++) {
                std::string value;
                resultSet->GetString(i, value);
                // ZLOGI("column %lu: %s.", i, value.c_str());
                if (value.length() > columnWidths[i]) {
                    columnWidths[i] = value.length();
                }
            }
            resultSet->GoToNextRow();
        }
        // print table header
        std::ostringstream oss;
        oss << std::endl;
        for (size_t i = 0; i < columnNames.size(); i++) {
            oss << std::setw(columnWidths[i]) << std::left << columnNames[i] << " | ";
        }
        oss << std::endl;
        for (size_t i = 0; i < columnNames.size(); i++) {
            oss << std::setfill('-') << std::setw(columnWidths[i]) << ""
                << "-|";
        }
        oss << std::endl;

        // print table rows
        resultSet->GoToFirstRow();
        isEnd = false;
        while (!resultSet->IsEnded(isEnd) && !isEnd) {
            for (size_t i = 0; i < columnNames.size(); i++) {
                std::string value;
                resultSet->GetString(i, value);
                oss << std::setfill(' ') << std::setw(columnWidths[i]) << value << " | ";
            }
            oss << std::endl;
            resultSet->GoToNextRow();
        }
        ZLOGI("Table %s:\n%s", table_name.c_str(), oss.str().c_str());
    }

    void PrintTable(const std::string& table_name) {
        std::unique_ptr<ResultSet> resultSet = SelectAll(table_name);
        if (resultSet == nullptr) {
            ZLOGE("Failed to select data from table %s.", table_name.c_str());
            return;
        }
        PrintTable(resultSet.get(), table_name);
        resultSet->Close();
    }

    void PrintRemoteTable() {
        std::string table = GetRemoteTableName();
        if (!table.empty()) {
            PrintTable(table);
        }
    }

    void Subscribe() {
        SubscribeOption subscribeOption;
        subscribeOption.mode = REMOTE;
        RDBTestStoreObserver observer;
        int ret = rdbStore_->Subscribe(subscribeOption, &observer);
        if (ret != E_OK) {
            ZLOGE("Failed to Subscribe, err: %d.", ret);
        } else {
            ZLOGI("Succeed to Subscribe.");
        }
    }

    void UnSubscribe() {
        SubscribeOption subscribeOption;
        subscribeOption.mode = REMOTE;
        RDBTestStoreObserver observer; // TODO: 是否得和Subscribe是一致？
        int ret = rdbStore_->Subscribe(subscribeOption, &observer);
        if (ret != E_OK) {
            ZLOGE("Failed to UnSubscribe, err: %d.", ret);
        } else {
            ZLOGI("Succeed to UnSubscribe.");
        }
    }

    std::string GetRemoteTableName() {
        int errCode = E_ERROR;
        std::string remoteTable = rdbStore_->ObtainDistributedTableName(deviceInfos_[0].deviceId, "test1", errCode);
        if (errCode != E_OK) {
            ZLOGE("Failed to get remote table name.");
            return "";
        }
        ZLOGI("remote[%s] table name: %s", deviceInfos_[0].deviceId.c_str(), remoteTable.c_str());
        return remoteTable;
    }
private:
    std::shared_ptr<RdbStore> rdbStore_;

    void PrintDevicesInfo() {
        for (size_t i = 0; i < deviceInfos_.size(); ++i) {
            DeviceInfo info = deviceInfos_[i];
            ZLOGI("device[%lu]: id[%s], name[%s], type[%s]",
                i, info.deviceId.c_str(), info.deviceName.c_str(), info.deviceType.c_str());
        }
    }
};

// const std::string RDBTest::DATABASE_NAME = "/data/test/distributed_rdb.db";
const std::string RDBTest::DATABASE_NAME = "/data/app/el0/0/database/com.example.distributed.rdb/rdb/distributed_rdb.db";

void TempInsert(RDBTest *db, std::string table) {
    static int innerCount = 1;
    std::pair<std::string, std::string> item;
    std::vector<std::pair<std::string, std::string>> values;

    item.first = std::string("id");
    item.second = std::to_string(innerCount);
    values.emplace_back(item);

    item.first = std::string("name");
    item.second = std::string("zhangsan") + std::to_string(innerCount);
    values.emplace_back(item);

    db->Insert(table, values);
    innerCount++;
}

void autoTest() {
    RDBTest db;

    db.PrintTable("test1");

    db.BeginTransaction();

    TempInsert(&db, "test1");
    db.PrintTable("test1");

    db.BatchInsert("test1");
    db.PrintTable("test1");

    db.Replace("test1");
    db.PrintTable("test1");

    // std::vector<std::pair<std::string, std::string>> values{{"id", "666"}, {"name", "zhaoliu"}};
    db.Update("test1", {{"id", "666"}, {"name", "zhaoliu"}}, "id = ?", {"1"});
    db.PrintTable("test1");

    db.DeleteWhere("test1", "id = ?", {"666"});
    db.PrintTable("test1");

    // db.MarkAsCommit();
    // db.PrintTable("test1");

    // db.Commit();
    // db.PrintTable("test1");

    db.EndTransaction();
    db.PrintTable("test1");

    // db.RoolBack();
    // db.PrintTable("test1");

    // db.Backup("/tmp/");
    // db.PrintTable("test1");

    ZLOGI("Get Version:%d.", db.GetVersion());
    ZLOGI("Get Path:%s.", db.GetPath().c_str());
    ZLOGI("Is open:%d.", db.IsOpen());
    ZLOGI("Is in transaction:%d.", db.IsInTransaction());
    ZLOGI("Is read only:%d.", db.IsReadOnly());
    ZLOGI("Is memory db:%d.", db.IsMemoryRdb());
    // ZLOGI("");
}

void Run() {
    RDBTest db;

    std::string line;

    while (std::getline(std::cin, line)) {
        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;
        if (cmd == "print") {
            db.PrintTable("test1");
        } else if (cmd == "print_remote") {
            db.PrintRemoteTable();
        } else if (cmd == "insert") {
            TempInsert(&db, "test1");
        } else if (cmd == "insert_remote") {
            std::string remote_table = db.GetRemoteTableName();
            if (!remote_table.empty()) {
                TempInsert(&db, remote_table);
            }
        } else if (cmd == "create_table") {
            // db.CreateTable()
            continue;
        } else if (cmd == "sync") {
            int param;
            iss >> param;
            db.Sync(param);
        } else if (cmd == "remote_query") {
            db.RemoteQuery("test1");
        } else if (cmd == "subscribe") {
            db.Subscribe();
        } else if (cmd == "unsubscribe") {
            db.UnSubscribe();
        } else if (cmd == "exit") {
            break;
        } else {
            ZLOGE("Unknown command: %s", cmd.c_str());
        }
    }
}


int main() {
    // Run();
    autoTest();
    // RDBTest db;

    // create table
    // std::vector<std::string> fields = {"id INTEGER PRIMARY KEY AUTOINCREMENT", "name TEXT NOT NULL"};
    // bool result = db.CreateTable("test1", fields);
    // if (!result) {
    //     ZLOGE("Failed to create table.");
    //     return -1;
    // }

    // insert data
    // std::vector<std::pair<std::string, std::string>> values = {{"id", "1"}, {"name", "zhangsan"}};
    // bool result = db.Insert("test1", values);
    // values = {{"id", "2"}, {"name", "lisi"}};
    // result = db.Insert("test1", values);
    // values = {{"id", "3"}, {"name", "wangwu"}};
    // result = db.Insert("test1", values);
    // values = {{"id", "4"}, {"name", "zhaoliu"}};
    // result = db.Insert("test1", values);
    // if (!result) {
    //     ZLOGE("Failed to insert data.");
    // }

    // db.PrintTable("test1");

    // db.Sync();
    // // db.PrintTable("test1");

    // db.RemoteQuery("test1");

    // // update data
    // std::vector<std::pair<std::string, std::string>> updateValues = {{"name", "lisi"}};
    // result = db.Update("test1", updateValues, "id = ?", {"1"});
    // if (result) {
    //     ZLOGI("Data updated successfully.");
    // } else {
    //     ZLOGE("Failed to update data.");
    // }
    // db.PrintTable("test1");

    // // delete data
    // result = db.DeleteWhere("test1", "id = ?", {"1"});
    // if (result) {
    //     ZLOGI("Data deleted successfully.");
    // } else {
    //     ZLOGE("Failed to delete data.");
    // }
    // db.PrintTable("test1");

    // result = db.DeleteTable("test1");
    // if (!result) {
    //     ZLOGE("Failed to delete test1 table");
    // }

    // getchar();

    return 0;
}