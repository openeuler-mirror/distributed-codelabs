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

#include "devauthservprocessauthdata_fuzzer.h"
#include "device_auth.h"
#include "device_auth_defines.h"
#include "hc_dev_info.h"
#include "hc_log.h"
#include "hc_types.h"
#include "json_utils.h"
#include "securec.h"
#include <unistd.h>

namespace OHOS {
    static void OnError(int64_t requestId, int operationCode, int errorCode, const char *errorReturn)
    {
        LOGE("error return: %s", errorReturn);
    }

    static void OnFinish(int64_t requestId, int operationCode, const char *authReturn)
    {
        LOGI("return value: %s", authReturn);
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

    static DeviceAuthCallback g_gaCallback = {
        .onTransmit = OnTransmit,
        .onSessionKeyReturned = OnSessionKeyReturned,
        .onFinish = OnFinish,
        .onError = OnError,
        .onRequest = OnRequest,
    };

    bool FuzzDoProcessAuthData(const uint8_t* data, size_t size)
    {
        if (data == nullptr) {
            return false;
        }
        InitDeviceAuthService();
        const GroupAuthManager *gaInstance = GetGaInstance();
        int64_t reqId = 123;
        uint8_t *authData = reinterpret_cast<uint8_t *>(HcMalloc(size + 1, 0));
        if (authData == nullptr) {
            DestroyDeviceAuthService();
            return false;
        }
        if (memcpy_s(authData, size, data, size) != EOK) {
            HcFree(authData);
            DestroyDeviceAuthService();
            return false;
        }
        gaInstance->processData(reqId, authData, size, &g_gaCallback);
        HcFree(authData);
        sleep(1);
        DestroyDeviceAuthService();
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::FuzzDoProcessAuthData(data, size);
    return 0;
}

