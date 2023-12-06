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

#define LOG_TAG "KvStoreThreadPool"

#include "kv_store_thread_pool_impl.h"
#include "log_print.h"

namespace OHOS {
namespace DistributedKv {
std::shared_ptr<KvStoreThreadPool> KvStoreThreadPool::GetPool(int poolSize, std::string poolName, bool startImmediately)
{
    std::shared_ptr<KvStoreThreadPoolImpl> poolImpl =
        std::make_shared<KvStoreThreadPoolImpl>(poolSize, std::move(poolName), startImmediately);
    if (poolImpl == nullptr) {
        return nullptr;
    }
    return std::shared_ptr<KvStoreThreadPool>(std::dynamic_pointer_cast<KvStoreThreadPool>(poolImpl));
}

std::string KvStoreThreadPool::GetPoolName()
{
    return poolName_;
}
} // namespace DistributedKv
} // namespace OHOS
