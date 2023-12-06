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

#include "updatedbpassword_fuzzer.h"
#include "sqlite3_utils.h"
#include <securec.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include "softbus_adapter_mem.h"

namespace OHOS {
    bool UpdateDbPasswordFuzzTest(const uint8_t *data, size_t size)
    {
        if (data == nullptr || size == 0) {
            return true;
        }

        DbContext *ctx = nullptr;
        UpdateDbPassword(ctx, data, size);

        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::UpdateDbPasswordFuzzTest(data, size);
    return 0;
}