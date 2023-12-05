/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef ISO_AUTH_CLIENT_TASK_H
#define ISO_AUTH_CLIENT_TASK_H

#include "account_task_main.h"
#include "account_version_util.h"
#include "json_utils.h"
#include "iso_auth_task_common.h"

typedef struct {
    TaskBase taskBase;
    IsoAuthParams params;
} IsoAuthClientTask;

#ifdef __cplusplus
extern "C" {
#endif

TaskBase *CreateIsoAuthClientTask(const CJson *in, CJson *out, const AccountVersionInfo *verInfo);

#ifdef __cplusplus
}
#endif

#endif