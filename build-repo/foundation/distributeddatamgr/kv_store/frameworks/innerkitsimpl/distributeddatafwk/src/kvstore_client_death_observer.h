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

#ifndef KVSTORE_CLIENT_DEATH_OBSERVER_H
#define KVSTORE_CLIENT_DEATH_OBSERVER_H

#include "ikvstore_client_death_observer.h"

namespace OHOS {
namespace DistributedKv {
class API_EXPORT KvStoreClientDeathObserver : public KvStoreClientDeathObserverStub {
public:
    KvStoreClientDeathObserver();

    virtual ~KvStoreClientDeathObserver();
};
}  // namespace DistributedKv
}  // namespace OHOS
#endif  // KVSTORE_CLIENT_DEATH_OBSERVER_H
