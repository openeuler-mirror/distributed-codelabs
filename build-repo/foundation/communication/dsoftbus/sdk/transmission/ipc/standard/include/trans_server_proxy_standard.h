/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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


#ifndef INTERFACES_INNERKITS_TRANS_SERVER_PROXY_STANDARD_H_
#define INTERFACES_INNERKITS_TRANS_SERVER_PROXY_STANDARD_H_

#include "if_softbus_server.h"

namespace OHOS {
class TransServerProxy : public IRemoteProxy<ISoftBusServer> {
public:
    explicit TransServerProxy(const sptr<IRemoteObject> &impl)
        : IRemoteProxy<ISoftBusServer>(impl) {}
    virtual ~TransServerProxy() = default;

    int32_t StartDiscovery(const char *pkgName, const SubscribeInfo *info) override;
    int32_t StopDiscovery(const char *pkgName, int subscribeId) override;
    int32_t PublishService(const char *pkgName, const PublishInfo *info) override;
    int32_t UnPublishService(const char *pkgName, int publishId) override;
    int32_t SoftbusRegisterService(const char *clientPkgName, const sptr<IRemoteObject> &object) override;

    int32_t CreateSessionServer(const char *pkgName, const char *sessionName) override;
    int32_t RemoveSessionServer(const char *pkgName, const char *sessionName) override;
    int32_t OpenSession(const SessionParam *param, TransInfo *info) override;
    int32_t OpenAuthSession(const char *sessionName, const ConnectionAddr *addrInfo) override;
    int32_t NotifyAuthSuccess(int32_t channelId, int32_t channelType) override;
    int32_t CloseChannel(int32_t channelId, int32_t channelType) override;
    int32_t SendMessage(int32_t channelId, int32_t channelType, const void *data,
        uint32_t len, int32_t msgType) override;

    int32_t JoinLNN(const char *pkgName, void *addr, uint32_t addrTypeLen) override;
    int32_t JoinMetaNode(const char *pkgName, void *addr, CustomData *customData, uint32_t addrTypeLen) override;
    int32_t LeaveLNN(const char *pkgName, const char *networkId) override;
    int32_t LeaveMetaNode(const char *pkgName, const char *networkId) override;
    int32_t GetAllOnlineNodeInfo(const char *pkgName, void **info, uint32_t infoTypeLen, int *infoNum) override;
    int32_t GetLocalDeviceInfo(const char *pkgName, void *info, uint32_t infoTypeLen) override;
    int32_t GetNodeKeyInfo(const char *pkgName, const char *networkId, int key, unsigned char *buf,
        uint32_t len) override;
    int32_t SetNodeDataChangeFlag(const char *pkgName, const char *networkId, uint16_t dataChangeFlag) override;
    int32_t StartTimeSync(const char *pkgName, const char *targetNetworkId, int32_t accuracy,
        int32_t period) override;
    int32_t StopTimeSync(const char *pkgName, const char *targetNetworkId) override;
    int32_t QosReport(int32_t channelId, int32_t chanType, int32_t appType, int quality) override;
    int32_t StreamStats(int32_t channelId, int32_t channelType, const StreamSendStats *data) override;
    int32_t GrantPermission(int uid, int pid, const char *sessionName) override;
    int32_t RemovePermission(const char *sessionName) override;
    int32_t RippleStats(int32_t channelId, int32_t channelType, const TrafficStats *statsData) override;

private:
    static inline BrokerDelegator<TransServerProxy> delegator_;
};
} // namespace OHOS

#endif // !defined(INTERFACES_INNERKITS_TRANS_SERVER_PROXY_STANDARD_H_)