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

#include "hisysevent_record_c.h"

#include "hisysevent_record.h"
#include "string_util.h"

namespace {
using HiSysEventRecordCls = OHOS::HiviewDFX::HiSysEventRecord;
constexpr int ERR_NULL = -1;

template <typename T>
int GetParamValue(const HiSysEventRecord& record, const char* name, T& value)
{
    if (record.jsonStr == nullptr || name == nullptr) {
        return ERR_NULL;
    }
    HiSysEventRecordCls recordObj(record.jsonStr);
    return recordObj.GetParamValue(name, value);
}

int GetParamValue(const HiSysEventRecord& record, const char* name, char** value)
{
    if (record.jsonStr == nullptr || name == nullptr) {
        return ERR_NULL;
    }
    HiSysEventRecordCls recordObj(record.jsonStr);
    std::string str;
    if (auto res = recordObj.GetParamValue(name, str); res != 0) {
        return res;
    }
    return OHOS::HiviewDFX::StringUtil::ConvertCString(str, value);
}

template <typename T>
int GetParamValues(const HiSysEventRecord& record, const char* name, T** value, size_t& len)
{
    if (record.jsonStr == nullptr || name == nullptr) {
        return ERR_NULL;
    }
    HiSysEventRecordCls recordObj(record.jsonStr);
    std::vector<T> dataVec;
    if (auto res = recordObj.GetParamValue(name, dataVec); res != 0) {
        return res;
    }
    if (dataVec.empty()) {
        return 0;
    }
    len = dataVec.size();
    T* data = new(std::nothrow) T[len];
    for (size_t i = 0; i < len; i++) {
        data[i] = dataVec[i];
    }
    *value = data;
    return 0;
}

int GetParamValues(const HiSysEventRecord& record, const char* name, char*** value, size_t& len)
{
    if (record.jsonStr == nullptr || name == nullptr) {
        return ERR_NULL;
    }
    HiSysEventRecordCls recordObj(record.jsonStr);
    std::vector<std::string> dataVec;
    if (auto res = recordObj.GetParamValue(name, dataVec); res != 0) {
        return res;
    }
    if (dataVec.empty()) {
        return 0;
    }
    return OHOS::HiviewDFX::StringUtil::ConvertCStringVec(dataVec, value, len);
}

int GetParamNames(const HiSysEventRecord& record, char*** names, size_t& len)
{
    if (record.jsonStr == nullptr) {
        return ERR_NULL;
    }
    HiSysEventRecordCls recordObj(record.jsonStr);
    std::vector<std::string> dataVec;
    recordObj.GetParamNames(dataVec);
    if (dataVec.empty()) {
        return 0;
    }
    return OHOS::HiviewDFX::StringUtil::ConvertCStringVec(dataVec, names, len);
}

int GetParamInt64Value(const HiSysEventRecord& record, const char* name, int64_t& value)
{
    return GetParamValue<int64_t>(record, name, value);
}

int GetParamUint64Value(const HiSysEventRecord& record, const char* name, uint64_t& value)
{
    return GetParamValue<uint64_t>(record, name, value);
}

int GetParamDoubleValue(const HiSysEventRecord& record, const char* name, double& value)
{
    return GetParamValue<double>(record, name, value);
}

int GetParamStringValue(const HiSysEventRecord& record, const char* name, char** value)
{
    return GetParamValue(record, name, value);
}

int GetParamInt64Values(const HiSysEventRecord& record, const char* name, int64_t** value, size_t& len)
{
    return GetParamValues<int64_t>(record, name, value, len);
}

int GetParamUint64Values(const HiSysEventRecord& record, const char* name, uint64_t** value, size_t& len)
{
    return GetParamValues<uint64_t>(record, name, value, len);
}

int GetParamDoubleValues(const HiSysEventRecord& record, const char* name, double** value, size_t& len)
{
    return GetParamValues<double>(record, name, value, len);
}

int GetParamStringValues(const HiSysEventRecord& record, const char* name, char*** value, size_t& len)
{
    return GetParamValues(record, name, value, len);
}
}

#ifdef __cplusplus
extern "C" {
#endif

void OH_HiSysEvent_GetParamNames(const HiSysEventRecord& record, char*** names, size_t& len)
{
    GetParamNames(record, names, len);
}

int OH_HiSysEvent_GetParamInt64Value(const HiSysEventRecord& record, const char* name, int64_t& value)
{
    return GetParamInt64Value(record, name, value);
}

int OH_HiSysEvent_GetParamUint64Value(const HiSysEventRecord& record, const char* name, uint64_t& value)
{
    return GetParamUint64Value(record, name, value);
}

int OH_HiSysEvent_GetParamDoubleValue(const HiSysEventRecord& record, const char* name, double& value)
{
    return GetParamDoubleValue(record, name, value);
}

int OH_HiSysEvent_GetParamStringValue(const HiSysEventRecord& record, const char* name, char** value)
{
    return GetParamStringValue(record, name, value);
}

int OH_HiSysEvent_GetParamInt64Values(const HiSysEventRecord& record, const char* name, int64_t** value, size_t& len)
{
    return GetParamInt64Values(record, name, value, len);
}

int OH_HiSysEvent_GetParamUint64Values(const HiSysEventRecord& record, const char* name, uint64_t** value, size_t& len)
{
    return GetParamUint64Values(record, name, value, len);
}

int OH_HiSysEvent_GetParamDoubleValues(const HiSysEventRecord& record, const char* name, double** value, size_t& len)
{
    return GetParamDoubleValues(record, name, value, len);
}

int OH_HiSysEvent_GetParamStringValues(const HiSysEventRecord& record, const char* name, char*** value, size_t& len)
{
    return GetParamStringValues(record, name, value, len);
}
#ifdef __cplusplus
}
#endif
