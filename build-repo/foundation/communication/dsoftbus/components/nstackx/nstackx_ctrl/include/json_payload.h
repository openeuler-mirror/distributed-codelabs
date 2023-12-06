/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef JSON_PAYLOAD_H
#define JSON_PAYLOAD_H

#include <stdint.h>
#include "nstackx.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NSTACKX_MAX_URI_BUFFER_LENGTH 64

#define JSON_COAP_URI "coapUri"
#define JSON_CAPABILITY_BITMAP "capabilityBitmap"
#define JSON_DEVICE_ID "deviceId"
#define JSON_DEVICE_NAME "devicename"
#define JSON_DEVICE_WLAN_IP "wlanIp"
#define JSON_DEVICE_TYPE "type"
#define JSON_HICOM_VERSION "hicomversion"
#define JSON_REQUEST_MODE "mode"
#define JSON_DEVICE_HASH "deviceHash"
#define JSON_SERVICE_DATA "serviceData"
#define JSON_BUSINESS_TYPE "bType"
#define JSON_BUSINESS_DATA "bData"
#define JSON_EXTEND_SERVICE_DATA "extendServiceData"

#ifdef DFINDER_USE_MINI_NSTACKX
#define COAP_DEVICE_DISCOVER_URI "device_discover"
#endif

struct DeviceInfo;

#ifdef DFINDER_SUPPORT_MULTI_NIF
char *PrepareServiceDiscoverWithIdx(uint8_t isBroadcast, uint32_t idx);
#else
char *PrepareServiceDiscover(uint8_t isBroadcast);
#endif
int32_t ParseServiceDiscover(const uint8_t *buf, struct DeviceInfo *deviceInfo, char **remoteUrlPtr);

#ifdef __cplusplus
}
#endif
#endif /* #ifndef JSON_PAYLOAD_H */
