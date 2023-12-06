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

#ifndef DISTRIBUTEDDB_CONSTANT_H
#define DISTRIBUTEDDB_CONSTANT_H

#include <string>

namespace DistributedDB {
class DBConstant {
public:
    static constexpr size_t MAX_KEY_SIZE = 1024;
    static constexpr size_t MAX_VALUE_SIZE = 4194304;
    static constexpr size_t MAX_BATCH_SIZE = 128;
    static constexpr size_t MAX_DEV_LENGTH = 128;
    static constexpr size_t MAX_TRANSACTION_ENTRY_SIZE = 128;

    static constexpr size_t MAX_DATA_DIR_LENGTH = 512;

    static constexpr size_t MAX_INKEYS_SIZE = 128;
    static constexpr size_t MAX_SQL_ARGS_COUNT = 100;

    static constexpr int DB_TYPE_LOCAL = 1;
    static constexpr int DB_TYPE_MULTI_VER = 2;
    static constexpr int DB_TYPE_SINGLE_VER = 3;

    static constexpr int QUEUED_SYNC_LIMIT_DEFAULT = 32;
    static constexpr int QUEUED_SYNC_LIMIT_MIN = 1;
    static constexpr int QUEUED_SYNC_LIMIT_MAX = 4096;

    static constexpr int MAX_DEVICES_SIZE = 100;
    static constexpr int MAX_COMMIT_SIZE = 1000000;
    static constexpr int MAX_ENTRIES_SIZE = 1000000;

    static constexpr uint32_t MAX_COLUMN  = 32767;

    static constexpr int MAX_REMOTEDATA_SIZE = 4194304;  // 4M.

    static constexpr int DEFAULT_ITER_TIMES = 5000;

    // In querySync, when getting query data finished,
    // if the block size reach the half of max block size, will get deleted data next;
    // if the block size not reach the half of max block size, will not get deleted data.
    static constexpr float QUERY_SYNC_THRESHOLD = 0.50;

    static constexpr uint64_t MAX_USER_ID_LENGTH = 128;
    static constexpr uint64_t MAX_APP_ID_LENGTH = 128;
    static constexpr uint64_t MAX_STORE_ID_LENGTH = 128;

    static const std::string MULTI_SUB_DIR;
    static const std::string SINGLE_SUB_DIR;
    static const std::string LOCAL_SUB_DIR;

    static const std::string MAINDB_DIR;
    static const std::string METADB_DIR;
    static const std::string CACHEDB_DIR;

    static const std::string LOCAL_DATABASE_NAME;
    static const std::string MULTI_VER_DATA_STORE;
    static const std::string MULTI_VER_COMMIT_STORE;
    static const std::string MULTI_VER_VALUE_STORE;
    static const std::string MULTI_VER_META_STORE;
    static const std::string SINGLE_VER_DATA_STORE;
    static const std::string SINGLE_VER_META_STORE;
    static const std::string SINGLE_VER_CACHE_STORE;

    static const std::string SQLITE_URL_PRE;
    static const std::string SQLITE_DB_EXTENSION;
    static const std::string SQLITE_MEMDB_IDENTIFY;
    static const std::string SCHEMA_KEY;
    static const std::string RELATIONAL_SCHEMA_KEY;

    static const std::string PATH_POSTFIX_UNPACKED;
    static const std::string PATH_POSTFIX_IMPORT_BACKUP;
    static const std::string PATH_POSTFIX_IMPORT_ORIGIN;
    static const std::string PATH_POSTFIX_IMPORT_DUP;
    static const std::string PATH_POSTFIX_EXPORT_BACKUP;
    static const std::string PATH_POSTFIX_DB_INCOMPLETE; // use for make sure create datebase and set label complete

    static const std::string REKEY_FILENAME_POSTFIX_PRE;
    static const std::string REKEY_FILENAME_POSTFIX_OK;
    static const std::string UPGRADE_POSTFIX;
    static const std::string SET_SECOPT_POSTFIX; // used for make sure meta split upgrade atomically

    static const std::string PATH_BACKUP_POSTFIX;

    static const std::string ID_CONNECTOR;

    static const std::string DELETE_KVSTORE_REMOVING;
    static const std::string DB_LOCK_POSTFIX;

    static const std::string SUBSCRIBE_QUERY_PREFIX;
    static const std::string TRIGGER_REFERENCES_NEW;
    static const std::string TRIGGER_REFERENCES_OLD;

    static const std::string UPDATE_META_FUNC;

    static const std::string SYSTEM_TABLE_PREFIX;

    static constexpr uint32_t AUTO_SYNC_TIMEOUT = 5000; // 5s
    static constexpr uint32_t MANUAL_SYNC_TIMEOUT = 5000; // 5s

    static const size_t MAX_NORMAL_PACK_ITEM_SIZE = 4000;
    static const size_t MAX_HPMODE_PACK_ITEM_SIZE = 2000; // slide window mode to reduce last ack transfer time

    static constexpr uint32_t MIN_MTU_SIZE = 1024; // 1KB
    static constexpr uint32_t MAX_MTU_SIZE = 5242880; // 5MB

    static constexpr uint32_t MIN_TIMEOUT = 5000; // 5s
    static constexpr uint32_t MAX_TIMEOUT = 60000; // 60s

    static constexpr uint8_t DEFAULT_COMPTRESS_RATE = 100;

    static constexpr size_t MAX_SYNC_BLOCK_SIZE = 31457280; // 30MB

    static constexpr int DOUBLE_PRECISION = 15;
    static constexpr int MAX_DISTRIBUTED_TABLE_COUNT = 32;

    static constexpr uint64_t MAX_LOG_SIZE_HIGH = 0x400000000ULL; // 16GB
    static constexpr uint64_t MAX_LOG_SIZE_LOW = 0x400000ULL; // 4MB
    static constexpr uint64_t MAX_LOG_SIZE_DEFAULT = 0x40000000ULL; // 1GB

    static constexpr int DEF_LIFE_CYCLE_TIME = 60000; // 60S

    static constexpr int RELATIONAL_LOG_TABLE_FIELD_NUM = 7; // field num is relational distributed log table

    static constexpr uint64_t IGNORE_CONNECTION_ID = 0;
    // For relational
    static const std::string RELATIONAL_PREFIX;
    static const std::string TIMESTAMP_ALIAS;

    static const std::string LOG_TABLE_VERSION_1;
    static const std::string LOG_TABLE_VERSION_2;
    static const std::string LOG_TABLE_VERSION_CURRENT;

    static const std::string LOG_TABLE_VERSION_KEY;

    static constexpr uint32_t MAX_CONDITION_KEY_LEN = 128;
    static constexpr uint32_t MAX_CONDITION_VALUE_LEN = 128;
    static constexpr uint32_t MAX_CONDITION_COUNT = 32;
};
} // namespace DistributedDB

#endif // DISTRIBUTEDDB_CONSTANT_H
