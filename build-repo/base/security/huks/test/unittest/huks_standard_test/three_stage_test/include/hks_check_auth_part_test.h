/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HKS_CHECK_AUTH_TEST_H
#define HKS_CHECK_AUTH_TEST_H

#include <string>
#include <vector>
#include "hks_three_stage_test_common.h"
namespace Unittest::CheckAuthTest {
struct TestAuthCaseParams {
    std::vector<HksParam> genParams;
    std::vector<HksParam> initParams;
    HksErrorCode initResult = HksErrorCode::HKS_SUCCESS;
};

static const uint32_t IV_SIZE = 16;
static uint8_t IV[IV_SIZE] = {0};

int HksCheckAuthTest001(void);
int HksCheckAuthTest002(void);
int HksCheckAuthTest003(void);
int HksCheckAuthTest004(void);
int HksCheckAuthTest005(void);
int HksCheckAuthTest006(void);
int HksCheckAuthTest007(void);
int HksCheckAuthTest008(void);
}
#endif // HKS_CHECK_AUTH_TEST_H

