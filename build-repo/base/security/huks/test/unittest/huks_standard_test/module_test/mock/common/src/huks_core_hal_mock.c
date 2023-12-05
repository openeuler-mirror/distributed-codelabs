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

#include "huks_core_hal.h"
#include "huks_core_hal_mock.h"

#include "hks_log.h"

#include "securec.h"

static bool g_isEnableCreateOrDestroy = true;
static bool g_isEnableHdi = true;

static struct HuksHdi g_mockHid = { 0 };

int32_t HksCreateHuksHdiDevice(struct HuksHdi **halDevice)
{
    if (g_isEnableCreateOrDestroy) {
        if (g_isEnableHdi) {
            (void)memset_s(&g_mockHid, sizeof(struct HuksHdi), 1, sizeof(struct HuksHdi));
        } else {
            (void)memset_s(&g_mockHid, sizeof(struct HuksHdi), 0, sizeof(struct HuksHdi));
        }
        *halDevice = &g_mockHid;
        return HKS_SUCCESS;
    }
    return HKS_FAILURE;
}

int32_t HksDestroyHuksHdiDevice(struct HuksHdi **halDevice)
{
    if (g_isEnableCreateOrDestroy) {
        return HKS_SUCCESS;
    }
    return HKS_FAILURE;
}

void HksEnableCreateOrDestroy(bool isEnable)
{
    g_isEnableCreateOrDestroy = isEnable;
}

void HksEnableSetHid(bool isEnable)
{
    g_isEnableHdi = isEnable;
}
