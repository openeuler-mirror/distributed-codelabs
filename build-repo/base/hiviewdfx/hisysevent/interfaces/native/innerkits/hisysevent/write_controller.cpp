/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "write_controller.h"

#include <__mutex_base>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <ostream>
#include <sys/time.h>
#include <sstream>
#include <string>

#include "hilog/log.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, 0xD002D08, "WRITE_CONTROLLER" };
constexpr char STR_CONCAT = '_';
}

bool WriteController::CheckLimitWritingEvent(const ControlParam& param, const char* domain, const char* eventName,
    const char* func, int64_t line)
{
    HiLog::Debug(LABEL, "{ .period = [%{public}zu, .threshold = %{public}zu}", param.period, param.threshold);
    std::lock_guard<std::mutex> lock(lmtMutex);
    std::string key = ConcatenateInfoAsKey(eventName, func, line);
    EventLimitStat stat = lruCache.Get(key);
    if (!stat.IsValid()) {
        stat.count = 1;
        lruCache.Put(key, stat);
        return false;
    }
    timeval cur;
    gettimeofday(&cur, nullptr);
    if (stat.begin.tv_sec + static_cast<time_t>(param.period) < cur.tv_sec) {
        stat.count = 1;
        stat.begin = cur;
        lruCache.Put(key, stat);
        return false;
    }
    stat.count++;
    if (stat.count <= param.threshold) {
        lruCache.Put(key, stat);
        return false;
    }
    lruCache.Put(key, stat);
    HiLog::Error(LABEL, "[%{public}lld, %{public}lld] discard %{public}zu event(s) "
        "with domain %{public}s and name %{public}s which wrote in function %{public}s.",
        static_cast<long long>(stat.begin.tv_sec), static_cast<long long>(cur.tv_sec),
        stat.count - param.threshold, domain, eventName, func);
    return true;
}

std::string WriteController::ConcatenateInfoAsKey(const char* eventName, const char* func, int64_t line) const
{
    std::stringstream ss;
    ss << eventName << STR_CONCAT << std::string(func) << STR_CONCAT << line;
    std::string concaStr = ss.str();
    ss.clear();
    return concaStr;
}
} // HiviewDFX
} // OHOS