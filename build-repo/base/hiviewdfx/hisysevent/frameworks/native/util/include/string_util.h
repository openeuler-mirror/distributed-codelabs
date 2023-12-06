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
#ifndef HISYSEVENT_STRING_UTIL
#define HISYSEVENT_STRING_UTIL

#include <string>
#include <vector>

namespace OHOS {
namespace HiviewDFX {
namespace StringUtil {
int CopyCString(char* dst, const std::string& src, size_t len);
int CreateCString(char** dst, const std::string& src, size_t len = 1024); // default maxLen is 1024
int ConvertCString(const std::string& str, char** sp, size_t len = 8 * 1024); // default maxLen is 8*1024
int ConvertCStringVec(const std::vector<std::string>& vec, char*** strs, size_t& len);
void MemsetSafe(void* dest, size_t destSize);

template <typename T> void DeletePointer(T** p)
{
    if (p == nullptr || *p == nullptr) {
        return;
    }
    delete[] *p;
    *p = nullptr;
}

template <typename T> void DeletePointers(T*** ps, size_t len)
{
    if (ps == nullptr || *ps == nullptr) {
        return;
    }
    auto realPs = *ps;
    for (size_t i = 0; i < len; i++) {
        DeletePointer<T>(&realPs[i]);
    }
    delete[] realPs;
    realPs = nullptr;
}
} // namespace StringUtil
} // namespace HiviewDFX
} // namespace OHOS
#endif // HISYSEVENT_STRING_UTIL
