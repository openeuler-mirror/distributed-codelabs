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
#include "db_common.h"
#include "generic_single_ver_kv_entry.h"
#include "platform_specific.h"
#include "relational_remote_query_continue_token.h"
#include "runtime_context.h"
#include "virtual_relational_ver_sync_db_interface.h"
#include "virtual_single_ver_sync_db_Interface.h"

namespace DistributedDB {
namespace {
    int GetEntriesFromItems(std::vector<SingleVerKvEntry *> &entries, const std::vector<DataItem> &dataItems)
    {
        int errCode = E_OK;
        for (const auto &item : dataItems) {
            auto entry = new (std::nothrow) GenericSingleVerKvEntry();
            if (entry == nullptr) {
                LOGE("Create entry failed.");
                errCode = -E_OUT_OF_MEMORY;
                break;
            }
            DataItem storageItem;
            storageItem.key = item.key;
            storageItem.value = item.value;
            storageItem.flag = item.flag;
            storageItem.timestamp = item.timestamp;
            storageItem.writeTimestamp = item.writeTimestamp;
            storageItem.hashKey = item.hashKey;
            entry->SetEntryData(std::move(storageItem));
            entries.push_back(entry);
        }
        if (errCode != E_OK) {
            LOGD("[GetEntriesFromItems] failed:%d", errCode);
            for (auto &kvEntry : entries) {
                delete kvEntry;
                kvEntry = nullptr;
            }
            entries.clear();
        }
        LOGD("[GetEntriesFromItems] size:%zu", dataItems.size());
        return errCode;
    }

    std::string GetStr(const std::vector<uint8_t> &vec)
    {
        std::string str;
        DBCommon::VectorToString(vec, str);
        return str;
    }
}

VirtualRelationalVerSyncDBInterface::VirtualRelationalVerSyncDBInterface()
{
    (void)OS::GetCurrentSysTimeInMicrosecond(dbCreateTime_);
    LOGD("virtual device init db createTime");
}

int VirtualRelationalVerSyncDBInterface::PutSyncDataWithQuery(const QueryObject &object,
    const std::vector<SingleVerKvEntry *> &entries, const std::string &deviceName)
{
    LOGD("[PutSyncData] size %zu", entries.size());
    std::vector<DataItem> dataItems;
    for (auto itemEntry : entries) {
        auto *entry = static_cast<GenericSingleVerKvEntry *>(itemEntry);
        if (entry != nullptr) {
            DataItem item;
            item.origDev = entry->GetOrigDevice();
            item.flag = entry->GetFlag();
            item.timestamp = entry->GetTimestamp();
            item.writeTimestamp = entry->GetWriteTimestamp();
            entry->GetKey(item.key);
            entry->GetValue(item.value);
            entry->GetHashKey(item.hashKey);
            dataItems.push_back(item);
        }
    }
    OptTableDataWithLog optTableDataWithLog;
    optTableDataWithLog.tableName = object.GetTableName();
    int errCode = DataTransformer::TransformDataItem(dataItems, localFieldInfo_,
        localFieldInfo_, optTableDataWithLog);
    if (errCode != E_OK) {
        return errCode;
    }
    for (const auto &optRowDataWithLog : optTableDataWithLog.dataList) {
        VirtualRowData virtualRowData;
        virtualRowData.logInfo = optRowDataWithLog.logInfo;
        size_t index = 0;
        for (const auto &optItem : optRowDataWithLog.optionalData) {
            if (index >= localFieldInfo_.size()) {
                break;
            }
            DataValue dataValue = std::move(optItem);
            LOGD("type:%d", static_cast<int>(optItem.GetType()));
            virtualRowData.objectData.PutDataValue(localFieldInfo_[index].GetFieldName(), dataValue);
            index++;
        }
        syncData_[object.GetTableName()][GetStr(virtualRowData.logInfo.hashKey)] = virtualRowData;
    }
    LOGD("tableName %s", optTableDataWithLog.tableName.c_str());
    return errCode;
}

int VirtualRelationalVerSyncDBInterface::PutLocalData(const std::vector<VirtualRowData> &dataList,
    const std::string &tableName)
{
    for (const auto &item : dataList) {
        localData_[tableName][GetStr(item.logInfo.hashKey)] = item;
    }
    return E_OK;
}

int VirtualRelationalVerSyncDBInterface::GetSyncData(QueryObject &query,
    const SyncTimeRange &timeRange, const DataSizeSpecInfo &dataSizeInfo,
    ContinueToken &continueStmtToken, std::vector<SingleVerKvEntry *> &entries) const
{
    if (localData_.find(query.GetTableName()) == localData_.end()) {
        LOGD("[GetSyncData] No Data Return");
        return E_OK;
    }
    std::vector<DataItem> dataItemList;
    TableDataWithLog tableDataWithLog = {query.GetTableName(), {}};
    for (const auto &[hashKey, virtualData] : localData_[query.GetTableName()]) {
        if (virtualData.logInfo.timestamp < timeRange.beginTime ||
            virtualData.logInfo.timestamp >= timeRange.endTime) {
            LOGD("ignore hashkey %s", DBCommon::TransferStringToHex(hashKey).c_str());
            continue;
        }
        RowDataWithLog rowData;
        for (const auto &field : localFieldInfo_) {
            DataValue dataValue;
            (void)virtualData.objectData.GetDataValue(field.GetFieldName(), dataValue);
            rowData.rowData.push_back(std::move(dataValue));
        }
        rowData.logInfo = virtualData.logInfo;
        tableDataWithLog.dataList.push_back(rowData);
    }

    int errCode = DataTransformer::TransformTableData(tableDataWithLog, localFieldInfo_, dataItemList);
    if (errCode != E_OK) {
        return errCode;
    }
    continueStmtToken = nullptr;
    return GetEntriesFromItems(entries, dataItemList);
}

RelationalSchemaObject VirtualRelationalVerSyncDBInterface::GetSchemaInfo() const
{
    return schemaObj_;
}


void VirtualRelationalVerSyncDBInterface::SetSchemaInfo(const RelationalSchemaObject &schema)
{
    schemaObj_ = schema;
}

int VirtualRelationalVerSyncDBInterface::GetDatabaseCreateTimestamp(Timestamp &outTime) const
{
    outTime = dbCreateTime_;
    return E_OK;
}

int VirtualRelationalVerSyncDBInterface::GetBatchMetaData(const std::vector<Key> &keys,
    std::vector<Entry> &entries) const
{
    int errCode = E_OK;
    for (const auto &key : keys) {
        Entry entry;
        entry.key = key;
        errCode = GetMetaData(key, entry.value);
        if (errCode != E_OK) {
            return errCode;
        }
        entries.push_back(entry);
    }
    return errCode;
}

int VirtualRelationalVerSyncDBInterface::PutBatchMetaData(std::vector<Entry> &entries)
{
    int errCode = E_OK;
    for (const auto &entry : entries) {
        errCode = PutMetaData(entry.key, entry.value);
        if (errCode != E_OK) {
            return errCode;
        }
    }
    return errCode;
}

std::vector<QuerySyncObject> VirtualRelationalVerSyncDBInterface::GetTablesQuery()
{
    return {};
}

int VirtualRelationalVerSyncDBInterface::LocalDataChanged(int notifyEvent, std::vector<QuerySyncObject> &queryObj)
{
    return E_OK;
}

int VirtualRelationalVerSyncDBInterface::GetInterfaceType() const
{
    return SYNC_RELATION;
}

void VirtualRelationalVerSyncDBInterface::IncRefCount()
{
}

void VirtualRelationalVerSyncDBInterface::DecRefCount()
{
}

std::vector<uint8_t> VirtualRelationalVerSyncDBInterface::GetIdentifier() const
{
    return {};
}

void VirtualRelationalVerSyncDBInterface::GetMaxTimestamp(Timestamp &stamp) const
{
    for (const auto &item : syncData_) {
        for (const auto &entry : item.second) {
            if (stamp < entry.second.logInfo.timestamp) {
                stamp = entry.second.logInfo.timestamp;
            }
        }
    }
    LOGD("VirtualSingleVerSyncDBInterface::GetMaxTimestamp time = %" PRIu64, stamp);
}

int VirtualRelationalVerSyncDBInterface::GetMetaData(const Key &key, Value &value) const
{
    auto iter = metadata_.find(key);
    if (iter != metadata_.end()) {
        value = iter->second;
        return E_OK;
    }
    return -E_NOT_FOUND;
}

int VirtualRelationalVerSyncDBInterface::PutMetaData(const Key &key, const Value &value)
{
    metadata_[key] = value;
    return E_OK;
}

int VirtualRelationalVerSyncDBInterface::DeleteMetaData(const std::vector<Key> &keys)
{
    for (const auto &key : keys) {
        (void)metadata_.erase(key);
    }
    return E_OK;
}

int VirtualRelationalVerSyncDBInterface::DeleteMetaDataByPrefixKey(const Key &keyPrefix) const
{
    size_t prefixKeySize = keyPrefix.size();
    for (auto iter = metadata_.begin();iter != metadata_.end();) {
        if (prefixKeySize <= iter->first.size() &&
            keyPrefix == Key(iter->first.begin(), std::next(iter->first.begin(), prefixKeySize))) {
            iter = metadata_.erase(iter);
        } else {
            ++iter;
        }
    }
    return E_OK;
}

int VirtualRelationalVerSyncDBInterface::GetAllMetaKeys(std::vector<Key> &keys) const
{
    for (const auto &iter : metadata_) {
        keys.push_back(iter.first);
    }
    LOGD("GetAllMetaKeys size %zu", keys.size());
    return E_OK;
}

const RelationalDBProperties &VirtualRelationalVerSyncDBInterface::GetDbProperties() const
{
    return rdbProperties_;
}

void VirtualRelationalVerSyncDBInterface::SetLocalFieldInfo(const std::vector<FieldInfo> &localFieldInfo)
{
    localFieldInfo_.clear();
    localFieldInfo_ = localFieldInfo;
}

int VirtualRelationalVerSyncDBInterface::GetAllSyncData(const std::string &tableName,
    std::vector<VirtualRowData> &data)
{
    if (syncData_.find(tableName) == syncData_.end()) {
        return -E_NOT_FOUND;
    }
    for (const auto &entry : syncData_[tableName]) {
        data.push_back(entry.second);
    }
    return E_OK;
}

int VirtualRelationalVerSyncDBInterface::GetVirtualSyncData(const std::string &tableName,
    const std::string &hashKey, VirtualRowData &data)
{
    if (syncData_.find(tableName) == syncData_.end()) {
        return -E_NOT_FOUND;
    }
    if (syncData_.find(hashKey) == syncData_.end()) {
        return -E_NOT_FOUND;
    }
    data = syncData_[tableName][hashKey];
    return E_OK;
}

void VirtualRelationalVerSyncDBInterface::EraseSyncData(const std::string &tableName)
{
    if (syncData_.find(tableName) == syncData_.end()) {
        return;
    }
    syncData_.erase(tableName);
}

int VirtualRelationalVerSyncDBInterface::CreateDistributedDeviceTable(const std::string &device,
    const RelationalSyncStrategy &syncStrategy)
{
    return permitCreateDistributedTable_ ? E_OK : -E_NOT_SUPPORT;
}

int VirtualRelationalVerSyncDBInterface::RegisterSchemaChangedCallback(const std::function<void()> &onSchemaChanged)
{
    return E_OK;
}

void VirtualRelationalVerSyncDBInterface::SetTableInfo(const TableInfo &tableInfo)
{
    schemaObj_.AddRelationalTable(tableInfo);
}

int VirtualRelationalVerSyncDBInterface::GetMaxTimestamp(const std::string &tableName, Timestamp &timestamp) const
{
    (void)tableName;
    timestamp = 0;
    return E_OK;
}

int VirtualRelationalVerSyncDBInterface::ExecuteQuery(const PreparedStmt &prepStmt, size_t packetSize,
    RelationalRowDataSet &data, ContinueToken &token) const
{
    return E_OK;
}

const RelationalDBProperties &VirtualRelationalVerSyncDBInterface::GetRelationalDbProperties() const
{
    return rdbProperties_;
}

void VirtualRelationalVerSyncDBInterface::SetPermitCreateDistributedTable(bool permitCreateDistributedTable)
{
    permitCreateDistributedTable_ = permitCreateDistributedTable;
}

void ObjectData::PutDataValue(const std::string &fieldName, const DataValue &value) const
{
    fieldData[fieldName] = value;
}

int ObjectData::GetDataValue(const std::string &fieldName, DataValue &value) const
{
    if (fieldData.find(fieldName) == fieldData.end()) {
        return -E_NOT_FOUND;
    }
    value = fieldData[fieldName];
    return E_OK;
}

int VirtualRelationalVerSyncDBInterface::GetSecurityOption(SecurityOption &option) const
{
    return RuntimeContext::GetInstance()->GetSecurityOption("", option);
}

void VirtualRelationalVerSyncDBInterface::ReleaseRemoteQueryContinueToken(ContinueToken &token) const
{
    auto remoteToken = static_cast<RelationalRemoteQueryContinueToken *>(token);
    delete remoteToken;
    remoteToken = nullptr;
    token = nullptr;
}
}
#endif