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

#include "external_interface_adapter.h"

#include <stddef.h>
#include <string.h>

#include "device_auth.h"
#include "device_auth_defines.h"
#include "hks_adapter.h"
#include "hks_api.h"
#include "hks_param.h"
#include "hks_type.h"
#include "securec.h"

#include "device_security_defines.h"
#include "utils_json.h"
#include "utils_log.h"
#include "utils_mem.h"

#define DEVICE_AUTH_INPUT_PARAM_STRING_LENGTH 512
#define DSLM_CERT_CHAIN_BASE_LENGTH 4096

#define DSLM_INFO_MAX_LEN_UDID 68
#define DSLM_INFO_MAX_LEN_SERIAL 68
#define DSLM_INFO_MAX_LEN_VERSION 128
#define DSLM_INFO_MAX_LEN_CRED 2048
#define DSLM_INFO_MAX_LEN_NONCE 2048

#define EMPTY_PK_INFO "[]"
#define DEFAULT_PK_INFO "[{\"groupId\" : \"0\",\"publicKey\" : \"0\"}]"

static int32_t GenerateFuncParamJson(bool isSelfPk, const char *udidStr, char *dest, uint32_t destMax);

int32_t GetPkInfoListStr(bool isSelf, const char *udidStr, char **pkInfoList)
{
    SECURITY_LOG_INFO("start");

    char paramJson[DEVICE_AUTH_INPUT_PARAM_STRING_LENGTH] = {0};
    char *returnInfoList = NULL;
    uint32_t returnInfoNum = 0;

    int32_t ret = GenerateFuncParamJson(isSelf, udidStr, paramJson, DEVICE_AUTH_INPUT_PARAM_STRING_LENGTH);
    if (ret != SUCCESS) {
        SECURITY_LOG_ERROR("GenerateFuncParamJson failed");
        return ret;
    }

    const DeviceGroupManager *interface = GetGmInstance();
    if (interface == NULL) {
        SECURITY_LOG_ERROR("GetGmInstance null");
        return ERR_CALL_EXTERNAL_FUNC;
    }
    ret = interface->getPkInfoList(ANY_OS_ACCOUNT, "dslm_service", paramJson, &returnInfoList, &returnInfoNum);
    if (ret == HC_ERR_ONLY_ACCOUNT_RELATED) {
        SECURITY_LOG_INFO("device auth cred situation");
        ret = SUCCESS;
    }
    if (ret != SUCCESS) {
        SECURITY_LOG_INFO("getPkInfoList failed, ret = %{public}d", ret);
        return ERR_CALL_EXTERNAL_FUNC;
    }

    char *pkInfoBuff = returnInfoList;
    if (pkInfoBuff == NULL || strcmp(pkInfoBuff, EMPTY_PK_INFO) == 0) {
        SECURITY_LOG_INFO("current pkInfoList is %s", pkInfoBuff == NULL ? "null" : "empty");
        pkInfoBuff = DEFAULT_PK_INFO;
    }

    do {
        char *output = (char *)MALLOC(strlen(pkInfoBuff) + 1);
        if (output == NULL) {
            SECURITY_LOG_ERROR("malloc error");
            ret = ERR_MEMORY_ERR;
            break;
        }

        if (strcpy_s(output, strlen(pkInfoBuff) + 1, pkInfoBuff) != EOK) {
            SECURITY_LOG_ERROR("strcpy_s error");
            ret = ERR_MEMORY_ERR;
            FREE(output);
            break;
        }
        *pkInfoList = output;
        ret = SUCCESS;
    } while (0);

    if (returnInfoList != NULL) {
        interface->destroyInfo(&returnInfoList);
    }
    return ret;
}

int32_t DslmCredAttestAdapter(struct DslmInfoInCertChain *info, uint8_t **certChain, uint32_t *certChainLen)
{
    SECURITY_LOG_INFO("start");

    const char alias[] = "dslm_key";
    struct HksBlob keyAlias = {sizeof(alias), (uint8_t *)alias};

    if (HksGenerateKeyAdapter(&keyAlias) != SUCCESS) {
        SECURITY_LOG_ERROR("HksGenerateKeyAdapter failed");
        return ERR_HUKS_ERR;
    }
    struct HksParam inputData[] = {
        {.tag = HKS_TAG_ATTESTATION_CHALLENGE, .blob = {strlen(info->nonceStr) + 1, (uint8_t *)info->nonceStr}},
        {.tag = HKS_TAG_ATTESTATION_ID_SEC_LEVEL_INFO, .blob = {strlen(info->credStr) + 1, (uint8_t *)info->credStr}},
        {.tag = HKS_TAG_ATTESTATION_ID_UDID, .blob = {strlen(info->udidStr) + 1, (uint8_t *)info->udidStr}},
        {.tag = HKS_TAG_ATTESTATION_ID_ALIAS, .blob = keyAlias},
    };

    struct HksParamSet *inputParam = NULL;
    uint32_t certChainMaxLen = strlen(info->credStr) + strlen(info->nonceStr) + DSLM_CERT_CHAIN_BASE_LENGTH;
    struct HksCertChain *hksCertChain = NULL;
    const struct HksCertChainInitParams certParam = {true, true, true, certChainMaxLen};

    int32_t ret = ConstructHksCertChain(&hksCertChain, &certParam);
    if (ret != SUCCESS) {
        SECURITY_LOG_ERROR("ConstructHksCertChain failed, ret = %{public}d ", ret);
        return ret;
    }
    if (FillHksParamSet(&inputParam, inputData, sizeof(inputData) / sizeof(inputData[0])) != SUCCESS) {
        SECURITY_LOG_ERROR("FillHksParamSet failed");
        DestroyHksCertChain(hksCertChain);
        return ERR_CALL_EXTERNAL_FUNC;
    }
    ret = HksAttestKey(&keyAlias, inputParam, hksCertChain);
    if (ret != HKS_SUCCESS) {
        SECURITY_LOG_ERROR("HksAttestKey failed, ret = %{public}d ", ret);
        HksFreeParamSet(&inputParam);
        DestroyHksCertChain(hksCertChain);
        return ERR_CALL_EXTERNAL_FUNC;
    }
    ret = HksCertChainToBuffer(hksCertChain, certChain, certChainLen);
    if (ret != SUCCESS) {
        SECURITY_LOG_ERROR("HksCertChainToHksBlob failed");
        HksFreeParamSet(&inputParam);
        DestroyHksCertChain(hksCertChain);
        FREE(*certChain);
        *certChain = NULL;
        return ret;
    }
    HksFreeParamSet(&inputParam);
    DestroyHksCertChain(hksCertChain);
    SECURITY_LOG_DEBUG("success, certChainLen = %{public}d ", *certChainLen);
    return SUCCESS;
}

int32_t ValidateCertChainAdapter(const uint8_t *data, uint32_t dataLen, struct DslmInfoInCertChain *resultInfo)
{
    SECURITY_LOG_INFO("start");

    char nonceStr[DSLM_INFO_MAX_LEN_NONCE] = {0};
    char credStr[DSLM_INFO_MAX_LEN_CRED] = {0};
    char udidStr[DSLM_INFO_MAX_LEN_UDID] = {0};
    struct HksParam outputData[] = {
        {.tag = HKS_TAG_ATTESTATION_CHALLENGE, .blob = {DSLM_INFO_MAX_LEN_NONCE, (uint8_t *)nonceStr}},
        {.tag = HKS_TAG_ATTESTATION_ID_SEC_LEVEL_INFO, .blob = {DSLM_INFO_MAX_LEN_CRED, (uint8_t *)credStr}},
        {.tag = HKS_TAG_ATTESTATION_ID_UDID, .blob = {DSLM_INFO_MAX_LEN_UDID, (uint8_t *)udidStr}},
    };
    struct HksParamSet *outputParam = NULL;
    struct HksBlob certBlob[CERT_CHAIN_CERT_NUM] = {{0}};
    struct HksCertChain hksCertChain = {&certBlob[0], CERT_CHAIN_CERT_NUM};

    if (BufferToHksCertChain(data, dataLen, &hksCertChain) != SUCCESS) {
        SECURITY_LOG_ERROR("BufferToHksCertChain failed");
        return ERR_CALL_EXTERNAL_FUNC;
    }
    if (FillHksParamSet(&outputParam, outputData, sizeof(outputData) / sizeof(outputData[0])) != SUCCESS) {
        SECURITY_LOG_ERROR("FillHksParamSet failed");
        return ERR_CALL_EXTERNAL_FUNC;
    }
    if (HksValidateCertChain(&hksCertChain, outputParam) != HKS_SUCCESS) {
        SECURITY_LOG_ERROR("HksValidateCertChain failed");
        HksFreeParamSet(&outputParam);
        return ERR_CALL_EXTERNAL_FUNC;
    }
    uint32_t cnt = 0;
    struct HksBlob *blob = &outputParam->params[cnt].blob;
    if (memcpy_s(resultInfo->nonceStr, DSLM_INFO_MAX_LEN_NONCE, blob->data, blob->size) != EOK) {
        HksFreeParamSet(&outputParam);
        return ERR_MEMORY_ERR;
    }
    blob = &outputParam->params[++cnt].blob;
    if (memcpy_s(resultInfo->credStr, DSLM_INFO_MAX_LEN_CRED, blob->data, blob->size) != EOK) {
        HksFreeParamSet(&outputParam);
        return ERR_MEMORY_ERR;
    }
    blob = &outputParam->params[++cnt].blob;
    if (memcpy_s(resultInfo->udidStr, DSLM_INFO_MAX_LEN_UDID, blob->data, blob->size) != EOK) {
        HksFreeParamSet(&outputParam);
        return ERR_MEMORY_ERR;
    }

    SECURITY_LOG_INFO("success");
    HksFreeParamSet(&outputParam);
    return SUCCESS;
}

int32_t HksAttestIsReadyAdapter(void)
{
    if (HcmIsDeviceKeyExist(NULL) != HKS_SUCCESS) {
        SECURITY_LOG_ERROR("Hks attest not ready");
        return ERR_CALL_EXTERNAL_FUNC;
    }
    return SUCCESS;
}

static int32_t GenerateFuncParamJson(bool isSelfPk, const char *udidStr, char *dest, uint32_t destMax)
{
    JsonHandle json = CreateJson(NULL);
    if (json == NULL) {
        return ERR_INVALID_PARA;
    }

    AddFieldBoolToJson(json, "isSelfPk", isSelfPk);
    AddFieldStringToJson(json, "udid", udidStr);

    char *paramsJsonBuffer = ConvertJsonToString(json);
    if (paramsJsonBuffer == NULL) {
        DestroyJson(json);
        return ERR_MEMORY_ERR;
    }
    DestroyJson(json);
    if (strcpy_s(dest, destMax, paramsJsonBuffer) != EOK) {
        FREE(paramsJsonBuffer);
        paramsJsonBuffer = NULL;
        return ERR_MEMORY_ERR;
    }
    FREE(paramsJsonBuffer);
    paramsJsonBuffer = NULL;
    return SUCCESS;
}

int32_t InitDslmInfoInCertChain(struct DslmInfoInCertChain *saveInfo)
{
    if (saveInfo == NULL) {
        return ERR_INVALID_PARA;
    }
    saveInfo->nonceStr = (char *)MALLOC(DSLM_INFO_MAX_LEN_NONCE);
    if (saveInfo->nonceStr == NULL) {
        return ERR_NO_MEMORY;
    }
    saveInfo->credStr = (char *)MALLOC(DSLM_INFO_MAX_LEN_CRED);
    if (saveInfo->credStr == NULL) {
        FREE(saveInfo->nonceStr);
        saveInfo->nonceStr = NULL;
        return ERR_NO_MEMORY;
    }
    saveInfo->udidStr = (char *)MALLOC(DSLM_INFO_MAX_LEN_UDID);
    if (saveInfo->udidStr == NULL) {
        FREE(saveInfo->nonceStr);
        saveInfo->nonceStr = NULL;
        FREE(saveInfo->credStr);
        saveInfo->credStr = NULL;
        return ERR_NO_MEMORY;
    }
    return SUCCESS;
}

void DestroyDslmInfoInCertChain(struct DslmInfoInCertChain *saveInfo)
{
    if (saveInfo == NULL) {
        return;
    }
    if (saveInfo->nonceStr != NULL) {
        FREE(saveInfo->nonceStr);
        saveInfo->nonceStr = NULL;
    }
    if (saveInfo->credStr != NULL) {
        FREE(saveInfo->credStr);
        saveInfo->credStr = NULL;
    }
    if (saveInfo->udidStr != NULL) {
        FREE(saveInfo->udidStr);
        saveInfo->udidStr = NULL;
    }
    (void)memset_s(saveInfo, sizeof(struct DslmInfoInCertChain), 0, sizeof(struct DslmInfoInCertChain));
}
