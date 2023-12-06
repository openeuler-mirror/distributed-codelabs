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

#include "devauthservregdatachangelistener_fuzzer.h"
#include "device_auth.h"
#include "device_auth_defines.h"
#include "hc_dev_info.h"
#include "hc_log.h"

namespace OHOS {
    static void OnGroupCreated(const char *groupInfo) {}

    static void OnGroupDeleted(const char *groupInfo) {}

    static void OnDeviceBound(const char *peerUdid, const char *groupInfo) {}

    static void OnDeviceUnBound(const char *peerUdid, const char *groupInfo) {}

    static void OnDeviceNotTrusted(const char *peerUdid) {}

    static void OnLastGroupDeleted(const char *peerUdid, int groupType) {}

    static void OnTrustedDeviceNumChanged(int curTrustedDeviceNum) {}

    static DataChangeListener g_listener = {
        .onGroupCreated = OnGroupCreated,
        .onGroupDeleted = OnGroupDeleted,
        .onDeviceBound = OnDeviceBound,
        .onDeviceUnBound = OnDeviceUnBound,
        .onDeviceNotTrusted = OnDeviceNotTrusted,
        .onLastGroupDeleted = OnLastGroupDeleted,
        .onTrustedDeviceNumChanged = OnTrustedDeviceNumChanged,
    };

    bool FuzzDoRegDataChangeListener(const uint8_t* data, size_t size)
    {
        if (data == nullptr) {
            return false;
        }
        InitDeviceAuthService();
        std::string appId(reinterpret_cast<const char *>(data), size);
        const DeviceGroupManager *gmInstance = GetGmInstance();
        gmInstance->regDataChangeListener(appId.c_str(), &g_listener);
        DestroyDeviceAuthService();
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::FuzzDoRegDataChangeListener(data, size);
    return 0;
}

