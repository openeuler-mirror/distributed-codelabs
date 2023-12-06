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

#ifndef SOFTBUS_PROXYCHANNEL_MESSAGE_H
#define SOFTBUS_PROXYCHANNEL_MESSAGE_H
#include "stdint.h"
#include "common_list.h"
#include "softbus_app_info.h"
#include "softbus_conn_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef enum {
    PROXYCHANNEL_MSG_TYPE_NORMAL,
    PROXYCHANNEL_MSG_TYPE_HANDSHAKE,
    PROXYCHANNEL_MSG_TYPE_HANDSHAKE_ACK,
    PROXYCHANNEL_MSG_TYPE_RESET,
    PROXYCHANNEL_MSG_TYPE_KEEPALIVE,
    PROXYCHANNEL_MSG_TYPE_KEEPALIVE_ACK,
    PROXYCHANNEL_MSG_TYPE_MAX
} MsgType;

#define JSON_KEY_TYPE "TYPE"
#define JSON_KEY_IDENTITY "IDENTITY"
#define JSON_KEY_DEVICE_ID "DEVICE_ID"
#define JSON_KEY_DST_BUS_NAME "DST_BUS_NAME"
#define JSON_KEY_SRC_BUS_NAME "SRC_BUS_NAME"
#define JSON_KEY_HAS_PRIORITY "HAS_PRIORITY"
#define JSON_KEY_UID "UID"
#define JSON_KEY_PID "PID"
#define JSON_KEY_GROUP_ID "GROUP_ID"
#define JSON_KEY_PKG_NAME "PKG_NAME"
#define JSON_KEY_SESSION_KEY "SESSION_KEY"
#define JSON_KEY_REQUEST_ID "REQUEST_ID"
#define JSON_KEY_ENCRYPT "ENCRYPT"
#define JSON_KEY_ALGORITHM "ALGORITHM"
#define JSON_KEY_CRC "CRC"
#define JSON_KEY_BUSINESS_TYPE "BUSINESS_TYPE"

typedef struct {
    uint8_t type; // MsgType
    uint8_t cipher;
    int16_t myId;
    int16_t peerId;
    int16_t reserved;
} ProxyMessageHead;

typedef struct {
    ProxyMessageHead msgHead;
    int32_t dateLen;
    char *data;
    uint32_t connId;
    int64_t authId; /* for cipher */
} ProxyMessage;

#define VERSION 1
#define PROXY_CHANNEL_HEAD_LEN 8
#define VERSION_SHIFT 4
#define FOUR_BIT_MASK 0xF
#define ENCRYPTED 0x1
#define AUTH_SERVER_SIDE 0x2
#define USE_BLE_CIPHER 0x4
#define PROXY_BYTES_LENGTH_MAX (4 * 1024 * 1024)
#define PROXY_MESSAGE_LENGTH_MAX 1024

#define IDENTITY_LEN 32
typedef enum {
    PROXY_CHANNEL_STATUS_PYH_CONNECTED,
    PROXY_CHANNEL_STATUS_PYH_CONNECTING,
    PROXY_CHANNEL_STATUS_HANDSHAKEING,
    PROXY_CHANNEL_STATUS_KEEPLIVEING,
    PROXY_CHANNEL_STATUS_TIMEOUT,
    PROXY_CHANNEL_STATUS_HANDSHAKE_TIMEOUT,
    PROXY_CHANNEL_STATUS_COMPLETED
} ProxyChannelStatus;

#define BASE64KEY 45 // encrypt SessionKey len
typedef struct {
    char sessionKeyBase64[BASE64KEY];
    size_t len;
} SessionKeyBase64;

typedef struct {
    ListNode node;
    int32_t channelId;
    int32_t reqId;
    int8_t isServer;
    int8_t status;
    uint16_t timeout;
    int16_t myId;
    int16_t peerId;
    uint32_t connId;
    ConnectType type;
    int32_t seq;
    char identity[IDENTITY_LEN + 1];
    AppInfo appInfo;
    int64_t authId; /* for cipher */
} ProxyChannelInfo;

typedef struct {
    int32_t active;
    int32_t timeout;
    int32_t sliceNumber;
    int32_t expectedSeq;
    int32_t dataLen;
    int32_t bufLen;
    char *data;
} SliceProcessor;

#define PROCESSOR_MAX 3
typedef struct {
    ListNode head;
    int32_t channelId;
    SliceProcessor processor[PROCESSOR_MAX];
} ChannelSliceProcessor;

typedef struct {
    uint8_t *inData;
    uint32_t inLen;
    uint8_t *outData;
    uint32_t outLen;
} ProxyDataInfo;

int32_t TransProxyUnPackHandshakeErrMsg(const char *msg, int* errCode, int32_t len);
int32_t TransProxyUnpackHandshakeAckMsg(const char *msg, ProxyChannelInfo *chanInfo, int32_t len);
char* TransProxyPackHandshakeAckMsg(ProxyChannelInfo *chan);
char* TransProxyPackHandshakeErrMsg(int32_t errCode);
int32_t TransProxyParseMessage(char *data, int32_t len, ProxyMessage *msg);
int32_t TransProxyPackMessage(ProxyMessageHead *msg, int64_t authId, ProxyDataInfo *dataInfo);
char* TransProxyPackHandshakeMsg(ProxyChannelInfo *info);
int32_t TransProxyUnpackHandshakeMsg(const char *msg, ProxyChannelInfo *chan, int32_t len);
char* TransProxyPackIdentity(const char *identity);
int32_t TransProxyUnpackIdentity(const char *msg, char *identity, uint32_t identitySize, int32_t len);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif
