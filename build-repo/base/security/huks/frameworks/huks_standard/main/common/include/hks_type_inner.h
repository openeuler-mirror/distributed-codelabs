/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef HKS_TYPE_INNER_H
#define HKS_TYPE_INNER_H

#include "hks_type.h"
#include "securec.h"

#define HANDLE_SIZE              8
#define DEFAULT_AUTH_TIMEOUT     5

/* EnrolledIdInfo stored format: |-enrolledId len-|-enrolledId1 type-|-enrolledId1 value-|...|  */
#define ENROLLED_ID_INFO_MIN_LEN  (sizeof(uint32_t) + (sizeof(uint32_t) + sizeof(uint64_t)))

enum HksUserAuthResult {
    HKS_AUTH_RESULT_NONE = -2, // not support user auth
    HKS_AUTH_RESULT_INIT = -1,
    HKS_AUTH_RESULT_SUCCESS = 0,
    HKS_AUTH_RESULT_FAILED = 1,
};

enum HksStageType {
    HKS_STAGE_THREE = 0,
    HKS_STAGE_ONE = 1,
};

enum HksInnerTag {
    HKS_TAG_APPENDED_DATA_PREFIX = HKS_TAG_TYPE_BYTES | 10020,
    HKS_TAG_APPENDED_DATA_SUFFIX = HKS_TAG_TYPE_BYTES | 10021,
    
    /* Inner-use TAGS used for ipc serialization */
    HKS_TAG_PARAM0_BUFFER = HKS_TAG_TYPE_BYTES | 30001,
    HKS_TAG_PARAM1_BUFFER = HKS_TAG_TYPE_BYTES | 30002,
    HKS_TAG_PARAM2_BUFFER = HKS_TAG_TYPE_BYTES | 30003,
    HKS_TAG_PARAM3_BUFFER = HKS_TAG_TYPE_BYTES | 30004,
    HKS_TAG_PARAM4_BUFFER = HKS_TAG_TYPE_BYTES | 30005,
    HKS_TAG_PARAM0_UINT32 = HKS_TAG_TYPE_UINT | 30006,
    HKS_TAG_PARAM1_UINT32 = HKS_TAG_TYPE_UINT | 30007,
    HKS_TAG_PARAM2_UINT32 = HKS_TAG_TYPE_UINT | 30008,
    HKS_TAG_PARAM3_UINT32 = HKS_TAG_TYPE_UINT | 30009,
    HKS_TAG_PARAM4_UINT32 = HKS_TAG_TYPE_UINT | 30010,
    HKS_TAG_PARAM0_BOOL = HKS_TAG_TYPE_BOOL | 30011,
    HKS_TAG_PARAM1_BOOL = HKS_TAG_TYPE_BOOL | 30012,
    HKS_TAG_PARAM2_BOOL = HKS_TAG_TYPE_BOOL | 30013,
    HKS_TAG_PARAM3_BOOL = HKS_TAG_TYPE_BOOL | 30014,
    HKS_TAG_PARAM4_BOOL = HKS_TAG_TYPE_BOOL | 30015,
    HKS_TAG_PARAM0_NULL = HKS_TAG_TYPE_BYTES | 30016,
    HKS_TAG_PARAM1_NULL = HKS_TAG_TYPE_BYTES | 30017,
    HKS_TAG_PARAM2_NULL = HKS_TAG_TYPE_BYTES | 30018,
    HKS_TAG_PARAM3_NULL = HKS_TAG_TYPE_BYTES | 30019,
    HKS_TAG_PARAM4_NULL = HKS_TAG_TYPE_BYTES | 30020,

    HKS_TAG_ACCESS_TOKEN_ID = HKS_TAG_TYPE_UINT | 30021,
};

#define HKS_PARAM_BUFFER_NULL_INTERVAL ((HKS_TAG_PARAM0_NULL) - (HKS_TAG_PARAM0_BUFFER))

struct HksProcessInfo {
    struct HksBlob userId;
    struct HksBlob processName;
    int32_t userIdInt;
    uint64_t accessTokenId;
};

struct HksParamOut {
    uint32_t tag;
    union {
        bool *boolParam;
        int32_t *int32Param;
        uint32_t *uint32Param;
        uint64_t *uint64Param;
        struct HksBlob *blob;
    };
};

struct HksKeyMaterialHeader {
    enum HksKeyAlg keyAlg;
    uint32_t keySize;
};

#endif /* HKS_TYPE_INNER_H */
