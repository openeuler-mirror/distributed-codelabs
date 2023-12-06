/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "hks_ipc_service.h"

#include <dlfcn.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef HKS_CONFIG_FILE
#include HKS_CONFIG_FILE
#else
#include "hks_config.h"
#endif

#include "hks_client_service.h"
#include "hks_cmd_id.h"
#include "hks_ipc_serialization.h"
#include "hks_log.h"
#include "hks_mem.h"
#include "hks_response.h"
#include "hks_template.h"

#define MAX_KEY_SIZE         2048

#ifdef HKS_SUPPORT_ACCESS_TOKEN
static enum HksTag g_idList[] = {
    HKS_TAG_ATTESTATION_ID_BRAND,
    HKS_TAG_ATTESTATION_ID_DEVICE,
    HKS_TAG_ATTESTATION_ID_PRODUCT,
    HKS_TAG_ATTESTATION_ID_SERIAL,
    HKS_TAG_ATTESTATION_ID_IMEI,
    HKS_TAG_ATTESTATION_ID_MEID,
    HKS_TAG_ATTESTATION_ID_MANUFACTURER,
    HKS_TAG_ATTESTATION_ID_MODEL,
    HKS_TAG_ATTESTATION_ID_SOCID,
    HKS_TAG_ATTESTATION_ID_UDID,
};
#endif

void HksIpcServiceGenerateKey(const struct HksBlob *srcData, const uint8_t *context)
{
    struct HksBlob keyAlias = { 0, NULL };
    struct HksParamSet *inParamSet = NULL;
    struct HksBlob keyOut = { 0, NULL };
    struct HksProcessInfo processInfo = { { 0, NULL }, { 0, NULL } };
    int32_t ret;
    bool isNoneResponse = false;

    do {
        ret = HksGenerateKeyUnpack(srcData, &keyAlias, &inParamSet, &keyOut);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksGenerateKeyUnpack Ipc fail")

        if (keyOut.data == NULL) {
            isNoneResponse = true;
            keyOut.data = (uint8_t *)HksMalloc(MAX_KEY_SIZE);
            if (keyOut.data == NULL) {
                HKS_LOG_E("malloc fail.");
                ret = HKS_ERROR_MALLOC_FAIL;
                break;
            }
            keyOut.size = MAX_KEY_SIZE;
        }

        ret = HksGetProcessInfoForIPC(context, &processInfo);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksGetProcessInfoForIPC fail, ret = %" LOG_PUBLIC "d", ret)

        ret = HksServiceGenerateKey(&processInfo, &keyAlias, inParamSet, &keyOut);
        HKS_IF_NOT_SUCC_LOGE(ret, "HksServiceGenerateKey fail, ret = %" LOG_PUBLIC "d", ret)

        HKS_LOG_E("key out size = %" LOG_PUBLIC "x", keyOut.size);
    } while (0);

    if (isNoneResponse) {
        HksSendResponse(context, ret, NULL);
    } else {
        HksSendResponse(context, ret, &keyOut);
    }

    HKS_FREE_BLOB(keyOut);
    HKS_FREE_BLOB(processInfo.processName);
    HKS_FREE_BLOB(processInfo.userId);
}

void HksIpcServiceImportKey(const struct HksBlob *srcData, const uint8_t *context)
{
    struct HksBlob keyAlias = { 0, NULL };
    struct HksParamSet *paramSet = NULL;
    struct HksBlob key = { 0, NULL };
    struct HksProcessInfo processInfo = { { 0, NULL }, { 0, NULL } };
    int32_t ret;

    do {
        ret  = HksImportKeyUnpack(srcData, &keyAlias, &paramSet, &key);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksImportKeyUnpack Ipc fail")

        ret = HksGetProcessInfoForIPC(context, &processInfo);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksGetProcessInfoForIPC fail, ret = %" LOG_PUBLIC "d", ret)

        ret =  HksServiceImportKey(&processInfo, &keyAlias, paramSet, &key);
        HKS_IF_NOT_SUCC_LOGE(ret, "HksServiceImportKey fail, ret = %" LOG_PUBLIC "d", ret)
    } while (0);

    HksSendResponse(context, ret, NULL);

    HKS_FREE_BLOB(processInfo.processName);
    HKS_FREE_BLOB(processInfo.userId);
}

void HksIpcServiceImportWrappedKey(const struct HksBlob *srcData, const uint8_t *context)
{
    struct HksBlob keyAlias = { 0, NULL };
    struct HksBlob wrappingKeyAlias = { 0, NULL };
    struct HksParamSet *paramSet = NULL;
    struct HksBlob wrappedKeyData = { 0, NULL };
    struct HksProcessInfo processInfo = { { 0, NULL }, { 0, NULL } };
    int32_t ret;

    do {
        ret  = HksImportWrappedKeyUnpack(srcData, &keyAlias, &wrappingKeyAlias, &paramSet, &wrappedKeyData);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "unpack data for Ipc fail")

        ret = HksGetProcessInfoForIPC(context, &processInfo);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "get process info fail, ret = %" LOG_PUBLIC "d", ret)

        ret =  HksServiceImportWrappedKey(&processInfo, &keyAlias, &wrappingKeyAlias, paramSet, &wrappedKeyData);
        HKS_IF_NOT_SUCC_LOGE(ret, "do import wrapped key fail, ret = %" LOG_PUBLIC "d", ret)
    } while (0);

    HksSendResponse(context, ret, NULL);

    HKS_FREE_BLOB(processInfo.processName);
    HKS_FREE_BLOB(processInfo.userId);
}

void HksIpcServiceExportPublicKey(const struct HksBlob *srcData, const uint8_t *context)
{
    struct HksBlob keyAlias = { 0, NULL };
    struct HksBlob key = { 0, NULL };
    struct HksProcessInfo processInfo = { { 0, NULL }, { 0, NULL } };
    int32_t ret;

    do {
        ret  = HksExportPublicKeyUnpack(srcData, &keyAlias, &key);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksExportKeyUnpack Ipc fail")

        ret = HksGetProcessInfoForIPC(context, &processInfo);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksGetProcessInfoForIPC fail, ret = %" LOG_PUBLIC "d", ret)

        ret = HksServiceExportPublicKey(&processInfo, &keyAlias, &key);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksServiceExportPublicKey fail, ret = %" LOG_PUBLIC "d", ret)

        HksSendResponse(context, ret, &key);
    } while (0);

    if (ret != HKS_SUCCESS) {
        HksSendResponse(context, ret, NULL);
    }

    HKS_FREE_BLOB(key);
    HKS_FREE_BLOB(processInfo.processName);
    HKS_FREE_BLOB(processInfo.userId);
}

void HksIpcServiceDeleteKey(const struct HksBlob *srcData, const uint8_t *context)
{
    struct HksProcessInfo processInfo = { { 0, NULL }, { 0, NULL } };
    int32_t ret;
    do {
        ret = HksGetProcessInfoForIPC(context, &processInfo);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksGetProcessInfoForIPC fail, ret = %" LOG_PUBLIC "d", ret)

        ret = HksServiceDeleteKey(&processInfo, srcData);
        HKS_IF_NOT_SUCC_LOGE(ret, "HksIpcServiceDeleteKey fail")
    } while (0);

    HksSendResponse(context, ret, NULL);

    HKS_FREE_BLOB(processInfo.processName);
    HKS_FREE_BLOB(processInfo.userId);
}

void HksIpcServiceGetKeyParamSet(const struct HksBlob *srcData, const uint8_t *context)
{
    struct HksBlob keyAlias = { 0, NULL };
    struct HksParamSet *paramSet = NULL;
    struct HksProcessInfo processInfo = { { 0, NULL }, { 0, NULL } };
    int32_t ret;

    do {
        ret = HksGetKeyParamSetUnpack(srcData, &keyAlias, &paramSet);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksGenerateKeyUnpack Ipc fail");
            return HksSendResponse(context, ret, NULL);
        }

        ret = HksGetProcessInfoForIPC(context, &processInfo);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksGetProcessInfoForIPC fail, ret = %" LOG_PUBLIC "d", ret)

        ret = HksServiceGetKeyParamSet(&processInfo, &keyAlias, paramSet);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksServiceGetKeyParamSet fail, ret = %" LOG_PUBLIC "d", ret)

        struct HksBlob paramSetOut = { paramSet->paramSetSize, (uint8_t *)paramSet };
        HksSendResponse(context, ret, &paramSetOut);
    } while (0);

    if (ret != HKS_SUCCESS) {
        HksSendResponse(context, ret, NULL);
    }

    HKS_FREE_PTR(paramSet);
    HKS_FREE_BLOB(processInfo.processName);
    HKS_FREE_BLOB(processInfo.userId);
}

void HksIpcServiceKeyExist(const struct HksBlob *srcData, const uint8_t *context)
{
    struct HksProcessInfo processInfo = { { 0, NULL }, { 0, NULL } };
    int32_t ret;

    do {
        ret = HksGetProcessInfoForIPC(context, &processInfo);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksGetProcessInfoForIPC fail, ret = %" LOG_PUBLIC "d", ret)

        ret = HksServiceKeyExist(&processInfo, srcData);
        HKS_IF_NOT_SUCC_LOGE(ret, "HksServiceKeyExist fail, ret = %" LOG_PUBLIC "d", ret)
    } while (0);

    HksSendResponse(context, ret, NULL);

    HKS_FREE_BLOB(processInfo.processName);
    HKS_FREE_BLOB(processInfo.userId);
}

void HksIpcServiceGenerateRandom(const struct HksBlob *srcData, const uint8_t *context)
{
    struct HksBlob processName = { 0, NULL };
    struct HksBlob random = { 0, NULL };
    int32_t ret;

    do {
        if ((srcData == NULL) || (srcData->data == NULL) || (srcData->size < sizeof(uint32_t))) {
            HKS_LOG_E("invalid srcData");
            ret = HKS_ERROR_INVALID_ARGUMENT;
            break;
        }

        random.size = *((uint32_t *)(srcData->data));
        if (IsInvalidLength(random.size)) {
            HKS_LOG_E("invalid size %" LOG_PUBLIC "u", random.size);
            ret = HKS_ERROR_INVALID_ARGUMENT;
            break;
        }

        random.data = (uint8_t *)HksMalloc(random.size);
        if (random.data == NULL) {
            ret = HKS_ERROR_MALLOC_FAIL;
            break;
        }

        ret = HksGetProcessNameForIPC(context, &processName);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksGetProcessNameForIPC fail, ret = %" LOG_PUBLIC "d", ret)

        ret = HksServiceGenerateRandom(&processName, &random);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksServiceGenerateRandom fail, ret = %" LOG_PUBLIC "d", ret)

        HksSendResponse(context, ret, &random);
    } while (0);

    if (ret != HKS_SUCCESS) {
        HksSendResponse(context, ret, NULL);
    }

    HKS_FREE_BLOB(random);
    HKS_FREE_BLOB(processName);
}

void HksIpcServiceSign(const struct HksBlob *srcData, const uint8_t *context)
{
    struct HksBlob keyAlias = { 0, NULL };
    struct HksParamSet *inParamSet = NULL;
    struct HksBlob unsignedData = { 0, NULL };
    struct HksBlob signature = { 0, NULL };
    struct HksProcessInfo processInfo = { { 0, NULL }, { 0, NULL } };
    int32_t ret;

    do {
        ret = HksSignUnpack(srcData, &keyAlias, &inParamSet, &unsignedData, &signature);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksSignUnpack Ipc fail")

        ret = HksGetProcessInfoForIPC(context, &processInfo);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksGetProcessInfoForIPC fail, ret = %" LOG_PUBLIC "d", ret)

        ret = HksServiceSign(&processInfo, &keyAlias, inParamSet, &unsignedData, &signature);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksServiceSign fail")

        HksSendResponse(context, ret, &signature);
    } while (0);

    if (ret != HKS_SUCCESS) {
        HksSendResponse(context, ret, NULL);
    }

    HKS_FREE_BLOB(signature);
    HKS_FREE_BLOB(processInfo.processName);
    HKS_FREE_BLOB(processInfo.userId);
}

void HksIpcServiceVerify(const struct HksBlob *srcData, const uint8_t *context)
{
    struct HksBlob keyAlias = { 0, NULL };
    struct HksParamSet *inParamSet = NULL;
    struct HksBlob unsignedData = { 0, NULL };
    struct HksBlob signature = { 0, NULL };
    struct HksProcessInfo processInfo = { { 0, NULL }, { 0, NULL } };
    int32_t ret;

    do {
        ret = HksVerifyUnpack(srcData, &keyAlias, &inParamSet, &unsignedData, &signature);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksVerifyUnpack Ipc fail")

        ret = HksGetProcessInfoForIPC(context, &processInfo);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksGetProcessInfoForIPC fail, ret = %" LOG_PUBLIC "d", ret)

        ret = HksServiceVerify(&processInfo, &keyAlias, inParamSet, &unsignedData, &signature);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksServiceVerify fail")
    } while (0);

    HksSendResponse(context, ret, NULL);

    HKS_FREE_BLOB(processInfo.processName);
    HKS_FREE_BLOB(processInfo.userId);
}

void HksIpcServiceEncrypt(const struct HksBlob *srcData, const uint8_t *context)
{
    struct HksBlob keyAlias = { 0, NULL };
    struct HksParamSet *inParamSet = NULL;
    struct HksBlob plainText = { 0, NULL };
    struct HksBlob cipherText = { 0, NULL };
    struct HksProcessInfo processInfo = { { 0, NULL }, { 0, NULL } };
    int32_t ret;

    do {
        ret = HksEncryptDecryptUnpack(srcData, &keyAlias, &inParamSet, &plainText, &cipherText);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksEncryptDecryptUnpack Ipc fail")

        ret = HksGetProcessInfoForIPC(context, &processInfo);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksGetProcessInfoForIPC fail, ret = %" LOG_PUBLIC "d", ret)

        ret = HksServiceEncrypt(&processInfo, &keyAlias, inParamSet, &plainText, &cipherText);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksServiceEncrypt fail")

        HksSendResponse(context, ret, &cipherText);
    } while (0);

    if (ret != HKS_SUCCESS) {
        HksSendResponse(context, ret, NULL);
    }

    HKS_FREE_BLOB(cipherText);
    HKS_FREE_BLOB(processInfo.processName);
    HKS_FREE_BLOB(processInfo.userId);
}

void HksIpcServiceDecrypt(const struct HksBlob *srcData, const uint8_t *context)
{
    struct HksBlob keyAlias = { 0, NULL };
    struct HksParamSet *inParamSet = NULL;
    struct HksBlob plainText = { 0, NULL };
    struct HksBlob cipherText = { 0, NULL };
    struct HksProcessInfo processInfo = { { 0, NULL }, { 0, NULL } };
    int32_t ret;

    do {
        ret = HksEncryptDecryptUnpack(srcData, &keyAlias, &inParamSet, &cipherText, &plainText);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksEncryptDecryptUnpack Ipc fail")

        ret = HksGetProcessInfoForIPC(context, &processInfo);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksGetProcessInfoForIPC fail, ret = %" LOG_PUBLIC "d", ret)

        ret = HksServiceDecrypt(&processInfo, &keyAlias, inParamSet, &cipherText, &plainText);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksServiceDecrypt fail")

        HksSendResponse(context, ret, &plainText);
    } while (0);

    if (ret != HKS_SUCCESS) {
        HksSendResponse(context, ret, NULL);
    }

    HKS_FREE_BLOB(plainText);
    HKS_FREE_BLOB(processInfo.processName);
    HKS_FREE_BLOB(processInfo.userId);
}

void HksIpcServiceAgreeKey(const struct HksBlob *srcData, const uint8_t *context)
{
    struct HksBlob privateKey = { 0, NULL };
    struct HksBlob peerPublicKey = { 0, NULL };
    struct HksBlob agreedKey = { 0, NULL };
    struct HksParamSet *inParamSet = NULL;
    struct HksProcessInfo processInfo = { { 0, NULL }, { 0, NULL } };
    int32_t ret;

    do {
        ret = HksAgreeKeyUnpack(srcData, &inParamSet, &privateKey, &peerPublicKey, &agreedKey);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksAgreeKeyUnpack Ipc fail")

        ret = HksGetProcessInfoForIPC(context, &processInfo);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksGetProcessInfoForIPC fail, ret = %" LOG_PUBLIC "d", ret)

        ret = HksServiceAgreeKey(&processInfo, inParamSet, &privateKey, &peerPublicKey, &agreedKey);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksServiceAgreeKey fail, ret = %" LOG_PUBLIC "d", ret)

        HksSendResponse(context, ret, &agreedKey);
    } while (0);

    if (ret != HKS_SUCCESS) {
        HksSendResponse(context, ret, NULL);
    }

    HKS_FREE_BLOB(agreedKey);
    HKS_FREE_BLOB(processInfo.processName);
    HKS_FREE_BLOB(processInfo.userId);
}

void HksIpcServiceDeriveKey(const struct HksBlob *srcData, const uint8_t *context)
{
    struct HksBlob masterKey = { 0, NULL };
    struct HksBlob derivedKey = { 0, NULL };
    struct HksParamSet *inParamSet = NULL;
    struct HksProcessInfo processInfo = { { 0, NULL }, { 0, NULL } };
    int32_t ret;

    do {
        ret = HksDeriveKeyUnpack(srcData, &inParamSet, &masterKey, &derivedKey);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksDeriveKeyUnpack Ipc fail")

        ret = HksGetProcessInfoForIPC(context, &processInfo);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksGetProcessInfoForIPC fail, ret = %" LOG_PUBLIC "d", ret)

        ret = HksServiceDeriveKey(&processInfo, inParamSet, &masterKey, &derivedKey);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksServiceDeriveKey fail, ret = %" LOG_PUBLIC "d", ret)

        HksSendResponse(context, ret, &derivedKey);
    } while (0);

    if (ret != HKS_SUCCESS) {
        HksSendResponse(context, ret, NULL);
    }

    HKS_FREE_BLOB(derivedKey);
    HKS_FREE_BLOB(processInfo.processName);
    HKS_FREE_BLOB(processInfo.userId);
}

void HksIpcServiceMac(const struct HksBlob *srcData, const uint8_t *context)
{
    struct HksBlob key = { 0, NULL };
    struct HksParamSet *inParamSet = NULL;
    struct HksBlob inputData = { 0, NULL };
    struct HksBlob mac = { 0, NULL };
    struct HksProcessInfo processInfo = { { 0, NULL }, { 0, NULL } };
    int32_t ret;

    do {
        ret = HksHmacUnpack(srcData, &key, &inParamSet, &inputData, &mac);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksHmacUnpack Ipc fail")

        ret = HksGetProcessInfoForIPC(context, &processInfo);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksGetProcessInfoForIPC fail, ret = %" LOG_PUBLIC "d", ret)

        ret = HksServiceMac(&processInfo, &key, inParamSet, &inputData, &mac);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksServiceMac fail, ret = %" LOG_PUBLIC "d", ret)

        HksSendResponse(context, ret, &mac);
    } while (0);

    if (ret != HKS_SUCCESS) {
        HksSendResponse(context, ret, NULL);
    }

    HKS_FREE_BLOB(mac);
    HKS_FREE_BLOB(processInfo.processName);
    HKS_FREE_BLOB(processInfo.userId);
}

static void FreeKeyInfo(uint32_t listCount, struct HksKeyInfo **keyInfoList)
{
    if ((keyInfoList == NULL) || (*keyInfoList == NULL)) {
        return;
    }

    for (uint32_t i = 0; i < listCount; ++i) {
        if ((*keyInfoList)[i].alias.data != NULL) {
            HKS_FREE_BLOB((*keyInfoList)[i].alias);
        }
        if ((*keyInfoList)[i].paramSet != NULL) {
            HksFree((*keyInfoList)[i].paramSet);
            (*keyInfoList)[i].paramSet = NULL;
        }
    }

    HKS_FREE_PTR(*keyInfoList);
}

void HksIpcServiceGetKeyInfoList(const struct HksBlob *srcData, const uint8_t *context)
{
    uint32_t inputCount = 0;
    struct HksKeyInfo *keyInfoList = NULL;
    struct HksBlob keyInfoListBlob = { 0, NULL };
    struct HksProcessInfo processInfo = { { 0, NULL }, { 0, NULL } };
    int32_t ret;

    do {
        ret = HksGetKeyInfoListUnpack(srcData, &inputCount, &keyInfoList);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksGetKeyInfoListUnpack Ipc fail")

        ret = HksGetProcessInfoForIPC(context, &processInfo);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksGetProcessInfoForIPC fail, ret = %" LOG_PUBLIC "d", ret)

        uint32_t listCount = inputCount;
        ret = HksServiceGetKeyInfoList(&processInfo, keyInfoList, &listCount);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksServiceGetKeyInfoList fail, ret = %" LOG_PUBLIC "d", ret)

        keyInfoListBlob.size = sizeof(listCount);
        for (uint32_t i = 0; i < listCount; ++i) {
            keyInfoListBlob.size += sizeof(keyInfoList[i].alias.size) + ALIGN_SIZE(keyInfoList[i].alias.size) +
                ALIGN_SIZE(keyInfoList[i].paramSet->paramSetSize);
        }

        keyInfoListBlob.data = (uint8_t *)HksMalloc(keyInfoListBlob.size);
        if (keyInfoListBlob.data == NULL) {
            ret = HKS_ERROR_MALLOC_FAIL;
            break;
        }

        ret = HksGetKeyInfoListPackFromService(&keyInfoListBlob, listCount, keyInfoList);
        HKS_IF_NOT_SUCC_BREAK(ret)

        HksSendResponse(context, ret, &keyInfoListBlob);
    } while (0);

    if (ret != HKS_SUCCESS) {
        HksSendResponse(context, ret, NULL);
    }

    FreeKeyInfo(inputCount, &keyInfoList);
    HKS_FREE_BLOB(keyInfoListBlob);
    HKS_FREE_BLOB(processInfo.processName);
    HKS_FREE_BLOB(processInfo.userId);
}

int32_t HksAttestAccessControl(struct HksParamSet *paramSet)
{
#ifdef HKS_SUPPORT_ACCESS_TOKEN
    for (uint32_t i = 0; i < sizeof(g_idList) / sizeof(g_idList[0]); i++) {
        for (uint32_t j = 0; j < paramSet->paramsCnt; j++) {
            if (paramSet->params[j].tag == g_idList[i]) {
                return SensitivePermissionCheck();
            }
        }
    }
#endif
    return HKS_SUCCESS;
}

void HksIpcServiceAttestKey(const struct HksBlob *srcData, const uint8_t *context)
{
    struct HksBlob keyAlias = { 0, NULL };
    struct HksParamSet *inParamSet = NULL;
    struct HksBlob certChainBlob = { 0, NULL };
    struct HksProcessInfo processInfo = { { 0, NULL }, { 0, NULL } };
    int32_t ret;

    do {
        ret = HksCertificateChainUnpack(srcData, &keyAlias, &inParamSet, &certChainBlob);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksCertificateChainUnpack Ipc fail")

        ret = HksGetProcessInfoForIPC(context, &processInfo);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksGetProcessInfoForIPC fail, ret = %" LOG_PUBLIC "d", ret)

        ret = HksAttestAccessControl(inParamSet);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksAttestAccessControl fail, ret = %" LOG_PUBLIC "d", ret)

        ret = HksServiceAttestKey(&processInfo, &keyAlias, inParamSet, &certChainBlob);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksServiceAttestKey fail, ret = %" LOG_PUBLIC "d", ret)

        HksSendResponse(context, ret, &certChainBlob);
    } while (0);

    if (ret != HKS_SUCCESS) {
        HksSendResponse(context, ret, NULL);
    }

    HKS_FREE_BLOB(processInfo.processName);
    HKS_FREE_BLOB(processInfo.userId);
    HKS_FREE_BLOB(certChainBlob);
}

static int32_t IpcServiceInit(const struct HksProcessInfo *processInfo, const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSet, struct HksBlob *outData)
{
    uint8_t handleData[HANDLE_SIZE] = {0};
    uint8_t tokenData[TOKEN_SIZE] = {0};
    struct HksBlob handle = { sizeof(handleData), handleData };
    struct HksBlob token = { sizeof(tokenData), tokenData };

    int32_t ret = HksServiceInit(processInfo, keyAlias, paramSet, &handle, &token);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "service init failed, ret = %" LOG_PUBLIC "d", ret)

    if ((handle.size != HANDLE_SIZE) || (token.size > TOKEN_SIZE)) {
        HKS_LOG_E("invalid handle size[%" LOG_PUBLIC "u], or token size[%" LOG_PUBLIC "u]", handle.size, token.size);
        return HKS_ERROR_BAD_STATE;
    }

    if (outData->size < (handle.size + token.size)) {
        HKS_LOG_E("ipc out size[%" LOG_PUBLIC "u] too small", outData->size);
        return HKS_ERROR_BUFFER_TOO_SMALL;
    }

    (void)memcpy_s(outData->data, outData->size, handle.data, handle.size);

    if (token.size != 0 &&
        memcpy_s(outData->data + handle.size, outData->size - handle.size, token.data, token.size) != EOK) {
        HKS_LOG_E("copy token failed");
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    outData->size = handle.size + token.size;

    return HKS_SUCCESS;
}

void HksIpcServiceInit(const struct HksBlob *paramSetBlob, struct HksBlob *outData, const uint8_t *context)
{
    int32_t ret;
    struct HksParamSet *inParamSet = NULL;
    struct HksParamSet *paramSet   = NULL;
    struct HksBlob keyAlias        = { 0, NULL };
    struct HksBlob paramsBlob      = { 0, NULL };
    struct HksProcessInfo processInfo = { { 0, NULL }, { 0, NULL } };

    do {
        ret = HksGetParamSet((struct HksParamSet *)paramSetBlob->data, paramSetBlob->size, &paramSet);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksGetParamSet fail, ret = %" LOG_PUBLIC "d", ret)

        struct HksParamOut params[] = {
            {
                .tag = HKS_TAG_PARAM0_BUFFER,
                .blob = &keyAlias
            }, {
                .tag = HKS_TAG_PARAM1_BUFFER,
                .blob = &paramsBlob
            },
        };

        ret = HksParamSetToParams(paramSet, params, HKS_ARRAY_SIZE(params));
        HKS_IF_NOT_SUCC_BREAK(ret)

        ret = HksGetParamSet((struct HksParamSet *)paramsBlob.data, paramsBlob.size, &inParamSet);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksGetParamSet fail, ret = %" LOG_PUBLIC "d", ret)

        ret = HksGetProcessInfoForIPC(context, &processInfo);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksGetProcessInfoForIPC fail, ret = %" LOG_PUBLIC "d", ret)

        ret = IpcServiceInit(&processInfo, &keyAlias, inParamSet, outData);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "ipc service init fail, ret = %" LOG_PUBLIC "d", ret)
    } while (0);

    HksSendResponse(context, ret, outData);
    HksFreeParamSet(&paramSet);
    HksFreeParamSet(&inParamSet);
    HKS_FREE_BLOB(processInfo.processName);
    HKS_FREE_BLOB(processInfo.userId);
}

void HksIpcServiceUpdate(const struct HksBlob *paramSetBlob, struct HksBlob *outData, const uint8_t *context)
{
    int32_t ret;
    struct HksParamSet *inParamSet = NULL;
    struct HksParamSet *paramSet   = NULL;
    struct HksBlob paramsBlob      = { 0, NULL };
    struct HksBlob handle          = { 0, NULL };
    struct HksBlob inData          = { 0, NULL };
    struct HksProcessInfo processInfo = { { 0, NULL }, { 0, NULL } };

    do {
        ret = HksGetParamSet((struct HksParamSet *)paramSetBlob->data, paramSetBlob->size, &paramSet);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksGetParamSet fail, ret = %" LOG_PUBLIC "d", ret)

        struct HksParamOut params[] = {
            {
                .tag = HKS_TAG_PARAM0_BUFFER,
                .blob = &paramsBlob
            }, {
                .tag = HKS_TAG_PARAM1_BUFFER,
                .blob = &handle
            }, {
                .tag = HKS_TAG_PARAM2_BUFFER,
                .blob = &inData
            },
        };

        ret = HksParamSetToParams(paramSet, params, HKS_ARRAY_SIZE(params));
        HKS_IF_NOT_SUCC_BREAK(ret)

        ret = HksGetParamSet((struct HksParamSet *)paramsBlob.data, paramsBlob.size, &inParamSet);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksGetParamSet fail, ret = %" LOG_PUBLIC "d", ret)

        ret = HksGetProcessInfoForIPC(context, &processInfo);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksGetProcessInfoForIPC fail, ret = %" LOG_PUBLIC "d", ret)

        ret = HksServiceUpdate(&handle, &processInfo, inParamSet, &inData, outData);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksServiceUpdate fail, ret = %" LOG_PUBLIC "d", ret)
    } while (0);

    HksSendResponse(context, ret, outData);
    HksFreeParamSet(&paramSet);
    HksFreeParamSet(&inParamSet);
    HKS_FREE_BLOB(processInfo.processName);
    HKS_FREE_BLOB(processInfo.userId);
}

void HksIpcServiceFinish(const struct HksBlob *paramSetBlob, struct HksBlob *outData, const uint8_t *context)
{
    int32_t ret;
    struct HksParamSet *inParamSet = NULL;
    struct HksParamSet *paramSet   = NULL;
    struct HksBlob paramsBlob      = { 0, NULL };
    struct HksBlob handle          = { 0, NULL };
    struct HksBlob inData          = { 0, NULL };
    struct HksProcessInfo processInfo = { { 0, NULL }, { 0, NULL } };

    do {
        ret = HksGetParamSet((struct HksParamSet *)paramSetBlob->data, paramSetBlob->size, &paramSet);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksGetParamSet fail, ret = %" LOG_PUBLIC "d", ret)

        struct HksParamOut params[] = {
            { .tag = HKS_TAG_PARAM0_BUFFER, .blob = &paramsBlob },
            { .tag = HKS_TAG_PARAM1_BUFFER, .blob = &handle },
            { .tag = HKS_TAG_PARAM2_BUFFER, .blob = &inData },
        };

        ret = HksParamSetToParams(paramSet, params, HKS_ARRAY_SIZE(params));
        HKS_IF_NOT_SUCC_BREAK(ret)

        ret = HksGetParamSet((struct HksParamSet *)paramsBlob.data, paramsBlob.size, &inParamSet);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksGetParamSet fail, ret = %" LOG_PUBLIC "d", ret)

        ret = HksGetProcessInfoForIPC(context, &processInfo);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksGetProcessInfoForIPC fail, ret = %" LOG_PUBLIC "d", ret)

        ret = HksServiceFinish(&handle, &processInfo, inParamSet, &inData, outData);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksServiceFinish fail, ret = %" LOG_PUBLIC "d", ret)
    } while (0);

    HksSendResponse(context, ret, outData);
    HksFreeParamSet(&paramSet);
    HksFreeParamSet(&inParamSet);
    HKS_FREE_BLOB(processInfo.processName);
    HKS_FREE_BLOB(processInfo.userId);
}

void HksIpcServiceAbort(const struct HksBlob *paramSetBlob, struct HksBlob *outData, const uint8_t *context)
{
    (void)outData;
    int32_t ret;
    struct HksParamSet *inParamSet = NULL;
    struct HksParamSet *paramSet   = NULL;
    struct HksBlob handle          = { 0, NULL };
    struct HksBlob paramsBlob      = { 0, NULL };
    struct HksProcessInfo processInfo = { { 0, NULL }, { 0, NULL } };

    do {
        ret = HksGetParamSet((struct HksParamSet *)paramSetBlob->data, paramSetBlob->size, &paramSet);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksGetParamSet fail, ret = %" LOG_PUBLIC "d", ret)

        struct HksParamOut params[] = {
            {
                .tag = HKS_TAG_PARAM0_BUFFER,
                .blob = &paramsBlob
            }, {
                .tag = HKS_TAG_PARAM1_BUFFER,
                .blob = &handle
            },
        };
        ret = HksParamSetToParams(paramSet, params, HKS_ARRAY_SIZE(params));
        HKS_IF_NOT_SUCC_BREAK(ret)

        ret = HksGetParamSet((struct HksParamSet *)paramsBlob.data, paramsBlob.size, &inParamSet);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksGetParamSet fail, ret = %" LOG_PUBLIC "d", ret)

        ret = HksGetProcessInfoForIPC(context, &processInfo);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksGetProcessInfoForIPC fail, ret = %" LOG_PUBLIC "d", ret)

        ret = HksServiceAbort(&handle, &processInfo, inParamSet);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "HksServiceAbort fail, ret = %" LOG_PUBLIC "d", ret)
    } while (0);

    HksSendResponse(context, ret, NULL);
    HksFreeParamSet(&paramSet);
    HksFreeParamSet(&inParamSet);
    HKS_FREE_BLOB(processInfo.processName);
    HKS_FREE_BLOB(processInfo.userId);
}
