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

#ifndef DISTRIBUTEDDB_TYPES_H
#define DISTRIBUTEDDB_TYPES_H

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "db_constant.h"
#include "types_export.h"

namespace DistributedDB {
using Timestamp = uint64_t;
using ContinueToken = void *;
using DeviceID = std::string;
using TimeOffset = int64_t;
using ErrorCode = int;
using SyncId = uint64_t;
using WaterMark = uint64_t;
using DatabaseCorruptHandler = std::function<void()>;
using DatabaseLifeCycleNotifier = std::function<void(const std::string &, const std::string &)>;
const uint32_t MTU_SIZE = 5 * 1024 * 1024; // 5 M, 1024 is scale

struct DataItem {
    Key key;
    Value value;
    Timestamp timestamp = 0;
    uint64_t flag = 0;
    std::string origDev;
    Timestamp writeTimestamp = 0;
    std::string dev;
    bool neglect = false;
    Key hashKey{};
    static constexpr uint64_t DELETE_FLAG = 0x01;
    static constexpr uint64_t LOCAL_FLAG = 0x02;
    static constexpr uint64_t REMOVE_DEVICE_DATA_FLAG = 0x04; // only use for cachedb
    static constexpr uint64_t REMOVE_DEVICE_DATA_NOTIFY_FLAG = 0x08; // only use for cachedb
    // Only use for query sync and subscribe. ATTENTION!!! this flag should not write into mainDB.
    // Mark the changed row data does not match with query sync(or subscribe) condition.
    static constexpr uint64_t REMOTE_DEVICE_DATA_MISS_QUERY = 0x10;
    static constexpr uint64_t UPDATE_FLAG = 0X20;
};

struct PragmaPublishInfo {
    Key key;
    bool deleteLocal = false;
    bool updateTimestamp = false;
    KvStoreNbPublishAction action;
};

struct PragmaUnpublishInfo {
    Key key;
    bool isDeleteSync = false;
    bool isUpdateTime = false;
};

struct IOption {
    static constexpr int LOCAL_DATA = 1;
    static constexpr int SYNC_DATA = 2;
    int dataType = LOCAL_DATA;
};

struct DataSizeSpecInfo {
    uint32_t blockSize = MTU_SIZE;
    size_t packetSize = DBConstant::MAX_HPMODE_PACK_ITEM_SIZE;
};

enum NotificationEventType {
    DATABASE_COMMIT_EVENT = 0
};

// Following are schema related common definition
using FieldName = std::string;
using FieldPath = std::vector<std::string>;
// Normally, LEAF_FIELD_NULL will not appear in valid schema. LEAF_FIELD_LONG contain LEAF_FIELD_INTEGER, both are
// signed type and LEAF_FIELD_DOUBLE contain LEAF_FIELD_LONG. We don't parse into an array, so array are always leaf
// type. We parse into an object, LEAF_FIELD_OBJECT means an empty object, INTERNAL_FIELD_OBJECT however not empty.
enum class FieldType {
    LEAF_FIELD_NULL,
    LEAF_FIELD_BOOL,
    LEAF_FIELD_INTEGER,
    LEAF_FIELD_LONG,
    LEAF_FIELD_DOUBLE,
    LEAF_FIELD_STRING,
    LEAF_FIELD_ARRAY,
    LEAF_FIELD_OBJECT,
    INTERNAL_FIELD_OBJECT,
};
using TypeValue = std::pair<FieldType, FieldValue>; // Define for parameter convenience

// Schema compatibility check behave differently for different value source
enum class ValueSource {
    FROM_LOCAL,
    FROM_SYNC,
    FROM_DBFILE,
};
// Represent raw-value from database to avoid copy. the first is the value start pointer, the second is the length.
using RawValue = std::pair<const uint8_t *, uint32_t>;
using RawString = const std::string::value_type *;

enum class OperatePerm {
    NORMAL_PERM,
    REKEY_MONOPOLIZE_PERM,
    IMPORT_MONOPOLIZE_PERM,
    DISABLE_PERM,
};

enum SingleVerConflictResolvePolicy {
    DEFAULT_LAST_WIN = 0,
    DENY_OTHER_DEV_AMEND_CUR_DEV_DATA = 1,
};

struct SyncTimeRange {
    Timestamp beginTime = 0;
    Timestamp deleteBeginTime = 0;
    Timestamp endTime = static_cast<Timestamp>(INT64_MAX);
    Timestamp deleteEndTime = static_cast<Timestamp>(INT64_MAX);
    Timestamp lastQueryTime = 0;
    bool IsValid() const
    {
        return (beginTime <= endTime && deleteBeginTime <= deleteEndTime);
    }
};

// field types stored in sqlite
enum class StorageType {
    STORAGE_TYPE_NONE = 0,
    STORAGE_TYPE_NULL,
    STORAGE_TYPE_INTEGER,
    STORAGE_TYPE_REAL,
    STORAGE_TYPE_TEXT,
    STORAGE_TYPE_BLOB
};

// Table mode of device data for relational store
enum DistributedTableMode : int {
    COLLABORATION = 0, // Save all devices data in user table
    SPLIT_BY_DEVICE // Save device data in each table split by device
};

enum class SortType {
    NONE = 0,
    TIMESTAMP_ASC,
    TIMESTAMP_DESC
};
} // namespace DistributedDB
#endif // DISTRIBUTEDDB_TYPES_H
