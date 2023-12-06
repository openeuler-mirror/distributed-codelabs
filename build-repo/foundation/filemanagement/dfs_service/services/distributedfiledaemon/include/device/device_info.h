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
#ifndef DEVICE_INFO_H
#define DEVICE_INFO_H

#include <atomic>
#include <string>

#include "device_manager.h"
#include "dm_device_info.h"
#include "ipc/i_daemon.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class DeviceInfo final {
public:
    DeviceInfo() = default;
    ~DeviceInfo() = default;
    explicit DeviceInfo(const DistributedHardware::DmDeviceInfo &nodeInfo);
    DeviceInfo(const DeviceInfo &nodeInfo);
    DeviceInfo &operator=(const DistributedHardware::DmDeviceInfo &nodeInfo);

    void SetCid(const std::string &cid);

    const std::string &GetCid() const;

private:
    friend class DeviceManagerAgent;
    std::atomic<bool> initCidFlag_ { false };
    std::string cid_;
    std::string udid_;
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // DEVICE_INFO_H