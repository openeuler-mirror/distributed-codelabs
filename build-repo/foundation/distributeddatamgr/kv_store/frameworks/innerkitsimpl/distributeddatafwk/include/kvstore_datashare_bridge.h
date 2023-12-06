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

#ifndef KVSTORE_DATASHARE_BRIDGE_H
#define KVSTORE_DATASHARE_BRIDGE_H

#include "kvstore_result_set.h"
#include "single_kvstore.h"
// #include "result_set_bridge.h"
// #include "datashare_errno.h"

namespace OHOS {
namespace DistributedKv {
class KvStoreDataShareBridge : public DataShare::ResultSetBridge {
public:
    KvStoreDataShareBridge(std::shared_ptr<KvStoreResultSet> kvResultSet);

    ~KvStoreDataShareBridge() = default;

    int GetRowCount(int32_t &count) override;

    int GetAllColumnNames(std::vector<std::string> &columnNames) override;

    int OnGo(int32_t startRowIndex, int32_t targetRowIndex, DataShare::ResultSetBridge::Writer &writer) override;
  
private:
    int Count();

    bool FillBlock(int startRowIndex, DataShare::ResultSetBridge::Writer &writer);

    static constexpr int32_t INVALID_COUNT = -1;

    int32_t resultRowCount {INVALID_COUNT};

    std::shared_ptr<KvStoreResultSet> kvResultSet_;
};
} // namespace DistributedKv
} // namespace OHOS
#endif // KVSTORE_DATASHARE_BRIDGE_H
