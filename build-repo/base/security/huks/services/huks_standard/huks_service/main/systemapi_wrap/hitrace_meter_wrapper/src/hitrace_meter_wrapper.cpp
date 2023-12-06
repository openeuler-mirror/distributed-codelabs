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

#include "hitrace_meter_wrapper.h"

#include "hks_log.h"
// #include "hitrace_meter.h"

#include <cstdlib>

void HksTraceMeterStart(uint64_t label, const char *value, float limit)
{
    return;
    // const std::string valueString(value);
    // StartTrace(label, valueString, limit);
}

void HksTraceMeterFinish(uint64_t label)
{
    return;
    // FinishTrace(label);
}