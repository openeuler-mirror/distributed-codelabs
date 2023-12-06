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

#ifndef HKS_IMPORT_SIGN_VERIFY_TEST_H
#define HKS_IMPORT_SIGN_VERIFY_TEST_H

#include "hks_three_stage_test_common.h"

namespace Unittest::ImportSignVerifyTest {
constexpr uint32_t LENGTH_TO_BE_OPERATED = 200;
constexpr uint32_t LENGTH_MAX = 512;

constexpr uint32_t TAG_ALG_ID = 0;
constexpr uint32_t TAG_PURPOSE_ID = 1;
constexpr uint32_t TAG_KEY_SIZE_ID = 2;
constexpr uint32_t TAG_DIGEST_ID = 3;
constexpr uint32_t TAG_IMPOT_TYPE_ID = 4;
}

#endif // HKS_IMPORT_SIGN_VERIFY_TEST_H
