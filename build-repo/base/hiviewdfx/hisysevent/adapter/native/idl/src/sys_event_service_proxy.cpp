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

#include "sys_event_service_proxy.h"

#include "errors.h"
#include "hilog/log.h"
#include "parcelable_vector_rw.h"
#include "query_argument.h"
#include "ret_code.h"

namespace OHOS {
namespace HiviewDFX {
static constexpr HiLogLabel LABEL = { LOG_CORE, 0xD002D08, "HiView-SysEventServiceProxy" };
int32_t SysEventServiceProxy::AddListener(const std::vector<SysEventRule>& rules,
    const sptr<ISysEventCallback>& callback)
{
    auto remote = Remote();
    if (remote == nullptr) {
        HiLog::Error(LABEL, "SysEventService Remote is NULL.");
        return ERR_REMOTE_SERVICE_IS_NULL;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(SysEventServiceProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "write descriptor failed.");
        return ERR_CAN_NOT_WRITE_DESCRIPTOR;
    }
    bool ret = WriteVectorToParcel(data, rules);
    if (!ret) {
        HiLog::Error(LABEL, "parcel write rules failed.");
        return ERR_CAN_NOT_WRITE_PARCEL;
    }
    if (callback == nullptr) {
        return ERR_PARCEL_DATA_IS_NULL;
    }
    ret = data.WriteRemoteObject(callback->AsObject());
    if (!ret) {
        HiLog::Error(LABEL, "parcel write callback failed.");
        return ERR_CAN_NOT_WRITE_REMOTE_OBJECT;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t res = remote->SendRequest(ADD_SYS_EVENT_LISTENER, data, reply, option);
    if (res != ERR_OK) {
        HiLog::Error(LABEL, "send request failed, error is %{public}d.", res);
        return ERR_CAN_NOT_SEND_REQ;
    }
    int32_t result;
    ret = reply.ReadInt32(result);
    if (!ret) {
        HiLog::Error(LABEL, "parcel read result failed.");
        return ERR_CAN_NOT_READ_PARCEL;
    }
    return result;
}

int32_t SysEventServiceProxy::RemoveListener(const sptr<ISysEventCallback> &callback)
{
    auto remote = Remote();
    if (remote == nullptr) {
        HiLog::Error(LABEL, "SysEventService Remote is null.");
        return ERR_REMOTE_SERVICE_IS_NULL;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(SysEventServiceProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "write descriptor failed.");
        return ERR_CAN_NOT_WRITE_DESCRIPTOR;
    }
    if (callback == nullptr) {
        return ERR_PARCEL_DATA_IS_NULL;
    }
    bool ret = data.WriteRemoteObject(callback->AsObject());
    if (!ret) {
        HiLog::Error(LABEL, "parcel write object in callback failed.");
        return ERR_CAN_NOT_WRITE_REMOTE_OBJECT;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t res = remote->SendRequest(REMOVE_SYS_EVENT_LISTENER, data, reply, option);
    if (res != ERR_OK) {
        HiLog::Error(LABEL, "send request failed, error is %{public}d.", res);
        return ERR_CAN_NOT_SEND_REQ;
    }
    int32_t result;
    ret = reply.ReadInt32(result);
    if (!ret) {
        HiLog::Error(LABEL, "parcel read result failed.");
        return ERR_CAN_NOT_READ_PARCEL;
    }
    return result;
}

int32_t SysEventServiceProxy::Query(const QueryArgument& queryArgument, const std::vector<SysEventQueryRule>& rules,
    const sptr<IQuerySysEventCallback>& callback)
{
    auto remote = Remote();
    if (remote == nullptr) {
        HiLog::Error(LABEL, "SysEventService Remote is null.");
        return ERR_REMOTE_SERVICE_IS_NULL;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(SysEventServiceProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "write descriptor failed.");
        return ERR_CAN_NOT_WRITE_DESCRIPTOR;
    }
    if (!data.WriteParcelable(&queryArgument)) {
        HiLog::Error(LABEL, "parcel write query arguments failed.");
        return ERR_CAN_NOT_WRITE_PARCEL;
    }
    bool ret = WriteVectorToParcel(data, rules);
    if (!ret) {
        HiLog::Error(LABEL, "parcel write query rules failed.");
        return ERR_CAN_NOT_WRITE_PARCEL;
    }
    if (callback == nullptr) {
        return ERR_PARCEL_DATA_IS_NULL;
    }
    ret = data.WriteRemoteObject(callback->AsObject());
    if (!ret) {
        HiLog::Error(LABEL, "parcel write callback failed.");
        return ERR_CAN_NOT_WRITE_REMOTE_OBJECT;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t res = remote->SendRequest(QUERY_SYS_EVENT, data, reply, option);
    if (res != ERR_OK) {
        HiLog::Error(LABEL, "send request failed, error is %{public}d.", res);
        return ERR_CAN_NOT_SEND_REQ;
    }
    int32_t result;
    ret = reply.ReadInt32(result);
    if (!ret) {
        HiLog::Error(LABEL, "parcel read result failed.");
        return ERR_CAN_NOT_READ_PARCEL;
    }
    return result;
}

int32_t SysEventServiceProxy::SetDebugMode(const sptr<ISysEventCallback>& callback, bool mode)
{
    auto remote = Remote();
    if (remote == nullptr) {
        HiLog::Error(LABEL, "SysEventService Remote is null.");
        return ERR_REMOTE_SERVICE_IS_NULL;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(SysEventServiceProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "write descriptor failed.");
        return ERR_CAN_NOT_WRITE_DESCRIPTOR;
    }
    if (callback == nullptr) {
        return ERR_PARCEL_DATA_IS_NULL;
    }
    bool ret = data.WriteRemoteObject(callback->AsObject());
    if (!ret) {
        HiLog::Error(LABEL, "parcel write callback failed.");
        return ERR_CAN_NOT_WRITE_REMOTE_OBJECT;
    }
    ret = data.WriteBool(mode);
    if (!ret) {
        HiLog::Error(LABEL, "parcel write mode failed.");
        return ERR_CAN_NOT_WRITE_PARCEL;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t res = remote->SendRequest(SET_DEBUG_MODE, data, reply, option);
    if (res != ERR_OK) {
        HiLog::Error(LABEL, "send request failed, error is %{public}d.", res);
        return ERR_CAN_NOT_SEND_REQ;
    }
    int32_t result;
    ret = reply.ReadInt32(result);
    if (!ret) {
        HiLog::Error(LABEL, "parcel read result failed.");
        return ERR_CAN_NOT_READ_PARCEL;
    }
    return result;
}
} // namespace HiviewDFX
} // namespace OHOS

