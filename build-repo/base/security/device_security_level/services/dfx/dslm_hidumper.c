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

#include "dslm_hidumper.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "securec.h"

#include "utils_datetime.h"
#include "utils_log.h"

#include "dslm_credential.h"
#include "dslm_device_list.h"
#include "dslm_fsm_process.h"
#include "dslm_notify_node.h"

#define SPLIT_LINE "------------------------------------------------------"
#define END_LINE "\n"

#define TIME_STRING_LEN 256
#define COST_STRING_LEN 64
#define NOTIFY_NODE_MAX_CNT 1024

static const char *GetTimeStringFromTimeStamp(uint64_t timeStamp)
{
    static char timeBuff[TIME_STRING_LEN] = {0};
    DateTime dateTime = {0};
    bool success = false;
    do {
        (void)memset_s(timeBuff, TIME_STRING_LEN, 0, TIME_STRING_LEN);
        if (timeStamp == 0) {
            break;
        }
        if (!GetDateTimeByMillisecondSinceBoot(timeStamp, &dateTime)) {
            SECURITY_LOG_ERROR("GetTimeStringFromTimeStamp GetDateTimeByMillisecondSinceBoot error");
            break;
        }
        int ret =
            snprintf_s(timeBuff, TIME_STRING_LEN, TIME_STRING_LEN - 1, "%04hu-%02hu-%02hu %02hu:%02hu:%02hu.%03hu",
                dateTime.year, dateTime.mon, dateTime.day, dateTime.hour, dateTime.min, dateTime.sec, dateTime.msec);
        if (ret < 0) {
            break;
        }
        success = true;
    } while (0);

    if (!success) {
        if (snprintf_s(timeBuff, TIME_STRING_LEN, TIME_STRING_LEN - 1, "-") < 0) {
            SECURITY_LOG_ERROR("GetTimeStringFromTimeStamp snprintf_s error");
        }
    }
    return timeBuff;
}

static const char *GetCostTime(const uint64_t beginTime, const uint64_t endTime)
{
    static char costBuff[COST_STRING_LEN] = {0};

    if (beginTime == 0 || endTime == 0) {
        return "";
    }

    if (endTime < beginTime) {
        return "";
    }
    uint32_t cost = (uint32_t)(endTime - beginTime);
    if (snprintf_s(costBuff, COST_STRING_LEN, COST_STRING_LEN - 1, "(cost %ums)", cost) < 0) {
        return "";
    };
    return costBuff;
}

static const char *GetMachineState(const DslmDeviceInfo *info)
{
    uint32_t state = GetCurrentMachineState(info);
    switch (state) {
        case STATE_INIT:
            return "STATE_INIT";
        case STATE_WAITING_CRED_RSP:
            return "STATE_WAITING_CRED_RSP";
        case STATE_SUCCESS:
            return "STATE_SUCCESS";
        case STATE_FAILED:
            return "STATE_FAILED";
        default:
            return "STATE_UNKOWN";
    }
}

static const char *GetCreadType(const DslmDeviceInfo *info)
{
    switch (info->credInfo.credType) {
        case CRED_TYPE_MINI:
            return "mini";
        case CRED_TYPE_SMALL:
            return "small";
        case CRED_TYPE_STANDARD:
            return "standard";
        case CRED_TYPE_LARGE:
            return "large";
        default:
            return "default";
    }
}

static int32_t GetPendingNotifyNodeCnt(const DslmDeviceInfo *info)
{
    int result = 0;
    LockDslmStateMachine((DslmDeviceInfo *)info);
    ListNode *node = NULL;
    FOREACH_LIST_NODE (node, &info->notifyList) {
        result++;
        if (result >= NOTIFY_NODE_MAX_CNT) {
            break;
        }
    }
    UnLockDslmStateMachine((DslmDeviceInfo *)info);
    return result;
}

static void GetDefaultStatus(int32_t *requestResult, int32_t *verifyResult, uint32_t *credLevel)
{
    if (requestResult == NULL || verifyResult == NULL || credLevel == NULL) {
        return;
    }
    const DeviceIdentify identify = {DEVICE_ID_MAX_LEN, {0}};
    RequestObject object;

    object.arraySize = 1;
    object.credArray[0] = CRED_TYPE_STANDARD;
    object.challenge = 0x0;
    object.version = GetCurrentVersion();

    DslmCredBuff *cred = NULL;
    *requestResult = DefaultRequestDslmCred(&identify, &object, &cred);

    DslmCredInfo info;
    (void)memset_s(&info, sizeof(DslmCredInfo), 0, sizeof(DslmCredInfo));

    *verifyResult = DefaultVerifyDslmCred(&identify, object.challenge, cred, &info);
    *credLevel = info.credLevel;
    DestroyDslmCred(cred);
}

static void PrintBanner(int fd)
{
    dprintf(fd, " ___  ___ _    __  __   ___  _   _ __  __ ___ ___ ___ " END_LINE);
    dprintf(fd, "|   \\/ __| |  |  \\/  | |   \\| | | |  \\/  | _ \\ __| _ \\" END_LINE);
    dprintf(fd, "| |) \\__ \\ |__| |\\/| | | |) | |_| | |\\/| |  _/ __|   /" END_LINE);
    dprintf(fd, "|___/|___/____|_|  |_| |___/ \\___/|_|  |_|_| |___|_|_\\" END_LINE);
}

static void DumpDeviceDetails(const DslmDeviceInfo *info, int32_t fd)
{
    dprintf(fd, "DEVICE_ID                 : %x" END_LINE, info->machine.machineId);
    dprintf(fd, "DEVICE_TYPE               : %u" END_LINE, info->deviceType);
    dprintf(fd, END_LINE);

    dprintf(fd, "DEVICE_ONLINE_STATUS      : %s" END_LINE, (info->onlineStatus != 0) ? "online" : "offline");
    dprintf(fd, "DEVICE_ONLINE_TIME        : %s" END_LINE, GetTimeStringFromTimeStamp(info->lastOnlineTime));
    dprintf(fd, "DEVICE_OFFLINE_TIME       : %s" END_LINE, GetTimeStringFromTimeStamp(info->lastOfflineTime));
    dprintf(fd, "DEVICE_REQUEST_TIME       : %s" END_LINE, GetTimeStringFromTimeStamp(info->lastRequestTime));
    dprintf(fd, "DEVICE_RESPONSE_TIME      : %s%s" END_LINE, GetTimeStringFromTimeStamp(info->lastResponseTime),
        GetCostTime(info->lastRequestTime, info->lastResponseTime));
    dprintf(fd, "DEVICE_VERIFY_TIME        : %s%s" END_LINE, GetTimeStringFromTimeStamp(info->lastVerifyTime),
        GetCostTime(info->lastResponseTime, info->lastVerifyTime));
    dprintf(fd, END_LINE);

    dprintf(fd, "DEVICE_PENDING_CNT        : %d" END_LINE, GetPendingNotifyNodeCnt(info));
    dprintf(fd, "DEVICE_MACHINE_STATUS     : %s" END_LINE, GetMachineState(info));
    dprintf(fd, "DEVICE_VERIFIED_LEVEL     : %u" END_LINE, info->credInfo.credLevel);
    dprintf(fd, "DEVICE_VERIFIED_RESULT    : %s" END_LINE, (info->result == 0) ? "success" : "failed");
    dprintf(fd, END_LINE);

    dprintf(fd, "CRED_TYPE                 : %s" END_LINE, GetCreadType(info));
    dprintf(fd, "CRED_RELEASE_TYPE         : %s" END_LINE, info->credInfo.releaseType);
    dprintf(fd, "CRED_SIGN_TIME            : %s" END_LINE, info->credInfo.signTime);
    dprintf(fd, "CRED_MANUFACTURE          : %s" END_LINE, info->credInfo.manufacture);
    dprintf(fd, "CRED_BAND                 : %s" END_LINE, info->credInfo.brand);
    dprintf(fd, "CRED_MODEL                : %s" END_LINE, info->credInfo.model);
    dprintf(fd, "CRED_SOFTWARE_VERSION     : %s" END_LINE, info->credInfo.softwareVersion);
    dprintf(fd, "CRED_SECURITY_LEVEL       : %s" END_LINE, info->credInfo.securityLevel);
    dprintf(fd, "CRED_VERSION              : %s" END_LINE, info->credInfo.version);
    dprintf(fd, END_LINE);
}

static void DumpHistoryCalls(const DslmDeviceInfo *info, int32_t fd)
{
    dprintf(fd, "SDK_CALL_HISTORY: " END_LINE);
    ListNode *node = NULL;
    int32_t index = 0;
    FOREACH_LIST_NODE (node, &info->historyList) {
        index++;
        DslmNotifyListNode *notifyNode = LIST_ENTRY(node, DslmNotifyListNode, linkNode);

        char timeStart[TIME_STRING_LEN] = {0};
        if (strcpy_s(timeStart, TIME_STRING_LEN, GetTimeStringFromTimeStamp(notifyNode->start)) != EOK) {
            continue;
        }
        char timeStop[TIME_STRING_LEN] = {0};
        if (strcpy_s(timeStop, TIME_STRING_LEN, GetTimeStringFromTimeStamp(notifyNode->stop)) != EOK) {
            continue;
        }

        uint32_t cost = (notifyNode->stop > notifyNode->start) ? (notifyNode->stop - notifyNode->start) : 0;
        dprintf(fd, "#%-4d pid:%-6u seq:%-4u req:%-26s res:%-26s ret:%-4u cost:%ums" END_LINE, index, notifyNode->owner,
            notifyNode->cookie, timeStart, timeStop, notifyNode->result, cost);

        if (index >= NOTIFY_NODE_MAX_CNT) {
            break;
        }
    }
}

static void DumpOneDevice(const DslmDeviceInfo *info, int32_t fd)
{
    if (info == NULL) {
        return;
    }

    dprintf(fd, SPLIT_LINE END_LINE);
    DumpDeviceDetails(info, fd);
    DumpHistoryCalls(info, fd);
    dprintf(fd, SPLIT_LINE END_LINE);
}

static void PrintAllDevices(int fd)
{
    ForEachDeviceDump(DumpOneDevice, fd);
}

static void PrintDefaultStatus(int fd)
{
    int32_t requestResult = 0;
    int32_t verifyResult = 0;
    uint32_t credLevel = 0;

    GetDefaultStatus(&requestResult, &verifyResult, &credLevel);

    const time_t YEAR_TIME_2022 = 1640966400;
    struct timeval timeVal = {0};
    gettimeofday(&timeVal, NULL);
    char *notice = timeVal.tv_sec <= YEAR_TIME_2022 ? "(please check the system time)" : "";

    dprintf(fd, SPLIT_LINE END_LINE);
    dprintf(fd, "REQUEST_TEST              : %s" END_LINE, requestResult == SUCCESS ? "success" : "failed");
    dprintf(fd, "VERIFY_TEST               : %s%s" END_LINE, verifyResult == SUCCESS ? "success" : "failed", notice);
    dprintf(fd, "SELF_CRED_LEVEL           : %u" END_LINE, credLevel);
    dprintf(fd, SPLIT_LINE END_LINE);
}

void DslmDumper(int fd)
{
    PrintBanner(fd);
    PrintDefaultStatus(fd);
    PrintAllDevices(fd);
}