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

#ifndef HKS_COMMON_CHECK_H
#define HKS_COMMON_CHECK_H

#include <stdbool.h>
#include <stdint.h>

#include "hks_type.h"

#define HKS_DIGEST_MD5_LEN 16
#define HKS_DIGEST_SHA1_LEN 20
#define HKS_DIGEST_SHA224_LEN 28
#define HKS_DIGEST_SHA256_LEN 32
#define HKS_DIGEST_SHA384_LEN 48
#define HKS_DIGEST_SHA512_LEN 64
#define HKS_DIGEST_SM3_LEN 32

#ifdef __cplusplus
extern "C" {
#endif

int32_t HksCheckBlob4(const struct HksBlob *data1, const struct HksBlob *data2,
    const struct HksBlob *data3, const struct HksBlob *data4);

int32_t HksCheckBlob3(const struct HksBlob *data1, const struct HksBlob *data2, const struct HksBlob *data3);

int32_t HksCheckBlob2(const struct HksBlob *data1, const struct HksBlob *data2);

int32_t HksCheckParamSetValidity(const struct HksParamSet *paramSet);

int32_t HksCheckBlob4AndParamSet(const struct HksBlob *data1, const struct HksBlob *data2,
    const struct HksBlob *data3, const struct HksBlob *data4, const struct HksParamSet *paramSet);

int32_t HksCheckBlob3AndParamSet(const struct HksBlob *data1, const struct HksBlob *data2,
    const struct HksBlob *data3, const struct HksParamSet *paramSet);

int32_t HksCheckBlob2AndParamSet(const struct HksBlob *data1, const struct HksBlob *data2,
    const struct HksParamSet *paramSet);

int32_t HksCheckBlobAndParamSet(const struct HksBlob *data, const struct HksParamSet *paramSet);

int32_t HksGetDigestLen(uint32_t digest, uint32_t *digestLen);

int32_t HksCheckAesAeMode(const struct HksParamSet *paramSet, bool *isAes, bool *isAeMode);

int32_t HksCheckWrappedDataFormatValidity(const struct HksBlob *wrappedData, uint32_t validTotalBlobs,
    const uint32_t *validBlobLengths);

int32_t HksGetBlobFromWrappedData(const struct HksBlob *wrappedData, uint32_t blobIndex, uint32_t totalBlobs,
    struct HksBlob *blob);
#ifdef __cplusplus
}
#endif

#endif /* HKS_COMMON_CHECK_H */
