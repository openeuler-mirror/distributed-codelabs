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
#include <unistd.h>
#include <cstddef>
#include <cstdint>
#include <vector>
#include "distributed_kv_data_manager.h"
#include "types.h"

using namespace testing::ext;
using namespace OHOS::DistributedKv;

class SingleKvStoreClientTest : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();

    static std::shared_ptr<SingleKvStore> singleKvStore; // declare kvstore instance.
    static Status status_;
    static int MAX_VALUE_SIZE;
};

const std::string VALID_SCHEMA_STRICT_DEFINE = "{\"SCHEMA_VERSION\":\"1.0\","
        "\"SCHEMA_MODE\":\"STRICT\","
        "\"SCHEMA_SKIPSIZE\":0,"
        "\"SCHEMA_DEFINE\":{"
            "\"age\":\"INTEGER, NOT NULL\""
        "},"
        "\"SCHEMA_INDEXES\":[\"$.age\"]}";

std::shared_ptr<SingleKvStore> SingleKvStoreClientTest::singleKvStore = nullptr;
Status SingleKvStoreClientTest::status_ = Status::ERROR;
int SingleKvStoreClientTest::MAX_VALUE_SIZE = 4 * 1024 * 1024; // max value size is 4M.

void SingleKvStoreClientTest::SetUpTestCase(void)
{
    DistributedKvDataManager manager;
    Options options = { .createIfMissing = true, .encrypt = false, .autoSync = true,
                        .kvStoreType = KvStoreType::SINGLE_VERSION };
    options.area = EL1;
    options.baseDir = std::string("/data/service/el1/public/database/odmf");
    AppId appId = { "odmf" };
    StoreId storeId = { "student_single" }; // define kvstore(database) name.
    mkdir(options.baseDir.c_str(), (S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH));
    // [create and] open and initialize kvstore instance.
    status_ = manager.GetSingleKvStore(options, appId, storeId, singleKvStore);
}

void SingleKvStoreClientTest::TearDownTestCase(void)
{
    (void)remove("/data/service/el1/public/database/odmf/key");
    (void)remove("/data/service/el1/public/database/odmf/kvdb");
    (void)remove("/data/service/el1/public/database/odmf");
}

void SingleKvStoreClientTest::SetUp(void)
{}

void SingleKvStoreClientTest::TearDown(void)
{}

class KvStoreObserverTestImpl : public KvStoreObserver {
public:
    std::vector<Entry> insertEntries_;
    std::vector<Entry> updateEntries_;
    std::vector<Entry> deleteEntries_;
    bool isClear_ = false;
    KvStoreObserverTestImpl();
    ~KvStoreObserverTestImpl()
    {}

    KvStoreObserverTestImpl(const KvStoreObserverTestImpl &) = delete;
    KvStoreObserverTestImpl &operator=(const KvStoreObserverTestImpl &) = delete;
    KvStoreObserverTestImpl(KvStoreObserverTestImpl &&) = delete;
    KvStoreObserverTestImpl &operator=(KvStoreObserverTestImpl &&) = delete;

    void OnChange(const ChangeNotification &changeNotification);

    // reset the callCount_ to zero.
    void ResetToZero();

    uint64_t GetCallCount() const;

private:
    uint64_t callCount_;
};

void KvStoreObserverTestImpl::OnChange(const ChangeNotification &changeNotification)
{
    callCount_++;
    const auto &insert = changeNotification.GetInsertEntries();
    insertEntries_.clear();
    for (const auto &entry : insert) {
        insertEntries_.push_back(entry);
    }

    const auto &update = changeNotification.GetUpdateEntries();
    updateEntries_.clear();
    for (const auto &entry : update) {
        updateEntries_.push_back(entry);
    }

    const auto &del = changeNotification.GetDeleteEntries();
    deleteEntries_.clear();
    for (const auto &entry : del) {
        deleteEntries_.push_back(entry);
    }

    isClear_ = changeNotification.IsClear();
}

KvStoreObserverTestImpl::KvStoreObserverTestImpl()
{
    callCount_ = 0;
    insertEntries_ = {};
    updateEntries_ = {};
    deleteEntries_ = {};
    isClear_ = false;
}

void KvStoreObserverTestImpl::ResetToZero()
{
    callCount_ = 0;
}

uint64_t KvStoreObserverTestImpl::GetCallCount() const
{
    return callCount_;
}

class KvStoreSyncCallbackTestImpl : public KvStoreSyncCallback {
public:
    void SyncCompleted(const std::map<std::string, Status> &results);
};

void KvStoreSyncCallbackTestImpl::SyncCompleted(const std::map<std::string, Status> &results)
{}

/**
* @tc.name: GetStoreId001
* @tc.desc: Get a single KvStore instance.
* @tc.type: FUNC
* @tc.require: SR000DORPS AR000DPRQ7 AR000DDPRPL
* @tc.author: hongbo
*/
HWTEST_F(SingleKvStoreClientTest, GetStoreId001, TestSize.Level1)
{
    EXPECT_NE(singleKvStore, nullptr) << "kvStorePtr is null.";

    auto storID = singleKvStore->GetStoreId();
    EXPECT_EQ(storID.storeId, "student_single");
}

/**
* @tc.name: PutGetDelete001
* @tc.desc: put value and delete value
* @tc.type: FUNC
* @tc.require: SR000DORPS AR000DPRQ7 AR000DDPRPL
* @tc.author: hongbo
*/
HWTEST_F(SingleKvStoreClientTest, PutGetDelete001, TestSize.Level1)
{
    EXPECT_NE(singleKvStore, nullptr) << "kvStorePtr is null.";

    Key skey = {"single_001"};
    Value sval = {"value_001"};
    auto status = singleKvStore->Put(skey, sval);
    EXPECT_EQ(status, Status::SUCCESS) << "putting data failed";

    auto delStatus = singleKvStore->Delete(skey);
    EXPECT_EQ(delStatus, Status::SUCCESS) << "deleting data failed";

    auto notExistStatus = singleKvStore->Delete(skey);
    EXPECT_EQ(notExistStatus, Status::SUCCESS) << "deleting non-existing data failed";

    auto spaceStatus = singleKvStore->Put(skey, {""});
    EXPECT_EQ(spaceStatus, Status::SUCCESS) << "putting space failed";

    auto spaceKeyStatus = singleKvStore->Put({""}, {""});
    EXPECT_NE(spaceKeyStatus, Status::SUCCESS) << "putting space keys failed";

    Status validStatus = singleKvStore->Put(skey, sval);
    EXPECT_EQ(validStatus, Status::SUCCESS) << "putting valid keys and values failed";

    Value rVal;
    auto validPutStatus = singleKvStore->Get(skey, rVal);
    EXPECT_EQ(validPutStatus, Status::SUCCESS) << "Getting value failed";
    EXPECT_EQ(sval, rVal) << "Got and put values not equal";
}

/**
* @tc.name: GetEntriesAndResultSet001
* @tc.desc: Batch put values and get values.
* @tc.type: FUNC
* @tc.require: SR000DORPS AR000DPRQ7 AR000DDPRPL
* @tc.author: hongbo
*/
HWTEST_F(SingleKvStoreClientTest, GetEntriesAndResultSet001, TestSize.Level1)
{
    EXPECT_NE(singleKvStore, nullptr) << "kvStorePtr is null.";

    // prepare 10
    size_t sum = 10;
    int sum_1 = 10;
    std::string prefix = "prefix_";
    for (size_t i = 0; i < sum; i++) {
        singleKvStore->Put({prefix + std::to_string(i)}, {std::to_string(i)});
    }

    std::vector<Entry> results;
    singleKvStore->GetEntries({prefix}, results);
    EXPECT_EQ(results.size(), sum) << "entries size is not equal 10.";

    std::shared_ptr<KvStoreResultSet> resultSet;
    Status status = singleKvStore->GetResultSet({prefix}, resultSet);
    EXPECT_EQ(status, Status::SUCCESS);
    EXPECT_EQ(resultSet->GetCount(), sum_1) << "resultSet size is not equal 10.";
    resultSet->IsFirst();
    resultSet->IsAfterLast();
    resultSet->IsBeforeFirst();
    resultSet->MoveToPosition(1);
    resultSet->IsLast();
    resultSet->MoveToPrevious();
    resultSet->MoveToNext();
    resultSet->MoveToLast();
    resultSet->MoveToFirst();
    resultSet->GetPosition();
    Entry entry;
    resultSet->GetEntry(entry);

    for (size_t i = 0; i < sum; i++) {
        singleKvStore->Delete({prefix + std::to_string(i)});
    }

    auto closeResultSetStatus = singleKvStore->CloseResultSet(resultSet);
    EXPECT_EQ(closeResultSetStatus, Status::SUCCESS) << "close resultSet failed.";
}

/**
* @tc.name: GetEntriesByDataQuery
* @tc.desc: Batch put values and get values.
* @tc.type: FUNC
* @tc.require: I5GFGR
* @tc.author: Sven Wang
*/
HWTEST_F(SingleKvStoreClientTest, GetEntriesByDataQuery, TestSize.Level1)
{
    EXPECT_NE(singleKvStore, nullptr) << "kvStorePtr is null.";

    // prepare 10
    size_t sum = 10;
    int sum_1 = 10;
    std::string prefix = "prefix_";
    for (size_t i = 0; i < sum; i++) {
        singleKvStore->Put({prefix + std::to_string(i)}, {std::to_string(i)});
    }

    std::vector<Entry> results;
    singleKvStore->GetEntries({ prefix }, results);
    EXPECT_EQ(results.size(), sum) << "entries size is not equal 10.";
    DataQuery dataQuery;
    dataQuery.KeyPrefix(prefix);
    dataQuery.Limit(10, 0);
    std::shared_ptr<KvStoreResultSet> resultSet;
    Status status = singleKvStore->GetResultSet(dataQuery, resultSet);
    EXPECT_EQ(status, Status::SUCCESS);
    EXPECT_EQ(resultSet->GetCount(), sum_1) << "resultSet size is not equal 10.";
    resultSet->IsFirst();
    resultSet->IsAfterLast();
    resultSet->IsBeforeFirst();
    resultSet->MoveToPosition(1);
    resultSet->IsLast();
    resultSet->MoveToPrevious();
    resultSet->MoveToNext();
    resultSet->MoveToLast();
    resultSet->MoveToFirst();
    resultSet->GetPosition();
    Entry entry;
    resultSet->GetEntry(entry);

    for (size_t i = 0; i < sum; i++) {
        singleKvStore->Delete({prefix + std::to_string(i)});
    }

    auto closeResultSetStatus = singleKvStore->CloseResultSet(resultSet);
    EXPECT_EQ(closeResultSetStatus, Status::SUCCESS) << "close resultSet failed.";
}


/**
* @tc.name: GetEmptyEntries
* @tc.desc: Batch get empty values.
* @tc.type: FUNC
* @tc.require:
* @tc.author: Sven Wang
*/
HWTEST_F(SingleKvStoreClientTest, GetEmptyEntries, TestSize.Level1)
{
    EXPECT_NE(singleKvStore, nullptr) << "kvStorePtr is null.";
    std::vector<Entry> results;
    auto status = singleKvStore->GetEntries({ "SUCCESS_TEST" }, results);
    EXPECT_EQ(status, Status::SUCCESS) << "status is not SUCCESS.";
    EXPECT_EQ(results.size(), 0) << "entries size is not empty.";
}

/**
* @tc.name: Subscribe001
* @tc.desc: Put data and get callback.
* @tc.type: FUNC
* @tc.require: SR000DORPS AR000DPRQ7 AR000DDPRPL
* @tc.author: hongbo
*/
HWTEST_F(SingleKvStoreClientTest, Subscribe001, TestSize.Level1)
{
    auto observer = std::make_shared<KvStoreObserverTestImpl>();
    auto subStatus = singleKvStore->SubscribeKvStore(SubscribeType::SUBSCRIBE_TYPE_ALL, observer);
    EXPECT_EQ(subStatus, Status::SUCCESS) << "subscribe kvStore observer failed.";
    // subscribe repeated observer;
    auto repeatedSubStatus = singleKvStore->SubscribeKvStore(SubscribeType::SUBSCRIBE_TYPE_ALL, observer);
    EXPECT_NE(repeatedSubStatus, Status::SUCCESS) << "repeat subscribe kvStore observer failed.";

    auto unSubStatus = singleKvStore->UnSubscribeKvStore(SubscribeType::SUBSCRIBE_TYPE_ALL, observer);
    EXPECT_EQ(unSubStatus, Status::SUCCESS) << "unsubscribe kvStore observer failed.";
}

/**
* @tc.name: SyncCallback001
* @tc.desc: Register sync callback.
* @tc.type: FUNC
* @tc.require: SR000DORPS AR000DPRQ7 AR000DDPRPL
* @tc.author: hongbo
*/
HWTEST_F(SingleKvStoreClientTest, SyncCallback001, TestSize.Level1)
{
    EXPECT_NE(singleKvStore, nullptr) << "kvStorePtr is null.";

    auto syncCallback = std::make_shared<KvStoreSyncCallbackTestImpl>();
    auto syncStatus = singleKvStore->RegisterSyncCallback(syncCallback);
    EXPECT_EQ(syncStatus, Status::SUCCESS) << "register sync callback failed.";

    auto unRegStatus = singleKvStore->UnRegisterSyncCallback();
    EXPECT_EQ(unRegStatus, Status::SUCCESS) << "un register sync callback failed.";

    Key skey = {"single_001"};
    Value sval = {"value_001"};
    singleKvStore->Put(skey, sval);
    singleKvStore->Delete(skey);

    std::map<std::string, Status> results;
    results.insert({"aaa", Status::INVALID_ARGUMENT});
    syncCallback->SyncCompleted(results);
}

/**
* @tc.name: RemoveDeviceData001
* @tc.desc: Remove device data.
* @tc.type: FUNC
* @tc.require: SR000DORPS AR000DPRQ7 AR000DDPRPL
* @tc.author: hongbo
*/
HWTEST_F(SingleKvStoreClientTest, RemoveDeviceData001, TestSize.Level1)
{
    EXPECT_NE(singleKvStore, nullptr) << "kvStorePtr is null.";

    Key skey = {"single_001"};
    Value sval = {"value_001"};
    singleKvStore->Put(skey, sval);

    std::string deviceId = "no_exist_device_id";
    auto removeStatus = singleKvStore->RemoveDeviceData(deviceId);
    EXPECT_NE(removeStatus, Status::SUCCESS) << "remove device should not return success";

    Value retVal;
    auto getRet = singleKvStore->Get(skey, retVal);
    EXPECT_EQ(getRet, Status::SUCCESS) << "get value failed.";
    EXPECT_EQ(retVal.Size(), sval.Size()) << "data base should be null.";
}

/**
* @tc.name: SyncData001
* @tc.desc: Synchronize device data.
* @tc.type: FUNC
* @tc.require: SR000DORPS AR000DPRQ7 AR000DDPRPL
* @tc.author: hongbo
*/
HWTEST_F(SingleKvStoreClientTest, SyncData001, TestSize.Level1)
{
    EXPECT_NE(singleKvStore, nullptr) << "kvStorePtr is null.";
    std::string deviceId = "no_exist_device_id";
    std::vector<std::string> deviceIds = { deviceId };
    auto syncStatus = singleKvStore->Sync(deviceIds, SyncMode::PUSH);
    EXPECT_NE(syncStatus, Status::SUCCESS) << "sync device should not return success";
}

/**
* @tc.name: TestSchemaStoreC001
* @tc.desc: Test schema single store.
* @tc.type: FUNC
* @tc.require: AR000DPSF1
* @tc.author: zuojiangjiang
*/
HWTEST_F(SingleKvStoreClientTest, TestSchemaStoreC001, TestSize.Level1)
{
    std::shared_ptr<SingleKvStore> singleKvStore;
    DistributedKvDataManager manager;
    Options options;
    options.encrypt = true;
    options.area = EL1;
    options.kvStoreType = KvStoreType::SINGLE_VERSION;
    options.baseDir = "/data/service/el1/public/database/odmf";
    options.schema = VALID_SCHEMA_STRICT_DEFINE;
    AppId appId = { "odmf" };
    StoreId storeId = { "schema_store_id" };
    (void)manager.GetSingleKvStore(options, appId, storeId, singleKvStore);
    ASSERT_NE(singleKvStore, nullptr) << "kvStorePtr is null.";
    auto result = singleKvStore->GetStoreId();
    EXPECT_EQ(result.storeId, "schema_store_id");

    Key testKey = {"TestSchemaStoreC001_key"};
    Value testValue = {"{\"age\":10}"};
    auto testStatus = singleKvStore->Put(testKey, testValue);
    EXPECT_EQ(testStatus, Status::SUCCESS) << "putting data failed";
    Value resultValue;
    auto getRet = singleKvStore->Get(testKey, resultValue);
    EXPECT_EQ(getRet, Status::SUCCESS) << "get value failed.";
    manager.DeleteKvStore(appId, storeId, options.baseDir);
}

/**
* @tc.name: SyncData001
* @tc.desc: Synchronize device data.
* @tc.type: FUNC
* @tc.require: SR000DOGQE AR000DPUAN
* @tc.author: wangtao
*/
HWTEST_F(SingleKvStoreClientTest, SyncData002, TestSize.Level1)
{
    EXPECT_NE(singleKvStore, nullptr) << "kvStorePtr is null.";
    std::string deviceId = "no_exist_device_id";
    std::vector<std::string> deviceIds = { deviceId };
    uint32_t allowedDelayMs = 200;
    auto syncStatus = singleKvStore->Sync(deviceIds, SyncMode::PUSH, allowedDelayMs);
    EXPECT_EQ(syncStatus, Status::SUCCESS) << "sync device should return success";
}

/**
* @tc.name: SyncData002
* @tc.desc: Set sync parameters - success.
* @tc.type: FUNC
* @tc.require: SR000DOGQE AR000DPUAO
* @tc.author: wangtao
*/
HWTEST_F(SingleKvStoreClientTest, SetSync001, TestSize.Level1)
{
    EXPECT_NE(singleKvStore, nullptr) << "kvStorePtr is null.";
    KvSyncParam syncParam{ 500 }; // 500ms
    auto ret = singleKvStore->SetSyncParam(syncParam);
    EXPECT_EQ(ret, Status::SUCCESS) << "set sync param should return success";

    KvSyncParam syncParamRet;
    singleKvStore->GetSyncParam(syncParamRet);
    EXPECT_EQ(syncParamRet.allowedDelayMs, syncParam.allowedDelayMs);
}

/**
* @tc.name: SyncData002
* @tc.desc: Set sync parameters - failed.
* @tc.type: FUNC
* @tc.require: SR000DOGQE AR000DPUAO
* @tc.author: wangtao
*/
HWTEST_F(SingleKvStoreClientTest, SetSync002, TestSize.Level1)
{
    EXPECT_NE(singleKvStore, nullptr) << "kvStorePtr is null.";
    KvSyncParam syncParam2{ 50 }; // 50ms
    auto ret = singleKvStore->SetSyncParam(syncParam2);
    EXPECT_NE(ret, Status::SUCCESS) << "set sync param should not return success";

    KvSyncParam syncParamRet2;
    ret = singleKvStore->GetSyncParam(syncParamRet2);
    EXPECT_NE(syncParamRet2.allowedDelayMs, syncParam2.allowedDelayMs);
}

/**
* @tc.name: SingleKvStoreDdmPutBatch001
* @tc.desc: Batch put data.
* @tc.type: FUNC
* @tc.require: AR000DPSEA
* @tc.author: shanshuangshuang
*/
HWTEST_F(SingleKvStoreClientTest, DdmPutBatch001, TestSize.Level2)
{
    EXPECT_NE(nullptr, singleKvStore) << "singleKvStore is nullptr";

    // store entries to kvstore.
    std::vector<Entry> entries;
    Entry entry1, entry2, entry3;
    entry1.key = "KvStoreDdmPutBatch001_1";
    entry1.value = "age:20";
    entry2.key = "KvStoreDdmPutBatch001_2";
    entry2.value = "age:19";
    entry3.key = "KvStoreDdmPutBatch001_3";
    entry3.value = "age:23";
    entries.push_back(entry1);
    entries.push_back(entry2);
    entries.push_back(entry3);

    Status status = singleKvStore->PutBatch(entries);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putbatch data return wrong status";
    // get value from kvstore.
    Value valueRet1;
    Status statusRet1 = singleKvStore->Get(entry1.key, valueRet1);
    EXPECT_EQ(Status::SUCCESS, statusRet1) << "KvStoreSnapshot get data return wrong status";
    EXPECT_EQ(entry1.value, valueRet1) << "value and valueRet are not equal";

    Value valueRet2;
    Status statusRet2 = singleKvStore->Get(entry2.key, valueRet2);
    EXPECT_EQ(Status::SUCCESS, statusRet2) << "KvStoreSnapshot get data return wrong status";
    EXPECT_EQ(entry2.value, valueRet2) << "value and valueRet are not equal";

    Value valueRet3;
    Status statusRet3 = singleKvStore->Get(entry3.key, valueRet3);
    EXPECT_EQ(Status::SUCCESS, statusRet3) << "KvStoreSnapshot get data return wrong status";
    EXPECT_EQ(entry3.value, valueRet3) << "value and valueRet are not equal";
}

/**
* @tc.name: SingleKvStoreDdmPutBatch002
* @tc.desc: Batch update data.
* @tc.type: FUNC
* @tc.require: AR000DPSEA
* @tc.author: shanshuangshuang
*/
HWTEST_F(SingleKvStoreClientTest, DdmPutBatch002, TestSize.Level2)
{
    EXPECT_NE(nullptr, singleKvStore) << "singleKvStore is nullptr";

    // before update.
    std::vector<Entry> entriesBefore;
    Entry entry1, entry2, entry3;
    entry1.key = "SingleKvStoreDdmPutBatch002_1";
    entry1.value = "age:20";
    entry2.key = "SingleKvStoreDdmPutBatch002_2";
    entry2.value = "age:19";
    entry3.key = "SingleKvStoreDdmPutBatch002_3";
    entry3.value = "age:23";
    entriesBefore.push_back(entry1);
    entriesBefore.push_back(entry2);
    entriesBefore.push_back(entry3);

    Status status = singleKvStore->PutBatch(entriesBefore);
    EXPECT_EQ(Status::SUCCESS, status) << "SingleKvStore putbatch data return wrong status";

    // after update.
    std::vector<Entry> entriesAfter;
    Entry entry4, entry5, entry6;
    entry4.key = "SingleKvStoreDdmPutBatch002_1";
    entry4.value = "age:20, sex:girl";
    entry5.key = "SingleKvStoreDdmPutBatch002_2";
    entry5.value = "age:19, sex:boy";
    entry6.key = "SingleKvStoreDdmPutBatch002_3";
    entry6.value = "age:23, sex:girl";
    entriesAfter.push_back(entry4);
    entriesAfter.push_back(entry5);
    entriesAfter.push_back(entry6);

    status = singleKvStore->PutBatch(entriesAfter);
    EXPECT_EQ(Status::SUCCESS, status) << "SingleKvStore putbatch failed, wrong status";

    // get value from kvstore.
    Value valueRet1;
    Status statusRet1 = singleKvStore->Get(entry4.key, valueRet1);
    EXPECT_EQ(Status::SUCCESS, statusRet1) << "SingleKvStore getting data failed, wrong status";
    EXPECT_EQ(entry4.value, valueRet1) << "value and valueRet are not equal";

    Value valueRet2;
    Status statusRet2 = singleKvStore->Get(entry5.key, valueRet2);
    EXPECT_EQ(Status::SUCCESS, statusRet2) << "SingleKvStore getting data failed, wrong status";
    EXPECT_EQ(entry5.value, valueRet2) << "value and valueRet are not equal";

    Value valueRet3;
    Status statusRet3 = singleKvStore->Get(entry6.key, valueRet3);
    EXPECT_EQ(Status::SUCCESS, statusRet3) << "SingleKvStore get data return wrong status";
    EXPECT_EQ(entry6.value, valueRet3) << "value and valueRet are not equal";
}

/**
* @tc.name: DdmPutBatch003
* @tc.desc: Batch put data that contains invalid data.
* @tc.type: FUNC
* @tc.require: AR000DPSEA
* @tc.author: shanshuangshuang
*/
HWTEST_F(SingleKvStoreClientTest, DdmPutBatch003, TestSize.Level2)
{
    EXPECT_NE(nullptr, singleKvStore) << "singleKvStore is nullptr";

    std::vector<Entry> entries;
    Entry entry1, entry2, entry3;
    entry1.key = "         ";
    entry1.value = "age:20";
    entry2.key = "student_name_caixu";
    entry2.value = "         ";
    entry3.key = "student_name_liuyue";
    entry3.value = "age:23";
    entries.push_back(entry1);
    entries.push_back(entry2);
    entries.push_back(entry3);

    Status status = singleKvStore->PutBatch(entries);
    EXPECT_EQ(Status::INVALID_ARGUMENT, status) << "singleKvStore putbatch data return wrong status";
}

/**
* @tc.name: DdmPutBatch004
* @tc.desc: Batch put data that contains invalid data.
* @tc.type: FUNC
* @tc.require: AR000DPSEA
* @tc.author: shanshuangshuang
*/
HWTEST_F(SingleKvStoreClientTest, DdmPutBatch004, TestSize.Level2)
{
    EXPECT_NE(nullptr, singleKvStore) << "singleKvStore is nullptr";

    std::vector<Entry> entries;
    Entry entry1, entry2, entry3;
    entry1.key = "";
    entry1.value = "age:20";
    entry2.key = "student_name_caixu";
    entry2.value = "";
    entry3.key = "student_name_liuyue";
    entry3.value = "age:23";
    entries.push_back(entry1);
    entries.push_back(entry2);
    entries.push_back(entry3);

    Status status = singleKvStore->PutBatch(entries);
    EXPECT_EQ(Status::INVALID_ARGUMENT, status) << "singleKvStore putbatch data return wrong status";
}

static std::string SingleGenerate1025KeyLen()
{
    std::string str("prefix");
    // Generate a key with a length of more than 1024 bytes.
    for (int i = 0; i < 1024; i++) {
        str += "a";
    }
    return str;
}
/**
* @tc.name: DdmPutBatch005
* @tc.desc: Batch put data that contains invalid data.
* @tc.type: FUNC
* @tc.require: AR000DPSEA
* @tc.author: shanshuangshuang
*/
HWTEST_F(SingleKvStoreClientTest, DdmPutBatch005, TestSize.Level2)
{

    EXPECT_NE(nullptr, singleKvStore) << "singleKvStore is nullptr";

    std::vector<Entry> entries;
    Entry entry1, entry2, entry3;
    entry1.key = SingleGenerate1025KeyLen();
    entry1.value = "age:20";
    entry2.key = "student_name_caixu";
    entry2.value = "age:19";
    entry3.key = "student_name_liuyue";
    entry3.value = "age:23";
    entries.push_back(entry1);
    entries.push_back(entry2);
    entries.push_back(entry3);

    Status status = singleKvStore->PutBatch(entries);
    EXPECT_EQ(Status::INVALID_ARGUMENT, status) << "KvStore putbatch data return wrong status";
}

/**
* @tc.name: DdmPutBatch006
* @tc.desc: Batch put large data.
* @tc.type: FUNC
* @tc.require: AR000DPSEA
* @tc.author: shanshuangshuang
*/
HWTEST_F(SingleKvStoreClientTest, DdmPutBatch006, TestSize.Level2)
{
    EXPECT_NE(nullptr, singleKvStore) << "singleKvStore is nullptr";

    std::vector<uint8_t> val(MAX_VALUE_SIZE);
    for (int i = 0; i < MAX_VALUE_SIZE; i++) {
        val[i] = static_cast<uint8_t>(i);
    }
    Value value = val;

    std::vector<Entry> entries;
    Entry entry1, entry2, entry3;
    entry1.key = "SingleKvStoreDdmPutBatch006_1";
    entry1.value = value;
    entry2.key = "SingleKvStoreDdmPutBatch006_2";
    entry2.value = value;
    entry3.key = "SingleKvStoreDdmPutBatch006_3";
    entry3.value = value;
    entries.push_back(entry1);
    entries.push_back(entry2);
    entries.push_back(entry3);
    Status status = singleKvStore->PutBatch(entries);
    EXPECT_EQ(Status::SUCCESS, status) << "singleKvStore putbatch data return wrong status";

    // get value from kvstore.
    Value valueRet1;
    Status statusRet1 = singleKvStore->Get(entry1.key, valueRet1);
    EXPECT_EQ(Status::SUCCESS, statusRet1) << "singleKvStore get data return wrong status";
    EXPECT_EQ(entry1.value, valueRet1) << "value and valueRet are not equal";

    Value valueRet2;
    Status statusRet2 = singleKvStore->Get(entry2.key, valueRet2);
    EXPECT_EQ(Status::SUCCESS, statusRet2) << "singleKvStore get data return wrong status";
    EXPECT_EQ(entry2.value, valueRet2) << "value and valueRet are not equal";

    Value valueRet3;
    Status statusRet3 = singleKvStore->Get(entry3.key, valueRet3);
    EXPECT_EQ(Status::SUCCESS, statusRet3) << "singleKvStore get data return wrong status";
    EXPECT_EQ(entry3.value, valueRet3) << "value and valueRet are not equal";
}

/**
* @tc.name: DdmDeleteBatch001
* @tc.desc: Batch delete data.
* @tc.type: FUNC
* @tc.require: AR000DPSEA
* @tc.author: shanshuangshuang
*/
HWTEST_F(SingleKvStoreClientTest, DdmDeleteBatch001, TestSize.Level2)
{
    EXPECT_NE(nullptr, singleKvStore) << "singleKvStore is nullptr";

    // store entries to kvstore.
    std::vector<Entry> entries;
    Entry entry1, entry2, entry3;
    entry1.key = "SingleKvStoreDdmDeleteBatch001_1";
    entry1.value = "age:20";
    entry2.key = "SingleKvStoreDdmDeleteBatch001_2";
    entry2.value = "age:19";
    entry3.key = "SingleKvStoreDdmDeleteBatch001_3";
    entry3.value = "age:23";
    entries.push_back(entry1);
    entries.push_back(entry2);
    entries.push_back(entry3);

    std::vector<Key> keys;
    keys.push_back("SingleKvStoreDdmDeleteBatch001_1");
    keys.push_back("SingleKvStoreDdmDeleteBatch001_2");
    keys.push_back("SingleKvStoreDdmDeleteBatch001_3");

    Status status1 = singleKvStore->PutBatch(entries);
    EXPECT_EQ(Status::SUCCESS, status1) << "singleKvStore putbatch data return wrong status";

    Status status2 = singleKvStore->DeleteBatch(keys);
    EXPECT_EQ(Status::SUCCESS, status2) << "singleKvStore deletebatch data return wrong status";
    std::vector<Entry> results;
    singleKvStore->GetEntries("SingleKvStoreDdmDeleteBatch001_", results);
    size_t sum = 0;
    EXPECT_EQ(results.size(), sum) << "entries size is not equal 0.";
}

/**
* @tc.name: DdmDeleteBatch002
* @tc.desc: Batch delete data when some keys are not in KvStore.
* @tc.type: FUNC
* @tc.require: AR000DPSEA
* @tc.author: shanshuangshuang
*/
HWTEST_F(SingleKvStoreClientTest, DdmDeleteBatch002, TestSize.Level2)
{
    EXPECT_NE(nullptr, singleKvStore) << "singleKvStore is nullptr";

    // store entries to kvstore.
    std::vector<Entry> entries;
    Entry entry1, entry2, entry3;
    entry1.key = "SingleKvStoreDdmDeleteBatch002_1";
    entry1.value = "age:20";
    entry2.key = "SingleKvStoreDdmDeleteBatch002_2";
    entry2.value = "age:19";
    entry3.key = "SingleKvStoreDdmDeleteBatch002_3";
    entry3.value = "age:23";
    entries.push_back(entry1);
    entries.push_back(entry2);
    entries.push_back(entry3);

    std::vector<Key> keys;
    keys.push_back("SingleKvStoreDdmDeleteBatch002_1");
    keys.push_back("SingleKvStoreDdmDeleteBatch002_2");
    keys.push_back("SingleKvStoreDdmDeleteBatch002_3");
    keys.push_back("SingleKvStoreDdmDeleteBatch002_4");

    Status status1 = singleKvStore->PutBatch(entries);
    EXPECT_EQ(Status::SUCCESS, status1) << "KvStore putbatch data return wrong status";

    Status status2 = singleKvStore->DeleteBatch(keys);
    EXPECT_EQ(Status::SUCCESS, status2) << "KvStore deletebatch data return wrong status";
    std::vector<Entry> results;
    singleKvStore->GetEntries("SingleKvStoreDdmDeleteBatch002_", results);
    size_t sum = 0;
    EXPECT_EQ(results.size(), sum) << "entries size is not equal 0.";
}

/**
* @tc.name: DdmDeleteBatch003
* @tc.desc: Batch delete data when some keys are invalid.
* @tc.type: FUNC
* @tc.require: AR000DPSEA
* @tc.author: shanshuangshuang
*/
HWTEST_F(SingleKvStoreClientTest, DdmDeleteBatch003, TestSize.Level2)
{
    EXPECT_NE(nullptr, singleKvStore) << "singleKvStore is nullptr";

    // Store entries to KvStore. 
    std::vector<Entry> entries;
    Entry entry1, entry2, entry3;
    entry1.key = "SingleKvStoreDdmDeleteBatch003_1";
    entry1.value = "age:20";
    entry2.key = "SingleKvStoreDdmDeleteBatch003_2";
    entry2.value = "age:19";
    entry3.key = "SingleKvStoreDdmDeleteBatch003_3";
    entry3.value = "age:23";
    entries.push_back(entry1);
    entries.push_back(entry2);
    entries.push_back(entry3);

    std::vector<Key> keys;
    keys.push_back("SingleKvStoreDdmDeleteBatch003_1");
    keys.push_back("SingleKvStoreDdmDeleteBatch003_2");
    keys.push_back("");

    Status status1 = singleKvStore->PutBatch(entries);
    EXPECT_EQ(Status::SUCCESS, status1) << "SingleKvStore putbatch data return wrong status";

    Status status2 = singleKvStore->DeleteBatch(keys);
    EXPECT_EQ(Status::INVALID_ARGUMENT, status2) << "KvStore deletebatch data return wrong status";
    std::vector<Entry> results;
    singleKvStore->GetEntries("SingleKvStoreDdmDeleteBatch003_", results);
    size_t sum = 3;
    EXPECT_EQ(results.size(), sum) << "entries size is not equal 3.";
}

/**
* @tc.name: DdmDeleteBatch004
* @tc.desc: Batch delete data when some keys are invalid.
* @tc.type: FUNC
* @tc.require: AR000DPSEA
* @tc.author: shanshuangshuang
*/
HWTEST_F(SingleKvStoreClientTest, DdmDeleteBatch004, TestSize.Level2)
{
    EXPECT_NE(nullptr, singleKvStore) << "singleKvStore is nullptr";

    // store entries to kvstore.
    std::vector<Entry> entries;
    Entry entry1, entry2, entry3;
    entry1.key = "SingleKvStoreDdmDeleteBatch004_1";
    entry1.value = "age:20";
    entry2.key = "SingleKvStoreDdmDeleteBatch004_2";
    entry2.value = "age:19";
    entry3.key = "SingleKvStoreDdmDeleteBatch004_3";
    entry3.value = "age:23";
    entries.push_back(entry1);
    entries.push_back(entry2);
    entries.push_back(entry3);

    std::vector<Key> keys;
    keys.push_back("SingleKvStoreDdmDeleteBatch004_1");
    keys.push_back("SingleKvStoreDdmDeleteBatch004_2");
    keys.push_back("          ");

    Status status1 = singleKvStore->PutBatch(entries);
    EXPECT_EQ(Status::SUCCESS, status1) << "SingleKvStore putbatch data return wrong status";

    std::vector<Entry> results1;
    singleKvStore->GetEntries("SingleKvStoreDdmDeleteBatch004_", results1);
    size_t sum1 = 3;
    EXPECT_EQ(results1.size(), sum1) << "entries size1111 is not equal 3.";

    Status status2 = singleKvStore->DeleteBatch(keys);
    EXPECT_EQ(Status::INVALID_ARGUMENT, status2) << "SingleKvStore deletebatch data return wrong status";
    std::vector<Entry> results;
    singleKvStore->GetEntries("SingleKvStoreDdmDeleteBatch004_", results);
    size_t sum = 3;
    EXPECT_EQ(results.size(), sum) << "entries size is not equal 3.";
}

/**
* @tc.name: DdmDeleteBatch005
* @tc.desc: Batch delete data when some keys are invalid.
* @tc.type: FUNC
* @tc.require: AR000DPSEA
* @tc.author: shanshuangshuang
*/
HWTEST_F(SingleKvStoreClientTest, DdmDeleteBatch005, TestSize.Level2)
{
    EXPECT_NE(nullptr, singleKvStore) << "singleKvStore is nullptr";

    // store entries to kvstore.
    std::vector<Entry> entries;
    Entry entry1, entry2, entry3;
    entry1.key = "SingleKvStoreDdmDeleteBatch005_1";
    entry1.value = "age:20";
    entry2.key = "SingleKvStoreDdmDeleteBatch005_2";
    entry2.value = "age:19";
    entry3.key = "SingleKvStoreDdmDeleteBatch005_3";
    entry3.value = "age:23";
    entries.push_back(entry1);
    entries.push_back(entry2);
    entries.push_back(entry3);

    std::vector<Key> keys;
    keys.push_back("SingleKvStoreDdmDeleteBatch005_1");
    keys.push_back("SingleKvStoreDdmDeleteBatch005_2");
    Key keyTmp = SingleGenerate1025KeyLen();
    keys.push_back(keyTmp);

    Status status1 = singleKvStore->PutBatch(entries);
    EXPECT_EQ(Status::SUCCESS, status1) << "SingleKvStore putbatch data return wrong status";

    std::vector<Entry> results1;
    singleKvStore->GetEntries("SingleKvStoreDdmDeleteBatch005_", results1);
    size_t sum1 = 3;
    EXPECT_EQ(results1.size(), sum1) << "entries111 size is not equal 3.";

    Status status2 = singleKvStore->DeleteBatch(keys);
    EXPECT_EQ(Status::INVALID_ARGUMENT, status2) << "SingleKvStore deletebatch data return wrong status";
    std::vector<Entry> results;
    singleKvStore->GetEntries("SingleKvStoreDdmDeleteBatch005_", results);
    size_t sum = 3;
    EXPECT_EQ(results.size(), sum) << "entries size is not equal 3.";
}

/**
* @tc.name: Transaction001
* @tc.desc: Batch delete data when some keys are invalid.
* @tc.type: FUNC
* @tc.require: AR000DPSEA
* @tc.author: shanshuangshuang
*/
HWTEST_F(SingleKvStoreClientTest, Transaction001, TestSize.Level2)
{
    EXPECT_NE(nullptr, singleKvStore) << "singleKvStore is nullptr";
    std::shared_ptr<KvStoreObserverTestImpl> observer = std::make_shared<KvStoreObserverTestImpl>();
    observer->ResetToZero();

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    Status status = singleKvStore->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";

    Key key1 = "SingleKvStoreTransaction001_1";
    Value value1 = "subscribe";

    std::vector<Entry> entries;
    Entry entry1, entry2, entry3;
    entry1.key = "SingleKvStoreTransaction001_2";
    entry1.value = "subscribe";
    entry2.key = "SingleKvStoreTransaction001_3";
    entry2.value = "subscribe";
    entry3.key = "SingleKvStoreTransaction001_4";
    entry3.value = "subscribe";
    entries.push_back(entry1);
    entries.push_back(entry2);
    entries.push_back(entry3);

    std::vector<Key> keys;
    keys.push_back("SingleKvStoreTransaction001_2");
    keys.push_back("ISingleKvStoreTransaction001_3");

    status = singleKvStore->StartTransaction();
    EXPECT_EQ(Status::SUCCESS, status) << "SingleKvStore startTransaction return wrong status";

    status = singleKvStore->Put(key1, value1);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "SingleKvStore put data return wrong status";
    status = singleKvStore->PutBatch(entries);
    EXPECT_EQ(Status::SUCCESS, status) << "SingleKvStore putbatch data return wrong status";
    status = singleKvStore->Delete(key1);
    EXPECT_EQ(Status::SUCCESS, status) << "SingleKvStore delete data return wrong status";
    status = singleKvStore->DeleteBatch(keys);
    EXPECT_EQ(Status::SUCCESS, status) << "SingleKvStore DeleteBatch data return wrong status";
    status = singleKvStore->Commit();
    EXPECT_EQ(Status::SUCCESS, status) << "SingleKvStore Commit return wrong status";

    usleep(200000);
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 1);

    status = singleKvStore->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
}

/**
* @tc.name: Transaction002
* @tc.desc: Batch delete data when some keys are invalid.
* @tc.type: FUNC
* @tc.require: AR000DPSEA
* @tc.author: shanshuangshuang
*/
HWTEST_F(SingleKvStoreClientTest, Transaction002, TestSize.Level2)
{
    EXPECT_NE(nullptr, singleKvStore) << "singleKvStore is nullptr";
    std::shared_ptr<KvStoreObserverTestImpl> observer = std::make_shared<KvStoreObserverTestImpl>();
    observer->ResetToZero();

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    Status status = singleKvStore->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";

    Key key1 = "SingleKvStoreTransaction002_1";
    Value value1 = "subscribe";

    std::vector<Entry> entries;
    Entry entry1, entry2, entry3;
    entry1.key = "SingleKvStoreTransaction002_2";
    entry1.value = "subscribe";
    entry2.key = "SingleKvStoreTransaction002_3";
    entry2.value = "subscribe";
    entry3.key = "SingleKvStoreTransaction002_4";
    entry3.value = "subscribe";
    entries.push_back(entry1);
    entries.push_back(entry2);
    entries.push_back(entry3);

    std::vector<Key> keys;
    keys.push_back("SingleKvStoreTransaction002_2");
    keys.push_back("SingleKvStoreTransaction002_3");

    status = singleKvStore->StartTransaction();
    EXPECT_EQ(Status::SUCCESS, status) << "SingleKvStore startTransaction return wrong status";

    status = singleKvStore->Put(key1, value1);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "SingleKvStore put data return wrong status";
    status = singleKvStore->PutBatch(entries);
    EXPECT_EQ(Status::SUCCESS, status) << "SingleKvStore putbatch data return wrong status";
    status = singleKvStore->Delete(key1);
    EXPECT_EQ(Status::SUCCESS, status) << "SingleKvStore delete data return wrong status";
    status = singleKvStore->DeleteBatch(keys);
    EXPECT_EQ(Status::SUCCESS, status) << "SingleKvStore DeleteBatch data return wrong status";
    status = singleKvStore->Rollback();
    EXPECT_EQ(Status::SUCCESS, status) << "SingleKvStore Commit return wrong status";

    usleep(200000);
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 0);
    EXPECT_EQ(static_cast<int>(observer->insertEntries_.size()), 0);
    EXPECT_EQ(static_cast<int>(observer->updateEntries_.size()), 0);
    EXPECT_EQ(static_cast<int>(observer->deleteEntries_.size()), 0);

    status = singleKvStore->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
    observer = nullptr;
}

/**
* @tc.name: DeviceSync001
* @tc.desc: Test sync enable.
* @tc.type: FUNC
* @tc.require:AR000EPAM8 AR000EPAMD
* @tc.author: HongBo
*/
HWTEST_F(SingleKvStoreClientTest, DeviceSync001, TestSize.Level1)
{
    std::shared_ptr<SingleKvStore> singleKvStore;
    DistributedKvDataManager manager;
    Options options;
    options.encrypt = true;
    options.area = EL1;
    options.kvStoreType = KvStoreType::SINGLE_VERSION;
    options.baseDir = "/data/service/el1/public/database/odmf";
    AppId appId = { "odmf" };
    StoreId storeId = { "schema_store_id001" };
    manager.GetSingleKvStore(options, appId, storeId, singleKvStore);
    ASSERT_NE(singleKvStore, nullptr) << "kvStorePtr is null.";
    auto result = singleKvStore->GetStoreId();
    EXPECT_EQ(result.storeId, "schema_store_id001");

    auto testStatus = singleKvStore->SetCapabilityEnabled(true);
    EXPECT_EQ(testStatus, Status::SUCCESS) << "set fail";
    manager.DeleteKvStore(appId, storeId, options.baseDir);
}

/**
* @tc.name: DeviceSync002
* @tc.desc: Test sync enable.
* @tc.type: FUNC
* @tc.require:SR000EPA22 AR000EPAM9
* @tc.author: HongBo
*/
HWTEST_F(SingleKvStoreClientTest, DeviceSync002, TestSize.Level1)
{
    std::shared_ptr<SingleKvStore> singleKvStore;
    DistributedKvDataManager manager;
    Options options;
    options.encrypt = true;
    options.area = EL1;
    options.kvStoreType = KvStoreType::SINGLE_VERSION;
    options.baseDir = "/data/service/el1/public/database/odmf";
    AppId appId = { "odmf" };
    StoreId storeId = { "schema_store_id002" };
    manager.GetSingleKvStore(options, appId, storeId, singleKvStore);
    ASSERT_NE(singleKvStore, nullptr) << "kvStorePtr is null.";
    auto result = singleKvStore->GetStoreId();
    EXPECT_EQ(result.storeId, "schema_store_id002");

    std::vector<std::string> local = {"A", "B"};
    std::vector<std::string> remote = {"C", "D"};
    auto testStatus = singleKvStore->SetCapabilityRange(local, remote);
    EXPECT_EQ(testStatus, Status::SUCCESS) << "set range fail";
    manager.DeleteKvStore(appId, storeId, options.baseDir);
}


/**
* @tc.name: DisableCapability
* @tc.desc: disable capability
* @tc.type: FUNC
* @tc.require: I605H3
* @tc.author: WangKai
*/
HWTEST_F(SingleKvStoreClientTest, DisableCapability, TestSize.Level1)
{
    std::shared_ptr<SingleKvStore> singleKvStore;
    DistributedKvDataManager manager;
    Options options;
    options.encrypt = true;
    options.area = EL1;
    options.kvStoreType = KvStoreType::SINGLE_VERSION;
    options.baseDir = "/data/service/el1/public/database/odmf";
    AppId appId = { "odmf" };
    StoreId storeId = { "schema_store_id001" };
    manager.GetSingleKvStore(options, appId, storeId, singleKvStore);
    ASSERT_NE(singleKvStore, nullptr) << "kvStorePtr is null.";
    auto result = singleKvStore->GetStoreId();
    EXPECT_EQ(result.storeId, "schema_store_id001");

    auto testStatus = singleKvStore->SetCapabilityEnabled(false);
    EXPECT_EQ(testStatus, Status::SUCCESS) << "set success";
    manager.DeleteKvStore(appId, storeId, options.baseDir);
}

/**
* @tc.name: SyncWithCondition001
* @tc.desc: sync device data with condition;
* @tc.type: FUNC
* @tc.require: AR000GH097
* @tc.author: liuwenhui
*/
HWTEST_F(SingleKvStoreClientTest, SyncWithCondition001, TestSize.Level1)
{
    EXPECT_NE(singleKvStore, nullptr) << "kvStorePtr is null.";
    std::vector<std::string> deviceIds = {"invalid_device_id1", "invalid_device_id2"};
    DataQuery dataQuery;
    dataQuery.KeyPrefix("name");
    auto syncStatus = singleKvStore->Sync(deviceIds, SyncMode::PUSH, dataQuery, nullptr);
    EXPECT_NE(syncStatus, Status::SUCCESS) << "sync device should not return success";
}

/**
 * @tc.name: SubscribeWithQuery001
 * desc: subscribe and sync device data with query;
 * type: FUNC
 * require: AR000GH096
 * author:taoyuxin
 */
HWTEST_F(SingleKvStoreClientTest, SubscribeWithQuery001, TestSize.Level1)
{
    EXPECT_NE(singleKvStore, nullptr) << "kvStorePtr is null.";
    std::vector<std::string> deviceIds = {"invalid_device_id1", "invalid_device_id2"};
    DataQuery dataQuery;
    dataQuery.KeyPrefix("name");
    auto syncStatus = singleKvStore->SubscribeWithQuery(deviceIds, dataQuery);
    EXPECT_NE(syncStatus, Status::SUCCESS) << "sync device should not return success";
}

/**
 * @tc.name: UnSubscribeWithQuery001
 * desc: subscribe and sync device data with query;
 * type: FUNC
 * require: SR000GH095
 * author:taoyuxin
 */
HWTEST_F(SingleKvStoreClientTest, UnSubscribeWithQuery001, TestSize.Level1)
{
    EXPECT_NE(singleKvStore, nullptr) << "kvStorePtr is null.";
    std::vector<std::string> deviceIds = {"invalid_device_id1", "invalid_device_id2"};
    DataQuery dataQuery;
    dataQuery.KeyPrefix("name");
    auto unSubscribeStatus = singleKvStore->UnsubscribeWithQuery(deviceIds, dataQuery);
    EXPECT_NE(unSubscribeStatus, Status::SUCCESS) << "sync device should not return success";
}
