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

#include "stringfilter.h"

#include <iosfwd>
#include <istream>
#include <ostream>
#include <sstream>
#include <streambuf>
#include <string>
#include <utility>
#include <vector>

namespace OHOS {
namespace HiviewDFX {
char StringFilter::charTab_[StringFilter::CHAR_RANGE][StringFilter::MAP_STR_LEN];
int StringFilter::statTab_[StringFilter::STATE_NUM][StringFilter::CHAR_RANGE];
StringFilter StringFilter::filter_;

StringFilter::StringFilter()
{
    // build mapping table for special chars
    std::vector<std::pair<char, std::string>> mapping = {
        {'\\', "\\\\"}, {'\"', "\\\""}, {'\b', "\\b"},
        {'\f', "\\f"}, {'\n', "\\n"}, {'\r', "\\r"}, {'\t', "\\t"}
    };
    std::stringstream ss;
    for (int i = 0; i < CHAR_RANGE; ++i) {
        ss.clear();
        ss << static_cast<char>(i);
        ss >> charTab_[i];
    }
    for (auto e : mapping) {
        ss.clear();
        ss << e.second;
        ss >> charTab_[static_cast<int>(e.first)];
    }

    int i = 0;
    // build state transition table
    for (i = 0; i < CHAR_RANGE; ++i) {
        statTab_[STATE_BEGIN][i] = STATE_STOP;
    }
    for (i = 'a'; i <= 'z'; ++i) {
        statTab_[STATE_BEGIN][i] = STATE_RUN;
    }
    for (i = 'A'; i <= 'Z'; ++i) {
        statTab_[STATE_BEGIN][i] = STATE_RUN;
    }

    for (i = 0; i < CHAR_RANGE; ++i) {
        statTab_[STATE_RUN][i] = STATE_STOP;
    }
    for (i = '0'; i <= '9'; ++i) {
        statTab_[STATE_RUN][i] = STATE_RUN;
    }
    for (i = 'a'; i <= 'z'; ++i) {
        statTab_[STATE_RUN][i] = STATE_RUN;
    }
    for (i = 'A'; i <= 'Z'; ++i) {
        statTab_[STATE_RUN][i] = STATE_RUN;
    }
    statTab_[STATE_RUN][static_cast<int>('_')] = STATE_RUN;
}

std::string StringFilter::EscapeToRaw(const std::string &text)
{
    std::string rawText = "";
    for (auto c : text) {
        unsigned int ic = static_cast<unsigned int>(c);
        if (ic < CHAR_RANGE && charTab_[ic][1]) {
            rawText.append(charTab_[ic]);
        } else {
            rawText.push_back(c);
        }
    }
    return rawText;
}

bool StringFilter::IsValidName(const std::string &text, unsigned int maxSize)
{
    if (text.empty()) {
        return false;
    }
    if (text.length() > maxSize) {
        return false;
    }
    int state = STATE_BEGIN;
    for (auto c : text) {
        unsigned int ic = static_cast<unsigned int>(c);
        if ((ic >= CHAR_RANGE) || (state < 0) || (state >= StringFilter::STATE_NUM)) {
            return false;
        }
        state = statTab_[state][ic];
        if (state == STATE_STOP) {
            return false;
        }
    }
    return true;
}

StringFilter& StringFilter::GetInstance()
{
    return filter_;
}
} // namespace HiviewDFX
} // namespace OHOS

