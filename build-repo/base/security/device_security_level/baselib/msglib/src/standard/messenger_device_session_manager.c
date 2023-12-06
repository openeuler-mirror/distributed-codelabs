/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "messenger_device_session_manager.h"

#include <stdlib.h>

#include "securec.h"
#include "session.h"

#include "messenger_device_status_manager.h"
#include "messenger_utils.h"
#include "utils_list.h"
#include "utils_log.h"
#include "utils_mem.h"
#include "utils_mutex.h"

#define IS_SERVER 0
#define MSG_BUFF_MAX_LENGTH (81920 * 4)

static int MessengerOnSessionOpened(int sessionId, int result);
static void MessengerOnSessionClosed(int sessionId);
static void MessengerOnBytesReceived(int sessionId, const void *data, unsigned int dataLen);
static void MessengerOnMessageReceived(int sessionId, const void *data, unsigned int dataLen);

typedef struct DeviceSessionManager {
    const ISessionListener listener;
    ListHead pendingSendList;
    ListHead openedSessionList;
    DeviceMessageReceiver messageReceiver;
    MessageSendResultNotifier sendResultNotifier;
    const char *pkgName;
    const char *primarySessName;
    const char *secondarySessName;
    WorkQueue *queue;
    Mutex mutex;
} DeviceSessionManager;

typedef struct QueueMsgData {
    DeviceIdentify srcIdentity;
    uint32_t msgLen;
    uint8_t msgData[1];
} QueueMsgData;

typedef struct PendingMsgData {
    ListNode link;
    uint32_t transNo;
    DeviceIdentify destIdentity;
    uint32_t msgLen;
    uint8_t msgData[1];
} PendingMsgData;

typedef struct SessionInfo {
    ListNode link;
    int32_t sessionId;
    uint32_t maskId;
    DeviceIdentify identity;
} SessionInfo;

static DeviceSessionManager *GetDeviceSessionManagerInstance(void)
{
    static DeviceSessionManager manager = {
        {
            .OnSessionOpened = MessengerOnSessionOpened,
            .OnSessionClosed = MessengerOnSessionClosed,
            .OnBytesReceived = MessengerOnBytesReceived,
            .OnMessageReceived = MessengerOnMessageReceived,
        },
        .pendingSendList = INIT_LIST(manager.pendingSendList),
        .openedSessionList = INIT_LIST(manager.openedSessionList),
        .messageReceiver = NULL,
        .sendResultNotifier = NULL,
        .queue = NULL,
        .mutex = INITED_MUTEX,
    };
    return &manager;
}

static void ProcessSessionMessageReceived(const uint8_t *data, uint32_t len)
{
    if (data == NULL || len == 0) {
        return;
    }
    QueueMsgData *queueData = (QueueMsgData *)data;
    if (queueData->msgLen + sizeof(QueueMsgData) != len) {
        SECURITY_LOG_ERROR("invalid input");
        return;
    }

    DeviceSessionManager *instance = GetDeviceSessionManagerInstance();
    DeviceMessageReceiver messageReceiver = instance->messageReceiver;
    if (messageReceiver == NULL) {
        SECURITY_LOG_ERROR("messageReceiver is null");
        return;
    }
    messageReceiver(&queueData->srcIdentity, queueData->msgData, queueData->msgLen);
    FREE(queueData);
}

static void OnSessionMessageReceived(const DeviceIdentify *devId, const uint8_t *msg, uint32_t msgLen)
{
    DeviceSessionManager *instance = GetDeviceSessionManagerInstance();
    WorkQueue *queue = instance->queue;
    if (queue == NULL) {
        SECURITY_LOG_ERROR("queue is null");
        return;
    }
    DeviceMessageReceiver messageReceiver = instance->messageReceiver;
    if (messageReceiver == NULL) {
        SECURITY_LOG_ERROR("messageReceiver is null");
        return;
    }
    uint32_t queueDataLen = sizeof(QueueMsgData) + msgLen;
    QueueMsgData *queueData = MALLOC(queueDataLen);
    if (queueData == NULL) {
        SECURITY_LOG_ERROR("malloc result null");
        return;
    }
    uint32_t ret = (uint32_t)memcpy_s(&queueData->srcIdentity, sizeof(DeviceIdentify), devId, sizeof(DeviceIdentify));
    if (ret != EOK) {
        SECURITY_LOG_ERROR("memcpy failed");
        FREE(queueData);
        return;
    }
    ret = (uint32_t)memcpy_s(queueData->msgData, msgLen, msg, msgLen);
    if (ret != EOK) {
        SECURITY_LOG_ERROR("memcpy failed");
        FREE(queueData);
        return;
    }
    queueData->msgLen = msgLen;
    ret = QueueWork(queue, ProcessSessionMessageReceived, (uint8_t *)queueData, queueDataLen);
    if (ret != WORK_QUEUE_OK) {
        SECURITY_LOG_ERROR("QueueWork failed, ret is %{public}u", ret);
        FREE(queueData);
        return;
    }
}

static bool GetDeviceIdentityFromSessionId(int sessionId, DeviceIdentify *identity, uint32_t *maskId)
{
    if (identity == NULL || maskId == NULL) {
        return false;
    }
    char networkId[DEVICE_ID_MAX_LEN + 1] = {0};
    int ret = GetPeerDeviceId(sessionId, networkId, DEVICE_ID_MAX_LEN + 1);
    if (ret != 0) {
        SECURITY_LOG_INFO("GetPeerDeviceId failed, sessionId is %{public}d, result is %{public}d", sessionId, ret);
        return false;
    }

    if (!MessengerGetDeviceIdentifyByNetworkId(networkId, identity)) {
        SECURITY_LOG_ERROR("MessengerGetDeviceIdentifyByNetworkId failed");
        return false;
    }

    *maskId = MaskDeviceIdentity((const char *)identity->identity, DEVICE_ID_MAX_LEN);

    return true;
}

static int MessengerOnSessionOpened(int sessionId, int result)
{
    int side = GetSessionSide(sessionId);
    SECURITY_LOG_INFO("sessionId=%{public}d, side=%{public}s, result=%{public}d", sessionId,
        (side == IS_SERVER) ? "server" : "client", result);

    if (side == IS_SERVER) {
        return 0;
    }
    if (result != 0) {
        return 0;
    }

    DeviceIdentify identity = {DEVICE_ID_MAX_LEN, {0}};
    uint32_t maskId;
    bool ret = GetDeviceIdentityFromSessionId(sessionId, &identity, &maskId);
    if (ret == false) {
        SECURITY_LOG_ERROR("GetDeviceIdentityFromSessionId failed");
        return 0;
    }

    SessionInfo *sessionInfo = MALLOC(sizeof(SessionInfo));
    if (sessionInfo == NULL) {
        SECURITY_LOG_ERROR("malloc failed, sessionInfo is null");
        return 0;
    }
    sessionInfo->sessionId = sessionId;
    sessionInfo->maskId = maskId;
    (void)memcpy_s(&sessionInfo->identity, sizeof(DeviceIdentify), &identity, sizeof(DeviceIdentify));

    DeviceSessionManager *instance = GetDeviceSessionManagerInstance();
    LockMutex(&instance->mutex);
    AddListNodeBefore(&sessionInfo->link, &instance->openedSessionList);

    ListNode *node = NULL;
    ListNode *temp = NULL;

    FOREACH_LIST_NODE_SAFE (node, &instance->pendingSendList, temp) {
        PendingMsgData *msgData = LIST_ENTRY(node, PendingMsgData, link);
        if (!IsSameDevice(&msgData->destIdentity, &identity)) {
            continue;
        }

        RemoveListNode(node);
        int sent = SendBytes(sessionId, msgData->msgData, msgData->msgLen);
        if (sent != 0) {
            SECURITY_LOG_ERROR("SendBytes error code = %{public}d", ret);
        }
        FREE(msgData);
    }

    UnlockMutex(&instance->mutex);
    return 0;
}

static void MessengerOnSessionClosed(int sessionId)
{
    int side = GetSessionSide(sessionId);
    SECURITY_LOG_INFO("sessionId=%{public}d, side=%{public}s", sessionId, (side == IS_SERVER) ? "server" : "client");

    if (side == IS_SERVER) {
        return;
    }

    DeviceSessionManager *instance = GetDeviceSessionManagerInstance();
    LockMutex(&instance->mutex);
    ListNode *node = NULL;
    ListNode *temp = NULL;
    FOREACH_LIST_NODE_SAFE (node, &instance->openedSessionList, temp) {
        SessionInfo *info = LIST_ENTRY(node, SessionInfo, link);
        if (info->sessionId == sessionId) {
            SECURITY_LOG_INFO("device=%{public}x", info->maskId);
            RemoveListNode(node);
            FREE(info);
        }
    }
    UnlockMutex(&instance->mutex);
}

static void MessengerOnBytesReceived(int sessionId, const void *data, unsigned int dataLen)
{
    if (data == NULL || dataLen == 0 || dataLen > MSG_BUFF_MAX_LENGTH) {
        SECURITY_LOG_ERROR("invalid msg received");
        return;
    }

    DeviceIdentify identity = {DEVICE_ID_MAX_LEN, {0}};
    uint32_t maskId;
    bool ret = GetDeviceIdentityFromSessionId(sessionId, &identity, &maskId);
    if (ret == false) {
        return;
    }
    SECURITY_LOG_INFO("device=%{public}x***, data length is %{public}u", maskId, dataLen);
    OnSessionMessageReceived(&identity, (const uint8_t *)data, (uint32_t)dataLen);
}

static void MessengerOnMessageReceived(int sessionId, const void *data, unsigned int dataLen)
{
    return MessengerOnBytesReceived(sessionId, data, dataLen);
}

static bool TryToCreateSessionServer(const char *pkgName, const char *sessionName, const ISessionListener *listener)
{
    int try = 0;
    int ret = CreateSessionServer(pkgName, sessionName, listener);
    while (ret != 0 && try < MAX_TRY_TIMES) {
        MessengerSleep(1); // sleep 1 second and try again
        ret = CreateSessionServer(pkgName, sessionName, listener);
        try++;
    }

    if (ret != 0) {
        SECURITY_LOG_ERROR("CreateSessionServer failed = %{public}d", ret);
        return false;
    }
    return true;
}

bool InitDeviceSessionManager(WorkQueue *queue, const MessengerConfig *config)
{
    if ((queue == NULL) || (config == NULL)) {
        return false;
    }
    DeviceSessionManager *inst = GetDeviceSessionManagerInstance();
    inst->pkgName = config->pkgName;
    inst->primarySessName = config->primarySessName;
    inst->secondarySessName = config->secondarySessName;
    inst->messageReceiver = config->messageReceiver;
    inst->sendResultNotifier = config->sendResultNotifier;
    inst->queue = queue;

    bool succ = TryToCreateSessionServer(inst->pkgName, inst->primarySessName, &inst->listener);
    SECURITY_LOG_INFO("CreateSessionServer %{public}s = %{public}s", inst->primarySessName, succ ? "succ" : "fail");

    if (inst->secondarySessName == NULL) {
        return succ;
    }

    succ = TryToCreateSessionServer(inst->pkgName, inst->secondarySessName, &inst->listener);
    SECURITY_LOG_INFO("CreateSessionServer %{public}s = %{public}s", inst->secondarySessName, succ ? "succ" : "fail");
    return succ;
}

bool DeInitDeviceSessionManager(void)
{
    DeviceSessionManager *instance = GetDeviceSessionManagerInstance();
    int ret = RemoveSessionServer(instance->pkgName, instance->primarySessName);
    if (ret != 0) {
        SECURITY_LOG_ERROR("RemoveSessionServer %{public}s failed = %{public}d", instance->primarySessName, ret);
    }

    if (instance->secondarySessName) {
        ret = RemoveSessionServer(instance->pkgName, instance->primarySessName);
        if (ret != 0) {
            SECURITY_LOG_ERROR("RemoveSessionServer %{public}s failed = %{public}d", instance->primarySessName, ret);
        }
    }

    LockMutex(&instance->mutex);
    instance->pkgName = NULL;
    instance->primarySessName = NULL;
    instance->secondarySessName = NULL;
    instance->messageReceiver = NULL;
    instance->sendResultNotifier = NULL;
    instance->queue = NULL;

    ListNode *node = NULL;
    ListNode *temp = NULL;

    FOREACH_LIST_NODE_SAFE (node, &instance->pendingSendList, temp) {
        PendingMsgData *msgData = LIST_ENTRY(node, PendingMsgData, link);
        RemoveListNode(node);
        FREE(msgData);
    }

    FOREACH_LIST_NODE_SAFE (node, &instance->openedSessionList, temp) {
        SessionInfo *info = LIST_ENTRY(node, SessionInfo, link);
        RemoveListNode(node);
        FREE(info);
    }

    DestroyWorkQueue(instance->queue);
    UnlockMutex(&instance->mutex);

    SECURITY_LOG_INFO("RemoveSessionServer success");
    return true;
}

static bool GetOpenedSessionId(const DeviceIdentify *devId, int32_t *sessionId)
{
    if (devId == NULL || sessionId == NULL) {
        return false;
    }
    DeviceSessionManager *instance = GetDeviceSessionManagerInstance();

    bool find = false;
    LockMutex(&instance->mutex);
    ListNode *node = NULL;
    uint32_t mask = MaskDeviceIdentity((const char *)&devId->identity[0], devId->length);

    FOREACH_LIST_NODE (node, &instance->openedSessionList) {
        SessionInfo *sessionInfo = LIST_ENTRY(node, SessionInfo, link);
        if (IsSameDevice(&sessionInfo->identity, devId)) {
            *sessionId = sessionInfo->sessionId;
            find = true;
            break;
        }
    }
    UnlockMutex(&instance->mutex);
    SECURITY_LOG_DEBUG("device %{public}x %{public}s", mask, find ? "exist" : "no exist");
    return find;
}

static void PushMsgDataToPendingList(uint32_t transNo, const DeviceIdentify *devId, const uint8_t *msg, uint32_t msgLen)
{
    PendingMsgData *data = MALLOC(sizeof(PendingMsgData) + msgLen);
    if (data == NULL) {
        SECURITY_LOG_ERROR("malloc failed, data is null");
        return;
    }
    data->transNo = transNo;
    data->msgLen = msgLen;
    (void)memcpy_s(&data->destIdentity, sizeof(DeviceIdentify), devId, sizeof(DeviceIdentify));
    (void)memcpy_s(data->msgData, msgLen, msg, msgLen);
    DeviceSessionManager *instance = GetDeviceSessionManagerInstance();
    LockMutex(&instance->mutex);
    AddListNodeBefore(&data->link, &instance->pendingSendList);
    UnlockMutex(&instance->mutex);
}

static void CreateNewDeviceSession(const DeviceIdentify *devId)
{
    uint32_t mask = MaskDeviceIdentity((const char *)&devId->identity[0], devId->length);
    char deviceName[DEVICE_ID_MAX_LEN + 1] = {0};
    bool succ = MessengerGetNetworkIdByDeviceIdentify(devId, deviceName, DEVICE_ID_MAX_LEN + 1);
    if (!succ) {
        SECURITY_LOG_ERROR("get network id failed");
        return;
    }

    const SessionAttribute attr = {
        .dataType = TYPE_BYTES,
    };

    const char *primary = GetDeviceSessionManagerInstance()->primarySessName;
    const char *secondary = GetDeviceSessionManagerInstance()->secondarySessName;

    int ret = OpenSession(primary, primary, deviceName, "", &attr);
    if (ret <= 0) {
        // open failed, need to try again.
        ret = OpenSession(primary, primary, deviceName, "", &attr);
    }
    SECURITY_LOG_INFO("open 1st session %{public}s device %{public}x ret is %{public}d", primary, mask, ret);

    if (secondary == NULL || ret) {
        return;
    }

    ret = OpenSession(primary, secondary, deviceName, "", &attr);
    if (ret <= 0) {
        // open failed, need to try again.
        ret = OpenSession(primary, secondary, deviceName, "", &attr);
    }
    SECURITY_LOG_INFO("open 2nd session %{public}s device %{public}x ret is %{public}d", secondary, mask, ret);
}

void MessengerSendMsgTo(uint64_t transNo, const DeviceIdentify *devId, const uint8_t *msg, uint32_t msgLen)
{
    if (devId == NULL || msg == NULL || msgLen == 0 || msgLen > MSG_BUFF_MAX_LENGTH) {
        SECURITY_LOG_ERROR("invalid params");
        return;
    }

    static DeviceIdentify self = {0, {0}};
    uint32_t devType;
    MessengerGetSelfDeviceIdentify(&self, &devType);

    if (IsSameDevice(&self, devId)) {
        SECURITY_LOG_DEBUG("loopback msg");
        OnSessionMessageReceived(devId, msg, msgLen);
        return;
    }

    int32_t sessionId;
    bool find = GetOpenedSessionId(devId, &sessionId);
    if (find) {
        int ret = SendBytes(sessionId, msg, msgLen);
        if (ret != 0) {
            SECURITY_LOG_ERROR("SendBytes error code = %{public}d", ret);
        }
        return;
    }

    PushMsgDataToPendingList(transNo, devId, msg, msgLen);
    CreateNewDeviceSession(devId);
}