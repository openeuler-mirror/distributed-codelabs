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

#include "dslm_core_process.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>

#include "securec.h"

#include "device_security_defines.h"
#include "dslm_callback_info.h"
#include "dslm_core_defines.h"
#include "dslm_cred.h"
#include "dslm_credential.h"
#include "dslm_device_list.h"
#include "dslm_fsm_process.h"
#include "dslm_hievent.h"
#include "dslm_hitrace.h"
#include "dslm_messenger_wrapper.h"
#include "dslm_msg_serialize.h"
#include "dslm_msg_utils.h"
#include "dslm_notify_node.h"
#include "utils_datetime.h"
#include "utils_log.h"
#include "utils_mutex.h"
#include "utils_state_machine.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_NOTIFY_SIZE 64

static const DeviceIdentify *RefreshDeviceOnlineStatus(const DeviceIdentify *deviceId);

int32_t OnPeerMsgRequestInfoReceived(const DeviceIdentify *deviceId, const uint8_t *msg, uint32_t len)
{
    if (deviceId == NULL || msg == NULL || len == 0) {
        return ERR_INVALID_PARA;
    }

    SECURITY_LOG_DEBUG("msg is %s", (char *)msg);
    MessageBuff buff = {.length = len, .buff = (uint8_t *)msg};

    RequestObject reqObject;
    (void)memset_s(&reqObject, sizeof(RequestObject), 0, sizeof(RequestObject));

    // Parse the msg
    int32_t ret = ParseDeviceSecInfoRequest(&buff, &reqObject);
    if (ret != SUCCESS) {
        return ret;
    }

    // process
    DslmCredBuff *cred = NULL;
    ret = DefaultRequestDslmCred(deviceId, &reqObject, &cred);
    if (ret != SUCCESS) {
        return ret;
    }

    // build and send response
    MessageBuff *resBuff = NULL;
    ret = BuildDeviceSecInfoResponse(reqObject.challenge, cred, &resBuff);
    if (ret == SUCCESS) {
        SendMsgToDevice(0, deviceId, resBuff->buff, resBuff->length);
        FreeMessageBuff(resBuff);
    }
    DestroyDslmCred(cred);

    return SUCCESS;
}

int32_t OnPeerMsgResponseInfoReceived(const DeviceIdentify *deviceId, const uint8_t *msg, uint32_t len)
{
    if (deviceId == NULL || msg == NULL || len == 0) {
        return ERR_INVALID_PARA;
    }
    SECURITY_LOG_DEBUG("msg is %s", (char *)msg);

    DslmDeviceInfo *deviceInfo = GetDslmDeviceInfo(deviceId);
    if (deviceInfo == NULL) {
        SECURITY_LOG_ERROR("no existed device");
        return ERR_NOEXIST_DEVICE;
    }

    MessageBuff buff = {
        .length = len,
        .buff = (uint8_t *)msg,
    };

    ScheduleDslmStateMachine(deviceInfo, EVENT_CRED_RSP, &buff);
    ReportHiEventInfoSync(deviceInfo);
    return SUCCESS;
}

int32_t OnMsgSendResultNotifier(const DeviceIdentify *deviceId, uint64_t transNo, uint32_t result)
{
    SECURITY_LOG_INFO("msg trans is %{public}u result %{public}u", (uint32_t)transNo, result);

    if (result == SUCCESS) {
        return SUCCESS;
    }

    DslmDeviceInfo *deviceInfo = GetDslmDeviceInfo(deviceId);
    if (deviceInfo == NULL) {
        return SUCCESS;
    }

    SECURITY_LOG_INFO("current DslmDeviceInfo transNum is %{public}u", (uint32_t)deviceInfo->transNum);
    if (deviceInfo->transNum != transNo) {
        return SUCCESS;
    }

    if (deviceInfo->credInfo.credLevel != 0) {
        return SUCCESS;
    }

    ScheduleDslmStateMachine(deviceInfo, EVENT_MSG_SEND_FAILED, &result);

    return SUCCESS;
}

int32_t OnRequestDeviceSecLevelInfo(const DeviceIdentify *deviceId, const RequestOption *option, uint32_t owner,
    uint32_t cookie, RequestCallback callback)
{
    if (deviceId == NULL || option == NULL || callback == NULL) {
        SECURITY_LOG_ERROR("invalid params");
        return ERR_INVALID_PARA;
    }

    if (GetMessengerStatus() != true) {
        SECURITY_LOG_ERROR("softbus service not startup complete");
        return ERR_MSG_NOT_INIT;
    }

    const DeviceIdentify *curr = RefreshDeviceOnlineStatus(deviceId);

    DslmDeviceInfo *deviceInfo = GetDslmDeviceInfo(curr);
    if (deviceInfo == NULL) {
        SECURITY_LOG_ERROR("input device not exist");
        return ERR_NOEXIST_DEVICE;
    }

    ReportHiEventAppInvoke(deviceInfo);

    if (deviceInfo->onlineStatus != ONLINE_STATUS_ONLINE) {
        SECURITY_LOG_ERROR("input device not online");
        return ERR_NOT_ONLINE;
    }

    if (deviceInfo->notifyListSize >= MAX_NOTIFY_SIZE) {
        SECURITY_LOG_ERROR("input device's notifyList is overloaded");
        return ERR_SA_BUSY;
    }

    DslmNotifyListNode notifyNode;
    (void)memset_s(&notifyNode, sizeof(notifyNode), 0, sizeof(notifyNode));
    notifyNode.owner = owner;
    notifyNode.cookie = cookie;
    notifyNode.requestCallback = callback;
    notifyNode.start = GetMillisecondSinceBoot();
    notifyNode.keep = option->timeout * 1000; // 1000 ms per second
    ScheduleDslmStateMachine(deviceInfo, EVENT_SDK_GET, &notifyNode);
    return SUCCESS;
}

int32_t OnPeerStatusReceiver(const DeviceIdentify *deviceId, uint32_t status, uint32_t devType)
{
    DslmDeviceInfo *info = CreatOrGetDslmDeviceInfo(deviceId);
    if (info == NULL) {
        return SUCCESS;
    }

    if (info->onlineStatus == status) {
        return SUCCESS;
    }

    uint32_t event = (status == ONLINE_STATUS_ONLINE) ? EVENT_DEVICE_ONLINE : EVENT_DEVICE_OFFLINE;

    ScheduleDslmStateMachine(info, event, &devType);
    return SUCCESS;
}

bool InitSelfDeviceSecureLevel(void)
{
    uint32_t devType = 0;
    const DeviceIdentify *device = GetSelfDevice(&devType);
    if (device->length == 0) {
        SECURITY_LOG_ERROR("GetSelfDevice failed");
        ReportHiEventInitSelfFailed("GetSelfDevice failed");
        return false;
    }

    DslmDeviceInfo *info = CreatOrGetDslmDeviceInfo(device);
    if (info == NULL) {
        SECURITY_LOG_ERROR("CreatOrGetDslmDeviceInfo failed");
        ReportHiEventInitSelfFailed("CreatOrGetDslmDeviceInfo failed");
        return false;
    }

    info->deviceType = devType;
    info->onlineStatus = ONLINE_STATUS_ONLINE;
    if (info->lastOnlineTime == 0) {
        info->lastOnlineTime = GetMillisecondSinceBoot();
    }

    if (info->credInfo.credLevel > 0) {
        info->result = SUCCESS;
        return true;
    }
    int32_t ret = DefaultInitDslmCred(&info->credInfo);
    if (ret == SUCCESS && info->credInfo.credLevel > 0) {
        info->machine.currState = STATE_SUCCESS;
        info->result = SUCCESS;
        return true;
    }

    ret = OnPeerStatusReceiver(device, ONLINE_STATUS_ONLINE, devType);
    if (ret != SUCCESS) {
        SECURITY_LOG_ERROR("make self online failed");
    }
    return true;
}

bool InitDslmProcess(void)
{
    static bool isInited = false;
    static Mutex initMutex = INITED_MUTEX;

    if (GetMessengerStatus() == false) {
        return false;
    }

    if (isInited == true) {
        return true;
    }

    LockMutex(&initMutex);
    bool result = InitSelfDeviceSecureLevel();
    if (result) {
        isInited = true;
    }
    UnlockMutex(&initMutex);
    return isInited;
}

bool DeinitDslmProcess(void)
{
    return true;
}

static const DeviceIdentify *RefreshDeviceOnlineStatus(const DeviceIdentify *deviceId)
{
    uint32_t devType = 0;
    if (deviceId == NULL) {
        return NULL;
    }

    if (deviceId->identity[0] == 0) {
        SECURITY_LOG_INFO("RefreshDeviceOnlineStatus to self");
        return GetSelfDevice(&devType);
    }

    if (GetPeerDeviceOnlineStatus(deviceId, &devType)) {
        (void)OnPeerStatusReceiver(deviceId, ONLINE_STATUS_ONLINE, devType);
    }

    if (IsSameDevice(deviceId, GetSelfDevice((&devType)))) {
        (void)InitSelfDeviceSecureLevel();
    }

    return deviceId;
}

#ifdef __cplusplus
}
#endif
