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

#ifndef HKS_OPENSSL_ENGINE_H
#define HKS_OPENSSL_ENGINE_H

#include <openssl/ossl_typ.h>
#include <stdint.h>

#include "hks_type.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HKS_OPENSSL_SUCCESS    1     /* openssl return 1: success */
#define HKS_OPENSSL_ERROR_LEN 128
#define BIT_NUM_OF_UINT8 8
#define AFTER_HASH_LEN_16 16
#define AFTER_HASH_LEN_20 20
#define AFTER_HASH_LEN_28 28
#define AFTER_HASH_LEN_32 32
#define AFTER_HASH_LEN_48 48
#define AFTER_HASH_LEN_64 64

void HksLogOpensslError(void);

int32_t HksOpensslCheckBlob(const struct HksBlob *blob);

const EVP_MD *GetOpensslAlg(uint32_t alg);

const EVP_MD *GetOpensslAlgFromLen(uint32_t len);

#ifdef __cplusplus
}
#endif

#endif /* HKS_OPENSSL_ENGINE_H */
