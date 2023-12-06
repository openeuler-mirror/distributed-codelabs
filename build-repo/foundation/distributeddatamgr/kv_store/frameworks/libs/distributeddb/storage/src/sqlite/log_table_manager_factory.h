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

#ifndef LOG_TABLE_MANAGER_FACTORY_H
#define LOG_TABLE_MANAGER_FACTORY_H
#include <memory>
#include "sqlite_log_table_manager.h"
#include "types_export.h"

namespace DistributedDB {
class LogTableManagerFactory final {
public:
    static std::unique_ptr<SqliteLogTableManager> GetTableManager(DistributedTableMode mode);

private:
    LogTableManagerFactory() {}
    ~LogTableManagerFactory() {}
};
}
#endif // LOG_TABLE_MANAGER_FACTORY_H