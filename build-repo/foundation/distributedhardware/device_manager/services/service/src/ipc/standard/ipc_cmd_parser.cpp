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

#include <memory>

#include "device_manager_service.h"
#include "dm_constants.h"
#include "dm_device_info.h"
#include "dm_log.h"
#include "dm_subscribe_info.h"
#include "dm_publish_info.h"
#include "ipc_cmd_register.h"
#include "ipc_def.h"
#include "ipc_notify_auth_result_req.h"
#include "ipc_notify_credential_req.h"
#include "ipc_notify_device_found_req.h"
#include "ipc_notify_device_state_req.h"
#include "ipc_notify_discover_result_req.h"
#include "ipc_notify_publish_result_req.h"
#include "ipc_notify_verify_auth_result_req.h"
#include "ipc_server_stub.h"

namespace OHOS {
namespace DistributedHardware {
ON_IPC_SET_REQUEST(SERVER_DEVICE_STATE_NOTIFY, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    if (pBaseReq == nullptr) {
        return ERR_DM_FAILED;
    }

    std::shared_ptr<IpcNotifyDeviceStateReq> pReq = std::static_pointer_cast<IpcNotifyDeviceStateReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    int32_t deviceState = pReq->GetDeviceState();
    DmDeviceInfo deviceInfo = pReq->GetDeviceInfo();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(deviceState)) {
        LOGE("write state failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteRawData(&deviceInfo, sizeof(DmDeviceInfo))) {
        LOGE("write deviceInfo failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(SERVER_DEVICE_STATE_NOTIFY, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    if (pBaseRsp == nullptr) {
        LOGE("pBaseRsp is null");
        return ERR_DM_FAILED;
    }
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(SERVER_DEVICE_FOUND, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    if (pBaseReq == nullptr) {
        return ERR_DM_FAILED;
    }

    std::shared_ptr<IpcNotifyDeviceFoundReq> pReq = std::static_pointer_cast<IpcNotifyDeviceFoundReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    uint16_t subscribeId = pReq->GetSubscribeId();
    DmDeviceInfo deviceInfo = pReq->GetDeviceInfo();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt16((int16_t)subscribeId)) {
        LOGE("write subscribeId failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteRawData(&deviceInfo, sizeof(DmDeviceInfo))) {
        LOGE("write deviceInfo failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(SERVER_DEVICE_FOUND, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    if (pBaseRsp == nullptr) {
        LOGE("pBaseRsp is null");
        return ERR_DM_FAILED;
    }
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(SERVER_DISCOVER_FINISH, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    if (pBaseReq == nullptr) {
        return ERR_DM_FAILED;
    }
    std::shared_ptr<IpcNotifyDiscoverResultReq> pReq = std::static_pointer_cast<IpcNotifyDiscoverResultReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    uint16_t subscribeId = pReq->GetSubscribeId();
    int32_t result = pReq->GetResult();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt16((int16_t)subscribeId)) {
        LOGE("write subscribeId failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(SERVER_DISCOVER_FINISH, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    if (pBaseRsp == nullptr) {
        LOGE("pBaseRsp is null");
        return ERR_DM_FAILED;
    }
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(SERVER_PUBLISH_FINISH, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    if (pBaseReq == nullptr) {
        return ERR_DM_FAILED;
    }
    std::shared_ptr<IpcNotifyPublishResultReq> pReq = std::static_pointer_cast<IpcNotifyPublishResultReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    int32_t publishId = pReq->GetPublishId();
    int32_t result = pReq->GetResult();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(publishId)) {
        LOGE("write publishId failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(SERVER_PUBLISH_FINISH, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    if (pBaseRsp == nullptr) {
        LOGE("pBaseRsp is null");
        return ERR_DM_FAILED;
    }
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(SERVER_AUTH_RESULT, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    if (pBaseReq == nullptr) {
        return ERR_DM_FAILED;
    }
    std::shared_ptr<IpcNotifyAuthResultReq> pReq = std::static_pointer_cast<IpcNotifyAuthResultReq>(pBaseReq);

    std::string pkgName = pReq->GetPkgName();
    std::string deviceId = pReq->GetDeviceId();
    std::string token = pReq->GetPinToken();
    int32_t status = pReq->GetStatus();
    int32_t reason = pReq->GetReason();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(deviceId)) {
        LOGE("write deviceId failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(token)) {
        LOGE("write token failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(status)) {
        LOGE("write status failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(reason)) {
        LOGE("write reason failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(SERVER_AUTH_RESULT, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    if (pBaseRsp == nullptr) {
        LOGE("pBaseRsp is null");
        return ERR_DM_FAILED;
    }
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(SERVER_VERIFY_AUTH_RESULT, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    if (pBaseReq == nullptr) {
        return ERR_DM_FAILED;
    }
    std::shared_ptr<IpcNotifyVerifyAuthResultReq> pReq =
        std::static_pointer_cast<IpcNotifyVerifyAuthResultReq>(pBaseReq);

    std::string pkgName = pReq->GetPkgName();
    std::string deviceId = pReq->GetDeviceId();
    int32_t result = pReq->GetResult();
    int32_t flag = pReq->GetFlag();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(deviceId)) {
        LOGE("write deviceId failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(flag)) {
        LOGE("write flag failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(SERVER_VERIFY_AUTH_RESULT, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    if (pBaseRsp == nullptr) {
        LOGE("pBaseRsp is null");
        return ERR_DM_FAILED;
    }
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(SERVER_DEVICE_FA_NOTIFY, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    if (pBaseReq == nullptr) {
        return ERR_DM_FAILED;
    }

    std::shared_ptr<IpcNotifyDMFAResultReq> pReq = std::static_pointer_cast<IpcNotifyDMFAResultReq>(pBaseReq);

    std::string packagname = pReq->GetPkgName();
    std::string paramJson = pReq->GetJsonParam();
    if (!data.WriteString(packagname)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(paramJson)) {
        LOGE("write paramJson failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(SERVER_DEVICE_FA_NOTIFY, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    if (pBaseRsp == nullptr) {
        LOGE("pBaseRsp is null");
        return ERR_DM_FAILED;
    }
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_CMD(GET_TRUST_DEVICE_LIST, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    std::string extra = data.ReadString();
    std::vector<DmDeviceInfo> deviceList;
    int32_t result = DeviceManagerService::GetInstance().GetTrustedDeviceList(pkgName, extra, deviceList);
    int32_t infoNum =(int32_t)(deviceList.size());
    DmDeviceInfo deviceInfo;
    if (!reply.WriteInt32(infoNum)) {
        LOGE("write infoNum failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!deviceList.empty()) {
        for (; !deviceList.empty();) {
            deviceInfo = deviceList.back();
            deviceList.pop_back();

            if (!reply.WriteRawData(&deviceInfo, sizeof(DmDeviceInfo))) {
                LOGE("write subscribeInfo failed");
                return ERR_DM_IPC_WRITE_FAILED;
            }
        }
    }
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(REGISTER_DEVICE_MANAGER_LISTENER, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    sptr<IRemoteObject> listener = data.ReadRemoteObject();
    int32_t result = IpcServerStub::GetInstance().RegisterDeviceManagerListener(pkgName, listener);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(UNREGISTER_DEVICE_MANAGER_LISTENER, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    int32_t result = IpcServerStub::GetInstance().UnRegisterDeviceManagerListener(pkgName);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(START_DEVICE_DISCOVER, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    std::string extra = data.ReadString();
    DmSubscribeInfo *subscribeInfo = (DmSubscribeInfo *)data.ReadRawData(sizeof(DmSubscribeInfo));
    int32_t result = ERR_DM_POINT_NULL;

    if (subscribeInfo != nullptr) {
        result = DeviceManagerService::GetInstance().StartDeviceDiscovery(pkgName, *subscribeInfo, extra);
    }
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(STOP_DEVICE_DISCOVER, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    uint16_t subscribeId = (uint16_t)(data.ReadInt32());
    int32_t result = DeviceManagerService::GetInstance().StopDeviceDiscovery(pkgName, subscribeId);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(PUBLISH_DEVICE_DISCOVER, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    DmPublishInfo *publishInfo = (DmPublishInfo *)data.ReadRawData(sizeof(DmPublishInfo));
    int32_t result = ERR_DM_POINT_NULL;

    if (publishInfo != nullptr) {
        result = DeviceManagerService::GetInstance().PublishDeviceDiscovery(pkgName, *publishInfo);
    }
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(UNPUBLISH_DEVICE_DISCOVER, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    int32_t publishId = data.ReadInt32();
    int32_t result = DeviceManagerService::GetInstance().UnPublishDeviceDiscovery(pkgName, publishId);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(AUTHENTICATE_DEVICE, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    std::string extra = data.ReadString();
    std::string deviceId = data.ReadString();
    int32_t authType = data.ReadInt32();
    int32_t result = DM_OK;
    result = DeviceManagerService::GetInstance().AuthenticateDevice(pkgName, authType, deviceId, extra);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(UNAUTHENTICATE_DEVICE, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    std::string deviceId = data.ReadString();
    int32_t result = DeviceManagerService::GetInstance().UnAuthenticateDevice(pkgName, deviceId);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(VERIFY_AUTHENTICATION, MessageParcel &data, MessageParcel &reply)
{
    std::string authPara = data.ReadString();
    int32_t result = DeviceManagerService::GetInstance().VerifyAuthentication(authPara);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(GET_LOCAL_DEVICE_INFO, MessageParcel &data, MessageParcel &reply)
{
    DmDeviceInfo localDeviceInfo;
    int32_t result = 0;
    result = DeviceManagerService::GetInstance().GetLocalDeviceInfo(localDeviceInfo);

    if (!reply.WriteRawData(&localDeviceInfo, sizeof(DmDeviceInfo))) {
        LOGE("write subscribeInfo failed");
    }

    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(GET_UDID_BY_NETWORK, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    std::string netWorkId = data.ReadString();
    std::string udid;
    int32_t result = DeviceManagerService::GetInstance().GetUdidByNetworkId(pkgName, netWorkId, udid);

    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!reply.WriteString(udid)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(GET_UUID_BY_NETWORK, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    std::string netWorkId = data.ReadString();
    std::string uuid;
    int32_t result = DeviceManagerService::GetInstance().GetUuidByNetworkId(pkgName, netWorkId, uuid);

    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!reply.WriteString(uuid)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(SERVER_GET_DMFA_INFO, MessageParcel &data, MessageParcel &reply)
{
    std::string packName = data.ReadString();
    DmAuthParam authParam;
    if (DeviceManagerService::GetInstance().GetFaParam(packName, authParam) != DM_OK) {
        LOGE("ipc read fa parm failed");
        return ERR_DM_IPC_READ_FAILED;
    }
    int32_t appIconLen = authParam.imageinfo.GetAppIconLen();
    int32_t appThumbnailLen = authParam.imageinfo.GetAppThumbnailLen();

    reply.WriteInt32(authParam.direction);
    reply.WriteInt32(authParam.authType);
    reply.WriteString(authParam.authToken);
    reply.WriteString(authParam.packageName);
    reply.WriteString(authParam.appName);
    reply.WriteString(authParam.appDescription);
    reply.WriteInt32(authParam.business);
    reply.WriteInt32(authParam.pincode);
    reply.WriteInt32(appIconLen);
    reply.WriteInt32(appThumbnailLen);

    if (appIconLen > 0 && authParam.imageinfo.GetAppIcon() != nullptr) {
        if (!reply.WriteRawData(authParam.imageinfo.GetAppIcon(), appIconLen)) {
            LOGE("write appIcon failed");
            return ERR_DM_IPC_WRITE_FAILED;
        }
    }
    if (appThumbnailLen > 0 && authParam.imageinfo.GetAppThumbnail() != nullptr) {
        if (!reply.WriteRawData(authParam.imageinfo.GetAppThumbnail(), appThumbnailLen)) {
            LOGE("write appThumbnail failed");
            return ERR_DM_IPC_WRITE_FAILED;
        }
    }
    return DM_OK;
}

ON_IPC_CMD(SERVER_USER_AUTH_OPERATION, MessageParcel &data, MessageParcel &reply)
{
    std::string packageName = data.ReadString();
    int32_t action = data.ReadInt32();
    std::string params = data.ReadString();
    int result = DeviceManagerService::GetInstance().SetUserOperation(packageName, action, params);
    if (!reply.WriteInt32(result)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return result;
}

ON_IPC_CMD(REGISTER_DEV_STATE_CALLBACK, MessageParcel &data, MessageParcel &reply)
{
    std::string packageName = data.ReadString();
    std::string extra = data.ReadString();
    int result = DeviceManagerService::GetInstance().RegisterDevStateCallback(packageName, extra);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return result;
}

ON_IPC_CMD(UNREGISTER_DEV_STATE_CALLBACK, MessageParcel &data, MessageParcel &reply)
{
    std::string packageName = data.ReadString();
    std::string extra = data.ReadString();
    int result = DeviceManagerService::GetInstance().UnRegisterDevStateCallback(packageName, extra);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return result;
}

ON_IPC_CMD(REQUEST_CREDENTIAL, MessageParcel &data, MessageParcel &reply)
{
    std::string packageName = data.ReadString();
    std::string reqJsonStr = data.ReadString();
    std::string returnJsonStr;
    int32_t ret = DeviceManagerService::GetInstance().RequestCredential(reqJsonStr, returnJsonStr);
    if (!reply.WriteInt32(ret)) {
        LOGE("write ret failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (ret == DM_OK && !returnJsonStr.empty()) {
        if (!reply.WriteString(returnJsonStr))
        {
            LOGE("write returnJsonStr failed");
            return ERR_DM_IPC_WRITE_FAILED;
        }
    }
    return DM_OK;
}

ON_IPC_CMD(IMPORT_CREDENTIAL, MessageParcel &data, MessageParcel &reply)
{
    std::string packageName = data.ReadString();
    std::string credentialInfo = data.ReadString();
    int32_t ret = DeviceManagerService::GetInstance().ImportCredential(packageName, credentialInfo);
    if (!reply.WriteInt32(ret)) {
        LOGE("write ret failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(DELETE_CREDENTIAL, MessageParcel &data, MessageParcel &reply)
{
    std::string packageName = data.ReadString();
    std::string deleteInfo = data.ReadString();
    int32_t ret = DeviceManagerService::GetInstance().DeleteCredential(packageName, deleteInfo);
    if (!reply.WriteInt32(ret)) {
        LOGE("write ret failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(REGISTER_CREDENTIAL_CALLBACK, MessageParcel &data, MessageParcel &reply)
{
    std::string packageName = data.ReadString();
    int result = DeviceManagerService::GetInstance().RegisterCredentialCallback(packageName);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return result;
}

ON_IPC_CMD(UNREGISTER_CREDENTIAL_CALLBACK, MessageParcel &data, MessageParcel &reply)
{
    std::string packageName = data.ReadString();
    int result = DeviceManagerService::GetInstance().UnRegisterCredentialCallback(packageName);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return result;
}

ON_IPC_CMD(NOTIFY_EVENT, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    int32_t eventId = data.ReadInt32();
    std::string event = data.ReadString();
    int32_t result = DeviceManagerService::GetInstance().NotifyEvent(pkgName, eventId, event);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_SET_REQUEST(SERVER_CREDENTIAL_RESULT, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    if (pBaseReq == nullptr) {
        return ERR_DM_FAILED;
    }
    std::shared_ptr<IpcNotifyCredentialReq> pReq = std::static_pointer_cast<IpcNotifyCredentialReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    int32_t action = pReq->GetCredentialAction();
    std::string credentialResult = pReq->GetCredentialResult();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(action)) {
        LOGE("write action failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(credentialResult)) {
        LOGE("write credentialResult failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }

    return DM_OK;
}

ON_IPC_READ_RESPONSE(SERVER_CREDENTIAL_RESULT, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    if (pBaseRsp == nullptr) {
        LOGE("pBaseRsp is null");
        return ERR_DM_FAILED;
    }
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
