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

#define LOG_TAG "RdbStoreConfig"

#include "rdb_store_config.h"

#include "logger.h"
#include "rdb_errno.h"

namespace OHOS::NativeRdb {
RdbStoreConfig::RdbStoreConfig(const RdbStoreConfig &config)
{
    name = config.GetName();
    path = config.GetPath();
    storageMode = config.GetStorageMode();
    journalMode = config.GetJournalMode();
    syncMode = config.GetSyncMode();
    readOnly = config.IsReadOnly();
    databaseFileType = config.GetDatabaseFileType();
    securityLevel = config.GetSecurityLevel();
    isCreateNecessary_ = config.IsCreateNecessary();
}

RdbStoreConfig::RdbStoreConfig(const std::string &name, StorageMode storageMode, bool isReadOnly,
    const std::vector<uint8_t> &encryptKey, const std::string &journalMode, const std::string &syncMode,
    const std::string &databaseFileType, SecurityLevel securityLevel, bool isCreateNecessary)
    : name(name),
      path(name),
      storageMode(storageMode),
      journalMode(journalMode),
      syncMode(syncMode),
      readOnly(isReadOnly),
      databaseFileType(databaseFileType),
      securityLevel(securityLevel),
      isCreateNecessary_(isCreateNecessary)
{
}

RdbStoreConfig::~RdbStoreConfig() = default;

/**
 * Obtains the database name.
 */
std::string RdbStoreConfig::GetName() const
{
    return name;
}

/**
 * Obtains the database path.
 */
std::string RdbStoreConfig::GetPath() const
{
    return path;
}

/**
 * Obtains the storage mode.
 */
StorageMode RdbStoreConfig::GetStorageMode() const
{
    return storageMode;
}

/**
 * Obtains the journal mode in this {@code StoreConfig} object.
 */
std::string RdbStoreConfig::GetJournalMode() const
{
    return journalMode;
}

/**
 * Obtains the synchronization mode in this {@code StoreConfig} object.
 */
std::string RdbStoreConfig::GetSyncMode() const
{
    return syncMode;
}

/**
 * Checks whether the database is read-only.
 */
bool RdbStoreConfig::IsReadOnly() const
{
    return readOnly;
}

/**
 * Checks whether the database is memory.
 */
bool RdbStoreConfig::IsMemoryRdb() const
{
    return GetStorageMode() == StorageMode::MODE_MEMORY;
}

/**
 * Obtains the database file type in this {@code StoreConfig} object.
 */
std::string RdbStoreConfig::GetDatabaseFileType() const
{
    return databaseFileType;
}

void RdbStoreConfig::SetName(std::string name)
{
    this->name = std::move(name);
}

/**
 * Sets the journal mode  for the object.
 */
void RdbStoreConfig::SetJournalMode(JournalMode journalMode)
{
    this->journalMode = GetJournalModeValue(journalMode);
}

void RdbStoreConfig::SetDatabaseFileType(DatabaseFileType type)
{
    this->databaseFileType = GetDatabaseFileTypeValue(type);
}

/**
 * Sets the path  for the object.
 */
void RdbStoreConfig::SetPath(std::string path)
{
    this->path = std::move(path);
}

void RdbStoreConfig::SetStorageMode(StorageMode storageMode)
{
    this->storageMode = storageMode;
}

void RdbStoreConfig::SetReadOnly(bool readOnly)
{
    this->readOnly = readOnly;
}

#if !defined(WINDOWS_PLATFORM) && !defined(MAC_PLATFORM)
int RdbStoreConfig::SetDistributedType(DistributedType type)
{
    if (type < DistributedType::RDB_DEVICE_COLLABORATION || type >= DistributedType::RDB_DISTRIBUTED_TYPE_MAX) {
        LOG_ERROR("type is invalid");
        return E_ERROR;
    }
    distributedType_ = type;
    return E_OK;
}

DistributedType RdbStoreConfig::GetDistributedType() const
{
    return distributedType_;
}
#endif

int RdbStoreConfig::SetBundleName(const std::string &bundleName)
{
    if (bundleName.empty()) {
        LOG_ERROR("bundleName is empty");
        return E_ERROR;
    }
    bundleName_ = bundleName;
    return E_OK;
}

std::string RdbStoreConfig::GetBundleName() const
{
    return bundleName_;
}

void RdbStoreConfig::SetModuleName(const std::string &moduleName)
{
    moduleName_ = moduleName;
}

std::string RdbStoreConfig::GetModuleName() const
{
    return moduleName_;
}

void RdbStoreConfig::SetServiceName(const std::string &serviceName)
{
    SetBundleName(serviceName);
}

void RdbStoreConfig::SetArea(int32_t area)
{
    area_ = area + 1;
}

int32_t RdbStoreConfig::GetArea() const
{
    return area_;
}

std::string RdbStoreConfig::GetJournalModeValue(JournalMode journalMode)
{
    std::string value = "";

    switch (journalMode) {
        case JournalMode::MODE_DELETE:
            return "DELETE";
        case JournalMode::MODE_TRUNCATE:
            return "TRUNCATE";
        case JournalMode::MODE_PERSIST:
            return  "PERSIST";
        case JournalMode::MODE_MEMORY:
            return "MEMORY";
        case JournalMode::MODE_WAL:
            return "WAL";
        case JournalMode::MODE_OFF:
            return "OFF";
        default:
            break;
    }
    return value;
}

std::string RdbStoreConfig::GetSyncModeValue(SyncMode syncMode)
{
    std::string value = "";
    switch (syncMode) {
        case SyncMode::MODE_OFF:
            return "MODE_OFF";
        case SyncMode::MODE_NORMAL:
            return "MODE_NORMAL";
        case SyncMode::MODE_FULL:
            return "MODE_FULL";
        case SyncMode::MODE_EXTRA:
            return "MODE_EXTRA";
        default:
            break;
    }

    return value;
}

std::string RdbStoreConfig::GetDatabaseFileTypeValue(DatabaseFileType databaseFileType)
{
    std::string value = "";
    switch (databaseFileType) {
        case DatabaseFileType::NORMAL:
            return "db";
        case DatabaseFileType::BACKUP:
            return "backup";
        case DatabaseFileType::CORRUPT:
            return "corrupt";
        default:
            break;
    }

    return value;
}

void RdbStoreConfig::SetSecurityLevel(SecurityLevel sl)
{
    securityLevel = sl;
}

SecurityLevel RdbStoreConfig::GetSecurityLevel() const
{
    return securityLevel;
}

void RdbStoreConfig::SetEncryptStatus(const bool status)
{
    this->isEncrypt_ = status;
}

bool RdbStoreConfig::IsEncrypt() const
{
    return this->isEncrypt_;
}
std::string RdbStoreConfig::GetUri() const
{
    return uri_;
}

void RdbStoreConfig::SetUri(const std::string &uri)
{
    uri_ = uri;
}

std::string RdbStoreConfig::GetReadPermission() const
{
    return readPermission_;
}

void RdbStoreConfig::SetReadPermission(const std::string &permission)
{
    readPermission_ = permission;
}

std::string RdbStoreConfig::GetWritePermission() const
{
    return writePermission_;
}

void RdbStoreConfig::SetWritePermission(const std::string &permission)
{
    writePermission_ = permission;
}
bool RdbStoreConfig::IsCreateNecessary() const
{
    return isCreateNecessary_;
}

void RdbStoreConfig::SetCreateNecessary(bool isCreateNecessary)
{
    isCreateNecessary_ = isCreateNecessary;
}
} // namespace OHOS::NativeRdb
