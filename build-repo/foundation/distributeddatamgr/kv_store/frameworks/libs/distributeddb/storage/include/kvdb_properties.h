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

#ifndef KV_DB_PROPERTIES_H
#define KV_DB_PROPERTIES_H

#include <map>
#include <string>
#include <vector>

#include "db_properties.h"
#include "schema_object.h"

namespace DistributedDB {
class KvDBProperties final : public DBProperties {
public:
    KvDBProperties();
    ~KvDBProperties() override;

    // Get the sub directory for different type database.
    static std::string GetStoreSubDirectory(int type);

    // Get the password
    void GetPassword(CipherType &type, CipherPassword &password) const;

    // Set the password
    void SetPassword(CipherType type, const CipherPassword &password);

    // is schema exist
    bool IsSchemaExist() const;

    // set schema
    void SetSchema(const SchemaObject &schema);

    // get schema
    SchemaObject GetSchema() const;

    // If it does not exist, use the int map default value 0
    int GetSecLabel() const;

    int GetSecFlag() const;

    // Get schema const reference if you can guarantee the lifecycle of this KvDBProperties
    // The upper code will not change the schema if it is already set
    const SchemaObject &GetSchemaConstRef() const;

    static const std::string FILE_NAME;
    static const std::string SYNC_MODE;
    static const std::string MEMORY_MODE;
    static const std::string ENCRYPTED_MODE;
    static const std::string FIRST_OPEN_IS_READ_ONLY;
    static const std::string CREATE_DIR_BY_STORE_ID_ONLY;
    static const std::string SECURITY_LABEL;
    static const std::string SECURITY_FLAG;
    static const std::string CONFLICT_RESOLVE_POLICY;
    static const std::string CHECK_INTEGRITY;
    static const std::string RM_CORRUPTED_DB;
    static const std::string COMPRESS_ON_SYNC;
    static const std::string COMPRESSION_RATE;
    static const std::string LOCAL_ONLY;

    static const int LOCAL_TYPE = 1;
    static const int MULTI_VER_TYPE = 2;
    static const int SINGLE_VER_TYPE = 3;

private:
    CipherType cipherType_;
    CipherPassword password_;
    SchemaObject schema_;
};
} // namespace DistributedDB

#endif // KV_DB_PROPERTIES_H
