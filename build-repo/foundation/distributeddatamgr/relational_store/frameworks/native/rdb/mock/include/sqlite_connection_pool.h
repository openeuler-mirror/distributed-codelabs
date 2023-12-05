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

/*
根据给定的代码，`SqliteConnectionPool` 类是一个用于管理 SQLite 数据库连接的连接池。它提供了以下主要功能：

1. 连接池的创建和销毁：通过静态方法 `Create` 创建连接池，并在不需要时通过析构函数销毁连接池。

2. 获取和释放数据库连接：提供了获取读写连接和释放连接的方法，可以从连接池中获取数据库连接，用于数据库操作，并在使用完毕后将连接归还到连接池。

3. 多线程支持：支持多线程并发获取和释放数据库连接，通过互斥锁和条件变量来保证连接的安全获取和释放。

4. 连接池大小控制：连接池中可同时管理多个数据库连接，可以根据配置的数据库连接数来控制连接池的大小。

5. 事务管理：提供了一个事务栈，用于管理数据库的事务操作。

6. 数据库配置和恢复：支持数据库的配置和恢复操作，如设置数据库的本地化配置和更换数据库文件进行恢复等。

总体而言，`SqliteConnectionPool` 类是一个用于管理和维护 SQLite 数据库连接的连接池，它通过连接池中的连接来实现多线程并发访问数据库，并提供了事务管理和数据库配置恢复等功能，方便高效地进行数据库操作。
*/

#ifndef NATIVE_RDB_SQLITE_CONNECTION_POOL_H
#define NATIVE_RDB_SQLITE_CONNECTION_POOL_H

#include <condition_variable>
#include <mutex>
#include <vector>
#include <sstream>
#include <iostream>
#include <iterator>
#include <stack>

#include "rdb_store_config.h"
#include "sqlite_config.h"
#include "sqlite_connection.h"
#include "base_transaction.h"

namespace OHOS {
namespace NativeRdb {
class SqliteConnectionPool {
public:
    static SqliteConnectionPool *Create(const RdbStoreConfig &storeConfig, int &errCode);
    ~SqliteConnectionPool();
    SqliteConnection *AcquireConnection(bool isReadOnly);
    void ReleaseConnection(SqliteConnection *connection);
    int ReOpenAvailableReadConnections();
#ifdef RDB_SUPPORT_ICU
    int ConfigLocale(const std::string localeStr);
#endif
    int ChangeDbFileForRestore(const std::string newPath, const std::string backupPath,
        const std::vector<uint8_t> &newKey);
    std::stack<BaseTransaction> &getTransactionStack();

private:
    explicit SqliteConnectionPool(const RdbStoreConfig &storeConfig);
    int Init();
    void InitReadConnectionCount();
    SqliteConnection *AcquireWriteConnection();
    void ReleaseWriteConnection();
    SqliteConnection *AcquireReadConnection();
    void ReleaseReadConnection(SqliteConnection *connection);
    void CloseAllConnections();
    bool IsOverLength(const std::vector<uint8_t> &newKey);
    int InnerReOpenReadConnections();

    SqliteConfig config;
    SqliteConnection *writeConnection;
    std::mutex writeMutex;
    std::condition_variable writeCondition;
    bool writeConnectionUsed;

    std::vector<SqliteConnection *> readConnections;
    std::mutex readMutex;
    std::mutex rdbMutex;
    std::condition_variable readCondition;
    int readConnectionCount;
    int idleReadConnectionCount;
    const static int LIMITATION = 1024;

    std::stack<BaseTransaction> transactionStack;
};
} // namespace NativeRdb
} // namespace OHOS
#endif
