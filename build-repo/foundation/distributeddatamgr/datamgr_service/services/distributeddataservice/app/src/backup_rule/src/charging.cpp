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
#define LOG_TAG "Charging"
#include "charging.h"
#include "battery_info.h"
#include "battery_srv_client.h"
#include "log_print.h"

namespace OHOS {
namespace DistributedData {
__attribute__((used)) Charging Charging::instance_;
Charging::Charging() noexcept
{
    BackupRuleManager::GetInstance().RegisterPlugin(
        "Charging", [this]() -> auto { return this; });
}

bool Charging::CanBackup()
{
    auto &batterySrvClient = PowerMgr::BatterySrvClient::GetInstance();
    auto chargingStatus = batterySrvClient.GetChargingStatus();
    if (chargingStatus != PowerMgr::BatteryChargeState::CHARGE_STATE_ENABLE) {
        if (chargingStatus != PowerMgr::BatteryChargeState::CHARGE_STATE_FULL) {
            ZLOGE("the device is not in charge state, chargingStatus=%{public}d.", chargingStatus);
            return false;
        }
        auto batteryPluggedType = batterySrvClient.GetPluggedType();
        if (batteryPluggedType != PowerMgr::BatteryPluggedType::PLUGGED_TYPE_AC &&
            batteryPluggedType != PowerMgr::BatteryPluggedType::PLUGGED_TYPE_USB &&
            batteryPluggedType != PowerMgr::BatteryPluggedType::PLUGGED_TYPE_WIRELESS) {
            ZLOGE("the device is not in charge full statue, the batteryPluggedType is %{public}d.", batteryPluggedType);
            return false;
        }
    }
    return true;
}
} // namespace DistributedData
} // namespace OHOS