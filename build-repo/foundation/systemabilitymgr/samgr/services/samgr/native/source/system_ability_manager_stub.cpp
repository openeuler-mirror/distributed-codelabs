/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "system_ability_manager_stub.h"

#include <unistd.h>
#include <cinttypes>

// #include "accesstoken_kit.h"
#include "datetime_ex.h"
#include "errors.h"
// #include "hitrace_meter.h"
#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "sam_log.h"
#include "string_ex.h"
#include "system_ability_manager.h"
#include "tools.h"

#ifdef WITH_SELINUX
#include "service_checker.h"
#endif

namespace {
#ifdef WITH_SELINUX
    using namespace OHOS::HiviewDFX;
    OHOS::HiviewDFX::HiLogLabel label_ = { LOG_CORE, 0xD001800, "SA_SELINUX" };
    std::unique_ptr<ServiceChecker> selinuxChecker_ = std::make_unique<ServiceChecker>(false);
#endif

    bool CheckGetSAPermission(const int32_t said)
    {
#ifdef WITH_SELINUX
        int64_t begin = OHOS::GetTickCount();
        auto callingPid = OHOS::IPCSkeleton::GetCallingPid();
        auto ret = selinuxChecker_->GetServiceCheck(callingPid, std::to_string(said)) == 0;
        HiLog::Debug(label_, "[Performance] GetServiceCheck SA : %{public}d spend %{public}" PRId64 " ms",
            said, OHOS::GetTickCount() - begin);
        return  ret;
#else
        return true; // if not support selinux, not check selinux permission
#endif
    }

    bool CheckAddOrRemovePermission(const int32_t said)
    {
#ifdef WITH_SELINUX
        int64_t begin = OHOS::GetTickCount();
        auto callingPid = OHOS::IPCSkeleton::GetCallingPid();
        auto ret = selinuxChecker_->AddServiceCheck(callingPid, std::to_string(said)) == 0;
        HiLog::Debug(label_, "[Performance] AddServiceCheck SA : %{public}d spend %{public}" PRId64 " ms",
            said, OHOS::GetTickCount() - begin);
        return ret;
#else
        return true; // if not support selinux, not check selinux permission
#endif
    }

    bool CheckGetRemoteSAPermission(const int32_t said)
    {
#ifdef WITH_SELINUX
        int64_t begin = OHOS::GetTickCount();
        auto callingPid = OHOS::IPCSkeleton::GetCallingPid();
        auto ret = selinuxChecker_->GetRemoteServiceCheck(callingPid, std::to_string(said)) == 0;
        HiLog::Debug(label_, "[Performance] GetRemoteServiceCheck SA : %{public}d spend %{public}" PRId64 " ms",
            said, OHOS::GetTickCount() - begin);
        return ret;
#else
        return true; // if not support selinux, not check selinux permission
#endif
    }

    bool CheckListSAPermission()
    {
#ifdef WITH_SELINUX
        int64_t begin = OHOS::GetTickCount();
        auto callingPid = OHOS::IPCSkeleton::GetCallingPid();
        auto ret = selinuxChecker_->ListServiceCheck(callingPid) == 0;
        HiLog::Debug(label_, "[Performance] ListServiceCheck spend %{public}" PRId64 " ms",
            OHOS::GetTickCount() - begin);
        return ret;
#else
        return true; // if not support selinux, not check selinux permission
#endif
    }
}

// using namespace OHOS::Security;
namespace OHOS {
SystemAbilityManagerStub::SystemAbilityManagerStub()
{
    memberFuncMap_[GET_SYSTEM_ABILITY_TRANSACTION] =
        &SystemAbilityManagerStub::GetSystemAbilityInner;
    memberFuncMap_[CHECK_SYSTEM_ABILITY_TRANSACTION] =
        &SystemAbilityManagerStub::CheckSystemAbilityInner;
    memberFuncMap_[ADD_SYSTEM_ABILITY_TRANSACTION] =
        &SystemAbilityManagerStub::AddSystemAbilityInner;
    memberFuncMap_[REMOVE_SYSTEM_ABILITY_TRANSACTION] =
        &SystemAbilityManagerStub::RemoveSystemAbilityInner;
    memberFuncMap_[LIST_SYSTEM_ABILITY_TRANSACTION] =
        &SystemAbilityManagerStub::ListSystemAbilityInner;
    memberFuncMap_[SUBSCRIBE_SYSTEM_ABILITY_TRANSACTION] =
        &SystemAbilityManagerStub::SubsSystemAbilityInner;
    memberFuncMap_[CHECK_REMOTE_SYSTEM_ABILITY_TRANSACTION] =
        &SystemAbilityManagerStub::CheckRemtSystemAbilityInner;
    memberFuncMap_[ADD_ONDEMAND_SYSTEM_ABILITY_TRANSACTION] =
        &SystemAbilityManagerStub::AddOndemandSystemAbilityInner;
    memberFuncMap_[CHECK_SYSTEM_ABILITY_IMMEDIATELY_TRANSACTION] =
        &SystemAbilityManagerStub::CheckSystemAbilityImmeInner;
    memberFuncMap_[UNSUBSCRIBE_SYSTEM_ABILITY_TRANSACTION] =
        &SystemAbilityManagerStub::UnSubsSystemAbilityInner;
    memberFuncMap_[ADD_SYSTEM_PROCESS_TRANSACTION] =
        &SystemAbilityManagerStub::AddSystemProcessInner;
    memberFuncMap_[LOAD_SYSTEM_ABILITY_TRANSACTION] =
        &SystemAbilityManagerStub::LoadSystemAbilityInner;
    memberFuncMap_[LOAD_REMOTE_SYSTEM_ABILITY_TRANSACTION] =
        &SystemAbilityManagerStub::LoadRemoteSystemAbilityInner;
}

int32_t SystemAbilityManagerStub::OnRemoteRequest(uint32_t code,
    MessageParcel& data, MessageParcel& reply, MessageOption &option)
{
    HILOGI("SystemAbilityManagerStub::OnReceived, code = %{public}u, callerPid = %{public}d, flags= %{public}d",
        code, IPCSkeleton::GetCallingPid(), option.GetFlags());
    if (!EnforceInterceToken(data)) {
        HILOGE("SystemAbilityManagerStub::OnReceived, code = %{public}u, check interfaceToken failed", code);
        return ERR_PERMISSION_DENIED;
    }
    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data, reply);
        }
    }
    HILOGW("SystemAbilityManagerStub: default case, need check.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

bool SystemAbilityManagerStub::EnforceInterceToken(MessageParcel& data)
{
    std::u16string interfaceToken = data.ReadInterfaceToken();
    return interfaceToken == SAMANAGER_INTERFACE_TOKEN;
}

int32_t SystemAbilityManagerStub::ListSystemAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CanRequest()) {
        HILOGE("ListSystemAbilityInner PERMISSION DENIED!");
        return ERR_PERMISSION_DENIED;
    }

    if (!CheckListSAPermission()) {
        HILOGE("ListSystemAbilityInner selinux permission denied!");
        return ERR_PERMISSION_DENIED;
    }

    int32_t dumpFlag = 0;
    bool ret = data.ReadInt32(dumpFlag);
    if (!ret) {
        HILOGW("SystemAbilityManagerStub::ListSystemAbilityInner read dumpflag failed!");
        return ERR_FLATTEN_OBJECT;
    }

    std::vector<std::u16string> saNameVector = ListSystemAbilities(dumpFlag);
    if (saNameVector.empty()) {
        HILOGI("List System Abilities list errors");
        ret = reply.WriteInt32(ERR_INVALID_VALUE);
    } else {
        HILOGI("SystemAbilityManagerStub::ListSystemAbilityInner list success");
        ret = reply.WriteInt32(ERR_NONE);
        if (ret) {
            ret = reply.WriteString16Vector(saNameVector);
        }
    }

    if (!ret) {
        HILOGW("SystemAbilityManagerStub::ListSystemAbilityInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t SystemAbilityManagerStub::SubsSystemAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CanRequest()) {
        HILOGE("SubsSystemAbilityInner PERMISSION DENIED!");
        return ERR_PERMISSION_DENIED;
    }
    int32_t systemAbilityId = data.ReadInt32();
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("SystemAbilityManagerStub::SubsSystemAbilityInner read systemAbilityId failed!");
        return ERR_NULL_OBJECT;
    }
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    if (remoteObject == nullptr) {
        HILOGW("SystemAbilityManagerStub::SubsSystemAbilityInner read listener failed!");
        return ERR_NULL_OBJECT;
    }
    sptr<ISystemAbilityStatusChange> listener = iface_cast<ISystemAbilityStatusChange>(remoteObject);
    if (listener == nullptr) {
        HILOGW("SystemAbilityManagerStub::SubsSystemAbilityInner iface_cast failed!");
        return ERR_NULL_OBJECT;
    }
    int32_t result = SubscribeSystemAbility(systemAbilityId, listener);
    HILOGD("SystemAbilityManagerStub::SubsSystemAbilityInner result is %d", result);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGW("SystemAbilityManagerStub::SubsSystemAbilityInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }

    return result;
}

int32_t SystemAbilityManagerStub::UnSubsSystemAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CanRequest()) {
        HILOGE("UnSubsSystemAbilityInner PERMISSION DENIED!");
        return ERR_PERMISSION_DENIED;
    }
    int32_t systemAbilityId = data.ReadInt32();
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("SystemAbilityManagerStub::UnSubsSystemAbilityInner read systemAbilityId failed!");
        return ERR_NULL_OBJECT;
    }
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    if (remoteObject == nullptr) {
        HILOGW("SystemAbilityManagerStub::UnSubscribeSystemAbility read listener failed!");
        return ERR_NULL_OBJECT;
    }
    sptr<ISystemAbilityStatusChange> listener = iface_cast<ISystemAbilityStatusChange>(remoteObject);
    if (listener == nullptr) {
        HILOGW("SystemAbilityManagerStub::UnSubscribeSystemAbility iface_cast failed!");
        return ERR_NULL_OBJECT;
    }
    int32_t result = UnSubscribeSystemAbility(systemAbilityId, listener);
    HILOGD("SystemAbilityManagerStub::UnSubscribeSystemAbility result is %d", result);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGW("SystemAbilityManagerStub::UnSubscribeSystemAbility write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }

    return result;
}

int32_t SystemAbilityManagerStub::CheckRemtSystemAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CanRequest()) {
        HILOGE("CheckRemoteSystemAbilityInner PERMISSION DENIED!");
        return ERR_PERMISSION_DENIED;
    }
    int32_t systemAbilityId = data.ReadInt32();
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("SystemAbilityManagerStub::CheckRemtSystemAbilityInner read systemAbilityId failed!");
        return ERR_NULL_OBJECT;
    }

    if (!CheckGetRemoteSAPermission(systemAbilityId)) {
        HILOGE("CheckRemtSystemAbilityInner selinux permission denied!, SA : %{public}d", systemAbilityId);
        return ERR_PERMISSION_DENIED;
    }

    std::string deviceId;
    bool ret = data.ReadString(deviceId);
    if (!ret) {
        HILOGW("SystemAbilityManagerStub::CheckRemtSystemAbilityInner read deviceId failed!");
        return ERR_FLATTEN_OBJECT;
    }
    std::string uuid = SystemAbilityManager::GetInstance()->TransformDeviceId(deviceId, UUID, false);
    ret = reply.WriteRemoteObject(GetSystemAbility(systemAbilityId, uuid));
    if (!ret) {
        HILOGW("SystemAbilityManagerStub::CheckRemtSystemAbilityInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t SystemAbilityManagerStub::AddOndemandSystemAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CanRequest()) {
        HILOGE("AddOndemandSystemAbilityInner PERMISSION DENIED!");
        return ERR_PERMISSION_DENIED;
    }
    int32_t systemAbilityId = data.ReadInt32();
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("SystemAbilityManagerStub::AddOndemandSystemAbilityInner read systemAbilityId failed!");
        return ERR_NULL_OBJECT;
    }

    if (!CheckAddOrRemovePermission(systemAbilityId)) {
        HILOGE("AddOndemandSystemAbilityInner selinux permission denied! SA : %{public}d", systemAbilityId);
        return ERR_PERMISSION_DENIED;
    }

    std::u16string localManagerName = data.ReadString16();
    if (localManagerName.empty()) {
        HILOGW("SystemAbilityManagerStub::AddOndemandSystemAbilityInner read localName failed!");
        return ERR_NULL_OBJECT;
    }

    int32_t result = AddOnDemandSystemAbilityInfo(systemAbilityId, localManagerName);
    HILOGD("SystemAbilityManagerStub::AddOndemandSystemAbilityInner result is %d", result);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGW("SystemAbilityManagerStub::AddOndemandSystemAbilityInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }

    return result;
}

int32_t SystemAbilityManagerStub::CheckSystemAbilityImmeInner(MessageParcel& data, MessageParcel& reply)
{
    int32_t systemAbilityId = data.ReadInt32();
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("SystemAbilityManagerStub::CheckSystemAbilityImmeInner read systemAbilityId failed!");
        return ERR_NULL_OBJECT;
    }

    if (!CheckGetSAPermission(systemAbilityId)) {
        HILOGE("CheckSystemAbilityImmeInner selinux permission denied! SA : %{public}d", systemAbilityId);
        return ERR_PERMISSION_DENIED;
    }

    bool isExist = false;
    bool ret = data.ReadBool(isExist);
    if (!ret) {
        HILOGW("SystemAbilityManagerStub::CheckSystemAbilityImmeInner read isExist failed!");
        return ERR_FLATTEN_OBJECT;
    }
    ret = reply.WriteRemoteObject(CheckSystemAbility(systemAbilityId, isExist));
    if (!ret) {
        return ERR_FLATTEN_OBJECT;
    }

    ret = reply.WriteBool(isExist);
    if (!ret) {
        HILOGW("SystemAbilityManagerStub::CheckSystemAbilityImmeInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t SystemAbilityManagerStub::UnmarshalingSaExtraProp(MessageParcel& data, SAExtraProp& extraProp)
{
    bool isDistributed = false;
    bool ret = data.ReadBool(isDistributed);
    if (!ret) {
        HILOGW("SystemAbilityManagerStub::UnmarshalingSaExtraProp read isDistributed failed!");
        return ERR_FLATTEN_OBJECT;
    }

    int32_t dumpFlags = 0;
    ret = data.ReadInt32(dumpFlags);
    if (!ret || dumpFlags < 0) {
        HILOGW("SystemAbilityManagerStub::UnmarshalingSaExtraProp dumpFlags failed!");
        return ERR_FLATTEN_OBJECT;
    }
    std::u16string capability = data.ReadString16();
    std::u16string permission = data.ReadString16();
    extraProp.isDistributed = isDistributed;
    extraProp.dumpFlags = static_cast<uint32_t>(dumpFlags);
    extraProp.capability = capability;
    extraProp.permission = permission;
    return ERR_OK;
}

int32_t SystemAbilityManagerStub::AddSystemAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CanRequest()) {
        HILOGE("AddSystemAbilityInner PERMISSION DENIED!");
        return ERR_PERMISSION_DENIED;
    }
    int32_t systemAbilityId = data.ReadInt32();
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("SystemAbilityManagerStub::AddSystemAbilityExtraInner read systemAbilityId failed!");
        return ERR_NULL_OBJECT;
    }

    if (!CheckAddOrRemovePermission(systemAbilityId)) {
        HILOGE("AddSystemAbilityInner selinux permission denied! SA : %{public}d", systemAbilityId);
        return ERR_PERMISSION_DENIED;
    }

    auto object = data.ReadRemoteObject();
    if (object == nullptr) {
        HILOGW("SystemAbilityManagerStub::AddSystemAbilityExtraInner readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    SAExtraProp extraProp;
    int32_t result = UnmarshalingSaExtraProp(data, extraProp);
    if (result != ERR_OK) {
        HILOGW("SystemAbilityManagerStub::AddSystemAbilityExtraInner UnmarshalingSaExtraProp failed!");
        return result;
    }
    result = AddSystemAbility(systemAbilityId, object, extraProp);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGW("SystemAbilityManagerStub::AddSystemAbilityExtraInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

int32_t SystemAbilityManagerStub::GetSystemAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    int32_t systemAbilityId = data.ReadInt32();
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("SystemAbilityManagerStub::GetSystemAbilityInner read systemAbilityId failed!");
        return ERR_NULL_OBJECT;
    }

    if (!CheckGetSAPermission(systemAbilityId)) {
        HILOGE("GetSystemAbilityInner selinux permission denied! SA : %{public}d", systemAbilityId);
        return ERR_PERMISSION_DENIED;
    }

    bool ret = reply.WriteRemoteObject(GetSystemAbility(systemAbilityId));
    if (!ret) {
        HILOGW("SystemAbilityManagerStub:GetSystemAbilityInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_NONE;
}

int32_t SystemAbilityManagerStub::CheckSystemAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    int32_t systemAbilityId = data.ReadInt32();
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("SystemAbilityManagerStub::CheckSystemAbilityInner read systemAbilityId failed!");
        return ERR_NULL_OBJECT;
    }

    if (!CheckGetSAPermission(systemAbilityId)) {
        HILOGE("CheckSystemAbilityInner selinux permission denied! SA : %{public}d", systemAbilityId);
        return ERR_PERMISSION_DENIED;
    }

    bool ret = reply.WriteRemoteObject(CheckSystemAbility(systemAbilityId));
    if (!ret) {
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_NONE;
}

int32_t SystemAbilityManagerStub::RemoveSystemAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CanRequest()) {
        HILOGE("RemoveSystemAbilityInner PERMISSION DENIED!");
        return ERR_PERMISSION_DENIED;
    }
    int32_t systemAbilityId = data.ReadInt32();
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("SystemAbilityManagerStub::RemoveSystemAbilityInner read systemAbilityId failed!");
        return ERR_NULL_OBJECT;
    }

    if (!CheckAddOrRemovePermission(systemAbilityId)) {
        HILOGE("RemoveSystemAbilityInner selinux permission denied!SA : %{public}d", systemAbilityId);
        return ERR_PERMISSION_DENIED;
    }

    int32_t result = RemoveSystemAbility(systemAbilityId);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGW("SystemAbilityManagerStub::RemoveSystemAbilityInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}


int32_t SystemAbilityManagerStub::AddSystemProcessInner(MessageParcel& data, MessageParcel& reply)
{
    // if (!CanRequest()) {
    //     HILOGE("AddSystemProcessInner PERMISSION DENIED!");
    //     return ERR_PERMISSION_DENIED;
    // }
    std::u16string procName = data.ReadString16();
    if (procName.empty()) {
        HILOGW("SystemAbilityManagerStub::AddSystemProcessInner read process name failed!");
        return ERR_NULL_OBJECT;
    }

    sptr<IRemoteObject> procObject = data.ReadRemoteObject();
    if (procObject == nullptr) {
        HILOGW("SystemAbilityManagerStub::AddSystemProcessInner readParcelable failed!");
        return ERR_NULL_OBJECT;
    }

    int32_t result = AddSystemProcess(procName, procObject);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGW("SystemAbilityManagerStub::AddSystemProcessInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

int32_t SystemAbilityManagerStub::LoadSystemAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    int32_t systemAbilityId = data.ReadInt32();
    std::string loadSystemAbilityTag = ToString(systemAbilityId) + "_LoadSystemAbility";
    // HITRACE_METER_NAME(HITRACE_TAG_SAMGR, loadSystemAbilityTag);
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("SystemAbilityManagerStub::LoadSystemAbilityInner read systemAbilityId failed!");
        return ERR_INVALID_VALUE;
    }

    if (!CheckGetSAPermission(systemAbilityId)) {
        HILOGE("LoadSystemAbilityInner selinux permission denied!SA : %{public}d", systemAbilityId);
        return ERR_PERMISSION_DENIED;
    }

    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    if (remoteObject == nullptr) {
        HILOGW("SystemAbilityManagerStub::LoadSystemAbilityInner read callback failed!");
        return ERR_INVALID_VALUE;
    }
    sptr<ISystemAbilityLoadCallback> callback = iface_cast<ISystemAbilityLoadCallback>(remoteObject);
    if (callback == nullptr) {
        HILOGW("SystemAbilityManagerStub::LoadSystemAbilityInner iface_cast failed!");
        return ERR_INVALID_VALUE;
    }
    int32_t result = LoadSystemAbility(systemAbilityId, callback);
    HILOGD("SystemAbilityManagerStub::LoadSystemAbilityInner result is %{public}d", result);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGW("SystemAbilityManagerStub::LoadSystemAbilityInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

int32_t SystemAbilityManagerStub::LoadRemoteSystemAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    int32_t systemAbilityId = data.ReadInt32();
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("SystemAbilityManagerStub::LoadRemoteSystemAbilityInner systemAbilityId invalid");
        return ERR_INVALID_VALUE;
    }

    if (!CheckGetRemoteSAPermission(systemAbilityId)) {
        HILOGE("LoadRemoteSystemAbilityInner selinux permission denied!SA : %{public}d", systemAbilityId);
        return ERR_PERMISSION_DENIED;
    }

    std::string deviceId = data.ReadString();
    if (deviceId.empty()) {
        HILOGW("SystemAbilityManagerStub::LoadRemoteSystemAbilityInner read deviceId failed");
        return ERR_INVALID_VALUE;
    }
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    if (remoteObject == nullptr) {
        HILOGW("SystemAbilityManagerStub::LoadRemoteSystemAbilityInner read callback failed!");
        return ERR_INVALID_VALUE;
    }
    sptr<ISystemAbilityLoadCallback> callback = iface_cast<ISystemAbilityLoadCallback>(remoteObject);
    if (callback == nullptr) {
        HILOGW("SystemAbilityManagerStub::LoadRemoteSystemAbilityInner iface_cast failed!");
        return ERR_INVALID_VALUE;
    }
    int32_t result = LoadSystemAbility(systemAbilityId, deviceId, callback);
    HILOGD("SystemAbilityManagerStub::LoadRemoteSystemAbilityInner result is %{public}d", result);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGW("SystemAbilityManagerStub::LoadRemoteSystemAbilityInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

bool SystemAbilityManagerStub::CanRequest()
{
    return true;
    // auto accessTokenId = IPCSkeleton::GetCallingTokenID();
    // AccessToken::ATokenTypeEnum tokenType = AccessToken::AccessTokenKit::GetTokenTypeFlag(accessTokenId);
    // HILOGD("SystemAbilityManagerStub::CanRequest tokenId:%{private}u, tokenType:%{public}d",
    //     accessTokenId, tokenType);
    // return (tokenType == AccessToken::ATokenTypeEnum::TOKEN_NATIVE);
}
} // namespace OHOS
