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

#include "disc_manager.h"
#include "common_list.h"
#include "disc_ble_dispatcher.h"
#include "disc_coap.h"
#include "securec.h"
#include "softbus_adapter_mem.h"
#include "softbus_adapter_thread.h"
#include "softbus_def.h"
#include "softbus_error_code.h"
#include "softbus_log.h"
#include "softbus_hisysevt_discreporter.h"
#include "softbus_utils.h"

#define SEC_TIME_PARAM 1000LL

static bool g_firstDiscFlag = false;
static bool g_onDeviceFound = false;
static int64_t g_currentTime = 0;

static bool g_isInited = false;

static SoftBusList *g_publishInfoList = NULL;
static SoftBusList *g_discoveryInfoList = NULL;

static DiscoveryFuncInterface *g_discCoapInterface = NULL;
static DiscoveryFuncInterface *g_discBleInterface = NULL;

static DiscInnerCallback g_discMgrMediumCb;

static ListNode g_capabilityList[CAPABILITY_MAX_BITNUM];

static const char *g_discModuleMap[] = {
    "MODULE_LNN",
    "MODULE_CONN",
};

typedef enum {
    MIN_SERVICE = 0,
    PUBLISH_SERVICE = MIN_SERVICE,
    PUBLISH_INNER_SERVICE = 1,
    SUBSCRIBE_SERVICE = 2,
    SUBSCRIBE_INNER_SERVICE = 3,
    MAX_SERVICE = SUBSCRIBE_INNER_SERVICE,
} ServiceType;

typedef union {
    PublishOption publishOption;
    SubscribeOption subscribeOption;
} InnerOption;

typedef union  {
    IServerDiscInnerCallback serverCb;
    DiscInnerCallback innerCb;
} InnerCallback;

typedef struct {
    ListNode node;
    char packageName[PKG_NAME_SIZE_MAX];
    InnerCallback callback;
    uint32_t infoNum;
    ListNode InfoList;
} DiscItem;

typedef struct {
    ListNode node;
    int32_t id;
    DiscoverMode mode;
    ExchangeMedium medium;
    InnerOption option;
    ListNode capNode;
    DiscItem *item;
} DiscInfo;

typedef struct {
    ListNode node;
    int32_t id;
    char *pkgName;
} IdContainer;

static void BitmapSet(uint32_t *bitMap, uint32_t pos)
{
    *bitMap |= 1U << pos;
}

static bool IsBitmapSet(const uint32_t *bitMap, uint32_t pos)
{
    return ((1U << pos) & (*bitMap)) ? true : false;
}

static int32_t CallSpecificInterfaceFunc(const InnerOption *option, const DiscoveryFuncInterface *interface,
                                         const DiscoverMode mode, InterfaceFuncType type)
{
    DISC_CHECK_AND_RETURN_RET_LOG(interface != NULL, SOFTBUS_DISCOVER_MANAGER_INNERFUNCTION_FAIL,
                                  "interface is null");
    switch (type) {
        case PUBLISH_FUNC:
            return ((mode == DISCOVER_MODE_ACTIVE) ? (interface->Publish(&(option->publishOption))) :
                (interface->StartScan(&(option->publishOption))));
        case UNPUBLISH_FUNC:
            return ((mode == DISCOVER_MODE_ACTIVE) ? (interface->Unpublish(&(option->publishOption))) :
                (interface->StopScan(&(option->publishOption))));
        case STARTDISCOVERTY_FUNC:
            return ((mode == DISCOVER_MODE_ACTIVE) ? (interface->StartAdvertise(&(option->subscribeOption))) :
                (interface->Subscribe(&(option->subscribeOption))));
        case STOPDISCOVERY_FUNC:
            return ((mode == DISCOVER_MODE_ACTIVE) ? (interface->StopAdvertise(&(option->subscribeOption))) :
                (interface->Unsubscribe(&(option->subscribeOption))));
        default:
            return SOFTBUS_DISCOVER_MANAGER_INNERFUNCTION_FAIL;
    }
}

static int64_t GetCurrentTimeUs(void)
{
    SoftBusSysTime time;
    SoftBusGetTime(&time);
    return time.sec * SEC_TIME_PARAM + (time.usec / SEC_TIME_PARAM);
}

static void StartFirstDiscoveryTimeRecord(void)
{
    if (g_firstDiscFlag == false) {
        g_currentTime = GetCurrentTimeUs();
        g_firstDiscFlag = true;
        g_onDeviceFound = false;
        DLOGI("First discovery time record start");
    }
}

static void FinishFirstDiscoveryTimeRecord(void)
{
    if (g_onDeviceFound == false) {
        int64_t lastTime = g_currentTime;
        g_currentTime = GetCurrentTimeUs();
        g_currentTime -= lastTime;
        g_onDeviceFound = true;
        g_firstDiscFlag = false;
        DLOGI("First discovery time record finish");
    }
}

static int32_t CallInterfaceByMedium(const DiscInfo *info, const InterfaceFuncType type)
{
    SoftbusRecordDiscScanTimes(info->medium);
    StartFirstDiscoveryTimeRecord();

    switch (info->medium) {
        case COAP:
            return CallSpecificInterfaceFunc(&(info->option), g_discCoapInterface, info->mode, type);
        case BLE:
            return CallSpecificInterfaceFunc(&(info->option), g_discBleInterface, info->mode, type);
        case AUTO: {
            int coapRes = CallSpecificInterfaceFunc(&(info->option), g_discCoapInterface, info->mode, type);
            int bleRes = CallSpecificInterfaceFunc(&(info->option), g_discBleInterface, info->mode, type);
            DISC_CHECK_AND_RETURN_RET_LOG(coapRes == SOFTBUS_OK || bleRes == SOFTBUS_OK,
                                          SOFTBUS_DISCOVER_MANAGER_INNERFUNCTION_FAIL, "all medium failed");
            return SOFTBUS_OK;
        }
        default:
            return SOFTBUS_DISCOVER_MANAGER_INNERFUNCTION_FAIL;
    }
}

static int32_t TransferStringCapToBitmap(const char *capability)
{
    DISC_CHECK_AND_RETURN_RET_LOG(capability != NULL, SOFTBUS_DISCOVER_MANAGER_CAPABILITY_INVALID,
                                  "capability is null");

    for (uint32_t i = 0; i < sizeof(g_capabilityMap) / sizeof(g_capabilityMap[0]); i++) {
        if (strcmp(capability, g_capabilityMap[i].capability) == 0) {
            DLOGI("capability=%s", capability);
            return g_capabilityMap[i].bitmap;
        }
    }

    return SOFTBUS_DISCOVER_MANAGER_CAPABILITY_INVALID;
}

static void AddDiscInfoToCapabilityList(DiscInfo *info, const ServiceType type)
{
    DISC_CHECK_AND_RETURN_LOG(type == SUBSCRIBE_SERVICE || type == SUBSCRIBE_INNER_SERVICE, "no need to add");

    for (uint32_t tmp = 0; tmp < CAPABILITY_MAX_BITNUM; tmp++) {
        if (IsBitmapSet(&(info->option.subscribeOption.capabilityBitmap[0]), tmp) == true) {
            if (type == SUBSCRIBE_SERVICE) {
                ListTailInsert(&(g_capabilityList[tmp]), &(info->capNode));
            } else {
                ListNodeInsert(&(g_capabilityList[tmp]), &(info->capNode));
            }
            break;
        }
    }
}

static void RemoveDiscInfoFromCapabilityList(DiscInfo *info, const ServiceType type)
{
    DISC_CHECK_AND_RETURN_LOG(type == SUBSCRIBE_SERVICE || type == SUBSCRIBE_INNER_SERVICE, "no need to delete");
    ListDelete(&(info->capNode));
}

static void FreeDiscInfo(DiscInfo *info, const ServiceType type)
{
    if ((type == PUBLISH_SERVICE) || (type == PUBLISH_INNER_SERVICE)) {
        SoftBusFree(info->option.publishOption.capabilityData);
    }

    if ((type == SUBSCRIBE_SERVICE) || (type == SUBSCRIBE_INNER_SERVICE)) {
        SoftBusFree(info->option.subscribeOption.capabilityData);
    }
    SoftBusFree(info);
}

static bool IsInnerModule(const DiscInfo *infoNode)
{
    for (uint32_t i = 0; i < MODULE_MAX; i++) {
        DLOGI("%s", infoNode->item->packageName);
        if (strcmp(infoNode->item->packageName, g_discModuleMap[i]) == 0) {
            DLOGI("true");
            return true;
        }
    }
    DLOGI("false");
    return false;
}

static void InnerDeviceFound(const DiscInfo *infoNode, const DeviceInfo *device,
                             const InnerDeviceInfoAddtions *additions)
{
    if (IsInnerModule(infoNode) == false) {
        (void)infoNode->item->callback.serverCb.OnServerDeviceFound(infoNode->item->packageName, device, additions);
        return;
    }

    if (GetCallLnnStatus()) {
        FinishFirstDiscoveryTimeRecord();
        (void)SoftbusRecordFirstDiscTime(infoNode->medium, (uint32_t)g_currentTime);
        if (infoNode->item->callback.innerCb.OnDeviceFound != NULL) {
            infoNode->item->callback.innerCb.OnDeviceFound(device, additions);
        }
    }
}

static void DiscOnDeviceFound(const DeviceInfo *device, const InnerDeviceInfoAddtions *additions)
{
    DISC_CHECK_AND_RETURN_LOG(device != NULL, "device is null");
    DISC_CHECK_AND_RETURN_LOG(additions != NULL, "additions is null");

    DLOGI("capabilityBitmap=%d, medium=%d", device->capabilityBitmap[0], additions->medium);
    for (uint32_t tmp = 0; tmp < CAPABILITY_MAX_BITNUM; tmp++) {
        if (IsBitmapSet((uint32_t *)device->capabilityBitmap, tmp) == false) {
            continue;
        }

        if (SoftBusMutexLock(&(g_discoveryInfoList->lock)) != 0) {
            DLOGE("lock failed");
            return;
        }
        DiscInfo *infoNode = NULL;
        LIST_FOR_EACH_ENTRY(infoNode, &(g_capabilityList[tmp]), DiscInfo, capNode) {
            DLOGI("find callback id=%d", infoNode->id);
            InnerDeviceFound(infoNode, device, additions);
        }
        (void)SoftBusMutexUnlock(&(g_discoveryInfoList->lock));
    }
}

static int32_t CheckPublishInfo(const PublishInfo *info)
{
    DISC_CHECK_AND_RETURN_RET_LOG(info->mode == DISCOVER_MODE_PASSIVE || info->mode == DISCOVER_MODE_ACTIVE,
                                  SOFTBUS_INVALID_PARAM, "mode is invalid");
    DISC_CHECK_AND_RETURN_RET_LOG(info->medium >= AUTO && info->medium <= COAP,
                                  SOFTBUS_DISCOVER_MANAGER_INVALID_MEDIUM, "mode is invalid");
    DISC_CHECK_AND_RETURN_RET_LOG(info->freq >= LOW && info->freq <= SUPER_HIGH,
                                  SOFTBUS_INVALID_PARAM, "freq is invalid");
    DISC_CHECK_AND_RETURN_RET_LOG((info->capabilityData == NULL && info->dataLen == 0) ||
                                  (info->capabilityData != NULL && info->dataLen <= MAX_CAPABILITYDATA_LEN),
                                  SOFTBUS_INVALID_PARAM, "data and length invalid");
    return SOFTBUS_OK;
}

static int32_t CheckSubscribeInfo(const SubscribeInfo *info)
{
    DISC_CHECK_AND_RETURN_RET_LOG(info->mode == DISCOVER_MODE_PASSIVE || info->mode == DISCOVER_MODE_ACTIVE,
                                  SOFTBUS_INVALID_PARAM, "mode is invalid");
    DISC_CHECK_AND_RETURN_RET_LOG(info->medium >= AUTO && info->medium <= COAP,
                                  SOFTBUS_DISCOVER_MANAGER_INVALID_MEDIUM, "mode is invalid");
    DISC_CHECK_AND_RETURN_RET_LOG(info->freq >= LOW && info->freq <= SUPER_HIGH,
                                  SOFTBUS_INVALID_PARAM, "freq is invalid");
    DISC_CHECK_AND_RETURN_RET_LOG((info->capabilityData == NULL && info->dataLen == 0) ||
                                  (info->capabilityData != NULL && info->dataLen <= MAX_CAPABILITYDATA_LEN),
                                  SOFTBUS_INVALID_PARAM, "data and length invalid");
    return SOFTBUS_OK;
}

static void SetDiscItemCallback(DiscItem *itemNode, const InnerCallback *cb, const ServiceType type)
{
    if ((type != SUBSCRIBE_INNER_SERVICE) && (type != SUBSCRIBE_SERVICE)) {
        return;
    }
    if (type == SUBSCRIBE_SERVICE) {
        itemNode->callback.serverCb.OnServerDeviceFound = cb->serverCb.OnServerDeviceFound;
        return;
    }
    if ((itemNode->callback.innerCb.OnDeviceFound != NULL) && (cb->innerCb.OnDeviceFound == NULL)) {
        return;
    }
    itemNode->callback.innerCb.OnDeviceFound = cb->innerCb.OnDeviceFound;
}

static DiscItem *CreateDiscItem(SoftBusList *serviceList, const char *packageName, const InnerCallback *cb,
                                const ServiceType type)
{
    DiscItem *itemNode = (DiscItem *)SoftBusCalloc(sizeof(DiscItem));
    DISC_CHECK_AND_RETURN_RET_LOG(itemNode != NULL, NULL, "calloc item node failed");

    if (strcpy_s(itemNode->packageName, PKG_NAME_SIZE_MAX, packageName) != EOK) {
        SoftBusFree(itemNode);
        return NULL;
    }

    if ((type == PUBLISH_INNER_SERVICE) || (type == SUBSCRIBE_INNER_SERVICE)) {
        ListNodeInsert(&(serviceList->list), &(itemNode->node));
    } else if ((type == PUBLISH_SERVICE) || (type == SUBSCRIBE_SERVICE)) {
        ListTailInsert(&(serviceList->list), &(itemNode->node));
    }

    SetDiscItemCallback(itemNode, cb, type);

    serviceList->cnt++;
    ListInit(&(itemNode->InfoList));
    return itemNode;
}

static DiscInfo *CreateDiscInfoForPublish(const PublishInfo *info)
{
    DiscInfo *infoNode = (DiscInfo *)SoftBusCalloc(sizeof(DiscInfo));
    DISC_CHECK_AND_RETURN_RET_LOG(infoNode != NULL, NULL, "calloc info node failed");

    ListInit(&(infoNode->node));
    ListInit(&(infoNode->capNode));

    infoNode->id = info->publishId;
    infoNode->medium = info->medium;
    infoNode->mode = info->mode;

    PublishOption *option = &infoNode->option.publishOption;
    option->freq = info->freq;
    option->ranging = info->ranging;
    option->dataLen = info->dataLen;

    if (info->dataLen != 0) {
        option->capabilityData = (uint8_t *)SoftBusCalloc(info->dataLen);
        if (option->capabilityData == NULL) {
            DLOGE("alloc capability data failed");
            SoftBusFree(infoNode);
            return NULL;
        }
        (void)memcpy_s(option->capabilityData, info->dataLen, info->capabilityData, info->dataLen);
    }

    int32_t bitmap = TransferStringCapToBitmap(info->capability);
    if (bitmap < 0) {
        DLOGE("capability not found");
        FreeDiscInfo(infoNode, PUBLISH_SERVICE);
        return NULL;
    }
    BitmapSet(option->capabilityBitmap, (uint32_t)bitmap);

    return infoNode;
}

static DiscInfo *CreateDiscInfoForSubscribe(const SubscribeInfo *info)
{
    DiscInfo *infoNode = (DiscInfo *)SoftBusCalloc(sizeof(DiscInfo));
    DISC_CHECK_AND_RETURN_RET_LOG(infoNode != NULL, NULL, "alloc info node failed");

    ListInit(&(infoNode->node));
    ListInit(&(infoNode->capNode));

    infoNode->id = info->subscribeId;
    infoNode->medium = info->medium;
    infoNode->mode = info->mode;

    SubscribeOption *option = &infoNode->option.subscribeOption;
    option->freq = info->freq;
    option->dataLen = info->dataLen;
    option->isSameAccount = info->isSameAccount;
    option->isWakeRemote = info->isWakeRemote;

    if (info->dataLen != 0) {
        option->capabilityData = (uint8_t *)SoftBusCalloc(info->dataLen);
        if (option->capabilityData == NULL) {
            DLOGE("alloc capability data failed");
            SoftBusFree(infoNode);
            return NULL;
        }
        (void)memcpy_s(option->capabilityData, info->dataLen, info->capabilityData, info->dataLen);
    }

    int32_t bimap = TransferStringCapToBitmap(info->capability);
    if (bimap < 0) {
        DLOGE("capability not found");
        FreeDiscInfo(infoNode, SUBSCRIBE_SERVICE);
        return NULL;
    }
    BitmapSet(option->capabilityBitmap, (uint32_t)bimap);

    return infoNode;
}

static int32_t AddDiscInfoToList(SoftBusList *serviceList, const char *packageName, const InnerCallback *cb,
                                 DiscInfo *info, ServiceType type)
{
    if (SoftBusMutexLock(&(serviceList->lock)) != 0) {
        DLOGE("lock failed");
        return SOFTBUS_LOCK_ERR;
    }

    DiscItem *itemNode = NULL;
    bool exist = false;
    LIST_FOR_EACH_ENTRY(itemNode, &(serviceList->list), DiscItem, node) {
        if (strcmp(itemNode->packageName, packageName) != 0) {
            continue;
        }

        DiscInfo *infoNode = NULL;
        LIST_FOR_EACH_ENTRY(infoNode, &(itemNode->InfoList), DiscInfo, node) {
            if (infoNode->id == info->id) {
                DLOGI("id already existed");
                (void)SoftBusMutexUnlock(&(serviceList->lock));
                return SOFTBUS_DISCOVER_MANAGER_DUPLICATE_PARAM;
            }
        }

        SetDiscItemCallback(itemNode, cb, type);
        exist = true;
        itemNode->infoNum++;
        info->item = itemNode;
        ListTailInsert(&(itemNode->InfoList), &(info->node));
        AddDiscInfoToCapabilityList(info, type);
        break;
    }

    if (exist == false) {
        itemNode = CreateDiscItem(serviceList, packageName, cb, type);
        if (itemNode == NULL) {
            DLOGE("itemNode create failed");
            (void)SoftBusMutexUnlock(&(serviceList->lock));
            return SOFTBUS_DISCOVER_MANAGER_ITEM_NOT_CREATE;
        }

        itemNode->infoNum++;
        info->item = itemNode;
        ListTailInsert(&(itemNode->InfoList), &(info->node));
        AddDiscInfoToCapabilityList(info, type);
    }

    (void)SoftBusMutexUnlock(&(serviceList->lock));
    return SOFTBUS_OK;
}

static int32_t AddDiscInfoToPublishList(const char *packageName, const InnerCallback *cb, DiscInfo *info,
                                        ServiceType type)
{
    return AddDiscInfoToList(g_publishInfoList, packageName, cb, info, type);
}

static int32_t AddDiscInfoToDiscoveryList(const char *packageName, const InnerCallback *cb, DiscInfo *info,
                                          ServiceType type)
{
    return AddDiscInfoToList(g_discoveryInfoList, packageName, cb, info, type);
}

static DiscInfo *RemoveInfoFromList(SoftBusList *serviceList, const char *packageName, const int32_t id,
                                    const ServiceType type)
{
    if (SoftBusMutexLock(&(serviceList->lock)) != 0) {
        DLOGE("lock failed");
        return NULL;
    }

    bool isIdExist = false;
    DiscItem *itemNode = NULL;
    DiscInfo *infoNode = NULL;

    LIST_FOR_EACH_ENTRY(itemNode, &(serviceList->list), DiscItem, node) {
        if (strcmp(itemNode->packageName, packageName) != 0) {
            continue;
        }

        if (itemNode->infoNum == 0) {
            serviceList->cnt--;
            ListDelete(&(itemNode->node));
            SoftBusFree(itemNode);
            (void)SoftBusMutexUnlock(&(serviceList->lock));
            return NULL;
        }

        LIST_FOR_EACH_ENTRY(infoNode, &(itemNode->InfoList), DiscInfo, node) {
            if (infoNode->id != id) {
                continue;
            }
            isIdExist = true;
            itemNode->infoNum--;
            RemoveDiscInfoFromCapabilityList(infoNode, type);
            ListDelete(&(infoNode->node));

            if (itemNode->infoNum == 0) {
                serviceList->cnt--;
                ListDelete(&(itemNode->node));
                SoftBusFree(itemNode);
            }
            break;
        }
        break;
    }

    (void)SoftBusMutexUnlock(&(serviceList->lock));

    if (isIdExist == false) {
        DLOGI("can not find publishId");
        return NULL;
    }
    return infoNode;
}

static DiscInfo *RemoveInfoFromPublishList(const char *packageName, const int32_t id, const ServiceType type)
{
    return RemoveInfoFromList(g_publishInfoList, packageName, id, type);
}

static DiscInfo *RemoveInfoFromDiscoveryList(const char *packageName, const int32_t id, const ServiceType type)
{
    return RemoveInfoFromList(g_discoveryInfoList, packageName, id, type);
}

static int32_t InnerPublishService(const char *packageName, DiscInfo *info, const ServiceType type)
{
    int32_t ret = AddDiscInfoToPublishList(packageName, NULL, info, type);
    DISC_CHECK_AND_RETURN_RET_LOG(ret == SOFTBUS_OK, ret, "add info to list failed");

    ret = CallInterfaceByMedium(info, PUBLISH_FUNC);
    if (ret != SOFTBUS_OK) {
        DLOGE("DiscInterfaceByMedium failed");
        ListDelete(&(info->node));
        info->item->infoNum--;
    }

    return ret;
}

static int32_t InnerUnPublishService(const char *packageName, int32_t publishId, const ServiceType type)
{
    DiscInfo *infoNode = RemoveInfoFromPublishList(packageName, publishId, type);
    DISC_CHECK_AND_RETURN_RET_LOG(infoNode != NULL, SOFTBUS_DISCOVER_MANAGER_INFO_NOT_DELETE,
                                  "delete info from list failed");

    int32_t ret = CallInterfaceByMedium(infoNode, UNPUBLISH_FUNC);
    DISC_CHECK_AND_RETURN_RET_LOG(ret == SOFTBUS_OK, ret, "DiscInterfaceByMedium failed");

    FreeDiscInfo(infoNode, type);
    return SOFTBUS_OK;
}

static int32_t InnerStartDiscovery(const char *packageName, DiscInfo *info, const IServerDiscInnerCallback *cb,
                                   const ServiceType type)
{
    InnerCallback callback;
    callback.serverCb.OnServerDeviceFound = NULL;
    if (cb != NULL) {
        callback.serverCb.OnServerDeviceFound = cb->OnServerDeviceFound;
    }

    int32_t ret = AddDiscInfoToDiscoveryList(packageName, &callback, info, type);
    DISC_CHECK_AND_RETURN_RET_LOG(ret == SOFTBUS_OK, ret, "add info to list failed");

    (void)SoftBusReportDiscStartupEvt(packageName);

    ret = CallInterfaceByMedium(info, STARTDISCOVERTY_FUNC);
    if (ret != SOFTBUS_OK) {
        DLOGE("DiscInterfaceByMedium failed");
        RemoveDiscInfoFromCapabilityList(info, type);
        ListDelete(&(info->node));
        info->item->infoNum--;
    }
    return ret;
}

static int32_t InnerStopDiscovery(const char *packageName, int32_t subscribeId, const ServiceType type)
{
    DiscInfo *infoNode = RemoveInfoFromDiscoveryList(packageName, subscribeId, type);
    DISC_CHECK_AND_RETURN_RET_LOG(infoNode != NULL, SOFTBUS_DISCOVER_MANAGER_INFO_NOT_DELETE,
                                  "delete info from list failed");

    int32_t ret = CallInterfaceByMedium(infoNode, STOPDISCOVERY_FUNC);
    DISC_CHECK_AND_RETURN_RET_LOG(ret == SOFTBUS_OK, ret, "DiscInterfaceByMedium failed");

    FreeDiscInfo(infoNode, type);
    return SOFTBUS_OK;
}

static const char* TransferModuleIdToPackageName(DiscModule moduleId)
{
    return g_discModuleMap[moduleId - 1];
}

static int32_t InnerSetDiscoveryCallback(const char *packageName, const DiscInnerCallback *cb)
{
    if (SoftBusMutexLock(&(g_discoveryInfoList->lock)) != 0) {
        DLOGE("lock failed");
        return SOFTBUS_LOCK_ERR;
    }

    bool isIdExist = false;
    DiscItem *itemNode = NULL;
    InnerCallback callback;
    LIST_FOR_EACH_ENTRY(itemNode, &(g_discoveryInfoList->list), DiscItem, node) {
        if (strcmp(itemNode->packageName, packageName) != 0) {
            continue;
        }
        itemNode->callback.innerCb.OnDeviceFound = cb->OnDeviceFound;
        isIdExist = true;
        break;
    }
    if (isIdExist == false) {
        callback.innerCb.OnDeviceFound = cb->OnDeviceFound;
        itemNode = CreateDiscItem(g_discoveryInfoList, packageName, &callback, SUBSCRIBE_INNER_SERVICE);
        if (itemNode == NULL) {
            DLOGE("itemNode create failed");
            (void)SoftBusMutexUnlock(&(g_discoveryInfoList->lock));
            return SOFTBUS_DISCOVER_MANAGER_ITEM_NOT_CREATE;
        }
    }
    (void)SoftBusMutexUnlock(&(g_discoveryInfoList->lock));
    return SOFTBUS_OK;
}

int32_t DiscSetDiscoverCallback(DiscModule moduleId, const DiscInnerCallback *callback)
{
    DISC_CHECK_AND_RETURN_RET_LOG(moduleId >= MODULE_MIN && moduleId <= MODULE_MAX && callback != NULL,
                                  SOFTBUS_INVALID_PARAM, "invalid parameters");
    DISC_CHECK_AND_RETURN_RET_LOG(g_isInited == true, SOFTBUS_DISCOVER_MANAGER_NOT_INIT, "manager is not inited");
    return InnerSetDiscoveryCallback(TransferModuleIdToPackageName(moduleId), callback);
}

int32_t DiscPublish(DiscModule moduleId, const PublishInfo *info)
{
    DISC_CHECK_AND_RETURN_RET_LOG(moduleId >= MODULE_MIN && moduleId <= MODULE_MAX && info != NULL,
                                  SOFTBUS_INVALID_PARAM, "invalid parameters");
    DISC_CHECK_AND_RETURN_RET_LOG(info->mode == DISCOVER_MODE_ACTIVE, SOFTBUS_INVALID_PARAM, "mode is not active");
    DISC_CHECK_AND_RETURN_RET_LOG(CheckPublishInfo(info) == SOFTBUS_OK, SOFTBUS_INVALID_PARAM, "invalid info");
    DISC_CHECK_AND_RETURN_RET_LOG(g_isInited == true, SOFTBUS_DISCOVER_MANAGER_NOT_INIT, "manager is not inited");

    DiscInfo *infoNode = CreateDiscInfoForPublish(info);
    DISC_CHECK_AND_RETURN_RET_LOG(infoNode != NULL, SOFTBUS_DISCOVER_MANAGER_INFO_NOT_CREATE, "create info failed");

    int32_t ret = InnerPublishService(TransferModuleIdToPackageName(moduleId), infoNode, PUBLISH_INNER_SERVICE);
    if (ret != SOFTBUS_OK) {
        FreeDiscInfo(infoNode, PUBLISH_INNER_SERVICE);
    }
    return ret;
}

int32_t DiscStartScan(DiscModule moduleId, const PublishInfo *info)
{
    DISC_CHECK_AND_RETURN_RET_LOG(moduleId >= MODULE_MIN && moduleId <= MODULE_MAX && info != NULL,
                                  SOFTBUS_INVALID_PARAM, "invalid parameters");
    DISC_CHECK_AND_RETURN_RET_LOG(info->mode == DISCOVER_MODE_PASSIVE, SOFTBUS_INVALID_PARAM, "mode is not passive");
    DISC_CHECK_AND_RETURN_RET_LOG(CheckPublishInfo(info) == SOFTBUS_OK, SOFTBUS_INVALID_PARAM, "invalid info");
    DISC_CHECK_AND_RETURN_RET_LOG(g_isInited == true, SOFTBUS_DISCOVER_MANAGER_NOT_INIT, "manager is not inited");

    DiscInfo *infoNode = CreateDiscInfoForPublish(info);
    DISC_CHECK_AND_RETURN_RET_LOG(infoNode != NULL, SOFTBUS_DISCOVER_MANAGER_INFO_NOT_CREATE, "create info failed");

    int32_t ret = InnerPublishService(TransferModuleIdToPackageName(moduleId), infoNode, PUBLISH_INNER_SERVICE);
    if (ret != SOFTBUS_OK) {
        FreeDiscInfo(infoNode, PUBLISH_INNER_SERVICE);
    }
    return ret;
}

int32_t DiscUnpublish(DiscModule moduleId, int32_t publishId)
{
    DISC_CHECK_AND_RETURN_RET_LOG(moduleId >= MODULE_MIN && moduleId <= MODULE_MAX,
                                  SOFTBUS_INVALID_PARAM, "invalid moduleId");
    DISC_CHECK_AND_RETURN_RET_LOG(g_isInited == true, SOFTBUS_DISCOVER_MANAGER_NOT_INIT, "manager is not inited");

    return InnerUnPublishService(TransferModuleIdToPackageName(moduleId), publishId, PUBLISH_INNER_SERVICE);
}

int32_t DiscStartAdvertise(DiscModule moduleId, const SubscribeInfo *info)
{
    DISC_CHECK_AND_RETURN_RET_LOG(moduleId >= MODULE_MIN && moduleId <= MODULE_MAX && info != NULL,
                                  SOFTBUS_INVALID_PARAM, "invalid parameters");
    DISC_CHECK_AND_RETURN_RET_LOG(info->mode == DISCOVER_MODE_ACTIVE, SOFTBUS_INVALID_PARAM, "mode is not active");
    DISC_CHECK_AND_RETURN_RET_LOG(CheckSubscribeInfo(info) == SOFTBUS_OK, SOFTBUS_INVALID_PARAM, "invalid info");
    DISC_CHECK_AND_RETURN_RET_LOG(g_isInited == true, SOFTBUS_DISCOVER_MANAGER_NOT_INIT, "manager is not inited");

    DiscInfo *infoNode = CreateDiscInfoForSubscribe(info);
    DISC_CHECK_AND_RETURN_RET_LOG(infoNode != NULL, SOFTBUS_DISCOVER_MANAGER_INFO_NOT_CREATE, "create info failed");

    int32_t ret = InnerStartDiscovery(TransferModuleIdToPackageName(moduleId), infoNode, NULL, SUBSCRIBE_INNER_SERVICE);
    if (ret != SOFTBUS_OK) {
        FreeDiscInfo(infoNode, SUBSCRIBE_INNER_SERVICE);
    }
    return ret;
}

int32_t DiscSubscribe(DiscModule moduleId, const SubscribeInfo *info)
{
    DISC_CHECK_AND_RETURN_RET_LOG(moduleId >= MODULE_MIN && moduleId <= MODULE_MAX && info != NULL,
                                  SOFTBUS_INVALID_PARAM, "invalid parameters");
    DISC_CHECK_AND_RETURN_RET_LOG(info->mode == DISCOVER_MODE_PASSIVE, SOFTBUS_INVALID_PARAM, "mode is not passive");
    DISC_CHECK_AND_RETURN_RET_LOG(CheckSubscribeInfo(info) == SOFTBUS_OK, SOFTBUS_INVALID_PARAM, "invalid info");
    DISC_CHECK_AND_RETURN_RET_LOG(g_isInited == true, SOFTBUS_DISCOVER_MANAGER_NOT_INIT, "manager is not inited");

    DiscInfo *infoNode = CreateDiscInfoForSubscribe(info);
    DISC_CHECK_AND_RETURN_RET_LOG(infoNode != NULL, SOFTBUS_DISCOVER_MANAGER_INFO_NOT_CREATE, "create info failed");

    int32_t ret = InnerStartDiscovery(TransferModuleIdToPackageName(moduleId), infoNode, NULL, SUBSCRIBE_INNER_SERVICE);
    if (ret != SOFTBUS_OK) {
        FreeDiscInfo(infoNode, SUBSCRIBE_INNER_SERVICE);
    }
    return ret;
}

int32_t DiscStopAdvertise(DiscModule moduleId, int32_t subscribeId)
{
    DISC_CHECK_AND_RETURN_RET_LOG(moduleId >= MODULE_MIN && moduleId <= MODULE_MAX,
                                  SOFTBUS_INVALID_PARAM, "invalid moduleId");
    DISC_CHECK_AND_RETURN_RET_LOG(g_isInited == true, SOFTBUS_DISCOVER_MANAGER_NOT_INIT, "manager is not inited");

    return InnerStopDiscovery(TransferModuleIdToPackageName(moduleId), subscribeId, SUBSCRIBE_INNER_SERVICE);
}

int32_t DiscPublishService(const char *packageName, const PublishInfo *info)
{
    DISC_CHECK_AND_RETURN_RET_LOG(packageName != NULL && info != NULL, SOFTBUS_INVALID_PARAM, "invalid parameters");
    DISC_CHECK_AND_RETURN_RET_LOG(strlen(packageName) < PKG_NAME_SIZE_MAX,
                                  SOFTBUS_INVALID_PARAM, "package name too long");
    DISC_CHECK_AND_RETURN_RET_LOG(CheckPublishInfo(info) == SOFTBUS_OK, SOFTBUS_INVALID_PARAM, "invalid info");
    DISC_CHECK_AND_RETURN_RET_LOG(g_isInited == true, SOFTBUS_DISCOVER_MANAGER_NOT_INIT, "manager is not inited");

    DiscInfo *infoNode = CreateDiscInfoForPublish(info);
    DISC_CHECK_AND_RETURN_RET_LOG(infoNode != NULL, SOFTBUS_DISCOVER_MANAGER_INFO_NOT_CREATE, "create info failed");

    int32_t ret = InnerPublishService(packageName, infoNode, PUBLISH_SERVICE);
    if (ret != SOFTBUS_OK) {
        FreeDiscInfo(infoNode, PUBLISH_SERVICE);
    }
    return ret;
}

int32_t DiscUnPublishService(const char *packageName, int32_t publishId)
{
    DISC_CHECK_AND_RETURN_RET_LOG(packageName != NULL && strlen(packageName) < PKG_NAME_SIZE_MAX,
                                  SOFTBUS_INVALID_PARAM, "invalid parameters");
    DISC_CHECK_AND_RETURN_RET_LOG(g_isInited == true, SOFTBUS_DISCOVER_MANAGER_NOT_INIT, "manager is not inited");

    return InnerUnPublishService(packageName, publishId, PUBLISH_SERVICE);
}

int32_t DiscStartDiscovery(const char *packageName, const SubscribeInfo *info, const IServerDiscInnerCallback *cb)
{
    DISC_CHECK_AND_RETURN_RET_LOG(packageName != NULL && strlen(packageName) < PKG_NAME_SIZE_MAX,
                                  SOFTBUS_INVALID_PARAM, "invalid package name");
    DISC_CHECK_AND_RETURN_RET_LOG(info != NULL && cb != NULL, SOFTBUS_INVALID_PARAM, "invalid parameters");
    DISC_CHECK_AND_RETURN_RET_LOG(g_isInited == true, SOFTBUS_DISCOVER_MANAGER_NOT_INIT, "manager is not inited");
    DISC_CHECK_AND_RETURN_RET_LOG(CheckSubscribeInfo(info) == SOFTBUS_OK, SOFTBUS_INVALID_PARAM, "invalid info");

    DiscInfo *infoNode = CreateDiscInfoForSubscribe(info);
    DISC_CHECK_AND_RETURN_RET_LOG(infoNode != NULL, SOFTBUS_DISCOVER_MANAGER_INFO_NOT_CREATE, "create info failed");

    int32_t ret = InnerStartDiscovery(packageName, infoNode, cb, SUBSCRIBE_SERVICE);
    if (ret != SOFTBUS_OK) {
        FreeDiscInfo(infoNode, SUBSCRIBE_SERVICE);
    }
    return ret;
}

int32_t DiscStopDiscovery(const char *packageName, int32_t subscribeId)
{
    DISC_CHECK_AND_RETURN_RET_LOG(packageName != NULL && strlen(packageName) < PKG_NAME_SIZE_MAX,
                                  SOFTBUS_INVALID_PARAM, "invalid parameters");
    DISC_CHECK_AND_RETURN_RET_LOG(g_isInited == true, SOFTBUS_DISCOVER_MANAGER_NOT_INIT, "manager is not inited");

    return InnerStopDiscovery(packageName, subscribeId, SUBSCRIBE_SERVICE);
}

void DiscLinkStatusChanged(LinkStatus status, ExchangeMedium medium)
{
    if (medium == COAP) {
        if (g_discCoapInterface != NULL) {
            g_discCoapInterface->LinkStatusChanged(status);
        }
    } else {
        DLOGE("not support medium=%d", medium);
    }
}

void DiscDeviceInfoChanged(InfoTypeChanged type)
{
    DLOGI("type=%d", type);
    if (g_discBleInterface != NULL && g_discBleInterface->UpdateLocalDeviceInfo != NULL) {
        g_discBleInterface->UpdateLocalDeviceInfo(type);
    }
    if (g_discCoapInterface != NULL && g_discCoapInterface->UpdateLocalDeviceInfo != NULL) {
        g_discCoapInterface->UpdateLocalDeviceInfo(type);
    }
}

static IdContainer* CreateIdContainer(int32_t id, const char *pkgName)
{
    IdContainer *container = SoftBusCalloc(sizeof(IdContainer));
    if (container == NULL) {
        return NULL;
    }

    ListInit(&container->node);
    container->id = id;

    uint32_t nameLen = strlen(pkgName) + 1;
    container->pkgName = SoftBusCalloc(nameLen);
    if (container->pkgName == NULL) {
        SoftBusFree(container);
        return NULL;
    }

    if (strcpy_s(container->pkgName, nameLen, pkgName) != EOK) {
        SoftBusFree(container);
        return NULL;
    }

    return container;
}

static void DestroyIdContainer(IdContainer* container)
{
    SoftBusFree(container->pkgName);
    SoftBusFree(container);
}

static void CleanupPublishDiscovery(ListNode *ids, ServiceType type)
{
    IdContainer *it = NULL;
    int32_t ret = SOFTBUS_ERR;

    LIST_FOR_EACH_ENTRY(it, ids, IdContainer, node) {
        if (type == PUBLISH_SERVICE) {
            ret = DiscUnPublishService(it->pkgName, it->id);
            DLOGE("clean publish pkgName=%s id=%d ret=%d", it->pkgName, it->id, ret);
            return;
        } else if (type == SUBSCRIBE_SERVICE) {
            ret = DiscStopDiscovery(it->pkgName, it->id);
            DLOGE("clean subscribe pkgName=%s id=%d ret=%d", it->pkgName, it->id, ret);
        }
    }
}

static void RemoveDiscInfoByPackageName(SoftBusList *itemList, const ServiceType type, const char *pkgName)
{
    ListNode ids;
    ListInit(&ids);

    if (SoftBusMutexLock(&itemList->lock) != 0) {
        DLOGE("lock failed");
        return;
    }

    DiscItem *itemNode = NULL;
    IdContainer *container = NULL;
    LIST_FOR_EACH_ENTRY(itemNode, &itemList->list, DiscItem, node) {
        if (pkgName != NULL) {
            if (strcmp(itemNode->packageName, pkgName) != 0) {
                continue;
            }
        }

        DiscInfo *infoNode = NULL;
        LIST_FOR_EACH_ENTRY(infoNode, &itemNode->InfoList, DiscInfo, node) {
            container = CreateIdContainer(infoNode->id, itemNode->packageName);
            if (container == NULL) {
                DLOGE("CreateIdContainer failed");
                (void)SoftBusMutexUnlock(&itemList->lock);
                goto CLEANUP;
            }
            ListTailInsert(&ids, &container->node);
        }
    }

    (void)SoftBusMutexUnlock(&itemList->lock);
    CleanupPublishDiscovery(&ids, type);

CLEANUP:
    while (!IsListEmpty(&ids)) {
        container = LIST_ENTRY(ids.next, IdContainer, node);
        ListDelete(&container->node);
        DestroyIdContainer(container);
    }
}

static void RemoveAllDiscInfoForPublish(void)
{
    RemoveDiscInfoByPackageName(g_publishInfoList, PUBLISH_SERVICE, NULL);
    DestroySoftBusList(g_publishInfoList);
    g_publishInfoList = NULL;
}

static void RemoveAllDiscInfoForDiscovery(void)
{
    RemoveDiscInfoByPackageName(g_discoveryInfoList, SUBSCRIBE_SERVICE, NULL);
    DestroySoftBusList(g_discoveryInfoList);
    g_discoveryInfoList = NULL;
}

static void RemoveDiscInfoForPublish(const char *pkgName)
{
    RemoveDiscInfoByPackageName(g_publishInfoList, PUBLISH_SERVICE, pkgName);
}

static void RemoveDiscInfoForDiscovery(const char *pkgName)
{
    RemoveDiscInfoByPackageName(g_discoveryInfoList, SUBSCRIBE_SERVICE, pkgName);
}

void DiscMgrDeathCallback(const char *pkgName)
{
    DISC_CHECK_AND_RETURN_LOG(pkgName != NULL, "pkgName is null");
    DISC_CHECK_AND_RETURN_LOG(g_isInited == true, "disc manager is not inited");

    DLOGI("%s is dead", pkgName);
    RemoveDiscInfoForPublish(pkgName);
    RemoveDiscInfoForDiscovery(pkgName);
}

int32_t DiscMgrInit(void)
{
    DISC_CHECK_AND_RETURN_RET_LOG(g_isInited == false, SOFTBUS_OK, "already inited");

    g_discMgrMediumCb.OnDeviceFound = DiscOnDeviceFound;

    g_discCoapInterface = DiscCoapInit(&g_discMgrMediumCb);
    g_discBleInterface = DiscBleInit(&g_discMgrMediumCb);
    DISC_CHECK_AND_RETURN_RET_LOG(g_discBleInterface != NULL || g_discCoapInterface != NULL,
                                  SOFTBUS_ERR, "ble and coap both init failed");

    g_publishInfoList = CreateSoftBusList();
    DISC_CHECK_AND_RETURN_RET_LOG(g_publishInfoList != NULL, SOFTBUS_ERR, "init publish info list failed");
    g_discoveryInfoList = CreateSoftBusList();
    DISC_CHECK_AND_RETURN_RET_LOG(g_discoveryInfoList != NULL, SOFTBUS_ERR, "init discovery info list failed");

    for (int32_t i = 0; i < CAPABILITY_MAX_BITNUM; i++) {
        ListInit(&g_capabilityList[i]);
    }

    g_isInited = true;
    return SOFTBUS_OK;
}

void DiscMgrDeinit(void)
{
    DISC_CHECK_AND_RETURN_LOG(g_isInited == true, "disc manager is not inited");

    RemoveAllDiscInfoForPublish();
    RemoveAllDiscInfoForDiscovery();

    g_discCoapInterface = NULL;
    g_discBleInterface = NULL;

    DiscCoapDeinit();
    DiscBleDeinit();

    g_isInited = false;
    DLOGI("disc manager deinit success");
}