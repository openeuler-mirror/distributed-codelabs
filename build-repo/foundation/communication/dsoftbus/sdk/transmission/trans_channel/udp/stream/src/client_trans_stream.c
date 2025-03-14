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

#include "client_trans_stream.h"

#include "client_trans_udp_stream_interface.h"
#include "session.h"
#include "softbus_errcode.h"
#include "softbus_log.h"
#include "softbus_utils.h"
#include "trans_server_proxy.h"

static const UdpChannelMgrCb *g_udpChannelMgrCb = NULL;

void RegisterStreamCb(const UdpChannelMgrCb *cb)
{
    if (cb == NULL || cb->OnUdpChannelOpened == NULL ||
        cb->OnUdpChannelClosed == NULL || cb->OnStreamReceived == NULL) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "udp channel callback is invalid");
        return;
    }

    g_udpChannelMgrCb = cb;
}

void UnregisterStreamCb(void)
{
    g_udpChannelMgrCb = NULL;
}

static void SetStreamChannelStatus(int32_t channelId, int32_t status)
{
    if (g_udpChannelMgrCb == NULL) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "udp channel callback is null.");
        return;
    }

    switch (status) {
        case STREAM_CONNECTED:
            SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_INFO, "cId=%d dstream connected.", channelId);
            if (g_udpChannelMgrCb->OnUdpChannelOpened == NULL) {
                SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "udp channel callback on udp channel opened is null.");
                return;
            }
            g_udpChannelMgrCb->OnUdpChannelOpened(channelId);
            break;
        case STREAM_CLOSED:
            SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_INFO, "cId=%d dstream closed.", channelId);
            break;
        case STREAM_INIT:
            SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_INFO, "cId=%d dstream init.", channelId);
            break;
        case STREAM_OPENING:
            SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_INFO, "cId=%d dstream opening.", channelId);
            break;
        case STREAM_CONNECTING:
            SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_INFO, "cId=%d dstream connecting.", channelId);
            break;
        case STREAM_CLOSING:
            SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_INFO, "cId=%d dstream closing.", channelId);
            break;
        default:
            SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "cId=%d unsupport stream status=%d.", channelId, status);
            break;
    }
}

static void OnStreamReceived(int32_t channelId, const StreamData *data, const StreamData *ext,
    const StreamFrameInfo *param)
{
    if ((g_udpChannelMgrCb == NULL) || (g_udpChannelMgrCb->OnStreamReceived == NULL)) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "udp channel callback on stream received is null.");
        return;
    }

    g_udpChannelMgrCb->OnStreamReceived(channelId, data, ext, param);
}

static void OnQosEvent(int channelId, int eventId, int tvCount, const QosTv *tvList)
{
    if ((g_udpChannelMgrCb == NULL) || (g_udpChannelMgrCb->OnQosEvent == NULL)) {
        return;
    }
    g_udpChannelMgrCb->OnQosEvent(channelId, eventId, tvCount, tvList);
}

static void OnFrameStats(int32_t channelId, const StreamSendStats *data)
{
    int32_t ret = ServerIpcStreamStats(channelId, CHANNEL_TYPE_UDP, data);
    SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_INFO, "notify frame stats to server, channelId:%d", channelId);
    if ((ret != SOFTBUS_OK) && (ret != SOFTBUS_NOT_IMPLEMENT)) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "ipc to server fail, reason:%d", ret);
        return;
    }
}

static void OnRippleStats(int32_t channelId, const TrafficStats *data)
{
    int32_t ret = ServerIpcRippleStats(channelId, CHANNEL_TYPE_UDP, data);
    SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_INFO, "notify ripple stats to server, channelId:%d", channelId);
    if ((ret != SOFTBUS_OK) && (ret != SOFTBUS_NOT_IMPLEMENT)) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "ipc to server fail, reason:%d", ret);
        return;
    }
}

static IStreamListener g_streamCallcb = {
    .OnStatusChange = SetStreamChannelStatus,
    .OnStreamReceived = OnStreamReceived,
    .OnQosEvent = OnQosEvent,
    .OnFrameStats = OnFrameStats,
    .OnRippleStats = OnRippleStats,
};

int32_t TransOnstreamChannelOpened(const ChannelInfo *channel, int32_t *streamPort)
{
    SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_INFO, "OnstreamChannelOpened enter.");
    if (channel == NULL || streamPort == NULL) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "%s:invalid param.", __func__);
        return SOFTBUS_INVALID_PARAM;
    }
    StreamType streamType = (StreamType)channel->streamType;
    if (streamType != RAW_STREAM && streamType != COMMON_VIDEO_STREAM && streamType != COMMON_AUDIO_STREAM) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "stream type invalid. type = %d", channel->streamType);
        return SOFTBUS_INVALID_PARAM;
    }
    if (channel->isServer) {
        VtpStreamOpenParam p1 = {
            "DSOFTBUS_STREAM",
            channel->myIp,
            NULL,
            -1,
            streamType,
            (uint8_t*)channel->sessionKey,
            channel->keyLen,
        };

        int32_t port = StartVtpStreamChannelServer(channel->channelId, &p1, &g_streamCallcb);
        if (port <= 0) {
            SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "start stream channel as server failed.");
            return SOFTBUS_TRANS_UDP_START_STREAM_SERVER_FAILED;
        }
        *streamPort = port;
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_INFO, "stream server success, listen port = %d.", port);
    } else {
        VtpStreamOpenParam p1 = {
            "DSOFTBUS_STREAM",
            channel->myIp,
            channel->peerIp,
            channel->peerPort,
            streamType,
            (uint8_t *)channel->sessionKey,
            channel->keyLen,
        };

        int ret = StartVtpStreamChannelClient(channel->channelId, &p1, &g_streamCallcb);
        if (ret <= 0) {
            SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "start stream channel as client failed.ret:%d", ret);
            return SOFTBUS_TRANS_UDP_START_STREAM_CLIENT_FAILED;
        }
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_INFO, "stream start client success.");
        if ((g_udpChannelMgrCb == NULL) || (g_udpChannelMgrCb->OnUdpChannelOpened == NULL)) {
            SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "udp channel callback on udp channel opened is null.");
            return SOFTBUS_ERR;
        }
        g_udpChannelMgrCb->OnUdpChannelOpened(channel->channelId);
    }
    return SOFTBUS_OK;
}

int32_t TransSendStream(int32_t channelId, const StreamData *data, const StreamData *ext, const StreamFrameInfo *param)
{
    return SendVtpStream(channelId, data, ext, param);
}

int32_t TransCloseStreamChannel(int32_t channelId)
{
    SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_INFO, "close stream channel.[channelId = %d]", channelId);
    if (CloseVtpStreamChannel(channelId, "DSOFTBUS_STREAM") != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "close stream channel failed.");
        return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}
