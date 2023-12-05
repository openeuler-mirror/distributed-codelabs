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

#include "hisyseventmanager_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <vector>

#include "hisysevent_manager.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
class TestQueryCallback : public HiSysEventQueryCallback {
public:
    TestQueryCallback() {}
    virtual ~TestQueryCallback() {}

    void OnQuery(std::shared_ptr<std::vector<HiSysEventRecord>> sysEvents) override
    {}

    void OnComplete(int32_t reason, int32_t total) override
    {}
};

void HiSysEventRecordTest(const HiSysEventRecord& record, const std::string& data)
{
    (void)record.AsJson();
    (void)record.GetDomain();
    (void)record.GetEventName();
    (void)record.GetLevel();
    (void)record.GetTag();
    (void)record.GetTimeZone();
    (void)record.GetEventType();
    (void)record.GetTraceFlag();
    (void)record.GetPid();
    (void)record.GetTid();
    (void)record.GetUid();
    (void)record.GetPspanId();
    (void)record.GetSpanId();
    (void)record.GetTime();
    (void)record.GetTraceId();
    std::vector<std::string> params;
    record.GetParamNames(params);

    int64_t intValue = 0;
    (void)record.GetParamValue(data, intValue);
    uint64_t uintValue = 0;
    (void)record.GetParamValue(data, uintValue);
    double dValue = 0;
    (void)record.GetParamValue(data, dValue);
    std::string strValue;
    (void)record.GetParamValue(data, strValue);
    std::vector<int64_t> intValues;
    (void)record.GetParamValue(data, intValues);
    std::vector<uint64_t> uintValues;
    (void)record.GetParamValue(data, uintValues);
    std::vector<double> dValues;
    (void)record.GetParamValue(data, dValues);
    std::vector<std::string> strValues;
    (void)record.GetParamValue(data, strValues);
}

void HiSysEventQueryTest(const std::string& strData, int64_t intData)
{
    auto callback = std::make_shared<TestQueryCallback>();
    QueryArg arg(0, intData, intData);
    std::vector<QueryRule> rules = { QueryRule(strData, {strData}) };
    (void)HiSysEventManager::Query(arg, rules, callback);
}
}

void HiSysEventRecordFuzzTest(const uint8_t* data, size_t size)
{
    std::string strData((const char*) data, size);
    const std::string jsonStr = R"~({"domain_":"test_domain","name_":"test_name"})~";
    HiSysEventRecord record(jsonStr);
    HiSysEventRecordTest(record, strData);
}

void HiSysEventQueryFuzzTest(const uint8_t* data, size_t size)
{
    std::string strData((const char*) data, size);
    int64_t intData = static_cast<int64_t>(*data);
    HiSysEventQueryTest(strData, intData);
}
} // namespace HiviewDFX
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::HiviewDFX::HiSysEventRecordFuzzTest(data, size);
    OHOS::HiviewDFX::HiSysEventQueryFuzzTest(data, size);
    return 0;
}

