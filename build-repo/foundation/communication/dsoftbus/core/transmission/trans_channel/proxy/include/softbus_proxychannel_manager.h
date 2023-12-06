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

#ifndef SOFTBUS_PROXYCHANNEL_MANAGER_H
#define SOFTBUS_PROXYCHANNEL_MANAGER_H

#include "stdint.h"
#include "softbus_app_info.h"
#include "softbus_conn_interface.h"
#include "softbus_proxychannel_message.h"
#include "trans_channel_callback.h"

#ifdef __cplusplus
extern "C" {
#endif


int32_t TransProxyManagerInit(const IServerChannelCallBack *cb);
void TransProxyManagerDeinit(void);

int32_t TransProxyAuthSessionDataLenCheck(uint32_t dataLen, int32_t type);

int32_t TransProxyGetNewChanSeq(int32_t channelId);
int32_t TransProxyOpenProxyChannel(const AppInfo *appInfo, const ConnectOption *connInfo, int32_t *channelId);
int32_t TransProxyCloseProxyChannel(int32_t channelId);
int32_t TransProxySendMsg(int32_t channelId, const char *data, uint32_t dataLen, int32_t priority);

void TransProxyDelByConnId(uint32_t connId);
void TransProxyDelChanByReqId(int32_t reqId);
void TransProxyDelChanByChanId(int32_t chanlId);

void TransProxyOpenProxyChannelSuccess(int32_t chanId);
void TransProxyOpenProxyChannelFail(int32_t channelId, const AppInfo *appInfo, int32_t errCode);
void TransProxyonMessageReceived(const ProxyMessage *msg);

int32_t TransProxyGetSessionKeyByChanId(int32_t channelId, char *sessionKey, uint32_t sessionKeySize);
int16_t TransProxyGetNewMyId(void);
int32_t TransProxyGetSendMsgChanInfo(int32_t channelId, ProxyChannelInfo *chanInfo);

int32_t TransProxyCreateChanInfo(ProxyChannelInfo *chan, int32_t channelId, const AppInfo *appInfo);
void TransProxyChanProcessByReqId(int32_t reqId, uint32_t connId);

int64_t TransProxyGetAuthId(int32_t channelId);
int32_t TransProxyGetNameByChanId(int32_t chanId, char *pkgName, char *sessionName,
    uint16_t pkgLen, uint16_t sessionLen);

void TransProxyDeathCallback(const char *pkgName);

int32_t TransProxyGetAppInfoByChanId(int32_t chanId, AppInfo* appInfo);
int32_t TransProxyGetConnIdByChanId(int32_t channelId, int32_t *connId);
int32_t TransProxyGetConnOptionByChanId(int32_t channelId, ConnectOption *connOpt);

int32_t TransProxyGetAppInfoType(int16_t myId, const char *identity);

#ifdef __cplusplus
}
#endif

#endif
