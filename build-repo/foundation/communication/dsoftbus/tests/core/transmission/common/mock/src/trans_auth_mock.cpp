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

#include "trans_auth_mock.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
void *g_authInterface;
TransAuthInterfaceMock::TransAuthInterfaceMock()
{
    g_authInterface = reinterpret_cast<void *>(this);
}

TransAuthInterfaceMock::~TransAuthInterfaceMock()
{
    g_authInterface = nullptr;
}

static TransAuthInterface *GetTransAuthInterface()
{
    return reinterpret_cast<TransAuthInterfaceMock *>(g_authInterface);
}

extern "C" {
int32_t AuthInit(void)
{
    return GetTransAuthInterface()->AuthInit();
}

void AuthDeinit(void)
{
    GetTransAuthInterface()->AuthDeinit();
}

int32_t RegAuthVerifyListener(const AuthVerifyListener *listener)
{
    return GetTransAuthInterface()->RegAuthVerifyListener(listener);
}

void UnregAuthVerifyListener(void)
{
    GetTransAuthInterface()->UnregAuthVerifyListener();
}

int32_t RegAuthTransListener(int32_t module, const AuthTransListener *listener)
{
    return GetTransAuthInterface()->RegAuthTransListener(module, listener);
}

void UnregAuthTransListener(int32_t module)
{
    GetTransAuthInterface()->UnregAuthTransListener(module);
}

int32_t RegGroupChangeListener(const GroupChangeListener *listener)
{
    return GetTransAuthInterface()->RegGroupChangeListener(listener);
}

void UnregGroupChangeListener(void)
{
    GetTransAuthInterface()->UnregGroupChangeListener();
}

uint32_t AuthGenRequestId(void)
{
    return GetTransAuthInterface()->AuthGenRequestId();
}

int32_t AuthFlushDevice(const char *uuid)
{
    return GetTransAuthInterface()->AuthFlushDevice(uuid);
}

void AuthHandleLeaveLNN(int64_t authId)
{
    GetTransAuthInterface()->AuthHandleLeaveLNN(authId);
}

int32_t AuthStartListening(AuthLinkType type, const char *ip, int32_t port)
{
    return GetTransAuthInterface()->AuthStartListening(type, ip, port);
}

void AuthStopListening(AuthLinkType type)
{
    GetTransAuthInterface()->AuthStopListening(type);
}

int32_t AuthOpenConn(const AuthConnInfo *info, uint32_t requestId, const AuthConnCallback *callback, bool isMeta)
{
    return GetTransAuthInterface()->AuthOpenConn(info, requestId, callback, isMeta);
}

void AuthCloseConn(int64_t authId)
{
    GetTransAuthInterface()->AuthCloseConn(authId);
}

int32_t AuthPostTransData(int64_t authId, const AuthTransData *dataInfo)
{
    return GetTransAuthInterface()->AuthPostTransData(authId, dataInfo);
}

int32_t AuthGetPreferConnInfo(const char *uuid, AuthConnInfo *connInfo, bool isMeta)
{
    return GetTransAuthInterface()->AuthGetPreferConnInfo(uuid, connInfo, isMeta);
}

int64_t AuthGetLatestIdByUuid(const char *uuid, bool isIpConnection, bool isMeta)
{
    return GetTransAuthInterface()->AuthGetLatestIdByUuid(uuid, isIpConnection, isMeta);
}

int64_t AuthGetIdByConnInfo(const AuthConnInfo *connInfo, bool isServer, bool isMeta)
{
    return GetTransAuthInterface()->AuthGetIdByConnInfo(connInfo, isServer, isMeta);
}

int64_t AuthGetIdByP2pMac(const char *p2pMac, AuthLinkType type, bool isServer, bool isMeta)
{
    return GetTransAuthInterface()->AuthGetIdByP2pMac(p2pMac, type, isServer, isMeta);
}

uint32_t AuthGetEncryptSize(uint32_t inLen)
{
    return GetTransAuthInterface()->AuthGetEncryptSize(inLen);
}

uint32_t AuthGetDecryptSize(uint32_t inLen)
{
    return GetTransAuthInterface()->AuthGetDecryptSize(inLen);
}

int32_t AuthSetP2pMac(int64_t authId, const char *p2pMac)
{
    return GetTransAuthInterface()->AuthSetP2pMac(authId, p2pMac);
}

int32_t AuthGetConnInfo(int64_t authId, AuthConnInfo *connInfo)
{
    return GetTransAuthInterface()->AuthGetConnInfo(authId, connInfo);
}

int32_t AuthGetServerSide(int64_t authId, bool *isServer)
{
    return GetTransAuthInterface()->AuthGetServerSide(authId, isServer);
}

int32_t AuthGetDeviceUuid(int64_t authId, char *uuid, uint16_t size)
{
    return GetTransAuthInterface()->AuthGetDeviceUuid(authId, uuid, size);
}

int32_t AuthGetVersion(int64_t authId, SoftBusVersion *version)
{
    return GetTransAuthInterface()->AuthGetVersion(authId, version);
}

int32_t AuthGetMetaType(int64_t authId, bool *isMetaAuth)
{
    return GetTransAuthInterface()->AuthGetMetaType(authId, isMetaAuth);
}

int32_t AuthMetaStartVerify(uint32_t connectionId, const uint8_t *key, uint32_t keyLen,
    uint32_t requestId, const AuthVerifyCallback *callBack)
{
    return GetTransAuthInterface()->AuthMetaStartVerify(connectionId, key, keyLen, requestId, callBack);
}

void AuthMetaReleaseVerify(int64_t authId)
{
    return GetTransAuthInterface()->AuthMetaReleaseVerify(authId);
}

int32_t AuthEncrypt(int64_t authId, const uint8_t *inData,
    uint32_t inLen, uint8_t *outData, uint32_t *outLen)
{
    return GetTransAuthInterface()->AuthEncrypt(authId, inData, inLen, outData, outLen);
}

int32_t AuthDecrypt(int64_t authId, const uint8_t *inData,
    uint32_t inLen, uint8_t *outData, uint32_t *outLen)
{
    return GetTransAuthInterface()->AuthDecrypt(authId, inData, inLen, outData, outLen);
}

int32_t LnnGetLocalStrInfo(InfoKey key, char *info, uint32_t len)
{
    return GetTransAuthInterface()->LnnGetLocalStrInfo(key, info, len);
}

int32_t LnnGetNetworkIdByUuid(const char *uuid, char *buf, uint32_t len)
{
    return GetTransAuthInterface()->LnnGetNetworkIdByUuid(uuid, buf, len);
}

int32_t LnnGetRemoteStrInfo(const char *networkId, InfoKey key, char *info, uint32_t len)
{
    return GetTransAuthInterface()->LnnGetRemoteStrInfo(networkId, key, info, len);
}

int32_t LnnGetNetworkIdByBtMac(const char *btMac, char *buf, uint32_t len)
{
    return GetTransAuthInterface()->LnnGetNetworkIdByBtMac(btMac, buf, len);
}
}
}
