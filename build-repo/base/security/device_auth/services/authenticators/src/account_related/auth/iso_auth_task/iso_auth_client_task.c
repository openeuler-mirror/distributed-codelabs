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

#include "iso_auth_client_task.h"
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
    TASK_STATUS_ISO_MAIN_BEGIN = 0,
    TASK_STATUS_ISO_MAIN_STEP_ONE = 1,
    TASK_STATUS_ISO_MAIN_STEP_TWO = 2,
    TASK_STATUS_ISO_MAIN_END = 3,
};

static AccountTaskType GetIsoAuthClientType(void)
{
    return TASK_TYPE_ISO_AUTH_CLIENT;
}

static int32_t AddBeginDataToJson(const IsoAuthParams *params, CJson *sendToPeer)
{
    CJson *data = CreateJson();
    if (data == NULL) {
        LOGE("Create data json failed.");
        return HC_ERR_JSON_CREATE;
    }

    if (AddByteToJson(data, FIELD_SALT,
        params->isoBaseParams.randSelf.val, params->isoBaseParams.randSelf.length) != CLIB_SUCCESS) {
        LOGE("Add saltSelf to json failed.");
        goto CLEAN_UP;
    }
    if (AddByteToJson(data, FIELD_PAYLOAD,
        params->isoBaseParams.authIdSelf.val, params->isoBaseParams.authIdSelf.length) != CLIB_SUCCESS) {
        LOGE("Add payloadSelf to json failed.");
        goto CLEAN_UP;
    }
    if (AddByteToJson(data, FIELD_SEED, params->seed, sizeof(params->seed)) != CLIB_SUCCESS) {
        LOGE("Add seed to json failed.");
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

static int32_t PackIsoAuthClientBeginMsg(const IsoAuthParams *params, CJson *out)
{
    CJson *sendToPeer = CreateJson();
    if (sendToPeer == NULL) {
        LOGE("Create sendToPeer json failed.");
        return HC_ERR_JSON_CREATE;
    }

    if (AddIntToJson(sendToPeer, FIELD_AUTH_FORM, params->authForm) != CLIB_SUCCESS) {
        LOGE("Add authForm to json failed.");
        goto CLEAN_UP;
    }
    if (AddIntToJson(sendToPeer, FIELD_STEP, CMD_ISO_AUTH_MAIN_ONE) != CLIB_SUCCESS) {
        LOGE("Add step code to json failed.");
        goto CLEAN_UP;
    }
    if (AddIntToJson(sendToPeer, FIELD_CREDENTIAL_TYPE, params->credentialType) != CLIB_SUCCESS) {
        LOGE("Add credentialType to json failed.");
        goto CLEAN_UP;
    }
    if (AddStringToJson(sendToPeer, FIELD_USER_ID, params->userIdSelf) != CLIB_SUCCESS) {
        LOGE("Add userIdSelf to json failed.");
        goto CLEAN_UP;
    }
    if (AddByteToJson(sendToPeer, FIELD_DEV_ID, params->devIdSelf.val, params->devIdSelf.length) != CLIB_SUCCESS) {
        LOGE("Add devIdSelf to json failed.");
        goto CLEAN_UP;
    }
    if (AddStringToJson(sendToPeer, FIELD_DEVICE_ID, params->deviceIdSelf) != CLIB_SUCCESS) {
        LOGE("Add deviceIdSelf to json failed.");
        goto CLEAN_UP;
    }

    if (AddBeginDataToJson(params, sendToPeer) != HC_SUCCESS) {
        LOGE("AddBeginDataToJson failed.");
        goto CLEAN_UP;
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

static int32_t AccountAuthGenSeed(IsoAuthParams *params)
{
    Uint8Buff seedBuff = { params->seed, sizeof(params->seed) };
    int32_t res = params->isoBaseParams.loader->generateRandom(&seedBuff);
    if (res != HC_SUCCESS) {
        LOGE("GenerateRandom for seed failed, res: %d.", res);
    }
    return res;
}

static int32_t IsoAuthClientBegin(TaskBase *task, const CJson *in, CJson *out, int32_t *status)
{
    (void)in;
    if (task->taskStatus != TASK_STATUS_ISO_MAIN_BEGIN) {
        LOGD("The message is repeated, ignore it, taskStatus: %d.", task->taskStatus);
        *status = IGNORE_MSG;
        return HC_SUCCESS;
    }

    IsoAuthClientTask *innerTask = (IsoAuthClientTask *)task;
    int32_t ret = IsoClientGenRandom(&innerTask->params.isoBaseParams);
    if (ret != HC_SUCCESS) {
        LOGE("IsoClientGenRandom failed, res: %d.", ret);
        return ret;
    }
    ret = AccountAuthGenSeed(&innerTask->params);
    if (ret != HC_SUCCESS) {
        LOGE("AccountAuthGenSeed failed, res: %d.", ret);
        return ret;
    }

    // Send params to server.
    ret = PackIsoAuthClientBeginMsg(&innerTask->params, out);
    if (ret != HC_SUCCESS) {
        LOGE("PackIsoAuthClientBeginMsg failed, ret: %d.", ret);
        return ret;
    }

    innerTask->taskBase.taskStatus = TASK_STATUS_ISO_MAIN_STEP_ONE;
    *status = CONTINUE;
    return HC_SUCCESS;
}

static int32_t ParseIsoAuthServerGetTokenMsg(IsoAuthParams *params, const CJson *in, Uint8Buff *peerToken)
{
    const char *userIdPeer = GetStringFromJson(in, FIELD_USER_ID);
    if (userIdPeer == NULL) {
        LOGE("Failed to get userIdPeer from input data for client in sym auth.");
        return HC_ERR_JSON_GET;
    }
    if (strcpy_s(params->userIdPeer, DEV_AUTH_USER_ID_SIZE, userIdPeer) != EOK) {
        LOGE("Copy for userIdPeer failed for client in sym auth.");
        return HC_ERR_MEMORY_COPY;
    }
    if (GetByteFromJson(in, FIELD_SALT, params->isoBaseParams.randPeer.val,
        params->isoBaseParams.randPeer.length) != CLIB_SUCCESS) {
        LOGE("Get saltPeer from json failed for client.");
        return HC_ERR_JSON_GET;
    }
    if (GetByteFromJson(in, FIELD_TOKEN, peerToken->val, peerToken->length) != CLIB_SUCCESS) {
        LOGE("Get peerToken from json failed for client.");
        return HC_ERR_JSON_GET;
    }
    int32_t res = ExtractAndVerifyPayload(params, in);
    if (res != HC_SUCCESS) {
        LOGE("ExtractAndVerifyPayload failed for client, res: %d.", res);
    }
    return res;
}

static int32_t PackIsoAuthClientGetTokenMsg(const IsoAuthParams *params, CJson *out)
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

    if (AddIntToJson(sendToPeer, FIELD_AUTH_FORM, params->authForm) != CLIB_SUCCESS) {
        LOGE("Add authForm to json failed.");
        goto CLEAN_UP;
    }
    if (AddIntToJson(sendToPeer, FIELD_STEP, CMD_ISO_AUTH_MAIN_TWO) != CLIB_SUCCESS) {
        LOGE("Add step code to json failed.");
        goto CLEAN_UP;
    }
    if (AddByteToJson(data, FIELD_TOKEN, params->hmacToken, sizeof(params->hmacToken)) != CLIB_SUCCESS) {
        LOGE("Add hmacToken to json failed.");
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

static int32_t IsoAuthClientGetToken(TaskBase *task, const CJson *in, CJson *out, int32_t *status)
{
    IsoAuthClientTask *innerTask = (IsoAuthClientTask *)task;
    if (innerTask->taskBase.taskStatus < TASK_STATUS_ISO_MAIN_STEP_ONE) {
        LOGE("Message code is not match with task status, taskStatus: %d", innerTask->taskBase.taskStatus);
        return HC_ERR_BAD_MESSAGE;
    }
    if (innerTask->taskBase.taskStatus > TASK_STATUS_ISO_MAIN_STEP_ONE) {
        LOGI("The message is repeated, ignore it, taskStatus: %d.", innerTask->taskBase.taskStatus);
        *status = IGNORE_MSG;
        return HC_SUCCESS;
    }

    // Receive params from server.
    uint8_t peerToken[HMAC_TOKEN_SIZE] = { 0 };
    Uint8Buff peerTokenBuf = { peerToken, HMAC_TOKEN_SIZE };
    int32_t res = ParseIsoAuthServerGetTokenMsg(&innerTask->params, in, &peerTokenBuf);
    if (res != HC_SUCCESS) {
        LOGE("ParseIsoAuthServerGetTokenMsg failed, res: %d.", res);
        return res;
    }

    // Get psk and process hmacToken.
    res = AccountAuthGeneratePsk(&innerTask->params);
    if (res != HC_SUCCESS) {
        LOGE("AccountAuthGeneratePsk failed, res: %d.", res);
        return res;
    }
    Uint8Buff selfTokenBuf = { innerTask->params.hmacToken, HMAC_TOKEN_SIZE };
    res = IsoClientCheckAndGenToken(&(innerTask->params.isoBaseParams), &peerTokenBuf, &selfTokenBuf);
    if (res != HC_SUCCESS) {
        LOGE("IsoClientCheckAndGenToken failed, res: %d.", res);
        return res;
    }

    // Send params to server.
    res = PackIsoAuthClientGetTokenMsg(&innerTask->params, out);
    if (res != HC_SUCCESS) {
        LOGE("PackIsoAuthClientGetTokenMsg failed, res: %d.", res);
        return res;
    }

    innerTask->taskBase.taskStatus = TASK_STATUS_ISO_MAIN_STEP_TWO;
    *status = CONTINUE;
    return HC_SUCCESS;
}

static int32_t IsoAuthClientGetSessionKey(TaskBase *task, const CJson *in, CJson *out, int32_t *status)
{
    IsoAuthClientTask *innerTask = (IsoAuthClientTask *)task;
    if (innerTask->taskBase.taskStatus < TASK_STATUS_ISO_MAIN_STEP_TWO) {
        LOGE("Message code is not match with task status, taskStatus: %d", innerTask->taskBase.taskStatus);
        return HC_ERR_BAD_MESSAGE;
    }
    if (innerTask->taskBase.taskStatus > TASK_STATUS_ISO_MAIN_STEP_TWO) {
        LOGI("The message is repeated, ignore it, taskStatus: %d", innerTask->taskBase.taskStatus);
        *status = IGNORE_MSG;
        return HC_SUCCESS;
    }

    // Receive params from server.
    uint8_t authResultHmac[AUTH_RESULT_MAC_SIZE] = { 0 };
    if (GetByteFromJson(in, FIELD_AUTH_RESULT_MAC, authResultHmac, sizeof(authResultHmac)) != CLIB_SUCCESS) {
        LOGE("Get authResultHmac from json failed.");
        return HC_ERR_JSON_GET;
    }

    // Generate and verify the HMAC, then generate session key.
    int32_t res = IsoClientGenSessionKey(&(innerTask->params.isoBaseParams), 0, authResultHmac, sizeof(authResultHmac));
    if (res != HC_SUCCESS) {
        LOGE("IsoClientGenSessionKey failed, res: %d.", res);
        return res;
    }

    res = AuthIsoSendFinalToOut(&innerTask->params, out);
    if (res != HC_SUCCESS) {
        LOGE("AuthIsoSendFinalToOut failed, res: %d.", res);
        return res;
    }

    innerTask->taskBase.taskStatus = TASK_STATUS_ISO_MAIN_END;
    *status = FINISH;
    return HC_SUCCESS;
}

static int32_t ProcessClientTask(TaskBase *task, const CJson *in, CJson *out, int32_t *status)
{
    int32_t res;
    if (task->taskStatus == TASK_STATUS_ISO_MAIN_BEGIN) {
        res = IsoAuthClientBegin(task, in, out, status);
        if (res != HC_SUCCESS) {
            LOGE("IsoAuthClientBegin failed, res: %d.", res);
        }
        return res;
    }

    int32_t authStep;
    if (GetIntFromJson(in, FIELD_STEP, &authStep) != CLIB_SUCCESS) {
        LOGE("Get message code from json failed.");
        return HC_ERR_JSON_GET;
    }
    switch (authStep) {
        case RET_ISO_AUTH_FOLLOWER_ONE:
            res = IsoAuthClientGetToken(task, in, out, status);
            break;
        case RET_ISO_AUTH_FOLLOWER_TWO:
            res = IsoAuthClientGetSessionKey(task, in, out, status);
            break;
        default:
            res = HC_ERR_BAD_MESSAGE;
    }
    if (res != HC_SUCCESS) {
        LOGE("Process iso auth client failed, step: %d, res: %d.", authStep, res);
    }
    return res;
}

static void DestroyAuthClientAuthTask(TaskBase *task)
{
    if (task == NULL) {
        return;
    }
    IsoAuthClientTask *innerTask = (IsoAuthClientTask *)task;
    DestroyIsoAuthParams(&(innerTask->params));
    HcFree(innerTask);
}

TaskBase *CreateIsoAuthClientTask(const CJson *in, CJson *out, const AccountVersionInfo *verInfo)
{
    if ((in == NULL) || (out == NULL) || (verInfo == NULL)) {
        LOGE("Params is null for client sym auth.");
        return NULL;
    }
    IsoAuthClientTask *task = (IsoAuthClientTask *)HcMalloc(sizeof(IsoAuthClientTask), 0);
    if (task == NULL) {
        LOGE("Malloc for IsoAuthClientTask failed.");
        return NULL;
    }
    task->taskBase.getTaskType = GetIsoAuthClientType;
    task->taskBase.process = ProcessClientTask;
    task->taskBase.destroyTask = DestroyAuthClientAuthTask;

    int32_t res = InitIsoAuthParams(in, &(task->params), verInfo);
    if (res != HC_SUCCESS) {
        LOGE("InitIsoAuthParams failed, res: %d.", res);
        DestroyAuthClientAuthTask((TaskBase *)task);
        return NULL;
    }

    task->taskBase.taskStatus = TASK_STATUS_ISO_MAIN_BEGIN;
    return (TaskBase *)task;
}
