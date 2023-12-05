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

#include "messenger_device_status_manager.h"

#include <stdlib.h>

#include "securec.h"
#include "softbus_bus_center.h"
#include "softbus_common.h"

#include "messenger_utils.h"
#include "utils_log.h"
#include "utils_mem.h"

static void MessengerOnNodeOnline(NodeBasicInfo *info);
static void MessengerOnNodeOffline(NodeBasicInfo *info);
static void MessengerOnNodeBasicInfoChanged(NodeBasicInfoType type, NodeBasicInfo *info);
static int32_t InitDeviceOnlineProcessor(const DeviceIdentify *devId, uint32_t devType, void *para);

typedef struct DeviceStatusManager {
    const INodeStateCb nodeStateCb;
    DeviceStatusReceiver deviceStatusReceiver;
    const char *pkgName;
    WorkQueue *queue;
} DeviceStatusManager;

typedef struct QueueStatusData {
    DeviceIdentify srcIdentity;
    uint32_t status;
    uint32_t devType;
} QueueStatusData;

static DeviceStatusManager *GetDeviceManagerInstance(void)
{
    static DeviceStatusManager manager = {
        {
            .events = EVENT_NODE_STATE_ONLINE | EVENT_NODE_STATE_OFFLINE,
            .onNodeOnline = MessengerOnNodeOnline,
            .onNodeOffline = MessengerOnNodeOffline,
            .onNodeBasicInfoChanged = MessengerOnNodeBasicInfoChanged,
        },
        .deviceStatusReceiver = NULL,
        .pkgName = NULL,
        .queue = NULL,
    };
    return &manager;
}

static void ProcessDeviceStatusReceived(const uint8_t *data, uint32_t len)
{
    if (data == NULL || len == 0) {
        return;
    }
    QueueStatusData *queueData = (QueueStatusData *)data;
    if (sizeof(QueueStatusData) != len) {
        SECURITY_LOG_ERROR("ProcessDeviceStatusReceived, invalid input");
        return;
    }

    DeviceStatusManager *instance = GetDeviceManagerInstance();
    DeviceStatusReceiver deviceStatusReceiver = instance->deviceStatusReceiver;
    if (deviceStatusReceiver == NULL) {
        SECURITY_LOG_ERROR("ProcessSessionMessageReceived, messageReceiver is null");
        return;
    }
    deviceStatusReceiver(&queueData->srcIdentity, queueData->status, queueData->devType);
    FREE(queueData);
}

static void ProcessDeviceStatusReceiver(const DeviceIdentify *devId, uint32_t status, uint32_t devType)
{
    DeviceStatusManager *instance = GetDeviceManagerInstance();

    WorkQueue *queue = instance->queue;
    if (queue == NULL) {
        SECURITY_LOG_ERROR("ProcessDeviceStatusReceiver, queue is null");
        return;
    }

    DeviceStatusReceiver deviceStatusReceiver = instance->deviceStatusReceiver;
    if (deviceStatusReceiver == NULL) {
        SECURITY_LOG_ERROR("ProcessDeviceStatusReceiver, messageReceiver is null");
        return;
    }

    QueueStatusData *data = MALLOC(sizeof(QueueStatusData));
    if (data == NULL) {
        SECURITY_LOG_ERROR("ProcessDeviceStatusReceiver, malloc result null");
        return;
    }

    uint32_t ret = (uint32_t)memcpy_s(&data->srcIdentity, sizeof(DeviceIdentify), devId, sizeof(DeviceIdentify));
    if (ret != EOK) {
        SECURITY_LOG_ERROR("ProcessDeviceStatusReceiver, memcpy failed");
        FREE(data);
        return;
    }
    data->devType = devType;
    data->status = status;

    ret = QueueWork(queue, ProcessDeviceStatusReceived, (uint8_t *)data, sizeof(QueueStatusData));
    if (ret != WORK_QUEUE_OK) {
        SECURITY_LOG_ERROR("ProcessDeviceStatusReceiver, QueueWork failed, ret is %{public}u", ret);
        FREE(data);
        return;
    }
}

static void MessengerOnNodeStateChange(NodeBasicInfo *info, uint32_t state)
{
    if (info == NULL) {
        SECURITY_LOG_ERROR("MessengerOnNodeStateChange process input is null.");
        return;
    }
    DeviceStatusManager *instance = GetDeviceManagerInstance();

    char udid[UDID_BUF_LEN] = {0};
    if (GetNodeKeyInfo(instance->pkgName, info->networkId, NODE_KEY_UDID, (uint8_t *)udid, UDID_BUF_LEN) != 0) {
        SECURITY_LOG_ERROR("MessengerOnNodeStateChange process get device identity error.");
        return;
    }

    DeviceIdentify identity = {0, {0}};
    identity.length = UDID_BUF_LEN - 1;
    if (memcpy_s(identity.identity, DEVICE_ID_MAX_LEN, udid, UDID_BUF_LEN - 1) != EOK) {
        SECURITY_LOG_ERROR("MessengerOnNodeStateChange copy device error");
    }
    uint32_t maskId = MaskDeviceIdentity(udid, UDID_BUF_LEN);
    SECURITY_LOG_INFO("MessengerOnNodeStateChange device(%{public}x*** change to %{public}s, deviceType is %{public}d)",
        maskId, (state == EVENT_NODE_STATE_ONLINE) ? "online" : "offline", info->deviceTypeId);

    ProcessDeviceStatusReceiver(&identity, state, info->deviceTypeId);
}

static void MessengerOnNodeOnline(NodeBasicInfo *info)
{
    return MessengerOnNodeStateChange(info, EVENT_NODE_STATE_ONLINE);
}

static void MessengerOnNodeOffline(NodeBasicInfo *info)
{
    return MessengerOnNodeStateChange(info, EVENT_NODE_STATE_OFFLINE);
}

static void MessengerOnNodeBasicInfoChanged(NodeBasicInfoType type, NodeBasicInfo *info)
{
    (void)type;
    (void)info;
}

static int32_t InitDeviceOnlineProcessor(const DeviceIdentify *devId, uint32_t devType, void *para)
{
    (void)para;
    ProcessDeviceStatusReceiver(devId, EVENT_NODE_STATE_ONLINE, devType);
    return 0;
}

bool InitDeviceStatusManager(WorkQueue *queue, const char *pkgName, DeviceStatusReceiver deviceStatusReceiver)
{
    if (deviceStatusReceiver == NULL) {
        return false;
    }

    DeviceStatusManager *instance = GetDeviceManagerInstance();
    instance->pkgName = pkgName;
    instance->deviceStatusReceiver = deviceStatusReceiver;
    instance->queue = queue;

    int try = 0;
    int32_t ret = RegNodeDeviceStateCb(pkgName, (INodeStateCb *)&instance->nodeStateCb);
    while (ret != 0 && try < MAX_TRY_TIMES) {
        MessengerSleep(1); // sleep 1 second and try again
        ret = RegNodeDeviceStateCb(pkgName, (INodeStateCb *)&instance->nodeStateCb);
        try++;
    }

    if (ret != 0) {
        SECURITY_LOG_ERROR("InitDeviceManager RegNodeDeviceStateCb failed = %{public}d", ret);
        return false;
    }

    MessengerForEachDeviceProcess(InitDeviceOnlineProcessor, NULL);
    SECURITY_LOG_INFO("InitDeviceManager RegNodeDeviceStateCb success");
    return true;
}

bool DeInitDeviceStatusManager(void)
{
    DeviceStatusManager *instance = GetDeviceManagerInstance();

    int32_t ret = UnregNodeDeviceStateCb((INodeStateCb *)&instance->nodeStateCb);
    if (ret != 0) {
        SECURITY_LOG_ERROR("DeInitDeviceManager UnregNodeDeviceStateCb failed = %{public}d", ret);
        return false;
    }
    instance->pkgName = NULL;
    instance->deviceStatusReceiver = NULL;
    instance->queue = NULL;
    DestroyWorkQueue(instance->queue);

    SECURITY_LOG_INFO("DeInitDeviceManager UnregNodeDeviceStateCb success");
    return true;
}

static bool MessengerConvertNodeToIdentity(const NodeBasicInfo *node, DeviceIdentify *devId)
{
    if ((node == NULL) || (devId == NULL)) {
        return false;
    }
    char udid[UDID_BUF_LEN] = {0};

    DeviceStatusManager *instance = GetDeviceManagerInstance();
    if (GetNodeKeyInfo(instance->pkgName, node->networkId, NODE_KEY_UDID, (uint8_t *)udid, UDID_BUF_LEN) != 0) {
        SECURITY_LOG_ERROR("MessengerGetSelfDeviceIdentify GetNodeKeyInfo error.");
        return false;
    }

    if (memcpy_s(devId->identity, DEVICE_ID_MAX_LEN, udid, DEVICE_ID_MAX_LEN) != EOK) {
        SECURITY_LOG_ERROR("MessengerGetSelfDeviceIdentify memcpy error");
        return false;
    }
    devId->length = DEVICE_ID_MAX_LEN;
    return true;
}

bool MessengerGetDeviceNodeBasicInfo(const DeviceIdentify *devId, NodeBasicInfo *info)
{
    if (devId == NULL || info == NULL) {
        return false;
    }
    DeviceStatusManager *instance = GetDeviceManagerInstance();

    NodeBasicInfo *infoList = NULL;

    int infoListLen = 0;
    int32_t ret = GetAllNodeDeviceInfo(instance->pkgName, &infoList, &infoListLen);
    if (ret != 0) {
        SECURITY_LOG_ERROR("MessengerGetDeviceOnlineStatus GetAllNodeDeviceInfo failed = %{public}d", ret);
        return false;
    }

    bool find = false;
    for (int loop = 0; loop < infoListLen; loop++) {
        const NodeBasicInfo *node = infoList + loop;
        DeviceIdentify curr = {DEVICE_ID_MAX_LEN, {0}};
        bool convert = MessengerConvertNodeToIdentity(node, &curr);
        if (convert != true) {
            continue;
        }

        if (IsSameDevice(devId, &curr)) {
            find = true;
            (void)memcpy_s(info, sizeof(NodeBasicInfo), node, sizeof(NodeBasicInfo));
            break;
        }
    }

    if (infoList != NULL) {
        FreeNodeInfo(infoList);
    }
    return find;
}

bool MessengerGetDeviceOnlineStatus(const DeviceIdentify *devId, uint32_t *devType)
{
    if (devId == NULL) {
        return false;
    }
    NodeBasicInfo info = {{0}, {0}, 0};
    bool result = MessengerGetDeviceNodeBasicInfo(devId, &info);
    if (result == true && devType != NULL) {
        *devType = info.deviceTypeId;
    }
    return result;
}

bool MessengerGetDeviceNetworkId(const DeviceIdentify *devId, char *networkId, uint32_t len)
{
    if (devId == NULL || networkId == NULL || len == 0) {
        return false;
    }
    NodeBasicInfo info = {{0}, {0}, 0};
    bool result = MessengerGetDeviceNodeBasicInfo(devId, &info);
    if (result != true) {
        return false;
    }

    int32_t ret = memcpy_s(networkId, len, info.networkId, NETWORK_ID_BUF_LEN);
    if (ret != EOK) {
        SECURITY_LOG_ERROR("MessengerGetDeviceNetworkId memcpy error");
        return false;
    }
    return true;
}

bool MessengerGetSelfDeviceIdentify(DeviceIdentify *devId, uint32_t *devType)
{
    if (devId == NULL || devType == NULL) {
        return false;
    }

    DeviceStatusManager *instance = GetDeviceManagerInstance();

    NodeBasicInfo info;
    int32_t ret = GetLocalNodeDeviceInfo(instance->pkgName, &info);
    if (ret != 0) {
        SECURITY_LOG_ERROR("MessengerGetSelfDeviceIdentify GetLocalNodeDeviceInfo failed = %{public}d", ret);
        return false;
    }

    bool convert = MessengerConvertNodeToIdentity(&info, devId);
    if (convert == false) {
        return false;
    }
    *devType = info.deviceTypeId;

    uint32_t maskId = MaskDeviceIdentity((const char *)&devId->identity[0], UDID_BUF_LEN);
    SECURITY_LOG_DEBUG("MessengerGetSelfDeviceIdentify device %{public}x***, deviceType is %{public}d", maskId,
        info.deviceTypeId);
    return true;
}

void MessengerForEachDeviceProcess(const DeviceProcessor processor, void *para)
{
    if (processor == NULL) {
        return;
    }
    DeviceStatusManager *instance = GetDeviceManagerInstance();

    NodeBasicInfo *infoList = NULL;

    int infoListLen = 0;
    int32_t ret = GetAllNodeDeviceInfo(instance->pkgName, &infoList, &infoListLen);
    if (ret != 0) {
        SECURITY_LOG_ERROR("MessengerForEachDeviceProcess GetAllNodeDeviceInfo failed = %{public}d", ret);
        return;
    }

    for (int loop = 0; loop < infoListLen; loop++) {
        const NodeBasicInfo *node = infoList + loop;
        DeviceIdentify devId = {DEVICE_ID_MAX_LEN, {0}};
        bool convert = MessengerConvertNodeToIdentity(node, &devId);
        if (convert == true) {
            processor(&devId, node->deviceTypeId, para);
        }
    }

    if (infoList != NULL) {
        FreeNodeInfo(infoList);
    }
}
