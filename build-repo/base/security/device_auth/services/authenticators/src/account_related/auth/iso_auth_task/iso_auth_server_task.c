/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "iso_auth_server_task.h"
#include "account_module_defines.h"
#include "clib_error.h"
#include "common_defs.h"
#include "device_auth.h"
#include "device_auth_defines.h"
#include "hc_log.h"
#include "hc_types.h"
#include "iso_auth_task_common.h"
#include "iso_protocol_common.h"
#include "protocol_common.h"

enum {
    TASK_STATUS_SERVER_BEGIN_TOKEN = 0,
    TASK_STATUS_SERVER_GEN_SESSION_KEY = 1,
    TASK_STATUS_SERVER_END = 2,
};

static AccountTaskType GetIsoAuthServerType(void)
{
    return TASK_TYPE_ISO_AUTH_SERVER;
}

static int32_t ParseIsoAuthClientBeginMsg(IsoAuthParams *params, const CJson *in)
{
    const char *userIdPeer = GetStringFromJson(in, FIELD_USER_ID);
    if (userIdPeer == NULL) {
        LOGE("Failed to get userIdPeer from input data for server in sym auth.");
        return HC_ERR_JSON_GET;
    }
    if (strcpy_s(params->userIdPeer, DEV_AUTH_USER_ID_SIZE, userIdPeer) != EOK) {
        LOGE("Copy for userIdPeer failed for server in sym auth.");
        return HC_ERR_MEMORY_COPY;
    }
    if (GetByteFromJson(in, FIELD_SEED, params->seed, sizeof(params->seed)) != CLIB_SUCCESS) {
        LOGE("Get seed from json failed for server.");
        return HC_ERR_JSON_GET;
    }
    if (GetByteFromJson(in, FIELD_SALT, params->isoBaseParams.randPeer.val,
        params->isoBaseParams.randPeer.length) != CLIB_SUCCESS) {
        LOGE("Get saltPeer from json failed for server.");
        return HC_ERR_JSON_GET;
    }
    int32_t res = ExtractAndVerifyPayload(params, in);
    if (res != HC_SUCCESS) {
        LOGE("ExtractAndVerifyPayload failed for server, res: %d.", res);
    }
    return res;
}

static int32_t AddGetTokenDataToJson(const IsoAuthParams *params, CJson *sendToPeer)
{
    CJson *data = CreateJson();
    if (data == NULL) {
        LOGE("Create data json failed.");
        return HC_ERR_JSON_CREATE;
    }
    if (AddByteToJson(data, FIELD_PAYLOAD,
        params->isoBaseParams.authIdSelf.val, params->isoBaseParams.authIdSelf.length) != CLIB_SUCCESS) {
        LOGE("Add payloadSelf to json failed.");
        goto CLEAN_UP;
    }
    if (AddByteToJson(data, FIELD_TOKEN, params->hmacToken, sizeof(params->hmacToken)) != CLIB_SUCCESS) {
        LOGE("Add hmacToken to json failed.");
        goto CLEAN_UP;
    }
    if (AddByteToJson(data, FIELD_SALT,
        params->isoBaseParams.randSelf.val, params->isoBaseParams.randSelf.length) != CLIB_SUCCESS) {
        LOGE("Add saltSelf to json failed.");
        goto CLEAN_UP;
    }
    if (AddObjToJson(sendToPeer, FIELD_DATA, data) != CLIB_SUCCESS) {
        LOGE("Add data json obj to json failed.");
        goto CLEAN_UP;
    }

    FreeJson(data);
    return HC_SUCCESS;
CLEAN_UP:
    FreeJson(data);
    return HC_ERR_JSON_ADD;
}

static int32_t PackIsoAuthServerGetTokenMsg(const IsoAuthParams *params, CJson *out)
{
    CJson *sendToPeer = CreateJson();
    if (sendToPeer == NULL) {
        LOGE("Create sendToPeer json is null in server.");
        return HC_ERR_JSON_CREATE;
    }

    if (AddIntToJson(sendToPeer, FIELD_STEP, RET_ISO_AUTH_FOLLOWER_ONE) != CLIB_SUCCESS) {
        LOGE("Add step code to json failed in server.");
        goto CLEAN_UP;
    }
    if (AddIntToJson(sendToPeer, FIELD_AUTH_FORM, params->authForm) != CLIB_SUCCESS) {
        LOGE("Add authForm to json failed in server.");
        goto CLEAN_UP;
    }
    if (AddStringToJson(sendToPeer, FIELD_USER_ID, params->userIdSelf) != CLIB_SUCCESS) {
        LOGE("Add userIdSelf to json failed in server.");
        goto CLEAN_UP;
    }
    if (AddByteToJson(sendToPeer, FIELD_DEV_ID, params->devIdSelf.val, params->devIdSelf.length) != CLIB_SUCCESS) {
        LOGE("Add devIdSelf to json failed in server.");
        goto CLEAN_UP;
    }
    if (AddStringToJson(sendToPeer, FIELD_DEVICE_ID, params->deviceIdSelf) != CLIB_SUCCESS) {
        LOGE("Add deviceIdSelf to json failed in server.");
        goto CLEAN_UP;
    }

    int32_t res = AddGetTokenDataToJson(params, sendToPeer);
    if (res != HC_SUCCESS) {
        LOGE("AddGetTokenDataToJson failed, res: %d.", res);
        FreeJson(sendToPeer);
        return res;
    }

    if (AddObjToJson(out, FIELD_SEND_TO_PEER, sendToPeer) != CLIB_SUCCESS) {
        LOGE("Add sendToPeer to json failed.");
        goto CLEAN_UP;
    }

    FreeJson(sendToPeer);
    return HC_SUCCESS;
CLEAN_UP:
    FreeJson(sendToPeer);
    return HC_ERR_JSON_ADD;
}

static int32_t IsoAuthServerGetToken(TaskBase *task, const CJson *in, CJson *out, int32_t *status)
{
    IsoAuthServerTask *innerTask = (IsoAuthServerTask *)task;
    if (innerTask->taskBase.taskStatus < TASK_STATUS_SERVER_BEGIN_TOKEN) {
        LOGE("Message code is not match with task status, taskStatus: %d", innerTask->taskBase.taskStatus);
        return HC_ERR_BAD_MESSAGE;
    }
    if (innerTask->taskBase.taskStatus > TASK_STATUS_SERVER_BEGIN_TOKEN) {
        LOGI("The message is repeated, ignore it, taskStatus: %d.", innerTask->taskBase.taskStatus);
        *status = IGNORE_MSG;
        return HC_SUCCESS;
    }

    // Receive params from client.
    int32_t res = ParseIsoAuthClientBeginMsg(&innerTask->params, in);
    if (res != HC_SUCCESS) {
        LOGE("ParseIsoAuthClientBeginMsg failed, res: %d.", res);
        return res;
    }

    // Get psk and process hmacToken.
    res = AccountAuthGeneratePsk(&innerTask->params);
    if (res != HC_SUCCESS) {
        LOGE("AccountAuthGeneratePsk failed, res: %d.", res);
        return res;
    }
    Uint8Buff selfTokenBuf = { innerTask->params.hmacToken, HMAC_TOKEN_SIZE };
    res = IsoServerGenRandomAndToken(&innerTask->params.isoBaseParams, &selfTokenBuf);
    if (res != HC_SUCCESS) {
        LOGE("IsoServerGenRandomAndToken failed, res: %d.", res);
        return res;
    }

    // Send params to client.
    res = PackIsoAuthServerGetTokenMsg(&innerTask->params, out);
    if (res != HC_SUCCESS) {
        LOGE("PackIsoAuthServerGetTokenMsg failed, res: %d.", res);
        return res;
    }

    innerTask->taskBase.taskStatus = TASK_STATUS_SERVER_GEN_SESSION_KEY;
    *status = CONTINUE;
    return HC_SUCCESS;
}

static int32_t PackCalTokenAndSessionKeyMsg(const IsoAuthParams *params, CJson *out, const Uint8Buff *authResultMac)
{
    CJson *sendToPeer = CreateJson();
    if (sendToPeer == NULL) {
        LOGE("Create sendToPeer json failed.");
        return HC_ERR_JSON_CREATE;
    }
    CJson *data = CreateJson();
    if (data == NULL) {
        LOGE("Create data json failed.");
        FreeJson(sendToPeer);
        return HC_ERR_JSON_CREATE;
    }

    if (AddIntToJson(sendToPeer, FIELD_STEP, RET_ISO_AUTH_FOLLOWER_TWO) != CLIB_SUCCESS) {
        LOGE("Add step code to json failed.");
        goto CLEAN_UP;
    }
    if (AddIntToJson(sendToPeer, FIELD_AUTH_FORM, params->authForm) != CLIB_SUCCESS) {
        LOGE("Add authForm to json failed.");
        goto CLEAN_UP;
    }
    if (AddByteToJson(data, FIELD_AUTH_RESULT_MAC, authResultMac->val, authResultMac->length) != CLIB_SUCCESS) {
        LOGE("Add authResultMac to json failed.");
        goto CLEAN_UP;
    }
    if (AddObjToJson(sendToPeer, FIELD_DATA, data) != CLIB_SUCCESS) {
        LOGE("Add data json obj to json failed.");
        goto CLEAN_UP;
    }
    if (AddObjToJson(out, FIELD_SEND_TO_PEER, sendToPeer) != CLIB_SUCCESS) {
        LOGE("Add sendToPeer to json failed.");
        goto CLEAN_UP;
    }
    FreeJson(sendToPeer);
    FreeJson(data);
    return HC_SUCCESS;
CLEAN_UP:
    FreeJson(sendToPeer);
    FreeJson(data);
    return HC_ERR_JSON_ADD;
}

static int32_t IsoAuthServerCalTokenAndSessionKey(TaskBase *task, const CJson *in, CJson *out, int32_t *status)
{
    IsoAuthServerTask *innerTask = (IsoAuthServerTask *)task;
    if (innerTask->taskBase.taskStatus < TASK_STATUS_SERVER_GEN_SESSION_KEY) {
        LOGE("Message code is not match with task status, taskStatus: %d", innerTask->taskBase.taskStatus);
        return HC_ERR_BAD_MESSAGE;
    }
    if (innerTask->taskBase.taskStatus > TASK_STATUS_SERVER_GEN_SESSION_KEY) {
        LOGI("The message is repeated, ignore it, taskStatus: %d.", innerTask->taskBase.taskStatus);
        *status = IGNORE_MSG;
        return HC_SUCCESS;
    }

    // Receive params from client.
    uint8_t peerToken[HMAC_TOKEN_SIZE] = { 0 };
    Uint8Buff peerTokenBuf = { peerToken, HMAC_TOKEN_SIZE };
    if (GetByteFromJson(in, FIELD_TOKEN, peerToken, sizeof(peerToken)) != CLIB_SUCCESS) {
        LOGE("Get peerToken from json failed.");
        return HC_ERR_JSON_GET;
    }

    // Process hmacToken and generate session key.
    uint8_t authResultMac[AUTH_RESULT_MAC_SIZE] = { 0 };
    Uint8Buff authResultMacBuf = { authResultMac, AUTH_RESULT_MAC_SIZE };
    int32_t res = IsoServerGenSessionKeyAndCalToken(&(innerTask->params.isoBaseParams),
        &peerTokenBuf, &authResultMacBuf);
    if (res != HC_SUCCESS) {
        LOGE("IsoServerGenSessionKeyAndCalToken failed, res: %d.", res);
        return res;
    }

    // Return params to client.
    res = PackCalTokenAndSessionKeyMsg(&innerTask->params, out, &authResultMacBuf);
    if (res != HC_SUCCESS) {
        LOGE("PackCalTokenAndSessionKeyMsg failed, res: %d.", res);
        return res;
    }

    // Return params to server self.
    res = AuthIsoSendFinalToOut(&innerTask->params, out);
    if (res != HC_SUCCESS) {
        LOGE("AuthIsoSendFinalToOut failed, res: %d.", res);
        return res;
    }

    innerTask->taskBase.taskStatus = TASK_STATUS_SERVER_END;
    *status = FINISH;
    return HC_SUCCESS;
}

static int32_t ProcessServerTask(TaskBase *task, const CJson *in, CJson *out, int32_t *status)
{
    int32_t authStep;
    if (GetIntFromJson(in, FIELD_STEP, &authStep) != CLIB_SUCCESS) {
        LOGE("Get step code from json failed.");
        return HC_ERR_JSON_GET;
    }
    int32_t res;
    switch (authStep) {
        case CMD_ISO_AUTH_MAIN_ONE:
            res = IsoAuthServerGetToken(task, in, out, status);
            break;
        case CMD_ISO_AUTH_MAIN_TWO:
            res = IsoAuthServerCalTokenAndSessionKey(task, in, out, status);
            break;
        default:
            res = HC_ERR_BAD_MESSAGE;
    }
    if (res != HC_SUCCESS) {
        LOGE("Process iso auth server failed, step: %d, res: %d.", authStep, res);
    }
    return res;
}

static void DestroyAuthServerAuthTask(TaskBase *task)
{
    if (task == NULL) {
        return;
    }
    IsoAuthServerTask *innerTask = (IsoAuthServerTask *)task;
    DestroyIsoAuthParams(&(innerTask->params));
    HcFree(innerTask);
}

TaskBase *CreateIsoAuthServerTask(const CJson *in, CJson *out, const AccountVersionInfo *verInfo)
{
    if ((in == NULL) || (out == NULL) || (verInfo == NULL)) {
        LOGE("Params is null for server sym auth.");
        return NULL;
    }
    IsoAuthServerTask *task = (IsoAuthServerTask *)HcMalloc(sizeof(IsoAuthServerTask), 0);
    if (task == NULL) {
        LOGE("Malloc for IsoAuthServerTask failed.");
        return NULL;
    }
    task->taskBase.getTaskType = GetIsoAuthServerType;
    task->taskBase.process = ProcessServerTask;
    task->taskBase.destroyTask = DestroyAuthServerAuthTask;

    int32_t res = InitIsoAuthParams(in, &(task->params), verInfo);
    if (res != HC_SUCCESS) {
        LOGE("InitIsoAuthParams failed, res: %d.", res);
        DestroyAuthServerAuthTask((TaskBase *)task);
        return NULL;
    }

    task->taskBase.taskStatus = TASK_STATUS_SERVER_BEGIN_TOKEN;
    return (TaskBase *)task;
}
