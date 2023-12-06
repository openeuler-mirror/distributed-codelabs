/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SEC_UTILS_DATETIME_H
#define SEC_UTILS_DATETIME_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DateTime {
    uint16_t hour;
    uint16_t min;
    uint16_t sec;
    uint16_t msec;
    uint16_t year;
    uint16_t mon;
    uint16_t day;
} DateTime;

uint64_t GetMillisecondSinceBoot(void);

uint64_t GetMillisecondSince1970(void);

bool GetDateTimeByMillisecondSince1970(uint64_t input, DateTime *datetime);

bool GetDateTimeByMillisecondSinceBoot(uint64_t input, DateTime *datetime);

#ifdef __cplusplus
}
#endif

#endif // SEC_UTILS_DATETIME_H