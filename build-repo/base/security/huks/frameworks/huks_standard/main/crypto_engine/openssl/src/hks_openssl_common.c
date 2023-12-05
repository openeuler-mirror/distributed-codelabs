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

#include "hks_openssl_common.h"

#include <openssl/rand.h>
#include <stddef.h>

#include "hks_log.h"
#include "hks_mem.h"
#include "hks_openssl_engine.h"
#include "hks_template.h"
#include "securec.h"

int32_t HksOpensslGenerateRandomKey(const uint32_t keySize, struct HksBlob *key)
{
    uint32_t keySizeByte = keySize / BIT_NUM_OF_UINT8;
    int32_t ret = HKS_ERROR_CRYPTO_ENGINE_ERROR;

    uint8_t *tmpKey = (uint8_t *)HksMalloc(keySizeByte);
    HKS_IF_NULL_LOGE_RETURN(tmpKey, HKS_ERROR_MALLOC_FAIL, "malloc buffer failed")

    do {
        if (RAND_bytes(tmpKey, keySizeByte) <= 0) {
            HKS_LOG_E("generate key is failed:0x%" LOG_PUBLIC "x", ret);
            break;
        }

        key->data = tmpKey;
        key->size = keySizeByte;
        ret = HKS_SUCCESS;
    } while (0);

    if (ret != HKS_SUCCESS) {
        (void)memset_s(tmpKey, keySizeByte, 0, keySizeByte);
        HksFree(tmpKey);
    }
    return ret;
}

int32_t HksOpensslFillRandom(struct HksBlob *randomData)
{
    int ret = RAND_bytes(randomData->data, randomData->size);
    if (ret <= 0) {
        HKS_LOG_E("generate random failed, ret = 0x%" LOG_PUBLIC "x", ret);
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    if (randomData->size == 1) {
        return HKS_SUCCESS;
    }

    uint32_t j = 0;

    for (uint32_t i = 0; i < randomData->size; i++) {
        if (randomData->data[i] == 0) {
            j++;
        }
    }
    if (j == randomData->size) {
        HKS_LOG_E("fill random failed, size %" LOG_PUBLIC "x", randomData->size);
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }
    HKS_LOG_D("generate random success");
    return HKS_SUCCESS;
}

int32_t HksOpensslFillPrivRandom(struct HksBlob *randomData)
{
    int ret = RAND_priv_bytes(randomData->data, randomData->size);
    if (ret <= 0) {
        HKS_LOG_E("generate private random failed, ret = 0x%" LOG_PUBLIC "x", ret);
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    if (randomData->size == 1) {
        return HKS_SUCCESS;
    }

    uint32_t j = 0;

    for (uint32_t i = 0; i < randomData->size; i++) {
        if (randomData->data[i] == 0) {
            j++;
        }
    }
    if (j == randomData->size) {
        HKS_LOG_E("fill private random failed, size %" LOG_PUBLIC "x", randomData->size);
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }
    HKS_LOG_D("generate private random success");
    return HKS_SUCCESS;
}

int32_t HksOpensslGetMainKey(const struct HksBlob *message, struct HksBlob *mainKey)
{
    (void)message;

#ifndef _HARDWARE_ROOT_KEY_
    (void)mainKey;
    return HKS_ERROR_NOT_SUPPORTED;
#else
    /*
     * Currently, root key is implemented using stubs.
     * Product adaptation needs to be performed based on hardware capabilities.
     */
    uint8_t stubBuf[] = {
        0x0c, 0xb4, 0x29, 0x39, 0xb7, 0x46, 0xa6, 0x4b,
        0xdd, 0xf3, 0x75, 0x4c, 0xe0, 0x73, 0x91, 0x51,
        0xc4, 0x88, 0xbe, 0xa4, 0xe1, 0x87, 0xb5, 0x42,
        0x06, 0x27, 0x08, 0x21, 0xe2, 0x8f, 0x9b, 0xc1,
    };

    if (memcpy_s(mainKey->data, mainKey->size, stubBuf, sizeof(stubBuf)) != EOK) {
        HKS_LOG_E("memcpy failed, get stub main key failed");
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    return HKS_SUCCESS;
#endif
}
