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

#include "static_subscriber_connection.h"

#include "event_log_wrapper.h"

namespace OHOS {
namespace EventFwk {
void StaticSubscriberConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    EVENT_LOGI("enter");
    proxy_ = new (std::nothrow) AppExecFwk::StaticSubscriberProxy(remoteObject);
    if (proxy_ == nullptr) {
        EVENT_LOGE("failed to create obj!");
    }
    ErrCode ec = proxy_->OnReceiveEvent(&event_);
    EVENT_LOGI("end, errorCode = %d", ec);
}

void StaticSubscriberConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    EVENT_LOGI("enter");
}
}  // namespace EventFwk
}  // namespace OHOS
