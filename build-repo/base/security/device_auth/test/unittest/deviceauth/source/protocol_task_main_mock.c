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

#include "hc_log.h"
#include "hc_types.h"
#include "iso_task_main.h"
#include "iso_base_cur_task.h"
#include "iso_client_task.h"
#include "iso_server_task.h"
#include "pake_v1_task_main.h"
#include "pake_base_cur_task.h"
#include "pake_v1_client_task.h"
#include "pake_v1_server_task.h"
#include "protocol_task_main_mock.h"

static bool g_isIsoSupported = true;
static bool g_isPakeV1Supported = true;

void SetPakeV1Supported(bool isSupported)
{
    g_isPakeV1Supported = isSupported;
}

bool IsSupportPakeV1()
{
    if (g_isPakeV1Supported) {
        LOGI("pake v1 support.");
    } else {
        LOGI("pake v1 not support.");
    }
    return g_isPakeV1Supported;
}

SubTaskBase *CreatePakeV1SubTask(const CJson *in)
{
    bool isClient = true;
    if (GetBoolFromJson(in, FIELD_IS_CLIENT, &isClient) != HC_SUCCESS) {
        LOGE("Get isClient failed.");
        return NULL;
    }
    if (isClient) {
        return CreatePakeV1ClientTask(in);
    } else {
        return CreatePakeV1ServerTask(in);
    }
}

void SetIsoSupported(bool isSupported)
{
    g_isIsoSupported = isSupported;
}

bool IsIsoSupported(void)
{
    if (g_isIsoSupported) {
        LOGI("iso support.");
    } else {
        LOGI("iso not support.");
    }
    return g_isIsoSupported;
}

SubTaskBase *CreateIsoSubTask(const CJson *in)
{
    bool isClient = true;
    if (GetBoolFromJson(in, FIELD_IS_CLIENT, &isClient) != HC_SUCCESS) {
        LOGE("Get isClient failed.");
        return NULL;
    }
    if (isClient) {
        return CreateIsoClientTask(in);
    } else {
        return CreateIsoServerTask(in);
    }
}