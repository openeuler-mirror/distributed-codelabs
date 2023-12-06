/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef UTILS_LOG_H
#define UTILS_LOG_H

#include <string>

#include "hilog/log_c.h"
#include "hilog/log_cpp.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
#ifndef LOG_DOMAIN
#define LOG_DOMAIN 0xD001600
#endif

#ifndef LOG_TAG
#define LOG_TAG "distributedfile"
#endif

static constexpr HiviewDFX::HiLogLabel APP_LABEL = {LOG_CORE, LOG_DOMAIN, LOG_TAG};

std::string GetFileNameFromFullPath(const char *str);
#define PRINT_LOG(Level, fmt, ...)                                                 \
    HiviewDFX::HiLog::Level(APP_LABEL, "[%{public}s:%{public}d->%{public}s] " fmt, \
                            GetFileNameFromFullPath(__FILE__).c_str(), __LINE__, __FUNCTION__, ##__VA_ARGS__)

#define LOGD(fmt, ...) PRINT_LOG(Debug, fmt, ##__VA_ARGS__)
#define LOGI(fmt, ...) PRINT_LOG(Info, fmt, ##__VA_ARGS__)
#define LOGW(fmt, ...) PRINT_LOG(Warn, fmt, ##__VA_ARGS__)
#define LOGE(fmt, ...) PRINT_LOG(Error, fmt, ##__VA_ARGS__)
#define LOGF(fmt, ...) PRINT_LOG(Fatal, fmt, ##__VA_ARGS__)
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // UTILS_LOG_H