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

#include "hks_adapter.h"

#include <stddef.h>

#include "securec.h"
#include "hks_api.h"
#include "hks_type.h"
#include "hks_param.h"

#include "device_security_defines.h"
#include "utils_log.h"
#include "utils_mem.h"
#include "utils_tlv.h"

#define MAX_ENTRY 8
#define TYPE_NONCE 0x200
#define TYPE_CERT_BASE 0x100
#define TYPE_CERT_END (TYPE_CERT_BASE + MAX_ENTRY)
#define LIST_MAX_SIZE 10240

// need free
int32_t FillHksParamSet(struct HksParamSet **paramSet, struct HksParam *param, int32_t paramNums)
{
    if (param == NULL) {
        SECURITY_LOG_ERROR("param is null");
        return ERR_INVALID_PARA;
    }
    int32_t ret = HksInitParamSet(paramSet);
    if (ret != HKS_SUCCESS) {
        SECURITY_LOG_ERROR("HksInitParamSet failed, hks ret = %{public}d", ret);
        return ERR_INVALID_PARA;
    }
    ret = HksAddParams(*paramSet, param, paramNums);
    if (ret != HKS_SUCCESS) {
        SECURITY_LOG_ERROR("HksAddParams failed, hks ret = %{public}d", ret);
        HksFreeParamSet(paramSet);
        return ERR_INVALID_PARA;
    }
    ret = HksBuildParamSet(paramSet);
    if (ret != HKS_SUCCESS) {
        SECURITY_LOG_ERROR("HksBuildParamSet failed, hks ret = %{public}d", ret);
        HksFreeParamSet(paramSet);
        return ERR_INVALID_PARA;
    }
    return SUCCESS;
}

int32_t HksGenerateKeyAdapter(const struct HksBlob *keyAlias)
{
    if (keyAlias == NULL) {
        SECURITY_LOG_ERROR("keyAlias is null");
        return ERR_INVALID_PARA;
    }
    struct HksParam tmpParams[] = {
        {.tag = HKS_TAG_KEY_STORAGE_FLAG, .uint32Param = HKS_STORAGE_PERSISTENT},
        {.tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_RSA},
        {.tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_RSA_KEY_SIZE_2048},
        {.tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_VERIFY},
        {.tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA256},
        {.tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_PSS},
        {.tag = HKS_TAG_KEY_GENERATE_TYPE, .uint32Param = HKS_KEY_GENERATE_TYPE_DEFAULT},
        {.tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_ECB},
    };
    struct HksParamSet *paramSet = NULL;
    if (FillHksParamSet(&paramSet, tmpParams, sizeof(tmpParams) / sizeof(tmpParams[0])) != SUCCESS) {
        SECURITY_LOG_ERROR("FillHksParamSet failed");
        return ERR_HUKS_ERR;
    }
    int32_t ret = HksGenerateKey(keyAlias, paramSet, NULL);
    if (ret != HKS_SUCCESS) {
        SECURITY_LOG_ERROR("HksGenerateKey failed, hks ret = %{public}d", ret);
        HksFreeParamSet(&paramSet);
        return ERR_HUKS_ERR;
    }
    HksFreeParamSet(&paramSet);
    return SUCCESS;
}

// need free
int32_t ConstructHksCertChain(struct HksCertChain **certChain, const struct HksCertChainInitParams *certChainParam)
{
    if (certChainParam == NULL || certChainParam->certChainExist == false ||
        certChainParam->certCountValid == false || certChainParam->certDataExist == false) {
        return ERR_INVALID_PARA;
    }

    *certChain = (struct HksCertChain *)MALLOC(sizeof(struct HksCertChain));
    if (*certChain == NULL) {
        return ERR_NO_MEMORY;
    }
    (*certChain)->certsCount = CERT_CHAIN_CERT_NUM;
    (*certChain)->certs = (struct HksBlob *)MALLOC(sizeof(struct HksBlob) * ((*certChain)->certsCount));
    if ((*certChain)->certs == NULL) {
        FREE(*certChain);
        *certChain = NULL;
        return ERR_NO_MEMORY;
    }
    for (uint32_t i = 0; i < (*certChain)->certsCount; i++) {
        (*certChain)->certs[i].size = 0;
        (*certChain)->certs[i].data = NULL;
    }
    for (uint32_t i = 0; i < (*certChain)->certsCount; i++) {
        (*certChain)->certs[i].size = certChainParam->certDataSize;
        (*certChain)->certs[i].data = (uint8_t *)MALLOC((*certChain)->certs[i].size);
        if ((*certChain)->certs[i].data == NULL) {
            DestroyHksCertChain(*certChain);
            *certChain = NULL;
            return ERR_NO_MEMORY;
        }
        (void)memset_s((*certChain)->certs[i].data, certChainParam->certDataSize, 0, certChainParam->certDataSize);
    }
    return SUCCESS;
}

void DestroyHksCertChain(struct HksCertChain *certChain)
{
    if (certChain == NULL || certChain->certs == NULL || certChain->certsCount <= 0) {
        return;
    }
    for (uint32_t i = 0; i < certChain->certsCount; i++) {
        if (certChain->certs[i].data != NULL) {
            FREE(certChain->certs[i].data);
            certChain->certs[i].data = NULL;
        }
    }
    FREE(certChain->certs);
    certChain->certs = NULL;
    FREE(certChain);
}

// need free
int32_t HksCertChainToBuffer(const struct HksCertChain *hksCertChain, uint8_t **data, uint32_t *dataLen)
{
    if (hksCertChain == NULL) {
        return ERR_INVALID_PARA;
    }

    TlvCommon tlvs[MAX_ENTRY];
    (void)memset_s(&tlvs[0], sizeof(tlvs), 0, sizeof(tlvs));
    uint32_t tlvCnt = 0;
    for (uint32_t i = 0; i < hksCertChain->certsCount; i++) {
        tlvs[tlvCnt].tag = TYPE_CERT_BASE + 1;
        tlvs[tlvCnt].len = hksCertChain->certs[i].size;
        tlvs[tlvCnt].value = hksCertChain->certs[i].data;
        tlvCnt++;
    }

    uint8_t *out = MALLOC(LIST_MAX_SIZE);
    if (out == NULL) {
        return ERR_NO_MEMORY;
    }
    (void)memset_s(out, LIST_MAX_SIZE, 0, LIST_MAX_SIZE);
    if (Serialize(tlvs, tlvCnt, out, LIST_MAX_SIZE, dataLen) != TLV_OK) {
        FREE(out);
        return ERR_NO_MEMORY;
    }
    *data = out;
    return SUCCESS;
}

// point to exist memory, no need free but can not change data
int32_t BufferToHksCertChain(const uint8_t *data, uint32_t dataLen, struct HksCertChain *hksCertChain)
{
    if (data == NULL || dataLen == 0) {
        return ERR_INVALID_PARA;
    }
    TlvCommon tlvs[MAX_ENTRY];
    (void)memset_s(&tlvs[0], sizeof(tlvs), 0, sizeof(tlvs));

    uint32_t cnt = 0;
    uint32_t ret = Deserialize(data, dataLen, &tlvs[0], MAX_ENTRY, &cnt);
    if (ret != TLV_OK || cnt == 0 || cnt > MAX_ENTRY) {
        return ERR_INVALID_PARA;
    }
    uint32_t certCnt = 0;
    for (uint32_t i = 0; i < cnt; i++) {
        if ((tlvs[i].tag >= TYPE_CERT_BASE) && (tlvs[i].tag <= TYPE_CERT_END)) {
            hksCertChain->certs[certCnt].data = tlvs[i].value;
            hksCertChain->certs[certCnt].size = tlvs[i].len;
            certCnt++;
        }
    }
    hksCertChain->certsCount = certCnt;
    return SUCCESS;
}