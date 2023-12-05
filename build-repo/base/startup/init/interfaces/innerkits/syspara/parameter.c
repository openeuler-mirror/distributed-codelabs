/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "parameter.h"

#include <stdint.h>
#include <stdlib.h>

#include "param_comm.h"
#include "init_param.h"
#include "init_utils.h"
#include "sysparam_errno.h"
#include "securec.h"
#include "beget_ext.h"

#define SN_LEN 65
#define SN_FILE "/etc/SN"

int WaitParameter(const char *key, const char *value, int timeout)
{
    BEGET_CHECK(!(key == NULL || value == NULL), return EC_INVALID);
    int ret = SystemWaitParameter(key, value, timeout);
    return GetSystemError(ret);
}

int GetParameter(const char *key, const char *def, char *value, uint32_t len)
{
    if ((key == NULL) || (value == NULL)) {
        return EC_INVALID;
    }
    int ret = GetParameter_(key, def, value, len);
    return (ret != 0) ? ret : strlen(value);
}

int SetParameter(const char *key, const char *value)
{
    if ((key == NULL) || (value == NULL)) {
        return EC_INVALID;
    }
    int ret = SystemSetParameter(key, value);
    return GetSystemError(ret);
}

const char *GetDeviceType(void)
{
    // static const char *productType = NULL;
    // const char *deviceType = GetProperty("const.product.devicetype", &productType);
    // if (deviceType != NULL) {
    //     return deviceType;
    // }
    // return GetProperty("const.build.characteristics", &productType);
    return "UNKNOWN";
}

// 暂时使用/etc/SN里面的数字代表udid
int GetDevUdid(char *udid, int size)
{
    FILE *fp;
    char *realPath = NULL;
    char sn[SN_LEN] = {0};
    // char out[UDID_LEN] = {0};
    int ret;

    realPath = realpath(SN_FILE, NULL);
    if (realPath == NULL) {
        printf("realpath fail.\n");
        goto err_realpath;
    }

    fp = fopen(realPath, "r");
    if (fp == NULL) {
        printf("open SN fail.\n");
        goto err_fopen;
    }

    ret = fscanf_s(fp, "%s", sn, SN_LEN);
    if (ret < 1) {
        printf("get sn fail.\n");
        goto err_out;
    }

    if (strcpy_s(udid, size, sn) != EOK) {
        return 1;
    }

    fclose(fp);
    return 0;
err_out:
    fclose(fp);
err_fopen:
    free(realPath);
err_realpath:
    return -1;
}

// int GetDevUdid(char *udid, int size)
// {
//     return GetDevUdid_(udid, size);
// }

int32_t GetIntParameter(const char *key, int32_t def)
{
    char value[MAX_INT_LEN] = {0};
    uint32_t size = sizeof(value);
    int ret = SystemGetParameter(key, value, &size);
    if (ret != 0) {
        return def;
    }
    long long int result = 0;
    if (StringToLL(value, &result) != 0) {
        return def;
    }
    if (result <= INT32_MIN || result >= INT32_MAX) {
        return def;
    }
    return (int32_t)result;
}

uint32_t GetUintParameter(const char *key, uint32_t def)
{
    char value[MAX_INT_LEN] = {0};
    uint32_t size = sizeof(value);
    int ret = SystemGetParameter(key, value, &size);
    if (ret != 0) {
        return def;
    }
    unsigned long long int result = 0;
    if (StringToULL(value, &result) != 0) {
        return def;
    }
    if (result >= UINT32_MAX) {
        return def;
    }
    return (uint32_t)result;
}
