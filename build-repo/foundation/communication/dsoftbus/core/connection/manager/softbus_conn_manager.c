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

#include "softbus_conn_manager.h"

#include <securec.h>

#include "br_connection.h"
#include "common_list.h"
#include "softbus_adapter_mem.h"
#include "softbus_adapter_thread.h"
#include "softbus_base_listener.h"
#include "softbus_ble_connection.h"
#include "softbus_conn_interface.h"
#include "softbus_datahead_transform.h"
#include "softbus_def.h"
#include "softbus_errcode.h"
#include "softbus_feature_config.h"
#include "softbus_log.h"
#include "softbus_socket.h"
#include "softbus_tcp_connect_manager.h"
#include "softbus_utils.h"
#include "softbus_hisysevt_connreporter.h"

ConnectFuncInterface *g_connManager[CONNECT_TYPE_MAX] = {0};
static SoftBusList *g_listenerList = NULL;
static bool g_isInited = false;
static SoftBusList *g_connTimeList = NULL;
#define SEC_TIME 1000LL

typedef struct TagConnListenerNode {
    ListNode node;
    ConnModule moduleId;
    ConnectCallback callback;
} ConnListenerNode;

typedef struct TagConnTimeNode {
    ListNode node;
    ConnectionInfo info;
    uint32_t startTime;
} ConnTimeNode;

static int32_t AddConnTimeNode(const ConnectionInfo *info, ConnTimeNode *timeNode)
{
    if (g_connTimeList == NULL) {
        SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "g_connTimeList is null");
        return SOFTBUS_ERR;
    }
    SoftBusSysTime now = {0};
    SoftBusGetTime(&now);
    timeNode->startTime = (uint32_t)now.sec * SEC_TIME + (uint32_t)now.usec / SEC_TIME;
    if (memcpy_s(&(timeNode->info), sizeof(ConnectionInfo), info, sizeof(ConnectionInfo)) != EOK) {
        SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "AddConnTimeNode:memcpy timenode failed");
        return SOFTBUS_ERR;
    }
    if (SoftBusMutexLock(&g_connTimeList->lock) != 0) {
        SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "AddConnTimeNode:lock mutex failed");
        return SOFTBUS_ERR;
    }
    ListAdd(&(g_connTimeList->list), &(timeNode->node));
    (void)SoftBusMutexUnlock(&g_connTimeList->lock);
    return SOFTBUS_OK;
}

static int32_t CompareConnectInfo(const ConnectionInfo *src, const ConnectionInfo *dst)
{
    if (src->type != dst->type) {
        return SOFTBUS_ERR;
    }
    switch (src->type) {
        case CONNECT_BLE:
            if (strcasecmp(src->bleInfo.bleMac, dst->bleInfo.bleMac) != 0) {
                SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "CompareConnectInfo:bleMac is different");
                return SOFTBUS_ERR;
            }
            break;
        case CONNECT_BR:
            if (strcasecmp(src->brInfo.brMac, dst->brInfo.brMac) != 0) {
                SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "CompareConnectInfo:brMac is different");
                return SOFTBUS_ERR;
            }
            break;
        case CONNECT_TCP:
            if (strcasecmp(src->socketInfo.addr, dst->socketInfo.addr) != 0) {
                SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "CompareConnectInfo:tcpIp is different");
                return SOFTBUS_ERR;
            }
            break;
        default:
            SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "CompareConnectInfo:do nothing");
            return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}

static ConnTimeNode *GetConnTimeNode(const ConnectionInfo *info)
{
    ConnTimeNode *listNode = NULL;
    if (SoftBusMutexLock(&g_connTimeList->lock) != 0) {
        SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "GetConnTimeNode lock mutex failed");
        return NULL;
    }
    LIST_FOR_EACH_ENTRY(listNode, &g_connTimeList->list, ConnTimeNode, node) {
        if (listNode != NULL) {
            if (CompareConnectInfo(&listNode->info, info) == SOFTBUS_OK) {
                (void)SoftBusMutexUnlock(&g_connTimeList->lock);
                return listNode;
            }
        }
    }
    (void)SoftBusMutexUnlock(&g_connTimeList->lock);
    return NULL;
}

static void FreeConnTimeNode(ConnTimeNode *timeNode)
{
    ConnTimeNode *removeNode = NULL;
    ConnTimeNode *next = NULL;
    if (g_connTimeList == NULL) {
        SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "connTimeList is null");
        return;
    }

    if (SoftBusMutexLock(&g_connTimeList->lock) != 0) {
        SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "lock mutex failed");
        return;
    }

    LIST_FOR_EACH_ENTRY_SAFE(removeNode, next, &g_connTimeList->list, ConnTimeNode, node) {
        if (removeNode->info.type == timeNode->info.type) {
            if (CompareConnectInfo(&removeNode->info, &timeNode->info) == SOFTBUS_OK) {
                ListDelete(&(removeNode->node));
                break;
            }
        }
    }
    (void)SoftBusMutexUnlock(&g_connTimeList->lock);
    SoftBusFree(removeNode);
    return;
}

static int32_t ModuleCheck(ConnModule moduleId)
{
    ConnModule id[] = {
        MODULE_TRUST_ENGINE, MODULE_HICHAIN, MODULE_AUTH_SDK, MODULE_AUTH_CONNECTION, MODULE_MESSAGE_SERVICE,
        MODULE_AUTH_CHANNEL, MODULE_AUTH_MSG, MODULE_BLUETOOTH_MANAGER, MODULE_CONNECTION, MODULE_DIRECT_CHANNEL,
        MODULE_PROXY_CHANNEL, MODULE_DEVICE_AUTH, MODULE_P2P_LINK, MODULE_UDP_INFO, MODULE_PKG_VERIFY,
        MODULE_META_AUTH, MODULE_BLE_NET, MODULE_BLE_CONN
    };
    int32_t i;
    int32_t idNum = sizeof(id) / sizeof(ConnModule);

    for (i = 0; i < idNum; i++) {
        if (moduleId == id[i]) {
            return SOFTBUS_OK;
        }
    }
    SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "check module fail %d", moduleId);
    return SOFTBUS_ERR;
}

static int32_t ConnTypeCheck(ConnectType type)
{
    if (type >= CONNECT_TYPE_MAX) {
        SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "type is over max %d", type);
        return SOFTBUS_CONN_INVALID_CONN_TYPE;
    }

    if (g_connManager[type] == NULL) {
        SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "type is %d", type);
        return SOFTBUS_CONN_MANAGER_OP_NOT_SUPPORT;
    }
    return SOFTBUS_OK;
}

static int32_t GetAllListener(ConnListenerNode **node)
{
    ConnListenerNode *listenerNode = NULL;
    int32_t cnt = 0;

    if (g_listenerList == NULL) {
        SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "listener list is null");
        return cnt;
    }

    if (g_listenerList->cnt == 0) {
        SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "listener cnt is null");
        return cnt;
    }

    if (SoftBusMutexLock(&g_listenerList->lock) != 0) {
        SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "lock mutex failed");
        return 0;
    }
    *node = SoftBusCalloc(g_listenerList->cnt * sizeof(ConnListenerNode));
    if (*node == NULL) {
        (void)SoftBusMutexUnlock(&g_listenerList->lock);
        return cnt;
    }
    LIST_FOR_EACH_ENTRY(listenerNode, &g_listenerList->list, ConnListenerNode, node) {
        if (memcpy_s(*node + cnt, sizeof(ConnListenerNode), listenerNode, sizeof(ConnListenerNode)) != EOK) {
            SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "mem error");
            continue;
        }
        cnt++;
    }
    (void)SoftBusMutexUnlock(&g_listenerList->lock);
    return cnt;
}

static int32_t GetListenerByModuleId(ConnModule moduleId, ConnListenerNode *node)
{
    ConnListenerNode *listenerNode = NULL;

    if (g_listenerList == NULL) {
        return SOFTBUS_ERR;
    }
    int ret = SOFTBUS_OK;
    if (SoftBusMutexLock(&g_listenerList->lock) != 0) {
        SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "lock mutex failed");
        return SOFTBUS_ERR;
    }
    LIST_FOR_EACH_ENTRY(listenerNode, &g_listenerList->list, ConnListenerNode, node) {
        if (listenerNode->moduleId == moduleId) {
            if (memcpy_s(node, sizeof(ConnListenerNode), listenerNode, sizeof(ConnListenerNode)) != EOK) {
                ret = SOFTBUS_ERR;
            }
            (void)SoftBusMutexUnlock(&g_listenerList->lock);
            return ret;
        }
    }
    (void)SoftBusMutexUnlock(&g_listenerList->lock);
    return SOFTBUS_ERR;
}

static int32_t AddListener(ConnModule moduleId, const ConnectCallback *callback)
{
    ConnListenerNode *item = NULL;
    ConnListenerNode *listNode = NULL;

    if (g_listenerList == NULL) {
        return SOFTBUS_ERR;
    }
    if (SoftBusMutexLock(&g_listenerList->lock) != 0) {
        SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "lock mutex failed");
        return SOFTBUS_ERR;
    }
    LIST_FOR_EACH_ENTRY(listNode, &g_listenerList->list, ConnListenerNode, node) {
        if (listNode->moduleId == moduleId) {
            (void)SoftBusMutexUnlock(&g_listenerList->lock);
            return SOFTBUS_ERR;
        }
    }
    item = (ConnListenerNode *)SoftBusCalloc(sizeof(ConnListenerNode));
    if (item == NULL) {
        SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "malloc failed");
        (void)SoftBusMutexUnlock(&g_listenerList->lock);
        return SOFTBUS_ERR;
    }
    item->moduleId = moduleId;
    if (memcpy_s(&(item->callback), sizeof(ConnectCallback), callback, sizeof(ConnectCallback)) != EOK) {
        SoftBusFree(item);
        (void)SoftBusMutexUnlock(&g_listenerList->lock);
        return SOFTBUS_ERR;
    }
    ListAdd(&(g_listenerList->list), &(item->node));
    g_listenerList->cnt++;
    (void)SoftBusMutexUnlock(&g_listenerList->lock);
    return SOFTBUS_OK;
}

static void DelListener(ConnModule moduleId)
{
    ConnListenerNode *removeNode = NULL;
    if (g_listenerList == NULL) {
        SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "listenerList is null");
        return;
    }

    if (SoftBusMutexLock(&g_listenerList->lock) != 0) {
        SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "lock mutex failed");
        return;
    }

    LIST_FOR_EACH_ENTRY(removeNode, &g_listenerList->list, ConnListenerNode, node) {
        if (removeNode->moduleId == moduleId) {
            ListDelete(&(removeNode->node));
            SoftBusFree(removeNode);
            g_listenerList->cnt--;
            break;
        }
    }
    (void)SoftBusMutexUnlock(&g_listenerList->lock);
    return;
}

uint32_t ConnGetHeadSize(void)
{
    return sizeof(ConnPktHead);
}

uint32_t ConnGetNewRequestId(ConnModule moduleId)
{
#define REQID_MAX 1000000
    (void)moduleId;
    static uint32_t reqId = 1;
    reqId++;
    reqId = reqId % REQID_MAX + 1;
    return reqId;
}

void ConnManagerRecvData(uint32_t connectionId, ConnModule moduleId, int64_t seq, char *data, int32_t len)
{
    ConnListenerNode listener;
    int32_t ret;
    char* pkt = NULL;
    int32_t pktLen;

    if (data == NULL) {
        return;
    }

    if (len <= (int32_t)sizeof(ConnPktHead)) {
        SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "len %d \r\n", len);
        return;
    }

    ret = GetListenerByModuleId(moduleId, &listener);
    if (ret == SOFTBUS_ERR) {
        SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "GetListenerByModuleId failed moduleId %d \r\n", moduleId);
        return;
    }

    pktLen = len - sizeof(ConnPktHead);
    pkt = data + sizeof(ConnPktHead);
    listener.callback.OnDataReceived(connectionId, moduleId, seq, pkt, pktLen);
    return;
}

static void ReportConnectTime(const ConnectionInfo *info)
{
    if (info == NULL) {
        SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "ReportConnectTime:info is null");
        return;
    }
    SoftBusSysTime time = {0};
    ConnTimeNode *timeNode = GetConnTimeNode(info);
    if (timeNode == NULL) {
        SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "ReportConnectTime:get timeNode failed");
    } else {
        SoftBusGetTime(&time);
        uint32_t tmpTime = (uint32_t)time.sec * SEC_TIME + (uint32_t)time.usec / SEC_TIME;
        tmpTime -= timeNode->startTime;
        int ret = SoftbusRecordConnInfo(info->type, SOFTBUS_EVT_CONN_SUCC, tmpTime);
        if (ret != SOFTBUS_OK) {
            SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "ReportConnectTime:report conn time failed");
        }
        FreeConnTimeNode(timeNode);
    }
}

static void RecordStartTime(const ConnectOption *info)
{
    ConnectionInfo conInfo = {0};
    conInfo.type = info->type;
    switch (info->type) {
        case CONNECT_BR:
            if (memcpy_s(&conInfo.brInfo.brMac, BT_MAC_LEN, info->brOption.brMac, BT_MAC_LEN) != EOK) {
                SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "RecordStartTime:brMac memcpy failed");
                return;
            }
            break;
        case CONNECT_BLE:
            if (memcpy_s(&conInfo.bleInfo.bleMac, BT_MAC_LEN, info->bleOption.bleMac, BT_MAC_LEN) != EOK) {
                SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "RecordStartTime:bleMac memcpy failed");
                return;
            }
            break;
        case CONNECT_TCP:
            if (memcpy_s(&conInfo.socketInfo.addr, MAX_SOCKET_ADDR_LEN, info->socketOption.addr,
                MAX_SOCKET_ADDR_LEN) != EOK) {
                SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "RecordStartTime:addr memcpy failed");
                return;
            }
            break;
        default:
            SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "RecordStartTime:do nothing");
            break;
    }
    ConnTimeNode *timeNode = GetConnTimeNode(&conInfo);
    if (timeNode == NULL) {
        timeNode = (ConnTimeNode *)SoftBusCalloc(sizeof(ConnTimeNode));
        if (timeNode == NULL) {
            SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "malloc node failed");
        } else if (AddConnTimeNode(&conInfo, timeNode) != SOFTBUS_OK) {
            SoftBusFree(timeNode);
            SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "AddConnTimeNode failed");
        }
    }
}
static int32_t InitTimeNodeList()
{
    if (g_connTimeList == NULL) {
        g_connTimeList = CreateSoftBusList();
        if (g_connTimeList == NULL) {
            SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "create list failed \r\n");
            return SOFTBUS_ERR;
        }
    }
    return SOFTBUS_OK;
}

void ConnManagerConnected(uint32_t connectionId, const ConnectionInfo *info)
{
    ConnListenerNode *node = NULL;
    ConnListenerNode *listener = NULL;

    int32_t num = GetAllListener(&node);
    if (num == 0 || node == NULL) {
        SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "get node failed connId %u", connectionId);
        return;
    }

    for (int32_t i = 0; i < num; i++) {
        listener = node + i;
        listener->callback.OnConnected(connectionId, info);
    }
    SoftBusFree(node);
    ReportConnectTime(info);
    return;
}

void ConnManagerDisconnected(uint32_t connectionId, const ConnectionInfo *info)
{
    ConnListenerNode *node = NULL;
    ConnListenerNode *listener = NULL;

    int32_t num = GetAllListener(&node);
    if (num == 0 || node == NULL) {
        return;
    }
    for (int32_t i = 0; i < num; i++) {
        listener = node + i;
        listener->callback.OnDisconnected(connectionId, info);
    }
    SoftBusFree(node);
    return;
}

int32_t ConnSetConnectCallback(ConnModule moduleId, const ConnectCallback *callback)
{
    if (ModuleCheck(moduleId) != SOFTBUS_OK) {
        return SOFTBUS_INVALID_PARAM;
    }

    if (callback == NULL) {
        return SOFTBUS_INVALID_PARAM;
    }

    if ((callback->OnConnected == NULL) ||
        (callback->OnDisconnected == NULL) ||
        (callback->OnDataReceived == NULL)) {
        return SOFTBUS_INVALID_PARAM;
    }
    return AddListener(moduleId, callback);
}

void ConnUnSetConnectCallback(ConnModule moduleId)
{
    DelListener(moduleId);
    return;
}

int32_t ConnTypeIsSupport(ConnectType type)
{
    return ConnTypeCheck(type);
}

int32_t ConnConnectDevice(const ConnectOption *info, uint32_t requestId, const ConnectResult *result)
{
    if (info == NULL) {
        return SOFTBUS_INVALID_PARAM;
    }

    if (ConnTypeCheck(info->type) != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "connect type is err %d", info->type);
        return SOFTBUS_CONN_MANAGER_TYPE_NOT_SUPPORT;
    }

    if (g_connManager[info->type]->ConnectDevice == NULL) {
        SoftBusReportConnFaultEvt(info->type, SOFTBUS_HISYSEVT_CONN_MANAGER_OP_NOT_SUPPORT);
        return SOFTBUS_CONN_MANAGER_OP_NOT_SUPPORT;
    }
    RecordStartTime(info);
    return g_connManager[info->type]->ConnectDevice(info, requestId, result);
}

int32_t ConnPostBytes(uint32_t connectionId, ConnPostData *data)
{
    uint32_t type;
    ConnPktHead *head = NULL;

    if (data == NULL || data->buf == NULL) {
        return SOFTBUS_INVALID_PARAM;
    }

    if (data->len <= sizeof(ConnPktHead) || data->len > INT32_MAX) {
        SoftBusFree(data->buf);
        return SOFTBUS_CONN_MANAGER_PKT_LEN_INVALID;
    }

    type = (connectionId >> CONNECT_TYPE_SHIFT);
    if (ConnTypeCheck((ConnectType)type) != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "connectionId type is err %d", type);
        SoftBusFree(data->buf);
        return SOFTBUS_CONN_MANAGER_TYPE_NOT_SUPPORT;
    }

    if (g_connManager[type]->PostBytes == NULL) {
        SoftBusFree(data->buf);
        return SOFTBUS_CONN_MANAGER_OP_NOT_SUPPORT;
    }

    head = (ConnPktHead *)data->buf;
    head->magic = MAGIC_NUMBER;
    head->flag = data->flag;
    head->module = data->module;
    head->len = data->len - sizeof(ConnPktHead);
    head->seq = data->seq;
    PackConnPktHead(head);
    return g_connManager[type]->PostBytes(connectionId, data->buf, (int32_t)(data->len), data->pid, data->flag);
}

int32_t ConnDisconnectDevice(uint32_t connectionId)
{
    uint32_t type = (connectionId >> CONNECT_TYPE_SHIFT);
    if (ConnTypeCheck((ConnectType)type) != SOFTBUS_OK) {
        return SOFTBUS_CONN_MANAGER_TYPE_NOT_SUPPORT;
    }

    if (g_connManager[type]->DisconnectDevice == NULL) {
        return SOFTBUS_CONN_MANAGER_OP_NOT_SUPPORT;
    }
    return g_connManager[type]->DisconnectDevice(connectionId);
}

int32_t ConnDisconnectDeviceAllConn(const ConnectOption *option)
{
    if (option == NULL) {
        return SOFTBUS_INVALID_PARAM;
    }

    if (ConnTypeCheck(option->type) != SOFTBUS_OK) {
        return SOFTBUS_CONN_MANAGER_TYPE_NOT_SUPPORT;
    }

    if (g_connManager[option->type]->DisconnectDeviceNow == NULL) {
        return SOFTBUS_CONN_MANAGER_OP_NOT_SUPPORT;
    }
    return g_connManager[option->type]->DisconnectDeviceNow(option);
}

int32_t ConnGetConnectionInfo(uint32_t connectionId, ConnectionInfo *info)
{
    uint32_t type = (connectionId >> CONNECT_TYPE_SHIFT);
    if (ConnTypeCheck((ConnectType)type) != SOFTBUS_OK) {
        return SOFTBUS_CONN_MANAGER_TYPE_NOT_SUPPORT;
    }

    if (g_connManager[type]->GetConnectionInfo == NULL) {
        return SOFTBUS_CONN_MANAGER_OP_NOT_SUPPORT;
    }

    return g_connManager[type]->GetConnectionInfo(connectionId, info);
}

int32_t ConnStartLocalListening(const LocalListenerInfo *info)
{
    if (info == NULL) {
        return SOFTBUS_INVALID_PARAM;
    }

    if (ConnTypeCheck(info->type) != SOFTBUS_OK) {
        return SOFTBUS_CONN_MANAGER_TYPE_NOT_SUPPORT;
    }

    if (g_connManager[info->type]->StartLocalListening == NULL) {
        return SOFTBUS_CONN_MANAGER_OP_NOT_SUPPORT;
    }

    return g_connManager[info->type]->StartLocalListening(info);
}

int32_t ConnStopLocalListening(const LocalListenerInfo *info)
{
    if (info == NULL) {
        return SOFTBUS_INVALID_PARAM;
    }

    if (ConnTypeCheck(info->type) != SOFTBUS_OK) {
        return SOFTBUS_CONN_MANAGER_TYPE_NOT_SUPPORT;
    }

    if (g_connManager[info->type]->StopLocalListening == NULL) {
        return SOFTBUS_CONN_MANAGER_OP_NOT_SUPPORT;
    }

    return g_connManager[info->type]->StopLocalListening(info);
}

ConnectCallback g_connManagerCb = {0};

int32_t ConnServerInit(void)
{
    ConnectFuncInterface *connectObj = NULL;

    if (g_isInited) {
        return SOFTBUS_ERR;
    }

    int32_t ret = ConnInitSockets();
    if (ret != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "ConnInitSockets failed!ret=%" PRId32 " \r\n", ret);
        return ret;
    }

    ret = InitBaseListener();
    if (ret != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "InitBaseListener failed!ret=%" PRId32 " \r\n", ret);
        return ret;
    }

    g_connManagerCb.OnConnected = ConnManagerConnected;
    g_connManagerCb.OnDisconnected = ConnManagerDisconnected;
    g_connManagerCb.OnDataReceived = ConnManagerRecvData;

    int isSupportTcp = 0;
    (void)SoftbusGetConfig(SOFTBUS_INT_SUPPORT_TCP_PROXY, (unsigned char *)&isSupportTcp, sizeof(isSupportTcp));
    if (isSupportTcp) {
        connectObj = ConnInitTcp(&g_connManagerCb);
        if (connectObj != NULL) {
            g_connManager[CONNECT_TCP] = connectObj;
            SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_INFO, "init tcp ok \r\n");
        }
    }

    connectObj = ConnInitBr(&g_connManagerCb);
    if (connectObj != NULL) {
        g_connManager[CONNECT_BR] = connectObj;
        SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_INFO, "init br ok \r\n");
    }

    connectObj = ConnInitBle(&g_connManagerCb);
    if (connectObj != NULL) {
        g_connManager[CONNECT_BLE] = connectObj;
        SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_INFO, "init ble ok \r\n");
    }

    if (g_listenerList == NULL) {
        g_listenerList = CreateSoftBusList();
        if (g_listenerList == NULL) {
            SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "create list failed \r\n");
            return SOFTBUS_ERR;
        }
    }
    InitTimeNodeList();
        
    g_isInited = true;
    SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_INFO, "connect manager init success. \r\n");
    return SOFTBUS_OK;
}

void ConnServerDeinit(void)
{
    if (!g_isInited) {
        return;
    }

    ConnListenerNode *item = NULL;
    if (g_listenerList != NULL) {
        while (!IsListEmpty(&g_listenerList->list)) {
            item = LIST_ENTRY((&g_listenerList->list)->next, ConnListenerNode, node);
            ListDelete(&item->node);
            SoftBusFree(item);
        }
        DestroySoftBusList(g_listenerList);
        g_listenerList = NULL;
    }

    DeinitBaseListener();

    g_isInited = false;
}

bool CheckActiveConnection(const ConnectOption *info)
{
    if (info == NULL) {
        return false;
    }

    if (ConnTypeCheck(info->type) != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "connect type is err %d", info->type);
        return false;
    }

    if (g_connManager[info->type]->CheckActiveConnection == NULL) {
        return false;
    }

    return g_connManager[info->type]->CheckActiveConnection(info);
}

int32_t ConnUpdateConnection(uint32_t connectionId, UpdateOption *option)
{
    if (option == NULL) {
        return SOFTBUS_INVALID_PARAM;
    }

    uint32_t type = (connectionId >> CONNECT_TYPE_SHIFT);
    if (ConnTypeCheck((ConnectType)type) != SOFTBUS_OK) {
        return SOFTBUS_CONN_MANAGER_TYPE_NOT_SUPPORT;
    }
    if (g_connManager[type]->UpdateConnection == NULL) {
        return SOFTBUS_CONN_MANAGER_OP_NOT_SUPPORT;
    }
    return g_connManager[type]->UpdateConnection(connectionId, option);
}