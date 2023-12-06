/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef HIVIEWDFX_NAPI_HISYSEVENT_ADAPTER_H
#define HIVIEWDFX_NAPI_HISYSEVENT_ADAPTER_H

#include <string>
#include <unordered_map>
#include <vector>

#include "hisysevent.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace HiviewDFX {
using HiSysEventInfo = struct HiSysEventInfo {
    std::string domain;
    std::string name;
    HiSysEvent::EventType eventType;
    std::unordered_map<std::string, bool> boolParams;
    std::unordered_map<std::string, std::vector<bool>> boolArrayParams;
    std::unordered_map<std::string, double> doubleParams;
    std::unordered_map<std::string, std::vector<double>> doubleArrayParams;
    std::unordered_map<std::string, std::string> stringParams;
    std::unordered_map<std::string, std::vector<std::string>> stringArrayParams;
};

using HiSysEventAsyncContext = struct HiSysEventAsyncContext {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback;
    HiSysEventInfo eventInfo;
    int eventWroteResult;
};

class NapiHiSysEventAdapter {
public:
    static void Write(const napi_env env, HiSysEventAsyncContext* eventAsyncContext);

private:
    static void InnerWrite(HiSysEvent::EventBase& eventBase, const HiSysEventInfo& eventInfo);
    static int Write(const HiSysEventInfo& eventInfo);

    template<typename T>
    static void AppendData(HiSysEvent::EventBase& eventBase, std::unordered_map<std::string, T> tMap)
    {
        for (auto iter = tMap.cbegin(); iter != tMap.cend(); iter++) {
            auto key = iter->first;
            auto value = iter->second;
            HiSysEvent::AppendData<T>(eventBase, key, value);
        }
    }

    template<typename T>
    static void AppendArrayData(HiSysEvent::EventBase& eventBase,
        std::unordered_map<std::string, std::vector<T>> tMap)
    {
        for (auto iter = tMap.cbegin(); iter != tMap.cend(); iter++) {
            auto key = iter->first;
            auto value = iter->second;
            HiSysEvent::AppendArrayData<T>(eventBase, key, value);
        }
    }
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // HIVIEWDFX_NAPI_HISYSEVENT_ADAPTER_H