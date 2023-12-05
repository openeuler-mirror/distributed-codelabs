/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "dev_slinfo_adpt.h"
#include <dlfcn.h>
#include "securec.h"
#include "dev_slinfo_list.h"
#include "dev_slinfo_log.h"
#include "dev_slinfo_mgr.h"

#define MAX_LIST_LENGTH 128

void *g_deviceSecLevelHandle = NULL;
DeviceSecEnv g_deviceSecEnv;
static struct DATASLListParams *g_callbackList = NULL;

static void DestroyDeviceSecEnv(void)
{
    if (g_deviceSecLevelHandle != NULL) {
        (void)memset_s(&g_deviceSecEnv, sizeof(g_deviceSecEnv), 0, sizeof(g_deviceSecEnv));
        dlclose(g_deviceSecLevelHandle);
        g_deviceSecLevelHandle = NULL;
    }
    if (g_callbackList != NULL) {
        ClearList(g_callbackList);
        g_callbackList = NULL;
    }
    return;
}

static int32_t DlopenSDK(void)
{
    g_deviceSecLevelHandle = dlopen("libdslm_sdk.z.so", RTLD_LAZY | RTLD_NODELETE);
    if (g_deviceSecLevelHandle == NULL) {
        DATA_SEC_LOG_ERROR("failed to load libdevicesecmgrsdktmp: %s", dlerror());
        return DEVSL_ERROR;
    }

    return DEVSL_SUCCESS;
}

static int32_t InitDeviceSecEnv(void)
{
    if (g_deviceSecLevelHandle != NULL) {
        DATA_SEC_LOG_WARN("libdevicesecmgrsdk already loaded");
        return DEVSL_SUCCESS;
    }
    int32_t ret = DlopenSDK();
    if (ret != DEVSL_SUCCESS) {
        return ret;
    }
    RequestDeviceSecurityInfoFunction requestDeviceSecurityInfo = (RequestDeviceSecurityInfoFunction)dlsym(
        g_deviceSecLevelHandle, "RequestDeviceSecurityInfo");
    if (requestDeviceSecurityInfo == NULL) {
        dlclose(g_deviceSecLevelHandle);
        g_deviceSecLevelHandle = NULL;
        DATA_SEC_LOG_ERROR("failed to find symbol: %s", dlerror());
        return DEVSL_ERROR;
    }
    FreeDeviceSecurityInfoFunction freeDeviceSecurityInfo = (FreeDeviceSecurityInfoFunction)dlsym(
        g_deviceSecLevelHandle, "FreeDeviceSecurityInfo");
    if (freeDeviceSecurityInfo == NULL) {
        dlclose(g_deviceSecLevelHandle);
        g_deviceSecLevelHandle = NULL;
        DATA_SEC_LOG_ERROR("failed to find symbol: %s", dlerror());
        return DEVSL_ERROR;
    }
    GetDeviceSecurityLevelValueFunction getDeviceSecurityLevelValue = (GetDeviceSecurityLevelValueFunction)dlsym(
        g_deviceSecLevelHandle, "GetDeviceSecurityLevelValue");
    if (getDeviceSecurityLevelValue == NULL) {
        dlclose(g_deviceSecLevelHandle);
        g_deviceSecLevelHandle = NULL;
        DATA_SEC_LOG_ERROR("failed to find symbol: %s", dlerror());
        return DEVSL_ERROR;
    }
    RequestDeviceSecurityInfoAsyncFunction requestDeviceSecurityInfoAsync =
        (RequestDeviceSecurityInfoAsyncFunction)dlsym(g_deviceSecLevelHandle, "RequestDeviceSecurityInfoAsync");
    if (requestDeviceSecurityInfoAsync == NULL) {
        dlclose(g_deviceSecLevelHandle);
        g_deviceSecLevelHandle = NULL;
        DATA_SEC_LOG_ERROR("failed to find symbol: %s", dlerror());
        return DEVSL_ERROR;
    }
    (void)memset_s(&g_deviceSecEnv, sizeof(g_deviceSecEnv), 0, sizeof(g_deviceSecEnv));
    g_deviceSecEnv.requestDeviceSecurityInfo = requestDeviceSecurityInfo;
    g_deviceSecEnv.freeDeviceSecurityInfo = freeDeviceSecurityInfo;
    g_deviceSecEnv.getDeviceSecurityLevelValue = getDeviceSecurityLevelValue;
    g_deviceSecEnv.requestDeviceSecurityInfoAsync = requestDeviceSecurityInfoAsync;
    return DEVSL_SUCCESS;
}

int32_t StartDevslEnv(void)
{
    DATA_SEC_LOG_INFO("Enter InitDeviceSecEnv...");
    int32_t ret = InitDeviceSecEnv();
    DATA_SEC_LOG_INFO("InitDeviceSecEnv done!");
    if (ret != DEVSL_SUCCESS) {
        return DEVSL_ERR_DEVICE_SEC_SDK_INIT;
    }

    if (g_callbackList == NULL) {
        ret = InitPthreadMutex();
        if (ret != DEVSL_SUCCESS) {
            return DEVSL_ERR_MUTEX_LOCK_INIT;
        }
        g_callbackList = InitList();
    }

    if (g_callbackList == NULL) {
        return DEVSL_ERR_OUT_OF_MEMORY;
    }
    return DEVSL_SUCCESS;
}

void FinishDevslEnv(void)
{
    DestroyDeviceSecEnv();
    DestroyPthreadMutex();
    return;
}

int32_t GetDeviceSecLevelByUdid(const uint8_t *udid, uint32_t udidLen, int32_t *devLevel)
{
    DATA_SEC_LOG_INFO("Enter GetDeviceSecLevelByUdid...");
    if (g_deviceSecEnv.requestDeviceSecurityInfo == NULL) {
        DATA_SEC_LOG_ERROR("GetDeviceSecLevelByUdid: requestDeviceSecurityInfo is invalid");
        return DEVSL_ERROR;
    }

    if (g_deviceSecEnv.freeDeviceSecurityInfo == NULL) {
        DATA_SEC_LOG_ERROR("GetDeviceSecLevelByUdid: freeDeviceSecurityInfo is invalid");
        return DEVSL_ERROR;
    }

    if (g_deviceSecEnv.getDeviceSecurityLevelValue == NULL) {
        DATA_SEC_LOG_ERROR("GetDeviceSecByUdid: getDeviceSecurityLevelValue is invalid");
        return DEVSL_ERROR;
    }

    int32_t ret;
    struct DeviceSecurityInfo *info = NULL;

    struct DeviceIdentify devId;
    (void)memset_s(&devId, sizeof(devId), 0, sizeof(devId));

    ret = memcpy_s(devId.identity, MAX_UDID_LENGTH, udid, udidLen);
    if (ret != EOK) {
        DATA_SEC_LOG_ERROR("GetDeviceSecLevelByUdid: udid memcpy failed, ret is %d", ret);
        return DEVSL_ERR_OUT_OF_MEMORY;
    }
    devId.length = udidLen;

    ret = g_deviceSecEnv.requestDeviceSecurityInfo(&devId, NULL, &info);
    if (ret != SUCCESS) {
        DATA_SEC_LOG_ERROR("GetDeviceSecLevelByUdid: request device Security info failed, %d", ret);
        g_deviceSecEnv.freeDeviceSecurityInfo(info);
        return ret;
    }

    ret = g_deviceSecEnv.getDeviceSecurityLevelValue(info, devLevel);
    if (ret != SUCCESS) {
        DATA_SEC_LOG_ERROR("GetDeviceSecLevelByUdid: get device Security value failed, %d", ret);
        g_deviceSecEnv.freeDeviceSecurityInfo(info);
        return ret;
    }

    g_deviceSecEnv.freeDeviceSecurityInfo(info);
    DATA_SEC_LOG_INFO("GetDeviceSecLevelByUdid done!");
    return DEVSL_SUCCESS;
}

void OnApiDeviceSecInfoCallback(const DeviceIdentify *identify, struct DeviceSecurityInfo *info)
{
    DATA_SEC_LOG_INFO("Enter OnApiDeviceSecInfoCallback...");
    if (identify == NULL) {
        DATA_SEC_LOG_INFO("OnApiDeviceSecInfoCallback: DeviceIdentify is null");
        return;
    }
    int32_t ret = DEVSL_SUCCESS;

    if (info == NULL) {
        DATA_SEC_LOG_INFO("OnApiDeviceSecInfoCallback: DeviceSecurityInfo is null");
        ret = DEVSL_ERROR;
    }
    if (g_deviceSecEnv.getDeviceSecurityLevelValue == NULL) {
        DATA_SEC_LOG_ERROR("OnApiDeviceSecInfoCallback: getDeviceSecurityLevelValue is invalid");
        ret = DEVSL_ERROR;
    }

    if (g_deviceSecEnv.freeDeviceSecurityInfo == NULL) {
        DATA_SEC_LOG_ERROR("OnApiDeviceSecInfoCallback: freeDeviceSecurityInfo is invalid");
        ret = DEVSL_ERROR;
    }

    int32_t devLevel = DEFAULT_DEV_SEC_LEVEL;
    uint32_t levelInfo = DATA_SEC_LEVEL0;

    if (ret == DEVSL_SUCCESS) {
        ret = g_deviceSecEnv.getDeviceSecurityLevelValue(info, &devLevel);
        if (ret != SUCCESS) {
            DATA_SEC_LOG_ERROR("OnApiDeviceSecInfoCallback: get device security level value, %d", ret);
        } else {
            levelInfo = GetDataSecLevelByDevSecLevel(devLevel);
        }
        g_deviceSecEnv.freeDeviceSecurityInfo(info);
    }

    DEVSLQueryParams queryParams;
    (void)memset_s(&queryParams, sizeof(queryParams), 0, sizeof(queryParams));

    if (memcpy_s(queryParams.udid, MAX_UDID_LENGTH, identify->identity, identify->length) != EOK) {
        DATA_SEC_LOG_ERROR("OnApiDeviceSecInfoCallback: udid memcpy failed");
        return;
    }
    queryParams.udidLen = identify->length;

    if (g_callbackList != NULL) {
        LookupCallback(g_callbackList, &queryParams, ret, levelInfo);
    }
    DATA_SEC_LOG_INFO("OnApiDeviceSecInfoCallback done!");
}

int32_t GetDeviceSecLevelByUdidAsync(const uint8_t *udid, uint32_t udidLen)
{
    DATA_SEC_LOG_INFO("Enter GetDeviceSecLevelByUdidAsync...");
    if (g_deviceSecEnv.requestDeviceSecurityInfoAsync == NULL) {
        DATA_SEC_LOG_ERROR("GetDeviceSecLevelByUdidAsync: requestDeviceSecurityInfoAsync is invalid");
        return DEVSL_ERROR;
    }

    int32_t ret;
    DeviceIdentify devId;
    (void)memset_s(&devId, sizeof(devId), 0, sizeof(devId));

    ret = memcpy_s(devId.identity, MAX_UDID_LENGTH, udid, udidLen);
    if (ret != EOK) {
        DATA_SEC_LOG_ERROR("GetDeviceSecLevelByUdidAsync: memcpy udid failed, ret is %d", ret);
        return DEVSL_ERR_OUT_OF_MEMORY;
    }
    devId.length = udidLen;
    ret = g_deviceSecEnv.requestDeviceSecurityInfoAsync(&devId, NULL, OnApiDeviceSecInfoCallback);
    if (ret != SUCCESS) {
        DATA_SEC_LOG_ERROR("GetDeviceSecLevelByUdidAsync: request device security Info for Async failed, %d", ret);
        return ret;
    }

    DATA_SEC_LOG_INFO("GetDeviceSecLevelByUdidAsync done!");
    return ret;
}

int32_t CompareUdid(DEVSLQueryParams *queryParamsL, DEVSLQueryParams *queryParamsR)
{
    DATA_SEC_LOG_INFO("Enter CompareUdid...");
    uint32_t i;

    if (queryParamsL->udidLen != queryParamsR->udidLen) {
        return DEVSL_ERROR;
    }
    for (i = 0; i < queryParamsL->udidLen; i++) {
        if (queryParamsL->udid[i] != queryParamsR->udid[i]) {
            return DEVSL_ERROR;
        }
    }
    DATA_SEC_LOG_INFO("CompareUdid done!");
    return DEVSL_SUCCESS;
}

uint32_t GetDataSecLevelByDevSecLevel(int32_t devLevel)
{
    int32_t i;
    int32_t n;
    struct {
        int32_t devSecLevel;
        uint32_t dataSecLevel;
    } devTypeMap[] = {
        { DEV_SEC_LEVEL1, DATA_SEC_LEVEL1 },
        { DEV_SEC_LEVEL2, DATA_SEC_LEVEL2 },
        { DEV_SEC_LEVEL3, DATA_SEC_LEVEL3 },
        { DEV_SEC_LEVEL4, DATA_SEC_LEVEL4 },
        { DEV_SEC_LEVEL5, DATA_SEC_LEVEL4 },
    };

    n = (int32_t)(sizeof(devTypeMap) / sizeof(devTypeMap[0]));
    for (i = 0; i < n; i++) {
        if (devTypeMap[i].devSecLevel == devLevel) {
            return devTypeMap[i].dataSecLevel;
        }
    }
    if (i >= n) {
        DATA_SEC_LOG_WARN("GetDataSecLevelBySecLevel, unknown device level tag: %d", devLevel);
    }
    return DATA_SEC_LEVEL0;
}

int32_t UpdateCallbackListParams(DEVSLQueryParams *queryParams, HigestSecInfoCallback *callback)
{
    DATA_SEC_LOG_INFO("Enter UpdateCallbackListParams...");
    int32_t ret;
    int32_t result = DEVSL_ERR_REQUEST_DEVICE_EXCEED_LIMIT;
    uint32_t levelInfo = DEFAULT_DEV_SEC_LEVEL;

    struct DATASLCallbackParams *newListNode =
        (struct DATASLCallbackParams*)malloc(sizeof(struct DATASLCallbackParams));
    if (newListNode == NULL) {
        return DEVSL_ERR_OUT_OF_MEMORY;
    }

    ret = memcpy_s(newListNode->queryParams.udid, MAX_UDID_LENGTH, queryParams->udid, queryParams->udidLen);
    if (ret != EOK) {
        DATA_SEC_LOG_ERROR("UpdateCallbackListParams: memcpy udid failed, ret is %d", ret);
        free(newListNode);
        return DEVSL_ERR_OUT_OF_MEMORY;
    }
    newListNode->queryParams.udidLen = queryParams->udidLen;
    newListNode->callback = callback;

    ret = GetListLength(g_callbackList);
    if (ret == MAX_LIST_LENGTH) {
        g_callbackList->next->callbackParams->callback(queryParams, result, levelInfo);
        RemoveListNode(g_callbackList, g_callbackList->next->callbackParams);
    }

    ret = PushListNode(g_callbackList, newListNode);
    DATA_SEC_LOG_INFO("UpdateCallbackListParams done!");
    return ret;
}