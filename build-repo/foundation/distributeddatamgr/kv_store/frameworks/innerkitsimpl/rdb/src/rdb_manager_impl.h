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

#ifndef DISTRIBUTED_RDB_MANAGER_IMPL_H
#define DISTRIBUTED_RDB_MANAGER_IMPL_H

#include <map>
#include <memory>
#include <mutex>

#include "refbase.h"
#include "iremote_object.h"
#include "concurrent_map.h"
#include "rdb_types.h"
#include "visibility.h"

namespace OHOS::DistributedKv {
class KvStoreDataServiceProxy;
}

namespace OHOS::DistributedRdb {
class RdbService;
class RdbServiceProxy;
class API_EXPORT RdbManagerImpl {
public:
    static constexpr int GET_SA_RETRY_TIMES = 3;
    static constexpr int RETRY_INTERVAL = 1;
    static constexpr int WAIT_TIME = 2;

    static RdbManagerImpl &GetInstance();

    int GetRdbService(const RdbSyncerParam& param, std::shared_ptr<RdbService> &service);

    void OnRemoteDied();

    class ServiceDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit ServiceDeathRecipient(RdbManagerImpl* owner) : owner_(owner) {}
        void OnRemoteDied(const wptr<IRemoteObject> &object) override
        {
            if (owner_ != nullptr) {
                owner_->OnRemoteDied();
            }
        }
    private:
        RdbManagerImpl* owner_;
    };

private:
    RdbManagerImpl();

    ~RdbManagerImpl();

    void ResetServiceHandle();

    std::mutex mutex_;
    sptr<OHOS::DistributedKv::KvStoreDataServiceProxy> distributedDataMgr_;
    std::shared_ptr<RdbService> rdbService_;
    std::string bundleName_;
};
} // namespace OHOS::DistributedRdb
#endif
