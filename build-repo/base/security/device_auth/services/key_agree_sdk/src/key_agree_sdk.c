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

#include "key_agree_sdk.h"

#include "hc_log.h"
#include "key_agree_session.h"
#include "key_agree_session_manager.h"
#include "protocol_common.h"

#define MIN_SHARED_SECRET_LENGTH 6
#define MAX_SHARED_SECRET_LENGTH 64
#define MIN_DEVICE_ID_LENGTH 16
#define MAX_DEVICE_ID_LENGTH 256
#define MIN_MESSAGE_LENGTH 2048
#define MAX_MESSAGE_LENGTH 4096
#define MIN_SESSIONKEY_LENGTH 64
#define MAX_SESSIONKEY_LENGTH 1024
#define MAX_EXTRAS_LENGTH 4096

static int32_t CheckKeyAgreeStartSessionParams(const KeyAgreeBlob *sharedSecret,
    const KeyAgreeBlob *deviceId, const char *extras)
{
    if ((sharedSecret->length < MIN_SHARED_SECRET_LENGTH) || (sharedSecret->length > MAX_SHARED_SECRET_LENGTH)) {
        LOGE("Invalid length of sharedSecret!");
        return HC_ERR_INVALID_PARAMS;
    }
    if (strnlen((const char *)(sharedSecret->data), sharedSecret->length) > sharedSecret->length + 1) {
        LOGE("Length of sharedSecret check fail!");
        return HC_ERR_INVALID_PARAMS;
    }
    if ((deviceId->length < MIN_DEVICE_ID_LENGTH) || (deviceId->length > MAX_DEVICE_ID_LENGTH)) {
        LOGE("Invalid length of deviceId!");
        return HC_ERR_INVALID_PARAMS;
    }
    if (strnlen((const char *)(deviceId->data), deviceId->length) > deviceId->length + 1) {
        LOGE("Length of deviceId check fail!");
        return HC_ERR_INVALID_PARAMS;
    }
    if (((extras != NULL) && (HcStrlen(extras) > MAX_EXTRAS_LENGTH))) {
        LOGE("Invalid length of extras!");
        return HC_ERR_INVALID_PARAMS;
    }
    return HC_SUCCESS;
}

static void BuildErrorMessage(KeyAgreeResult errorCode, KeyAgreeBlob *out)
{
    CJson *outJson = CreateJson();
    if (outJson == NULL) {
        LOGE("Create json failed!");
        return;
    }
    int32_t res = AddIntToJson(outJson, FIELD_SDK_ERROR_CODE, errorCode);
    if (res != HC_SUCCESS) {
        LOGE("Add errorCode failed, res: %d.", res);
        FreeJson(outJson);
        return;
    }
    char *returnStr = PackJsonToString(outJson);
    FreeJson(outJson);
    if (returnStr == NULL) {
        LOGE("Pack json to string failed!");
        return;
    }
    uint32_t returnStrLen = HcStrlen(returnStr);
    if (memcpy_s(out->data, out->length, returnStr, returnStrLen + 1) != EOK) {
        LOGE("Memcpy for out json failed.");
    }
    out->length = returnStrLen + 1;
    FreeJsonString(returnStr);
}

static bool IsErrorMessage(const KeyAgreeBlob *in)
{
    int32_t errorCode;
    CJson *inJson = CreateJsonFromString((const char *)(in->data));
    if (inJson == NULL) {
        LOGE("Create json failed!");
        return true;
    }
    if (GetIntFromJson(inJson, FIELD_SDK_ERROR_CODE, &errorCode) != HC_SUCCESS) {
        LOGE("There is no field named errorCode!");
        FreeJson(inJson);
        return true;
    }
    FreeJson(inJson);
    if (errorCode != KEYAGREE_SUCCESS) {
        LOGE("Peer is onError, errorCode: %d.!", errorCode);
        return true;
    }
    return false;
}

static int32_t InitKeyAgreeStartSessionParams(SpekeSession *spekeSession, const KeyAgreeBlob *sharedSecret,
    const KeyAgreeBlob *deviceId, const char *extras)
{
    int32_t res = KEYAGREE_SUCCESS;
    do {
        if (InitSingleParam(&(spekeSession->sharedSecret), sharedSecret->length) != HC_SUCCESS) {
            LOGE("Init sharedSecret failed!");
            res = KEYAGREE_START_INIT_PARAMS_SHAREDSECRET_FAIL;
            break;
        }
        if (memcpy_s(spekeSession->sharedSecret.val, spekeSession->sharedSecret.length,
            sharedSecret->data, sharedSecret->length) != HC_SUCCESS) {
            LOGE("Memcpy for sharedSecret code failed.");
            res = KEYAGREE_START_MEMORY_COPY_SHAREDSECRET_FAIL;
            break;
        }
        if (InitSingleParam(&(spekeSession->deviceId), deviceId->length) != HC_SUCCESS) {
            LOGE("Init deviceId failed!");
            res = KEYAGREE_START_MEMORY_COPY_DEVID_FAIL;
            break;
        }
        if (memcpy_s(spekeSession->deviceId.val, spekeSession->deviceId.length,
            deviceId->data, deviceId->length) != HC_SUCCESS) {
            LOGE("Memcpy for deviceId failed.");
            res = KEYAGREE_START_MEMORY_COPY_DEVID_FAIL;
            break;
        }
        if (extras != NULL) {
            LOGI("This field named extras is not used in this version!");
        }
    } while (0);
    if (res != HC_SUCCESS) {
        FreeAndCleanKey(&spekeSession->sharedSecret);
        FreeAndCleanKey(&spekeSession->deviceId);
        HcFree(spekeSession->extras);
        spekeSession->extras = NULL;
    }
    return res;
}

KeyAgreeResult KeyAgreeInitSession(KeyAgreeSession *session, KeyAgreeProtocol protocol, KeyAgreeType type)
{
    LOGI("Key agree init begin!");
    if (session == NULL) {
        LOGE("Invalid input params.");
        return KEYAGREE_INIT_BAD_PARAMS;
    }
    SpekeSession *spekeSession = CreateSpekeSession();
    if (spekeSession == NULL) {
        LOGE("Init spekeSession fail!");
        return KEYAGREE_INIT_CREATE_SESSION_FAIL;
    }
    if (spekeSession->checkAndInitProtocol(spekeSession, protocol) != HC_SUCCESS) {
        LOGE("This protocol is not supported!");
        DestroySpekeSession(spekeSession);
        return KEYAGREE_INIT_NOT_SUPPORTED;
    }
    spekeSession->sessionId = session->sessionId;
    spekeSession->protocol = protocol;
    spekeSession->keyAgreeType = type;
    if (protocol != KEYAGREE_PROTOCOL_ANY) {
        spekeSession->versionInfo.versionStatus = VERSION_DECIDED;
        if (spekeSession->initSpekeSession(spekeSession, protocol) != HC_SUCCESS) {
            LOGE("Init protocol session fail!");
            DestroySpekeSession(spekeSession);
            return KEYAGREE_INIT_INIT_SESSION_FAIL;
        }
    } else {
        if (type == KEYAGREE_TYPE_CLIENT) {
            spekeSession->versionInfo.versionStatus = INITIAL;
        } else {
            spekeSession->versionInfo.versionStatus = VERSION_CONFIRM;
        }
    }
    if (GetManagerInstance()->addSession(session, spekeSession) != HC_SUCCESS) {
        LOGE("Add session fail!");
        DestroySpekeSession(spekeSession);
        return KEYAGREE_INIT_ADD_SESSION_FAIL;
    }
    LOGI("Init keyAgree session successfully!");
    return KEYAGREE_SUCCESS;
}

KeyAgreeResult KeyAgreeStartSession(KeyAgreeSession *session, const KeyAgreeBlob *sharedSecret,
    const KeyAgreeBlob *deviceId, const char *extras)
{
    LOGI("Key agree start session begin!");
    if ((sharedSecret == NULL) || (deviceId == NULL) || (session == NULL)) {
        LOGE("Invalid input params.");
        return KEYAGREE_START_BAD_PARAMS;
    }
    if (CheckKeyAgreeStartSessionParams(sharedSecret, deviceId, extras) != HC_SUCCESS) {
        LOGE("Invalid input length!");
        return KEYAGREE_START_INVAILD_LEN;
    }
    SpekeSession *spekeSession = GetManagerInstance()->getSession(session);
    if (spekeSession == NULL) {
        LOGE("GetSession fail!");
        return KEYAGREE_START_GET_SESSION_FAIL;
    }
    int32_t res = InitKeyAgreeStartSessionParams(spekeSession, sharedSecret, deviceId, extras);
    if (res != HC_SUCCESS) {
        LOGE("Init params fail!");
        return res;
    }
    LOGI("KeyAgree start successfully!");
    return res;
}

KeyAgreeResult KeyAgreeGenerateNextMessage(KeyAgreeSession *session, const KeyAgreeBlob *messageReceived,
    KeyAgreeBlob *messageToTransmit)
{
    LOGI("Key agree generate next message begin!");
    if ((session == NULL) || (messageToTransmit == NULL)) {
        LOGE("Invalid session or messageToTransmit!");
        return KEYAGREE_PROCESS_BAD_PARAMS;
    }
    if ((messageToTransmit->length < MIN_MESSAGE_LENGTH) || (messageToTransmit->length > MAX_MESSAGE_LENGTH)) {
        LOGE("Invalid length of messageToTransmit!");
        return KEYAGREE_PROCESS_INVAILD_LEN;
    }
    if (messageReceived != NULL && messageReceived->data != NULL &&
        strnlen((const char *)(messageReceived->data), messageReceived->length) > messageReceived->length  + 1) {
        LOGE("Length of messageToTransmit check fail!");
        return KEYAGREE_PROCESS_INVAILD_LEN;
    }
    if (messageReceived != NULL && messageReceived->data != NULL && IsErrorMessage(messageReceived)) {
        LOGE("Peer is onError!");
        return KEYAGREE_PROCESS_PEER_IS_ONERROR;
    }
    SpekeSession *spekeSession = GetManagerInstance()->getSession(session);
    if (spekeSession == NULL) {
        LOGE("GetSession fail!");
        return KEYAGREE_PROCESS_GET_SESSION_FAIL;
    }
    if ((messageReceived == NULL) && (spekeSession->step != STEP_INIT)) {
        LOGE("This is not first step, messageReceived is null!");
        return KEYAGREE_PROCESS_BAD_PARAMS;
    }
    if (spekeSession->versionInfo.versionStatus != VERSION_DECIDED) {
        LOGI("Protocol is not decided, need protocol agree!");
        if (spekeSession->processProtocolAgree(spekeSession, messageReceived, messageToTransmit) != HC_SUCCESS) {
            LOGE("Protocol agree fail!");
            BuildErrorMessage(KEYAGREE_FAIL, messageToTransmit);
            return KEYAGREE_PROCESS_PROTOCOL_AGREE_FAIL;
        }
    } else {
        LOGI("Protocol decided, process key agree!");
        if (spekeSession->processSession(spekeSession, messageReceived, messageToTransmit) != HC_SUCCESS) {
            LOGE("Protocol agree fail!");
            BuildErrorMessage(KEYAGREE_FAIL, messageToTransmit);
            return KEYAGREE_PROCESS_PROTOCOL_SESSION_FAIL;
        }
    }
    LOGI("KeyAgree generate next message successfully!");
    return KEYAGREE_SUCCESS;
}

KeyAgreeResult KeyAgreeIsFinish(KeyAgreeSession *session)
{
    LOGI("Key agree is finish start!");
    if (session == NULL) {
        LOGE("Invalid input params.");
        return KEYAGREE_IS_FINSIH_BAD_PARAMS;
    }
    KeyAgreeSessionManager *sessionManager = GetManagerInstance();
    if (sessionManager == NULL) {
        LOGE("Get session mgr fail!");
        return KEYAGREE_IS_FINSIH_GET_SESSION_MGR_FAIL;
    }
    SpekeSession *spekeSession = sessionManager->getSession(session);
    if (spekeSession == NULL) {
        LOGE("GetSession fail!");
        return KEYAGREE_IS_FINSIH_GET_SESSION_FAIL;
    }
    if (!spekeSession->isFinish) {
        LOGE("Keg agree is not finish!");
        return KEYAGREE_IS_FINSIH_NOT_FINSIH;
    }
    LOGI("Keg agree is finish!");
    return KEYAGREE_SUCCESS;
}

KeyAgreeResult KeyAgreeGetResult(KeyAgreeSession *session, KeyAgreeBlob *sessionKey)
{
    LOGI("Key agree get result start!");
    if ((session == NULL) || (sessionKey == NULL)) {
        LOGE("Invalid session or sessionKey!");
        return KEYAGREE_GET_RESULT_BAD_PARAMS;
    }
    if ((sessionKey->length < MIN_SESSIONKEY_LENGTH) || (sessionKey->length > MAX_SESSIONKEY_LENGTH)) {
        LOGE("Invalid length of sessionKey!");
        return KEYAGREE_GET_RESULT_INVAILD_LEN;
    }
    KeyAgreeSessionManager *sessionManager = GetManagerInstance();
    if (sessionManager == NULL) {
        LOGE("Get session mgr fail!");
        return KEYAGREE_GET_RESULT_GET_SESSION_MGR_FAIL;
    }
    SpekeSession *spekeSession = sessionManager->getSession(session);
    if (spekeSession == NULL) {
        LOGE("GetSession fail!");
        return KEYAGREE_GET_RESULT_GET_SESSION_FAIL;
    }
    if (!spekeSession->isFinish) {
        LOGE("Keg agree is not finish, sessionKey is not generate!");
        return KEYAGREE_GET_RESULT_NOT_FINSIH;
    }
    if (memcpy_s(sessionKey->data, spekeSession->baseParam.sessionKey.length + 1,
        spekeSession->baseParam.sessionKey.val, spekeSession->baseParam.sessionKey.length + 1) != EOK) {
        LOGE("Memcpy for seesionKey failed.");
        return KEYAGREE_GET_RESULT_MEMCPY_FAILED;
    }
    sessionKey->length = spekeSession->baseParam.sessionKey.length;
    LOGI("Keg agree is finish, return seesionKey.");
    return KEYAGREE_SUCCESS;
}

void KeyAgreeFreeSession(KeyAgreeSession *session)
{
    LOGI("Key agree free session start!");
    if (session == NULL) {
        LOGE("Invalid params!");
        return;
    }
    KeyAgreeSessionManager *sessionManager = GetManagerInstance();
    if (sessionManager == NULL) {
        LOGE("Get session mgr fail!");
        return;
    }
    SpekeSession *spekeSession = sessionManager->getSession(session);
    if (spekeSession == NULL) {
        LOGE("GetSession fail!");
        return;
    }
    if (sessionManager == NULL || sessionManager->deleteSession(session) != HC_SUCCESS) {
        LOGE("Delete session fail!");
        return;
    }
    DestroySpekeSession(spekeSession);
}