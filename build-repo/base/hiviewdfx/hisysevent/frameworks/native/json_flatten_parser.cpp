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

#include "json_flatten_parser.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <iosfwd>
#include <string>
#include <vector>

namespace OHOS {
namespace HiviewDFX {
namespace {
    struct Initializer {
        Initializer()
        {
            JsonFlattenParser::Initialize();
        }
    };
}

uint8_t JsonFlattenParser::charFilter[JsonFlattenParser::CHAR_RANGE] { 0 };

void JsonFlattenParser::Initialize()
{
    for (char c = '0'; c <= '9'; c++) {
        charFilter[static_cast<uint8_t>(c)] = NUMBER_FLAG;
    }
    charFilter[static_cast<uint8_t>('-')] = NUMBER_FLAG;
    charFilter[static_cast<uint8_t>('+')] = NUMBER_FLAG;
    charFilter[static_cast<uint8_t>('.')] = NUMBER_FLAG;
    charFilter[static_cast<uint8_t>('"')] = STRING_FLAG;
    charFilter[static_cast<uint8_t>('{')] = BRACKET_FLAG;
    charFilter[static_cast<uint8_t>('[')] = BRACKET_FLAG;
}

JsonFlattenParser::JsonFlattenParser(const std::string& json)
{
    static Initializer initialize;
    Parse(json);
}

void JsonFlattenParser::Parse(const std::string& json)
{
    curPos = 0;
    kvList.clear();
    while (curPos < json.length()) {
        if (charFilter[static_cast<uint8_t>(json[curPos])] != STRING_FLAG) {
            ++curPos;
            continue;
        }
        std::string key = ParseKey(json);
        std::string val = ParseValue(json);
        kvList.emplace_back(key, val);
    }
}

std::string JsonFlattenParser::Print(PrintKvHandler handler)
{
    std::string json = "{";
    if (!kvList.empty()) {
        for (size_t i = 0; i < kvList.size() - 1; i++) {
            json += (handler(kvList[i]) + ",");
        }
        json += handler(kvList.back());
    }
    json += "}";
    return json;
}

std::string JsonFlattenParser::ParseKey(const std::string& json)
{
    std::string key;
    ++curPos; // eat left quotation
    while (curPos < json.length()) {
        if (charFilter[static_cast<uint8_t>(json[curPos])] == STRING_FLAG) {
            break;
        }
        key.push_back(json[curPos]);
        ++curPos;
    }
    ++curPos; // eat right quotation
    return key;
}

std::string JsonFlattenParser::ParseValue(const std::string& json)
{
    std::string value;
    bool valueParsed = false;
    while (curPos < json.length()) {
        int charCode = static_cast<uint8_t>(json[curPos]);
        switch (charFilter[charCode]) {
            case BRACKET_FLAG:
                value = ParseBrackets(json, json[curPos]);
                valueParsed = true;
                break;
            case NUMBER_FLAG:
                value = ParseNumer(json);
                valueParsed = true;
                break;
            case STRING_FLAG:
                value = ParseString(json);
                valueParsed = true;
                break;
            default:
                ++curPos;
                valueParsed = false;
                break;
        }
        if (valueParsed) {
            break;
        }
    }
    return value;
}

std::string JsonFlattenParser::ParseNumer(const std::string& json)
{
    std::string number;
    while (curPos < json.length()) {
        if (charFilter[static_cast<uint8_t>(json[curPos])] != NUMBER_FLAG) {
            break;
        }
        number.push_back(json[curPos]);
        ++curPos;
    }
    return number;
}

std::string JsonFlattenParser::ParseString(const std::string& json)
{
    std::string txt;
    txt.push_back(json[curPos++]);
    while (curPos < json.length()) {
        if (charFilter[static_cast<uint8_t>(json[curPos])] == STRING_FLAG &&
            json[curPos - 1] != '\\') {
            break;
        }
        txt.push_back(json[curPos]);
        ++curPos;
    }
    txt.push_back(json[curPos++]);
    return txt;
}

std::string JsonFlattenParser::ParseBrackets(const std::string& json, char leftBracket)
{
    std::string val;
    char rightBracket = leftBracket + 2; // 2: '[' + 2 = ']', '{' + 2 = '}'
    int counter = 1;
    val.push_back(json[curPos++]);
    while (curPos < json.length()) {
        if (json[curPos] == leftBracket) {
            ++counter;
        } else if (json[curPos] == rightBracket) {
            --counter;
            if (counter == 0) {
                break;
            }
        }
        val.push_back(json[curPos++]);
    }
    val.push_back(json[curPos++]);
    return val;
}
} // namespace HiviewDFX
} // namespace OHOS
