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

#include "async_common_event_result.h"
#include "asynccommoneventresult_fuzzer.h"
#include "securec.h"

namespace OHOS {
namespace {
    constexpr size_t U32_AT_SIZE = 4;
    constexpr uint8_t ENABLE = 2;
}
bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    std::string stringData(data);
    int32_t code = U32_AT(reinterpret_cast<const uint8_t*>(data));
    bool enabled = *data % ENABLE;
    std::shared_ptr<EventFwk::AsyncCommonEventResult> result = std::make_shared<EventFwk::AsyncCommonEventResult>(
        code, stringData, enabled, enabled, nullptr);
    if (result != nullptr) {
        // test SetCode function
        result->SetCode(code);
        // test GetCode function
        result->GetCode();
        // test SetData function
        result->SetData(stringData);
        // test GetData function
        result->GetData();
        // test SetCodeAndData function
        result->SetCodeAndData(code, stringData);
        // test AbortCommonEvent function
        result->AbortCommonEvent();
        // test ClearAbortCommonEvent function
        result->ClearAbortCommonEvent();
        // test GetAbortCommonEvent function
        result->GetAbortCommonEvent();
        // test IsOrderedCommonEvent function
        result->IsOrderedCommonEvent();
        // test IsStickyCommonEvent function
        return result->IsStickyCommonEvent();
    } else {
        return false;
    }
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        return 0;
    }

    if (size < OHOS::U32_AT_SIZE) {
        return 0;
    }

    char* ch = (char *)malloc(size + 1);
    if (ch == nullptr) {
        return 0;
    }

    (void)memset_s(ch, size + 1, 0x00, size + 1);
    if (memcpy_s(ch, size, data, size) != EOK) {
        free(ch);
        ch = nullptr;
        return 0;
    }

    OHOS::DoSomethingInterestingWithMyAPI(ch, size);
    free(ch);
    ch = nullptr;
    return 0;
}
