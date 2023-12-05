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

#include "event_runner.h"
#include "eventrunner_fuzzer.h"
#include "securec.h"

namespace OHOS {
namespace {
    constexpr size_t U32_AT_SIZE = 4;
}

class DumperTest : public AppExecFwk::Dumper {
public:
    DumperTest() = default;
    virtual ~DumperTest()
    {};
    void Dump(const std::string &message) override
    {}
    std::string GetTag() override
    {
        return {};
    }
};

class LoggerTest : public AppExecFwk::Logger {
public:
    LoggerTest() = default;
    virtual ~LoggerTest()
    {};
    void Log(const std::string &line) override
    {}
};

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    DumperTest dumper;
    std::string stringData(data);
    std::shared_ptr<LoggerTest> logger = std::make_shared<LoggerTest>();
    auto runner = AppExecFwk::EventRunner::Create(true);
    runner->Dump(dumper);
    runner->DumpRunnerInfo(stringData);
    runner->SetLogger(logger);
    runner->GetCurrentEventQueue();
    runner->GetThreadId();
    runner->IsCurrentRunnerThread();
    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        return 0;
    }

    if (size < OHOS::U32_AT_SIZE) {
        return 0;
    }

    char* ch = (char *)malloc(size + 1);
    if (ch == nullptr) {
        return 0;
    }

    (void)memset_s(ch, size + 1, 0x00, size + 1);
    if (memcpy_s(ch, size, data, size) != EOK) {
        free(ch);
        ch = nullptr;
        return 0;
    }

    OHOS::DoSomethingInterestingWithMyAPI(ch, size);
    free(ch);
    ch = nullptr;
    return 0;
}
