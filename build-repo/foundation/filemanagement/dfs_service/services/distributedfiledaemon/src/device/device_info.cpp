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

#include "device/device_info.h"
#include "dfsu_exception.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace std;

DeviceInfo::DeviceInfo(const DistributedHardware::DmDeviceInfo &nodeInfo)
{
    cid_ = string(nodeInfo.deviceId);
    initCidFlag_ = true;
    // convert networkId to udid
    auto &deviceManager = DistributedHardware::DeviceManager::GetInstance();
    deviceManager.GetUdidByNetworkId(IDaemon::SERVICE_NAME, cid_, udid_);
}

DeviceInfo &DeviceInfo::operator=(const DistributedHardware::DmDeviceInfo &nodeInfo)
{
    cid_ = string(nodeInfo.deviceId);
    initCidFlag_ = true;
    return *this;
}

DeviceInfo::DeviceInfo(const DeviceInfo &nodeInfo) : cid_(nodeInfo.cid_), udid_(nodeInfo.udid_)
{
    initCidFlag_.store(nodeInfo.initCidFlag_.load());
}

void DeviceInfo::SetCid(const string &cid)
{
    if (initCidFlag_ == false) {
        cid_ = cid;
        initCidFlag_ = true;
    } else {
        LOGI("Cid is already initialized");
    }
}

const string &DeviceInfo::GetCid() const
{
    if (initCidFlag_ == false) {
        ThrowException(ERR_DEVICE_CID_UN_INIT, "cid uninitialized");
    }
    return cid_;
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
