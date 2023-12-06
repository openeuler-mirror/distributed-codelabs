/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef BASE_STARTUP_INIT_PARAM_H
#define BASE_STARTUP_INIT_PARAM_H
#include <stdint.h>
#include <stdio.h>
#ifdef PARAM_SUPPORT_TRIGGER
#include "cJSON.h"
#endif
#include "sys_param.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define DEFAULT_PARAM_WAIT_TIMEOUT 30 // 30s
#define DEFAULT_PARAM_SET_TIMEOUT 10 // 10s

#ifndef PARAM_NAME_LEN_MAX
#define PARAM_CONST_VALUE_LEN_MAX 4096
#define PARAM_VALUE_LEN_MAX  96
#define PARAM_NAME_LEN_MAX  96
#endif

typedef enum {
    PARAM_CODE_ERROR = -1,
    PARAM_CODE_SUCCESS = 0,
    PARAM_CODE_INVALID_PARAM = 100,
    PARAM_CODE_INVALID_NAME,
    PARAM_CODE_INVALID_VALUE,
    PARAM_CODE_REACHED_MAX,
    PARAM_CODE_NOT_SUPPORT,
    PARAM_CODE_TIMEOUT,
    PARAM_CODE_NOT_FOUND,
    PARAM_CODE_READ_ONLY,
    PARAM_CODE_FAIL_CONNECT,
    PARAM_CODE_NODE_EXIST, // 9
    PARAM_CODE_INVALID_SOCKET,
    DAC_RESULT_INVALID_PARAM = 1000,
    DAC_RESULT_FORBIDED,
    PARAM_CODE_MAX
} PARAM_CODE;

/**
 * 对外接口
 * 设置参数，主要用于其他进程使用，通过管道修改参数。
 *
 */
int SystemSetParameter(const char *name, const char *value);

/**
 * 对外接口
 * 查询参数，主要用于其他进程使用，需要给定足够的内存保存参数。
 * 如果 value == null，获取value的长度
 * 否则value的大小认为是len
 *
 */
#define SystemGetParameter SystemReadParam

/**
 * 外部接口
 * 等待某个参数值被修改，阻塞直到参数值被修改或超时
 *
 */
int SystemWaitParameter(const char *name, const char *value, int32_t timeout);

typedef void (*ParameterChangePtr)(const char *key, const char *value, void *context);
int SystemWatchParameter(const char *keyprefix, ParameterChangePtr change, void *context);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif