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
#ifndef SAMGR_TEST_UNITTEST_INCLUDE_MOCK_IRO_SENDREQUEST_H
#define SAMGR_TEST_UNITTEST_INCLUDE_MOCK_IRO_SENDREQUEST_H

#include "iremote_broker.h"
#include "iremote_object.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"
#include "refbase.h"
#include "sam_log.h"

namespace OHOS {
class MockIroSendrequest : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.MockIroSendrequest");
};

class MockIroSendrequesteStub : public IRemoteStub<MockIroSendrequest> {
public:
    MockIroSendrequesteStub() = default;
    virtual ~MockIroSendrequesteStub() = default;
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    int32_t result_ = 0;
    bool flag_ = false;
};

} // namespace OHOS
#endif /* SAMGR_TEST_UNITTEST_INCLUDE_MOCK_IRO_SENDREQUEST_H */