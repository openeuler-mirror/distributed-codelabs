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

#include "utils_timer.h"

#include <functional>
#include <new>

#include "nocopyable.h"
#include "singleton.h"
#include "timer.h"

#ifdef __cplusplus
extern "C" {
#endif

namespace {
using namespace OHOS;
class UtilsTimer final : public OHOS::Utils::Timer, public DelayedRefSingleton<UtilsTimer> {
public:
    UtilsTimer();
    ~UtilsTimer() override;
};

UtilsTimer::~UtilsTimer()
{
    this->Shutdown();
}

UtilsTimer::UtilsTimer() : Timer("timer_process")
{
    this->Setup();
}
} // namespace

void DoTimerProcess(TimerProc callback, const void *context)
{
    if (callback != nullptr) {
        callback(context);
    }
}

TimerHandle StartPeriodicTimerTask(uint32_t interval, TimerProc callback, const void *context)
{
    UtilsTimer &st = DelayedRefSingleton<UtilsTimer>::GetInstance();
    uint32_t timerId = st.Register(std::bind(&DoTimerProcess, callback, context), interval, false);
    return static_cast<TimerHandle>(timerId);
}

TimerHandle StartOnceTimerTask(uint32_t interval, TimerProc callback, const void *context)
{
    UtilsTimer &st = DelayedRefSingleton<UtilsTimer>::GetInstance();
    uint32_t timerId = st.Register(std::bind(&DoTimerProcess, callback, context), interval, true);
    return static_cast<TimerHandle>(timerId);
}

void StopTimerTask(TimerHandle handle)
{
    UtilsTimer &st = DelayedRefSingleton<UtilsTimer>::GetInstance();
    st.Unregister(static_cast<uint32_t>(handle));
}
#ifdef __cplusplus
}
#endif
