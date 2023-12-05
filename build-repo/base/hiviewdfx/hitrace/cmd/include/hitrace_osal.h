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

#ifndef BIN_INCLUDE_HITRACE_CAPTURE_H
#define BIN_INCLUDE_HITRACE_CAPTURE_H

#include <cstdint>
#include <iosfwd>
#include <string>

namespace OHOS {
namespace HiviewDFX {
namespace HitraceOsal {
enum TraceType { USER, KERNEL };
constexpr int MAX_SYS_FILES = 11;
struct TagCategory {
    std::string name;
    std::string description;
    uint64_t tag;
    TraceType type;
    struct {
        std::string path;
    } SysFiles[MAX_SYS_FILES];
};

std::string GetPropertyInner(const std::string& property, const std::string& value);
bool SetPropertyInner(const std::string& property, const std::string& value);
bool RefreshBinderServices(void);
bool RefreshHalServices(void);
}
}
}
#endif // BIN_INCLUDE_HITRACE_CAPTURE_H
