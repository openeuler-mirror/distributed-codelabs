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

#ifndef DEV_SLINFO_ADPT_H
#define DEV_SLINFO_ADPT_H

#include "device_security_info.h"
#include "dev_slinfo_mgr.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEFAULT_DEV_SEC_LEVEL 0
#define DEV_SEC_LEVEL1 1 /* sl1 */
#define DEV_SEC_LEVEL2 2 /* sl2 */
#define DEV_SEC_LEVEL3 3 /* sl3 */
#define DEV_SEC_LEVEL4 4 /* sl4 */
#define DEV_SEC_LEVEL5 5 /* sl5 */

typedef int32_t (*RequestDeviceSecurityInfoFunction)(const DeviceIdentify *identify,
                                                     const RequestOption *option,
                                                     DeviceSecurityInfo **info);

typedef int32_t (*RequestDeviceSecurityInfoAsyncFunction)(const DeviceIdentify *identify,
                                                          const RequestOption *option,
                                                          DeviceSecurityInfoCallback callback);

typedef void (*FreeDeviceSecurityInfoFunction)(DeviceSecurityInfo *info);

typedef uint32_t (*GetDeviceSecurityLevelValueFunction)(const DeviceSecurityInfo *info, int32_t *level);

typedef struct {
    RequestDeviceSecurityInfoFunction requestDeviceSecurityInfo;
    RequestDeviceSecurityInfoAsyncFunction requestDeviceSecurityInfoAsync;
    FreeDeviceSecurityInfoFunction freeDeviceSecurityInfo;
    GetDeviceSecurityLevelValueFunction getDeviceSecurityLevelValue;
    int32_t version;
} DeviceSecEnv;

int32_t StartDevslEnv(void);

void FinishDevslEnv(void);

int32_t GetDeviceSecLevelByUdid(const uint8_t *udid, uint32_t udidLen, int32_t *devLevel);

int32_t CompareUdid(DEVSLQueryParams *queryParamsL, DEVSLQueryParams *queryParamsR);

int32_t GetDeviceSecLevelByUdidAsync(const uint8_t *udid, uint32_t udidLen);

uint32_t GetDataSecLevelByDevSecLevel(int32_t devLevel);

int32_t UpdateCallbackListParams(DEVSLQueryParams *queryParams, HigestSecInfoCallback *callback);

#ifdef __cplusplus
}
#endif
#endif