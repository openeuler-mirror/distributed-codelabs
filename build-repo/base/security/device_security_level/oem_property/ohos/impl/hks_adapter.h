/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HKS_ADAPTER_H
#define HKS_ADAPTER_H

#include <stdint.h>

#include "hks_api.h"
#include "hks_param.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CERT_CHAIN_CERT_NUM 4

struct HksCertChainInitParams {
    bool certChainExist;
    bool certCountValid;
    bool certDataExist;
    uint32_t certDataSize;
};

int32_t FillHksParamSet(struct HksParamSet **paramSet, struct HksParam *param, int32_t paramNums);
int32_t HksGenerateKeyAdapter(const struct HksBlob *keyAlias);
int32_t ConstructHksCertChain(struct HksCertChain **certChain, const struct HksCertChainInitParams *certChainParam);
void DestroyHksCertChain(struct HksCertChain *certChain);
int32_t HksCertChainToBuffer(const struct HksCertChain *hksCertChain, uint8_t **data, uint32_t *dataLen);
int32_t BufferToHksCertChain(const uint8_t *data, uint32_t dataLen, struct HksCertChain *hksCertChain);

#ifdef __cplusplus
}
#endif

#endif // HKS_ADAPTER_H