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

#ifndef NATIVE_RDB_SQLITE_CONFIG_H
#define NATIVE_RDB_SQLITE_CONFIG_H

#include <string>
#include <vector>

#include "rdb_store_config.h"

namespace OHOS {
namespace NativeRdb {

class SqliteConfig {
public:
    explicit SqliteConfig(const RdbStoreConfig &config);
    ~SqliteConfig();
    std::string GetPath() const;
    void SetPath(std::string newPath);
    StorageMode GetStorageMode() const;
    std::string GetJournalMode() const;
    std::string GetSyncMode() const;
    std::string GetDatabaseFileType() const;
    bool IsReadOnly() const;
    bool IsEncrypt() const;
    std::string GetBundleName() const;
    bool IsCreateNecessary() const;
    void SetCreateNecessary(bool CreateNecessary);
private:
    std::string path;
    StorageMode storageMode;
    std::string journalMode;
    std::string syncMode;
    bool readOnly;
    std::string databaseFileType;

    // Encryption
    bool isEncrypt = false;
    std::string bundleName;
    bool isCreateNecessary;
};

} // namespace NativeRdb
} // namespace OHOS
#endif
