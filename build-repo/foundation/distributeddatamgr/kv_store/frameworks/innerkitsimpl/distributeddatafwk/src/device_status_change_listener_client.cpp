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

#include "device_status_change_listener_client.h"
#include <utility>

namespace OHOS::DistributedKv {
DeviceStatusChangeListenerClient::DeviceStatusChangeListenerClient(
    std::shared_ptr<DeviceStatusChangeListener> listener) : listener_(std::move(listener))
{}

void DeviceStatusChangeListenerClient::Online(const std::string &device)
{
    if (listener_ == nullptr) {
        return;
    }
    DeviceInfo info;
    info.deviceId = device;
    listener_->OnDeviceChanged(info, DeviceChangeType::DEVICE_ONLINE);
}

void DeviceStatusChangeListenerClient::Offline(const std::string &device)
{
    if (listener_ == nullptr) {
        return;
    }
    DeviceInfo info;
    info.deviceId = device;
    listener_->OnDeviceChanged(info, DeviceChangeType::DEVICE_OFFLINE);
}
} // namespace OHOS::DistributedKv
