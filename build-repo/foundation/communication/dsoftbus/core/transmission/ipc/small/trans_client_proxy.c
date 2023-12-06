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

#include "trans_client_proxy.h"

#include "ipc_skeleton.h"
#include "serializer.h"
#include "softbus_adapter_mem.h"
#include "softbus_base_listener.h"
#include "softbus_client_info_manager.h"
#include "softbus_errcode.h"
#include "softbus_ipc_def.h"
#include "softbus_log.h"
#include "softbus_socket.h"

static int32_t GetSvcIdentityByPkgName(const char *pkgName, SvcIdentity *svc)
{
    struct CommonScvId svcId = {0};
    if (SERVER_GetIdentityByPkgName(pkgName, &svcId) != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "ger identity failed");
        return SOFTBUS_ERR;
    }
    svc->handle = svcId.handle;
    svc->token = svcId.token;
    svc->cookie = svcId.cookie;

    return SOFTBUS_OK;
}

static int32_t OnUdpChannelOpenedAsServer(const SvcIdentity *svc, IpcIo *io)
{
    IpcIo reply;
    uintptr_t ptr = 0;
    MessageOption option;
    MessageOptionInit(&option);
    int32_t ans = SendRequest(*svc, CLIENT_ON_CHANNEL_OPENED, io, &reply, option, &ptr);
    if (ans != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "OnChannelOpened SendRequest failed");
        FreeBuffer((void *)ptr);
        return SOFTBUS_ERR;
    }
    int32_t udpPort;
    ReadInt32(&reply, &udpPort);
    FreeBuffer((void *)ptr);
    return udpPort;
}

int32_t ClientIpcOnChannelOpened(const char *pkgName, const char *sessionName, const ChannelInfo *channel)
{
    SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_INFO, "on channel opened ipc server push");
    IpcIo io;
    uint8_t tmpData[MAX_SOFT_BUS_IPC_LEN_EX];
    if (channel->channelType == CHANNEL_TYPE_TCP_DIRECT) {
        IpcIoInit(&io, tmpData, MAX_SOFT_BUS_IPC_LEN_EX, 1);
    } else {
        IpcIoInit(&io, tmpData, MAX_SOFT_BUS_IPC_LEN_EX, 0);
    }
    WriteString(&io, sessionName);
    WriteInt32(&io, channel->channelId);
    WriteInt32(&io, channel->channelType);
    WriteBool(&io, channel->isServer);
    WriteBool(&io, channel->isEnabled);
    WriteInt32(&io, channel->peerUid);
    WriteInt32(&io, channel->peerPid);
    WriteString(&io, channel->groupId);
    WriteUint32(&io, channel->keyLen);
    WriteBuffer(&io, channel->sessionKey, channel->keyLen);
    WriteString(&io, channel->peerSessionName);
    WriteString(&io, channel->peerDeviceId);
    if (channel->channelType == CHANNEL_TYPE_TCP_DIRECT) {
        if (!WriteFileDescriptor(&io, channel->fd)) {
            return SOFTBUS_ERR;
        }
    }
    SvcIdentity svc = {0};
    if (GetSvcIdentityByPkgName(pkgName, &svc) != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "OnChannelOpened get svc failed.");
        return SOFTBUS_ERR;
    }
    WriteInt32(&io, channel->businessType);
    if (channel->channelType == CHANNEL_TYPE_UDP) {
        WriteString(&io, channel->myIp);
        WriteInt32(&io, channel->streamType);
        WriteBool(&io, channel->isUdpFile);
        if (channel->isServer) {
            return OnUdpChannelOpenedAsServer(&svc, &io);
        }
        WriteInt32(&io, channel->peerPort);
        WriteString(&io, channel->peerIp);
    }
    WriteInt32(&io, channel->routeType);
    MessageOption option;
    MessageOptionInit(&option);
    option.flags = TF_OP_ASYNC;
    int32_t ans = SendRequest(svc, CLIENT_ON_CHANNEL_OPENED, &io, NULL, option, NULL);
    if (ans != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "OnChannelOpened SendRequest failed");
    }
    return ans;
}

int32_t ClientIpcOnChannelOpenFailed(const char *pkgName, int32_t channelId, int32_t channelType, int32_t errCode)
{
    SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_INFO, "on channel open failed ipc server push");
    IpcIo io;
    uint8_t tmpData[MAX_SOFT_BUS_IPC_LEN];
    IpcIoInit(&io, tmpData, MAX_SOFT_BUS_IPC_LEN, 0);
    WriteInt32(&io, channelId);
    WriteInt32(&io, channelType);
    WriteInt32(&io, errCode);
    SvcIdentity svc = {0};
    if (GetSvcIdentityByPkgName(pkgName, &svc) != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "OnChannelOpenFailed get svc failed.");
        return SOFTBUS_ERR;
    }
    MessageOption option;
    MessageOptionInit(&option);
    option.flags = TF_OP_ASYNC;
    int32_t ans = SendRequest(svc, CLIENT_ON_CHANNEL_OPENFAILED, &io, NULL, option, NULL);
    if (ans != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "OnChannelOpenFailed SendRequest failed");
    }
    return ans;
}

int32_t ClientIpcOnChannelLinkDown(const char *pkgName, const char *networkId, int32_t routeType)
{
    if (pkgName == NULL || networkId == NULL) {
        return SOFTBUS_INVALID_PARAM;
    }
    SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_INFO, "ClientIpcOnChannelLinkDown: pkgName=%s", pkgName);

    IpcIo io;
    uint8_t tmpData[MAX_SOFT_BUS_IPC_LEN];
    IpcIoInit(&io, tmpData, MAX_SOFT_BUS_IPC_LEN, 0);
    WriteString(&io, networkId);
    WriteInt32(&io, routeType);
    SvcIdentity svc = {0};
    if (GetSvcIdentityByPkgName(pkgName, &svc) != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "OnLeaveLNNResult callback get svc failed.");
        return SOFTBUS_ERR;
    }
    MessageOption option;
    MessageOptionInit(&option);
    option.flags = TF_OP_ASYNC;
    int32_t ans = SendRequest(svc, CLIENT_ON_CHANNEL_LINKDOWN, &io, NULL, option, NULL);
    if (ans != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "ClientIpcOnChannelLinkDown callback SendRequest failed.");
        return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}

int32_t ClientIpcOnChannelClosed(const char *pkgName, int32_t channelId, int32_t channelType)
{
    SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_INFO, "on channel closed ipc server push");
    IpcIo io;
    uint8_t tmpData[MAX_SOFT_BUS_IPC_LEN];
    IpcIoInit(&io, tmpData, MAX_SOFT_BUS_IPC_LEN, 0);
    WriteInt32(&io, channelId);
    WriteInt32(&io, channelType);
    SvcIdentity svc = {0};
    if (GetSvcIdentityByPkgName(pkgName, &svc) != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "OnChannelOpenClosed get svc failed.");
        return SOFTBUS_ERR;
    }
    MessageOption option;
    MessageOptionInit(&option);
    option.flags = TF_OP_ASYNC;
    int32_t ans = SendRequest(svc, CLIENT_ON_CHANNEL_CLOSED, &io, NULL, option, NULL);
    if (ans != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "OnChannelOpenClosed SendRequest failed");
    }
    return ans;
}

int32_t ClientIpcOnChannelMsgReceived(const char *pkgName, int32_t channelId, int32_t channelType,
                                      const void *data, unsigned int len, int32_t type)
{
    SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_INFO, "on channel closed ipc server push");
    IpcIo io;
    uint8_t *tmpData = (uint8_t *)SoftBusCalloc(len + MAX_SOFT_BUS_IPC_LEN);
    if (tmpData == NULL) {
        HILOG_ERROR(SOFTBUS_HILOG_ID, "tmpData is null");
        return SOFTBUS_ERR;
    }
    IpcIoInit(&io, tmpData, len + MAX_SOFT_BUS_IPC_LEN, 0);
    WriteInt32(&io, channelId);
    WriteInt32(&io, channelType);
    WriteInt32(&io, type);
    WriteUint32(&io, len);
    WriteBuffer(&io, data, len);
    SvcIdentity svc = {0};
    if (GetSvcIdentityByPkgName(pkgName, &svc) != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "OnChannelOpenClosed get svc failed.");
        SoftBusFree(tmpData);
        return SOFTBUS_ERR;
    }
    MessageOption option;
    MessageOptionInit(&option);
    option.flags = TF_OP_ASYNC;
    int32_t ans = SendRequest(svc, CLIENT_ON_CHANNEL_MSGRECEIVED, &io, NULL, option, NULL);
    if (ans != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "OnChannelOpenClosed SendRequest failed");
    }
    SoftBusFree(tmpData);
    return ans;
}

int32_t ClientIpcOnChannelQosEvent(const char *pkgName, const QosParam *param)
{
    (void)pkgName;
    (void)param;
    return SOFTBUS_FUNC_NOT_SUPPORT;
}
