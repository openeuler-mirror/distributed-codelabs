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

#include "openauthsession_fuzzer.h"
#include <cstddef>
#include <cstdint>
#include <securec.h>
#include "softbus_def.h"
#include "inner_session.h"
#include "session.h"
#include "softbus_utils.h"

namespace OHOS {
ConnectionAddr g_addr;
const char *g_testSessionName   = "com.plrdtest.dsoftbus";

void OpenAuthSessionTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    char tmp[PKG_NAME_SIZE_MAX + 1] = {0};
    if (memcpy_s(tmp, sizeof(tmp) - 1, data, size) != EOK) {
        return;
    }

    OpenAuthSession(g_testSessionName, &g_addr, -1, (const char*)tmp);
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::OpenAuthSessionTest(data, size);

    return 0;
}
