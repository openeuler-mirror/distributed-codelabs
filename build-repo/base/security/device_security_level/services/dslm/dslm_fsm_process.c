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

#include "dslm_fsm_process.h"

#include <securec.h>
#include <stdbool.h>
#include <stddef.h>

#include "device_security_defines.h"
#include "utils_datetime.h"
#include "utils_hexstring.h"
#include "utils_list.h"
#include "utils_log.h"
#include "utils_mem.h"
#include "utils_mutex.h"
#include "utils_state_machine.h"
#include "utils_timer.h"

#include "dslm_callback_info.h"
#include "dslm_core_defines.h"
#include "dslm_cred.h"
#include "dslm_device_list.h"
#include "dslm_hitrace.h"
#include "dslm_inner_process.h"
#include "dslm_msg_serialize.h"
#include "dslm_notify_node.h"

#define REQUEST_INTERVAL (24 * 60 * 60 * 1000)

typedef bool DslmInfoChecker(const DslmDeviceInfo *devInfo, const DslmNotifyListNode *node, DslmCallbackInfo *cbInfo,
    uint32_t *result);

static bool SdkTimeoutChecker(const DslmDeviceInfo *devInfo, const DslmNotifyListNode *node, DslmCallbackInfo *cbInfo,
    uint32_t *result);
static bool RequestDoneChecker(const DslmDeviceInfo *devInfo, const DslmNotifyListNode *node, DslmCallbackInfo *cbInfo,
    uint32_t *result);

static uint32_t GenerateMachineId(const DeviceIdentify *identity);
static bool CheckTimesAndSendCredRequest(DslmDeviceInfo *info, bool enforce);
static void StopSendDeviceInfoRequestTimer(DslmDeviceInfo *info);
static void ProcessSendDeviceInfoCallback(DslmDeviceInfo *info, DslmInfoChecker checker);

static void TimerProcessSendDeviceInfoRequestTimeOut(const void *context);
static void TimerProcessSdkRequestTimeout(const void *context);

static bool ProcessDeviceOnline(const StateMachine *machine, uint32_t event, const void *para);
static bool ProcessSendCredRequest(const StateMachine *machine, uint32_t event, const void *para);
static bool ProcessSdkRequest(const StateMachine *machine, uint32_t event, const void *para);
static bool ProcessSendRequestFailed(const StateMachine *machine, uint32_t event, const void *para);
static bool ProcessDeviceOffline(const StateMachine *machine, uint32_t event, const void *para);
static bool ProcessVerifyCredMessage(const StateMachine *machine, uint32_t event, const void *para);
static bool ProcessSdkTimeout(const StateMachine *machine, uint32_t event, const void *para);

static void RefreshNotifyList(DslmDeviceInfo *info);
static void RefreshHistoryList(DslmDeviceInfo *info);

static uint32_t GenerateMachineId(const DeviceIdentify *identity)
{
#define MASK_LOW 0x00ffU
#define MACHINE_ID_LENGTH 4U
#define SHIFT_LENGTH 8U
#define MASK_HIGH 0xff00U
    uint16_t machineId = 0;
    HexStringToByte((const char *)identity->identity, MACHINE_ID_LENGTH, (uint8_t *)&machineId, sizeof(machineId));
    return ((machineId & MASK_HIGH) >> SHIFT_LENGTH) | ((machineId & MASK_LOW) << SHIFT_LENGTH);
}

static void TimerProcessSendDeviceInfoRequestTimeOut(const void *context)
{
    if (context == NULL) {
        return;
    }
    // the context info will never be freed, so feel free use it.
    ScheduleDslmStateMachine((DslmDeviceInfo *)context, EVENT_TIME_OUT, NULL);
}

static void TimerProcessSdkRequestTimeout(const void *context)
{
    if (context == NULL) {
        return;
    }
    // the context info will never be freed, so feel free use it.
    ScheduleDslmStateMachine((DslmDeviceInfo *)context, EVENT_SDK_TIMEOUT, NULL);
}

static void StopSendDeviceInfoRequestTimer(DslmDeviceInfo *info)
{
    if (info->timeHandle != 0) {
        StopTimerTask(info->timeHandle);
        info->timeHandle = 0;
    }
}

static void StartSendDeviceInfoRequestTimer(DslmDeviceInfo *info)
{
    info->timeHandle = StartOnceTimerTask(SEND_MSG_TIMEOUT_LEN, TimerProcessSendDeviceInfoRequestTimeOut, info);
}

static bool CheckTimesAndSendCredRequest(DslmDeviceInfo *info, bool enforce)
{
#ifndef MAX_SEND_TIMES
#define MAX_SEND_TIMES 5
#endif

#ifndef SEND_MSG_TIMEOUT_LEN
#define SEND_MSG_TIMEOUT_LEN 40000
#endif

    if (!enforce && info->queryTimes > MAX_SEND_TIMES) {
        return false;
    }
    DslmStartProcessTraceAsync("SendCredRequest", info->machine.machineId, info->queryTimes + 1);
    CheckAndGenerateChallenge(info);
    SendDeviceInfoRequest(info);
    info->queryTimes++;
    info->lastRequestTime = GetMillisecondSinceBoot();

    StopSendDeviceInfoRequestTimer(info);
    StartSendDeviceInfoRequestTimer(info);
    return true;
}

static void ProcessSendDeviceInfoCallback(DslmDeviceInfo *info, DslmInfoChecker checker)
{
#ifndef MAX_HISTORY_CNT
#define MAX_HISTORY_CNT 30U
#endif

    if (info == NULL || checker == NULL) {
        return;
    }
    ListNode *node = NULL;
    ListNode *temp = NULL;
    SECURITY_LOG_DEBUG("ProcessSendDeviceInfoCallback for device %{public}x.", info->machine.machineId);
    FOREACH_LIST_NODE_SAFE (node, &info->notifyList, temp) {
        DslmNotifyListNode *notifyNode = LIST_ENTRY(node, DslmNotifyListNode, linkNode);
        uint32_t result;
        DslmCallbackInfo cbInfo;
        bool check = checker(info, notifyNode, &cbInfo, &result);
        if (!check) {
            continue;
        }
        SECURITY_LOG_DEBUG("ProcessSendDeviceInfoCallback result %{public}u for device %{public}x, level %{public}u.",
            result, info->machine.machineId, cbInfo.level);

        notifyNode->requestCallback(notifyNode->owner, notifyNode->cookie, result, &cbInfo);
        notifyNode->stop = GetMillisecondSinceBoot();
        notifyNode->result = result;

        RemoveListNode(node);
        DslmFinishProcessTraceAsync("SDK_GET", notifyNode->owner, notifyNode->cookie);

        AddListNodeBefore(node, &info->historyList);
    }

    RefreshNotifyList(info);
    RefreshHistoryList(info);
}

static bool CheckNeedToResend(const DslmDeviceInfo *info)
{
    if (info->credInfo.credLevel == 0) {
        return true;
    }
    if (info->lastOnlineTime < info->lastRequestTime) {
        return true;
    }
    if (info->lastOnlineTime - info->lastRequestTime > (uint64_t)REQUEST_INTERVAL) {
        return true;
    }
    return false;
}

static bool ProcessDeviceOnline(const StateMachine *machine, uint32_t event, const void *para)
{
    DslmDeviceInfo *info = STATE_MACHINE_ENTRY(machine, DslmDeviceInfo, machine);
    if (para != NULL) {
        info->deviceType = *(uint32_t *)para;
    }
    info->onlineStatus = ONLINE_STATUS_ONLINE;
    info->queryTimes = 0;
    info->lastOnlineTime = GetMillisecondSinceBoot();
    if (!CheckNeedToResend(info)) {
        SECURITY_LOG_DEBUG("last request time is last than 24 hours");
        ScheduleDslmStateMachine(info, EVENT_TO_SYNC, NULL);
        return true;
    }
    return ProcessSendCredRequest(machine, event, para);
}

static bool ProcessSendCredRequest(const StateMachine *machine, uint32_t event, const void *para)
{
    DslmDeviceInfo *info = STATE_MACHINE_ENTRY(machine, DslmDeviceInfo, machine);
    bool enforce = (para != NULL);
    return CheckTimesAndSendCredRequest(info, enforce);
}

static bool ProcessSdkRequest(const StateMachine *machine, uint32_t event, const void *para)
{
    DslmDeviceInfo *deviceInfo = STATE_MACHINE_ENTRY(machine, DslmDeviceInfo, machine);
    DslmNotifyListNode *inputNotify = (DslmNotifyListNode *)para;
    if (inputNotify == NULL) {
        return false;
    }

    DslmNotifyListNode *notify = MALLOC(sizeof(DslmNotifyListNode));
    if (notify == NULL) {
        SECURITY_LOG_ERROR("malloc failed, notifyNode is null");
        return false;
    }
    (void)memset_s(notify, sizeof(DslmNotifyListNode), 0, sizeof(DslmNotifyListNode));
    notify->owner = inputNotify->owner;
    notify->cookie = inputNotify->cookie;
    notify->requestCallback = inputNotify->requestCallback;
    notify->start = inputNotify->start;
    notify->keep = inputNotify->keep;
    if (notify->cookie == 0 || notify->requestCallback == NULL) {
        SECURITY_LOG_ERROR("ProcessSdkRequest invalid cookie or callback.");
        FREE(notify);
        notify = NULL;
        return false;
    }

    DslmStartProcessTraceAsync("SDK_GET", notify->owner, notify->cookie);
    AddListNode(&notify->linkNode, &deviceInfo->notifyList);
    RefreshNotifyList(deviceInfo);
    SECURITY_LOG_DEBUG(
        "ProcessSdkRequest, device is %{public}x, owner is %{public}u, cookie is %{public}u, keep is %{public}u",
        deviceInfo->machine.machineId, notify->owner, notify->cookie, notify->keep);
    uint32_t state = GetCurrentMachineState(deviceInfo);
    if (state == STATE_SUCCESS || state == STATE_FAILED || deviceInfo->credInfo.credLevel != 0) {
        ProcessSendDeviceInfoCallback(deviceInfo, RequestDoneChecker);
        return true;
    }

    StartOnceTimerTask(notify->keep, TimerProcessSdkRequestTimeout, deviceInfo);
    return true;
}

static bool ProcessSendRequestFailed(const StateMachine *machine, uint32_t event, const void *para)
{
#define ERR_SESSION_OPEN_FAILED 2
    DslmDeviceInfo *info = STATE_MACHINE_ENTRY(machine, DslmDeviceInfo, machine);
    if (para == NULL) {
        return false;
    }

    uint32_t reason = *(uint32_t *)para;
    info->result = reason;
    if (reason == ERR_SESSION_OPEN_FAILED) {
        info->result = ERR_MSG_OPEN_SESSION;
        StopSendDeviceInfoRequestTimer(info);
        ProcessSendDeviceInfoCallback(info, RequestDoneChecker);
        return false;
    }

    return CheckTimesAndSendCredRequest(info, false);
}

static bool ProcessDeviceOffline(const StateMachine *machine, uint32_t event, const void *para)
{
    DslmDeviceInfo *info = STATE_MACHINE_ENTRY(machine, DslmDeviceInfo, machine);
    info->onlineStatus = ONLINE_STATUS_OFFLINE;
    info->queryTimes = 0;
    info->lastOfflineTime = GetMillisecondSinceBoot();
    StopSendDeviceInfoRequestTimer(info);
    ProcessSendDeviceInfoCallback(info, RequestDoneChecker);
    return true;
}

static bool ProcessVerifyCredMessage(const StateMachine *machine, uint32_t event, const void *para)
{
    DslmDeviceInfo *deviceInfo = STATE_MACHINE_ENTRY(machine, DslmDeviceInfo, machine);
    MessageBuff *buff = (MessageBuff *)para;

    deviceInfo->lastResponseTime = GetMillisecondSinceBoot();
    deviceInfo->result = (uint32_t)VerifyDeviceInfoResponse(deviceInfo, buff);
    deviceInfo->lastVerifyTime = GetMillisecondSinceBoot();
    DslmFinishProcessTraceAsync("SendCredRequest", machine->machineId, deviceInfo->queryTimes);
    ProcessSendDeviceInfoCallback(deviceInfo, RequestDoneChecker);

    if (deviceInfo->result == SUCCESS) {
        SECURITY_LOG_INFO("ProcessVerifyCredMessage success, level is %{public}u", deviceInfo->credInfo.credLevel);
        StopSendDeviceInfoRequestTimer(deviceInfo);
        return true;
    }

    (void)CheckTimesAndSendCredRequest(deviceInfo, false);
    return false;
}

static bool ProcessSdkTimeout(const StateMachine *machine, uint32_t event, const void *para)
{
    DslmDeviceInfo *info = STATE_MACHINE_ENTRY(machine, DslmDeviceInfo, machine);
    ProcessSendDeviceInfoCallback(info, SdkTimeoutChecker);
    return true;
}

static bool SdkTimeoutChecker(const DslmDeviceInfo *devInfo, const DslmNotifyListNode *node, DslmCallbackInfo *cbInfo,
    uint32_t *result)
{
    uint64_t curr = GetMillisecondSinceBoot();
    if (node->start + node->keep > curr) {
        return false;
    }

    SECURITY_LOG_INFO("SdkTimeout, device is %{public}x, owner is %{public}u, cookie is %{public}u, keep is %{public}u",
        devInfo->machine.machineId, node->owner, node->cookie, node->keep);

    *result = ERR_TIMEOUT;
    cbInfo->level = 0;
    cbInfo->extraLen = 0;
    cbInfo->extraBuff = NULL;
    return true;
}

static bool RequestDoneChecker(const DslmDeviceInfo *devInfo, const DslmNotifyListNode *node, DslmCallbackInfo *cbInfo,
    uint32_t *result)
{
    *result = devInfo->result;
    cbInfo->level = devInfo->credInfo.credLevel;
    cbInfo->extraLen = 0;
    cbInfo->extraBuff = NULL;

    SECURITY_LOG_INFO(
        "RequestDone, device is %{public}x, owner is %{public}u, cookie is %{public}u, keep is %{public}u",
        devInfo->machine.machineId, node->owner, node->cookie, node->keep);

    return true;
}

static void RefreshNotifyList(DslmDeviceInfo *info)
{
    if (info == NULL) {
        return;
    }

    // just refresh the notify list size
    ListNode *node = NULL;
    uint32_t size = 0;
    FOREACH_LIST_NODE (node, &info->notifyList) {
        size++;
    }
    info->notifyListSize = size;

    SECURITY_LOG_INFO("device %{public}x 's notify list size update to %{public}u", info->machine.machineId,
        info->notifyListSize);
}

static void RefreshHistoryList(DslmDeviceInfo *info)
{
    if (info == NULL) {
        return;
    }

    // only hold the lasted MAX_HISTORY_CNT node
    ListNode *node = NULL;
    ListNode *temp = NULL;

    uint32_t historyCnt = 0;
    FOREACH_LIST_NODE_SAFE (node, &info->historyList, temp) {
        historyCnt++;
    }
    uint32_t delCnt = historyCnt > MAX_HISTORY_CNT ? (historyCnt - MAX_HISTORY_CNT) : 0;

    info->historyListSize = historyCnt - delCnt;

    FOREACH_LIST_NODE_SAFE (node, &info->historyList, temp) {
        if (delCnt <= 0) {
            break;
        }
        delCnt--;
        DslmNotifyListNode *notifyNode = LIST_ENTRY(node, DslmNotifyListNode, linkNode);
        RemoveListNode(node);
        FREE(notifyNode);
    }
}

void InitDslmStateMachine(DslmDeviceInfo *info)
{
    if (info == NULL) {
        return;
    }
    uint32_t machineId = GenerateMachineId(&info->identity);
    InitStateMachine(&info->machine, machineId, STATE_INIT);
    SECURITY_LOG_INFO("InitDslmStateMachine success, machineId is %{public}x", machineId);
}

void ScheduleDslmStateMachine(DslmDeviceInfo *info, uint32_t event, const void *para)
{
    if (info == NULL) {
        return;
    }

    static const StateNode stateNodes[] = {
        {STATE_INIT, EVENT_DEVICE_ONLINE, ProcessDeviceOnline, STATE_WAITING_CRED_RSP, STATE_FAILED},
        {STATE_INIT, EVENT_SDK_GET, ProcessSdkRequest, STATE_INIT, STATE_INIT},
        {STATE_WAITING_CRED_RSP, EVENT_DEVICE_ONLINE, ProcessDeviceOnline, STATE_WAITING_CRED_RSP, STATE_FAILED},
        {STATE_WAITING_CRED_RSP, EVENT_CRED_RSP, ProcessVerifyCredMessage, STATE_SUCCESS, STATE_FAILED},
        {STATE_WAITING_CRED_RSP, EVENT_MSG_SEND_FAILED, ProcessSendRequestFailed, STATE_WAITING_CRED_RSP, STATE_FAILED},
        {STATE_WAITING_CRED_RSP, EVENT_TIME_OUT, ProcessSendCredRequest, STATE_WAITING_CRED_RSP, STATE_FAILED},
        {STATE_WAITING_CRED_RSP, EVENT_DEVICE_OFFLINE, ProcessDeviceOffline, STATE_INIT, STATE_INIT},
        {STATE_WAITING_CRED_RSP, EVENT_TO_SYNC, NULL, STATE_SUCCESS, STATE_SUCCESS},
        {STATE_WAITING_CRED_RSP, EVENT_SDK_GET, ProcessSdkRequest, STATE_WAITING_CRED_RSP, STATE_WAITING_CRED_RSP},
        {STATE_WAITING_CRED_RSP, EVENT_SDK_TIMEOUT, ProcessSdkTimeout, STATE_WAITING_CRED_RSP, STATE_WAITING_CRED_RSP},
        {STATE_SUCCESS, EVENT_DEVICE_OFFLINE, ProcessDeviceOffline, STATE_INIT, STATE_INIT},
        {STATE_SUCCESS, EVENT_SDK_GET, ProcessSdkRequest, STATE_SUCCESS, STATE_SUCCESS},
        {STATE_FAILED, EVENT_DEVICE_ONLINE, ProcessDeviceOnline, STATE_WAITING_CRED_RSP, STATE_FAILED},
        {STATE_FAILED, EVENT_CRED_RSP, ProcessVerifyCredMessage, STATE_SUCCESS, STATE_FAILED},
        {STATE_FAILED, EVENT_DEVICE_OFFLINE, ProcessDeviceOffline, STATE_INIT, STATE_INIT},
        {STATE_FAILED, EVENT_CHECK, ProcessSendCredRequest, STATE_WAITING_CRED_RSP, STATE_WAITING_CRED_RSP},
        {STATE_FAILED, EVENT_SDK_GET, ProcessSdkRequest, STATE_FAILED, STATE_FAILED},
        {STATE_FAILED, EVENT_SDK_TIMEOUT, ProcessSdkTimeout, STATE_FAILED, STATE_FAILED},
    };

    static const uint32_t nodeCnt = sizeof(stateNodes) / sizeof(StateNode);
    DslmStartStateMachineTrace(info->machine.machineId, event);
    ScheduleMachine(stateNodes, nodeCnt, &info->machine, event, para);
    DslmFinishProcessTrace();
}

uint32_t GetCurrentMachineState(const DslmDeviceInfo *info)
{
    if (info == NULL) {
        return STATE_FAILED;
    }
    return info->machine.currState;
}

void LockDslmStateMachine(DslmDeviceInfo *info)
{
    LockMutex(&info->machine.mutex);
}

void UnLockDslmStateMachine(DslmDeviceInfo *info)
{
    UnlockMutex(&info->machine.mutex);
}