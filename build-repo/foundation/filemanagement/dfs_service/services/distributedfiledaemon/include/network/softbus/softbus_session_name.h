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

#ifndef SOFTBUS_SESSION_NAME_H
#define SOFTBUS_SESSION_NAME_H

#include <string>

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class SoftbusSessionName final {
public:
    explicit SoftbusSessionName(std::string path) : path_(path) {}
    ~SoftbusSessionName() = default;
    std::string ToString()
    {
        return prefix + path_;
    }

private:
    const std::string prefix = "DistributedFileService";
    std::string path_;
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // SOFTBUS_SESSION_NAME_H