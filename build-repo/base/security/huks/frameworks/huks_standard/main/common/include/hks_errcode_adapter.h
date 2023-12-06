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
#ifndef HKS_ERRCODE_ADAPTER_H
#define HKS_ERRCODE_ADAPTER_H

#ifdef __cplusplus
#include <cstdbool>
#include <cstdlib>
#include <cstdint>
#else
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#endif

#include "hks_type.h"

struct HksResult {
    int32_t errorCode;
    const char *errorMsg;
    uint8_t *data;
};

struct HksError {
    int32_t innerErrCode;
    struct HksResult hksResult;
};

#ifdef __cplusplus
extern "C" {
#endif

struct HksResult HksConvertErrCode(int32_t result);

#ifdef __cplusplus
}
#endif

#endif /* HKS_ERRCODE_ADAPTER_H */
