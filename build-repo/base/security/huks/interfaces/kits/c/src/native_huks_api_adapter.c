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

#include "hks_api.h"
#include "hks_param.h"

#include "native_huks_api_adapter.h"
#include "native_huks_param.h"

int32_t HuksAttestAdapter(const struct OH_Huks_Blob *keyAlias, const struct OH_Huks_ParamSet *paramSet,
    struct OH_Huks_CertChain *certChain)
{
    int32_t ret;
    struct HksParamSet *newParamSet = NULL;
    do {
        if (paramSet == NULL) {
            ret = HKS_ERROR_NULL_POINTER;
            break;
        }
        ret = HksCheckParamSet((struct HksParamSet *)paramSet, paramSet->paramSetSize);
        if (ret != HKS_SUCCESS) {
            break;
        }
        ret = HksInitParamSet(&newParamSet);
        if (ret != HKS_SUCCESS) {
            break;
        }
        ret = HksFreshParamSet((struct HksParamSet *)paramSet, false);
        if (ret != HKS_SUCCESS) {
            break;
        }
        ret = HksAddParams(newParamSet, (const struct HksParam *)paramSet->params, paramSet->paramsCnt);
        if (ret != HKS_SUCCESS) {
            break;
        }
        struct HksParam isBase64Param = { .tag = HKS_TAG_ATTESTATION_BASE64, .boolParam = true };
        ret = HksAddParams(newParamSet,  &isBase64Param, 1);
        if (ret != HKS_SUCCESS) {
            break;
        }
        ret = HksBuildParamSet(&newParamSet);
        if (ret != HKS_SUCCESS) {
            break;
        }
        ret = HksAttestKey((const struct HksBlob *)keyAlias, (struct HksParamSet *)newParamSet,
            (struct HksCertChain *)certChain);
    } while (0);
    HksFreeParamSet(&newParamSet);
    return ret;
}