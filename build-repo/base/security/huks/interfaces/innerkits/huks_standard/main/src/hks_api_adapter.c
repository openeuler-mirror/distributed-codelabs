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

#include "hks_api_adapter.h"

#include <stddef.h>

#include "hks_client_ipc.h"
#include "hks_client_service_adapter.h"
#include "hks_client_service_adapter_common.h"
#include "hks_log.h"
#include "hks_mem.h"
#include "hks_param.h"
#include "hks_template.h"
#include "securec.h"

#ifdef _CUT_AUTHENTICATE_
#undef HKS_SUPPORT_API_IMPORT
#undef HKS_SUPPORT_API_EXPORT
#undef HKS_SUPPORT_API_AGREE_KEY
#endif

#ifdef HKS_SUPPORT_API_IMPORT
int32_t HksImportKeyAdapter(const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSet, const struct HksBlob *key)
{
    struct HksBlob innerKey = { 0, NULL };

    struct HksParam *importKeyTypeParam = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_IMPORT_KEY_TYPE, &importKeyTypeParam);
    if ((ret == HKS_SUCCESS) &&
        ((importKeyTypeParam->uint32Param == HKS_KEY_TYPE_PRIVATE_KEY) ||
        (importKeyTypeParam->uint32Param == HKS_KEY_TYPE_KEY_PAIR))) {
        ret = CopyToInnerKey(key, &innerKey);
    } else {
        ret = GetHksPubKeyInnerFormat(paramSet, key, &innerKey);
    }
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("translate key to inner format failed, ret = %" LOG_PUBLIC "d", ret);
        return ret;
    }

    ret = HksClientImportKey(keyAlias, paramSet, &innerKey);
    (void)memset_s(innerKey.data, innerKey.size, 0, innerKey.size);
    HKS_FREE_BLOB(innerKey);
    return ret;
}
#endif

#ifdef HKS_SUPPORT_API_AGREE_KEY
int32_t HksAgreeKeyAdapter(const struct HksParamSet *paramSet, const struct HksBlob *privateKey,
    const struct HksBlob *peerPublicKey, struct HksBlob *agreedKey)
{
    struct HksBlob publicKey = { 0, NULL };
    int32_t ret = GetHksPubKeyInnerFormat(paramSet, peerPublicKey, &publicKey);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("get public key from x509 format failed, ret = %" LOG_PUBLIC "d", ret);
        return ret;
    }

    ret = HksClientAgreeKey(paramSet, privateKey, &publicKey, agreedKey);
    (void)memset_s(publicKey.data, publicKey.size, 0, publicKey.size);
    HKS_FREE_BLOB(publicKey);
    return ret;
}
#endif

#ifdef HKS_SUPPORT_API_EXPORT
int32_t HksExportPublicKeyAdapter(const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSet, struct HksBlob *key)
{
    uint8_t *buffer = (uint8_t *)HksMalloc(MAX_KEY_SIZE);
    HKS_IF_NULL_LOGE_RETURN(buffer, HKS_ERROR_MALLOC_FAIL, "malloc failed")
    (void)memset_s(buffer, MAX_KEY_SIZE, 0, MAX_KEY_SIZE);
    struct HksBlob publicKey = { MAX_KEY_SIZE, buffer };

    int32_t ret = HksClientExportPublicKey(keyAlias, paramSet, &publicKey);
    if (ret == HKS_SUCCESS) {
        struct HksBlob x509Key = { 0, NULL };
        ret = TranslateToX509PublicKey(&publicKey, &x509Key);
        if (ret != HKS_SUCCESS) {
            HKS_FREE_PTR(buffer);
            return ret;
        }

        if ((CheckBlob(key) != HKS_SUCCESS) || (memcpy_s(key->data, key->size, x509Key.data, x509Key.size) != EOK)) {
            ret = HKS_ERROR_INSUFFICIENT_DATA;
            HKS_LOG_E("x509 format memcpy failed");
        } else {
            key->size = x509Key.size;
        }

        HKS_FREE_BLOB(x509Key);
    }
    HKS_FREE_BLOB(publicKey);
    return ret;
}
#endif
