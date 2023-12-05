/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "group_manager.h"

#include "common_defs.h"
#include "data_manager.h"
#include "group_operation.h"

int32_t CreateGroupImpl(int32_t osAccountId, int64_t requestId, const char *appId, const char *createParams)
{
    return IsGroupSupport() ? GetGroupImplInstance()->createGroup(osAccountId, requestId, appId, createParams)
                            : HC_ERR_NOT_SUPPORT;
}

int32_t DeleteGroupImpl(int32_t osAccountId, int64_t requestId, const char *appId, const char *disbandParams)
{
    return IsGroupSupport() ? GetGroupImplInstance()->deleteGroup(osAccountId, requestId, appId, disbandParams)
                            : HC_ERR_NOT_SUPPORT;
}

int32_t AddMemberToGroupImpl(int32_t osAccountId, int64_t requestId, const char *appId, const char *addParams)
{
    return IsGroupSupport() ? GetGroupImplInstance()->addMember(osAccountId, requestId, appId, addParams)
                            : HC_ERR_NOT_SUPPORT;
}

int32_t DeleteMemberFromGroupImpl(int32_t osAccountId, int64_t requestId, const char *appId, const char *deleteParams)
{
    return IsGroupSupport() ? GetGroupImplInstance()->deleteMember(osAccountId, requestId, appId, deleteParams)
                            : HC_ERR_NOT_SUPPORT;
}

int32_t AddMultiMembersToGroupImpl(int32_t osAccountId, const char *appId, const char *addParams)
{
    return IsGroupSupport() ? GetGroupImplInstance()->addMultiMembers(osAccountId, appId, addParams)
                            : HC_ERR_NOT_SUPPORT;
}

int32_t DelMultiMembersFromGroupImpl(int32_t osAccountId, const char *appId, const char *deleteParams)
{
    return IsGroupSupport() ? GetGroupImplInstance()->delMultiMembers(osAccountId, appId, deleteParams)
                            : HC_ERR_NOT_SUPPORT;
}

int32_t ProcessBindDataImpl(int64_t requestId, const uint8_t *data, uint32_t dataLen)
{
    return IsGroupSupport() ? GetGroupImplInstance()->processBindData(requestId, data, dataLen) : HC_ERR_NOT_SUPPORT;
}

int32_t RegListenerImpl(const char *appId, const DataChangeListener *listener)
{
    return IsGroupSupport() ? GetGroupImplInstance()->regListener(appId, listener) : HC_ERR_NOT_SUPPORT;
}

int32_t UnRegListenerImpl(const char *appId)
{
    return IsGroupSupport() ? GetGroupImplInstance()->unRegListener(appId) : HC_ERR_NOT_SUPPORT;
}

int32_t CheckAccessToGroupImpl(int32_t osAccountId, const char *appId, const char *groupId)
{
    return IsGroupSupport() ? GetGroupImplInstance()->checkAccessToGroup(osAccountId, appId, groupId)
                            : HC_ERR_NOT_SUPPORT;
}

int32_t GetGroupInfoByIdImpl(int32_t osAccountId, const char *appId, const char *groupId, char **returnGroupInfo)
{
    return IsGroupSupport() ? GetGroupImplInstance()->getAccessibleGroupInfoById(osAccountId, appId, groupId,
        returnGroupInfo) : HC_ERR_NOT_SUPPORT;
}

int32_t GetGroupInfoImpl(int32_t osAccountId, const char *appId, const char *queryParams,
    char **returnGroupVec, uint32_t *groupNum)
{
    return IsGroupSupport() ? GetGroupImplInstance()->getAccessibleGroupInfo(osAccountId, appId, queryParams,
        returnGroupVec, groupNum) : HC_ERR_NOT_SUPPORT;
}

int32_t GetJoinedGroupsImpl(int32_t osAccountId, const char *appId, int groupType,
    char **returnGroupVec, uint32_t *groupNum)
{
    return IsGroupSupport() ? GetGroupImplInstance()->getAccessibleJoinedGroups(osAccountId, appId, groupType,
        returnGroupVec, groupNum) : HC_ERR_NOT_SUPPORT;
}

int32_t GetRelatedGroupsImpl(int32_t osAccountId, const char *appId, const char *peerDeviceId,
    char **returnGroupVec, uint32_t *groupNum)
{
    return IsGroupSupport() ? GetGroupImplInstance()->getAccessibleRelatedGroups(osAccountId, appId, peerDeviceId,
        false, returnGroupVec, groupNum) : HC_ERR_NOT_SUPPORT;
}

int32_t GetDeviceInfoByIdImpl(int32_t osAccountId, const char *appId, const char *deviceId, const char *groupId,
    char **returnDeviceInfo)
{
    return IsGroupSupport() ? GetGroupImplInstance()->getAccessibleDeviceInfoById(osAccountId, appId, deviceId,
        false, groupId, returnDeviceInfo) : HC_ERR_NOT_SUPPORT;
}

int32_t GetTrustedDevicesImpl(int32_t osAccountId, const char *appId, const char *groupId,
    char **returnDevInfoVec, uint32_t *deviceNum)
{
    return IsGroupSupport() ? GetGroupImplInstance()->getAccessibleTrustedDevices(osAccountId, appId, groupId,
        returnDevInfoVec, deviceNum) : HC_ERR_NOT_SUPPORT;
}

bool IsDeviceInGroupImpl(int32_t osAccountId, const char *appId, const char *groupId, const char *deviceId)
{
    return IsGroupSupport() ? GetGroupImplInstance()->isDeviceInAccessibleGroup(osAccountId, appId, groupId,
        deviceId, false) : false;
}

int32_t GetPkInfoListImpl(int32_t osAccountId, const char *appId, const char *queryParams,
    char **returnInfoList, uint32_t *returnInfoNum)
{
    return IsGroupSupport() ? GetGroupImplInstance()->getPkInfoList(osAccountId, appId, queryParams,
        returnInfoList, returnInfoNum) : HC_ERR_NOT_SUPPORT;
}

void CancelRequestImpl(int64_t requestId, const char *appId)
{
    if (IsGroupSupport()) {
        GetGroupImplInstance()->cancelGroupRequest(requestId, appId);
    }
}

void DestroyInfoImpl(char **returnInfo)
{
    if (IsGroupSupport()) {
        GetGroupImplInstance()->destroyInfo(returnInfo);
    }
}

int32_t InitGroupManager(void)
{
    if (InitDatabase() != HC_SUCCESS) {
        return HC_ERR_SERVICE_NEED_RESTART;
    }
    return IsGroupSupport() ? InitGroupRelatedModule() : HC_SUCCESS;
}

void DestroyGroupManager(void)
{
    if (IsGroupSupport()) {
        DestroyGroupRelatedModule();
    }
    DestroyDatabase();
}