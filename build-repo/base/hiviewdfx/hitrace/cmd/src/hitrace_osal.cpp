/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "hitrace_osal.h"

#include <cstdio>
#include <string>

#include "parameters.h"

namespace OHOS {
namespace HiviewDFX {
namespace HitraceOsal {
bool SetPropertyInner(const std::string& property, const std::string& value)
{
    bool result = OHOS::system::SetParameter(property, value);
    if (!result) {
        fprintf(stderr, "Error: Failed to set %s property.\n", value.c_str());
    }
    return result;
}

std::string GetPropertyInner(const std::string& property, const std::string& value)
{
    return OHOS::system::GetParameter(property, value);
}

bool RefreshBinderServices(void)
{
    return true;
}

bool RefreshHalServices(void)
{
    return true;
}
}
}
}
