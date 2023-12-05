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

#ifndef SEC_UTILS_LOG_H
#define SEC_UTILS_LOG_H

#include "hilog/log.h"

#ifndef __cplusplus
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "DSLM_SERVICE"

#ifdef LOG_DOMAIN
#undef LOG_DOMAIN
#endif
#define LOG_DOMAIN 0xD002F00

#define SECURITY_LOG_DEBUG(fmt, ...) HILOG_DEBUG(LOG_CORE, "[%{public}s]:" fmt, __func__, ##__VA_ARGS__)
#define SECURITY_LOG_INFO(fmt, ...) HILOG_INFO(LOG_CORE, "[%{public}s]:" fmt, __func__, ##__VA_ARGS__)
#define SECURITY_LOG_WARN(fmt, ...) HILOG_WARN(LOG_CORE, "[%{public}s]:" fmt, __func__, ##__VA_ARGS__)
#define SECURITY_LOG_ERROR(fmt, ...) HILOG_ERROR(LOG_CORE, "[%{public}s]:" fmt, __func__, ##__VA_ARGS__)
#define SECURITY_LOG_FATAL(fmt, ...) HILOG_FATAL(LOG_CORE, "[%{public}s]:" fmt, __func__, ##__VA_ARGS__)

#else // __cplusplus
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, 0xD002F00, "DSLM_SERVICE"};

#define SECURITY_LOG_DEBUG(fmt, ...) \
    OHOS::HiviewDFX::HiLog::Debug(LABEL, "[%{public}s]:" fmt, __func__, ##__VA_ARGS__)
#define SECURITY_LOG_INFO(fmt, ...) \
    OHOS::HiviewDFX::HiLog::Info(LABEL, "[%{public}s]:" fmt, __func__, ##__VA_ARGS__)
#define SECURITY_LOG_WARN(fmt, ...) \
    OHOS::HiviewDFX::HiLog::Warn(LABEL, "[%{public}s]:" fmt, __func__, ##__VA_ARGS__)
#define SECURITY_LOG_ERROR(fmt, ...) \
    OHOS::HiviewDFX::HiLog::Error(LABEL, "[%{public}s]:" fmt, __func__, ##__VA_ARGS__)
#define SECURITY_LOG_FATAL(fmt, ...) \
    OHOS::HiviewDFX::HiLog::Fatal(LABEL, "[%{public}s]:" fmt, __func__, ##__VA_ARGS__)

#endif // __cplusplus
#endif // SEC_UTILS_LOG_H
