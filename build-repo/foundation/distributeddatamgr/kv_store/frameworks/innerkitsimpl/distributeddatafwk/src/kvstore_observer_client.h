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

#ifndef KVSTORE_OBSERVER_CLIENT_H
#define KVSTORE_OBSERVER_CLIENT_H

#include <memory>
#include "change_notification.h"
#include "ikvstore_observer.h"
#include "kvstore_observer.h"
#include "refbase.h"

namespace OHOS {
namespace DistributedKv {
class KvStoreObserverClient : public KvStoreObserverStub {
public:
    explicit KvStoreObserverClient(std::shared_ptr<KvStoreObserver> kvStoreObserver);

    ~KvStoreObserverClient();

    void OnChange(const ChangeNotification &changeNotification) override;

private:
    static const int MAX_TRY_COUNT = 10;

    // client is responsible for free it when call UnSubscribeKvStore.
    std::shared_ptr<KvStoreObserver> kvStoreObserver_;
};
}  // namespace DistributedKv
}  // namespace OHOS
#endif  // KVSTORE_OBSERVER_CLIENT_H
