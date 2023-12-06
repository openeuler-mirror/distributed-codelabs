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
#include "permission_entry.h"

#include <securec.h>

#include "cJSON.h"
#include "common_list.h"
#include "permission_utils.h"
#include "regex.h"
#include "softbus_adapter_file.h"
#include "softbus_adapter_mem.h"
#include "softbus_adapter_thread.h"
#include "softbus_def.h"
#include "softbus_errcode.h"
#include "softbus_json_utils.h"
#include "softbus_permission.h"
#include "softbus_utils.h"

#define ENFORCING 1

#define PERMISSION_JSON_LEN 10000
#define TEMP_STR_MAX_LEN 128

/* permission entry key */
#define SESSION_NAME_STR "SESSION_NAME"
#define REGEXP_STR "REGEXP"
#define DEVID_STR "DEVID"
#define SEC_LEVEL_STR "SEC_LEVEL"
#define APP_INFO_STR "APP_INFO"

/* app info key */
#define APP_INFO_TYPE_STR "TYPE"
#define APP_INFO_PKG_NAME_STR "PKG_NAME"
#define APP_INFO_ACTION_STR "ACTIONS"
#define APP_INFO_UID_STR "UID"

/* permission entry regexp value */
#define TRUE_STR "true"
#define FALSE_STR "false"

/* permission entry sec level value */
#define PUBLIC_STR "public"
#define PRIVATE_STR "private"

/* permission entry devid value */
#define UDID_STR "UDID"
#define UUID_STR "UUID"
#define NETWORK_ID_STR "NETWORKID"

/* app info type value */
#define SYSTEM_APP_STR "system_app"
#define NATIVE_APP_STR "native_app"
#define SELF_APP_STR "self_app"
#define NORMAL_APP_STR "normal_app"
#define GRANTED_APP_STR "granted_app"

/* app info actions value */
#define OPEN_ACTIONS_STR "open"
#define CREATE_ACTIONS_STR "create"
#define ACTIONS_SPLIT ","

#define DBINDER_SERVICE_NAME "DBinderService"
#define DBINDER_BUS_NAME_PREFIX "DBinder"
#define DBINDER_PACKAGE_NAME "DBinderBus"
#define DYNAMIC_PERMISSION_MAX_SIZE 100

typedef struct {
    const char *key;
    int32_t value;
} PeMap;

static SoftBusList *g_permissionEntryList = NULL;
static SoftBusList *g_dynamicPermissionList = NULL;
static char g_permissonJson[PERMISSION_JSON_LEN];

static PeMap g_peMap[] = {
    {SYSTEM_APP_STR, SYSTEM_APP},
    {NATIVE_APP_STR, NATIVE_APP},
    {SELF_APP_STR, SELF_APP},
    {NORMAL_APP_STR, NORMAL_APP},
    {GRANTED_APP_STR, GRANTED_APP},
    {UDID_STR, UDID},
    {UUID_STR, UUID},
    {NETWORK_ID_STR, NETWORKID},
    {PRIVATE_STR, LEVEL_PRIVATE},
    {PUBLIC_STR, LEVEL_PUBLIC},
    {TRUE_STR, 1},
    {FALSE_STR, 0},
};

static int32_t ReadConfigJson(const char* permissionFile)
{
    (void)memset_s(g_permissonJson, PERMISSION_JSON_LEN, 0, PERMISSION_JSON_LEN);
    if (SoftBusReadFullFile(permissionFile, g_permissonJson, PERMISSION_JSON_LEN - 1) != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_ERROR, "ReadConfigJson failed.");
        return SOFTBUS_FILE_ERR;
    }
    return SOFTBUS_OK;
}

static int32_t GetPeMapValue(const char *string)
{
    uint32_t mapSize = sizeof(g_peMap) / sizeof(PeMap);
    uint32_t index;
    for (index = 0; index < mapSize; index++) {
        if (strcmp(string, g_peMap[index].key) == 0) {
            return g_peMap[index].value;
        }
    }
    return UNKNOWN_VALUE;
}

static bool StrIsEmpty(const char *string)
{
    if (string == NULL || strlen(string) == 0) {
        return true;
    }
    return false;
}

static bool StrStartWith(const char *string, const char *target)
{
    if (string == NULL || target == NULL) {
        return false;
    }
    size_t stringLen = strlen(string);
    size_t targetLen = strlen(target);
    if (stringLen == 0 || targetLen == 0 || stringLen < targetLen) {
        return false;
    }
    for (size_t index = 0; index < targetLen; index++) {
        if (string[index] != target[index]) {
            return false;
        }
    }
    return true;
}

static int32_t GetAppInfoByJsonObject(cJSON *object, SoftBusAppInfo *appInfo)
{
    char mapKey[TEMP_STR_MAX_LEN];
    int32_t ret = GetStringItemByJsonObject(object, APP_INFO_PKG_NAME_STR, appInfo->pkgName, PKG_NAME_SIZE_MAX);
    if (ret != SOFTBUS_OK) {
        if (ret == SOFTBUS_INVALID_PARAM) {
            SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_ERROR, "pkgname is too long");
            return SOFTBUS_INVALID_PARAM;
        }
        SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_INFO, "appInfo has no pkgname");
    }
    if (GetJsonObjectStringItem(object, APP_INFO_TYPE_STR, mapKey, TEMP_STR_MAX_LEN)) {
        appInfo->type = GetPeMapValue(mapKey);
        if (appInfo->type == UNKNOWN_VALUE) {
            return SOFTBUS_ERR;
        }
    } else {
        return SOFTBUS_ERR;
    }
    if (GetJsonObjectStringItem(object, APP_INFO_UID_STR, mapKey, TEMP_STR_MAX_LEN)) {
        appInfo->uid = atoi(mapKey);
    }
    if (GetJsonObjectStringItem(object, APP_INFO_ACTION_STR, mapKey, TEMP_STR_MAX_LEN)) {
        char *nextToken = NULL;
        char *actionStr = strtok_s(mapKey, ACTIONS_SPLIT, &nextToken);
        while (actionStr != NULL) {
            if (strcmp(actionStr, "open") == 0) {
                appInfo->actions |= ACTION_OPEN;
            } else if (strcmp(actionStr, "create") == 0) {
                appInfo->actions |= ACTION_CREATE;
            }
            actionStr = strtok_s(NULL, ACTIONS_SPLIT, &nextToken);
        }
    }
    if (appInfo->actions == 0) {
        return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}

static SoftBusAppInfo *ProcessAppInfo(cJSON *object)
{
    if (object == NULL) {
        return NULL;
    }
    SoftBusAppInfo *appInfo = (SoftBusAppInfo *)SoftBusCalloc(sizeof(SoftBusAppInfo));
    if (appInfo == NULL) {
        return NULL;
    }
    ListInit(&appInfo->node);
    appInfo->type = UNKNOWN_VALUE;
    appInfo->uid = UNKNOWN_VALUE;
    appInfo->pid = UNKNOWN_VALUE;
    appInfo->actions = 0;

    int32_t ret = GetAppInfoByJsonObject(object, appInfo);
    if (ret != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_ERROR, "get appInfo from json failed[%d]", ret);
        SoftBusFree(appInfo);
        return NULL;
    }
    return appInfo;
}

static SoftBusPermissionEntry *ProcessPermissionEntry(cJSON *object)
{
    if (object == NULL) {
        return NULL;
    }

    SoftBusPermissionEntry *permissionEntry = (SoftBusPermissionEntry *)SoftBusCalloc(sizeof(SoftBusPermissionEntry));
    if (permissionEntry == NULL) {
        return NULL;
    }
    ListInit(&permissionEntry->node);
    ListInit(&permissionEntry->appInfo);
    permissionEntry->regexp = false;
    permissionEntry->devId = UNKNOWN_VALUE;
    permissionEntry->secLevel = UNKNOWN_VALUE;

    char mapKey[TEMP_STR_MAX_LEN];
    int appInfoSize;
    int appInfoIndex;
    if (!GetJsonObjectStringItem(object, SESSION_NAME_STR, permissionEntry->sessionName, SESSION_NAME_SIZE_MAX)) {
        goto EXIT;
    }
    if (GetJsonObjectStringItem(object, REGEXP_STR, mapKey, TEMP_STR_MAX_LEN)) {
        permissionEntry->regexp = GetPeMapValue(mapKey);
    }
    if (GetJsonObjectStringItem(object, DEVID_STR, mapKey, TEMP_STR_MAX_LEN)) {
        permissionEntry->devId = GetPeMapValue(mapKey);
    }
    if (GetJsonObjectStringItem(object, SEC_LEVEL_STR, mapKey, TEMP_STR_MAX_LEN)) {
        permissionEntry->secLevel = GetPeMapValue(mapKey);
    }
    cJSON *appInfoArray = cJSON_GetObjectItem(object, APP_INFO_STR);
    if (appInfoArray != NULL) {
        appInfoSize = cJSON_GetArraySize(appInfoArray);
        for (appInfoIndex = 0; appInfoIndex < appInfoSize; appInfoIndex++) {
            SoftBusAppInfo *appInfo = ProcessAppInfo(cJSON_GetArrayItem(appInfoArray, appInfoIndex));
            if (appInfo != NULL) {
                ListNodeInsert(&permissionEntry->appInfo, &appInfo->node);
            }
        }
    }
    return permissionEntry;

EXIT:
    SoftBusFree(permissionEntry);
    return NULL;
}

static int32_t CompareString(const char *src, const char *dest, bool regexp)
{
    if (src == NULL || dest == NULL) {
        return SOFTBUS_PERMISSION_DENIED;
    }
    if (regexp) {
        regex_t regComp;
        if (regcomp(&regComp, src, REG_EXTENDED | REG_NOSUB) != 0) {
            SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_ERROR, "regcomp failed");
            return SOFTBUS_PERMISSION_DENIED;
        }
        if (regexec(&regComp, dest, 0, NULL, 0) == 0) {
            SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_INFO, "src:%s dest:%s", src, dest);
            regfree(&regComp);
            return SOFTBUS_OK;
        }
        regfree(&regComp);
    } else {
        if (strcmp(src, dest) == 0) {
            SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_INFO, "src:%s dest:%s", src, dest);
            return SOFTBUS_OK;
        }
    }
    return SOFTBUS_PERMISSION_DENIED;
}

static int32_t GetPermType(const SoftBusAppInfo *appInfo, const SoftBusPermissionItem *pItem)
{
    if (appInfo == NULL || pItem == NULL) {
        return SOFTBUS_INVALID_PARAM;
    }
    switch (appInfo->type) {
        case NATIVE_APP:
            /* same as system app */
        case SYSTEM_APP:
            if (pItem->permType == SYSTEM_APP ||
                pItem->permType == NATIVE_APP) {
                return pItem->permType;
            }
            break;
        case GRANTED_APP:
            if (pItem->actions == ACTION_CREATE) {
                if (pItem->permType == SYSTEM_APP ||
                    pItem->permType == NATIVE_APP ||
                    pItem->permType == NORMAL_APP) {
                    return pItem->permType;
                }
            } else if (pItem->actions == ACTION_OPEN) {
                if (pItem->permType == GRANTED_APP) {
                    return appInfo->type;
                }
            }
            break;
        case NORMAL_APP:
            if (pItem->permType == SYSTEM_APP ||
                pItem->permType == NATIVE_APP ||
                pItem->permType == NORMAL_APP) {
                return pItem->permType;
            }
            break;
        case SELF_APP:
            if (pItem->permType == SELF_APP) {
                return SELF_APP;
            }
            break;
        default:
            return SOFTBUS_PERMISSION_DENIED;
    }
    return SOFTBUS_PERMISSION_DENIED;
}

static int32_t CheckPermissionAppInfo(const SoftBusPermissionEntry *pe,
    const SoftBusPermissionItem *pItem)
{
    if (pe == NULL || pItem == NULL) {
        return SOFTBUS_INVALID_PARAM;
    }
    if (pItem->actions == 0) {
        return SOFTBUS_PERMISSION_DENIED;
    }
    int32_t permType;
    SoftBusAppInfo *appInfo = NULL;
    LIST_FOR_EACH_ENTRY(appInfo, &pe->appInfo, SoftBusAppInfo, node) {
        if ((appInfo->actions & pItem->actions) != pItem->actions) {
            continue;
        }
        permType = GetPermType(appInfo, pItem);
        if (permType < 0) {
            continue;
        }
        if ((appInfo->uid >= 0) && (appInfo->uid != pItem->uid)) {
            continue;
        }
        if ((appInfo->pid >= 0) && (appInfo->pid != pItem->pid)) {
            continue;
        }
        if (!StrIsEmpty(appInfo->pkgName)) {
            if (!StrIsEmpty(pItem->pkgName) &&
                (CompareString(appInfo->pkgName, pItem->pkgName, false) != SOFTBUS_OK)) {
                continue;
            }
            if (appInfo->type == SYSTEM_APP || appInfo->type == NORMAL_APP) {
                return permType;
            }
        }
        return permType;
    }
    return SOFTBUS_PERMISSION_DENIED;
}

static bool CheckDBinder(const char *sessionName)
{
    if (StrIsEmpty(sessionName)) {
        return false;
    }
    if (strcmp(DBINDER_SERVICE_NAME, sessionName) == 0) {
        return true;
    }
    if (StrStartWith(sessionName, DBINDER_BUS_NAME_PREFIX)) {
        return true;
    }
    return false;
}

static bool HaveGrantedPermission(const char *sessionName)
{
    if (sessionName == NULL || g_dynamicPermissionList == NULL) {
        return false;
    }
    SoftBusPermissionEntry *pe = NULL;
    // The lock was acquired before being called
    LIST_FOR_EACH_ENTRY(pe, &g_dynamicPermissionList->list, SoftBusPermissionEntry, node) {
        if (CompareString(pe->sessionName, sessionName, pe->regexp) == SOFTBUS_OK) {
            return true;
        }
    }
    return false;
}

int32_t LoadPermissionJson(const char *fileName)
{
    int ret = ReadConfigJson(fileName);
    if (ret != SOFTBUS_OK) {
        return ret;
    }
    if (g_permissionEntryList == NULL) {
        g_permissionEntryList = CreateSoftBusList();
        if (g_permissionEntryList == NULL) {
            return SOFTBUS_MALLOC_ERR;
        }
    }
    cJSON *jsonArray = cJSON_Parse(g_permissonJson);
    if (jsonArray == NULL) {
        SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_ERROR, "parse %s failed.", fileName);
        return SOFTBUS_PARSE_JSON_ERR;
    }
    int itemNum = cJSON_GetArraySize(jsonArray);
    if (itemNum <= 0) {
        cJSON_Delete(jsonArray);
        return SOFTBUS_PARSE_JSON_ERR;
    }
    int index;
    SoftBusPermissionEntry *pe = NULL;
    for (index = 0; index < itemNum; index++) {
        cJSON *permissionEntryObeject = cJSON_GetArrayItem(jsonArray, index);
        pe = ProcessPermissionEntry(permissionEntryObeject);
        if (pe != NULL) {
            ListNodeInsert(&g_permissionEntryList->list, &pe->node);
            g_permissionEntryList->cnt++;
        }
    }
    cJSON_Delete(jsonArray);
    return SOFTBUS_OK;
}

void ClearAppInfo(const ListNode *appInfo)
{
    if (appInfo == NULL) {
        return;
    }
    while (!IsListEmpty(appInfo)) {
        SoftBusAppInfo *item = LIST_ENTRY(appInfo->next, SoftBusAppInfo, node);
        ListDelete(&item->node);
        SoftBusFree(item);
    }
}

void DeinitPermissionJson(void)
{
    if (g_permissionEntryList == NULL) {
        return;
    }
    SoftBusMutexLock(&g_permissionEntryList->lock);
    while (!IsListEmpty(&g_permissionEntryList->list)) {
        SoftBusPermissionEntry *item = LIST_ENTRY((&g_permissionEntryList->list)->next, SoftBusPermissionEntry, node);
        ClearAppInfo(&item->appInfo);
        ListDelete(&item->node);
        SoftBusFree(item);
    }
    SoftBusMutexUnlock(&g_permissionEntryList->lock);
    DestroySoftBusList(g_permissionEntryList);
}

SoftBusPermissionItem *CreatePermissionItem(int32_t permType, int32_t uid, int32_t pid,
    const char *pkgName, uint32_t actions)
{
    SoftBusPermissionItem *pItem = SoftBusCalloc(sizeof(SoftBusPermissionItem));
    if (pItem == NULL) {
        return NULL;
    }
    pItem->permType = permType;
    pItem->uid = uid;
    pItem->pid = pid;
    pItem->pkgName = (char *)pkgName;
    pItem->actions = actions;
    return pItem;
}

int32_t CheckPermissionEntry(const char *sessionName, const SoftBusPermissionItem *pItem)
{
    if (sessionName == NULL || pItem == NULL || g_permissionEntryList == NULL) {
        return SOFTBUS_INVALID_PARAM;
    }
    int permType;
    SoftBusPermissionEntry *pe = NULL;
    bool isDynamicPermission = CheckDBinder(sessionName);
    SoftBusList *permissionList = isDynamicPermission ? g_dynamicPermissionList : g_permissionEntryList;

    (void)SoftBusMutexLock(&permissionList->lock);
    LIST_FOR_EACH_ENTRY(pe, &permissionList->list, SoftBusPermissionEntry, node) {
        if (CompareString(pe->sessionName, sessionName, pe->regexp) == SOFTBUS_OK) {
            if (isDynamicPermission) {
                (void)SoftBusMutexUnlock(&permissionList->lock);
                return GRANTED_APP;
            }
            permType = CheckPermissionAppInfo(pe, pItem);
            if (permType < 0) {
                (void)SoftBusMutexUnlock(&permissionList->lock);
                return ENFORCING ? SOFTBUS_PERMISSION_DENIED : permType;
            }
            (void)SoftBusMutexUnlock(&permissionList->lock);
            return permType;
        }
    }
    if (pItem->permType != NORMAL_APP) {
        (void)SoftBusMutexUnlock(&permissionList->lock);
        return ENFORCING ? SOFTBUS_PERMISSION_DENIED : permType;
    }
    if (pItem->actions == ACTION_CREATE) {
        if (IsValidPkgName(pItem->uid, pItem->pkgName) != SOFTBUS_OK) {
            (void)SoftBusMutexUnlock(&permissionList->lock);
            return ENFORCING ? SOFTBUS_PERMISSION_DENIED : permType;
        }
        if (!StrStartWith(sessionName, pItem->pkgName)) {
            (void)SoftBusMutexUnlock(&permissionList->lock);
            return ENFORCING ? SOFTBUS_PERMISSION_DENIED : permType;
        }
    }
    (void)SoftBusMutexUnlock(&permissionList->lock);
    return SOFTBUS_PERMISSION_DENIED;
}

bool PermIsSecLevelPublic(const char *sessionName)
{
    if (sessionName == NULL) {
        return false;
    }
    if (CheckDBinder(sessionName)) {
        return true;
    }
    SoftBusPermissionEntry *pe = NULL;
    bool ret = false;

    if (SoftBusMutexLock(&g_permissionEntryList->lock) != 0) {
        return false;
    }
    LIST_FOR_EACH_ENTRY(pe, &g_permissionEntryList->list, SoftBusPermissionEntry, node) {
        if (CompareString(pe->sessionName, sessionName, pe->regexp) == SOFTBUS_OK) {
            if (pe->secLevel == LEVEL_PUBLIC) {
                ret = true;
            }
            break;
        }
    }
    (void)SoftBusMutexUnlock(&g_permissionEntryList->lock);
    SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_INFO, "PermIsSecLevelPublic: %s is %d", sessionName, ret);
    return ret;
}

int32_t InitDynamicPermission(void)
{
    if (g_dynamicPermissionList == NULL) {
        g_dynamicPermissionList = CreateSoftBusList();
        if (g_dynamicPermissionList == NULL) {
            SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_INFO, "dynamic permission init failed");
            return SOFTBUS_MALLOC_ERR;
        }
    }
    return SOFTBUS_OK;
}

static int32_t NewDynamicPermissionEntry(SoftBusPermissionEntry *permissionEntry, const char *sessionName,
    int32_t callingUid, int32_t callingPid)
{
    if (permissionEntry == NULL) {
        SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_ERROR, "permission entry is null");
        return SOFTBUS_INVALID_PARAM;
    }
    if (sessionName == NULL) {
        SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_ERROR, "sessionName is null");
        return SOFTBUS_INVALID_PARAM;
    }
    ListInit(&permissionEntry->node);
    ListInit(&permissionEntry->appInfo);

    size_t length = strlen(sessionName);
    if (length >= SESSION_NAME_SIZE_MAX) {
        SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_ERROR, "the length [%zd] is too long for [%s]", length, sessionName);
        return SOFTBUS_INVALID_PARAM;
    }
    if (strcpy_s(permissionEntry->sessionName, SESSION_NAME_SIZE_MAX, sessionName) != EOK) {
        SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_ERROR, "strcpy failed");
        return SOFTBUS_ERR;
    }
    permissionEntry->regexp = false;
    permissionEntry->devId = UNKNOWN_VALUE;
    permissionEntry->secLevel = LEVEL_PUBLIC;

    SoftBusAppInfo *appInfo = (SoftBusAppInfo *)SoftBusCalloc(sizeof(SoftBusAppInfo));
    if (appInfo == NULL) {
        return SOFTBUS_MALLOC_ERR;
    }
    ListInit(&appInfo->node);
    if (strcpy_s(appInfo->pkgName, PKG_NAME_SIZE_MAX, DBINDER_PACKAGE_NAME) != EOK) {
        SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_ERROR, "strcpy failed");
        SoftBusFree(appInfo);
        return SOFTBUS_ERR;
    }
    appInfo->type = GRANTED_APP;
    appInfo->actions = ACTION_CREATE | ACTION_OPEN;
    appInfo->uid = callingUid;
    appInfo->pid = callingPid;
    ListNodeInsert(&permissionEntry->appInfo, &appInfo->node);
    return SOFTBUS_OK;
}

int32_t AddDynamicPermission(int32_t callingUid, int32_t callingPid, const char *sessionName)
{
    SoftBusMutexLock(&g_dynamicPermissionList->lock);
    if (g_dynamicPermissionList->cnt >= DYNAMIC_PERMISSION_MAX_SIZE) {
        SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_ERROR, "dynamic permission reach the upper limit");
        SoftBusMutexUnlock(&g_dynamicPermissionList->lock);
        return SOFTBUS_NO_ENOUGH_DATA;
    }

    if (HaveGrantedPermission(sessionName)) {
        SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_ERROR, "dynamic permission already granted");
        SoftBusMutexUnlock(&g_dynamicPermissionList->lock);
        return SOFTBUS_OK;
    }

    SoftBusPermissionEntry *permissionEntry = (SoftBusPermissionEntry *)SoftBusCalloc(sizeof(SoftBusPermissionEntry));
    if (permissionEntry == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "AddDynamicPermission malloc failed!");
        SoftBusMutexUnlock(&g_dynamicPermissionList->lock);
        return SOFTBUS_MALLOC_ERR;
    }
    
    int32_t ret = NewDynamicPermissionEntry(permissionEntry, sessionName, callingUid, callingPid);
    if (ret != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_ERROR, "NewDynamicPermissionEntry failed %d", ret);
        SoftBusFree(permissionEntry);
        SoftBusMutexUnlock(&g_dynamicPermissionList->lock);
        return ret;
    }

    ListNodeInsert(&g_dynamicPermissionList->list, &permissionEntry->node);
    g_dynamicPermissionList->cnt++;
    SoftBusMutexUnlock(&g_dynamicPermissionList->lock);

    SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_INFO, "%s dynamic permission granted", sessionName);
    return SOFTBUS_OK;
}

int32_t DeleteDynamicPermission(const char *sessionName)
{
    SoftBusMutexLock(&g_dynamicPermissionList->lock);
    SoftBusPermissionEntry *pe = NULL;
    LIST_FOR_EACH_ENTRY(pe, &g_dynamicPermissionList->list, SoftBusPermissionEntry, node) {
        if (CompareString(pe->sessionName, sessionName, pe->regexp) == SOFTBUS_OK) {
            ClearAppInfo(&pe->appInfo);
            ListDelete(&pe->node);
            SoftBusFree(pe);
            g_dynamicPermissionList->cnt--;
            SoftBusMutexUnlock(&g_dynamicPermissionList->lock);
            SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_INFO, "%s dynamic permission deleted", sessionName);
            return SOFTBUS_OK;
        }
    }
    SoftBusMutexUnlock(&g_dynamicPermissionList->lock);
    return SOFTBUS_NOT_FIND;
}
