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
 
#ifndef HISYSEVENT_RECORD_H
#define HISYSEVENT_RECORD_H

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "hisysevent.h"

namespace OHOS {
namespace HiviewDFX {
constexpr int VALUE_PARSED_SUCCEED = 0;
constexpr int ERR_INIT_FAILED = -1;
constexpr int ERR_KEY_NOT_EXIST = -2;
constexpr int ERR_TYPE_NOT_MATCH = -3;
class HiSysEventValue;
class HiSysEventRecord {
public:
    HiSysEventRecord(std::string jsonStr)
    {
        ParseJsonStr(jsonStr);
    }
    ~HiSysEventRecord() {}

public:
    std::string AsJson() const;
    std::string GetDomain() const;
    std::string GetEventName() const;
    std::string GetLevel() const;
    std::string GetTag() const;
    std::string GetTimeZone() const;
    HiSysEvent::EventType GetEventType() const;
    int GetTraceFlag() const;
    int64_t GetPid() const;
    int64_t GetTid() const;
    int64_t GetUid() const;
    uint64_t GetPspanId() const;
    uint64_t GetSpanId() const;
    uint64_t GetTime() const;
    uint64_t GetTraceId() const;
    void GetParamNames(std::vector<std::string>& params) const;

public:
    int GetParamValue(const std::string& param, int64_t& value) const;
    int GetParamValue(const std::string& param, uint64_t& value) const;
    int GetParamValue(const std::string& param, double& value) const;
    int GetParamValue(const std::string& param, std::string& value) const;
    int GetParamValue(const std::string& param, std::vector<int64_t>& value) const;
    int GetParamValue(const std::string& param, std::vector<uint64_t>& value) const;
    int GetParamValue(const std::string& param, std::vector<double>& value) const;
    int GetParamValue(const std::string& param, std::vector<std::string>& value) const;

private:
    int GetIntValueByKey(const std::string key) const;
    int64_t GetInt64ValueByKey(const std::string key) const;
    uint64_t GetUInt64ValueByKey(const std::string key) const;
    std::string GetStringValueByKey(const std::string key) const;

private:
    using JsonValue = std::shared_ptr<HiSysEventValue>;
    using TypeFilter = std::function<bool(JsonValue)>;
    using ValueAssigner = std::function<void(JsonValue)>;
    bool IsInt64ValueType(const JsonValue val) const;
    bool IsUInt64ValueType(const JsonValue val) const;
    bool IsDoubleValueType(const JsonValue val) const;
    bool IsStringValueType(const JsonValue val) const;
    bool IsArray(const JsonValue val, const TypeFilter filterFunc) const;
    void ParseJsonStr(const std::string jsonStr);
    int GetParamValue(const std::string& param, const TypeFilter filterFunc, const ValueAssigner assignFunc) const;

private:
    std::string jsonStr_;
    JsonValue jsonVal_;
};
} // namespace HiviewDFX
} // OHOS

#endif // HISYSEVENT_RECORD_H