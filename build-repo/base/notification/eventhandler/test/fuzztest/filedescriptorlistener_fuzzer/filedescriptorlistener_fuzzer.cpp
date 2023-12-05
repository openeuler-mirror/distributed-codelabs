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

#define private public
#define protected public
#include "file_descriptor_listener.h"
#undef private
#undef protected
#include "filedescriptorlistener_fuzzer.h"
#include "securec.h"

namespace OHOS {
namespace {
    constexpr size_t U32_AT_SIZE = 4;
}
bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    int32_t fileDescriptor = U32_AT(reinterpret_cast<const uint8_t*>(data));
    auto listener = std::make_shared<AppExecFwk::FileDescriptorListener>();
    listener -> OnReadable(fileDescriptor);
    listener -> OnWritable(fileDescriptor);
    listener -> OnShutdown(fileDescriptor);
    listener -> OnException(fileDescriptor);
    return true;
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
