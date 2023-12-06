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

#ifndef DSLM_DEVICE_LIST_H
#define DSLM_DEVICE_LIST_H

#include <stdbool.h>
#include <stdint.h>

#include "device_security_defines.h"
#include "dslm_core_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ONLINE_STATUS_ONLINE 1
#define ONLINE_STATUS_OFFLINE 0

typedef void (*ProcessDumpFunction)(const DslmDeviceInfo *info, int32_t dumpHandle);

DslmDeviceInfo *CreatOrGetDslmDeviceInfo(const DeviceIdentify *device);

DslmDeviceInfo *GetDslmDeviceInfo(const DeviceIdentify *device);

bool IsSameDevice(const DeviceIdentify *first, const DeviceIdentify *second);

void ForEachDeviceDump(const ProcessDumpFunction dumper, int32_t dumpHandle);

#ifdef __cplusplus
}
#endif

#endif // DSLM_DEVICE_LIST_H
