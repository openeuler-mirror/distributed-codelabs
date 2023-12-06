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

#ifndef KVDB_SYNCABLE_TEST_H
#define KVDB_SYNCABLE_TEST_H

#include <map>
#include <mutex>
#include <vector>

#include "single_ver_kvdb_sync_interface.h"
#include "query_object.h"
#include "store_types.h"

namespace DistributedDB {
struct VirtualDataItem {
    Key key;
    Value value;
    Timestamp timestamp = 0;
    Timestamp writeTimestamp = 0;
    uint64_t flag = 0;
    bool isLocal = true;
    uint32_t deviceId = 0; // 0: means local
    static const uint64_t DELETE_FLAG = 0x01;
    static const uint64_t LOCAL_FLAG = 0x02;
};
class VirtualSingleVerSyncDBInterface : public SingleVerKvDBSyncInterface {
public:
    VirtualSingleVerSyncDBInterface();
    int GetInterfaceType() const override;

    void IncRefCount() override;

    void DecRefCount() override;

    std::vector<uint8_t> GetIdentifier() const override;

    int GetMetaData(const Key& key, Value& value) const override;

    int PutMetaData(const Key& key, const Value& value) override;

    int DeleteMetaData(const std::vector<Key> &keys) override;
    // Delete multiple meta data records with key prefix in a transaction.
    int DeleteMetaDataByPrefixKey(const Key &keyPrefix) const override;

    int GetAllMetaKeys(std::vector<Key>& keys) const override;

    int GetSyncData(Timestamp begin, Timestamp end, std::vector<DataItem> &dataItems,
        ContinueToken &continueStmtToken, const DataSizeSpecInfo &dataSizeInfo) const override;

    int GetSyncDataNext(std::vector<DataItem> &dataItems, ContinueToken &continueStmtToken,
        const DataSizeSpecInfo &dataSizeInfo) const override;

    void ReleaseContinueToken(ContinueToken& continueStmtToken) const override;

    void GetMaxTimestamp(Timestamp& stamp) const override;

    int RemoveDeviceData(const std::string &deviceName, bool isNeedNotify) override;

    int GetSyncData(const Key& key, VirtualDataItem& dataItem);

    int PutSyncData(const DataItem& item);

    int PutData(const Key &key, const Value &value, const Timestamp &time, int flag);

    int GetSyncData(Timestamp begin, Timestamp end, std::vector<SingleVerKvEntry *> &entries,
        ContinueToken &continueStmtToken, const DataSizeSpecInfo &dataSizeInfo) const override;

    int GetSyncData(QueryObject &query, const SyncTimeRange &timeRange, const DataSizeSpecInfo &dataSizeInfo,
        ContinueToken &continueStmtToken, std::vector<SingleVerKvEntry *> &entries) const override;

    int GetSyncDataNext(std::vector<SingleVerKvEntry *> &entries, ContinueToken &continueStmtToken,
        const DataSizeSpecInfo &dataSizeInfo) const override;

    int PutSyncDataWithQuery(const QueryObject &query, const std::vector<SingleVerKvEntry *> &entries,
        const std::string &deviceName) override;

    SchemaObject GetSchemaInfo() const override;

    bool CheckCompatible(const std::string& schema, uint8_t type) const override;

    void SetSchemaInfo(const std::string& schema);

    const KvDBProperties &GetDbProperties() const override;

    void SetSaveDataDelayTime(uint64_t milliDelayTime);

    int GetSecurityOption(SecurityOption &option) const override;

    bool IsReadable() const override;

    void SetSecurityOption(SecurityOption &option);

    void NotifyRemotePushFinished(const std::string &targetId) const override;

    int GetDatabaseCreateTimestamp(Timestamp &outTime) const override;

    int GetCompressionOption(bool &needCompressOnSync, uint8_t &compressionRate) const override;
    int GetCompressionAlgo(std::set<CompressAlgorithm> &algorithmSet) const override;

    // return E_OK if subscribe is legal, ERROR on exception.
    int CheckAndInitQueryCondition(QueryObject &query) const override;

    int InterceptData(std::vector<SingleVerKvEntry *> &entries, const std::string &sourceID,
        const std::string &targetID) const override;

    int AddSubscribe(const std::string &subscribeId, const QueryObject &query, bool needCacheSubscribe) override;

    int RemoveSubscribe(const std::string &subscribeId) override;

    int RemoveSubscribe(const std::vector<std::string> &subscribeIds) override;

    void SetBusy(bool busy);

    void PutDeviceData(const std::string &deviceName, const Key &key, const Value &value);

    void GetDeviceData(const std::string &deviceName, const Key &key, Value &value);

    void SetIdentifier(std::vector<uint8_t> &identifier);

    void SetDbProperties(KvDBProperties &kvDBProperties);

    void DelayGetSyncData(uint32_t milliDelayTime);

    void SetGetDataErrCode(int whichTime, int errCode, bool isGetDataControl);
    void ResetDataControl();
private:
    int GetSyncData(Timestamp begin, Timestamp end, uint32_t blockSize, std::vector<VirtualDataItem>& dataItems,
        ContinueToken& continueStmtToken) const;

    int GetSyncDataNext(std::vector<VirtualDataItem>& dataItems,
        uint32_t blockSize, ContinueToken& continueStmtToken) const;

    int PutSyncData(std::vector<VirtualDataItem>& dataItems, const std::string &deviceName);

    int DataControl() const;

    mutable std::map<std::vector<uint8_t>, std::vector<uint8_t>> metadata_;
    std::vector<VirtualDataItem> dbData_;
    std::map<std::string, uint32_t> deviceMapping_; // key: deviceName, value: deviceId
    uint32_t availableDeviceId_ = 0;
    std::string schema_;
    SchemaObject schemaObj_;
    KvDBProperties properties_;
    uint64_t saveDataDelayTime_ = 0;
    SecurityOption secOption_;
    bool busy_ = false;

    std::mutex deviceDataLock_;
    std::map<std::string, std::map<Key, Value>> deviceData_;
    std::vector<uint8_t> identifier_;
    uint64_t getDataDelayTime_ = 0;
    uint64_t dbCreateTime_;

    int countDown_ = -1;
    int expectedErrCode_ = E_OK;
    bool isGetDataControl_ = true; // control get data: true, control save data : false
};
}  // namespace DistributedDB

#endif // KVDB_SYNCABLE_TEST_H