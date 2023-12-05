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

#include "key_agree_session.h"

#include "hc_log.h"
#include "key_agree_sdk.h"
#include "pake_v2_protocol_common.h"
#include "protocol_common.h"

#define MAX_AUTH_ID_LEN 256
#define MAJOR_VERSION_NO 1
#define TMP_VERSION_STR_LEN 30
#define DEC 10
#define MIN_SHAREDSECRET_LEN 4
#define MAX_SHAREDSECRET_LEN 1024

typedef int32_t (*PackageExtraData)(const PakeBaseParams *baseParams, CJson *payload);

const static ProtocolPrimeMod g_protocolAlgorithm[] = {
    EC_SPEKE_X25519,
    EC_SPEKE_P256,
    DL_SPEKE_MOD_384,
    DL_SPEKE_MOD_256
};

const static KeyAgreeProtocol g_keyAgreeProtocol[] = {
    KEYAGREE_PROTOCOL_EC_SPEKE_X25519,
    KEYAGREE_PROTOCOL_EC_SPEKE_P256,
    KEYAGREE_PROTOCOL_DL_SPEKE_384,
    KEYAGREE_PROTOCOL_DL_SPEKE_256
};

VersionStruct g_defaultVersion = { 1, 0, 0 };

static int32_t GetIdPeer(const CJson *in, const char *peerIdKey, const Uint8Buff *authIdSelf, Uint8Buff *authIdPeer)
{
    const char *authIdStr = GetStringFromJson(in, peerIdKey);
    if (authIdStr == NULL) {
        LOGE("Get peer id from json failed.");
        return HC_ERR_JSON_GET;
    }
    uint32_t authIdLen = HcStrlen(authIdStr) / BYTE_TO_HEX_OPER_LENGTH;
    if (authIdLen == 0 || authIdLen > MAX_AUTH_ID_LEN) {
        LOGE("Invalid authIdPeerLen: %u.", authIdLen);
        return HC_ERR_INVALID_LEN;
    }
    int32_t res = InitSingleParam(authIdPeer, authIdLen);
    if (res != HC_SUCCESS) {
        LOGE("InitSingleParam for peer authId failed, res: %d.", res);
        return res;
    }
    if (HexStringToByte(authIdStr, authIdPeer->val, authIdPeer->length) != HC_SUCCESS) {
        LOGE("HexStringToByte for authIdPeer failed.");
        return HC_ERR_CONVERT_FAILED;
    }
    if ((authIdSelf->length == authIdPeer->length) &&
        memcmp(authIdSelf->val, authIdPeer->val, authIdSelf->length) == 0) {
        LOGE("Peer id can not be equal to self id.");
        return HC_ERR_INVALID_PARAMS;
    }
    return HC_SUCCESS;
}

static uint64_t GetKeyAgreeSdkPakeAlgInProtocol()
{
    uint64_t algInProtocol = SPEKE_MOD_NONE;
#ifdef P2P_PAKE_DL_PRIME_LEN_256
    algInProtocol |= DL_SPEKE_MOD_256;
#endif
#ifdef P2P_PAKE_DL_PRIME_LEN_384
    algInProtocol |= DL_SPEKE_MOD_384;
#endif
#ifdef P2P_PAKE_EC_PRIME_P256
    algInProtocol |= EC_SPEKE_P256;
#endif
#ifdef P2P_PAKE_EC_PRIME_X25519
    algInProtocol |= EC_SPEKE_X25519;
#endif
    return algInProtocol;
}

static const char *GetSlice(char *str, char delim, int32_t *nextIdx)
{
    if (str == NULL) {
        LOGE("Input str is NULL!");
        return str;
    }
    uint32_t len = HcStrlen(str);
    for (uint32_t i = 0; i < len; i++) {
        if (str[i] == delim) {
            *nextIdx = *nextIdx + i + 1;
            str[i] = '\0';
            return str;
        }
    }
    return str;
}

static bool IsVersionEqual(VersionStruct *src, VersionStruct *des)
{
    if ((src->first == des->first) && (src->second == des->second) && (src->third == des->third)) {
        return true;
    }
    return false;
}

static int32_t StringToVersion(const char* verStr, VersionStruct* version)
{
    CHECK_PTR_RETURN_ERROR_CODE(version, "version");
    CHECK_PTR_RETURN_ERROR_CODE(verStr, "verStr");

    const char *subVer = NULL;
    int32_t nextIdx = 0;
    uint64_t len = HcStrlen(verStr);
    char *verStrTmp = (char *)HcMalloc(len + 1, 0);
    if (verStrTmp == NULL) {
        LOGE("Malloc for verStrTmp failed.");
        return HC_ERR_ALLOC_MEMORY;
    }
    if (memcpy_s(verStrTmp, len + 1, verStr, len) != EOK) {
        LOGE("Memcpy for verStrTmp failed.");
        HcFree(verStrTmp);
        return HC_ERR_MEMORY_COPY;
    }
    subVer = GetSlice(verStrTmp, '.', &nextIdx);
    if (subVer == NULL) {
        goto CLEAN_UP;
    }
    version->first = (uint64_t)strtoul(subVer, NULL, DEC);
    subVer = GetSlice(verStrTmp + nextIdx, '.', &nextIdx);
    if (subVer == NULL) {
        goto CLEAN_UP;
    }
    version->second = (uint64_t)strtoul(subVer, NULL, DEC);
    subVer = GetSlice(verStrTmp + nextIdx, '.', &nextIdx);
    if (subVer == NULL) {
        goto CLEAN_UP;
    }
    version->third = (uint64_t)strtoul(subVer, NULL, DEC);
    HcFree(verStrTmp);
    return HC_SUCCESS;
CLEAN_UP:
    LOGE("GetSlice failed.");
    HcFree(verStrTmp);
    return HC_ERROR;
}

static int32_t VersionToString(const VersionStruct *version, char *verStr, uint32_t len)
{
    CHECK_PTR_RETURN_ERROR_CODE(version, "version");
    CHECK_PTR_RETURN_ERROR_CODE(verStr, "verStr");

    char tmpStr[TMP_VERSION_STR_LEN] = { 0 };
    if (sprintf_s(tmpStr, TMP_VERSION_STR_LEN, "%lld.%lld.%lld",
        version->first, version->second, version->third) <= 0) {
        LOGE("Convert version struct to string failed.");
        return HC_ERR_CONVERT_FAILED;
    }
    uint32_t tmpStrLen = HcStrlen(tmpStr);
    if (len < tmpStrLen + 1) {
        LOGE("The length of verStr is too short, len: %u.", len);
        return HC_ERR_INVALID_LEN;
    }
    if (memcpy_s(verStr, len, tmpStr, tmpStrLen + 1) != 0) {
        LOGE("Memcpy for verStr failed.");
        return HC_ERR_MEMORY_COPY;
    }
    return HC_SUCCESS;
}

void GetMaxVersion(uint64_t algInProtocol, VersionStruct *version)
{
    version->first = MAJOR_VERSION_NO;
    version->second = algInProtocol;
    version->third = 0;
}

int32_t AddVersionToJson(CJson *jsonObj, const VersionStruct *maxVer)
{
    CHECK_PTR_RETURN_ERROR_CODE(jsonObj, "jsonObj");
    CHECK_PTR_RETURN_ERROR_CODE(maxVer, "maxVer");

    char maxStr[TMP_VERSION_STR_LEN] = { 0 };
    int32_t maxRet = VersionToString(maxVer, maxStr, TMP_VERSION_STR_LEN);
    if (maxRet != HC_SUCCESS) {
        LOGE("MaxRet for version to string failed.");
        return HC_ERROR;
    }
    CJson* version = CreateJson();
    if (version == NULL) {
        LOGE("CreateJson for version failed.");
        return HC_ERR_JSON_CREATE;
    }
    if (AddStringToJson(version, FIELD_SDK_CURRENT_VERSION, maxStr) != HC_SUCCESS) {
        LOGE("Add max version to json failed.");
        FreeJson(version);
        return HC_ERR_JSON_ADD;
    }
    if (AddObjToJson(jsonObj, FIELD_SDK_VERSION, version) != HC_SUCCESS) {
        LOGE("Add version object to json failed.");
        FreeJson(version);
        return HC_ERR_JSON_ADD;
    }
    FreeJson(version);
    return HC_SUCCESS;
}

int32_t GetVersionFromJson(const CJson* jsonObj, VersionStruct *maxVer)
{
    CHECK_PTR_RETURN_ERROR_CODE(jsonObj, "jsonObj");
    CHECK_PTR_RETURN_ERROR_CODE(maxVer, "maxVer");

    const char *maxStr = GetStringFromJson(jsonObj, FIELD_SDK_CURRENT_VERSION);
    CHECK_PTR_RETURN_ERROR_CODE(maxStr, "maxStr");

    int32_t maxRet = StringToVersion(maxStr, maxVer);
    if (maxRet != HC_SUCCESS) {
        LOGE("Convert version string to struct failed.ionInfo");
        return HC_ERROR;
    }
    return HC_SUCCESS;
}

int32_t NegotiateVersion(VersionStruct *curVersionPeer, VersionStruct *curVersionSelf)
{
    if (IsVersionEqual(curVersionPeer, &g_defaultVersion)) {
        curVersionSelf->first = g_defaultVersion.first;
        curVersionSelf->second = g_defaultVersion.second;
        curVersionSelf->third = g_defaultVersion.third;
        return HC_SUCCESS;
    }
    curVersionSelf->second = curVersionSelf->second & curVersionPeer->second;
    if (curVersionSelf->second == 0) {
        LOGE("Unsupported version!");
        return HC_ERR_UNSUPPORTED_VERSION;
    }
    return HC_SUCCESS;
}

void InitVersionInfo(VersionInfo *versionInfo)
{
    uint64_t algInProtocol = GetKeyAgreeSdkPakeAlgInProtocol();
    GetMaxVersion(algInProtocol, &(versionInfo->curVersion));
    versionInfo->versionStatus = INITIAL;
}

int32_t GetAuthIdPeerFromPayload(const CJson *in, const Uint8Buff *authIdSelf, Uint8Buff *authIdPeer)
{
    const CJson *payload = GetObjFromJson(in, FIELD_SDK_PAYLOAD);
    if (payload == NULL) {
        LOGE("Not have payload.");
        return HC_ERR_JSON_GET;
    }
    int32_t res = GetIdPeer(payload, FIELD_PEER_AUTH_ID, authIdSelf, authIdPeer);
    if (res != HC_SUCCESS) {
        LOGE("GetIdPeer failed, res: %d.", res);
    }
    return res;
}

static int32_t ParsePakeServerConfirmMessage(PakeBaseParams *baseParams, const CJson *in)
{
    int32_t res = GetByteFromJson(in, FIELD_KCF_DATA, baseParams->kcfDataPeer.val,
        baseParams->kcfDataPeer.length);
    if (res != HC_SUCCESS) {
        LOGE("Get kcfDataPeer failed, res: %d.", res);
        return HC_ERR_JSON_GET;
    }
    return HC_SUCCESS;
}

static int32_t PackagePakeServerConfirmData(const PakeBaseParams *baseParams, CJson *payload)
{
    int32_t res = AddByteToJson(payload, FIELD_KCF_DATA, baseParams->kcfData.val, baseParams->kcfData.length);
    if (res != HC_SUCCESS) {
        LOGE("Add kcfData failed, res: %d.", res);
        return HC_ERR_JSON_ADD;
    }
    return HC_SUCCESS;
}

static int32_t GetDasEpkPeerFromJson(PakeBaseParams *baseParams, const CJson *in)
{
    const char *epkPeerHex = GetStringFromJson(in, FIELD_EPK);
    if (epkPeerHex == NULL) {
        LOGE("Get epkPeerHex failed.");
        return HC_ERR_JSON_GET;
    }
    int32_t res = InitSingleParam(&(baseParams->epkPeer), HcStrlen(epkPeerHex) / BYTE_TO_HEX_OPER_LENGTH);
    if (res != HC_SUCCESS) {
        LOGE("InitSingleParam for epkPeer failed, res: %d.", res);
        return res;
    }
    res = HexStringToByte(epkPeerHex, baseParams->epkPeer.val, baseParams->epkPeer.length);
    if (res != HC_SUCCESS) {
        LOGE("Convert epkPeer from hex string to byte failed, res: %d.", res);
        return HC_ERR_CONVERT_FAILED;
    }
    return HC_SUCCESS;
}

static int32_t ParsePakeClientConfirmMessage(PakeBaseParams *baseParams, const CJson *in)
{
    int32_t res = GetByteFromJson(in, FIELD_KCF_DATA, baseParams->kcfDataPeer.val, baseParams->kcfDataPeer.length);
    if (res != HC_SUCCESS) {
        LOGE("Get kcfDataPeer failed, res: %d.", res);
        return HC_ERR_JSON_GET;
    }
    res = GetDasEpkPeerFromJson(baseParams, in);
    if (res != HC_SUCCESS) {
        LOGE("GetDasEpkPeerFromJson failed, res: %d.", res);
    }
    return res;
}

static int32_t PackagePakeClientConfirmData(const PakeBaseParams *baseParams, CJson *payload)
{
    int32_t res = AddByteToJson(payload, FIELD_EPK, baseParams->epkSelf.val, baseParams->epkSelf.length);
    if (res != HC_SUCCESS) {
        LOGE("Add epkSelf failed, res: %d.", res);
        return HC_ERR_JSON_ADD;
    }
    res = AddByteToJson(payload, FIELD_KCF_DATA, baseParams->kcfData.val, baseParams->kcfData.length);
    if (res != HC_SUCCESS) {
        LOGE("Add kcfData failed, res: %d.", res);
        return HC_ERR_JSON_ADD;
    }
    res = AddByteToJson(payload, FIELD_PEER_AUTH_ID, baseParams->idSelf.val, baseParams->idSelf.length);
    if (res != HC_SUCCESS) {
        LOGE("Add idSelf failed, res: %d.", res);
        return HC_ERR_JSON_ADD;
    }
    return HC_SUCCESS;
}

static int32_t ParsePakeResponseMessage(PakeBaseParams *baseParams, const CJson *in)
{
    int32_t res = GetByteFromJson(in, FIELD_SALT, baseParams->salt.val, baseParams->salt.length);
    if (res != HC_SUCCESS) {
        LOGE("Get salt failed, res: %d.", res);
        return HC_ERR_JSON_GET;
    }
    res = GetDasEpkPeerFromJson(baseParams, in);
    if (res != HC_SUCCESS) {
        LOGE("GetDasEpkPeerFromJson failed, res: %d.", res);
    }
    return res;
}

static int32_t FillExtraData(SpekeSession *spekeSession, const CJson *jsonMessage)
{
    const CJson *version = GetObjFromJson(jsonMessage, FIELD_SDK_VERSION);
    if (version == NULL) {
        LOGE("Get version from json failed.");
        return HC_ERR_JSON_GET;
    }
    char *versionStr = PackJsonToString(version);
    if (versionStr == NULL) {
        LOGE("Pack json to versionStr failed.");
        return HC_ERR_PACKAGE_JSON_TO_STRING_FAIL;
    }
    uint32_t versionStrLength = HcStrlen(versionStr) + 1;
    int32_t res = HC_SUCCESS;
    do {
        res = InitSingleParam(&(spekeSession->baseParam.extraData), versionStrLength);
        if (res != HC_SUCCESS) {
            LOGE("InitSingleParam for extraData failed, res: %d.", res);
            break;
        }
        res = memcpy_s(spekeSession->baseParam.extraData.val, spekeSession->baseParam.extraData.length,
            versionStr, versionStrLength);
        if (res != HC_SUCCESS) {
            LOGE("Memcpy for extraData failed.");
            HcFree(spekeSession->baseParam.extraData.val);
            spekeSession->baseParam.extraData.val = NULL;
            break;
        }
    } while (0);
    FreeJsonString(versionStr);
    return res;
}

static int32_t FillPskAndDeviceId(SpekeSession *spekeSession)
{
    if (spekeSession->sharedSecret.length < MIN_SHAREDSECRET_LEN ||
        spekeSession->sharedSecret.length > MAX_SHAREDSECRET_LEN) {
        LOGE("sharedSecret len is invalid.");
        return HC_ERR_INVALID_LEN;
    }

    int32_t res = InitSingleParam(&(spekeSession->baseParam.psk), spekeSession->sharedSecret.length);
    if (res != HC_SUCCESS) {
        LOGE("InitSingleParam for psk failed, res: %d.", res);
        return res;
    }
    if (memcpy_s(spekeSession->baseParam.psk.val, spekeSession->baseParam.psk.length,
        spekeSession->sharedSecret.val, spekeSession->sharedSecret.length) != HC_SUCCESS) {
        LOGE("Memcpy for psk failed.");
        FreeAndCleanKey(&spekeSession->baseParam.psk);
        return HC_ERR_MEMORY_COPY;
    }

    res = InitSingleParam(&(spekeSession->baseParam.idSelf), spekeSession->deviceId.length);
    if (res != HC_SUCCESS) {
        LOGE("InitSingleParam for idSelf failed, res: %d.", res);
        FreeAndCleanKey(&spekeSession->baseParam.psk);
        return res;
    }
    if (memcpy_s(spekeSession->baseParam.idSelf.val, spekeSession->baseParam.idSelf.length,
        spekeSession->deviceId.val, spekeSession->deviceId.length) != HC_SUCCESS) {
        LOGE("Memcpy for idSelf failed.");
        FreeAndCleanKey(&spekeSession->baseParam.psk);
        FreeAndCleanKey(&spekeSession->baseParam.idSelf);
        return HC_ERR_MEMORY_COPY;
    }
    return HC_SUCCESS;
}

static int32_t ConstructOutJson(CJson *out)
{
    int32_t res;
    CJson *payload = NULL;
    CJson *sendToPeer = NULL;
    do {
        payload = CreateJson();
        if (payload == NULL) {
            LOGE("Create payload json failed.");
            res =  HC_ERR_JSON_CREATE;
            break;
        }
        sendToPeer = CreateJson();
        if (sendToPeer == NULL) {
            LOGE("Create sendToPeer json failed.");
            res =  HC_ERR_JSON_CREATE;
            break;
        }
        res = AddObjToJson(sendToPeer, FIELD_SDK_PAYLOAD, payload);
        if (res != HC_SUCCESS) {
            LOGE("Add payload to sendToPeer failed, res: %d.", res);
            break;
        }
        res = AddObjToJson(out, FIELD_SDK_SEND_TO_PEER, sendToPeer);
        if (res != HC_SUCCESS) {
            LOGE("Add sendToPeer to out failed, res: %d.", res);
            break;
        }
    } while (0);
    FreeJson(payload);
    FreeJson(sendToPeer);
    return res;
}

static int32_t PackagePakeResponseData(const PakeBaseParams *baseParams, CJson *payload)
{
    int32_t res = AddByteToJson(payload, FIELD_SALT, baseParams->salt.val, baseParams->salt.length);
    if (res != HC_SUCCESS) {
        LOGE("Add salt failed, res: %d.", res);
        return HC_ERR_JSON_ADD;
    }
    res = AddByteToJson(payload, FIELD_EPK, baseParams->epkSelf.val, baseParams->epkSelf.length);
    if (res != HC_SUCCESS) {
        LOGE("Add epkSelf failed, res: %d.", res);
        return HC_ERR_JSON_ADD;
    }
    res = AddByteToJson(payload, FIELD_PEER_AUTH_ID, baseParams->idSelf.val, baseParams->idSelf.length);
    if (res != HC_SUCCESS) {
        LOGE("Add idSelf failed, res: %d.", res);
        return res;
    }
    return HC_SUCCESS;
}

static int32_t PackageMsgForPakeResponse(SpekeSession *spekeSession, CJson *outJson, PackageExtraData packageExtraData)
{
    int32_t res = ConstructOutJson(outJson);
    if (res != HC_SUCCESS) {
        LOGE("ConstructOutJson failed, res: %d.", res);
        return res;
    }
    CJson *payload = GetObjFromJson(outJson, FIELD_SDK_PAYLOAD);
    if (payload == NULL) {
        LOGE("Get payload from json failed.");
        return HC_ERR_JSON_GET;
    }
    res = (*packageExtraData)(&spekeSession->baseParam, payload);
    if (res != HC_SUCCESS) {
        LOGE("PackagePakeResponseData failed, res: %d.", res);
        return res;
    }
    res = AddIntToJson(payload, FIELD_SDK_STEP, spekeSession->step);
    if (res != HC_SUCCESS) {
        LOGE("Add step failed, res: %d.", res);
        return res;
    }
    res = AddIntToJson(outJson, FIELD_SDK_ERROR_CODE, KEYAGREE_SUCCESS);
    if (res != HC_SUCCESS) {
        LOGE("Add errorCode failed, res: %d.", res);
        return res;
    }
    res = AddVersionToJson(outJson, &(spekeSession->versionInfo.curVersion));
    if (res != HC_SUCCESS) {
        LOGE("AddVersionToOut failed, res: %d.", res);
        return res;
    }
    return res;
}

static int32_t BuildAndPutOutMessage(SpekeSession *spekeSession, KeyAgreeBlob *out, PackageExtraData packageExtraData)
{
    CJson *outJson = CreateJson();
    if (outJson == NULL) {
        LOGE("Create outJson failed.");
        return HC_ERR_JSON_CREATE;
    }
    int32_t res = PackageMsgForPakeResponse(spekeSession, outJson, packageExtraData);
    if (res != HC_SUCCESS) {
        LOGE("PackageMsgForResponse failed, res: %d.", res);
        FreeJson(outJson);
        return res;
    }
    char *returnStr = PackJsonToString(outJson);
    FreeJson(outJson);
    if (returnStr == NULL) {
        LOGE("Pack json to string failed.");
        return HC_ERR_PACKAGE_JSON_TO_STRING_FAIL;
    }
    uint32_t returnStrLen = HcStrlen(returnStr);
    if (memcpy_s(out->data, out->length, returnStr, returnStrLen + 1) != EOK) {
        LOGE("Memcpy for out blob failed.");
        FreeJsonString(returnStr);
        return HC_ERR_MEMORY_COPY;
    }
    out->length = returnStrLen + 1;
    FreeJsonString(returnStr);
    return HC_SUCCESS;
}

static int32_t PakeResponse(SpekeSession *spekeSession, KeyAgreeBlob *out)
{
    spekeSession->baseParam.isClient = false;
    int32_t res = FillPskAndDeviceId(spekeSession);
    if (res != HC_SUCCESS) {
        LOGE("FillPskAndDeviceId failed, res: %d.", res);
        return res;
    }
    res = ServerResponsePakeV2Protocol(&spekeSession->baseParam);
    if (res != HC_SUCCESS) {
        LOGE("ServerResponsePakeV2Protocol failed, res:%d", res);
        return res;
    }
    res = BuildAndPutOutMessage(spekeSession, out, PackagePakeResponseData);
    if (res != HC_SUCCESS) {
        LOGE("BuildAndPutOutMessage failed, res:%d", res);
        return res;
    }
    return res;
}

static int32_t CheckPeerProtocolVersion(SpekeSession *spekeSession, CJson *inParams)
{
    VersionStruct curVersionPeer = { 0, 0, 0 };
    int32_t res = GetVersionFromJson(inParams, &curVersionPeer);
    if (res != HC_SUCCESS) {
        LOGE("Get peer version info failed, res: %d.", res);
        return res;
    }
    res = NegotiateVersion(&curVersionPeer, &(spekeSession->versionInfo.curVersion));
    if (res != HC_SUCCESS) {
        LOGE("NegotiateVersion failed, res: %d.", res);
        return res;
    }
    return HC_SUCCESS;
}

static int32_t PakeConfirm(SpekeSession *spekeSession, CJson *inParams, KeyAgreeBlob *out)
{
    int32_t res = CheckPeerProtocolVersion(spekeSession, inParams);
    if (res != HC_SUCCESS) {
        LOGE("Peer protocol is not supported!");
        return res;
    }
    spekeSession->baseParam.isClient = true;
    res = FillPskAndDeviceId(spekeSession);
    if (res != HC_SUCCESS) {
        LOGE("FillPskAndDeviceId failed, res: %d.", res);
        return res;
    }
    res = GetAuthIdPeerFromPayload(inParams, &(spekeSession->baseParam.idSelf), &(spekeSession->baseParam.idPeer));
    if (res != HC_SUCCESS) {
        LOGE("Get peer authId failed, res: %d.", res);
        return res;
    }
    res = ParsePakeResponseMessage(&(spekeSession->baseParam), inParams);
    if (res != HC_SUCCESS) {
        LOGE("ParsePakeResponseMessage failed, res: %d.", res);
        return res;
    }
    res = ClientConfirmPakeV2Protocol(&(spekeSession->baseParam));
    if (res != HC_SUCCESS) {
        LOGE("ClientConfirmPakeV2Protocol failed, res:%d", res);
        return res;
    }
    res = BuildAndPutOutMessage(spekeSession, out, PackagePakeClientConfirmData);
    if (res != HC_SUCCESS) {
        LOGE("BuildAndPutOutMessage failed, res:%d", res);
        return res;
    }
    return res;
}

static int32_t PakeServerConfirm(SpekeSession *spekeSession, CJson *inParams, KeyAgreeBlob *out)
{
    int32_t res = ParsePakeClientConfirmMessage(&(spekeSession->baseParam), inParams);
    if (res != HC_SUCCESS) {
        LOGE("ParsePakeClientConfirmMessage failed, res: %d", res);
        return res;
    }
    res = GetAuthIdPeerFromPayload(inParams, &(spekeSession->baseParam.idSelf), &(spekeSession->baseParam.idPeer));
    if (res != HC_SUCCESS) {
        LOGE("Get peer authId failed, res: %d.", res);
        return res;
    }
    res = ServerConfirmPakeV2Protocol(&(spekeSession->baseParam));
    if (res != HC_SUCCESS) {
        LOGE("ServerConfirmPakeV2Protocol failed, res:%d", res);
        return res;
    }
    res = BuildAndPutOutMessage(spekeSession, out, PackagePakeServerConfirmData);
    if (res != HC_SUCCESS) {
        LOGE("BuildAndPutOutMessage failed, res:%d", res);
        return res;
    }
    spekeSession->isFinish = true;
    return res;
}

static int32_t PakeClientVerifyConfirm(SpekeSession *spekeSession, CJson *inParams, KeyAgreeBlob *out)
{
    int32_t res = ParsePakeServerConfirmMessage(&(spekeSession->baseParam), inParams);
    if (res != HC_SUCCESS) {
        LOGE("ParsePakeServerConfirmMessage failed, res: %d.", res);
        return res;
    }
    res = ClientVerifyConfirmPakeV2Protocol(&(spekeSession->baseParam));
    if (res != HC_SUCCESS) {
        LOGE("ClientVerifyConfirmPakeV2Protocol failed, res: %d.", res);
        return res;
    }
    FreeAndCleanKey(&spekeSession->sharedSecret);
    out->length = 0;
    spekeSession->isFinish = true;
    return res;
}

static int32_t ProcessStep(ProtocolStep step, SpekeSession *spekeSession, CJson *inParams, KeyAgreeBlob *out)
{
    int32_t res = HC_SUCCESS;
    LOGI("In key agree step: %d.", step);
    switch (step) {
        case STEP_ONE:
            if (PakeResponse(spekeSession, out) != HC_SUCCESS) {
                LOGE("PakeResponse fail!");
                res = HC_ERROR;
            }
            break;
        case STEP_TWO:
            if (PakeConfirm(spekeSession, inParams, out) != HC_SUCCESS) {
                LOGE("PakeConfirm fail!");
                res = HC_ERROR;
            }
            break;
        case STEP_THREE:
            if (PakeServerConfirm(spekeSession, inParams, out) != HC_SUCCESS) {
                LOGE("PakeServerConfirm fail!");
                res = HC_ERROR;
            }
            break;
        case STEP_FOUR:
            if (PakeClientVerifyConfirm(spekeSession, inParams, out) != HC_SUCCESS) {
                LOGE("PakeClientVerifyConfirm fail!");
                res = HC_ERROR;
            }
            break;
        default:
            LOGI("Unkonw step, may key agree is finish!");
            res = HC_ERR_BAD_MESSAGE;
            break;
    }
    if (inParams != NULL) {
        FreeJson(inParams);
        inParams = NULL;
    }
    return res;
}

static int32_t ProcessSpekeSession(SpekeSession *spekeSession, const KeyAgreeBlob *in, KeyAgreeBlob *out)
{
    LOGI("Process session start!");
    CJson *inParams = NULL;
    int32_t step = 0;
    if ((spekeSession->step == STEP_INIT) && (in == NULL)) {
        LOGI("It is first message!");
        spekeSession->step = STEP_ONE;
    } else {
        if (in->data == NULL) {
            LOGE("Invalid params!");
            return HC_ERR_INVALID_PARAMS;
        }
        inParams = CreateJsonFromString((const char *)(in->data));
        if (inParams == NULL) {
            LOGE("Create from json failed!");
            return HC_ERROR;
        }
        if (GetIntFromJson(inParams, FIELD_SDK_STEP, &step) != HC_SUCCESS) {
            LOGI("There is no field named step, it is first protocol!");
            spekeSession->step = STEP_ONE;
        } else {
            spekeSession->step = step + 1;
        }
    }
    return ProcessStep(spekeSession->step, spekeSession, inParams, out);
}

static int32_t InitSpekeSession(SpekeSession *spekeSession, KeyAgreeProtocol protocol)
{
    switch (protocol) {
        case KEYAGREE_PROTOCOL_DL_SPEKE_256:
            LOGI("Init protocol of dl speke 256");
            spekeSession->baseParam.supportedPakeAlg = PAKE_ALG_DL;
            spekeSession->baseParam.supportedDlPrimeMod =
                spekeSession->baseParam.supportedDlPrimeMod | DL_PRIME_MOD_256;
            break;
        case KEYAGREE_PROTOCOL_DL_SPEKE_384:
            LOGI("Init protocol of dl speke 384");
            spekeSession->baseParam.supportedPakeAlg = PAKE_ALG_DL;
            spekeSession->baseParam.supportedDlPrimeMod =
                spekeSession->baseParam.supportedDlPrimeMod | DL_PRIME_MOD_384;
            break;
        case KEYAGREE_PROTOCOL_EC_SPEKE_P256:
            LOGI("Init protocol of ec speke p256");
            spekeSession->baseParam.supportedPakeAlg = PAKE_ALG_EC;
            spekeSession->baseParam.curveType = CURVE_256;
            break;
        case KEYAGREE_PROTOCOL_EC_SPEKE_X25519:
            LOGI("Init protocol of ec speke X25519");
            spekeSession->baseParam.supportedPakeAlg = PAKE_ALG_EC;
            spekeSession->baseParam.curveType = CURVE_25519;
            break;
        default:
            LOGE("Invalid protocol type!");
            return HC_ERROR;
            break;
    }
    return HC_SUCCESS;
}

static KeyAgreeProtocol GetKegAgreeProtocolType(VersionStruct *curVersion)
{
    for (uint32_t i = 0; i < sizeof(g_protocolAlgorithm); i++) {
        if ((curVersion->second & g_protocolAlgorithm[i]) == g_protocolAlgorithm[i]) {
            curVersion->second = g_protocolAlgorithm[i];
            return g_keyAgreeProtocol[i];
        }
    }
    LOGE("No protocol supported!");
    return KEYAGREE_PROTOCOL_ANY;
}

static int32_t ProcessProtocolInitial(SpekeSession *spekeSession, KeyAgreeBlob *out)
{
    CJson *outJsonMessage = CreateJson();
    if (outJsonMessage == NULL) {
        LOGE("CreateJson failed!");
        return HC_ERR_JSON_CREATE;
    }
    int32_t res = AddVersionToJson(outJsonMessage, &(spekeSession->versionInfo.curVersion));
    if (res != HC_SUCCESS) {
        LOGE("AddVersionToOut failed, res: %d.", res);
        FreeJson(outJsonMessage);
        return HC_ERR_JSON_ADD;
    }
    res = AddIntToJson(outJsonMessage, FIELD_SDK_ERROR_CODE, KEYAGREE_SUCCESS);
    if (res != HC_SUCCESS) {
        LOGE("Add errorCode failed, res: %d.", res);
        FreeJson(outJsonMessage);
        return res;
    }
    res = FillExtraData(spekeSession, outJsonMessage);
    if (res != HC_SUCCESS) {
        LOGE("Fill extra data failed, res: %d.", res);
        FreeJson(outJsonMessage);
        return res;
    }
    char *returnStr = PackJsonToString(outJsonMessage);
    FreeJson(outJsonMessage);
    if (returnStr == NULL) {
        LOGE("Pack json to string failed.");
        return HC_ERR_PACKAGE_JSON_TO_STRING_FAIL;
    }
    uint32_t returnStrLen = HcStrlen(returnStr);
    if (memcpy_s(out->data, out->length, returnStr, returnStrLen + 1) != EOK) {
        LOGE("Memcpy for out blob failed.");
        FreeJsonString(returnStr);
        return HC_ERR_MEMORY_COPY;
    }
    out->length = returnStrLen + 1;
    FreeJsonString(returnStr);
    spekeSession->versionInfo.versionStatus = VERSION_CONFIRM;
    spekeSession->step = STEP_ONE;
    return HC_SUCCESS;
}

static int32_t ProcessProtocolConfirm(SpekeSession *spekeSession, const KeyAgreeBlob *in, KeyAgreeBlob *out)
{
    if (spekeSession == NULL || in == NULL || in->data == NULL || out == NULL) {
        LOGE("Invalid params!");
        return HC_ERR_INVALID_PARAMS;
    }
    CJson *inJsonMessage = CreateJsonFromString((const char *)(in->data));
    if (inJsonMessage == NULL) {
        LOGE("Create json failed!");
        return HC_ERR_JSON_CREATE;
    }
    int32_t res;
    if (spekeSession->keyAgreeType == KEYAGREE_TYPE_SERVER) {
        res = FillExtraData(spekeSession, inJsonMessage);
        if (res != HC_SUCCESS) {
            LOGE("Fill extra data failed!");
            FreeJson(inJsonMessage);
            return res;
        }
    }
    VersionStruct curVersionPeer = { 0, 0, 0 };
    res = GetVersionFromJson(inJsonMessage, &curVersionPeer);
    FreeJson(inJsonMessage);
    if (res != HC_SUCCESS) {
        LOGE("Get peer version info failed, res: %d.", res);
        return res;
    }
    res = NegotiateVersion(&curVersionPeer, &(spekeSession->versionInfo.curVersion));
    if (res != HC_SUCCESS) {
        LOGE("NegotiateVersion failed, res: %d.", res);
        return res;
    }
    KeyAgreeProtocol protocolType = GetKegAgreeProtocolType(&(spekeSession->versionInfo.curVersion));
    if (protocolType == KEYAGREE_PROTOCOL_ANY) {
        LOGE("GetKegAgreeProtocolType failed!, it is no protocol support!");
        return HC_ERR_NOT_SUPPORT;
    }
    res = InitSpekeSession(spekeSession, protocolType);
    if (res != HC_SUCCESS) {
        LOGE("Init protocol session fail!");
        return res;
    }
    res = spekeSession->processSession(spekeSession, in, out);
    if (res != HC_SUCCESS) {
        LOGE("ProcessProtocolAgree:protocol agree fail!");
        return res;
    }
    spekeSession->versionInfo.versionStatus = VERSION_DECIDED;
    return HC_SUCCESS;
}

static int32_t ProcessProtocolAgree(SpekeSession *spekeSession, const KeyAgreeBlob *in, KeyAgreeBlob *out)
{
    int32_t res = HC_SUCCESS;
    if (spekeSession->versionInfo.versionStatus == INITIAL) {
        res = ProcessProtocolInitial(spekeSession, out);
    } else if (spekeSession->versionInfo.versionStatus == VERSION_CONFIRM) {
        res = ProcessProtocolConfirm(spekeSession, in, out);
    } else {
        LOGI("Version is decided, no need do protocol agree!");
    }
    if (res != HC_SUCCESS) {
        LOGE("ProcessProtocolAgree failed, versionStatus is: %d, res: %d.",
            spekeSession->versionInfo.versionStatus, res);
    }
    return res;
}

static int32_t CheckAndInitProtocol(SpekeSession *spekeSession, KeyAgreeProtocol protocol)
{
    int32_t res = HC_ERR_NOT_SUPPORT;
    uint64_t algInProtocol = SPEKE_MOD_NONE;
    switch (protocol) {
        case KEYAGREE_PROTOCOL_ANY:
            InitVersionInfo(&(spekeSession->versionInfo));
            return HC_SUCCESS;
        case KEYAGREE_PROTOCOL_DL_SPEKE_256:
#ifdef P2P_PAKE_DL_PRIME_LEN_256
            algInProtocol |= DL_SPEKE_MOD_256;
            res = HC_SUCCESS;
#endif
            break;
        case KEYAGREE_PROTOCOL_DL_SPEKE_384:
#ifdef P2P_PAKE_DL_PRIME_LEN_384
            algInProtocol |= DL_SPEKE_MOD_384;
            res = HC_SUCCESS;
#endif
            break;
        case KEYAGREE_PROTOCOL_EC_SPEKE_P256:
#ifdef P2P_PAKE_EC_PRIME_P256
            algInProtocol |= EC_SPEKE_P256;
            res = HC_SUCCESS;
#endif
            break;
        case KEYAGREE_PROTOCOL_EC_SPEKE_X25519:
#ifdef P2P_PAKE_EC_PRIME_X25519
            algInProtocol |= EC_SPEKE_X25519;
            res = HC_SUCCESS;
#endif
            break;
        default:
            break;
    }
    GetMaxVersion(algInProtocol, &(spekeSession->versionInfo.curVersion));
    return res;
}

void DestroySpekeSession(SpekeSession *spekeSession)
{
    if (spekeSession == NULL) {
        LOGE("Failed to destroy session, spekeSession is NULL!");
        return;
    }
    DestroyPakeV2BaseParams(&(spekeSession->baseParam));
    FreeAndCleanKey(&spekeSession->sharedSecret);
    FreeAndCleanKey(&spekeSession->deviceId);
    HcFree(spekeSession->extras);
    HcFree(spekeSession);
}

SpekeSession *CreateSpekeSession(void)
{
    SpekeSession *spekeSession = (SpekeSession *)HcMalloc(sizeof(SpekeSession), 0);
    if (spekeSession == NULL) {
        LOGE("Failed to allocate session memory!");
        return spekeSession;
    }
    if (InitPakeV2BaseParams(&(spekeSession->baseParam)) != HC_SUCCESS) {
        LOGE("InitPakeV2BaseParams failed!");
        HcFree(spekeSession);
        return NULL;
    }
    spekeSession->processProtocolAgree = ProcessProtocolAgree;
    spekeSession->processSession = ProcessSpekeSession;
    spekeSession->initSpekeSession = InitSpekeSession;
    spekeSession->checkAndInitProtocol = CheckAndInitProtocol;
    spekeSession->sharedSecret.val = NULL;
    spekeSession->sharedSecret.length = 0;
    spekeSession->deviceId.val = NULL;
    spekeSession->deviceId.length = 0;
    spekeSession->extras = NULL;
    spekeSession->protocol = KEYAGREE_PROTOCOL_ANY;
    spekeSession->isFinish = false;
    spekeSession->step = STEP_INIT;
    return spekeSession;
}