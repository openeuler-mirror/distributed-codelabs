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
#ifndef HKS_SECURE_ACCESS_H
#define HKS_SECURE_ACCESS_H

#include "hks_type.h"
#include "hks_keynode.h"

#ifdef __cplusplus
extern "C" {
#endif

int32_t HksCoreSecureAccessInitParams(struct HuksKeyNode *keyNode, const struct HksParamSet *initParamSet,
    struct HksBlob *token);

int32_t HksCoreSecureAccessVerifyParams(struct HuksKeyNode *keyNode, const struct HksParamSet *inParamSet);

int32_t HksCoreAppendAuthInfoBeforeUpdate(struct HuksKeyNode *keyNode, uint32_t pur,
    const struct HksParamSet *inParamSet, const struct HksBlob *inData, struct HksBlob *appendedData);

int32_t HksCoreAppendAuthInfoBeforeFinish(struct HuksKeyNode *keyNode, uint32_t pur,
    const struct HksParamSet *inParamSet, const struct HksBlob *inData, struct HksBlob *appendedData);

int32_t HksCoreAppendAuthInfoAfterFinish(struct HuksKeyNode *keyNode, uint32_t pur,
    const struct HksParamSet *inParamSet, uint32_t inOutDataBufferSize, struct HksBlob *inOutData);

int32_t HksProcessIdentityVerify(const struct HksParamSet *blobParamSet, const struct HksParamSet *runtimeParamSet);

#ifdef __cplusplus
}
#endif

#endif
