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
#ifndef DATA_OBJECT_MOCK_APP_DEVICE_CHANGE_LISTENER_H
#define DATA_OBJECT_MOCK_APP_DEVICE_CHANGE_LISTENER_H

#include <gmock/gmock.h>
#include "app_device_status_change_listener.h"

namespace OHOS {
namespace ObjectStore {
class MockAppDeviceStatusChangeListener : public AppDeviceStatusChangeListener {
    MOCK_CONST_METHOD2(OnDeviceChanged, void(const DeviceInfo &info, const DeviceChangeType &type));
};
} // namespace ObjectStore
} // namespace OHOS

#endif // DATA_OBJECT_MOCK_APP_DEVICE_CHANGE_LISTENER_H
