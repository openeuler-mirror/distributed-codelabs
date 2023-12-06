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

#ifndef HITRACE_ADAPTER_H
#define HITRACE_ADAPTER_H

#include <stdint.h>

#define TRACE_TAG_AUTH_PROCESS "auth_process"

#ifdef __cplusplus
extern "C" {
#endif

void DevAuthStartTrace(const char *value);

void DevAuthFinishTrace(void);

#ifdef __cplusplus
}
#endif

#endif // HITRACE_ADAPTER_H