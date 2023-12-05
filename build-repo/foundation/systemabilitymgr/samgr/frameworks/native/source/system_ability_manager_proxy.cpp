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

#include "system_ability_manager_proxy.h"

#include <unistd.h>
#include <vector>

#include "errors.h"
#include "ipc_types.h"
#include "iremote_object.h"
#include "isystem_ability_load_callback.h"
#include "isystem_ability_status_change.h"
#include "message_option.h"
#include "message_parcel.h"
#include "refbase.h"
#include "sam_log.h"
#include "string_ex.h"

using namespace std;
namespace OHOS {
namespace {
const int32_t RETRY_TIME_OUT_NUMBER = 10;
const int32_t SLEEP_INTERVAL_TIME = 100;
const int32_t SLEEP_ONE_MILLI_SECOND_TIME = 1000;
}
sptr<IRemoteObject> SystemAbilityManagerProxy::GetSystemAbility(int32_t systemAbilityId)
{
    return GetSystemAbilityWrapper(systemAbilityId);
}

sptr<IRemoteObject> SystemAbilityManagerProxy::GetSystemAbility(int32_t systemAbilityId,
    const std::string& deviceId)
{
    return GetSystemAbilityWrapper(systemAbilityId, deviceId);
}

sptr<IRemoteObject> SystemAbilityManagerProxy::GetSystemAbilityWrapper(int32_t systemAbilityId, const string& deviceId)
{
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("GetSystemAbilityWrapper systemAbilityId invalid:%{public}d!", systemAbilityId);
        return nullptr;
    }

    bool isExist = false;
    int32_t timeout = RETRY_TIME_OUT_NUMBER;
    HILOGD("GetSystemAbilityWrapper:Waiting for sa %{public}d, ", systemAbilityId);
    do {
        sptr<IRemoteObject> svc;
        if (deviceId.empty()) {
            svc = CheckSystemAbility(systemAbilityId, isExist);
            if (!isExist) {
                HILOGW("%{public}s:sa %{public}d is not exist", __func__, systemAbilityId);
                usleep(SLEEP_ONE_MILLI_SECOND_TIME * SLEEP_INTERVAL_TIME);
                continue;
            }
        } else {
            svc = CheckSystemAbility(systemAbilityId, deviceId);
        }

        if (svc != nullptr) {
            return svc;
        }
        usleep(SLEEP_ONE_MILLI_SECOND_TIME * SLEEP_INTERVAL_TIME);
    } while (timeout--);
    HILOGE("GetSystemAbilityWrapper sa %{public}d didn't start. Returning nullptr", systemAbilityId);
    return nullptr;
}

sptr<IRemoteObject> SystemAbilityManagerProxy::CheckSystemAbilityWrapper(int32_t code, MessageParcel& data)
{
    auto remote = Remote();
    if (remote == nullptr) {
        HILOGI("GetSystemAbilityWrapper remote is nullptr !");
        return nullptr;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(code, data, reply, option);
    if (err != ERR_NONE) {
        return nullptr;
    }
    return reply.ReadRemoteObject();
}

sptr<IRemoteObject> SystemAbilityManagerProxy::CheckSystemAbility(int32_t systemAbilityId)
{
    HILOGD("%{public}s called", __func__);
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("systemAbilityId:%{public}d invalid!", systemAbilityId);
        return nullptr;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        return nullptr;
    }
    bool ret = data.WriteInt32(systemAbilityId);
    if (!ret) {
        HILOGW("CheckSystemAbility Write systemAbilityId failed!");
        return nullptr;
    }
    return CheckSystemAbilityWrapper(CHECK_SYSTEM_ABILITY_TRANSACTION, data);
}

sptr<IRemoteObject> SystemAbilityManagerProxy::CheckSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    if (!CheckInputSysAbilityId(systemAbilityId) || deviceId.empty()) {
        HILOGW("CheckSystemAbility:systemAbilityId:%{public}d or deviceId is nullptr.", systemAbilityId);
        return nullptr;
    }

    HILOGD("CheckSystemAbility: ability id is : %{public}d, deviceId is %{private}s", systemAbilityId,
        deviceId.c_str());

    auto remote = Remote();
    if (remote == nullptr) {
        HILOGE("CheckSystemAbility remote is nullptr !");
        return nullptr;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        return nullptr;
    }
    bool ret = data.WriteInt32(systemAbilityId);
    if (!ret) {
        HILOGE("CheckSystemAbility parcel write name failed");
        return nullptr;
    }
    ret = data.WriteString(deviceId);
    if (!ret) {
        HILOGE("CheckSystemAbility parcel write deviceId failed");
        return nullptr;
    }

    return CheckSystemAbilityWrapper(CHECK_REMOTE_SYSTEM_ABILITY_TRANSACTION, data);
}

sptr<IRemoteObject> SystemAbilityManagerProxy::CheckSystemAbility(int32_t systemAbilityId, bool& isExist)
{
    HILOGD("%{public}s called, ability id is %{public}d, isExist is %{public}d", __func__, systemAbilityId, isExist);
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("CheckSystemAbility:systemAbilityId:%{public}d invalid!", systemAbilityId);
        return nullptr;
    }

    auto remote = Remote();
    if (remote == nullptr) {
        HILOGE("CheckSystemAbility remote is nullptr !");
        return nullptr;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        return nullptr;
    }
    bool ret = data.WriteInt32(systemAbilityId);
    if (!ret) {
        HILOGW("CheckSystemAbility Write systemAbilityId failed!");
        return nullptr;
    }

    ret = data.WriteBool(isExist);
    if (!ret) {
        HILOGW("CheckSystemAbility Write isExist failed!");
        return nullptr;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(CHECK_SYSTEM_ABILITY_IMMEDIATELY_TRANSACTION, data, reply, option);
    if (err != ERR_NONE) {
        return nullptr;
    }
    sptr<IRemoteObject> irsp(reply.ReadRemoteObject());

    ret = reply.ReadBool(isExist);
    if (!ret) {
        HILOGW("CheckSystemAbility Read isExist failed!");
        return nullptr;
    }

    return irsp;
}

int32_t SystemAbilityManagerProxy::AddOnDemandSystemAbilityInfo(int32_t systemAbilityId,
    const std::u16string& localAbilityManagerName)
{
    HILOGD("%{public}s called, system ability name is : %{public}d ", __func__, systemAbilityId);
    if (!CheckInputSysAbilityId(systemAbilityId) || localAbilityManagerName.empty()) {
        HILOGI("AddOnDemandSystemAbilityInfo invalid params!");
        return ERR_INVALID_VALUE;
    }

    auto remote = Remote();
    if (remote == nullptr) {
        HILOGE("AddOnDemandSystemAbilityInfo remote is nullptr !");
        return ERR_INVALID_OPERATION;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    bool ret = data.WriteInt32(systemAbilityId);
    if (!ret) {
        HILOGW("AddOnDemandSystemAbilityInfo Write systemAbilityId failed!");
        return ERR_FLATTEN_OBJECT;
    }

    ret = data.WriteString16(localAbilityManagerName);
    if (!ret) {
        HILOGW("AddOnDemandSystemAbilityInfo Write localAbilityManagerName failed!");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(ADD_ONDEMAND_SYSTEM_ABILITY_TRANSACTION, data, reply, option);

    HILOGI("%{public}s:add ondemand system ability %{public}d %{public}s, return %{public}d",
        __func__, systemAbilityId, err ? "fail" : "succ", err);
    if (err != ERR_NONE) {
        return err;
    }

    int32_t result = 0;
    ret = reply.ReadInt32(result);
    if (!ret) {
        HILOGW("AddOnDemandSystemAbilityInfo Read result failed!");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

int32_t SystemAbilityManagerProxy::RemoveSystemAbilityWrapper(int32_t code, MessageParcel& data)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGI("remote is nullptr !");
        return ERR_INVALID_OPERATION;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(code, data, reply, option);
    if (err != ERR_NONE) {
        HILOGE("RemoveSystemAbility SendRequest error:%{public}d!", err);
        return err;
    }

    int32_t result = 0;
    bool ret = reply.ReadInt32(result);
    if (!ret) {
        HILOGW("RemoveSystemAbility Read result failed!");
        return ERR_FLATTEN_OBJECT;
    }

    return result;
}

int32_t SystemAbilityManagerProxy::RemoveSystemAbility(int32_t systemAbilityId)
{
    HILOGD("%{public}s called, systemabilityId : %{public}d", __func__, systemAbilityId);
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("systemAbilityId:%{public}d is invalid!", systemAbilityId);
        return ERR_INVALID_VALUE;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    bool ret = data.WriteInt32(systemAbilityId);
    if (!ret) {
        HILOGW("RemoveSystemAbility Write systemAbilityId failed!");
        return ERR_FLATTEN_OBJECT;
    }
    return RemoveSystemAbilityWrapper(REMOVE_SYSTEM_ABILITY_TRANSACTION, data);
}

std::vector<u16string> SystemAbilityManagerProxy::ListSystemAbilities(unsigned int dumpFlags)
{
    HILOGD("%{public}s called", __func__);
    std::vector<u16string> saNames;

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGI("remote is nullptr !");
        return saNames;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        HILOGW("ListSystemAbilities write token failed!");
        return saNames;
    }
    bool ret = data.WriteInt32(dumpFlags);
    if (!ret) {
        HILOGW("ListSystemAbilities write dumpFlags failed!");
        return saNames;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(LIST_SYSTEM_ABILITY_TRANSACTION, data, reply, option);
    if (err != ERR_NONE) {
        HILOGW("ListSystemAbilities transact failed!");
        return saNames;
    }
    if (reply.ReadInt32() != ERR_NONE) {
        HILOGW("ListSystemAbilities remote failed!");
        return saNames;
    }
    if (!reply.ReadString16Vector(&saNames)) {
        HILOGW("ListSystemAbilities read reply failed");
        saNames.clear();
    }
    return saNames;
}

int32_t SystemAbilityManagerProxy::SubscribeSystemAbility(int32_t systemAbilityId,
    const sptr<ISystemAbilityStatusChange>& listener)
{
    HILOGI("%{public}s called, SubscribeSystemAbility systemAbilityId:%{public}d", __func__, systemAbilityId);
    if (!CheckInputSysAbilityId(systemAbilityId) || listener == nullptr) {
        HILOGE("SubscribeSystemAbility systemAbilityId:%{public}d or listener invalid!", systemAbilityId);
        return ERR_INVALID_VALUE;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGI("remote is nullptr !");
        return ERR_INVALID_OPERATION;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    bool ret = data.WriteInt32(systemAbilityId);
    if (!ret) {
        HILOGW("SubscribeSystemAbility Write systemAbilityId failed!");
        return ERR_FLATTEN_OBJECT;
    }

    ret = data.WriteRemoteObject(listener->AsObject());
    if (!ret) {
        HILOGW("SubscribeSystemAbility Write listenerName failed!");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(SUBSCRIBE_SYSTEM_ABILITY_TRANSACTION, data, reply, option);
    if (err != ERR_NONE) {
        HILOGE("SubscribeSystemAbility SendRequest error:%{public}d!", err);
        return err;
    }
    HILOGI("SubscribeSystemAbility SendRequest succeed!");
    int32_t result = 0;
    ret = reply.ReadInt32(result);
    if (!ret) {
        HILOGW("SubscribeSystemAbility Read result failed!");
        return ERR_FLATTEN_OBJECT;
    }

    return result;
}

int32_t SystemAbilityManagerProxy::UnSubscribeSystemAbility(int32_t systemAbilityId,
    const sptr<ISystemAbilityStatusChange>& listener)
{
    HILOGI("%{public}s called, UnSubscribeSystemAbility systemAbilityId:%{public}d", __func__, systemAbilityId);
    if (!CheckInputSysAbilityId(systemAbilityId) || listener == nullptr) {
        HILOGE("UnSubscribeSystemAbility systemAbilityId:%{public}d or listener invalid!", systemAbilityId);
        return ERR_INVALID_VALUE;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGI("remote is nullptr !");
        return ERR_INVALID_OPERATION;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    bool ret = data.WriteInt32(systemAbilityId);
    if (!ret) {
        HILOGW("UnSubscribeSystemAbility Write systemAbilityId failed!");
        return ERR_FLATTEN_OBJECT;
    }

    ret = data.WriteRemoteObject(listener->AsObject());
    if (!ret) {
        HILOGW("UnSubscribeSystemAbility Write listenerSaId failed!");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(UNSUBSCRIBE_SYSTEM_ABILITY_TRANSACTION, data, reply, option);
    if (err != ERR_NONE) {
        HILOGE("UnSubscribeSystemAbility SendRequest error:%{public}d!", err);
        return err;
    }
    HILOGI("UnSubscribeSystemAbility SendRequest succeed!");
    int32_t result = 0;
    ret = reply.ReadInt32(result);
    if (!ret) {
        HILOGW("UnSubscribeSystemAbility Read result failed!");
        return ERR_FLATTEN_OBJECT;
    }

    return result;
}

int32_t SystemAbilityManagerProxy::LoadSystemAbility(int32_t systemAbilityId,
    const sptr<ISystemAbilityLoadCallback>& callback)
{
    if (!CheckInputSysAbilityId(systemAbilityId) || callback == nullptr) {
        HILOGE("LoadSystemAbility systemAbilityId:%{public}d or callback invalid!", systemAbilityId);
        return ERR_INVALID_VALUE;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("LoadSystemAbility remote is null!");
        return ERR_INVALID_OPERATION;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        HILOGW("LoadSystemAbility Write interface token failed!");
        return ERR_FLATTEN_OBJECT;
    }
    bool ret = data.WriteInt32(systemAbilityId);
    if (!ret) {
        HILOGW("LoadSystemAbility Write systemAbilityId failed!");
        return ERR_FLATTEN_OBJECT;
    }
    ret = data.WriteRemoteObject(callback->AsObject());
    if (!ret) {
        HILOGW("LoadSystemAbility Write callback failed!");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(LOAD_SYSTEM_ABILITY_TRANSACTION, data, reply, option);
    if (err != ERR_NONE) {
        HILOGE("LoadSystemAbility systemAbilityId : %{public}d invalid error:%{public}d!", systemAbilityId, err);
        return err;
    }
    HILOGI("LoadSystemAbility systemAbilityId : %{public}d, SendRequest succeed!", systemAbilityId);
    int32_t result = 0;
    ret = reply.ReadInt32(result);
    if (!ret) {
        HILOGW("LoadSystemAbility Read reply failed!");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

int32_t SystemAbilityManagerProxy::LoadSystemAbility(int32_t systemAbilityId, const std::string& deviceId,
    const sptr<ISystemAbilityLoadCallback>& callback)
{
    if (!CheckInputSysAbilityId(systemAbilityId) || deviceId.empty() || callback == nullptr) {
        HILOGE("LoadSystemAbility systemAbilityId:%{public}d ,deviceId or callback invalid!", systemAbilityId);
        return ERR_INVALID_VALUE;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("LoadSystemAbility remote is null!");
        return ERR_INVALID_OPERATION;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        HILOGW("LoadSystemAbility write interface token failed!");
        return ERR_FLATTEN_OBJECT;
    }
    bool ret = data.WriteInt32(systemAbilityId);
    if (!ret) {
        HILOGW("LoadSystemAbility write systemAbilityId failed!");
        return ERR_FLATTEN_OBJECT;
    }
    ret = data.WriteString(deviceId);
    if (!ret) {
        HILOGW("LoadSystemAbility write deviceId failed!");
        return ERR_FLATTEN_OBJECT;
    }
    ret = data.WriteRemoteObject(callback->AsObject());
    if (!ret) {
        HILOGW("LoadSystemAbility Write callback failed!");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(LOAD_REMOTE_SYSTEM_ABILITY_TRANSACTION, data, reply, option);
    if (err != ERR_NONE) {
        HILOGE("LoadSystemAbility systemAbilityId : %{public}d invalid error:%{public}d!", systemAbilityId, err);
        return err;
    }
    HILOGD("LoadSystemAbility systemAbilityId : %{public}d for remote, SendRequest succeed!", systemAbilityId);
    int32_t result = 0;
    ret = reply.ReadInt32(result);
    if (!ret) {
        HILOGW("LoadSystemAbility read reply failed for remote!");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

int32_t SystemAbilityManagerProxy::MarshalSAExtraProp(const SAExtraProp& extraProp, MessageParcel& data) const
{
    if (!data.WriteBool(extraProp.isDistributed)) {
        HILOGW("MarshalSAExtraProp Write isDistributed failed!");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(extraProp.dumpFlags)) {
        HILOGW("MarshalSAExtraProp Write dumpFlags failed!");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteString16(extraProp.capability)) {
        HILOGW("MarshalSAExtraProp Write capability failed!");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteString16(extraProp.permission)) {
        HILOGW("MarshalSAExtraProp Write defPermission failed!");
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_OK;
}

int32_t SystemAbilityManagerProxy::AddSystemAbility(int32_t systemAbilityId, const sptr<IRemoteObject>& ability,
    const SAExtraProp& extraProp)
{
    HILOGD("%{public}s called, systemAbilityId is %{public}d", __func__, systemAbilityId);
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("systemAbilityId:%{public}d invalid.", systemAbilityId);
        return ERR_INVALID_VALUE;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(systemAbilityId)) {
        HILOGW("AddSystemAbility Write saId failed!");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteRemoteObject(ability)) {
        HILOGW("AddSystemAbility Write ability failed!");
        return ERR_FLATTEN_OBJECT;
    }

    int32_t ret = MarshalSAExtraProp(extraProp, data);
    if (ret != ERR_OK) {
        HILOGW("AddSystemAbility MarshalSAExtraProp failed!");
        return ret;
    }
    return AddSystemAbilityWrapper(ADD_SYSTEM_ABILITY_TRANSACTION, data);
}

int32_t SystemAbilityManagerProxy::AddSystemAbilityWrapper(int32_t code, MessageParcel& data)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGI("remote is nullptr !");
        return ERR_INVALID_OPERATION;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(code, data, reply, option);
    if (err != ERR_NONE) {
        HILOGE("AddSystemAbility SA invalid error:%{public}d!", err);
        return err;
    }
    int32_t result = 0;
    bool ret = reply.ReadInt32(result);
    if (!ret) {
        HILOGE("AddSystemAbility read result error!");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

int32_t SystemAbilityManagerProxy::AddSystemProcess(const u16string& procName, const sptr<IRemoteObject>& procObject)
{
    HILOGD("%{public}s called, process name is %{public}s", __func__, Str16ToStr8(procName).c_str());
    if (procName.empty()) {
        HILOGI("process name is invalid!");
        return ERR_INVALID_VALUE;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteString16(procName)) {
        HILOGW("AddSystemProcess Write name failed!");
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteRemoteObject(procObject)) {
        HILOGW("AddSystemProcess Write ability failed!");
        return ERR_FLATTEN_OBJECT;
    }
    return AddSystemAbilityWrapper(ADD_SYSTEM_PROCESS_TRANSACTION, data);
}
} // namespace OHOS
