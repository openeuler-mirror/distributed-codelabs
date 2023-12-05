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

#ifndef DEVICE_SECURITY_INFO_H
#define DEVICE_SECURITY_INFO_H

#include <stdint.h>

#include "device_security_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DeviceSecurityInfo DeviceSecurityInfo;

/**
 * Callback for requesting device security level information.
 */
typedef void DeviceSecurityInfoCallback(const DeviceIdentify *identify, struct DeviceSecurityInfo *info);

/**
 * Synchronously requests for the device security level of the local/neighbor devices.
 *
 * @param [in] identify Device identifier.
 * @param [in] option Option value.
 * @param [out] info Information containing the device security level.
 * @return
 */
int32_t RequestDeviceSecurityInfo(const DeviceIdentify *identify, const RequestOption *option,
    DeviceSecurityInfo **info);

/**
 * Asynchronously requests for the device security level of the local/neighbor devices.
 *
 * @param [in] identify Identify Device identifier.
 * @param [in] option Option value.
 * @param [in] info Callback used to return the device security level information.
 * @return
 */
int32_t RequestDeviceSecurityInfoAsync(const DeviceIdentify *identify, const RequestOption *option,
    DeviceSecurityInfoCallback callback);

/**
 * Releases device security level information.
 * @param info Device security level information in RequestDeviceSecurityInfo.
 */
void FreeDeviceSecurityInfo(DeviceSecurityInfo *info);

/**
 * Gets the device security level from DeviceSecurityInfo.
 * @param [in] info Device security level information.
 * @param [out] level Device security level.
 * @return
 */
int32_t GetDeviceSecurityLevelValue(const DeviceSecurityInfo *info, int32_t *level);

#ifdef __cplusplus
}
#endif

#endif // DEVICE_SECURITY_INFO_H
