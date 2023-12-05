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

/*
这段代码定义了一个名为 `IKvDB` 的接口类，继承了 `RefObject` 基类，用于提供一组接口函数，用于操作键值数据库。接口类中声明了一些纯虚函数，需要在派生类中进行实现。

其中，`Open` 函数用于打开数据库，需要传入一个 `KvDBProperties` 类型的对象作为参数。`GetMyProperties` 函数用于获取当前数据库的属性对象。`GetDBConnection` 函数用于创建一个数据库连接对象，需要传入一个整型引用类型的参数 `errCode`，表示创建连接时可能出现的错误码。`OnClose` 函数用于注册一个回调函数，在所有连接都被释放时调用。

`OpenPerformanceAnalysis` 和 `ClosePerformanceAnalysis` 用于开启和关闭性能分析功能。`WakeUpSyncer` 用于唤醒同步线程。`SetCorruptHandler` 用于设置数据库损坏处理器。`RemoveKvDB` 用于移除指定属性的数据库，`GetKvDBSize` 用于获取指定属性的数据库的大小。

`EnableAutonomicUpgrade` 用于启用自主升级功能。`CheckIntegrity` 用于检查数据库完整性。`GetStorePath` 用于获取数据库存储路径。`Dump` 用于将数据库信息转储到指定的文件描述符中。

该接口类的定义中还声明了一些宏，如 `DISABLE_COPY_ASSIGN_MOVE`，用于禁止对象的复制、赋值和移动操作。由于该接口类中的函数都是纯虚函数，需要在派生类中进行实现，因此该接口类也被称为纯虚类。
*/

#ifndef I_KV_DB_H
#define I_KV_DB_H

#include <functional>
#include <string>

#include "ref_object.h"
#include "macro_utils.h"
#include "kvdb_properties.h"
#include "ikvdb_connection.h"

namespace DistributedDB {
class IKvDB : public virtual RefObject {
public:
    IKvDB() = default;
    ~IKvDB() override {}
    DISABLE_COPY_ASSIGN_MOVE(IKvDB);

    // Open the database.
    virtual int Open(const KvDBProperties &kvDBProp) = 0;

    // Get the properties object of this database.
    virtual const KvDBProperties &GetMyProperties() const = 0;

    // Create a db connection.
    virtual IKvDBConnection *GetDBConnection(int &errCode) = 0;

    // Register callback invoked when all connections released.
    virtual void OnClose(const std::function<void(void)> &func) = 0;

    virtual void OpenPerformanceAnalysis() = 0;

    virtual void ClosePerformanceAnalysis() = 0;

    virtual void WakeUpSyncer() = 0;

    virtual void SetCorruptHandler(const DatabaseCorruptHandler &handler) = 0;

    virtual int RemoveKvDB(const KvDBProperties &properties) = 0;
    virtual int GetKvDBSize(const KvDBProperties &properties, uint64_t &size) const = 0;

    virtual void EnableAutonomicUpgrade() = 0;

    virtual int CheckIntegrity() const = 0;

    virtual std::string GetStorePath() const = 0;

    virtual void Dump(int fd) = 0;
};
} // namespace DistributedDB

#endif // I_KV_DB_H
