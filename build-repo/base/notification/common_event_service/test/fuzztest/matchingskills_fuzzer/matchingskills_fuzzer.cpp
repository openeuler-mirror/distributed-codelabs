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

#include "matchingskills_fuzzer.h"
#include "securec.h"
#define private public
#define protected public
#include "matching_skills.h"
#undef private
#undef protected

namespace OHOS {
namespace {
    constexpr size_t U32_AT_SIZE = 4;
}
bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    std::string stringData(data);
    size_t index = U32_AT(reinterpret_cast<const uint8_t*>(data));
    Parcel parcel;
    // test MatchingSkills class function
    EventFwk::MatchingSkills matchingSkills;
    // test HasEntity function
    matchingSkills.HasEntity(stringData);
    // test RemoveEntity function
    matchingSkills.RemoveEntity(stringData);
    // test CountEntities function
    matchingSkills.CountEntities();
    // test GetEvents function
    matchingSkills.GetEvents();
    // test RemoveEvent function
    matchingSkills.RemoveEvent(stringData);
    // test HasEvent function
    matchingSkills.HasEvent(stringData);
    // test HasScheme function
    matchingSkills.HasScheme(stringData);
    // test RemoveScheme function
    matchingSkills.RemoveScheme(stringData);
    // test CountSchemes function
    matchingSkills.CountSchemes();
    // test MatchEvent function
    matchingSkills.MatchEvent(stringData);
    // test MatchEntity function
    std::vector<std::string> permissions;
    permissions.emplace_back(stringData);
    matchingSkills.MatchEntity(permissions);
    // test MatchScheme function
    matchingSkills.MatchScheme(stringData);
    matchingSkills.GetEntity(index);
    matchingSkills.GetEvent(index);
    matchingSkills.GetScheme(index);
    matchingSkills.ReadFromParcel(parcel);
    matchingSkills.Unmarshalling(parcel);
    // test Match function
    AAFwk::Want want;
    return matchingSkills.Match(want);
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        return 0;
    }

    if (size < OHOS::U32_AT_SIZE) {
        return 0;
    }

    char* ch = (char *)malloc(size + 1);
    if (ch == nullptr) {
        return 0;
    }

    (void)memset_s(ch, size + 1, 0x00, size + 1);
    if (memcpy_s(ch, size, data, size) != EOK) {
        free(ch);
        ch = nullptr;
        return 0;
    }

    OHOS::DoSomethingInterestingWithMyAPI(ch, size);
    free(ch);
    ch = nullptr;
    return 0;
}
