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

#ifndef I_KVSTORE_OBSERVER_H
#define I_KVSTORE_OBSERVER_H

#include "change_notification.h"
#include "iremote_broker.h"
#include "ikvstore_observer.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"

namespace OHOS {
namespace DistributedKv {
class IKvStoreObserver : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.DistributedKv.IKvStoreObserver");
    virtual void OnChange(const ChangeNotification &changeNotification) = 0;
protected:
    static constexpr int64_t SWITCH_RAW_DATA_SIZE = 700 * 1024;
    static constexpr size_t MAX_IPC_CAPACITY = 800 * 1024;
};

class KvStoreObserverStub : public IRemoteStub<IKvStoreObserver> {
public:
    int OnRemoteRequest(uint32_t code, MessageParcel &data,
                        MessageParcel &reply, MessageOption &option) override;
};

class API_EXPORT KvStoreObserverProxy : public IRemoteProxy<IKvStoreObserver> {
public:
    explicit KvStoreObserverProxy(const sptr<IRemoteObject> &impl);
    ~KvStoreObserverProxy() = default;
    void OnChange(const ChangeNotification &changeNotification) override;
private:
    static inline BrokerDelegator<KvStoreObserverProxy> delegator_;
};
}  // namespace DistributedKv
}  // namespace OHOS

#endif  // I_KVSTORE_OBSERVER_H
