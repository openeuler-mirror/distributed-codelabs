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

#ifndef HKS_OPENSSL_SM2_H
#define HKS_OPENSSL_SM2_H

#include <stdint.h>

#include "hks_crypto_hal.h"
#include "hks_type.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SM2_KEYPAIR_CNT 3

#ifdef  HKS_SUPPORT_SM2_C
#ifdef HKS_SUPPORT_SM2_GENERATE_KEY
int32_t HksOpensslSm2GenerateKey(const struct HksKeySpec *spec, struct HksBlob *key);
#endif

#ifdef HKS_SUPPORT_SM2_GET_PUBLIC_KEY
int32_t HksOpensslGetSm2PubKey(const struct HksBlob *input, struct HksBlob *output);
#endif

#ifdef HKS_SUPPORT_SM2_SIGN_VERIFY
int32_t HksOpensslSm2Verify(const struct HksBlob *key, const struct HksUsageSpec *usageSpec,
    const struct HksBlob *message, const struct HksBlob *signature);
int32_t HksOpensslSm2Sign(const struct HksBlob *key, const struct HksUsageSpec *usageSpec,
    const struct HksBlob *message, struct HksBlob *signature);
#endif

#endif
#ifdef __cplusplus
}
#endif

#endif /* HKS_OPENSSL_SM2_H */
