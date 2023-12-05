/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifdef HKS_CONFIG_FILE
#include HKS_CONFIG_FILE
#else
#include "hks_config.h"
#endif

#include "huks_access.h"

#include "hks_cfi.h"
#include "huks_core_hal.h"

#include "hks_log.h"
#include "hks_mem.h"
#include "hks_template.h"

static struct HuksHdi *g_hksHalDevicePtr = NULL;

#ifndef _CUT_AUTHENTICATE_
ENABLE_CFI(int32_t HuksAccessModuleInit(void))
{
    HKS_IF_NOT_SUCC_RETURN(HksCreateHuksHdiDevice(&g_hksHalDevicePtr), HKS_ERROR_NULL_POINTER)

    HKS_IF_NULL_LOGE_RETURN(g_hksHalDevicePtr->HuksHdiModuleInit, HKS_ERROR_NULL_POINTER,
        "Module Init function is null pointer")

    return g_hksHalDevicePtr->HuksHdiModuleInit();
}

ENABLE_CFI(int32_t HuksAccessRefresh(void))
{
    HKS_IF_NOT_SUCC_RETURN(HksCreateHuksHdiDevice(&g_hksHalDevicePtr), HKS_ERROR_NULL_POINTER)

    HKS_IF_NULL_LOGE_RETURN(g_hksHalDevicePtr->HuksHdiRefresh, HKS_ERROR_NULL_POINTER,
        "Refresh function is null pointer")

    return g_hksHalDevicePtr->HuksHdiRefresh();
}

ENABLE_CFI(int32_t HuksAccessGenerateKey(const struct HksBlob *keyAlias, const struct HksParamSet *paramSetIn,
    const struct HksBlob *keyIn, struct HksBlob *keyOut))
{
    HKS_IF_NOT_SUCC_RETURN(HksCreateHuksHdiDevice(&g_hksHalDevicePtr), HKS_ERROR_NULL_POINTER)

    HKS_IF_NULL_LOGE_RETURN(g_hksHalDevicePtr->HuksHdiGenerateKey, HKS_ERROR_NULL_POINTER,
        "GenerateKey function is null pointer")

    return g_hksHalDevicePtr->HuksHdiGenerateKey(keyAlias, paramSetIn, keyIn, keyOut);
}

ENABLE_CFI(int32_t HuksAccessImportKey(const struct HksBlob *keyAlias, const struct HksBlob *key,
    const struct HksParamSet *paramSet, struct HksBlob *keyOut))
{
    HKS_IF_NOT_SUCC_RETURN(HksCreateHuksHdiDevice(&g_hksHalDevicePtr), HKS_ERROR_NULL_POINTER)

    HKS_IF_NULL_LOGE_RETURN(g_hksHalDevicePtr->HuksHdiImportKey, HKS_ERROR_NULL_POINTER,
        "ImportKey function is null pointer")

    return g_hksHalDevicePtr->HuksHdiImportKey(keyAlias, key, paramSet, keyOut);
}

ENABLE_CFI(int32_t HuksAccessImportWrappedKey(const struct HksBlob *wrappingKeyAlias, const struct HksBlob *key,
    const struct HksBlob *wrappedKeyData, const struct HksParamSet *paramSet, struct HksBlob *keyOut))
{
    HKS_IF_NOT_SUCC_RETURN(HksCreateHuksHdiDevice(&g_hksHalDevicePtr), HKS_ERROR_NULL_POINTER)

    HKS_IF_NULL_LOGE_RETURN(g_hksHalDevicePtr->HuksHdiImportWrappedKey, HKS_ERROR_NULL_POINTER,
        "ImportWrappedKey function is null pointer")

    return g_hksHalDevicePtr->HuksHdiImportWrappedKey(wrappingKeyAlias, key, wrappedKeyData, paramSet, keyOut);
}

ENABLE_CFI(int32_t HuksAccessExportPublicKey(const struct HksBlob *key, const struct HksParamSet *paramSet,
    struct HksBlob *keyOut))
{
    HKS_IF_NOT_SUCC_RETURN(HksCreateHuksHdiDevice(&g_hksHalDevicePtr), HKS_ERROR_NULL_POINTER)

    HKS_IF_NULL_LOGE_RETURN(g_hksHalDevicePtr->HuksHdiExportPublicKey, HKS_ERROR_NULL_POINTER,
        "ExportPublicKey function is null pointer")

    return g_hksHalDevicePtr->HuksHdiExportPublicKey(key, paramSet, keyOut);
}

ENABLE_CFI(int32_t HuksAccessInit(const struct  HksBlob *key, const struct HksParamSet *paramSet,
    struct HksBlob *handle, struct HksBlob *token))
{
    HKS_IF_NOT_SUCC_RETURN(HksCreateHuksHdiDevice(&g_hksHalDevicePtr), HKS_ERROR_NULL_POINTER)

    HKS_IF_NULL_LOGE_RETURN(g_hksHalDevicePtr->HuksHdiInit, HKS_ERROR_NULL_POINTER,
        "Init function is null pointer")

    return g_hksHalDevicePtr->HuksHdiInit(key, paramSet, handle, token);
}

ENABLE_CFI(int32_t HuksAccessUpdate(const struct HksBlob *handle, const struct HksParamSet *paramSet,
    const struct HksBlob *inData, struct HksBlob *outData))
{
    HKS_IF_NOT_SUCC_RETURN(HksCreateHuksHdiDevice(&g_hksHalDevicePtr), HKS_ERROR_NULL_POINTER)

    HKS_IF_NULL_LOGE_RETURN(g_hksHalDevicePtr->HuksHdiUpdate, HKS_ERROR_NULL_POINTER,
        "Update function is null pointer")

    return g_hksHalDevicePtr->HuksHdiUpdate(handle, paramSet, inData, outData);
}

ENABLE_CFI(int32_t HuksAccessFinish(const struct HksBlob *handle, const struct HksParamSet *paramSet,
    const struct HksBlob *inData, struct HksBlob *outData))
{
    HKS_IF_NOT_SUCC_RETURN(HksCreateHuksHdiDevice(&g_hksHalDevicePtr), HKS_ERROR_NULL_POINTER)

    HKS_IF_NULL_LOGE_RETURN(g_hksHalDevicePtr->HuksHdiFinish, HKS_ERROR_NULL_POINTER,
        "Finish function is null pointer")

    return g_hksHalDevicePtr->HuksHdiFinish(handle, paramSet, inData, outData);
}

ENABLE_CFI(int32_t HuksAccessAbort(const struct HksBlob *handle, const struct HksParamSet *paramSet))
{
    HKS_IF_NOT_SUCC_RETURN(HksCreateHuksHdiDevice(&g_hksHalDevicePtr), HKS_ERROR_NULL_POINTER)

    HKS_IF_NULL_LOGE_RETURN(g_hksHalDevicePtr->HuksHdiAbort, HKS_ERROR_NULL_POINTER,
        "Abort function is null pointer")

    return g_hksHalDevicePtr->HuksHdiAbort(handle, paramSet);
}

ENABLE_CFI(int32_t HuksAccessGetKeyProperties(const struct HksParamSet *paramSet, const struct HksBlob *key))
{
    HKS_IF_NOT_SUCC_RETURN(HksCreateHuksHdiDevice(&g_hksHalDevicePtr), HKS_ERROR_NULL_POINTER)

    HKS_IF_NULL_LOGE_RETURN(g_hksHalDevicePtr->HuksHdiGetKeyProperties, HKS_ERROR_NULL_POINTER,
        "GetKeyProperties function is null pointer")

    return g_hksHalDevicePtr->HuksHdiGetKeyProperties(paramSet, key);
}

ENABLE_CFI(int32_t HuksAccessGetAbility(int funcType))
{
    HKS_IF_NOT_SUCC_RETURN(HksCreateHuksHdiDevice(&g_hksHalDevicePtr), HKS_ERROR_NULL_POINTER)

    HKS_IF_NULL_LOGE_RETURN(g_hksHalDevicePtr->HuksHdiGetAbility, HKS_ERROR_NULL_POINTER,
        "GetAbility function is null pointer")

    return g_hksHalDevicePtr->HuksHdiGetAbility(funcType);
}

ENABLE_CFI(int32_t HuksAccessGetHardwareInfo(void))
{
    HKS_IF_NOT_SUCC_RETURN(HksCreateHuksHdiDevice(&g_hksHalDevicePtr), HKS_ERROR_NULL_POINTER)

    HKS_IF_NULL_LOGE_RETURN(g_hksHalDevicePtr->HuksHdiGetHardwareInfo, HKS_ERROR_NULL_POINTER,
        "GetHardwareInfo function is null pointer")

    return g_hksHalDevicePtr->HuksHdiGetHardwareInfo();
}

ENABLE_CFI(int32_t HuksAccessSign(const struct HksBlob *key, const struct HksParamSet *paramSet,
    const struct HksBlob *srcData, struct HksBlob *signature))
{
    HKS_IF_NOT_SUCC_RETURN(HksCreateHuksHdiDevice(&g_hksHalDevicePtr), HKS_ERROR_NULL_POINTER)

    HKS_IF_NULL_LOGE_RETURN(g_hksHalDevicePtr->HuksHdiSign, HKS_ERROR_NULL_POINTER,
        "Sign function is null pointer")

    return g_hksHalDevicePtr->HuksHdiSign(key, paramSet, srcData, signature);
}

ENABLE_CFI(int32_t HuksAccessVerify(const struct HksBlob *key, const struct HksParamSet *paramSet,
    const struct HksBlob *srcData, const struct HksBlob *signature))
{
    HKS_IF_NOT_SUCC_RETURN(HksCreateHuksHdiDevice(&g_hksHalDevicePtr), HKS_ERROR_NULL_POINTER)

    HKS_IF_NULL_LOGE_RETURN(g_hksHalDevicePtr->HuksHdiVerify, HKS_ERROR_NULL_POINTER,
        "Verify function is null pointer")

    return g_hksHalDevicePtr->HuksHdiVerify(key, paramSet, srcData, signature);
}

ENABLE_CFI(int32_t HuksAccessEncrypt(const struct HksBlob *key, const struct HksParamSet *paramSet,
    const struct HksBlob *plainText, struct HksBlob *cipherText))
{
    HKS_IF_NOT_SUCC_RETURN(HksCreateHuksHdiDevice(&g_hksHalDevicePtr), HKS_ERROR_NULL_POINTER)

    HKS_IF_NULL_LOGE_RETURN(g_hksHalDevicePtr->HuksHdiEncrypt, HKS_ERROR_NULL_POINTER,
        "Encrypt function is null pointer")

    return g_hksHalDevicePtr->HuksHdiEncrypt(key, paramSet, plainText, cipherText);
}

ENABLE_CFI(int32_t HuksAccessDecrypt(const struct HksBlob *key, const struct HksParamSet *paramSet,
    const struct HksBlob *cipherText, struct HksBlob *plainText))
{
    HKS_IF_NOT_SUCC_RETURN(HksCreateHuksHdiDevice(&g_hksHalDevicePtr), HKS_ERROR_NULL_POINTER)

    HKS_IF_NULL_LOGE_RETURN(g_hksHalDevicePtr->HuksHdiDecrypt, HKS_ERROR_NULL_POINTER,
        "Decrypt function is null pointer")

    return g_hksHalDevicePtr->HuksHdiDecrypt(key, paramSet, cipherText, plainText);
}

ENABLE_CFI(int32_t HuksAccessAgreeKey(const struct HksParamSet *paramSet, const struct HksBlob *privateKey,
    const struct HksBlob *peerPublicKey, struct HksBlob *agreedKey))
{
    HKS_IF_NOT_SUCC_RETURN(HksCreateHuksHdiDevice(&g_hksHalDevicePtr), HKS_ERROR_NULL_POINTER)

    HKS_IF_NULL_LOGE_RETURN(g_hksHalDevicePtr->HuksHdiAgreeKey, HKS_ERROR_NULL_POINTER,
        "AgreeKey function is null pointer")

    return g_hksHalDevicePtr->HuksHdiAgreeKey(paramSet, privateKey, peerPublicKey, agreedKey);
}

ENABLE_CFI(int32_t HuksAccessDeriveKey(const struct HksParamSet *paramSet, const struct HksBlob *kdfKey,
    struct HksBlob *derivedKey))
{
    HKS_IF_NOT_SUCC_RETURN(HksCreateHuksHdiDevice(&g_hksHalDevicePtr), HKS_ERROR_NULL_POINTER)

    HKS_IF_NULL_LOGE_RETURN(g_hksHalDevicePtr->HuksHdiDeriveKey, HKS_ERROR_NULL_POINTER,
        "DeriveKey function is null pointer")

    return g_hksHalDevicePtr->HuksHdiDeriveKey(paramSet, kdfKey, derivedKey);
}

ENABLE_CFI(int32_t HuksAccessMac(const struct HksBlob *key, const struct HksParamSet *paramSet,
    const struct HksBlob *srcData, struct HksBlob *mac))
{
    HKS_IF_NOT_SUCC_RETURN(HksCreateHuksHdiDevice(&g_hksHalDevicePtr), HKS_ERROR_NULL_POINTER)

    HKS_IF_NULL_LOGE_RETURN(g_hksHalDevicePtr->HuksHdiMac, HKS_ERROR_NULL_POINTER,
        "Mac function is null pointer")

    return g_hksHalDevicePtr->HuksHdiMac(key, paramSet, srcData, mac);
}

#ifdef _STORAGE_LITE_
ENABLE_CFI(int32_t HuksAccessCalcHeaderMac(const struct HksParamSet *paramSet, const struct HksBlob *salt,
    const struct HksBlob *srcData, struct HksBlob *mac))
{
    HKS_IF_NOT_SUCC_RETURN(HksCreateHuksHdiDevice(&g_hksHalDevicePtr), HKS_ERROR_NULL_POINTER)

    HKS_IF_NULL_LOGE_RETURN(g_hksHalDevicePtr->HuksHdiCalcMacHeader, HKS_ERROR_NULL_POINTER,
        "CalcMacHeader function is null pointer")

    return g_hksHalDevicePtr->HuksHdiCalcMacHeader(paramSet, salt, srcData, mac);
}
#endif

#ifdef HKS_SUPPORT_UPGRADE_STORAGE_DATA
ENABLE_CFI(int32_t HuksAccessUpgradeKeyInfo(const struct HksBlob *keyAlias, const struct HksBlob *keyInfo,
    struct HksBlob *keyOut))
{
    HKS_IF_NOT_SUCC_RETURN(HksCreateHuksHdiDevice(&g_hksHalDevicePtr), HKS_ERROR_NULL_POINTER)

    HKS_IF_NULL_LOGE_RETURN(g_hksHalDevicePtr->HuksHdiUpgradeKeyInfo, HKS_ERROR_NULL_POINTER,
        "UpgradeKeyInfo function is null pointer")

    return g_hksHalDevicePtr->HuksHdiUpgradeKeyInfo(keyAlias, keyInfo, keyOut);
}
#endif

#ifdef HKS_SUPPORT_API_ATTEST_KEY
ENABLE_CFI(int32_t HuksAccessAttestKey(const struct HksBlob *key, const struct HksParamSet *paramSet,
    struct HksBlob *certChain))
{
    HKS_IF_NOT_SUCC_RETURN(HksCreateHuksHdiDevice(&g_hksHalDevicePtr), HKS_ERROR_NULL_POINTER)

    HKS_IF_NULL_LOGE_RETURN(g_hksHalDevicePtr->HuksHdiAttestKey, HKS_ERROR_NULL_POINTER,
        "AttestKey function is null pointer")

    return g_hksHalDevicePtr->HuksHdiAttestKey(key, paramSet, certChain);
}
#endif

#endif /* _CUT_AUTHENTICATE_ */

ENABLE_CFI(int32_t HuksAccessGenerateRandom(const struct HksParamSet *paramSet, struct HksBlob *random))
{
    HKS_IF_NOT_SUCC_RETURN(HksCreateHuksHdiDevice(&g_hksHalDevicePtr), HKS_ERROR_NULL_POINTER)

    HKS_IF_NULL_LOGE_RETURN(g_hksHalDevicePtr->HuksHdiGenerateRandom, HKS_ERROR_NULL_POINTER,
        "GenerateRandom function is null pointer")

    return g_hksHalDevicePtr->HuksHdiGenerateRandom(paramSet, random);
}
