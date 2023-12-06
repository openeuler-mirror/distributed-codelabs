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

#ifndef BEGET_EXT_API_H
#define BEGET_EXT_API_H
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include "hilog/log.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define BASE_DOMAIN 0xD002C00
#undef LOG_DOMAIN
#undef LOG_TAG
#define LOG_DOMAIN (BASE_DOMAIN + 1)
#define LOG_TAG "BEGET"

#define BEGET_PUBLIC_API __attribute__((visibility ("default")))
#define BEGET_LOCAL_API __attribute__((visibility("hidden")))

#define BEGET_LOGI(fmt, ...) HILOG_INFO(LOG_CORE, fmt, ##__VA_ARGS__)
#define BEGET_LOGE(fmt, ...) HILOG_ERROR(LOG_CORE, fmt, ##__VA_ARGS__)
#define BEGET_LOGV(fmt, ...) HILOG_DEBUG(LOG_CORE, fmt, ##__VA_ARGS__)
#define BEGET_LOGW(fmt, ...) HILOG_WARN(LOG_CORE, fmt, ##__VA_ARGS__)

#define BEGET_ERROR_CHECK(ret, statement, format, ...) \
    if (!(ret)) {                                     \
        BEGET_LOGE(format, ##__VA_ARGS__);             \
        statement;                                    \
    }                                                 \

#define BEGET_INFO_CHECK(ret, statement, format, ...) \
    if (!(ret)) {                                    \
        BEGET_LOGI(format, ##__VA_ARGS__);            \
        statement;                                   \
    }                                          \

#define BEGET_WARNING_CHECK(ret, statement, format, ...) \
    if (!(ret)) {                                     \
        BEGET_LOGW(format, ##__VA_ARGS__);             \
        statement;                                    \
    }                                                 \

#define BEGET_CHECK(ret, statement) \
    if (!(ret)) {                  \
        statement;                 \
    }                         \

#define BEGET_CHECK_RETURN_VALUE(ret, result) \
    do {                                \
        if (!(ret)) {                            \
            return result;                       \
        }                                  \
    } while (0)

#define BEGET_CHECK_ONLY_RETURN(ret) \
    do {                                \
        if (!(ret)) {                   \
            return;                     \
        } \
    } while (0)

#define BEGET_CHECK_ONLY_ELOG(ret, format, ...) \
    do {                                       \
        if (!(ret)) {                          \
            BEGET_LOGE(format, ##__VA_ARGS__);  \
        } \
    } while (0)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif