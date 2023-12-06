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

#ifndef I_KVSTORE_CLIENT_DEATH_OBSERVER_H
#define I_KVSTORE_CLIENT_DEATH_OBSERVER_H

#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"
#include "visibility.h"

namespace OHOS {
namespace DistributedKv {
class IKvStoreClientDeathObserver : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.DistributedKv.IKvStoreClientDeathObserver");
};

class KvStoreClientDeathObserverStub : public IRemoteStub<IKvStoreClientDeathObserver> {
};

class API_EXPORT KvStoreClientDeathObserverProxy : public IRemoteProxy<IKvStoreClientDeathObserver> {
public:
    explicit KvStoreClientDeathObserverProxy(const sptr<IRemoteObject> &impl);
    ~KvStoreClientDeathObserverProxy() = default;
private:
    static inline BrokerDelegator<KvStoreClientDeathObserverProxy> delegator_;
};
}  // namespace DistributedKv
}  // namespace OHOS

#endif  // I_KVSTORE_CLIENT_DEATH_OBSERVER_H
