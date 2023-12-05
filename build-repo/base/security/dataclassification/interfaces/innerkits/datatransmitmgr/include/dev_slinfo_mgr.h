/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef DEV_SLINFO_MGR_H
#define DEV_SLINFO_MGR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_UDID_LENGTH 64

#define DATA_SEC_LEVEL0 0 /* s0 */
#define DATA_SEC_LEVEL1 1 /* s1 */
#define DATA_SEC_LEVEL2 2 /* s2 */
#define DATA_SEC_LEVEL3 3 /* s3 */
#define DATA_SEC_LEVEL4 4 /* s4 */

typedef struct {
    uint8_t udid[MAX_UDID_LENGTH];
    uint32_t udidLen;
} DEVSLQueryParams;

enum {
    DEVSL_SUCCESS = 0,
    DEVSL_ERROR = 100,
    DEVSL_ERR_REQUEST_DEVICE_EXCEED_LIMIT = 101,
    DEVSL_ERR_DEVICE_SEC_SDK_INIT = 102,
    DEVSL_ERR_OUT_OF_MEMORY = 103,
    DEVSL_ERR_MUTEX_LOCK_INIT = 104,
    DEVSL_ERR_BAD_PARAMETERS = 105,
};

int32_t DATASL_GetHighestSecLevel(DEVSLQueryParams *queryParams, uint32_t *levelInfo);

int32_t DATASL_OnStart(void);

void DATASL_OnStop(void);

typedef void HigestSecInfoCallback(DEVSLQueryParams *queryParams, int32_t result, uint32_t levelInfo);

int32_t DATASL_GetHighestSecLevelAsync(DEVSLQueryParams *queryParams, HigestSecInfoCallback *callback);

#ifdef __cplusplus
}
#endif
#endif