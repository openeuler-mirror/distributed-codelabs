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

#ifndef DEV_AUTH_BIGDATA_EVENT_H
#define DEV_AUTH_BIGDATA_EVENT_H

#include "stdint.h"
#include "json_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

void ReportHiEventCoreFuncInvoke(int32_t eventId, int32_t osAccountId, CJson *params, int32_t result);

#ifdef __cplusplus
}
#endif

#endif // DEV_AUTH_BIGDATA_EVENT_H
