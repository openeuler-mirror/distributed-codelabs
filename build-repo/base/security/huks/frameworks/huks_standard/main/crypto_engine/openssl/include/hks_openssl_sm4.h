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

#ifndef HKS_OPENSSL_SM4_H
#define HKS_OPENSSL_SM4_H

#include <stdint.h>

#include "hks_crypto_hal.h"
#include "hks_type.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HKS_SUPPORT_SM4_C
#ifdef HKS_SUPPORT_SM4_GENERATE_KEY
int32_t HksOpensslSm4GenerateKey(const struct HksKeySpec *spec, struct HksBlob *key);
#endif /* HKS_SUPPORT_SM4_GENERATE_KEY */

int32_t HksOpensslSm4EncryptInit(void **cryptoCtx, const struct HksBlob *key, const struct HksUsageSpec *usageSpec);

int32_t HksOpensslSm4EncryptUpdate(void *cryptoCtx, const struct HksBlob *message, struct HksBlob *cipherText);

int32_t HksOpensslSm4EncryptFinal(void **cryptoCtx, const struct HksBlob *message, struct HksBlob *cipherText,
    struct HksBlob *tagAead);

int32_t HksOpensslSm4DecryptInit(void **cryptoCtx, const struct HksBlob *key, const struct HksUsageSpec *usageSpec);

int32_t HksOpensslSm4DecryptUpdate(void *cryptoCtx, const struct HksBlob *message, struct HksBlob *plainText);

int32_t HksOpensslSm4DecryptFinal(void **cryptoCtx, const struct HksBlob *message, struct HksBlob *cipherText,
    struct HksBlob *tagAead);

void HksOpensslSm4HalFreeCtx(void **cryptoCtx);

#endif /* HKS_SUPPORT_SM4_C */

#ifdef __cplusplus
}
#endif

#endif /* HKS_OPENSSL_SM4_H */
