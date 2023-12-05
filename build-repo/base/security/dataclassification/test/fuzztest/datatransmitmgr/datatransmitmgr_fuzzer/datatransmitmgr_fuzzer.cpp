/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "datatransmitmgr_fuzzer.h"

#include <stddef.h>
#include <stdint.h>

#include "securec.h"
#include "dev_slinfo_mgr.h"
#include "softbus_bus_center.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "accesstoken_kit.h"

namespace OHOS {
    static void tmpCallback(DEVSLQueryParams *queryParams, int32_t result, uint32_t levelInfo)
    {
        return;
    }

    static void GetLocalUdid(DEVSLQueryParams *queryParams)
    {
        const char *pkgName = "ohos.dslm";
        NodeBasicInfo info;
        (void)GetLocalNodeDeviceInfo(pkgName, &info);
        if (GetNodeKeyInfo(pkgName, info.networkId, NODE_KEY_UDID, (uint8_t *)(queryParams->udid), UDID_BUF_LEN) != 0) {
            return;
        }
        queryParams->udidLen = MAX_UDID_LENGTH;
    }

    void FuzzDoDataTransmitMgr(const uint8_t* data, size_t size)
    {
        if (data == nullptr) {
            return;
        }
        if (size < MAX_UDID_LENGTH) {
            return;
        }
        uint32_t levelInfo = 0;
        DEVSLQueryParams queryParams;
        (void)memset_s(&queryParams, sizeof(DEVSLQueryParams), 0, sizeof(DEVSLQueryParams));
        queryParams.udidLen = MAX_UDID_LENGTH;
        (void)memcpy_s(queryParams.udid, queryParams.udidLen, data, MAX_UDID_LENGTH);

        int32_t ret = DATASL_OnStart();
        if (ret != DEVSL_SUCCESS) {
            return;
        }
        (void)DATASL_GetHighestSecLevelAsync(&queryParams, tmpCallback);
        (void)DATASL_GetHighestSecLevelAsync(nullptr, tmpCallback);
        (void)DATASL_GetHighestSecLevelAsync(&queryParams, nullptr);
        (void)DATASL_GetHighestSecLevel(&queryParams, &levelInfo);
        (void)DATASL_GetHighestSecLevel(nullptr, &levelInfo);
        (void)DATASL_GetHighestSecLevel(&queryParams, nullptr);
        GetLocalUdid(&queryParams);
        (void)DATASL_GetHighestSecLevelAsync(&queryParams, tmpCallback);
        (void)DATASL_GetHighestSecLevel(&queryParams, &levelInfo);
        DATASL_OnStop();
    }
}

static void NativeTokenGet()
{
    uint64_t tokenId;
    const char **perms = new const char *[1];
    perms[0] = "ohos.permission.DISTRIBUTED_DATASYNC";
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 1,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .aplStr = "system_basic",
    };

    infoInstance.processName = "DevSLMgrTest";
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
    delete[] perms;
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    NativeTokenGet();
    OHOS::FuzzDoDataTransmitMgr(data, size);
    return 0;
}