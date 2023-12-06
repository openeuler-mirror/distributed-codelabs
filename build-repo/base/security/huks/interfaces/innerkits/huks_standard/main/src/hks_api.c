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

#ifdef HKS_CONFIG_FILE
#include HKS_CONFIG_FILE
#else
#include "hks_config.h"
#endif

#include "hks_api.h"

#include <stddef.h>
#include <string.h>

#include "hks_api_adapter.h"
#include "hks_client_ipc.h"
#include "hks_local_engine.h"
#include "hks_ability.h"
#include "hks_log.h"
#include "hks_param.h"
#include "hks_template.h"
#include "hks_type.h"
#include "securec.h"

#ifdef HKS_SUPPORT_API_ATTEST_KEY
#include "hks_verifier.h"
#endif

#ifdef _CUT_AUTHENTICATE_
#undef HKS_SUPPORT_API_GENERATE_KEY
#undef HKS_SUPPORT_API_IMPORT
#undef HKS_SUPPORT_API_EXPORT
#undef HKS_SUPPORT_API_DELETE_KEY
#undef HKS_SUPPORT_API_GET_KEY_PARAM_SET
#undef HKS_SUPPORT_API_KEY_EXIST
#undef HKS_SUPPORT_API_SIGN_VERIFY
#undef HKS_SUPPORT_API_SIGN_VERIFY
#undef HKS_SUPPORT_API_AGREE_KEY
#undef HKS_SUPPORT_API_HASH
#undef HKS_SUPPORT_API_GET_KEY_INFO_LIST
#undef HKS_SUPPORT_API_ATTEST_KEY
#undef HKS_SUPPORT_API_GET_CERTIFICATE_CHAIN
#endif

HKS_API_EXPORT int32_t HksGetSdkVersion(struct HksBlob *sdkVersion)
{
    if ((sdkVersion == NULL) || (sdkVersion->data == NULL)) {
        return HKS_ERROR_NULL_POINTER;
    }

    uint32_t versionLen = strlen(HKS_SDK_VERSION);
    if (sdkVersion->size < (versionLen + 1)) {
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    (void)memcpy_s(sdkVersion->data, sdkVersion->size, HKS_SDK_VERSION, versionLen);

    sdkVersion->data[versionLen] = '\0';
    sdkVersion->size = versionLen;
    return HKS_SUCCESS;
}

HKS_API_EXPORT int32_t HksInitialize(void)
{
#ifndef _CUT_AUTHENTICATE_
    HKS_LOG_I("enter initialize");
    int32_t ret = HksClientInitialize();
    HKS_LOG_I("leave initialize, result = %" LOG_PUBLIC "d", ret);
    return ret;
#else
    (void)HksCryptoAbilityInit();
    return HKS_SUCCESS;
#endif
}

HKS_API_EXPORT int32_t HksRefreshKeyInfo(void)
{
#ifndef _CUT_AUTHENTICATE_
    HKS_LOG_I("enter refresh key info");
    int32_t ret = HksClientRefreshKeyInfo();
    HKS_LOG_I("leave refresh key info, result = %" LOG_PUBLIC "d", ret);
    return ret;
#else
    return HKS_ERROR_API_NOT_SUPPORTED;
#endif
}

HKS_API_EXPORT int32_t HksGenerateKey(const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSetIn, struct HksParamSet *paramSetOut)
{
#ifdef HKS_SUPPORT_API_GENERATE_KEY
    HKS_LOG_I("enter generate key");
    struct HksParam *storageFlag = NULL;
    int32_t ret = HksGetParam(paramSetIn, HKS_TAG_KEY_STORAGE_FLAG, &storageFlag);
    if ((ret == HKS_SUCCESS) && (storageFlag->uint32Param == HKS_STORAGE_TEMP)) {
        if ((paramSetIn == NULL) || (paramSetOut == NULL)) {
            return HKS_ERROR_NULL_POINTER;
        }
        ret = HksLocalGenerateKey(paramSetIn, paramSetOut);
        HKS_LOG_I("leave generate temp key, result = %" LOG_PUBLIC "d", ret);
        return ret;
    }

    /* generate persistent keys */
    if ((paramSetIn == NULL) || (keyAlias == NULL)) {
        return HKS_ERROR_NULL_POINTER;
    }
    ret = HksClientGenerateKey(keyAlias, paramSetIn, paramSetOut);
    HKS_LOG_I("leave generate persistent key, result = %" LOG_PUBLIC "d", ret);
    return ret;
#else
    (void)keyAlias;
    (void)paramSetIn;
    (void)paramSetOut;
    return HKS_ERROR_API_NOT_SUPPORTED;
#endif
}

HKS_API_EXPORT int32_t HksImportKey(const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSet, const struct HksBlob *key)
{
#ifdef HKS_SUPPORT_API_IMPORT
    HKS_LOG_I("enter import key");
    if ((keyAlias == NULL) || (paramSet == NULL) || (key == NULL)) {
        return HKS_ERROR_NULL_POINTER;
    }
    int32_t ret = HksImportKeyAdapter(keyAlias, paramSet, key);
    HKS_LOG_I("leave import key, result = %" LOG_PUBLIC "d", ret);
    return ret;
#else
    (void)keyAlias;
    (void)paramSet;
    (void)key;
    return HKS_ERROR_API_NOT_SUPPORTED;
#endif
}

HKS_API_EXPORT int32_t HksImportWrappedKey(const struct HksBlob *keyAlias, const struct HksBlob *wrappingKeyAlias,
    const struct HksParamSet *paramSet, const struct HksBlob *wrappedKeyData)
{
#ifdef HKS_SUPPORT_API_IMPORT_WRAPPED_KEY
    HKS_LOG_I("enter import wrapped key");
    if ((keyAlias == NULL) || (wrappingKeyAlias == NULL)|| (paramSet == NULL) || (wrappedKeyData == NULL)) {
        return HKS_ERROR_NULL_POINTER;
    }

    int32_t ret = HksClientImportWrappedKey(keyAlias, wrappingKeyAlias, paramSet, wrappedKeyData);

    HKS_LOG_I("leave import wrapped key, result = %" LOG_PUBLIC "d", ret);
    return ret;
#else
    (void)keyAlias;
    (void)wrappingKeyAlias;
    (void)paramSet;
    (void)wrappedKeyData;
    return HKS_ERROR_API_NOT_SUPPORTED;
#endif
}

HKS_API_EXPORT int32_t HksExportPublicKey(const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSet, struct HksBlob *key)
{
#ifdef HKS_SUPPORT_API_EXPORT
    HKS_LOG_I("enter export public key");
    if ((keyAlias == NULL) || (key == NULL)) {
        return HKS_ERROR_NULL_POINTER;
    }
    int32_t ret = HksExportPublicKeyAdapter(keyAlias, paramSet, key);

    HKS_LOG_I("leave export public key, result = %" LOG_PUBLIC "d", ret);
    return ret;
#else
    (void)keyAlias;
    (void)paramSet;
    (void)key;
    return HKS_ERROR_API_NOT_SUPPORTED;
#endif
}

HKS_API_EXPORT int32_t HksDeleteKey(const struct HksBlob *keyAlias, const struct HksParamSet *paramSet)
{
#ifdef HKS_SUPPORT_API_DELETE_KEY
    HKS_LOG_I("enter delete key");
    HKS_IF_NULL_RETURN(keyAlias, HKS_ERROR_NULL_POINTER)
    int32_t ret = HksClientDeleteKey(keyAlias, paramSet);
    HKS_LOG_I("leave delete key, result = %" LOG_PUBLIC "d", ret);
    return ret;
#else
    (void)keyAlias;
    (void)paramSet;
    return HKS_ERROR_API_NOT_SUPPORTED;
#endif
}

HKS_API_EXPORT int32_t HksGetKeyParamSet(const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSetIn, struct HksParamSet *paramSetOut)
{
#ifdef HKS_SUPPORT_API_GET_KEY_PARAM_SET
    HKS_LOG_I("enter get key paramset");
    (void)paramSetIn;
    if ((keyAlias == NULL) || (paramSetOut == NULL)) {
        return HKS_ERROR_NULL_POINTER;
    }
    int32_t ret = HksClientGetKeyParamSet(keyAlias, paramSetOut);
    HKS_LOG_I("leave get key paramset, result = %" LOG_PUBLIC "d", ret);
    return ret;
#else
    (void)keyAlias;
    (void)paramSetIn;
    (void)paramSetOut;
    return HKS_ERROR_API_NOT_SUPPORTED;
#endif
}

HKS_API_EXPORT int32_t HksKeyExist(const struct HksBlob *keyAlias, const struct HksParamSet *paramSet)
{
#ifdef HKS_SUPPORT_API_KEY_EXIST
    HKS_LOG_I("enter check key exist");
    HKS_IF_NULL_RETURN(keyAlias, HKS_ERROR_NULL_POINTER)
    int32_t ret = HksClientKeyExist(keyAlias, paramSet);
    HKS_LOG_I("leave check key exist, result = %" LOG_PUBLIC "d", ret);
    return ret;
#else
    (void)keyAlias;
    (void)paramSet;
    return HKS_ERROR_API_NOT_SUPPORTED;
#endif
}

HKS_API_EXPORT int32_t HksGenerateRandom(const struct HksParamSet *paramSet, struct HksBlob *random)
{
#ifdef HKS_SUPPORT_API_GENERATE_RANDOM
    HKS_LOG_I("enter generate random");
    HKS_IF_NULL_RETURN(random, HKS_ERROR_NULL_POINTER)

    int32_t ret = HksClientGenerateRandom(random, paramSet);
    HKS_LOG_I("leave generate random, result = %" LOG_PUBLIC "d", ret);
    return ret;
#else
    (void)paramSet;
    (void)random;
    return HKS_ERROR_API_NOT_SUPPORTED;
#endif
}

HKS_API_EXPORT int32_t HksSign(const struct HksBlob *key, const struct HksParamSet *paramSet,
    const struct HksBlob *srcData, struct HksBlob *signature)
{
#ifdef HKS_SUPPORT_API_SIGN_VERIFY
    HKS_LOG_I("enter sign");
    if ((key == NULL) || (paramSet == NULL) || (srcData == NULL) || (signature == NULL)) {
        return HKS_ERROR_NULL_POINTER;
    }

    struct HksParam *isKeyAlias = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_IS_KEY_ALIAS, &isKeyAlias);
    if ((ret == HKS_SUCCESS) && (!isKeyAlias->boolParam)) {
        return HksLocalSign(key, paramSet, srcData, signature);
    }

    return HksClientSign(key, paramSet, srcData, signature);
#else
    (void)key;
    (void)paramSet;
    (void)srcData;
    (void)signature;
    return HKS_ERROR_API_NOT_SUPPORTED;
#endif
}

HKS_API_EXPORT int32_t HksVerify(const struct HksBlob *key, const struct HksParamSet *paramSet,
    const struct HksBlob *srcData, const struct HksBlob *signature)
{
#ifdef HKS_SUPPORT_API_SIGN_VERIFY
    HKS_LOG_I("enter verify");
    if ((key == NULL) || (paramSet == NULL) || (srcData == NULL) || (signature == NULL)) {
        return HKS_ERROR_NULL_POINTER;
    }

    struct HksParam *isKeyAlias = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_IS_KEY_ALIAS, &isKeyAlias);
    if ((ret == HKS_SUCCESS) && (!isKeyAlias->boolParam)) {
        ret = HksLocalVerify(key, paramSet, srcData, signature);
        HKS_LOG_I("leave verify with plain key, result = %" LOG_PUBLIC "d", ret);
        return ret;
    }
    ret = HksClientVerify(key, paramSet, srcData, signature);
    HKS_LOG_I("leave verify with persistent key, result = %" LOG_PUBLIC "d", ret);
    return ret;
#else
    (void)key;
    (void)paramSet;
    (void)srcData;
    (void)signature;
    return HKS_ERROR_API_NOT_SUPPORTED;
#endif
}

HKS_API_EXPORT int32_t HksEncrypt(const struct HksBlob *key, const struct HksParamSet *paramSet,
    const struct HksBlob *plainText, struct HksBlob *cipherText)
{
#ifdef HKS_SUPPORT_API_CIPHER
    HKS_LOG_I("enter encrypt");
    if ((key == NULL) || (paramSet == NULL) || (plainText == NULL) || (cipherText == NULL)) {
        return HKS_ERROR_NULL_POINTER;
    }

    struct HksParam *isKeyAlias = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_IS_KEY_ALIAS, &isKeyAlias);
    if ((ret == HKS_SUCCESS) && (!isKeyAlias->boolParam)) {
        ret = HksLocalEncrypt(key, paramSet, plainText, cipherText);
        HKS_LOG_I("leave encrypt with plain key, result = %" LOG_PUBLIC "d", ret);
        return ret;
    }
#ifndef _CUT_AUTHENTICATE_
    ret = HksClientEncrypt(key, paramSet, plainText, cipherText);
    HKS_LOG_I("leave encrypt with persistent key, result = %" LOG_PUBLIC "d", ret);
    return ret;
#else
    return HKS_ERROR_NOT_SUPPORTED;
#endif
#else
    (void)key;
    (void)paramSet;
    (void)plainText;
    (void)cipherText;
    return HKS_ERROR_API_NOT_SUPPORTED;
#endif
}

HKS_API_EXPORT int32_t HksDecrypt(const struct HksBlob *key, const struct HksParamSet *paramSet,
    const struct HksBlob *cipherText, struct HksBlob *plainText)
{
#ifdef HKS_SUPPORT_API_CIPHER
    HKS_LOG_I("enter decrypt");
    if ((key == NULL) || (paramSet == NULL) || (cipherText == NULL) || (plainText == NULL)) {
        return HKS_ERROR_NULL_POINTER;
    }

    struct HksParam *isKeyAlias = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_IS_KEY_ALIAS, &isKeyAlias);
    if ((ret == HKS_SUCCESS) && (!isKeyAlias->boolParam)) {
        ret = HksLocalDecrypt(key, paramSet, cipherText, plainText);
        HKS_LOG_I("leave decrypt with plain key, result = %" LOG_PUBLIC "d", ret);
        return ret;
    }
#ifndef _CUT_AUTHENTICATE_
    ret = HksClientDecrypt(key, paramSet, cipherText, plainText);
    HKS_LOG_I("leave decrypt with persistent key, result = %" LOG_PUBLIC "d", ret);
    return ret;
#else
    return HKS_ERROR_NOT_SUPPORTED;
#endif
#else
    (void)key;
    (void)paramSet;
    (void)plainText;
    (void)cipherText;
    return HKS_ERROR_API_NOT_SUPPORTED;
#endif
}

HKS_API_EXPORT int32_t HksAgreeKey(const struct HksParamSet *paramSet, const struct HksBlob *privateKey,
    const struct HksBlob *peerPublicKey, struct HksBlob *agreedKey)
{
#ifdef HKS_SUPPORT_API_AGREE_KEY
    HKS_LOG_I("enter agree key");
    if ((paramSet == NULL) || (privateKey == NULL) || (peerPublicKey == NULL) || (agreedKey == NULL)) {
        return HKS_ERROR_NULL_POINTER;
    }

    struct HksParam *isKeyAlias = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_IS_KEY_ALIAS, &isKeyAlias);
    if ((ret == HKS_SUCCESS) && (!isKeyAlias->boolParam)) {
        ret = HksLocalAgreeKey(paramSet, privateKey, peerPublicKey, agreedKey);
        HKS_LOG_I("leave agree key with plain key, result = %" LOG_PUBLIC "d", ret);
        return ret;
    }

    ret = HksAgreeKeyAdapter(paramSet, privateKey, peerPublicKey, agreedKey);
    HKS_LOG_I("leave agree key with persistent key, result = %" LOG_PUBLIC "d", ret);
    return ret;
#else
    (void)paramSet;
    (void)privateKey;
    (void)peerPublicKey;
    (void)agreedKey;
    return HKS_ERROR_API_NOT_SUPPORTED;
#endif
}

HKS_API_EXPORT int32_t HksDeriveKey(const struct HksParamSet *paramSet, const struct HksBlob *mainKey,
    struct HksBlob *derivedKey)
{
#ifdef HKS_SUPPORT_API_DERIVE_KEY
    HKS_LOG_I("enter derive key");
    if ((paramSet == NULL) || (mainKey == NULL) || (derivedKey == NULL)) {
        return HKS_ERROR_NULL_POINTER;
    }

    struct HksParam *isKeyAlias = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_IS_KEY_ALIAS, &isKeyAlias);
    if ((ret == HKS_SUCCESS) && (!isKeyAlias->boolParam)) {
        ret = HksLocalDeriveKey(paramSet, mainKey, derivedKey);
        HKS_LOG_I("leave derive key with plain key, result = %" LOG_PUBLIC "d", ret);
        return ret;
    }
#ifndef _CUT_AUTHENTICATE_
    ret = HksClientDeriveKey(paramSet, mainKey, derivedKey);
    HKS_LOG_I("leave derive key with persistent key, result = %" LOG_PUBLIC "d", ret);
    return ret;
#else
    return HKS_ERROR_NOT_SUPPORTED;
#endif
#else
    (void)paramSet;
    (void)mainKey;
    (void)derivedKey;
    return HKS_ERROR_API_NOT_SUPPORTED;
#endif
}

HKS_API_EXPORT int32_t HksMac(const struct HksBlob *key, const struct HksParamSet *paramSet,
    const struct HksBlob *srcData, struct HksBlob *mac)
{
#ifdef HKS_SUPPORT_API_MAC
    HKS_LOG_I("enter mac");
    if ((key == NULL) || (paramSet == NULL) || (srcData == NULL) || (mac == NULL)) {
        return HKS_ERROR_NULL_POINTER;
    }

    struct HksParam *isKeyAlias = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_IS_KEY_ALIAS, &isKeyAlias);
    if ((ret == HKS_SUCCESS) && (!isKeyAlias->boolParam)) {
        ret = HksLocalMac(key, paramSet, srcData, mac);
        HKS_LOG_I("leave mac with plain key, result = %" LOG_PUBLIC "d", ret);
        return ret;
    }
#ifndef _CUT_AUTHENTICATE_
    ret = HksClientMac(key, paramSet, srcData, mac);
    HKS_LOG_I("leave mac with persistent key, result = %" LOG_PUBLIC "d", ret);
    return ret;
#else
    return HKS_ERROR_NOT_SUPPORTED;
#endif
#else
    (void)key;
    (void)paramSet;
    (void)srcData;
    (void)mac;
    return HKS_ERROR_API_NOT_SUPPORTED;
#endif
}

HKS_API_EXPORT int32_t HksHash(const struct HksParamSet *paramSet,
    const struct HksBlob *srcData, struct HksBlob *hash)
{
#ifdef HKS_SUPPORT_API_HASH
    HKS_LOG_I("enter hash");
    if ((paramSet == NULL) || (srcData == NULL) || (hash == NULL)) {
        return HKS_ERROR_NULL_POINTER;
    }
    int32_t ret = HksLocalHash(paramSet, srcData, hash);
    HKS_LOG_I("leave hash, result = %" LOG_PUBLIC "d", ret);
    return ret;
#else
    (void)paramSet;
    (void)srcData;
    (void)hash;
    return HKS_ERROR_API_NOT_SUPPORTED;
#endif
}

HKS_API_EXPORT int32_t HksGetKeyInfoList(const struct HksParamSet *paramSet,
    struct HksKeyInfo *keyInfoList, uint32_t *listCount)
{
#ifdef HKS_SUPPORT_API_GET_KEY_INFO_LIST
    HKS_LOG_I("enter get key info list");
    (void)paramSet;
    if ((keyInfoList == NULL) || (listCount == NULL)) {
        return HKS_ERROR_NULL_POINTER;
    }
    int32_t ret = HksClientGetKeyInfoList(keyInfoList, listCount);
    HKS_LOG_I("leave get key info list, result = %" LOG_PUBLIC "d", ret);
    return ret;
#else
    (void)paramSet;
    (void)keyInfoList;
    (void)listCount;
    return HKS_ERROR_API_NOT_SUPPORTED;
#endif
}

HKS_API_EXPORT int32_t HksAttestKey(const struct HksBlob *keyAlias, const struct HksParamSet *paramSet,
    struct HksCertChain *certChain)
{
#ifdef HKS_SUPPORT_API_ATTEST_KEY
    HKS_LOG_I("enter attest key");
    if ((keyAlias == NULL) || (paramSet == NULL) || (certChain == NULL)) {
        return HKS_ERROR_NULL_POINTER;
    }
    int32_t ret = HksClientAttestKey(keyAlias, paramSet, certChain);
    HKS_LOG_I("leave attest key, result = %" LOG_PUBLIC "d", ret);
    return ret;
#else
    (void)keyAlias;
    (void)paramSet;
    (void)certChain;
    return HKS_ERROR_API_NOT_SUPPORTED;
#endif
}

HKS_API_EXPORT int32_t HksGetCertificateChain(const struct HksBlob *keyAlias, const struct HksParamSet *paramSet,
    struct HksCertChain *certChain)
{
    (void)keyAlias;
    (void)paramSet;
    (void)certChain;
    return HKS_ERROR_API_NOT_SUPPORTED;
}

HKS_API_EXPORT int32_t HksWrapKey(const struct HksBlob *keyAlias, const struct HksBlob *targetKeyAlias,
    const struct HksParamSet *paramSet, struct HksBlob *wrappedData)
{
    (void)keyAlias;
    (void)targetKeyAlias;
    (void)paramSet;
    (void)wrappedData;
    return HKS_ERROR_API_NOT_SUPPORTED;
}

HKS_API_EXPORT int32_t HksUnwrapKey(const struct HksBlob *keyAlias, const struct HksBlob *targetKeyAlias,
    const struct HksBlob *wrappedData, const struct HksParamSet *paramSet)
{
    (void)keyAlias;
    (void)targetKeyAlias;
    (void)paramSet;
    (void)wrappedData;
    return HKS_ERROR_API_NOT_SUPPORTED;
}

HKS_API_EXPORT int32_t HksBnExpMod(struct HksBlob *x, const struct HksBlob *a,
    const struct HksBlob *e, const struct HksBlob *n)
{
#ifdef HKS_SUPPORT_API_BN_EXP_MOD
    HKS_LOG_I("enter bn exp mod");
    if ((x == NULL) || (a == NULL) || (e == NULL) || (n == NULL)) {
        return HKS_ERROR_NULL_POINTER;
    }

    int32_t ret = HksLocalBnExpMod(x, a, e, n);
    HKS_LOG_I("leave bn exp mod key, result = %" LOG_PUBLIC "d", ret);
    return ret;
#else
    (void)x;
    (void)a;
    (void)e;
    (void)n;
    return HKS_ERROR_API_NOT_SUPPORTED;
#endif
}

/*
 * Currently, the device certificate and device key are implemented using stubs.
 * By default, the device key exists.
*/
HKS_API_EXPORT int32_t HcmIsDeviceKeyExist(const struct HksParamSet *paramSet)
{
    (void)paramSet;
    return HKS_SUCCESS;
}

HKS_API_EXPORT int32_t HksValidateCertChain(const struct HksCertChain *certChain, struct HksParamSet *paramSetOut)
{
#ifdef HKS_SUPPORT_API_ATTEST_KEY
    HKS_LOG_I("enter validate cert chain");
    if ((paramSetOut == NULL) || (certChain == NULL)) {
        return HKS_ERROR_NULL_POINTER;
    }
    int32_t ret = HksClientValidateCertChain(certChain, paramSetOut);
    HKS_LOG_I("leave validate cert chain, result = %" LOG_PUBLIC "d", ret);
    return ret;
#else
    (void)certChain;
    (void)paramSetOut;
    return HKS_ERROR_API_NOT_SUPPORTED;
#endif
}

HKS_API_EXPORT int32_t HksInit(const struct HksBlob *keyAlias, const struct HksParamSet *paramSet,
    struct HksBlob *handle, struct HksBlob *token)
{
    HKS_LOG_I("enter init operation");
    if ((keyAlias == NULL) || (paramSet == NULL) || (handle == NULL)) { /* token can be null */
        HKS_LOG_E("the pointer param entered is invalid");
        return HKS_ERROR_NULL_POINTER;
    }

    int32_t ret = HksClientInit(keyAlias, paramSet, handle, token);
    HKS_LOG_I("leave init operation, result = %" LOG_PUBLIC "d", ret);
    return ret;
}

HKS_API_EXPORT int32_t HksUpdate(const struct HksBlob *handle, const struct HksParamSet *paramSet,
    const struct HksBlob *inData, struct HksBlob *outData)
{
    HKS_LOG_I("enter update operation");
    if ((handle == NULL) || (paramSet == NULL) || (inData == NULL) || (outData == NULL)) {
        HKS_LOG_E("the pointer param entered is invalid");
        return HKS_ERROR_NULL_POINTER;
    }

    int32_t ret = HksClientUpdate(handle, paramSet, inData, outData);
    HKS_LOG_I("leave update operation, result = %" LOG_PUBLIC "d", ret);
    return ret;
}

HKS_API_EXPORT int32_t HksFinish(const struct HksBlob *handle, const struct HksParamSet *paramSet,
    const struct HksBlob *inData, struct HksBlob *outData)
{
    HKS_LOG_I("enter finish operation");
    if ((handle == NULL) || (paramSet == NULL) || (inData == NULL) || (outData == NULL)) {
        HKS_LOG_E("the pointer param entered is invalid");
        return HKS_ERROR_NULL_POINTER;
    }

    int32_t ret = HksClientFinish(handle, paramSet, inData, outData);
    HKS_LOG_I("leave finish operation, result = %" LOG_PUBLIC "d", ret);
    return ret;
}

HKS_API_EXPORT int32_t HksAbort(const struct HksBlob *handle, const struct HksParamSet *paramSet)
{
    HKS_LOG_I("enter abort operation");
    if ((handle == NULL) || (paramSet == NULL)) {
        HKS_LOG_E("the pointer param entered is invalid");
        return HKS_ERROR_NULL_POINTER;
    }

    int32_t ret = HksClientAbort(handle, paramSet);
    HKS_LOG_I("leave abort operation, result = %" LOG_PUBLIC "d", ret);
    return ret;
}

