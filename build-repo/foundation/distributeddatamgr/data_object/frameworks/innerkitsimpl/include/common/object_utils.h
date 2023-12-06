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
#ifndef OBJECT_UTILS_H
#define OBJECT_UTILS_H

#include <string>

namespace OHOS::ObjectStore {
class ObjectUtils final {
public:
    ObjectUtils() = delete;
    ~ObjectUtils() = delete;

    static std::string GenObjectIdPrefix(
        const std::string &host, const std::string &user, const std::string &bundle, const std::string &store);

    static std::string GetObjectHost(const std::string &objectId);

    static std::string GetObjectStoreName(const std::string &objectId);
};
} // namespace OHOS::ObjectStore

#endif