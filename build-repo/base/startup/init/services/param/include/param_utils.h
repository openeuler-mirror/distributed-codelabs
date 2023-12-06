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

#ifndef BASE_STARTUP_PARAM_UTILS_H
#define BASE_STARTUP_PARAM_UTILS_H
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif
#define PARAM_ALIGN(len) (((len) + 0x03) & (~0x03))

#define PARAM_PERSIST_PREFIX "persist."

#define PIPE_PATH "/dev/unix/socket"
#define CLIENT_PIPE_NAME "/dev/unix/socket/paramservice"
#define PIPE_NAME "/dev/unix/socket/paramservice"
#define SYSTEM_PARAM_PATH "/system/etc/param"
#define USER_PARAM_PATH "/data/service/el1/public/param_service/"
#define USER_PARAM_FILE USER_PARAM_PATH "/user.para"
#define CONST_PREFIX "const."

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif