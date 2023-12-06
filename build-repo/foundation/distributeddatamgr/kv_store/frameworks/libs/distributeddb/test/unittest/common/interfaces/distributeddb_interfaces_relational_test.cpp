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

#include <gtest/gtest.h>
#include <queue>
#include <random>

#include "db_common.h"
#include "distributeddb_data_generate_unit_test.h"
#include "distributeddb_tools_unit_test.h"
#include "log_print.h"
#include "platform_specific.h"
#include "relational_store_manager.h"
#include "relational_store_sqlite_ext.h"
#include "relational_virtual_device.h"
#include "runtime_config.h"
#include "virtual_relational_ver_sync_db_interface.h"

using namespace testing::ext;
using namespace DistributedDB;
using namespace DistributedDBUnitTest;
using namespace std;

namespace {
    constexpr const char *DB_SUFFIX = ".db";
    constexpr const char *STORE_ID = "Relational_Store_ID";
    std::string g_testDir;
    std::string g_dbDir;
    DistributedDB::RelationalStoreManager g_mgr(APP_ID, USER_ID);

    const std::string DEVICE_A = "real_device";
    const std::string DEVICE_B = "deviceB";
    VirtualCommunicatorAggregator* g_communicatorAggregator = nullptr;
    RelationalVirtualDevice *g_deviceB = nullptr;

    const std::string NORMAL_CREATE_TABLE_SQL = "CREATE TABLE IF NOT EXISTS sync_data(" \
        "key         BLOB NOT NULL UNIQUE," \
        "value       BLOB," \
        "timestamp   INT  NOT NULL," \
        "flag        INT  NOT NULL," \
        "device      BLOB," \
        "ori_device  BLOB," \
        "hash_key    BLOB PRIMARY KEY NOT NULL," \
        "w_timestamp INT," \
        "UNIQUE(device, ori_device));" \
        "CREATE INDEX key_index ON sync_data (key, flag);";

    const std::string SIMPLE_CREATE_TABLE_SQL = "CREATE TABLE IF NOT EXISTS t1(a INT, b TEXT)";

    const std::string CREATE_TABLE_SQL_NO_PRIMARY_KEY = "CREATE TABLE IF NOT EXISTS sync_data(" \
        "key         BLOB NOT NULL UNIQUE," \
        "value       BLOB," \
        "timestamp   INT  NOT NULL," \
        "flag        INT  NOT NULL," \
        "device      BLOB," \
        "ori_device  BLOB," \
        "hash_key    BLOB NOT NULL," \
        "w_timestamp INT," \
        "UNIQUE(device, ori_device));" \
        "CREATE INDEX key_index ON sync_data (key, flag);";

    const std::string UNSUPPORTED_FIELD_TABLE_SQL = "CREATE TABLE IF NOT EXISTS test('$.ID' INT, val BLOB);";

    const std::string COMPOSITE_PRIMARY_KEY_TABLE_SQL = R"(CREATE TABLE workers (
            worker_id INTEGER,
            last_name VARCHAR NOT NULL,
            first_name VARCHAR,
            join_date DATE,
            PRIMARY KEY (last_name, first_name)
        );)";

    const std::string INSERT_SYNC_DATA_SQL = "INSERT OR REPLACE INTO sync_data (key, timestamp, flag, hash_key) "
        "VALUES('KEY', 123456789, 1, 'HASH_KEY');";

    const std::string INVALID_TABLE_FIELD_SQL = "create table if not exists t1 ('1 = 1; --' int primary key, b blob)";
}

class DistributedDBInterfacesRelationalTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBInterfacesRelationalTest::SetUpTestCase(void)
{
    DistributedDBToolsUnitTest::TestDirInit(g_testDir);
    LOGD("Test dir is %s", g_testDir.c_str());
    g_dbDir = g_testDir + "/";
    DistributedDBToolsUnitTest::RemoveTestDbFiles(g_testDir);

    g_communicatorAggregator = new (std::nothrow) VirtualCommunicatorAggregator();
    ASSERT_TRUE(g_communicatorAggregator != nullptr);
    RuntimeContext::GetInstance()->SetCommunicatorAggregator(g_communicatorAggregator);
}

void DistributedDBInterfacesRelationalTest::TearDownTestCase(void)
{
}

void DistributedDBInterfacesRelationalTest::SetUp(void)
{
    DistributedDBToolsUnitTest::PrintTestCaseInfo();

    g_deviceB = new (std::nothrow) RelationalVirtualDevice(DEVICE_B);
    ASSERT_TRUE(g_deviceB != nullptr);
    auto *syncInterfaceB = new (std::nothrow) VirtualRelationalVerSyncDBInterface();
    ASSERT_TRUE(syncInterfaceB != nullptr);
    ASSERT_EQ(g_deviceB->Initialize(g_communicatorAggregator, syncInterfaceB), E_OK);
    auto permissionCheckCallback = [] (const std::string &userId, const std::string &appId, const std::string &storeId,
        const std::string &deviceId, uint8_t flag) -> bool {
        return true;
    };
    EXPECT_EQ(RuntimeConfig::SetPermissionCheckCallback(permissionCheckCallback), OK);
}

void DistributedDBInterfacesRelationalTest::TearDown(void)
{
    DistributedDBToolsUnitTest::RemoveTestDbFiles(g_testDir);
    if (g_deviceB != nullptr) {
        delete g_deviceB;
        g_deviceB = nullptr;
    }
    PermissionCheckCallbackV2 nullCallback;
    EXPECT_EQ(RuntimeConfig::SetPermissionCheckCallback(nullCallback), OK);
    if (g_communicatorAggregator != nullptr) {
        g_communicatorAggregator->RegOnDispatch(nullptr);
    }
}

/**
  * @tc.name: RelationalStoreTest001
  * @tc.desc: Test open store and create distributed db
  * @tc.type: FUNC
  * @tc.require: AR000GK58F
  * @tc.author: lianhuix
  */
HWTEST_F(DistributedDBInterfacesRelationalTest, RelationalStoreTest001, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Prepare db file
     * @tc.expected: step1. Return OK.
     */
    sqlite3 *db = RelationalTestUtils::CreateDataBase(g_dbDir + STORE_ID + DB_SUFFIX);
    ASSERT_NE(db, nullptr);
    EXPECT_EQ(RelationalTestUtils::ExecSql(db, "PRAGMA journal_mode=WAL;"), SQLITE_OK);
    EXPECT_EQ(RelationalTestUtils::ExecSql(db, NORMAL_CREATE_TABLE_SQL), SQLITE_OK);
    RelationalTestUtils::CreateDeviceTable(db, "sync_data", "DEVICE_A");
    EXPECT_EQ(sqlite3_close_v2(db), SQLITE_OK);

    /**
     * @tc.steps:step2. open relational store, create distributed table, close store
     * @tc.expected: step2. Return OK.
     */
    RelationalStoreDelegate *delegate = nullptr;
    DBStatus status = g_mgr.OpenStore(g_dbDir + STORE_ID + DB_SUFFIX, STORE_ID, {}, delegate);
    EXPECT_EQ(status, OK);
    ASSERT_NE(delegate, nullptr);

    status = delegate->CreateDistributedTable("sync_data");
    EXPECT_EQ(status, OK);

    // test create same table again
    status = delegate->CreateDistributedTable("sync_data");
    EXPECT_EQ(status, OK);

    status = g_mgr.CloseStore(delegate);
    EXPECT_EQ(status, OK);

    /**
     * @tc.steps:step3. drop sync_data table
     * @tc.expected: step3. Return OK.
     */
    db = RelationalTestUtils::CreateDataBase(g_dbDir + STORE_ID + DB_SUFFIX);
    ASSERT_NE(db, nullptr);
    EXPECT_EQ(RelationalTestUtils::ExecSql(db, "drop table sync_data;"), SQLITE_OK);
    EXPECT_EQ(sqlite3_close_v2(db), SQLITE_OK);

    /**
     * @tc.steps:step4. open again, check auxiliary should be delete
     * @tc.expected: step4. Return OK.
     */
    delegate = nullptr;
    status = g_mgr.OpenStore(g_dbDir + STORE_ID + DB_SUFFIX, STORE_ID, {}, delegate);
    EXPECT_EQ(status, OK);
    ASSERT_NE(delegate, nullptr);
    status = g_mgr.CloseStore(delegate);
    EXPECT_EQ(status, OK);
}

/**
  * @tc.name: RelationalStoreTest002
  * @tc.desc: Test open store with invalid path or store ID
  * @tc.type: FUNC
  * @tc.require: AR000GK58F
  * @tc.author: lianhuix
  */
HWTEST_F(DistributedDBInterfacesRelationalTest, RelationalStoreTest002, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Prepare db file
     * @tc.expected: step1. Return OK.
     */
    sqlite3 *db = RelationalTestUtils::CreateDataBase(g_dbDir + STORE_ID + DB_SUFFIX);
    ASSERT_NE(db, nullptr);
    EXPECT_EQ(RelationalTestUtils::ExecSql(db, "PRAGMA journal_mode=WAL;"), SQLITE_OK);
    EXPECT_EQ(RelationalTestUtils::ExecSql(db, NORMAL_CREATE_TABLE_SQL), SQLITE_OK);
    EXPECT_EQ(sqlite3_close_v2(db), SQLITE_OK);

    /**
     * @tc.steps:step2. Test open store with invalid path or store ID
     * @tc.expected: step2. open store failed.
     */
    RelationalStoreDelegate *delegate = nullptr;

    // test open store with path not exist
    DBStatus status = g_mgr.OpenStore(g_dbDir + "tmp/" + STORE_ID + DB_SUFFIX, STORE_ID, {}, delegate);
    EXPECT_NE(status, OK);
    ASSERT_EQ(delegate, nullptr);

    // test open store with empty store_id
    status = g_mgr.OpenStore(g_dbDir + STORE_ID + DB_SUFFIX, {}, {}, delegate);
    EXPECT_NE(status, OK);
    ASSERT_EQ(delegate, nullptr);

    // test open store with path has invalid character
    status = g_mgr.OpenStore(g_dbDir + "t&m$p/" + STORE_ID + DB_SUFFIX, {}, {}, delegate);
    EXPECT_NE(status, OK);
    ASSERT_EQ(delegate, nullptr);

    // test open store with store_id has invalid character
    status = g_mgr.OpenStore(g_dbDir + STORE_ID + DB_SUFFIX, "Relation@al_S$tore_ID", {}, delegate);
    EXPECT_NE(status, OK);
    ASSERT_EQ(delegate, nullptr);

    // test open store with store_id length over MAX_STORE_ID_LENGTH
    status = g_mgr.OpenStore(g_dbDir + STORE_ID + DB_SUFFIX,
        std::string(DBConstant::MAX_STORE_ID_LENGTH + 1, 'a'), {}, delegate);
    EXPECT_NE(status, OK);
    ASSERT_EQ(delegate, nullptr);
}

/**
  * @tc.name: RelationalStoreTest003
  * @tc.desc: Test open store with journal_mode is not WAL
  * @tc.type: FUNC
  * @tc.require: AR000GK58F
  * @tc.author: lianhuix
  */
HWTEST_F(DistributedDBInterfacesRelationalTest, RelationalStoreTest003, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Prepare db file with string is not WAL
     * @tc.expected: step1. Return OK.
     */
    sqlite3 *db = RelationalTestUtils::CreateDataBase(g_dbDir + STORE_ID + DB_SUFFIX);
    ASSERT_NE(db, nullptr);
    EXPECT_EQ(RelationalTestUtils::ExecSql(db, "PRAGMA journal_mode=PERSIST;"), SQLITE_OK);
    EXPECT_EQ(RelationalTestUtils::ExecSql(db, NORMAL_CREATE_TABLE_SQL), SQLITE_OK);
    EXPECT_EQ(sqlite3_close_v2(db), SQLITE_OK);

    /**
     * @tc.steps:step2. Test open store
     * @tc.expected: step2. Open store failed.
     */
    RelationalStoreDelegate *delegate = nullptr;

    // test open store with journal mode is not WAL
    DBStatus status = g_mgr.OpenStore(g_dbDir + STORE_ID + DB_SUFFIX, STORE_ID, {}, delegate);
    EXPECT_NE(status, OK);
    ASSERT_EQ(delegate, nullptr);
}

/**
  * @tc.name: RelationalStoreTest004
  * @tc.desc: Test create distributed table with over limit
  * @tc.type: FUNC
  * @tc.require: AR000GK58F
  * @tc.author: lianhuix
  */
HWTEST_F(DistributedDBInterfacesRelationalTest, RelationalStoreTest004, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Prepare db file with multiple tables
     * @tc.expected: step1. Return OK.
     */
    sqlite3 *db = RelationalTestUtils::CreateDataBase(g_dbDir + STORE_ID + DB_SUFFIX);
    ASSERT_NE(db, nullptr);
    EXPECT_EQ(RelationalTestUtils::ExecSql(db, "PRAGMA journal_mode=WAL;"), SQLITE_OK);
    const int tableCount = DBConstant::MAX_DISTRIBUTED_TABLE_COUNT + 10; // 10: additional size for test abnormal scene
    for (int i=0; i<tableCount; i++) {
        std::string sql = "CREATE TABLE TEST_" + std::to_string(i) + "(id INT PRIMARY KEY, value TEXT);";
        EXPECT_EQ(RelationalTestUtils::ExecSql(db, sql), SQLITE_OK);
    }
    EXPECT_EQ(sqlite3_close_v2(db), SQLITE_OK);

    RelationalStoreDelegate *delegate = nullptr;

    /**
     * @tc.steps:step2. Open store and create multiple distributed table
     * @tc.expected: step2. The tables in limited quantity were created successfully, the others failed.
     */
    DBStatus status = g_mgr.OpenStore(g_dbDir + STORE_ID + DB_SUFFIX, STORE_ID, {}, delegate);
    EXPECT_EQ(status, OK);
    ASSERT_NE(delegate, nullptr);

    for (int i=0; i<tableCount; i++) {
        if (i < DBConstant::MAX_DISTRIBUTED_TABLE_COUNT) {
            EXPECT_EQ(delegate->CreateDistributedTable("TEST_" + std::to_string(i)), OK);
        } else {
            EXPECT_NE(delegate->CreateDistributedTable("TEST_" + std::to_string(i)), OK);
        }
    }

    /**
     * @tc.steps:step3. Close store
     * @tc.expected: step3. Return OK.
     */
    status = g_mgr.CloseStore(delegate);
    EXPECT_EQ(status, OK);
}

/**
  * @tc.name: RelationalStoreTest005
  * @tc.desc: Test create distributed table with invalid table name
  * @tc.type: FUNC
  * @tc.require: AR000GK58F
  * @tc.author: lianhuix
  */
HWTEST_F(DistributedDBInterfacesRelationalTest, RelationalStoreTest005, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Prepare db file
     * @tc.expected: step1. Return OK.
     */
    sqlite3 *db = RelationalTestUtils::CreateDataBase(g_dbDir + STORE_ID + DB_SUFFIX);
    ASSERT_NE(db, nullptr);
    EXPECT_EQ(RelationalTestUtils::ExecSql(db, "PRAGMA journal_mode=WAL;"), SQLITE_OK);
    EXPECT_EQ(RelationalTestUtils::ExecSql(db, NORMAL_CREATE_TABLE_SQL), SQLITE_OK);

    /**
     * @tc.steps:step2. Open store
     * @tc.expected: step2. return OK
     */
    RelationalStoreDelegate *delegate = nullptr;
    DBStatus status = g_mgr.OpenStore(g_dbDir + STORE_ID + DB_SUFFIX, STORE_ID, {}, delegate);
    EXPECT_EQ(status, OK);
    ASSERT_NE(delegate, nullptr);

    /**
     * @tc.steps:step3. Create distributed table with invalid table name
     * @tc.expected: step3. Create distributed table failed.
     */
    EXPECT_NE(delegate->CreateDistributedTable(DBConstant::SYSTEM_TABLE_PREFIX + "_tmp"), OK);

    EXPECT_EQ(delegate->CreateDistributedTable("Handle-J@^."), INVALID_ARGS);

    EXPECT_EQ(RelationalTestUtils::ExecSql(db, INVALID_TABLE_FIELD_SQL), SQLITE_OK);
    EXPECT_EQ(delegate->CreateDistributedTable("t1"), NOT_SUPPORT);

    /**
     * @tc.steps:step4. Close store
     * @tc.expected: step4. Return OK.
     */
    status = g_mgr.CloseStore(delegate);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(sqlite3_close_v2(db), SQLITE_OK);
}

/**
  * @tc.name: RelationalStoreTest006
  * @tc.desc: Test create distributed table with non primary key schema
  * @tc.type: FUNC
  * @tc.require: AR000GK58F
  * @tc.author: lianhuix
  */
HWTEST_F(DistributedDBInterfacesRelationalTest, RelationalStoreTest006, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Prepare db file
     * @tc.expected: step1. Return OK.
     */
    sqlite3 *db = RelationalTestUtils::CreateDataBase(g_dbDir + STORE_ID + DB_SUFFIX);
    ASSERT_NE(db, nullptr);
    EXPECT_EQ(RelationalTestUtils::ExecSql(db, "PRAGMA journal_mode=WAL;"), SQLITE_OK);
    EXPECT_EQ(RelationalTestUtils::ExecSql(db, CREATE_TABLE_SQL_NO_PRIMARY_KEY), SQLITE_OK);
    EXPECT_EQ(sqlite3_close_v2(db), SQLITE_OK);

    /**
     * @tc.steps:step2. Open store
     * @tc.expected: step2. return OK
     */
    RelationalStoreDelegate *delegate = nullptr;
    DBStatus status = g_mgr.OpenStore(g_dbDir + STORE_ID + DB_SUFFIX, STORE_ID, {}, delegate);
    EXPECT_EQ(status, OK);
    ASSERT_NE(delegate, nullptr);

    /**
     * @tc.steps:step3. Create distributed table with invalid table name
     * @tc.expected: step3. Create distributed table failed.
     */
    EXPECT_EQ(delegate->CreateDistributedTable("sync_data"), OK);

    /**
     * @tc.steps:step4. Close store
     * @tc.expected: step4. Return OK.
     */
    status = g_mgr.CloseStore(delegate);
    EXPECT_EQ(status, OK);
    delegate = nullptr;

    status = g_mgr.OpenStore(g_dbDir + STORE_ID + DB_SUFFIX, STORE_ID, {}, delegate);
    EXPECT_EQ(status, OK);
    ASSERT_NE(delegate, nullptr);

    status = g_mgr.CloseStore(delegate);
    EXPECT_EQ(status, OK);
}

/**
  * @tc.name: RelationalStoreTest007
  * @tc.desc: Test create distributed table with table has invalid field name
  * @tc.type: FUNC
  * @tc.require: AR000GK58F
  * @tc.author: lianhuix
  */
HWTEST_F(DistributedDBInterfacesRelationalTest, RelationalStoreTest007, TestSize.Level1)
{
    sqlite3 *db = RelationalTestUtils::CreateDataBase(g_dbDir + STORE_ID + DB_SUFFIX);
    ASSERT_NE(db, nullptr);
    EXPECT_EQ(RelationalTestUtils::ExecSql(db, "PRAGMA journal_mode=WAL;"), SQLITE_OK);
    EXPECT_EQ(RelationalTestUtils::ExecSql(db, UNSUPPORTED_FIELD_TABLE_SQL), SQLITE_OK);
    EXPECT_EQ(sqlite3_close_v2(db), SQLITE_OK);

    RelationalStoreDelegate *delegate = nullptr;
    DBStatus status = g_mgr.OpenStore(g_dbDir + STORE_ID + DB_SUFFIX, STORE_ID, {}, delegate);
    EXPECT_EQ(status, OK);
    ASSERT_NE(delegate, nullptr);

    EXPECT_EQ(delegate->CreateDistributedTable("test"), NOT_SUPPORT);
    status = g_mgr.CloseStore(delegate);
    EXPECT_EQ(status, OK);
}

/**
  * @tc.name: RelationalStoreTest008
  * @tc.desc: Test create distributed table with table has composite primary keys
  * @tc.type: FUNC
  * @tc.require: AR000GK58F
  * @tc.author: lianhuix
  */
HWTEST_F(DistributedDBInterfacesRelationalTest, RelationalStoreTest008, TestSize.Level1)
{
    sqlite3 *db = RelationalTestUtils::CreateDataBase(g_dbDir + STORE_ID + DB_SUFFIX);
    ASSERT_NE(db, nullptr);
    EXPECT_EQ(RelationalTestUtils::ExecSql(db, "PRAGMA journal_mode=WAL;"), SQLITE_OK);
    EXPECT_EQ(RelationalTestUtils::ExecSql(db, COMPOSITE_PRIMARY_KEY_TABLE_SQL), SQLITE_OK);
    EXPECT_EQ(sqlite3_close_v2(db), SQLITE_OK);

    RelationalStoreDelegate *delegate = nullptr;
    DBStatus status = g_mgr.OpenStore(g_dbDir + STORE_ID + DB_SUFFIX, STORE_ID, {}, delegate);
    EXPECT_EQ(status, OK);
    ASSERT_NE(delegate, nullptr);

    EXPECT_EQ(delegate->CreateDistributedTable("workers"), NOT_SUPPORT);
    status = g_mgr.CloseStore(delegate);
    EXPECT_EQ(status, OK);
}

/**
  * @tc.name: RelationalStoreTest009
  * @tc.desc: Test create distributed table with table has history data
  * @tc.type: FUNC
  * @tc.require: AR000GK58F
  * @tc.author: lianhuix
  */
HWTEST_F(DistributedDBInterfacesRelationalTest, RelationalStoreTest009, TestSize.Level1)
{
    sqlite3 *db = RelationalTestUtils::CreateDataBase(g_dbDir + STORE_ID + DB_SUFFIX);
    ASSERT_NE(db, nullptr);
    EXPECT_EQ(RelationalTestUtils::ExecSql(db, "PRAGMA journal_mode=WAL;"), SQLITE_OK);
    EXPECT_EQ(RelationalTestUtils::ExecSql(db, NORMAL_CREATE_TABLE_SQL), SQLITE_OK);
    EXPECT_EQ(RelationalTestUtils::ExecSql(db, INSERT_SYNC_DATA_SQL), SQLITE_OK);
    EXPECT_EQ(sqlite3_close_v2(db), SQLITE_OK);

    RelationalStoreDelegate *delegate = nullptr;
    DBStatus status = g_mgr.OpenStore(g_dbDir + STORE_ID + DB_SUFFIX, STORE_ID, {}, delegate);
    EXPECT_EQ(status, OK);
    ASSERT_NE(delegate, nullptr);

    EXPECT_EQ(delegate->CreateDistributedTable("sync_data"), NOT_SUPPORT);
    status = g_mgr.CloseStore(delegate);
    EXPECT_EQ(status, OK);
}


namespace {
void TableModifyTest(const std::string &modifySql, DBStatus expect)
{
    /**
     * @tc.steps:step1. Prepare db file
     * @tc.expected: step1. Return OK.
     */
    sqlite3 *db = RelationalTestUtils::CreateDataBase(g_dbDir + STORE_ID + DB_SUFFIX);
    ASSERT_NE(db, nullptr);
    EXPECT_EQ(RelationalTestUtils::ExecSql(db, "PRAGMA journal_mode=WAL;"), SQLITE_OK);
    EXPECT_EQ(RelationalTestUtils::ExecSql(db, NORMAL_CREATE_TABLE_SQL), SQLITE_OK);

    RelationalTestUtils::CreateDeviceTable(db, "sync_data", "DEVICE_A");
    RelationalTestUtils::CreateDeviceTable(db, "sync_data", "DEVICE_B");
    RelationalTestUtils::CreateDeviceTable(db, "sync_data", "DEVICE_C");

    /**
     * @tc.steps:step2. Open store
     * @tc.expected: step2. return OK
     */
    RelationalStoreDelegate *delegate = nullptr;
    DBStatus status = g_mgr.OpenStore(g_dbDir + STORE_ID + DB_SUFFIX, STORE_ID, {}, delegate);
    EXPECT_EQ(status, OK);
    ASSERT_NE(delegate, nullptr);

    /**
     * @tc.steps:step3. Create distributed table
     * @tc.expected: step3. Create distributed table OK.
     */
    EXPECT_EQ(delegate->CreateDistributedTable("sync_data"), OK);

    /**
     * @tc.steps:step4. Upgrade table with modifySql
     * @tc.expected: step4. return OK
     */
    EXPECT_EQ(RelationalTestUtils::ExecSql(db, modifySql), SQLITE_OK);

    /**
     * @tc.steps:step5. Create distributed table again
     * @tc.expected: step5. Create distributed table return expect.
     */
    EXPECT_EQ(delegate->CreateDistributedTable("sync_data"), expect);

    /**
     * @tc.steps:step6. Close store
     * @tc.expected: step6 Return OK.
     */
    status = g_mgr.CloseStore(delegate);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(sqlite3_close_v2(db), SQLITE_OK);
}
}

/**
  * @tc.name: RelationalTableModifyTest001
  * @tc.desc: Test modify distributed table with compatible upgrade
  * @tc.type: FUNC
  * @tc.require: AR000GK58F
  * @tc.author: lianhuix
  */
HWTEST_F(DistributedDBInterfacesRelationalTest, RelationalTableModifyTest001, TestSize.Level1)
{
    TableModifyTest("ALTER TABLE sync_data ADD COLUMN add_field INTEGER NOT NULL DEFAULT 123;", OK);
}

/**
  * @tc.name: RelationalTableModifyTest002
  * @tc.desc: Test modify distributed table with incompatible upgrade
  * @tc.type: FUNC
  * @tc.require: AR000GK58F
  * @tc.author: lianhuix
  */
HWTEST_F(DistributedDBInterfacesRelationalTest, RelationalTableModifyTest002, TestSize.Level1)
{
    TableModifyTest("ALTER TABLE sync_data ADD COLUMN add_field INTEGER NOT NULL;", SCHEMA_MISMATCH);
}

/**
  * @tc.name: RelationalTableModifyTest003
  * @tc.desc: Test modify distributed table with incompatible upgrade
  * @tc.type: FUNC
  * @tc.require: AR000GK58F
  * @tc.author: lianhuix
  */
HWTEST_F(DistributedDBInterfacesRelationalTest, RelationalTableModifyTest003, TestSize.Level1)
{
    TableModifyTest("ALTER TABLE sync_data DROP COLUMN w_timestamp;", SCHEMA_MISMATCH);
}

/**
  * @tc.name: RelationalTableModifyTest004
  * @tc.desc: Test upgrade distributed table with device table exists
  * @tc.type: FUNC
  * @tc.require: AR000GK58F
  * @tc.author: lianhuix
  */
HWTEST_F(DistributedDBInterfacesRelationalTest, RelationalTableModifyTest004, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Prepare db file
     * @tc.expected: step1. Return OK.
     */
    sqlite3 *db = RelationalTestUtils::CreateDataBase(g_dbDir + STORE_ID + DB_SUFFIX);
    ASSERT_NE(db, nullptr);
    EXPECT_EQ(RelationalTestUtils::ExecSql(db, "PRAGMA journal_mode=WAL;"), SQLITE_OK);
    EXPECT_EQ(RelationalTestUtils::ExecSql(db, NORMAL_CREATE_TABLE_SQL), SQLITE_OK);
    RelationalTestUtils::CreateDeviceTable(db, "sync_data", "DEVICE_A");
    RelationalTestUtils::CreateDeviceTable(db, "sync_data", "DEVICE_B");
    RelationalTestUtils::CreateDeviceTable(db, "sync_data", "DEVICE_C");

    /**
     * @tc.steps:step2. Open store
     * @tc.expected: step2. return OK
     */
    RelationalStoreDelegate *delegate = nullptr;
    DBStatus status = g_mgr.OpenStore(g_dbDir + STORE_ID + DB_SUFFIX, STORE_ID, {}, delegate);
    EXPECT_EQ(status, OK);
    ASSERT_NE(delegate, nullptr);

    /**
     * @tc.steps:step3. Create distributed table
     * @tc.expected: step3. Create distributed table OK.
     */
    EXPECT_EQ(delegate->CreateDistributedTable("sync_data"), OK);

    /**
     * @tc.steps:step4. Upgrade table
     * @tc.expected: step4. return OK
     */
    std::string modifySql = "ALTER TABLE sync_data ADD COLUMN add_field INTEGER;";
    EXPECT_EQ(RelationalTestUtils::ExecSql(db, modifySql), SQLITE_OK);
    std::string indexSql = "CREATE INDEX add_index ON sync_data (add_field);";
    EXPECT_EQ(RelationalTestUtils::ExecSql(db, indexSql), SQLITE_OK);
    std::string deleteIndexSql = "DROP INDEX IF EXISTS key_index";
    EXPECT_EQ(RelationalTestUtils::ExecSql(db, deleteIndexSql), SQLITE_OK);
    EXPECT_EQ(RelationalTestUtils::ExecSql(db, INSERT_SYNC_DATA_SQL), SQLITE_OK);

    /**
     * @tc.steps:step5. Create distributed table again
     * @tc.expected: step5. Create distributed table return expect.
     */
    EXPECT_EQ(delegate->CreateDistributedTable("sync_data"), OK);

    /**
     * @tc.steps:step6. Close store
     * @tc.expected: step6 Return OK.
     */
    status = g_mgr.CloseStore(delegate);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(sqlite3_close_v2(db), SQLITE_OK);
}

/**
  * @tc.name: RelationalTableModifyTest005
  * @tc.desc: Test modify distributed table with compatible upgrade
  * @tc.type: FUNC
  * @tc.require: AR000GK58F
  * @tc.author: lianhuix
  */
HWTEST_F(DistributedDBInterfacesRelationalTest, RelationalTableModifyTest005, TestSize.Level1)
{
    TableModifyTest("ALTER TABLE sync_data ADD COLUMN add_field STRING NOT NULL DEFAULT 'asdf';", OK);
}

/**
  * @tc.name: RelationalRemoveDeviceDataTest001
  * @tc.desc: Test remove device data
  * @tc.type: FUNC
  * @tc.require: AR000GK58F
  * @tc.author: lianhuix
  */
HWTEST_F(DistributedDBInterfacesRelationalTest, RelationalRemoveDeviceDataTest001, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Prepare db file
     * @tc.expected: step1. Return OK.
     */
    sqlite3 *db = RelationalTestUtils::CreateDataBase(g_dbDir + STORE_ID + DB_SUFFIX);
    ASSERT_NE(db, nullptr);
    EXPECT_EQ(RelationalTestUtils::ExecSql(db, "PRAGMA journal_mode=WAL;"), SQLITE_OK);
    EXPECT_EQ(RelationalTestUtils::ExecSql(db, NORMAL_CREATE_TABLE_SQL), SQLITE_OK);
    RelationalTestUtils::CreateDeviceTable(db, "sync_data", "DEVICE_A");
    RelationalTestUtils::CreateDeviceTable(db, "sync_data", "DEVICE_B");
    RelationalTestUtils::CreateDeviceTable(db, "sync_data", "DEVICE_C");

    /**
     * @tc.steps:step2. Open store
     * @tc.expected: step2. return OK
     */
    RelationalStoreDelegate *delegate = nullptr;
    DBStatus status = g_mgr.OpenStore(g_dbDir + STORE_ID + DB_SUFFIX, STORE_ID, {}, delegate);
    EXPECT_EQ(status, OK);
    ASSERT_NE(delegate, nullptr);

    /**
     * @tc.steps:step3. Remove device data
     * @tc.expected: step3. ok
     */
    EXPECT_EQ(delegate->CreateDistributedTable("sync_data"), OK);
    EXPECT_EQ(delegate->RemoveDeviceData("DEVICE_A"), OK);
    EXPECT_EQ(delegate->RemoveDeviceData("DEVICE_B"), OK);
    EXPECT_EQ(delegate->RemoveDeviceData("DEVICE_C", "sync_data"), OK);

    /**
     * @tc.steps:step4. Remove device data with invalid args
     * @tc.expected: step4. invalid
     */
    EXPECT_EQ(delegate->RemoveDeviceData(""), INVALID_ARGS);
    EXPECT_EQ(delegate->RemoveDeviceData("", "sync_data"), INVALID_ARGS);
    EXPECT_EQ(delegate->RemoveDeviceData("DEVICE_A", "Handle-J@^."), INVALID_ARGS);

    /**
     * @tc.steps:step5. Close store
     * @tc.expected: step5 Return OK.
     */
    status = g_mgr.CloseStore(delegate);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(sqlite3_close_v2(db), SQLITE_OK);
}

namespace {
struct TableT1 {
    int a;
    std::string b;
    int rowid;
    int flag;
    int timestamp;

    VirtualRowData operator() () const
    {
        VirtualRowData rowData;
        DataValue dA;
        dA = static_cast<int64_t>(a);
        rowData.objectData.PutDataValue("a", dA);
        DataValue dB;
        dB.SetText(b);
        rowData.objectData.PutDataValue("b", dB);
        rowData.logInfo.dataKey = rowid;
        rowData.logInfo.device = DEVICE_B;
        rowData.logInfo.originDev = DEVICE_B;
        rowData.logInfo.timestamp = timestamp;
        rowData.logInfo.wTimestamp = timestamp;
        rowData.logInfo.flag = flag;
        Key key;
        DBCommon::StringToVector(std::to_string(rowid), key);
        std::vector<uint8_t> hashKey;
        DBCommon::CalcValueHash(key, hashKey);
        rowData.logInfo.hashKey = hashKey;
        return rowData;
    }
};

void AddDeviceSchema(RelationalVirtualDevice *device, sqlite3 *db, const std::string &name)
{
    TableInfo table;
    SQLiteUtils::AnalysisSchema(db, name, table);
    device->SetLocalFieldInfo(table.GetFieldInfos());
    device->SetTableInfo(table);
}
}

/**
  * @tc.name: RelationalRemoveDeviceDataTest002
  * @tc.desc: Test remove device data and syn
  * @tc.type: FUNC
  * @tc.require: AR000GK58F
  * @tc.author: lianhuix
  */
HWTEST_F(DistributedDBInterfacesRelationalTest, RelationalRemoveDeviceDataTest002, TestSize.Level1)
{
    sqlite3 *db = RelationalTestUtils::CreateDataBase(g_dbDir + STORE_ID + DB_SUFFIX);
    ASSERT_NE(db, nullptr);
    EXPECT_EQ(RelationalTestUtils::ExecSql(db, "PRAGMA journal_mode=WAL;"), SQLITE_OK);
    EXPECT_EQ(RelationalTestUtils::ExecSql(db, SIMPLE_CREATE_TABLE_SQL), SQLITE_OK);
    AddDeviceSchema(g_deviceB, db, "t1");

    RelationalStoreDelegate *delegate = nullptr;
    DBStatus status = g_mgr.OpenStore(g_dbDir + STORE_ID + DB_SUFFIX, STORE_ID, {}, delegate);
    EXPECT_EQ(status, OK);
    ASSERT_NE(delegate, nullptr);

    EXPECT_EQ(delegate->CreateDistributedTable("t1"), OK);

    g_deviceB->PutDeviceData("t1", std::vector<TableT1>{
        {1, "111", 1, 2, 1}, // test data
        {2, "222", 2, 2, 2}, // test data
        {3, "333", 3, 2, 3}, // test data
        {4, "444", 4, 2, 4} // test data
    });
    std::vector<std::string> devices = {DEVICE_B};
    Query query = Query::Select("t1").NotEqualTo("a", 0);
    status = delegate->Sync(devices, SyncMode::SYNC_MODE_PULL_ONLY, query,
        [&devices](const std::map<std::string, std::vector<TableStatus>> &devicesMap) {
            EXPECT_EQ(devicesMap.size(), devices.size());
            EXPECT_EQ(devicesMap.at(DEVICE_B)[0].status, OK);
        }, true);
    EXPECT_EQ(status, OK);

    EXPECT_EQ(delegate->RemoveDeviceData(DEVICE_B), OK);

    int logCnt = -1;
    std::string checkLogSql = "SELECT count(*) FROM naturalbase_rdb_aux_t1_log WHERE device = '" + DEVICE_B + "'";
    RelationalTestUtils::ExecSql(db, checkLogSql, nullptr, [&logCnt](sqlite3_stmt *stmt) {
        logCnt = sqlite3_column_int(stmt, 0);
        return E_OK;
    });
    EXPECT_EQ(logCnt, 0);

    int dataCnt = -1;
    std::string deviceTable = g_mgr.GetDistributedTableName(DEVICE_B, "t1");
    std::string checkDataSql = "SELECT count(*) FROM " + deviceTable + " WHERE device = '" + DEVICE_B + "'";
    RelationalTestUtils::ExecSql(db, checkDataSql, nullptr, [&dataCnt](sqlite3_stmt *stmt) {
        dataCnt = sqlite3_column_int(stmt, 0);
        return E_OK;
    });
    EXPECT_EQ(logCnt, 0);

    status = g_mgr.CloseStore(delegate);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(sqlite3_close_v2(db), SQLITE_OK);
}

/**
  * @tc.name: RelationalOpenStorePathCheckTest001
  * @tc.desc: Test open store with same label but different path.
  * @tc.type: FUNC
  * @tc.require: AR000GK58F
  * @tc.author: lianhuix
  */
HWTEST_F(DistributedDBInterfacesRelationalTest, RelationalOpenStorePathCheckTest001, TestSize.Level1)
{
    std::string dir1 = g_dbDir + "dbDir1";
    EXPECT_EQ(OS::MakeDBDirectory(dir1), E_OK);
    sqlite3 *db1 = RelationalTestUtils::CreateDataBase(dir1 + STORE_ID + DB_SUFFIX);
    ASSERT_NE(db1, nullptr);
    EXPECT_EQ(RelationalTestUtils::ExecSql(db1, "PRAGMA journal_mode=WAL;"), SQLITE_OK);
    EXPECT_EQ(RelationalTestUtils::ExecSql(db1, NORMAL_CREATE_TABLE_SQL), SQLITE_OK);
    EXPECT_EQ(sqlite3_close_v2(db1), SQLITE_OK);

    std::string dir2 = g_dbDir + "dbDir2";
    EXPECT_EQ(OS::MakeDBDirectory(dir2), E_OK);
    sqlite3 *db2 = RelationalTestUtils::CreateDataBase(dir2 + STORE_ID + DB_SUFFIX);
    ASSERT_NE(db2, nullptr);
    EXPECT_EQ(RelationalTestUtils::ExecSql(db2, "PRAGMA journal_mode=WAL;"), SQLITE_OK);
    EXPECT_EQ(RelationalTestUtils::ExecSql(db2, NORMAL_CREATE_TABLE_SQL), SQLITE_OK);
    EXPECT_EQ(sqlite3_close_v2(db2), SQLITE_OK);

    DBStatus status = OK;
    RelationalStoreDelegate *delegate1 = nullptr;
    status = g_mgr.OpenStore(dir1 + STORE_ID + DB_SUFFIX, STORE_ID, {}, delegate1);
    EXPECT_EQ(status, OK);
    ASSERT_NE(delegate1, nullptr);

    RelationalStoreDelegate *delegate2 = nullptr;
    status = g_mgr.OpenStore(dir2 + STORE_ID + DB_SUFFIX, STORE_ID, {}, delegate2);
    EXPECT_EQ(status, INVALID_ARGS);
    ASSERT_EQ(delegate2, nullptr);

    status = g_mgr.CloseStore(delegate1);
    EXPECT_EQ(status, OK);

    status = g_mgr.CloseStore(delegate2);
    EXPECT_EQ(status, INVALID_ARGS);
}

HWTEST_F(DistributedDBInterfacesRelationalTest, RelationalOpenStorePressureTest001, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Prepare db file
     * @tc.expected: step1. Return OK.
     */
    sqlite3 *db = RelationalTestUtils::CreateDataBase(g_dbDir + STORE_ID + DB_SUFFIX);
    ASSERT_NE(db, nullptr);
    EXPECT_EQ(RelationalTestUtils::ExecSql(db, "PRAGMA journal_mode=WAL;"), SQLITE_OK);
    EXPECT_EQ(RelationalTestUtils::ExecSql(db, NORMAL_CREATE_TABLE_SQL), SQLITE_OK);
    EXPECT_EQ(sqlite3_close_v2(db), SQLITE_OK);

    DBStatus status = OK;
    for (int i = 0; i < 1000; i++) {
        RelationalStoreDelegate *delegate = nullptr;
        status = g_mgr.OpenStore(g_dbDir + STORE_ID + DB_SUFFIX, STORE_ID, {}, delegate);
        EXPECT_EQ(status, OK);
        ASSERT_NE(delegate, nullptr);

        status = g_mgr.CloseStore(delegate);
        EXPECT_EQ(status, OK);
        delegate = nullptr;
    }
}

HWTEST_F(DistributedDBInterfacesRelationalTest, RelationalOpenStorePressureTest002, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Prepare db file
     * @tc.expected: step1. Return OK.
     */
    sqlite3 *db = RelationalTestUtils::CreateDataBase(g_dbDir + STORE_ID + DB_SUFFIX);
    ASSERT_NE(db, nullptr);
    EXPECT_EQ(RelationalTestUtils::ExecSql(db, "PRAGMA journal_mode=WAL;"), SQLITE_OK);
    EXPECT_EQ(RelationalTestUtils::ExecSql(db, NORMAL_CREATE_TABLE_SQL), SQLITE_OK);
    EXPECT_EQ(sqlite3_close_v2(db), SQLITE_OK);

    std::queue<RelationalStoreDelegate *> delegateQueue;
    std::mutex queueLock;
    std::random_device rd;
    default_random_engine e(rd());
    uniform_int_distribution<unsigned> u(0, 9);

    std::thread openStoreThread([&, this]() {
        for (int i = 0; i < 1000; i++) {
            LOGD("++++> open store delegate: %d", i);
            RelationalStoreDelegate *delegate = nullptr;
            DBStatus status = g_mgr.OpenStore(g_dbDir + STORE_ID + DB_SUFFIX, STORE_ID, {}, delegate);
            EXPECT_EQ(status, OK);
            ASSERT_NE(delegate, nullptr);
            {
                std::lock_guard<std::mutex> lock(queueLock);
                delegateQueue.push(delegate);
            }
            LOGD("++++< open store delegate: %d", i);
        }
    });

    int cnt = 0;
    while (cnt < 1000) {
        RelationalStoreDelegate *delegate = nullptr;
        {
            std::lock_guard<std::mutex> lock(queueLock);
            if (delegateQueue.empty()) {
                std::this_thread::sleep_for(std::chrono::microseconds(100));
                continue;
            }
            delegate = delegateQueue.front();
            delegateQueue.pop();
        }
        LOGD("++++> close store delegate: %d", cnt);
        DBStatus status = g_mgr.CloseStore(delegate);
        LOGD("++++< close store delegate: %d", cnt);
        EXPECT_EQ(status, OK);
        delegate = nullptr;
        cnt++;
        std::this_thread::sleep_for(std::chrono::microseconds(100 * u(e)));
    }
    openStoreThread.join();
}
