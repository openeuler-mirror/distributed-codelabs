/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef HISYSEVENT_FRAMEWORKS_NATIVE_INCLUDE_JSON_FLATTEN_PARSER_H
#define HISYSEVENT_FRAMEWORKS_NATIVE_INCLUDE_JSON_FLATTEN_PARSER_H

#include <functional>
#include <vector>

namespace OHOS {
namespace HiviewDFX {
using KV = std::pair<std::string, std::string>;
using PrintKvHandler = std::function<std::string(KV&)>;
class JsonFlattenParser {
public:
    JsonFlattenParser(const std::string& json);

public:
    static void Initialize();

public:
    void Parse(const std::string& json);
    std::string Print(PrintKvHandler handler);

public:
    static constexpr uint8_t BRACKET_FLAG { 3 };
    static constexpr uint8_t NUMBER_FLAG { 1 };
    static constexpr uint8_t STRING_FLAG { 2 };
    static constexpr int CHAR_RANGE { 256 };

private:
    std::string ParseBrackets(const std::string& json, char leftBracket);
    std::string ParseKey(const std::string& json);
    std::string ParseNumer(const std::string& json);
    std::string ParseString(const std::string& json);
    std::string ParseValue(const std::string& json);

private:
    static uint8_t charFilter[CHAR_RANGE];
    std::vector<KV> kvList;
    size_t curPos {0};
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // HISYSEVENT_FRAMEWORKS_NATIVE_INCLUDE_JSON_FLATTEN_PARSER_H
