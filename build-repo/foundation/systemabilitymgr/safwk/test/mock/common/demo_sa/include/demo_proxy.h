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

#ifndef TEST_MOCK_COMMON_DEMO_SA_INCLUDE_DEAM_PROXY_H_
#define TEST_MOCK_COMMON_DEMO_SA_INCLUDE_DEAM_PROXY_H_

#include <string>

#include "demo_interface.h"
#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "refbase.h"

namespace OHOS {
class DemoProxy : public IRemoteProxy<DemoInterface> {
public:
    explicit DemoProxy(const sptr<IRemoteObject>& impl)
        : IRemoteProxy<DemoInterface>(impl) {}
    ~DemoProxy() = default;

    int AddVolume(int volume) override;
private:
    static inline BrokerDelegator<DemoProxy> delegator_;
};
}

#endif // TEST_MOCK_COMMON_DEMO_SA_INCLUDE_DEAM_PROXY_H_