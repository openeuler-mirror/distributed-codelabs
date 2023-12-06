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

#ifndef HKS_KEYNODE_H
#define HKS_KEYNODE_H

#include <stdint.h>

#include "hks_double_list.h"
#include "hks_type.h"

#define HKS_OPERATION_PARAM_CNT 3
#define HKS_UINT64_MAX 0xFFFFFFFFFFFFFFFF
#define HKS_KEYNODE_HANDLE_INVALID_VALUE 0
#define HKS_KEYNODE_HANDLE_INITIAL_VALUE 1

struct HuksKeyNode {
    struct DoubleList listHead;
    struct HksParamSet *keyBlobParamSet;
    
    /**
     * @brief used to cache params of caller and state or temp prop in memory during using key
     * CURRENT state:
     * uint64_t crypto_ctx;     // crypto engine context
     */
    struct HksParamSet *runtimeParamSet; // only used to store caller's paramset

    /**
     * @brief used to cache params of caller and state or temp prop in memory during using key
     * CURRENT state:
     * bool     isSecureAccess; // is set secure access
     * bool     isSupportSecureSign; // is set secure sign tag
     * bool     isAppendUpdateData;  // is append update data for auth info
     * uint8 challenge[TOKEN_SIZE];// key challenge(token)
     * uint32_t accessTime;     // key init access timestamp in second
     * int32_t keyAuthResult;   // key auth result cache
     */
    struct HksParamSet *authRuntimeParamSet;
    uint64_t handle;
};

#ifdef __cplusplus
extern "C" {
#endif

struct HuksKeyNode *HksCreateKeyNode(const struct HksBlob *key, const struct HksParamSet *paramSet);

struct HuksKeyNode *HksQueryKeyNode(uint64_t handle);

void HksDeleteKeyNode(uint64_t handle);

#ifdef __cplusplus
}
#endif

#endif