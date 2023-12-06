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

#include <gtest/gtest.h>
#include <thread>

#include "db_common.h"
#include "db_constant.h"
#include "db_errno.h"
#include "distributeddb_data_generate_unit_test.h"
#include "distributeddb_tools_unit_test.h"
#include "log_print.h"
#include "platform_specific.h"
#include "process_system_api_adapter_impl.h"
#include "runtime_context.h"
#include "sqlite_single_ver_natural_store.h"
#include "system_timer.h"

using namespace testing::ext;
using namespace DistributedDB;
using namespace DistributedDBUnitTest;
using namespace std;

namespace {
    // define some variables to init a KvStoreDelegateManager object.
    KvStoreDelegateManager g_mgr(APP_ID, USER_ID);
    string g_testDir;
    KvStoreConfig g_config;
    Key g_keyPrefix = {'A', 'B', 'C'};
    const int RESULT_SET_COUNT = 9;
    const int RESULT_SET_INIT_POS = -1;
    uint8_t g_testDict[RESULT_SET_COUNT] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};

    // define the g_kvNbDelegateCallback, used to get some information when open a kv store.
    DBStatus g_kvDelegateStatus = INVALID_ARGS;
    KvStoreNbDelegate *g_kvNbDelegatePtr = nullptr;
    KvStoreDelegate *g_kvDelegatePtr = nullptr;
    const int OBSERVER_SLEEP_TIME = 100;
    const int BATCH_PRESET_SIZE_TEST = 10;
    const int DIVIDE_BATCH_PRESET_SIZE = 5;
    const int VALUE_OFFSET = 5;

    const int DEFAULT_KEY_VALUE_SIZE = 10;

    const int CON_PUT_THREAD_NUM = 4;
    const int PER_THREAD_PUT_NUM = 100;

    // the type of g_kvNbDelegateCallback is function<void(DBStatus, KvStoreDelegate*)>
    auto g_kvNbDelegateCallback = bind(&DistributedDBToolsUnitTest::KvStoreNbDelegateCallback, placeholders::_1,
        placeholders::_2, std::ref(g_kvDelegateStatus), std::ref(g_kvNbDelegatePtr));

    // the type of g_kvDelegateCallback is function<void(DBStatus, KvStoreDelegate*)>
    auto g_kvDelegateCallback = bind(&DistributedDBToolsUnitTest::KvStoreDelegateCallback, placeholders::_1,
        placeholders::_2, std::ref(g_kvDelegateStatus), std::ref(g_kvDelegatePtr));

    enum LockState {
        UNLOCKED = 0,
        LOCKED
    };

    void InitResultSet()
    {
        Key testKey;
        Value testValue;
        for (int i = 0; i < RESULT_SET_COUNT; i++) {
            testKey.clear();
            testValue.clear();
            // set key
            testKey = g_keyPrefix;
            testKey.push_back(g_testDict[i]);
            // set value
            testValue.push_back(g_testDict[i]);
            // insert entry
            EXPECT_EQ(g_kvNbDelegatePtr->Put(testKey, testValue), OK);
        }
    }

    void ReadResultSet(KvStoreResultSet *readResultSet)
    {
        if (readResultSet == nullptr) {
            return;
        }
        // index from 0 to 8(first to last)
        for (int i = 0; i < RESULT_SET_COUNT; i++) {
            Entry entry;
            std::vector<uint8_t> cursorKey = g_keyPrefix;
            cursorKey.push_back(g_testDict[i]);
            std::vector<uint8_t> cursorValue;
            cursorValue.push_back(g_testDict[i]);
            EXPECT_TRUE(readResultSet->MoveToNext());
            EXPECT_EQ(readResultSet->GetEntry(entry), OK);
            EXPECT_EQ(entry.key, cursorKey);
            EXPECT_EQ(entry.value, cursorValue);
            EXPECT_TRUE(!readResultSet->IsBeforeFirst());
            EXPECT_TRUE(!readResultSet->IsAfterLast());
        }
        // change index to 8(last)
        EXPECT_EQ(readResultSet->GetPosition(), RESULT_SET_COUNT - 1);
        EXPECT_TRUE(!readResultSet->IsFirst());
        EXPECT_TRUE(readResultSet->IsLast());
        EXPECT_TRUE(!readResultSet->IsBeforeFirst());
        EXPECT_TRUE(!readResultSet->IsAfterLast());
    }

    void CheckResultSetValue(KvStoreResultSet *readResultSet, DBStatus errCode, int position)
    {
        if (readResultSet == nullptr) {
            return;
        }
        Entry entry;
        EXPECT_EQ(readResultSet->GetPosition(), position);
        EXPECT_EQ(readResultSet->GetEntry(entry), errCode);
        if (errCode == OK) {
            std::vector<uint8_t> cursorKey;
            std::vector<uint8_t> cursorValue;
            if (position > RESULT_SET_INIT_POS && position < RESULT_SET_COUNT) {
                uint8_t keyPostfix = g_testDict[position];
                // set key
                cursorKey = g_keyPrefix;
                cursorKey.push_back(keyPostfix);
                // set value
                cursorValue.push_back(keyPostfix);
            }
            // check key and value
            EXPECT_EQ(entry.key, cursorKey);
            EXPECT_EQ(entry.value, cursorValue);
        }
    }

    std::vector<Entry> g_entriesForConcurrency;
    void PutData(KvStoreNbDelegate *kvStore, int flag)
    {
        for (int i = 0; i < PER_THREAD_PUT_NUM; i++) {
            int index = flag * PER_THREAD_PUT_NUM + i;
            kvStore->Put(g_entriesForConcurrency[index].key, g_entriesForConcurrency[index].value);
        }
        LOGD("%dth put has been finished", flag);
    }

    bool CheckDataTimestamp(const std::string &storeId)
    {
        std::string identifier = USER_ID + "-" + APP_ID + "-" + storeId;
        std::string hashIdentifier = DBCommon::TransferHashString(identifier);
        std::string identifierName = DBCommon::TransferStringToHex(hashIdentifier);
        std::string storeDir = g_testDir + "/" + identifierName + "/" + DBConstant::SINGLE_SUB_DIR + "/" +
            DBConstant::MAINDB_DIR + "/" + DBConstant::SINGLE_VER_DATA_STORE + DBConstant::SQLITE_DB_EXTENSION;
        sqlite3 *db = nullptr;
        EXPECT_EQ(sqlite3_open_v2(storeDir.c_str(), &db, SQLITE_OPEN_READWRITE, nullptr), SQLITE_OK);
        if (db == nullptr) {
            return false;
        }

        static const std::string selectSQL = "select timestamp from sync_data order by rowid;";
        sqlite3_stmt *statement = nullptr;
        EXPECT_EQ(sqlite3_prepare(db, selectSQL.c_str(), -1, &statement, NULL), SQLITE_OK);
        std::vector<int64_t> timeVect;
        while (sqlite3_step(statement) == SQLITE_ROW) {
            timeVect.push_back(sqlite3_column_int64(statement, 0));
        }

        sqlite3_finalize(statement);
        statement = nullptr;
        (void)sqlite3_close_v2(db);
        db = nullptr;
        EXPECT_EQ(timeVect.size(), g_entriesForConcurrency.size());
        bool resultCheck = true;
        if (g_entriesForConcurrency.size() > 1) {
            for (size_t i = 1; i < timeVect.size(); i++) {
                if (timeVect[i] <= timeVect[i - 1]) {
                    resultCheck = false;
                    break;
                }
            }
        }

        return resultCheck;
    }
}
class DistributedDBInterfacesNBDelegateTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBInterfacesNBDelegateTest::SetUpTestCase(void)
{
    DistributedDBToolsUnitTest::TestDirInit(g_testDir);
    g_config.dataDir = g_testDir;
    g_mgr.SetKvStoreConfig(g_config);
}

void DistributedDBInterfacesNBDelegateTest::TearDownTestCase(void)
{
    if (DistributedDBToolsUnitTest::RemoveTestDbFiles(g_testDir) != 0) {
        LOGE("rm test db files error!");
    }
    RuntimeContext::GetInstance()->SetProcessSystemApiAdapter(nullptr);
}

void DistributedDBInterfacesNBDelegateTest::SetUp(void)
{
    DistributedDBToolsUnitTest::PrintTestCaseInfo();
    g_kvDelegateStatus = INVALID_ARGS;
    g_kvNbDelegatePtr = nullptr;
    g_kvDelegatePtr = nullptr;
}

void DistributedDBInterfacesNBDelegateTest::TearDown(void)
{
    if (g_kvDelegatePtr != nullptr) {
        g_mgr.CloseKvStore(g_kvNbDelegatePtr);
        g_kvNbDelegatePtr = nullptr;
    }
    RuntimeContext::GetInstance()->SetProcessSystemApiAdapter(nullptr);
}

/**
  * @tc.name: CombineTest001
  * @tc.desc: Test the NbDelegate for combined operation.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesNBDelegateTest, CombineTest001, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Get the nb delegate.
     * @tc.expected: step1. Get results OK and non-null delegate.
     */
    KvStoreNbDelegate::Option option = {true, false, false};
    g_mgr.GetKvStore("distributed_nb_delegate_test", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    Key key;
    key = {'A', 'C', 'Q'};
    Value value;
    value = {'G', 'D', 'O'};
    Value valueRead;
    KvStoreObserverUnitTest *observer = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(observer != nullptr);
    /**
     * @tc.steps:step2. Register the non-null observer for the special key.
     * @tc.expected: step2. Register results OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->RegisterObserver(key, OBSERVER_CHANGES_LOCAL_ONLY, observer), OK);
    /**
     * @tc.steps:step3. Put the local data.
     * @tc.expected: step3. Put returns OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocal(key, value), OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    /**
     * @tc.steps:step4. Check the local data.
     * @tc.expected: step4. The get data is equal to the put data.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(key, valueRead), OK);
    /**
     * @tc.steps:step5. Delete the local data.
     * @tc.expected: step5. Delete returns OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->DeleteLocal(key), OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    /**
     * @tc.steps:step6. Check the local data.
     * @tc.expected: step6. Couldn't find the deleted data.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(key, valueRead), NOT_FOUND);
    /**
     * @tc.steps:step7. UnRegister the observer.
     * @tc.expected: step7. Returns OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->UnRegisterObserver(observer), OK);
    delete observer;
    observer = nullptr;
    Key key1;
    key1 = {'D', 'B', 'N'};
    Value value1;
    value1 = {'P', 'D', 'G'};

    Key key2 = key1;
    Value value2;
    key2.push_back('U');
    value2 = {'C'};
    /**
     * @tc.steps:step8. Put the data.
     * @tc.expected: step8. Put returns OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Put(key1, value1), OK);
    Value valueRead2;
    /**
     * @tc.steps:step9. Check the data.
     * @tc.expected: step9. Getting the put data returns OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Get(key1, valueRead2), OK);
    /**
     * @tc.steps:step10. Put another data.
     * @tc.expected: step10. Returns OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Put(key2, value2), OK);
    std::vector<Entry> vect;
    /**
     * @tc.steps:step10. Get the batch data using the prefix key.
     * @tc.expected: step10. Results OK and the batch data size is equal to the put data size.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->GetEntries(key1, vect), OK);
    EXPECT_EQ(vect.size(), 2UL);
    /**
     * @tc.steps:step11. Delete one data.
     * @tc.expected: step11. Results OK and couldn't get the deleted data.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Delete(key1), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Get(key1, valueRead2), NOT_FOUND);

    LOGD("Close store");
    /**
     * @tc.steps:step12. Close the kv store.
     * @tc.expected: step12. Results OK and delete successfully.
     */
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_nb_delegate_test"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: CreateMemoryDb001
  * @tc.desc: Create memory database after.
  * @tc.type: FUNC
  * @tc.require: AR000CRAKN
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesNBDelegateTest, CreateMemoryDb001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Memory database by GetKvStore.
     * @tc.expected: step1. Create successfully.
     */
    const KvStoreNbDelegate::Option option = {true, true};
    g_mgr.SetKvStoreConfig(g_config);
    g_mgr.GetKvStore("distributed_Memorykvstore_001", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    KvStoreNbDelegate *kvNbDelegatePtr001 = g_kvNbDelegatePtr;

    /**
     * @tc.steps: step2. Duplicate create Memory database by GetKvStore.
     * @tc.expected: step2. Duplicate create successfully.
     */
    g_mgr.GetKvStore("distributed_Memorykvstore_001", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);

    /**
     * @tc.steps: step3. Duplicate create Memory database by GetKvStore.
     * @tc.expected: step3. Duplicate create successfully.
     */
    g_mgr.GetKvStore("distributed_Memorykvstore_002", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    KvStoreNbDelegate *kvNbDelegatePtr002 = g_kvNbDelegatePtr;

    g_mgr.CloseKvStore(kvNbDelegatePtr001);
    g_mgr.CloseKvStore(kvNbDelegatePtr002);
}

/**
  * @tc.name: CreateMemoryDb002
  * @tc.desc: The MemoryDB cannot be created or open, when the physical database has been opened
  * @tc.type: FUNC
  * @tc.require: AR000CRAKN
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesNBDelegateTest, CreateMemoryDb002, TestSize.Level1)
{
    KvStoreNbDelegate::Option option = {true, true};
    /**
     * @tc.steps: step1. Create SingleVer database by GetKvStore.
     * @tc.expected: step1. Create database success.
     */
    g_mgr.GetKvStore("distributed_Memorykvstore_002", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    KvStoreNbDelegate *delegate1 = g_kvNbDelegatePtr;
    g_kvNbDelegatePtr = nullptr;

    /**
     * @tc.steps: step2. Create Memory database by GetKvStore.
     * @tc.expected: step2. Create Memory database fail.
     */
    option.isMemoryDb = false;
    g_mgr.GetKvStore("distributed_Memorykvstore_002", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr == nullptr);
    EXPECT_TRUE(g_kvDelegateStatus != OK);
    g_mgr.CloseKvStore(delegate1);
    delegate1 = nullptr;
}

/**
  * @tc.name: CreateMemoryDb003
  * @tc.desc: The physical database cannot be created or open, when the MemoryDB has been opened.
  * @tc.type: FUNC
  * @tc.require: AR000CRAKN
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesNBDelegateTest, CreateMemoryDb003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Get singleVer kvStore by GetKvStore.
     * @tc.expected: step1. Get database success.
     */
    KvStoreDelegate::Option option;
    g_mgr.GetKvStore("distributed_Memorykvstore_003", option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    /**
     * @tc.steps: step2. Create Memory database by GetKvStore.
     * @tc.expected: step2. Create Memory database fail.
     */
    KvStoreNbDelegate::Option nbOption = {true, true};
    g_mgr.GetKvStore("distributed_Memorykvstore_003", nbOption, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr == nullptr);
    EXPECT_TRUE(g_kvDelegateStatus != OK);
    g_mgr.CloseKvStore(g_kvDelegatePtr);
    g_kvDelegatePtr = nullptr;
}

/**
  * @tc.name: OperMemoryDbData001
  * @tc.desc: Operate memory database
  * @tc.type: FUNC
  * @tc.require: AR000CRAKN
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesNBDelegateTest, OperMemoryDbData001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Memory database by GetKvStore.
     */
    const KvStoreNbDelegate::Option option = {true, true};
    g_mgr.GetKvStore("distributed_OperMemorykvstore_001", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    /**
     * @tc.steps: step2. Put (KEY_1,VALUE_1)(KEY_2,VALUE_2) to Memory database.
     * @tc.expected: step2. Success.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_1, VALUE_1), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_2, VALUE_2), OK);

    /**
     * @tc.steps: step3. Get (KEY_1,VALUE_1)(KEY_2,VALUE_2) to Memory database.
     * @tc.expected: step3. Success.
     */
    Value readValueKey1;
    Value readValueKey2;
    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY_1, readValueKey1), OK);
    EXPECT_EQ(readValueKey1, VALUE_1);

    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY_2, readValueKey2), OK);
    EXPECT_EQ(readValueKey2, VALUE_2);

    /**
     * @tc.steps: step4. Delete K1 from Memory database.
     * @tc.expected: step4. Success.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Delete(KEY_1), OK);

    /**
     * @tc.steps: step5. Get K1 from Memory database.
     * @tc.expected: step5. NOT_FOUND.
     */
    readValueKey1.clear();
    readValueKey2.clear();
    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY_1, readValueKey1), NOT_FOUND);

    /**
     * @tc.steps: step6. Update K2 value from Memory database.
     * @tc.expected: step6. Get the right value after the update.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_2, VALUE_3), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY_2, readValueKey2), OK);
    EXPECT_EQ(readValueKey2, VALUE_3);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
}

/**
  * @tc.name: CloseMemoryDb001
  * @tc.desc: Operate memory database after reopen memory database
  * @tc.type: FUNC
  * @tc.require: AR000CRAKN
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesNBDelegateTest, CloseMemoryDb001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Memory database by GetKvStore.
     */
    const KvStoreNbDelegate::Option option = {true, true};
    g_mgr.SetKvStoreConfig(g_config);
    g_mgr.GetKvStore("distributed_CloseMemorykvstore_001", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    /**
     * @tc.steps: step2/3. Put and get to Memory database.
     * @tc.expected: step2/3. Success and the value is right.
     */
    Value readValue;
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_1, VALUE_1), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY_1, readValue), OK);
    EXPECT_EQ(readValue, VALUE_1);

    /**
     * @tc.steps: step4. Close the Memory database.
     * @tc.expected: step4. Success.
     */
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);

    /**
     * @tc.steps: step5. Reopen the Memory database.
     * @tc.expected: step5. Success.
     */
    g_mgr.GetKvStore("distributed_CloseMemorykvstore_001", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    /**
     * @tc.steps: step6. Get the key1 which has been put into the Memory database.
     * @tc.expected: step6. Return NOT_FOUND.
     */
    readValue.clear();
    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY_1, readValue), NOT_FOUND);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
}

/**
  * @tc.name: ResultSetTest001
  * @tc.desc: Test the NbDelegate for result set function.
  * @tc.type: FUNC
  * @tc.require: AR000D08KT
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesNBDelegateTest, ResultSetTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. initialize result set.
     * @tc.expected: step1. Success.
     */
    KvStoreNbDelegate::Option option = {true, false, false};
    g_mgr.GetKvStore("distributed_nb_delegate_result_set_test", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    InitResultSet();

    /**
     * @tc.steps: step2. get entries using result set.
     * @tc.expected: step2. Success.
     */
    KvStoreResultSet *readResultSet = nullptr;
    EXPECT_EQ(g_kvNbDelegatePtr->GetEntries(g_keyPrefix, readResultSet), OK);
    ASSERT_TRUE(readResultSet != nullptr);
    EXPECT_EQ(readResultSet->GetCount(), RESULT_SET_COUNT);

    /**
     * @tc.steps: step3. result function check.
     * @tc.expected: step3. Success.
     */
    CheckResultSetValue(readResultSet, NOT_FOUND, RESULT_SET_INIT_POS);
    // index from 0 to 8(first to last)
    ReadResultSet(readResultSet);
    // change index to 9(after last)
    EXPECT_TRUE(!readResultSet->MoveToNext());
    CheckResultSetValue(readResultSet, NOT_FOUND, RESULT_SET_COUNT);
    // change index to 8(last)
    EXPECT_TRUE(readResultSet->MoveToPrevious());
    CheckResultSetValue(readResultSet, OK, RESULT_SET_COUNT - 1);
    // change index to 0(first)
    EXPECT_TRUE(readResultSet->MoveToFirst());
    CheckResultSetValue(readResultSet, OK, RESULT_SET_INIT_POS + 1);
    // change index to 8(last)
    EXPECT_TRUE(readResultSet->MoveToLast());
    CheckResultSetValue(readResultSet, OK, RESULT_SET_COUNT - 1);
    // move to -4: change index to -1
    EXPECT_TRUE(!readResultSet->MoveToPosition(RESULT_SET_INIT_POS - 3));
    CheckResultSetValue(readResultSet, NOT_FOUND, RESULT_SET_INIT_POS);
    // move to 10: change index to 9
    EXPECT_TRUE(!readResultSet->MoveToPosition(RESULT_SET_COUNT + 1));
    CheckResultSetValue(readResultSet, NOT_FOUND, RESULT_SET_COUNT);
    // change index to 2
    EXPECT_TRUE(readResultSet->MoveToPosition(RESULT_SET_INIT_POS + 3));
    CheckResultSetValue(readResultSet, OK, RESULT_SET_INIT_POS + 3);
    // move 0: change index to 2
    EXPECT_TRUE(readResultSet->Move(0));
    CheckResultSetValue(readResultSet, OK, RESULT_SET_INIT_POS + 3);
    // change index to 6
    EXPECT_TRUE(readResultSet->Move(RESULT_SET_INIT_POS + 5));
    CheckResultSetValue(readResultSet, OK, RESULT_SET_INIT_POS + 7);
    // change index to 3
    EXPECT_TRUE(readResultSet->Move(RESULT_SET_INIT_POS - 2));
    CheckResultSetValue(readResultSet, OK, RESULT_SET_INIT_POS + 4);
    // move -5: change index to -1
    EXPECT_TRUE(!readResultSet->Move(-5));
    CheckResultSetValue(readResultSet, NOT_FOUND, RESULT_SET_INIT_POS);

    // move INT_MIN: change index to -1
    EXPECT_TRUE(!readResultSet->Move(INT_MIN));
    CheckResultSetValue(readResultSet, NOT_FOUND, RESULT_SET_INIT_POS);

    EXPECT_TRUE(readResultSet->Move(5));
    EXPECT_TRUE(!readResultSet->Move(INT_MAX));
    CheckResultSetValue(readResultSet, NOT_FOUND, RESULT_SET_COUNT);

    /**
     * @tc.steps: step4. clear the result set resource.
     * @tc.expected: step4. Success.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->CloseResultSet(readResultSet), OK);
    EXPECT_TRUE(readResultSet == nullptr);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_nb_delegate_result_set_test"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: PutBatchVerify001
  * @tc.desc: This test case use to verify the putBatch interface function
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesNBDelegateTest, PutBatchVerify001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Get singleVer kvStore by GetKvStore.
     * @tc.expected: step1. Get database success.
     */
    const KvStoreNbDelegate::Option option = {true, true};
    g_mgr.SetKvStoreConfig(g_config);
    g_mgr.GetKvStore("distributed_PutBatchVerify_001", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    /**
     * @tc.steps: step2. Insert 10 records into database.
     * @tc.expected: step2. Insert successfully.
     */
    vector<Entry> entries;
    for (int i = 0; i < BATCH_PRESET_SIZE_TEST; i++) {
        Entry entry;
        entry.key.push_back(i);
        entry.value.push_back(i);
        entries.push_back(entry);
    }

    EXPECT_EQ(g_kvNbDelegatePtr->PutBatch(entries), OK);

    for (int i = 0; i < BATCH_PRESET_SIZE_TEST; i++) {
        Key key;
        key.push_back(i);
        Value value;
        g_kvNbDelegatePtr->Get(key, value);
        EXPECT_EQ(key, value);
    }

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: SingleVerPutBatch001
  * @tc.desc: Check for illegal parameters
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ8
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesNBDelegateTest, SingleVerPutBatch001, TestSize.Level1)
{
    /**
     * @tc.steps: step1.
     *  Create and construct three sets of vector <Entry>, each set of three data contains records:
     *  (K1, V1) It is illegal for K1 to be greater than 1K, and V1 is 1K in size
     *  (K2, V2) K2 is legal, V2 is greater than 4M
     *  (K3, V3) are not legal.
     */
    Key illegalKey;
    DistributedDBToolsUnitTest::GetRandomKeyValue(illegalKey, DBConstant::MAX_KEY_SIZE + 1); // 1K + 1
    Value illegalValue;
    DistributedDBToolsUnitTest::GetRandomKeyValue(illegalValue, DBConstant::MAX_VALUE_SIZE + 1); // 4M + 1
    vector<Entry> entrysKeyIllegal = {KV_ENTRY_1, KV_ENTRY_2, {illegalKey, VALUE_3}};
    vector<Entry> entrysValueIllegal = {KV_ENTRY_1, KV_ENTRY_2, {KEY_3, illegalValue}};
    vector<Entry> entrysIllegal = {KV_ENTRY_1, KV_ENTRY_2, {illegalKey, illegalValue}};

    const KvStoreNbDelegate::Option option = {true, false};
    g_mgr.SetKvStoreConfig(g_config);
    g_mgr.GetKvStore("distributed_SingleVerPutBatch_001", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    /**
     * @tc.steps: step2. PutBatch operates on three sets of data.
     * @tc.expected: step2. All three operations return INVALID_ARGS.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->PutBatch(entrysKeyIllegal), INVALID_ARGS);
    EXPECT_EQ(g_kvNbDelegatePtr->PutBatch(entrysValueIllegal), INVALID_ARGS);
    EXPECT_EQ(g_kvNbDelegatePtr->PutBatch(entrysIllegal), INVALID_ARGS);

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_SingleVerPutBatch_001"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: SingleVerPutBatch002
  * @tc.desc: PutBatch normal insert function test.
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ8
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesNBDelegateTest, SingleVerPutBatch002, TestSize.Level1)
{
    const KvStoreNbDelegate::Option option = {true, false};
    g_mgr.SetKvStoreConfig(g_config);
    g_mgr.GetKvStore("distributed_SingleVerPutBatch_002", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    /**
     * @tc.steps: step1.
     *  Create and build 4 groups of vector <Entry>, which are:
     *  Vect of empty objects;
     *  Vect1 of a legal Entry record;
     *  128 legal Entry records Vect2;
     *  129 legal Entry records Vect3;
     */
    vector<Entry> entrysMaxNumber;
    for (size_t i = 0; i < DBConstant::MAX_BATCH_SIZE; i++) {
        Entry entry;
        entry.key.push_back(i);
        entry.value.push_back(i);
        entrysMaxNumber.push_back(entry);
    }
    Key keyTemp = {'1', '1'};
    Value valueTemp;
    Entry entryTemp = {keyTemp, VALUE_1};
    vector<Entry> entrysOneRecord = {entryTemp};
    vector<Entry> entrysOverSize = entrysMaxNumber;
    entrysOverSize.push_back(entryTemp);
    /**
     * @tc.steps: step2. PutBatch operates on four sets of data. and use get check the result of Vect3.
     * @tc.expected: step2. Returns INVALID_ARGS for 129 records, and returns OK for the rest. all get return NOT_FOUND.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->PutBatch(entrysOverSize), INVALID_ARGS);
    for (size_t i = 0; i < entrysOverSize.size(); i++) {
        EXPECT_EQ(g_kvNbDelegatePtr->Get(entrysOverSize[i].key, valueTemp), NOT_FOUND);
    }
    /**
     * @tc.steps: step3. Use get check the result of Vect2.
     * @tc.expected: step3. Return OK and get the correct value.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->PutBatch(entrysOneRecord), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Get(keyTemp, valueTemp), OK);
    EXPECT_EQ(valueTemp, VALUE_1);
    EXPECT_EQ(g_kvNbDelegatePtr->PutBatch(entrysMaxNumber), OK);
     /**
     * @tc.steps: step4. Use get check the result of Vect3.
     * @tc.expected: step4. Return OK and get the correct value.
     */
    for (size_t i = 0; i < entrysMaxNumber.size(); i++) {
        EXPECT_EQ(g_kvNbDelegatePtr->Get(entrysMaxNumber[i].key, valueTemp), OK);
        EXPECT_EQ(valueTemp, entrysMaxNumber[i].value);
    }

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_SingleVerPutBatch_002"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: SingleVerPutBatch003
  * @tc.desc: Check interface atomicity
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ8
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesNBDelegateTest, SingleVerPutBatch003, TestSize.Level1)
{
    const KvStoreNbDelegate::Option option = {true, false};
    g_mgr.SetKvStoreConfig(g_config);
    g_mgr.GetKvStore("distributed_SingleVerPutBatch_003", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    /**
     * @tc.steps: step1. Create and construct a set of vector <Entry> with a total of 128 data,
     * including one illegal data. And call PutBatch interface to insert.
     */
    vector<Entry> entrysMaxNumber;
    for (size_t i = 0; i < DBConstant::MAX_BATCH_SIZE; i++) {
        Entry entry;
        entry.key.push_back(i);
        entry.value.push_back(i);
        entrysMaxNumber.push_back(entry);
    }
    Key illegalKey;
    Value valueTemp;
    DistributedDBToolsUnitTest::GetRandomKeyValue(illegalKey, DBConstant::MAX_KEY_SIZE + 1); // 1K + 1
    entrysMaxNumber[0].key = illegalKey;

    EXPECT_EQ(g_kvNbDelegatePtr->PutBatch(entrysMaxNumber), INVALID_ARGS);
    /**
     * @tc.steps: step2. Use Get interface to query 128 corresponding key values.
     * @tc.expected: step2. All Get interface return NOT_FOUND.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Get(entrysMaxNumber[0].key, valueTemp), INVALID_ARGS);
    for (size_t i = 1; i < entrysMaxNumber.size(); i++) {
        EXPECT_EQ(g_kvNbDelegatePtr->Get(entrysMaxNumber[i].key, valueTemp), NOT_FOUND);
    }
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_SingleVerPutBatch_003"), OK);
    g_kvNbDelegatePtr = nullptr;
}

static void PreparePutBatch004(vector<Entry> &entrys1, vector<Entry> &entrys2, vector<Entry> &entrys3)
{
    const KvStoreNbDelegate::Option option = {true, false};
    g_mgr.SetKvStoreConfig(g_config);
    g_mgr.GetKvStore("distributed_SingleVerPutBatch_004", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    for (int i = 0; i < BATCH_PRESET_SIZE_TEST; i++) {
        Entry entry;
        entry.key.push_back(i);
        entry.value.push_back(i);
        entrys1.push_back(entry);
    }

    for (int i = 0; i < DIVIDE_BATCH_PRESET_SIZE; i++) {
        Entry entry;
        entry.key.push_back(i);
        entry.value.push_back(i + VALUE_OFFSET);
        entrys2.push_back(entry);
    }

    for (int i = DIVIDE_BATCH_PRESET_SIZE; i < BATCH_PRESET_SIZE_TEST; i++) {
        Entry entry;
        entry.key.push_back(i);
        entry.value.push_back(i - VALUE_OFFSET);
        entrys3.push_back(entry);
    }
}

/**
  * @tc.name: SingleVerPutBatch004
  * @tc.desc: Check interface data insertion and update functions.
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ8
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesNBDelegateTest, SingleVerPutBatch004, TestSize.Level1)
{
    /**
     * @tc.steps: step1.
     *  Construct three groups of three vector <Entry>:
     *  (1) entrys1: key1 ~ 10, corresponding to Value1 ~ 10;
     *  (2) entrys2: key1 ~ 5, corresponding to Value6 ~ 10;
     *  (3) entrys3: key6 ~ 10, corresponding to Value1 ~ 5;
     */
    vector<Entry> entrys1;
    vector<Entry> entrys2;
    vector<Entry> entrys3;
    PreparePutBatch004(entrys1, entrys2, entrys3);
    /**
     * @tc.steps: step2. PutBatch entrys2.
     * @tc.expected: step2. PutBatch return OK.
     */
    Value valueRead;
    EXPECT_EQ(g_kvNbDelegatePtr->PutBatch(entrys2), OK);
    /**
     * @tc.steps: step3. Check PutBatch result.
     * @tc.expected: step3. Get correct value of key1~5. Key6~10 return NOT_FOUND.
     */
    for (int i = 0; i < BATCH_PRESET_SIZE_TEST; i++) {
        Key keyTemp;
        keyTemp.push_back(i);
        if (i < DIVIDE_BATCH_PRESET_SIZE) {
            Value valueTemp;
            valueTemp.push_back(i + VALUE_OFFSET);
            EXPECT_EQ(g_kvNbDelegatePtr->Get(keyTemp, valueRead), OK);
            EXPECT_EQ(valueRead, valueTemp);
            continue;
        }
        EXPECT_EQ(g_kvNbDelegatePtr->Get(keyTemp, valueRead), NOT_FOUND);
    }
    /**
     * @tc.steps: step4. PutBatch entrys1.
     * @tc.expected: step4. PutBatch return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->PutBatch(entrys1), OK);
    /**
     * @tc.steps: step5. Check PutBatch result.
     * @tc.expected: step5. Update and insert value of key1~10 to value1~10.
     */
    for (int i = 0; i < BATCH_PRESET_SIZE_TEST; i++) {
        Key keyTemp;
        keyTemp.push_back(i);
        if (i < DIVIDE_BATCH_PRESET_SIZE) {
            EXPECT_EQ(g_kvNbDelegatePtr->Get(keyTemp, valueRead), OK);
            EXPECT_EQ(valueRead, keyTemp);
            continue;
        }
        EXPECT_EQ(g_kvNbDelegatePtr->Get(keyTemp, valueRead), OK);
        EXPECT_EQ(valueRead, keyTemp);
    }
    /**
     * @tc.steps: step6. PutBatch entrys3.
     * @tc.expected: step6. PutBatch return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->PutBatch(entrys3), OK);
    /**
     * @tc.steps: step7. Check PutBatch result of key1~10.
     * @tc.expected: step7. Update value of key5~10 to value1~5.
     */
    for (int i = 0; i < BATCH_PRESET_SIZE_TEST; i++) {
        Key keyTemp;
        keyTemp.push_back(i);
        if (i < DIVIDE_BATCH_PRESET_SIZE) {
            EXPECT_EQ(g_kvNbDelegatePtr->Get(keyTemp, valueRead), OK);
            EXPECT_EQ(valueRead, keyTemp);
            continue;
        }
        Value valueTemp;
        valueTemp.push_back(i - VALUE_OFFSET);
        EXPECT_EQ(g_kvNbDelegatePtr->Get(keyTemp, valueRead), OK);
        EXPECT_EQ(valueRead, valueTemp);
    }

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_SingleVerPutBatch_004"), OK);
    g_kvNbDelegatePtr = nullptr;
}

static void CreatEntrys(int recordSize, vector<Key> &keys, vector<Value> &values, vector<Entry> &entries)
{
    keys.clear();
    values.clear();
    entries.clear();
    for (int i = 0; i < recordSize; i++) {
        string temp = to_string(i);
        Entry entry;
        Key keyTemp;
        Value valueTemp;
        for (auto &iter : temp) {
            entry.key.push_back(iter);
            entry.value.push_back(iter);
            keyTemp.push_back(iter);
            valueTemp.push_back(iter);
        }
        keys.push_back(keyTemp);
        values.push_back(valueTemp);
        entries.push_back(entry);
    }
}

/**
  * @tc.name: SingleVerDeleteBatch001
  * @tc.desc: Check for illegal parameters.
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ8
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesNBDelegateTest, SingleVerDeleteBatch001, TestSize.Level1)
{
    const KvStoreNbDelegate::Option option = {true, false};
    g_mgr.SetKvStoreConfig(g_config);
    g_mgr.GetKvStore("distributed_SingleVerPutBatch_001", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    /**
     * @tc.steps: step1. Create and construct a set of vector <Entry>, containing a total of 10 data keys1 ~ 10,
     *  Value1 ~ 10, and call Putbatch interface to insert data.
     * @tc.expected: step1. PutBatch successfully.
     */
    vector<Entry> entries;
    vector<Key> keys;
    vector<Value> values;
    Value valueRead;
    CreatEntrys(BATCH_PRESET_SIZE_TEST, keys, values, entries);
    vector<Entry> entrysBase = entries;
    vector<Key> keysBase = keys;
    EXPECT_EQ(g_kvNbDelegatePtr->PutBatch(entrysBase), OK);
    /**
     * @tc.steps: step2. Use Get to check data in database.
     * @tc.expected: step2. Get value1~10 by key1~10 successfully.
     */
    for (size_t i = 0; i < BATCH_PRESET_SIZE_TEST; i++) {
        EXPECT_EQ(g_kvNbDelegatePtr->Get(entrysBase[i].key, valueRead), OK);
    }
    /**
     * @tc.steps: step3. Use DeleteBatch interface to transfer 10 + 119 extra keys (total 129).
     * @tc.expected: step3. Return INVALID_ARGS.
     */
    CreatEntrys(DBConstant::MAX_BATCH_SIZE + 1, keys, values, entries);
    EXPECT_EQ(g_kvNbDelegatePtr->DeleteBatch(keys), INVALID_ARGS);
    /**
     * @tc.steps: step4. Use Get to check data in database.
     * @tc.expected: step4. Key1~10 still in database.
     */
    for (size_t i = 0; i < BATCH_PRESET_SIZE_TEST; i++) {
        EXPECT_EQ(g_kvNbDelegatePtr->Get(entrysBase[i].key, valueRead), OK);
    }
    /**
     * @tc.steps: step5. Use the DeleteBatch interface to pass in 10 included
     *  keys6 ~ 10 + 123 additional key values ​​(128 in total).
     * @tc.expected: step5. DeleteBatch OK.
     */
    CreatEntrys(DBConstant::MAX_BATCH_SIZE + DIVIDE_BATCH_PRESET_SIZE, keys, values, entries);
    keys.erase(keys.begin(), keys.begin() + DIVIDE_BATCH_PRESET_SIZE);
    EXPECT_EQ(g_kvNbDelegatePtr->DeleteBatch(keys), OK);
    /**
     * @tc.steps: step6. Use Get to check key1~10 in database.
     * @tc.expected: step6. Key1~5 in database, key6~10 have been deleted.
     */
    for (size_t i = 0; i < DIVIDE_BATCH_PRESET_SIZE; i++) {
        EXPECT_EQ(g_kvNbDelegatePtr->Get(entrysBase[i].key, valueRead), OK);
    }
    for (size_t i = DIVIDE_BATCH_PRESET_SIZE; i < BATCH_PRESET_SIZE_TEST; i++) {
        EXPECT_EQ(g_kvNbDelegatePtr->Get(entrysBase[i].key, valueRead), NOT_FOUND);
    }
    /**
     * @tc.steps: step7. Repeat Putbatch key1~10, value1~10.
     * @tc.expected: step7. Return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->PutBatch(entrysBase), OK);

    Key illegalKey;
    DistributedDBToolsUnitTest::GetRandomKeyValue(illegalKey, DBConstant::MAX_KEY_SIZE + 1); // 1K + 1
    keysBase.push_back(illegalKey);
    /**
     * @tc.steps: step8. Use DeleteBatch interface to pass in 10 + 1(larger than 1K) keys.
     * @tc.expected: step8. Return INVALID_ARGS.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->DeleteBatch(keysBase), INVALID_ARGS);
    /**
     * @tc.steps: step9. Use Get to check key1~10 in database.
     * @tc.expected: step9. Delete those data failed.
     */
    for (size_t i = 0; i < BATCH_PRESET_SIZE_TEST; i++) {
        EXPECT_EQ(g_kvNbDelegatePtr->Get(entrysBase[i].key, valueRead), OK);
    }
    /**
     * @tc.steps: step10. Use DeleteBatch interface to pass in 10(in database) + 1 valid keys.
     * @tc.expected: step10. Delete those data successfully.
     */
    keysBase.back().erase(keysBase.back().begin(), keysBase.back().begin() + 1);
    EXPECT_EQ(g_kvNbDelegatePtr->DeleteBatch(keysBase), OK);
    /**
     * @tc.steps: step11. Check data.
     * @tc.expected: step11. DeleteBatch successfully.
     */
    for (size_t i = 0; i < BATCH_PRESET_SIZE_TEST; i++) {
        EXPECT_EQ(g_kvNbDelegatePtr->Get(entrysBase[i].key, valueRead), NOT_FOUND);
    }

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_SingleVerPutBatch_001"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: SingleVerDeleteBatch002
  * @tc.desc: Check normal delete batch ability.
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ8
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesNBDelegateTest, SingleVerDeleteBatch002, TestSize.Level1)
{
    const KvStoreNbDelegate::Option option = {true, false};
    g_mgr.SetKvStoreConfig(g_config);
    g_mgr.GetKvStore("distributed_SingleVerPutBatch_002", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    /**
     * @tc.steps: step1. Create a group of vector <Entry>, containing a total of 10 data keys1 ~ 10, Value1 ~ 10,
     *  call the Putbatch interface to insert data.
     * @tc.expected: step1. Insert to database successfully.
     */
    vector<Entry> entries;
    vector<Key> keysBase;
    vector<Value> values;
    CreatEntrys(BATCH_PRESET_SIZE_TEST, keysBase, values, entries);

    EXPECT_EQ(g_kvNbDelegatePtr->PutBatch(entries), OK);
    /**
     * @tc.steps: step2. Check data.
     * @tc.expected: step2. Get key1~10 successfully.
     */
    Value valueRead;
    for (size_t i = 0; i < BATCH_PRESET_SIZE_TEST; i++) {
        EXPECT_EQ(g_kvNbDelegatePtr->Get(keysBase[i], valueRead), OK);
    }
    /**
     * @tc.steps: step3. DeleteBatch key1~5.
     * @tc.expected: step3. Return OK.
     */
    vector<Key> keys(keysBase.begin(), keysBase.begin() + DIVIDE_BATCH_PRESET_SIZE);
    EXPECT_EQ(g_kvNbDelegatePtr->DeleteBatch(keys), OK);
    /**
     * @tc.steps: step4. Check key1~10.
     * @tc.expected: step4. Key1~5 deleted, key6~10 existed.
     */
    for (size_t i = 0; i < DIVIDE_BATCH_PRESET_SIZE; i++) {
        EXPECT_EQ(g_kvNbDelegatePtr->Get(keysBase[i], valueRead), NOT_FOUND);
    }
    for (size_t i = DIVIDE_BATCH_PRESET_SIZE; i < BATCH_PRESET_SIZE_TEST; i++) {
        EXPECT_EQ(g_kvNbDelegatePtr->Get(keysBase[i], valueRead), OK);
    }
    /**
     * @tc.steps: step5. DeleteBatch key1~10.
     * @tc.expected: step5. Return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->DeleteBatch(keysBase), OK);
    /**
     * @tc.steps: step6. Check key1~10.
     * @tc.expected: step6. Key1~10 deleted successfully.
     */
    for (size_t i = 0; i < BATCH_PRESET_SIZE_TEST; i++) {
        EXPECT_EQ(g_kvNbDelegatePtr->Get(keysBase[i], valueRead), NOT_FOUND);
    }
    /**
     * @tc.steps: step7. DeleteBatch key1~10 once again.
     * @tc.expected: step7. Return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->DeleteBatch(keysBase), OK);

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_SingleVerPutBatch_002"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: SingleVerPutBatchObserver001
  * @tc.desc: Test the observer function of PutBatch() interface.
  * @tc.type: FUNC
  * @tc.require: AR000DPTTA
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesNBDelegateTest, SingleVerPutBatchObserver001, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Get the nb delegate.
     * @tc.expected: step1. Get results OK and non-null delegate.
     */
    KvStoreNbDelegate::Option option = {true, false, false};
    g_mgr.GetKvStore("distributed_SingleVerPutBatchObserver_001", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    KvStoreObserverUnitTest *observer = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(observer != nullptr);
    /**
     * @tc.steps:step2. Register the non-null observer for the special key.
     * @tc.expected: step2. Register results OK.
     */
    Key key;
    EXPECT_EQ(g_kvNbDelegatePtr->RegisterObserver(key, OBSERVER_CHANGES_NATIVE, observer), OK);
    /**
     * @tc.steps:step3. Put batch data.
     * @tc.expected: step3. Returns OK.
     */
    vector<Entry> entrysBase;
    vector<Key> keysBase;
    DistributedDBUnitTest::GenerateRecords(BATCH_PRESET_SIZE_TEST + 1, entrysBase, keysBase);

    vector<Entry> entries(entrysBase.begin(), entrysBase.end() - 1);
    EXPECT_EQ(entries.size(), 10UL);
    EXPECT_EQ(g_kvNbDelegatePtr->PutBatch(entries), OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_TRUE(DistributedDBToolsUnitTest::CheckObserverResult(entries, observer->GetEntriesInserted()));
    /**
     * @tc.steps:step4. Delete the batch data.
     * @tc.expected: step4. Returns OK.
     */
    vector<Key> keys(keysBase.begin() + 5, keysBase.end());
    EXPECT_EQ(keys.size(), 6UL);
    EXPECT_EQ(g_kvNbDelegatePtr->DeleteBatch(keys), OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    vector<Entry> entrysDel(entrysBase.begin() + 5, entrysBase.end() - 1);
    EXPECT_EQ(entrysDel.size(), 5UL);
    EXPECT_TRUE(DistributedDBToolsUnitTest::CheckObserverResult(entrysDel, observer->GetEntriesDeleted()));
    /**
     * @tc.steps:step5. UnRegister the observer.
     * @tc.expected: step5. Returns OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->UnRegisterObserver(observer), OK);
    delete observer;
    observer = nullptr;
    /**
     * @tc.steps:step6. Close the kv store.
     * @tc.expected: step6. Results OK and delete successfully.
     */
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_SingleVerPutBatchObserver_001"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: SingleVerPutBatchObserver002
  * @tc.desc: Test the observer function of PutBatch() for invalid input.
  * @tc.type: FUNC
  * @tc.require: AR000DPTTA
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesNBDelegateTest, SingleVerPutBatchObserver002, TestSize.Level4)
{
    /**
     * @tc.steps:step1. Get the nb delegate.
     * @tc.expected: step1. Get results OK and non-null delegate.
     */
    KvStoreNbDelegate::Option option = {true, false, false};
    g_mgr.GetKvStore("distributed_SingleVerPutBatchObserver_002", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    KvStoreObserverUnitTest *observer = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(observer != nullptr);
    /**
     * @tc.steps:step2. Register the non-null observer for the special key.
     * @tc.expected: step2. Register results OK.
     */
    Key key;
    EXPECT_EQ(g_kvNbDelegatePtr->RegisterObserver(key, OBSERVER_CHANGES_NATIVE, observer), OK);
    /**
     * @tc.steps:step3. Put 129 batch data.
     * @tc.expected: step3. Returns INVALID_ARGS.
     */
    vector<Entry> entrys1;
    vector<Key> keys1;
    DistributedDBUnitTest::GenerateRecords(DBConstant::MAX_BATCH_SIZE + 1, entrys1, keys1);

    EXPECT_EQ(entrys1.size(), 129UL);
    EXPECT_EQ(g_kvNbDelegatePtr->PutBatch(entrys1), INVALID_ARGS);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_TRUE(observer->GetEntriesInserted().empty());
    /**
     * @tc.steps:step4. Put invalid batch data.
     * @tc.expected: step4. Returns INVALID_ARGS.
     */
    vector<Entry> entrys2;
    vector<Key> keys2;
    DistributedDBUnitTest::GenerateRecords(BATCH_PRESET_SIZE_TEST, entrys2, keys2);
    EXPECT_EQ(entrys2.size(), 10UL);

    vector<Entry> entrysInvalid;
    vector<Key> keysInvalid;
    DistributedDBUnitTest::GenerateRecords(BATCH_PRESET_SIZE_TEST, entrysInvalid, keysInvalid,
        DBConstant::MAX_KEY_SIZE + 10);
    EXPECT_EQ(entrysInvalid.size(), 10UL);
    entrys2[0].key = entrysInvalid[0].key;

    EXPECT_EQ(g_kvNbDelegatePtr->PutBatch(entrys2), INVALID_ARGS);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_TRUE(observer->GetEntriesInserted().empty());
    /**
     * @tc.steps:step5. Put MAX valid value batch data.
     * @tc.expected: step5. Returns OK.
     */
    vector<Entry> entrys3;
    vector<Key> keys3;

    DistributedDBUnitTest::GenerateRecords(DBConstant::MAX_BATCH_SIZE, entrys3, keys3);
    EXPECT_EQ(g_kvNbDelegatePtr->PutBatch(entrys3), OK);
    LOGD("sleep begin");
    // sleep 20 seconds
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME * 10));
    LOGD("sleep end");
    EXPECT_TRUE(DistributedDBToolsUnitTest::CheckObserverResult(entrys3, observer->GetEntriesInserted()));
    /**
     * @tc.steps:step6. UnRegister the observer.
     * @tc.expected: step6. Returns OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->UnRegisterObserver(observer), OK);
    delete observer;
    observer = nullptr;
    /**
     * @tc.steps:step7. Close the kv store.
     * @tc.expected: step7. Results OK and delete successfully.
     */
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_SingleVerPutBatchObserver_002"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: SingleVerPutBatchObserver003
  * @tc.desc: Test the observer function of PutBatch() update function.
  * @tc.type: FUNC
  * @tc.require: AR000DPTTA
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesNBDelegateTest, SingleVerPutBatchObserver003, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Get the nb delegate.
     * @tc.expected: step1. Get results OK and non-null delegate.
     */
    KvStoreNbDelegate::Option option = {true, false, false};
    g_mgr.GetKvStore("distributed_SingleVerPutBatchObserver_003", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    KvStoreObserverUnitTest *observer = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(observer != nullptr);
    /**
     * @tc.steps:step2. Register the non-null observer for the special key.
     * @tc.expected: step2. Register results OK.
     */
    Key key;
    EXPECT_EQ(g_kvNbDelegatePtr->RegisterObserver(key, OBSERVER_CHANGES_NATIVE, observer), OK);
    /**
     * @tc.steps:step3. Put batch data.
     * @tc.expected: step3. Returns OK.
     */
    vector<Entry> entrysAdd;
    vector<Key> keysAdd;
    DistributedDBUnitTest::GenerateRecords(BATCH_PRESET_SIZE_TEST, entrysAdd, keysAdd);

    EXPECT_EQ(entrysAdd.size(), 10UL);
    EXPECT_EQ(g_kvNbDelegatePtr->PutBatch(entrysAdd), OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_TRUE(DistributedDBToolsUnitTest::CheckObserverResult(entrysAdd, observer->GetEntriesInserted()));
    /**
     * @tc.steps:step4. Update the batch data.
     * @tc.expected: step4. Returns OK.
     */
    vector<Entry> entrysUpdate;
    vector<Key> keysUpdate;
    DistributedDBUnitTest::GenerateRecords(BATCH_PRESET_SIZE_TEST, entrysUpdate, keysUpdate, DEFAULT_KEY_VALUE_SIZE,
        DEFAULT_KEY_VALUE_SIZE + 10);

    EXPECT_EQ(entrysUpdate.size(), 10UL);
    EXPECT_EQ(g_kvNbDelegatePtr->PutBatch(entrysUpdate), OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_TRUE(DistributedDBToolsUnitTest::CheckObserverResult(entrysUpdate, observer->GetEntriesUpdated()));
    /**
     * @tc.steps:step5. UnRegister the observer.
     * @tc.expected: step5. Returns OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->UnRegisterObserver(observer), OK);
    delete observer;
    observer = nullptr;
    /**
     * @tc.steps:step6. Close the kv store.
     * @tc.expected: step6. Results OK and delete successfully.
     */
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_SingleVerPutBatchObserver_003"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: SingleVerPutBatchObserver004
  * @tc.desc: Test the observer function of PutBatch(), same keys handle.
  * @tc.type: FUNC
  * @tc.require: AR000DPTTA
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesNBDelegateTest, SingleVerPutBatchObserver004, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Get the nb delegate.
     * @tc.expected: step1. Get results OK and non-null delegate.
     */
    KvStoreNbDelegate::Option option = {true, false, false};
    g_mgr.GetKvStore("distributed_SingleVerPutBatchObserver_004", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    KvStoreObserverUnitTest *observer = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(observer != nullptr);
    /**
     * @tc.steps:step2. Register the non-null observer for the special key.
     * @tc.expected: step2. Register results OK.
     */
    Key key;
    EXPECT_EQ(g_kvNbDelegatePtr->RegisterObserver(key, OBSERVER_CHANGES_NATIVE, observer), OK);
    /**
     * @tc.steps:step3. Put batch data.
     * @tc.expected: step3. Returns OK.
     */
    vector<Entry> entrys1;
    vector<Key> keys1;
    DistributedDBUnitTest::GenerateRecords(BATCH_PRESET_SIZE_TEST, entrys1, keys1);
    vector<Entry> entrys2;
    vector<Key> keys2;
    DistributedDBUnitTest::GenerateRecords(BATCH_PRESET_SIZE_TEST, entrys2, keys2, DEFAULT_KEY_VALUE_SIZE,
        DEFAULT_KEY_VALUE_SIZE + 10);
    entrys1.insert(entrys1.end(), entrys2.begin(), entrys2.end());

    EXPECT_EQ(entrys1.size(), 20UL);
    EXPECT_EQ(g_kvNbDelegatePtr->PutBatch(entrys1), OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_TRUE(DistributedDBToolsUnitTest::CheckObserverResult(entrys2, observer->GetEntriesInserted()));
    EXPECT_EQ(observer->GetEntriesUpdated().size(), 0UL);

    vector<Entry> entrys3;
    vector<Key> keys3;
    DistributedDBUnitTest::GenerateRecords(BATCH_PRESET_SIZE_TEST, entrys3, keys3, DEFAULT_KEY_VALUE_SIZE,
        DEFAULT_KEY_VALUE_SIZE + 20);
    vector<Entry> entrys4;
    vector<Key> keys4;
    DistributedDBUnitTest::GenerateRecords(BATCH_PRESET_SIZE_TEST, entrys4, keys4, DEFAULT_KEY_VALUE_SIZE,
        DEFAULT_KEY_VALUE_SIZE + 30);
    entrys3.insert(entrys3.end(), entrys4.begin(), entrys4.end());
    EXPECT_EQ(g_kvNbDelegatePtr->PutBatch(entrys3), OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_TRUE(DistributedDBToolsUnitTest::CheckObserverResult(entrys4, observer->GetEntriesUpdated()));
    EXPECT_EQ(observer->GetEntriesInserted().size(), 0UL);

    /**
     * @tc.steps:step4. UnRegister the observer.
     * @tc.expected: step4. Returns OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->UnRegisterObserver(observer), OK);
    delete observer;
    observer = nullptr;
    /**
     * @tc.steps:step5. Close the kv store.
     * @tc.expected: step5. Results OK and delete successfully.
     */
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_SingleVerPutBatchObserver_004"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: SingleVerDeleteBatchObserver001
  * @tc.desc: Test the observer function of DeleteBatch() interface.
  * @tc.type: FUNC
  * @tc.require: AR000DPTTA
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesNBDelegateTest, SingleVerDeleteBatchObserver001, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Get the nb delegate.
     * @tc.expected: step1. Get results OK and non-null delegate.
     */
    KvStoreNbDelegate::Option option = {true, false, false};
    g_mgr.GetKvStore("distributed_SingleVerDeleteBatchObserver_001", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    KvStoreObserverUnitTest *observer = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(observer != nullptr);
    /**
     * @tc.steps:step2. Register the non-null observer for the special key.
     * @tc.expected: step2. Register results OK.
     */
    Key key;
    EXPECT_EQ(g_kvNbDelegatePtr->RegisterObserver(key, OBSERVER_CHANGES_NATIVE, observer), OK);
    /**
     * @tc.steps:step3. Put batch data.
     * @tc.expected: step3. Returns OK.
     */
    vector<Entry> entries;
    vector<Key> keys;
    DistributedDBUnitTest::GenerateRecords(BATCH_PRESET_SIZE_TEST, entries, keys);
    EXPECT_EQ(entries.size(), 10UL);

    EXPECT_EQ(g_kvNbDelegatePtr->PutBatch(entries), OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_TRUE(DistributedDBToolsUnitTest::CheckObserverResult(entries, observer->GetEntriesInserted()));
    /**
     * @tc.steps:step4. Delete the batch data.
     * @tc.expected: step4. Returns OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->DeleteBatch(keys), OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_TRUE(DistributedDBToolsUnitTest::CheckObserverResult(entries, observer->GetEntriesDeleted()));
    /**
     * @tc.steps:step5. UnRegister the observer.
     * @tc.expected: step5. Returns OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->UnRegisterObserver(observer), OK);
    delete observer;
    observer = nullptr;
    /**
     * @tc.steps:step6. Close the kv store.
     * @tc.expected: step6. Results OK and delete successfully.
     */
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_SingleVerDeleteBatchObserver_001"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: SingleVerConcurrentPut001
  * @tc.desc: Test put the data concurrently, and check the timestamp.
  * @tc.type: FUNC
  * @tc.require: AR000DPTTA
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBInterfacesNBDelegateTest, SingleVerConcurrentPut001, TestSize.Level4)
{
    /**
     * @tc.steps:step1. Get the nb delegate.
     * @tc.expected: step1. Get results OK and non-null delegate.
     */
    KvStoreNbDelegate::Option option = {true, false, false};
    g_mgr.GetKvStore("concurrentPutTest", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    for (size_t i = 0; i < CON_PUT_THREAD_NUM * PER_THREAD_PUT_NUM; i++) {
        Entry entry;
        DistributedDBToolsUnitTest::GetRandomKeyValue(entry.key, DEFAULT_KEY_VALUE_SIZE);
        DistributedDBToolsUnitTest::GetRandomKeyValue(entry.value);
        g_entriesForConcurrency.push_back(std::move(entry));
    }

    /**
     * @tc.steps:step2. Put data concurrently in 4 threads.
     * @tc.expected: step2. Put OK, and the timestamp order is same with the rowid.
     */
    std::thread thread1(std::bind(PutData, g_kvNbDelegatePtr, 0)); // 0th thread.
    std::thread thread2(std::bind(PutData, g_kvNbDelegatePtr, 1)); // 1th thread.
    std::thread thread3(std::bind(PutData, g_kvNbDelegatePtr, 2)); // 2th thread.
    std::thread thread4(std::bind(PutData, g_kvNbDelegatePtr, 3)); // 3th thread.

    thread1.join();
    thread2.join();
    thread3.join();
    thread4.join();

    EXPECT_EQ(CheckDataTimestamp("concurrentPutTest"), true);

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("concurrentPutTest"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: SingleVerGetLocalEntries001
  * @tc.desc: Test GetLocalEntries interface for the single ver database.
  * @tc.type: FUNC
  * @tc.require: AR000DPTTA
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBInterfacesNBDelegateTest, SingleVerGetLocalEntries001, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Get the nb delegate.
     * @tc.expected: step1. Get results OK and non-null delegate.
     */
    KvStoreNbDelegate::Option option = {true, false, false};
    g_mgr.GetKvStore("concurrentPutTest", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    /**
     * @tc.steps:step2. Put one data whose key has prefix 'p' into the local zone.
     */
    Entry entry1 = {{'p'}, {'q'}};
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocal(entry1.key, entry1.value), OK);

    /**
     * @tc.steps:step3. Get batch data whose key has prefix 'k' from the local zone.
     * @tc.expected: step3. Get results NOT_FOUND.
     */
    std::vector<Entry> entries;
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocalEntries({'k'}, entries), NOT_FOUND);

    /**
     * @tc.steps:step4. Put two data whose key have prefix 'k' into the local zone.
     */
    Entry entry2 = {{'k', '1'}, {'d'}};
    Entry entry3 = {{'k', '2'}, {'d'}};
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocal(entry2.key, entry2.value), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocal(entry3.key, entry3.value), OK);

    /**
     * @tc.steps:step5. Get batch data whose key has prefix 'k' from the local zone.
     * @tc.expected: step5. Get results OK, and the entries size is 2.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocalEntries({'k'}, entries), OK);
    EXPECT_EQ(entries.size(), 2UL);

    /**
     * @tc.steps:step6. Get batch data whose key has empty prefix from the local zone.
     * @tc.expected: step6. Get results OK, and the entries size is 3.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocalEntries({}, entries), OK);
    EXPECT_EQ(entries.size(), 3UL);

    /**
     * @tc.steps:step7. Delete one data whose key has prefix 'k' from the local zone.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->DeleteLocal(entry3.key), OK);

    /**
     * @tc.steps:step8. Get batch data whose key has prefix 'k' from the local zone.
     * @tc.expected: step8. Get results OK, and the entries size is 1.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocalEntries({'k'}, entries), OK);
    EXPECT_EQ(entries.size(), 1UL);

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("concurrentPutTest"), OK);
    g_kvNbDelegatePtr = nullptr;
}

static vector<Entry> PreDataForQueryByPreFixKey()
{
    vector<Entry> res;
    for (int i = 0; i < 5; i++) { // Random 5 for test
        Key key = DistributedDBToolsUnitTest::GetRandPrefixKey({'a', 'b'}, 1024);
        std::string validData = "{\"field_name1\":null, \"field_name2\":" + std::to_string(rand()) + "}";
        Value value(validData.begin(), validData.end());
        res.push_back({key, value});
    }

    for (int i = 0; i < 5; i++) { // Random 5 for test
        Key key = DistributedDBToolsUnitTest::GetRandPrefixKey({'a', 'c'}, 1024);
        std::string validData = "{\"field_name1\":null, \"field_name2\":" + std::to_string(rand()) + "}";
        Value value(validData.begin(), validData.end());
        res.push_back({key, value});
    }
    return res;
}

/**
  * @tc.name: QueryPreFixKey002
  * @tc.desc: The query method without filtering the field can query non-schma databases
  * @tc.type: FUNC
  * @tc.require: AR000EPARK
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesNBDelegateTest, QueryPreFixKey002, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Create non-schma databases
     */
    KvStoreNbDelegate::Option option = {true, false, false};
    g_mgr.GetKvStore("QueryPreFixKey002", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    vector<Entry> entries = PreDataForQueryByPreFixKey();
    EXPECT_EQ(g_kvNbDelegatePtr->PutBatch(entries), OK);

    /**
     * @tc.steps:step2. Get query object with prefixkey limit combination.
     * @tc.expected: step2. Get results OK, and the entries size right.
     */
    Query query = Query::Select().PrefixKey({'a', 'c'});
    std::vector<Entry> entriesRes;
    int errCode = g_kvNbDelegatePtr->GetEntries(query, entriesRes);
    EXPECT_EQ(errCode, OK);
    EXPECT_EQ(entriesRes.size(), 5ul);
    for (size_t i = 0; i < entriesRes.size(); i++) {
        EXPECT_EQ(entriesRes[i].key.front(), 'a');
        EXPECT_EQ(entriesRes[i].key[1], 'c');
    }
    int count = -1;
    g_kvNbDelegatePtr->GetCount(query, count);
    EXPECT_EQ(count, 5);

    Query query1 = Query::Select().PrefixKey({}).Limit(4, 0);
    errCode = g_kvNbDelegatePtr->GetEntries(query1, entriesRes);
    EXPECT_EQ(errCode, OK);
    EXPECT_EQ(entriesRes.size(), 4ul);

    Query query2 = Query::Select().PrefixKey(Key(1025, 'a'));
    errCode = g_kvNbDelegatePtr->GetEntries(query2, entriesRes);
    EXPECT_EQ(errCode, INVALID_ARGS);

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_TRUE(g_mgr.DeleteKvStore("QueryPreFixKey002") == OK);
}

/**
  * @tc.name: SingleVerGetSecurityOption001
  * @tc.desc: Test GetSecurityOption interface for the single ver database.
  * @tc.type: FUNC
  * @tc.require: AR000EV1G2
  * @tc.author: liuwenkai
  */
HWTEST_F(DistributedDBInterfacesNBDelegateTest, SingleVerGetSecurityOption001, TestSize.Level1)
{
    SecurityOption savedOption;
    std::shared_ptr<IProcessSystemApiAdapter> adapter = std::make_shared<ProcessSystemApiAdapterImpl>();
    EXPECT_TRUE(adapter);
    RuntimeContext::GetInstance()->SetProcessSystemApiAdapter(adapter);
    KvStoreNbDelegate::Option option = {true, false, false};

    /**
     * @tc.steps:step1. Create databases without securityOption.
     * @tc.expected: step2. Returns a non-null kvstore but can not get SecurityOption.
     */
    g_mgr.GetKvStore("SingleVerGetSecurityOption001", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_TRUE(g_kvNbDelegatePtr->GetSecurityOption(savedOption) == OK);
    EXPECT_TRUE(savedOption.securityLabel == 0);
    EXPECT_TRUE(savedOption.securityFlag == 0);
    KvStoreNbDelegate *kvNbDelegatePtr1 = g_kvNbDelegatePtr;

    /**
     * @tc.steps:step2. Create databases with new securityOption(Check ignore the new option).
     * @tc.expected: step2. Returns non-null kvstore.
     */
    option.secOption.securityLabel = S3;
    option.secOption.securityFlag = 1;
    g_mgr.GetKvStore("SingleVerGetSecurityOption001", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_TRUE(g_kvNbDelegatePtr->GetSecurityOption(savedOption) == OK);
    EXPECT_TRUE(savedOption.securityLabel == 0);
    EXPECT_TRUE(savedOption.securityFlag == 0);

    EXPECT_EQ(g_mgr.CloseKvStore(kvNbDelegatePtr1), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    g_kvNbDelegatePtr = nullptr;
    EXPECT_TRUE(g_mgr.DeleteKvStore("SingleVerGetSecurityOption001") == OK);
}

/**
  * @tc.name: SingleVerGetSecurityOption002
  * @tc.desc: Test GetSecurityOption interface for the single ver database.
  * @tc.type: FUNC
  * @tc.require: AR000EV1G2
  * @tc.author: liuwenkai
  */
HWTEST_F(DistributedDBInterfacesNBDelegateTest, SingleVerGetSecurityOption002, TestSize.Level1)
{
    SecurityOption savedOption;
    std::shared_ptr<IProcessSystemApiAdapter> adapter = std::make_shared<ProcessSystemApiAdapterImpl>();
    EXPECT_TRUE(adapter != nullptr);
    RuntimeContext::GetInstance()->SetProcessSystemApiAdapter(adapter);
    KvStoreNbDelegate::Option option = {true, false, false};

    /**
     * @tc.steps:step1. Create databases with securityOption.
     * @tc.expected: step2. Returns a non-null kvstore and get right SecurityOption.
     */
    option.secOption.securityLabel = S3;
    option.secOption.securityFlag = 1;
    g_mgr.GetKvStore("SingleVerGetSecurityOption002", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_TRUE(g_kvNbDelegatePtr->GetSecurityOption(savedOption) == OK);
    EXPECT_TRUE(savedOption.securityLabel == S3);
    EXPECT_TRUE(savedOption.securityFlag == 1);
    KvStoreNbDelegate *kvNbDelegatePtr1 = g_kvNbDelegatePtr;

    /**
     * @tc.steps:step2. Create databases without securityOption.
     * @tc.expected: step2. Returns a non-null kvstore and get right SecurityOption.
     */
    option.secOption.securityLabel = 0;
    option.secOption.securityFlag = 0;
    g_mgr.GetKvStore("SingleVerGetSecurityOption002", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_TRUE(g_kvNbDelegatePtr->GetSecurityOption(savedOption) == OK);
    EXPECT_TRUE(savedOption.securityLabel == S3);
    EXPECT_TRUE(savedOption.securityFlag == 1);

    EXPECT_EQ(g_mgr.CloseKvStore(kvNbDelegatePtr1), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    g_kvNbDelegatePtr = nullptr;
    EXPECT_TRUE(g_mgr.DeleteKvStore("SingleVerGetSecurityOption002") == OK);
}

/**
 * @tc.name: MaxLogSize001
 * @tc.desc: Test the pragma cmd of the max log size limit.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: wangbingquan
 */
HWTEST_F(DistributedDBInterfacesNBDelegateTest, MaxLogSize001, TestSize.Level2)
{
    /**
     * @tc.steps:step1. Create database.
     * @tc.expected: step1. Returns a non-null kvstore.
     */
    KvStoreNbDelegate::Option option;
    g_mgr.GetKvStore("MaxLogSize001", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    /**
     * @tc.steps:step2. Setting the max log limit for the valid value.
     * @tc.expected: step2. Returns OK.
     */
    uint64_t logSize = DBConstant::MAX_LOG_SIZE_HIGH;
    PragmaData pragLimit = static_cast<PragmaData>(&logSize);
    EXPECT_EQ(g_kvNbDelegatePtr->Pragma(SET_MAX_LOG_LIMIT, pragLimit), OK);

    logSize = DBConstant::MAX_LOG_SIZE_LOW;
    pragLimit = static_cast<PragmaData>(&logSize);
    EXPECT_EQ(g_kvNbDelegatePtr->Pragma(SET_MAX_LOG_LIMIT, pragLimit), OK);

    logSize = 10 * 1024 * 1024; // 10M
    pragLimit = static_cast<PragmaData>(&logSize);
    EXPECT_EQ(g_kvNbDelegatePtr->Pragma(SET_MAX_LOG_LIMIT, pragLimit), OK);

    /**
     * @tc.steps:step3. Setting the max log limit for the invalid value.
     * @tc.expected: step3. Returns INLIVAD_ARGS.
     */
    logSize = DBConstant::MAX_LOG_SIZE_HIGH + 1;
    pragLimit = static_cast<PragmaData>(&logSize);
    EXPECT_EQ(g_kvNbDelegatePtr->Pragma(SET_MAX_LOG_LIMIT, pragLimit), INVALID_ARGS);

    logSize = DBConstant::MAX_LOG_SIZE_LOW - 1;
    pragLimit = static_cast<PragmaData>(&logSize);
    EXPECT_EQ(g_kvNbDelegatePtr->Pragma(SET_MAX_LOG_LIMIT, pragLimit), INVALID_ARGS);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    g_kvNbDelegatePtr = nullptr;
    EXPECT_TRUE(g_mgr.DeleteKvStore("MaxLogSize001") == OK);
}

/**
 * @tc.name: ForceCheckpoint002
 * @tc.desc: Test the checkpoint of the database.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: wangbingquan
 */
HWTEST_F(DistributedDBInterfacesNBDelegateTest, MaxLogSize002, TestSize.Level2)
{
    /**
     * @tc.steps:step1. Create database.
     * @tc.expected: step1. Returns a non-null kvstore.
     */
    KvStoreNbDelegate::Option option;
    g_mgr.GetKvStore("MaxLogSize002", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    /**
     * @tc.steps:step2. Put the random entry into the database.
     * @tc.expected: step2. Returns OK.
     */
    Key key;
    Value value;
    DistributedDBToolsUnitTest::GetRandomKeyValue(key, 30); // for 30B random key
    DistributedDBToolsUnitTest::GetRandomKeyValue(value, 3 * 1024 * 1024); // 3M value
    EXPECT_EQ(g_kvNbDelegatePtr->Put(key, value), OK);
    DistributedDBToolsUnitTest::GetRandomKeyValue(key, 40); // for 40B random key
    EXPECT_EQ(g_kvNbDelegatePtr->Put(key, value), OK);
    DistributedDBToolsUnitTest::GetRandomKeyValue(key, 20); // for 20B random key
    DistributedDBToolsUnitTest::GetRandomKeyValue(value, 1 * 1024 * 1024); // 1M value
    EXPECT_EQ(g_kvNbDelegatePtr->Put(key, value), OK);

    /**
     * @tc.steps:step3. Get the resultset.
     * @tc.expected: step3. Returns OK.
     */
    KvStoreResultSet *resultSet = nullptr;
    EXPECT_EQ(g_kvNbDelegatePtr->GetEntries(Key{}, resultSet), OK);
    ASSERT_NE(resultSet, nullptr);
    EXPECT_EQ(resultSet->GetCount(), 3); // size of all the entries is 3
    EXPECT_EQ(resultSet->MoveToFirst(), true);

    /**
     * @tc.steps:step4. Put more data into the database.
     * @tc.expected: step4. Returns OK.
     */
    uint64_t logSize = 6 * 1024 * 1024; // 6M for initial test.
    PragmaData pragLimit = static_cast<PragmaData>(&logSize);
    EXPECT_EQ(g_kvNbDelegatePtr->Pragma(SET_MAX_LOG_LIMIT, pragLimit), OK);
    DistributedDBToolsUnitTest::GetRandomKeyValue(key, 10); // for 10B random key(different size)
    DistributedDBToolsUnitTest::GetRandomKeyValue(value, 3 * 1024 * 1024); // 3MB
    EXPECT_EQ(g_kvNbDelegatePtr->Put(key, value), OK);
    DistributedDBToolsUnitTest::GetRandomKeyValue(key, 15); // for 15B random key(different size)
    EXPECT_EQ(g_kvNbDelegatePtr->Put(key, value), OK);

    /**
     * @tc.steps:step4. Put more data into the database while the log size is over the limit.
     * @tc.expected: step4. Returns LOG_OVER_LIMITS.
     */
    DistributedDBToolsUnitTest::GetRandomKeyValue(value, 25); // for 25B random key(different size)
    EXPECT_EQ(g_kvNbDelegatePtr->Put(key, value), LOG_OVER_LIMITS);
    EXPECT_EQ(g_kvNbDelegatePtr->Delete(key), LOG_OVER_LIMITS);
    EXPECT_EQ(g_kvNbDelegatePtr->StartTransaction(), LOG_OVER_LIMITS);
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocal(key, value), LOG_OVER_LIMITS);
    EXPECT_EQ(g_kvNbDelegatePtr->RemoveDeviceData("deviceA"), LOG_OVER_LIMITS);
    /**
     * @tc.steps:step4. Change the max log size limit, and put the data.
     * @tc.expected: step4. Returns OK.
     */
    logSize *= 10; // 10 multiple size
    pragLimit = static_cast<PragmaData>(&logSize);
    EXPECT_EQ(g_kvNbDelegatePtr->Pragma(SET_MAX_LOG_LIMIT, pragLimit), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Put(key, value), OK);
    g_kvNbDelegatePtr->CloseResultSet(resultSet);

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("MaxLogSize002"), OK);
}

/**
 * @tc.name: MaxLogCheckPoint001
 * @tc.desc: Pragma the checkpoint command.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: wangbingquan
 */
HWTEST_F(DistributedDBInterfacesNBDelegateTest, MaxLogCheckPoint001, TestSize.Level2)
{
    /**
     * @tc.steps:step1. Create database.
     * @tc.expected: step1. Returns a non-null kvstore.
     */
    KvStoreNbDelegate::Option option;
    g_mgr.GetKvStore("MaxLogCheckPoint001", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    /**
     * @tc.steps:step2. Put the random entry into the database.
     * @tc.expected: step2. Returns OK.
     */
    Key key;
    Value value;
    DistributedDBToolsUnitTest::GetRandomKeyValue(key, 30); // for 30B random key(different size)
    DistributedDBToolsUnitTest::GetRandomKeyValue(value, 1 * 1024 * 1024); // 1M
    EXPECT_EQ(g_kvNbDelegatePtr->Put(key, value), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Delete(key), OK);

    /**
     * @tc.steps:step3. Get the disk file size, execute the checkpoint and get the disk file size.
     * @tc.expected: step3. Returns OK and the file size is less than the size before checkpoint.
     */
    uint64_t sizeBeforeChk = 0;
    g_mgr.GetKvStoreDiskSize("MaxLogCheckPoint001", sizeBeforeChk);
    EXPECT_GT(sizeBeforeChk, 1 * 1024 * 1024ULL); // more than 1M
    int param = 0;
    PragmaData paraData = static_cast<PragmaData>(&param);
    g_kvNbDelegatePtr->Pragma(EXEC_CHECKPOINT, paraData);
    uint64_t sizeAfterChk = 0;
    g_mgr.GetKvStoreDiskSize("MaxLogCheckPoint001", sizeAfterChk);
    EXPECT_LT(sizeAfterChk, 100 * 1024ULL); // less than 100K
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("MaxLogCheckPoint001"), OK);
}

/**
  * @tc.name: CreateMemoryDbWithoutPath
  * @tc.desc: Create memory database without path.
  * @tc.type: FUNC
  * @tc.require: AR000CRAKN
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesNBDelegateTest, CreateMemoryDbWithoutPath, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Memory database by GetKvStore without path.
     * @tc.expected: step1. Create successfully.
     */
    KvStoreDelegateManager mgr(APP_ID, USER_ID);
    const KvStoreNbDelegate::Option option = {true, true};
    mgr.GetKvStore("memory_without_path", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_EQ(mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
}

/**
  * @tc.name: OpenStorePathCheckTest001
  * @tc.desc: Test open store with same label but different path.
  * @tc.type: FUNC
  * @tc.require: AR000GK58F
  * @tc.author: lianhuix
  */
HWTEST_F(DistributedDBInterfacesNBDelegateTest, OpenStorePathCheckTest001, TestSize.Level1)
{
    std::string dir1 = g_testDir + "/dbDir1";
    EXPECT_EQ(OS::MakeDBDirectory(dir1), E_OK);
    std::string dir2 = g_testDir + "/dbDir2";
    EXPECT_EQ(OS::MakeDBDirectory(dir2), E_OK);

    KvStoreDelegateManager mgr1(APP_ID, USER_ID);
    mgr1.SetKvStoreConfig({dir1});

    KvStoreNbDelegate *delegate1 = nullptr;
    auto callback1 = bind(&DistributedDBToolsUnitTest::KvStoreNbDelegateCallback, placeholders::_1,
        placeholders::_2, std::ref(g_kvDelegateStatus), std::ref(delegate1));

    KvStoreNbDelegate::Option option;
    mgr1.GetKvStore(STORE_ID_1, option, callback1);
    EXPECT_EQ(g_kvDelegateStatus, OK);
    ASSERT_NE(delegate1, nullptr);

    KvStoreNbDelegate *delegate2 = nullptr;
    auto callback2 = bind(&DistributedDBToolsUnitTest::KvStoreNbDelegateCallback, placeholders::_1,
        placeholders::_2, std::ref(g_kvDelegateStatus), std::ref(delegate2));
    KvStoreDelegateManager mgr2(APP_ID, USER_ID);
    mgr2.SetKvStoreConfig({dir2});
    mgr2.GetKvStore(STORE_ID_1, option, callback2);
    EXPECT_EQ(g_kvDelegateStatus, INVALID_ARGS);
    ASSERT_EQ(delegate2, nullptr);

    mgr1.CloseKvStore(delegate1);
    mgr1.DeleteKvStore(STORE_ID_1);
    mgr2.CloseKvStore(delegate2);
    mgr2.DeleteKvStore(STORE_ID_1);
}

namespace {
std::string GetRealFileUrl(const std::string &dbPath, const std::string &appId, const std::string &userId,
    const std::string &storeId)
{
    std::string hashIdentifier = DBCommon::TransferHashString(
        DBCommon::GenerateIdentifierId(storeId, appId, userId));
    return dbPath + "/" + DBCommon::TransferStringToHex(hashIdentifier) + "/single_ver/main/gen_natural_store.db";
}
}

/**
  * @tc.name: BusyTest001
  * @tc.desc: Test put kv data while another thread holds the transaction for one second
  * @tc.type: FUNC
  * @tc.require: AR000GK58F
  * @tc.author: lianhuix
  */
HWTEST_F(DistributedDBInterfacesNBDelegateTest, BusyTest001, TestSize.Level1)
{
    KvStoreDelegateManager mgr(APP_ID, USER_ID);
    mgr.SetKvStoreConfig(g_config);

    const KvStoreNbDelegate::Option option = {true, false, false};
    mgr.GetKvStore(STORE_ID_1, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    std::string dbPath = GetRealFileUrl(g_config.dataDir, APP_ID, USER_ID, STORE_ID_1);
    sqlite3 *db = RelationalTestUtils::CreateDataBase(dbPath);
    RelationalTestUtils::ExecSql(db, "BEGIN IMMEDIATE;");

    std::thread th([db]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        RelationalTestUtils::ExecSql(db, "COMMIT");
    });

    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_1, VALUE_1), OK);

    th.join();
    sqlite3_close_v2(db);
    EXPECT_EQ(mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(mgr.DeleteKvStore(STORE_ID_1), OK);
}

#ifdef RUNNING_ON_SIMULATED_ENV
/**
  * @tc.name: TimeChangeWithCloseStoreTest001
  * @tc.desc: Test close store with time changed
  * @tc.type: FUNC
  * @tc.require:
  * @tc.author: lianhuix
  */
HWTEST_F(DistributedDBInterfacesNBDelegateTest, TimeChangeWithCloseStoreTest001, TestSize.Level3)
{
    KvStoreDelegateManager mgr(APP_ID, USER_ID);
    mgr.SetKvStoreConfig(g_config);

    std::atomic<bool> isFinished(false);

    std::vector<std::thread> slowThreads;
    for (int i = 0; i < 10; i++) { // 10: thread to slow donw system
        std::thread th([&isFinished]() {
            while (!isFinished) {
                // pass
            }
        });
        slowThreads.emplace_back(std::move(th));
    }

    std::thread th([&isFinished]() {
        int timeChangedCnt = 0;
        while (!isFinished.load()) {
            OS::SetOffsetBySecond(100 - timeChangedCnt++ * 2); // 100 2 : fake system time change
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 100: wait for a while
        }
    });

    for (int i = 0; i < 100; i++) { // run 100 times
        const KvStoreNbDelegate::Option option = {true, false, false};
        mgr.GetKvStore(STORE_ID_1, option, g_kvNbDelegateCallback);
        ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
        EXPECT_EQ(g_kvDelegateStatus, OK);
        EXPECT_EQ(mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // 1000: wait for a while
    isFinished.store(true);
    th.join();
    for (auto &it : slowThreads) {
        it.join();
    }
    EXPECT_EQ(mgr.DeleteKvStore(STORE_ID_1), OK);
}

/**
  * @tc.name: TimeChangeWithCloseStoreTest002
  * @tc.desc: Test close store with time changed
  * @tc.type: FUNC
  * @tc.require:
  * @tc.author: zhangqiquan
  */
HWTEST_F(DistributedDBInterfacesNBDelegateTest, TimeChangeWithCloseStoreTest002, TestSize.Level3)
{
    KvStoreDelegateManager mgr(APP_ID, USER_ID);
    mgr.SetKvStoreConfig(g_config);

    const KvStoreNbDelegate::Option option = {true, false, false};
    mgr.GetKvStore(STORE_ID_1, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_EQ(g_kvDelegateStatus, OK);
    const int threadPoolMax = 10;
    for (int i = 0; i < threadPoolMax; ++i) {
        (void) RuntimeContext::GetInstance()->ScheduleTask([]() {
            std::this_thread::sleep_for(std::chrono::seconds(10)); // sleep 10s for block thread pool
        });
    }
    OS::SetOffsetBySecond(100); // 100 2 : fake system time change
    std::this_thread::sleep_for(std::chrono::seconds(1)); // sleep 1s for time tick

    EXPECT_EQ(mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    g_kvNbDelegatePtr = nullptr;

    EXPECT_EQ(mgr.DeleteKvStore(STORE_ID_1), OK);
    RuntimeContext::GetInstance()->StopTaskPool(); // stop all async task
}
#endif // RUNNING_ON_SIMULATED_ENV

/**
  * @tc.name: LocalStore001
  * @tc.desc: Test get kv store with localOnly
  * @tc.type: FUNC
  * @tc.require:
  * @tc.author: zhangqiquan
  */
HWTEST_F(DistributedDBInterfacesNBDelegateTest, LocalStore001, TestSize.Level1)
{
    KvStoreDelegateManager mgr(APP_ID, USER_ID);
    mgr.SetKvStoreConfig(g_config);

    /**
     * @tc.steps:step1. Create database with localOnly.
     * @tc.expected: step1. Returns a non-null store.
     */
    KvStoreNbDelegate::Option option = {true, false, false};
    option.localOnly = true;
    DBStatus openStatus = DBStatus::DB_ERROR;
    KvStoreNbDelegate *openDelegate = nullptr;
    mgr.GetKvStore(STORE_ID_1, option, [&openStatus, &openDelegate](DBStatus status, KvStoreNbDelegate *delegate) {
        openStatus = status;
        openDelegate = delegate;
    });
    ASSERT_TRUE(openDelegate != nullptr);
    EXPECT_EQ(openStatus, OK);
    /**
     * @tc.steps:step2. call sync and put/get interface.
     * @tc.expected: step2. sync return NOT_ACTIVE.
     */
    DBStatus actionStatus = openDelegate->Sync({}, SyncMode::SYNC_MODE_PUSH_ONLY, nullptr);
    EXPECT_EQ(actionStatus, DBStatus::NOT_ACTIVE);
    Key key = {'k'};
    Value expectValue = {'v'};
    EXPECT_EQ(openDelegate->Put(key, expectValue), OK);
    Value actualValue;
    EXPECT_EQ(openDelegate->Get(key, actualValue), OK);
    EXPECT_EQ(actualValue, expectValue);
    EXPECT_EQ(mgr.CloseKvStore(openDelegate), OK);
}

/**
  * @tc.name: LocalStore002
  * @tc.desc: Test get kv store different local mode
  * @tc.type: FUNC
  * @tc.require:
  * @tc.author: zhangqiquan
  */
HWTEST_F(DistributedDBInterfacesNBDelegateTest, LocalStore002, TestSize.Level1)
{
    KvStoreDelegateManager mgr(APP_ID, USER_ID);
    mgr.SetKvStoreConfig(g_config);

    /**
     * @tc.steps:step1. Create database with localOnly.
     * @tc.expected: step1. Returns a non-null store.
     */
    KvStoreNbDelegate::Option option = {true, false, false};
    option.localOnly = true;
    DBStatus openStatus = DBStatus::DB_ERROR;
    KvStoreNbDelegate *localDelegate = nullptr;
    mgr.GetKvStore(STORE_ID_1, option, [&openStatus, &localDelegate](DBStatus status, KvStoreNbDelegate *delegate) {
        openStatus = status;
        localDelegate = delegate;
    });
    ASSERT_TRUE(localDelegate != nullptr);
    EXPECT_EQ(openStatus, OK);
    /**
     * @tc.steps:step2. Create database without localOnly.
     * @tc.expected: step2. Returns a null store.
     */
    option.localOnly = false;
    KvStoreNbDelegate *syncDelegate = nullptr;
    mgr.GetKvStore(STORE_ID_1, option, [&openStatus, &syncDelegate](DBStatus status, KvStoreNbDelegate *delegate) {
        openStatus = status;
        syncDelegate = delegate;
    });
    EXPECT_EQ(syncDelegate, nullptr);
    EXPECT_EQ(openStatus, INVALID_ARGS);
    EXPECT_EQ(mgr.CloseKvStore(localDelegate), OK);
}