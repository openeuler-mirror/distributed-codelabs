/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "string_util.h"

#include "securec.h"

namespace OHOS {
namespace HiviewDFX {
namespace StringUtil {
int CopyCString(char* dst, const std::string& src, size_t len)
{
    if (src.length() > len) {
        return -1;
    }
    return strcpy_s(dst, src.length() + 1, src.c_str());
}

int CreateCString(char** dst, const std::string& src, size_t len)
{
    if (src.length() > len) {
        return -1;
    }
    char* data = new(std::nothrow) char[src.length() + 1]{0};
    if (auto res = strcpy_s(data, src.length() + 1, src.c_str()); res != 0) {
        delete[] data;
        return res;
    }
    *dst = data;
    return 0;
}

int ConvertCString(const std::string& str, char** sp, size_t len)
{
    if (str.length() > len) {
        return -1;
    }
    char* data = new(std::nothrow) char[str.length() + 1]{0};
    if (auto res = strcpy_s(data, str.length() + 1, str.c_str()); res != 0) {
        StringUtil::DeletePointer<char>(&data);
        return res;
    }
    *sp = data;
    return 0;
}

int ConvertCStringVec(const std::vector<std::string>& vec, char*** strs, size_t& len)
{
    if (vec.empty()) {
        return 0;
    }
    len = vec.size();
    char** data = new(std::nothrow) char* [len]{0};
    for (size_t i = 0; i < len; i++) {
        if (int res = ConvertCString(vec[i], &data[i]); res != 0) {
            StringUtil::DeletePointers<char>(&data, i);
            return res;
        }
    }
    *strs = data;
    return 0;
}

void MemsetSafe(void* dest, size_t destSize)
{
    (void)memset_s(dest, destSize, 0, destSize);
}
} // namespace StringUtil
} // namespace HiviewDFX
} // namespace OHOS
