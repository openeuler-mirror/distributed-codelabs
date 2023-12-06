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

#ifndef AUTH_NET_LEDGER_MOCK_H
#define AUTH_NET_LEDGER_MOCK_H

#include <gmock/gmock.h>
#include <mutex>

#include "bus_center_manager.h"
#include "lnn_node_info.h"

namespace OHOS {
class AuthNetLedgerInterface {
public:
    AuthNetLedgerInterface() {};
    virtual ~AuthNetLedgerInterface() {};

    virtual int32_t LnnGetLocalStrInfo(InfoKey key, char *info, uint32_t len) = 0;
    virtual int32_t LnnDeleteSpecificTrustedDevInfo(const char *udid) = 0;
    virtual const NodeInfo *LnnGetLocalNodeInfo(void) = 0;
    virtual int32_t LnnGetAuthPort(const NodeInfo *info) = 0;
    virtual int32_t LnnGetSessionPort(const NodeInfo *info) = 0;
    virtual int32_t LnnGetProxyPort(const NodeInfo *info) = 0;
    virtual const char *LnnGetBtMac(const NodeInfo *info) = 0;
    virtual const char *LnnGetDeviceName(const DeviceBasicInfo *info) = 0;
    virtual char *LnnConvertIdToDeviceType(uint16_t typeId) = 0;
    virtual const char *LnnGetDeviceUdid(const NodeInfo *info) = 0;
    virtual int32_t LnnGetP2pRole(const NodeInfo *info) = 0;
    virtual const char *LnnGetP2pMac(const NodeInfo *info) = 0;
    virtual uint64_t LnnGetSupportedProtocols(const NodeInfo *info) = 0;
    virtual int32_t LnnConvertDeviceTypeToId(const char *deviceType, uint16_t *typeId) = 0;
    virtual int32_t LnnGetLocalNumInfo(InfoKey key, int32_t *info) = 0;
};
class AuthNetLedgertInterfaceMock : public AuthNetLedgerInterface {
public:
    AuthNetLedgertInterfaceMock();
    ~AuthNetLedgertInterfaceMock() override;
    MOCK_METHOD3(LnnGetLocalStrInfo, int32_t (InfoKey, char *, uint32_t));
    MOCK_METHOD1(LnnDeleteSpecificTrustedDevInfo, int32_t (const char *));
    MOCK_METHOD0(LnnGetLocalNodeInfo, const NodeInfo *());
    MOCK_METHOD1(LnnGetAuthPort, int32_t (const NodeInfo *));
    MOCK_METHOD1(LnnGetSessionPort, int32_t (const NodeInfo *));
    MOCK_METHOD1(LnnGetProxyPort, int32_t (const NodeInfo *));
    MOCK_METHOD1(LnnGetBtMac, const char *(const NodeInfo *));
    MOCK_METHOD1(LnnGetDeviceName, const char *(const DeviceBasicInfo *));
    MOCK_METHOD1(LnnConvertIdToDeviceType, char *(uint16_t));
    MOCK_METHOD1(LnnGetDeviceUdid, const char *(const NodeInfo *));
    MOCK_METHOD1(LnnGetP2pRole, int32_t (const NodeInfo *));
    MOCK_METHOD1(LnnGetP2pMac, const char *(const NodeInfo *));
    MOCK_METHOD1(LnnGetSupportedProtocols, uint64_t (const NodeInfo *));
    MOCK_METHOD2(LnnConvertDeviceTypeToId, int32_t (const char *, uint16_t *));
    MOCK_METHOD2(LnnGetLocalNumInfo, int32_t (InfoKey, int32_t *));
};
} // namespace OHOS
#endif // AUTH_NET_LEDGER_MOCK_H