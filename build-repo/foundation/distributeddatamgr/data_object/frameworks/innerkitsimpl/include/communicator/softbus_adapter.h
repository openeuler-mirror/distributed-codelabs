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

#ifndef DISTRIBUTEDDATAFWK_SRC_SOFTBUS_ADAPTER_H
#define DISTRIBUTEDDATAFWK_SRC_SOFTBUS_ADAPTER_H
#include <map>
#include <memory>
#include <set>
#include <tuple>

#include "app_data_change_listener.h"
#include "app_device_status_change_listener.h"
#include "app_types.h"
#include "session.h"
#include "softbus_bus_center.h"
#include "condition_lock.h"

namespace OHOS {
namespace ObjectStore {
class SoftBusAdapter {
public:
    SoftBusAdapter();
    ~SoftBusAdapter();
    static std::shared_ptr<SoftBusAdapter> GetInstance();
    
    // add DeviceChangeListener to watch device change;
    Status StartWatchDeviceChange(const AppDeviceStatusChangeListener *observer, const PipeInfo &pipeInfo);
    // stop DeviceChangeListener to watch device change;
    Status StopWatchDeviceChange(const AppDeviceStatusChangeListener *observer, const PipeInfo &pipeInfo);
    void NotifyAll(const DeviceInfo &deviceInfo, const DeviceChangeType &type);
    DeviceInfo GetLocalDevice();
    std::vector<DeviceInfo> GetDeviceList() const;
    std::string GetUdidByNodeId(const std::string &nodeId) const;
    // get local device node information;
    DeviceInfo GetLocalBasicInfo() const;
    // get all remote connected device's node information;
    std::vector<DeviceInfo> GetRemoteNodesBasicInfo() const;
    static std::string ToBeAnonymous(const std::string &name);

    // add DataChangeListener to watch data change;
    Status StartWatchDataChange(const AppDataChangeListener *observer, const PipeInfo &pipeInfo);

    // stop DataChangeListener to watch data change;
    Status StopWatchDataChange(const AppDataChangeListener *observer, const PipeInfo &pipeInfo);

    // Send data to other device, function will be called back after sent to notify send result.
    Status SendData(
        const PipeInfo &pipeInfo, const DeviceId &deviceId, const uint8_t *ptr, int size, const MessageInfo &info);

    bool IsSameStartedOnPeer(const struct PipeInfo &pipeInfo, const struct DeviceId &peer);

    void SetMessageTransFlag(const PipeInfo &pipeInfo, bool flag);

    int CreateSessionServerAdapter(const std::string &sessionName);

    int RemoveSessionServerAdapter(const std::string &sessionName) const;

    void UpdateRelationship(const std::string &networkid, const DeviceChangeType &type);

    void InsertSession(const std::string &sessionName);

    void DeleteSession(const std::string &sessionName);

    void NotifyDataListeners(const uint8_t *ptr, const int size, const std::string &deviceId, const PipeInfo &pipeInfo);

    std::string ToNodeID(const std::string &nodeId) const;

    int32_t GetSessionStatus(int32_t sessionId);

    void OnSessionOpen(int32_t sessionId, int32_t status);

    void OnSessionClose(int32_t sessionId);

private:
    std::shared_ptr<ConditionLock<int32_t>> GetSemaphore (int32_t sessinId);
    mutable std::mutex networkMutex_{};
    mutable std::map<std::string, std::string> networkId2Udid_{};
    DeviceInfo localInfo_{};
    static std::shared_ptr<SoftBusAdapter> instance_;
    std::mutex deviceChangeMutex_;
    std::set<const AppDeviceStatusChangeListener *> listeners_{};
    std::mutex dataChangeMutex_{};
    std::map<std::string, const AppDataChangeListener *> dataChangeListeners_{};
    std::mutex busSessionMutex_{};
    std::map<std::string, bool> busSessionMap_{};
    bool flag_ = true; // only for br flag
    ISessionListener sessionListener_{};
    std::mutex statusMutex_ {};
    std::map<int32_t, std::shared_ptr<ConditionLock<int32_t>>> sessionsStatus_;
};
} // namespace ObjectStore
} // namespace OHOS
#endif /* DISTRIBUTEDDATAFWK_SRC_SOFTBUS_ADAPTER_H */