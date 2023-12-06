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

#include "iso_auth_task_common.h"
#include "clib_error.h"
#include "common_defs.h"
#include "device_auth.h"
#include "device_auth_defines.h"
#include "hc_log.h"
#include "hc_types.h"
#include "iso_auth_client_task.h"
#include "iso_auth_server_task.h"
#include "protocol_common.h"
#include "sym_token_manager.h"

#define KEY_INFO_PERSISTENT_TOKEN "persistent_token"
#define AUTH_TOKEN_SIZE_BYTE 32

bool IsIsoAuthTaskSupported(void)
{
    return true;
}

TaskBase *CreateIsoAuthTask(const CJson *in, CJson *out, const AccountVersionInfo *verInfo)
{
    bool isClient = false;
    if (GetBoolFromJson(in, FIELD_IS_CLIENT, &isClient) != CLIB_SUCCESS) {
        LOGD("Get isClient from json failed."); /* default: server. */
    }
    return isClient ? CreateIsoAuthClientTask(in, out, verInfo) :
        CreateIsoAuthServerTask(in, out, verInfo);
}

static int32_t FillUserId(const CJson *in, IsoAuthParams *params)
{
    params->userIdSelf = (char *)HcMalloc(DEV_AUTH_USER_ID_SIZE, 0);
    if (params->userIdSelf == NULL) {
        LOGE("Failed to malloc for userIdSelf.");
        return HC_ERR_ALLOC_MEMORY;
    }
    params->userIdPeer = (char *)HcMalloc(DEV_AUTH_USER_ID_SIZE, 0);
    if (params->userIdPeer == NULL) {
        LOGE("Failed to malloc for userIdPeer.");
        return HC_ERR_ALLOC_MEMORY;
    }
    const char *userIdSelf = GetStringFromJson(in, FIELD_SELF_USER_ID);
    if (userIdSelf == NULL) {
        LOGE("Failed to get self userId from input data in sym auth.");
        return HC_ERR_JSON_GET;
    }

    if (strcpy_s(params->userIdSelf, DEV_AUTH_USER_ID_SIZE, userIdSelf) != EOK) {
        LOGE("Copy for userIdSelf failed in sym auth.");
        return HC_ERR_MEMORY_COPY;
    }
    return HC_SUCCESS;
}

static int32_t FillPayload(const CJson *in, IsoAuthParams *params)
{
    int32_t res = InitSingleParam(&params->devIdSelf, DEV_AUTH_DEVICE_ID_SIZE);
    if (res != HC_SUCCESS) {
        LOGE("InitSingleParam for devIdSelf failed, res: %d.", res);
        return res;
    }
    const char *devIdSelf = GetStringFromJson(in, FIELD_SELF_DEV_ID);
    if (devIdSelf == NULL) {
        LOGE("Failed to get devIdSelf in sym auth.");
        return HC_ERR_JSON_GET;
    }
    uint32_t devIdLen = HcStrlen(devIdSelf);
    if (memcpy_s(params->devIdSelf.val, params->devIdSelf.length, devIdSelf, devIdLen + 1) != EOK) {
        LOGE("Copy for self devId failed in sym auth.");
        return HC_ERR_MEMORY_COPY;
    }
    params->devIdSelf.length = devIdLen;
    const char *selfDeviceId = GetStringFromJson(in, FIELD_SELF_DEVICE_ID);
    if (selfDeviceId == NULL) {
        LOGE("Failed to get self deviceId from input data in sym auth.");
        return HC_ERR_JSON_GET;
    }
    uint32_t selfDeviceIdLen = HcStrlen(selfDeviceId);
    params->deviceIdSelf = (char *)HcMalloc(selfDeviceIdLen + 1, 0);
    if (params->deviceIdSelf == NULL) {
        LOGE("Failed to malloc for selfDeviceId.");
        return HC_ERR_ALLOC_MEMORY;
    }
    if (memcpy_s(params->deviceIdSelf, selfDeviceIdLen + 1, selfDeviceId, selfDeviceIdLen) != EOK) {
        LOGE("Copy for deviceIdSelf failed in sym auth.");
        return HC_ERR_MEMORY_COPY;
    }
    params->isoBaseParams.authIdSelf.length = params->devIdSelf.length + HcStrlen(params->deviceIdSelf);
    res = InitSingleParam(&params->isoBaseParams.authIdSelf, params->isoBaseParams.authIdSelf.length);
    if (res != HC_SUCCESS) {
        LOGE("InitSingleParam for authIdSelf failed, res: %d.", res);
        return res;
    }
    if (memcpy_s(params->isoBaseParams.authIdSelf.val, params->isoBaseParams.authIdSelf.length,
        params->devIdSelf.val, params->devIdSelf.length) != EOK) {
        LOGE("Failed to memcpy devIdSelf for authId in sym auth.");
        return HC_ERR_MEMORY_COPY;
    }
    if (memcpy_s(params->isoBaseParams.authIdSelf.val + params->devIdSelf.length,
        params->isoBaseParams.authIdSelf.length - params->devIdSelf.length,
        params->deviceIdSelf, selfDeviceIdLen) != EOK) {
        LOGE("Failed to memcpy deviceIdSelf for authId in sym auth.");
        return HC_ERR_MEMORY_COPY;
    }
    return HC_SUCCESS;
}

static int32_t SetChallenge(IsoAuthParams *params, const CJson *in)
{
    if (((uint32_t)params->credentialType & SYMMETRIC_CRED) == SYMMETRIC_CRED) {
        params->challenge.length = HcStrlen(KEY_INFO_PERSISTENT_TOKEN);
        params->challenge.val = (uint8_t *)HcMalloc(params->challenge.length, 0);
        if (params->challenge.val == NULL) {
            LOGE("Failed to malloc for challenge.");
            return HC_ERR_ALLOC_MEMORY;
        }
        if (memcpy_s(params->challenge.val, params->challenge.length, KEY_INFO_PERSISTENT_TOKEN,
            HcStrlen(KEY_INFO_PERSISTENT_TOKEN)) != EOK) {
            LOGE("Copy for challenge failed in sym auth.");
            return HC_ERR_MEMORY_COPY;
        }
        return HC_SUCCESS;
    }
    LOGE("Invalid credentialType: %d", params->credentialType);
    return HC_ERR_INVALID_PARAMS;
}

static int32_t GenerateAuthTokenForAccessory(const IsoAuthParams *params, Uint8Buff *outKey)
{
    uint8_t keyAliasVal[SHA256_LEN] = { 0 };
    Uint8Buff keyAlias = { keyAliasVal, SHA256_LEN };
    int32_t res = GetSymTokenManager()->generateKeyAlias(params->userIdSelf, (const char *)(params->devIdSelf.val),
        &keyAlias);
    if (res != HC_SUCCESS) {
        LOGE("Failed to generate key alias for authCode!");
        return res;
    }
    res = InitSingleParam(outKey, AUTH_TOKEN_SIZE_BYTE);
    if (res != HC_SUCCESS) {
        LOGE("Malloc for authToken failed, res: %d.", res);
        return res;
    }
    Uint8Buff userIdSelfBuff = {
        .val = (uint8_t *)(params->userIdSelf),
        .length = HcStrlen(params->userIdSelf)
    };
    res = params->isoBaseParams.loader->computeHkdf(&keyAlias, &userIdSelfBuff, &params->challenge, outKey, true);
    if (res != HC_SUCCESS) {
        LOGE("Failed to computeHkdf from authCode to authToken.");
        FreeAndCleanKey(outKey);
    }
    return res;
}

static int32_t GenerateTokenAliasForController(const IsoAuthParams *params, Uint8Buff *authTokenAlias)
{
    int32_t res = InitSingleParam(authTokenAlias, SHA256_LEN);
    if (res != HC_SUCCESS) {
        LOGE("Malloc for authToken alias failed, res: %d.", res);
        return res;
    }
    res = GetSymTokenManager()->generateKeyAlias(params->userIdPeer, (const char *)(params->devIdPeer.val),
        authTokenAlias);
    if (res != HC_SUCCESS) {
        LOGE("Failed to generate key alias for authToken.");
        HcFree(authTokenAlias->val);
        authTokenAlias->val = NULL;
    }
    return res;
}

int32_t AccountAuthGeneratePsk(IsoAuthParams *params)
{
    bool isTokenStored = true;
    Uint8Buff authToken = { NULL, 0 };
    int32_t res;
    if (params->localDevType == DEVICE_TYPE_ACCESSORY) {
        LOGI("Account sym auth for accessory.");
        isTokenStored = false;
        res = GenerateAuthTokenForAccessory(params, &authToken);
    } else {
        LOGI("Account sym auth for controller.");
        res = GenerateTokenAliasForController(params, &authToken);
    }
    if (res != HC_SUCCESS) {
        LOGE("Failed to generate token-related info, res = %d.", res);
        return res;
    }
    Uint8Buff pskBuf = { params->isoBaseParams.psk, PSK_SIZE };
    Uint8Buff seedBuf = { params->seed, sizeof(params->seed) };
    res = params->isoBaseParams.loader->computeHmac(&authToken, &seedBuf, &pskBuf, isTokenStored);
    FreeAndCleanKey(&authToken);
    if (res != HC_SUCCESS) {
        LOGE("ComputeHmac for psk failed, res: %d.", res);
    }
    return res;
}

int32_t InitIsoAuthParams(const CJson *in, IsoAuthParams *params, const AccountVersionInfo *verInfo)
{
    params->versionNo = verInfo->versionNo;
    int32_t res = HC_ERR_JSON_GET;
    if (GetIntFromJson(in, FIELD_AUTH_FORM, &params->authForm) != CLIB_SUCCESS) {
        LOGE("Failed to get authForm from json in sym auth.");
        goto CLEAN_UP;
    }
    if (GetIntFromJson(in, FIELD_CREDENTIAL_TYPE, &params->credentialType) != CLIB_SUCCESS) {
        LOGE("Failed to get credentialType from json in sym auth.");
        goto CLEAN_UP;
    }
    if (GetIntFromJson(in, FIELD_LOCAL_DEVICE_TYPE, &params->localDevType) != CLIB_SUCCESS) {
        LOGE("Failed to get localDevType from json in sym auth.");
        goto CLEAN_UP;
    }
    res = InitIsoBaseParams(&params->isoBaseParams);
    if (res != HC_SUCCESS) {
        LOGE("InitIsoBaseParams failed, res: %x.", res);
        goto CLEAN_UP;
    }
    res = FillUserId(in, params);
    if (res != HC_SUCCESS) {
        LOGE("Failed to fill userId info, res = %d.", res);
        goto CLEAN_UP;
    }
    res = FillPayload(in, params);
    if (res != HC_SUCCESS) {
        LOGE("Failed to fill payload info, res = %d.", res);
        goto CLEAN_UP;
    }
    if (params->localDevType == DEVICE_TYPE_ACCESSORY) {
        res = SetChallenge(params, in);
        if (res != HC_SUCCESS) {
            LOGE("SetChallenge failed, res = %d.", res);
            goto CLEAN_UP;
        }
    }
    return HC_SUCCESS;
CLEAN_UP:
    DestroyIsoAuthParams(params);
    return res;
}

void DestroyIsoAuthParams(IsoAuthParams *params)
{
    LOGI("Destroy iso auth params begin.");
    if (params == NULL) {
        return;
    }
    DestroyIsoBaseParams(&params->isoBaseParams);

    HcFree(params->challenge.val);
    params->challenge.val = NULL;

    HcFree(params->userIdSelf);
    params->userIdSelf = NULL;

    HcFree(params->userIdPeer);
    params->userIdPeer = NULL;

    HcFree(params->devIdSelf.val);
    params->devIdSelf.val = NULL;

    HcFree(params->devIdPeer.val);
    params->devIdPeer.val = NULL;

    HcFree(params->deviceIdSelf);
    params->deviceIdSelf = NULL;

    HcFree(params->deviceIdPeer);
    params->deviceIdPeer = NULL;
}

static int32_t GetPayloadValue(IsoAuthParams *params, const CJson *in)
{
    const char *devIdPeerHex = GetStringFromJson(in, FIELD_DEV_ID);
    if (devIdPeerHex == NULL) {
        LOGE("Get peer devId hex failed.");
        return HC_ERR_JSON_GET;
    }
    uint32_t devIdPeerHexLen = HcStrlen(devIdPeerHex);
    // DevId is string, the id from phone is byte. For both cases, apply one more bit for '\0'.
    params->devIdPeer.val = (uint8_t *)HcMalloc(devIdPeerHexLen / BYTE_TO_HEX_OPER_LENGTH + 1, 0);
    if (params->devIdPeer.val == NULL) {
        LOGE("Failed to malloc for peer devId.");
        return HC_ERR_ALLOC_MEMORY;
    }
    params->devIdPeer.length = devIdPeerHexLen / BYTE_TO_HEX_OPER_LENGTH;
    if (HexStringToByte(devIdPeerHex, params->devIdPeer.val, params->devIdPeer.length) != CLIB_SUCCESS) {
        LOGE("Failed to convert peer devId.");
        return HC_ERR_CONVERT_FAILED;
    }
    const char *deviceIdPeer = GetStringFromJson(in, FIELD_DEVICE_ID);
    if (deviceIdPeer == NULL) {
        LOGE("Get peer deviceId failed.");
        return HC_ERR_JSON_GET;
    }
    uint32_t deviceIdPeerLen = HcStrlen(deviceIdPeer);
    params->deviceIdPeer = (char *)HcMalloc(deviceIdPeerLen + 1, 0);
    if (params->deviceIdPeer == NULL) {
        LOGE("Failed to malloc for peer deviceId.");
        return HC_ERR_ALLOC_MEMORY;
    }
    if (memcpy_s(params->deviceIdPeer, deviceIdPeerLen + 1, deviceIdPeer, deviceIdPeerLen) != EOK) {
        LOGE("Failed to copy peer deviceId.");
        return HC_ERR_MEMORY_COPY;
    }
    return HC_SUCCESS;
}

static int32_t ExtractPeerAuthId(IsoAuthParams *params, const CJson *in)
{
    const char *payloadHex = GetStringFromJson(in, FIELD_PAYLOAD);
    if (payloadHex == NULL) {
        LOGE("Get payloadHex peer from json failed.");
        return HC_ERR_JSON_GET;
    }
    int32_t res = InitSingleParam(&(params->isoBaseParams.authIdPeer), HcStrlen(payloadHex) / BYTE_TO_HEX_OPER_LENGTH);
    if (res != HC_SUCCESS) {
        LOGE("InitSingleParam for payload peer failed, res: %d.", res);
        return res;
    }
    if (HexStringToByte(payloadHex, params->isoBaseParams.authIdPeer.val,
        params->isoBaseParams.authIdPeer.length) != CLIB_SUCCESS) {
        LOGE("Convert payloadPeer from hex string to byte failed.");
        return HC_ERR_CONVERT_FAILED;
    }
    return HC_SUCCESS;
}

int32_t ExtractAndVerifyPayload(IsoAuthParams *params, const CJson *in)
{
    int32_t res = ExtractPeerAuthId(params, in);
    if (res != HC_SUCCESS) {
        LOGE("ExtractPeerAuthId failed, res: %d.", res);
        return res;
    }
    res = GetPayloadValue(params, in);
    if (res != HC_SUCCESS) {
        LOGE("GetPayloadValue failed, res: %d.", res);
        return res;
    }
    uint32_t deviceIdPeerLen = HcStrlen(params->deviceIdPeer);
    uint32_t len = params->devIdPeer.length + deviceIdPeerLen;
    char *combineString = (char *)HcMalloc(len, 0);
    if (combineString == NULL) {
        LOGE("Failed to malloc for combineString.");
        return HC_ERR_ALLOC_MEMORY;
    }
    if (memcpy_s(combineString, len, params->devIdPeer.val, params->devIdPeer.length) != EOK) {
        LOGE("Failed to copy peer devId.");
        HcFree(combineString);
        return HC_ERR_MEMORY_COPY;
    }
    if (memcpy_s(combineString + params->devIdPeer.length, len - params->devIdPeer.length, params->deviceIdPeer,
        deviceIdPeerLen) != EOK) {
        LOGE("Failed to copy peer deviceId.");
        HcFree(combineString);
        return HC_ERR_MEMORY_COPY;
    }
    if (memcmp(combineString, params->isoBaseParams.authIdPeer.val, len) != 0) {
        LOGE("Payload is not equal.");
        HcFree(combineString);
        return HC_ERR_MEMORY_COMPARE;
    }
    HcFree(combineString);
    return HC_SUCCESS;
}

int32_t AuthIsoSendFinalToOut(IsoAuthParams *params, CJson *out)
{
    CJson *sendToSelf = CreateJson();
    if (sendToSelf == NULL) {
        LOGE("Create sendToSelf json failed.");
        return HC_ERR_JSON_CREATE;
    }
    if (AddByteToJson(sendToSelf, FIELD_SESSION_KEY,
        params->isoBaseParams.sessionKey.val, params->isoBaseParams.sessionKey.length) != CLIB_SUCCESS) {
        LOGE("Add sessionKey to json failed.");
        goto CLEAN_UP;
    }
    if (AddStringToJson(sendToSelf, FIELD_USER_ID, params->userIdPeer) != CLIB_SUCCESS) {
        LOGE("Add userIdPeer to json failed.");
        goto CLEAN_UP;
    }
    if (AddStringToJson(sendToSelf, FIELD_DEVICE_ID, params->deviceIdPeer) != CLIB_SUCCESS) {
        LOGE("Add deviceIdPeer to json failed.");
        goto CLEAN_UP;
    }
    if (AddIntToJson(sendToSelf, FIELD_CREDENTIAL_TYPE, params->credentialType) != CLIB_SUCCESS) {
        LOGE("Add credentialType to json failed.");
        goto CLEAN_UP;
    }
    if (AddStringToJson(sendToSelf, FIELD_DEV_ID, (char *)params->devIdPeer.val) != CLIB_SUCCESS) {
        LOGE("Add devIdPeer to json failed.");
        goto CLEAN_UP;
    }
    if (AddObjToJson(out, FIELD_SEND_TO_SELF, sendToSelf) != CLIB_SUCCESS) {
        LOGE("Add sendToSelf to json failed.");
        goto CLEAN_UP;
    }
    FreeJson(sendToSelf);
    FreeAndCleanKey(&(params->isoBaseParams.sessionKey));
    return HC_SUCCESS;
CLEAN_UP:
    FreeJson(sendToSelf);
    FreeAndCleanKey(&(params->isoBaseParams.sessionKey));
    return HC_ERR_JSON_ADD;
}
