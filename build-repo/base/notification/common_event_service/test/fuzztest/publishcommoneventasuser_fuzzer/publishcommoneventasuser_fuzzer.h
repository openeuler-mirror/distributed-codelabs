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

#ifndef TEST_FUZZTEST_PUBLISHCOMMONEVENTASUSER_FUZZER_PUBLISHCOMMONEVENTASUSER_FUZZER_H
#define TEST_FUZZTEST_PUBLISHCOMMONEVENTASUSER_FUZZER_PUBLISHCOMMONEVENTASUSER_FUZZER_H

#define FUZZ_PROJECT_NAME "publishcommoneventasuser_fuzzer"

#include <cstdint>

uint32_t U32_AT(const uint8_t *ptr)
{
    // convert fuzz input data to an integer
    return (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3];
}

#endif // TEST_FUZZTEST_PUBLISHCOMMONEVENTASUSER_FUZZER_PUBLISHCOMMONEVENTASUSER_FUZZER_H
