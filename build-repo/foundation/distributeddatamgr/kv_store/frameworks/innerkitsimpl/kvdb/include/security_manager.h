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
#ifndef OHOS_DISTRIBUTED_DATA_FRAMEWORKS_KVDB_SECURITY_MANAGER_H
#define OHOS_DISTRIBUTED_DATA_FRAMEWORKS_KVDB_SECURITY_MANAGER_H
#include "types.h"
#include "types_export.h"
namespace OHOS::DistributedKv {
class SecurityManager {
public:
    using DBPassword = DistributedDB::CipherPassword;
    static SecurityManager &GetInstance();
    DBPassword GetDBPassword(const std::string &name, const std::string &path, bool needCreate = false);
    bool SaveDBPassword(const std::string &name, const std::string &path, const DBPassword &key);
    void DelDBPassword(const std::string &name, const std::string &path);
    void Init();

private:
    static constexpr const char *ROOT_KEY_ALIAS = "distributeddb_client_root_key";
    static constexpr const char *HKS_BLOB_TYPE_NONCE = "Z5s0Bo571KoqwIi6";
    static constexpr const char *HKS_BLOB_TYPE_AAD = "distributeddata_client";
    static constexpr int KEY_SIZE = 32;

    SecurityManager();
    ~SecurityManager();
    std::vector<uint8_t> Random(int32_t len);
    std::vector<uint8_t> LoadKeyFromFile(const std::string &name, const std::string &path);
    bool SaveKeyToFile(const std::string &name, const std::string &path, std::vector<uint8_t> &key);
    int32_t GenerateRootKey();
    int32_t CheckRootKey();
    std::function<void()> Retry();
    std::vector<uint8_t> Encrypt(const std::vector<uint8_t> &key);
    bool Decrypt(std::vector<uint8_t> &source, std::vector<uint8_t> &key);
    
    std::vector<uint8_t> vecRootKeyAlias_{};
    std::vector<uint8_t> vecNonce_{};
    std::vector<uint8_t> vecAad_{};
};
} // namespace OHOS::DistributedKv
#endif // OHOS_DISTRIBUTED_DATA_FRAMEWORKS_KVDB_SECURITY_MANAGER_H