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

#ifndef OHOS_HIVIEWDFX_SYS_EVENT_CALLBACK_STUB_H
#define OHOS_HIVIEWDFX_SYS_EVENT_CALLBACK_STUB_H

#include <cstdint>

#include "iremote_stub.h"
#include "isys_event_callback.h"
#include "message_option.h"
#include "message_parcel.h"
#include "nocopyable.h"
#include "refbase.h"

namespace OHOS {
namespace HiviewDFX {
class SysEventCallbackStub : public IRemoteStub<ISysEventCallback> {
public:
    SysEventCallbackStub() = default;
    virtual ~SysEventCallbackStub() = default;
    DISALLOW_COPY_AND_MOVE(SysEventCallbackStub);

    int32_t OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply,
        MessageOption& option) override;
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // OHOS_HIVIEWDFX_SYS_EVENT_CALLBACK_STUB_H
