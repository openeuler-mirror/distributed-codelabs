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

#include "softbus_connector.h"
#include "softbus_bus_center.h"
#include "dm_device_info.h"
#include "dm_publish_info.h"
#include "dm_subscribe_info.h"
#include "softbus_discovery_callback.h"
#include "softbus_publish_callback.h"
#include "softbus_session.h"
#include "softbus_state_callback.h"
#include "softbus_connector_fuzzer.h"

namespace OHOS {
namespace DistributedHardware {

void SoftBusConnectorFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }

    std::string szData(reinterpret_cast<const char*>(data), size);
    int32_t subscribeId = *(reinterpret_cast<const int32_t*>(data));
    int publishId = *(reinterpret_cast<const int*>(data));
    PublishResult pResult = (PublishResult)1;
    RefreshResult rResult = (RefreshResult)1;
    ConnectionAddr *addr = nullptr;
    const char *networkId = reinterpret_cast<const char*>(data);
    int32_t result = *(reinterpret_cast<const int32_t*>(data));

    SoftbusConnector::OnSoftbusPublishResult(publishId, pResult);
    SoftbusConnector::OnSoftbusDiscoveryResult(subscribeId, rResult);
    SoftbusConnector::OnSoftbusJoinLNNResult(addr, networkId, result);
    SoftbusConnector::GetConnectionIpAddress(szData, szData);
    SoftbusConnector::IsDeviceOnLine(szData);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::SoftBusConnectorFuzzTest(data, size);

    return 0;
}
