/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "devauthservauthdevice_fuzzer.h"
#include "device_auth.h"
#include "device_auth_defines.h"
#include "hc_dev_info.h"
#include "hc_log.h"
#include "json_utils.h"
#include "securec.h"
#include <unistd.h>

#define TEST_APP_NAME "ohos.security.deviceauth"

namespace OHOS {
    static char g_groupId[128] = { 0 };

    static void OnError(int64_t requestId, int operationCode, int errorCode, const char *errorReturn)
    {
        LOGE("error return: %s", errorReturn);
    }

    static void OnFinish(int64_t requestId, int operationCode, const char *authReturn)
    {
        LOGI("return value: %s", authReturn);
        if (operationCode == GROUP_CREATE) {
            CJson *json = CreateJsonFromString(authReturn);
            if (json != nullptr) {
                const char *data = GetStringFromJson(json, FIELD_GROUP_ID);
                if (data != nullptr) {
                    (void)memcpy_s(g_groupId, strlen(data), data, strlen(data));
                    g_groupId[strlen(data)] = 0;
                }
                FreeJson(json);
            }
        }
    }

    static void OnSessionKeyReturned(int64_t requestId, const uint8_t *sessionKey, uint32_t sessionKeyLen) {}

    static bool OnTransmit(int64_t requestId, const uint8_t *data, uint32_t dataLen)
    {
        return true;
    }

    static char *OnRequest(int64_t requestId, int operationCode, const char* reqParam)
    {
        return nullptr;
    }

    static DeviceAuthCallback g_devAuthCallback = {
        .onTransmit = OnTransmit,
        .onSessionKeyReturned = OnSessionKeyReturned,
        .onFinish = OnFinish,
        .onError = OnError,
        .onRequest = OnRequest,
    };

    static void GenerateAuthParamsJson(CJson *json, const char *deviceId)
    {
        AddStringToJson(json, FIELD_PEER_CONN_DEVICE_ID, deviceId);
        AddStringToJson(json, FIELD_SERVICE_PKG_NAME, TEST_APP_NAME);
        AddBoolToJson(json, FIELD_IS_CLIENT, true);
        AddBoolToJson(json, FIELD_IS_DEVICE_LEVEL, true);
    }

    bool FuzzDoAuthDevice(const uint8_t* data, size_t size)
    {
        if (data == nullptr) {
            return false;
        }
        InitDeviceAuthService();
        std::string appId(reinterpret_cast<const char *>(data), size);
        const DeviceGroupManager *gmInstance = GetGmInstance();
        gmInstance->regCallback(appId.c_str(), &g_devAuthCallback);
        CJson *createJson = CreateJson();
        AddStringToJson(createJson, FIELD_GROUP_NAME, appId.c_str());
        char localUdid[INPUT_UDID_LEN] = { 0 };
        HcGetUdid(reinterpret_cast<uint8_t *>(localUdid), INPUT_UDID_LEN);
        AddStringToJson(createJson, FIELD_DEVICE_ID, localUdid);
        AddIntToJson(createJson, FIELD_GROUP_TYPE, PEER_TO_PEER_GROUP);
        char *createParams = PackJsonToString(createJson);
        FreeJson(createJson);
        int64_t reqId = 123;
        gmInstance->createGroup(0, reqId, appId.c_str(), createParams);
        sleep(1);
        ClearAndFreeJsonString(createParams);
        std::string deviceId(reinterpret_cast<const char *>(data), size);
        CJson *authParamsJson = CreateJson();
        GenerateAuthParamsJson(authParamsJson, deviceId.c_str());
        char *authParams = PackJsonToString(authParamsJson);
        FreeJson(authParamsJson);
        LOGI("auth params str:%s", authParams);
        const GroupAuthManager *gaInstance = GetGaInstance();
        gaInstance->authDevice(0, reqId, authParams, &g_devAuthCallback);
        sleep(1);
        ClearAndFreeJsonString(authParams);
        DestroyDeviceAuthService();
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::FuzzDoAuthDevice(data, size);
    return 0;
}

