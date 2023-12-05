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

#include "systemabilitymanager_fuzzer.h"

#include "if_system_ability_manager.h"
#include "sam_mock_permission.h"
#include "system_ability_manager.h"

#include <cstddef>
#include <cstdint>
#include <unistd.h>
#include <cstdlib>
#include <fcntl.h>

namespace OHOS {
namespace Samgr {
namespace {
    constexpr size_t THRESHOLD = 10;
    constexpr uint8_t MAX_CALL_TRANSACTION = 64;
    constexpr int32_t OFFSET = 4;
    const std::u16string SAMGR_INTERFACE_TOKEN = u"ohos.samgr.accessToken";
    bool flag_ = false;
}

uint32_t Convert2Uint32(const uint8_t* ptr)
{
    if (ptr == nullptr) {
        return 0;
    }
    return (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | (ptr[3]); // this is a general method of converting in fuzz
}

void FuzzSystemAbilityManager(const uint8_t* rawData, size_t size)
{
    SamMockPermission::MockPermission();
    uint32_t code = Convert2Uint32(rawData);
    rawData = rawData + OFFSET;
    size = size - OFFSET;
    MessageParcel data;
    data.WriteInterfaceToken(SAMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;
    sptr<SystemAbilityManager> manager = SystemAbilityManager::GetInstance();
    if (!flag_) {
        manager->Init();
        flag_ = true;
    }
    manager->OnRemoteRequest(code % MAX_CALL_TRANSACTION, data, reply, option);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::Samgr::THRESHOLD) {
        return 0;
    }

    OHOS::Samgr::FuzzSystemAbilityManager(data, size);
    return 0;
}

