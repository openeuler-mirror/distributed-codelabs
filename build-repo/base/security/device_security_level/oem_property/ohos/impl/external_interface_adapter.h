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

#ifndef EXTERNAL_INTERFACE_ADAPTER_H
#define EXTERNAL_INTERFACE_ADAPTER_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct DslmInfoInCertChain {
    char *udidStr;
    char *credStr;
    char *nonceStr; // challenge + pkInfoList
};

int32_t GetPkInfoListStr(bool isSelf, const char *udidStr, char **pkInfoList);
int32_t DslmCredAttestAdapter(struct DslmInfoInCertChain *info, uint8_t **certChain, uint32_t *certChainLen);
int32_t ValidateCertChainAdapter(const uint8_t *data, uint32_t dataLen, struct DslmInfoInCertChain *resultInfo);
int32_t HksAttestIsReadyAdapter(void);

int32_t InitDslmInfoInCertChain(struct DslmInfoInCertChain *saveInfo);
void DestroyDslmInfoInCertChain(struct DslmInfoInCertChain *saveInfo);

#ifdef __cplusplus
}
#endif

#endif // EXTERNAL_INTERFACE_ADAPTER_H