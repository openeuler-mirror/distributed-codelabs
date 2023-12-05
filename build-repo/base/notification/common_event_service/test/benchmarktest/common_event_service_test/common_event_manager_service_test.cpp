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

#include <benchmark/benchmark.h>
#include <string>
#include <vector>

#define private public
#include "common_event_constant.h"
#include "common_event_listener.h"
#include "common_event_manager_service.h"
#include "mock_bundle_manager.h"
#include "singleton.h"

using namespace OHOS;
using namespace OHOS::EventFwk;

namespace {
class BenchmarkCommonEventManagerService : public benchmark::Fixture {
public:
    BenchmarkCommonEventManagerService()
    {
        commonEventManagerService_ = DelayedSingleton<CommonEventManagerService>::GetInstance().get();
        commonEventManagerService_->Init();
        Iterations(iterations);
        Repetitions(repetitions);
        ReportAggregatesOnly();
    }

    ~BenchmarkCommonEventManagerService() override = default;

    void SetUp(const ::benchmark::State &state) override
    {}
    void TearDown(const ::benchmark::State &state) override
    {}
 
protected:
    const int32_t repetitions = 3;
    const int32_t iterations = 100;
    sptr<CommonEventManagerService> commonEventManagerService_;
};

class CommonEventSubscriberBenchmark : public CommonEventSubscriber {
public:
    explicit CommonEventSubscriberBenchmark(const CommonEventSubscribeInfo &subscribeInfo) : CommonEventSubscriber(
        subscribeInfo) {};
    virtual ~CommonEventSubscriberBenchmark() {};
    virtual void OnReceiveEvent(const CommonEventData &data) {};
};
/**
 * @tc.name: CommonEventSubscribeTestCase001
 * @tc.desc: SubscribeCommonEvent: event is normal
 * @tc.type: FUNC
 * @tc.require:
 */
BENCHMARK_F(BenchmarkCommonEventManagerService, CommonEventSubscribeTestCase001)(benchmark::State &state)
{
    std::string eventName = "SUBSCRIBEEVENT_BENCHMARK";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventSubscriberBenchmark>(subscribeInfo);
    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);

    while (state.KeepRunning()) {
        bool result = commonEventManagerService_->SubscribeCommonEvent(
        subscribeInfo, commonEventListener);
        if (!result) {
            state.SkipWithError("SubscribeCommonEvent failed.");
        }
    }
}

/**
 * @tc.name: CommonEventSubscribeTestCase002
 * @tc.desc: SubscribeCommonEvent: event is number
 * @tc.type: FUNC
 * @tc.require:
 */
BENCHMARK_F(BenchmarkCommonEventManagerService, CommonEventSubscribeTestCase002)(benchmark::State &state)
{
    std::string eventName = "1";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventSubscriberBenchmark>(subscribeInfo);
    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);

    while (state.KeepRunning()) {
        bool result = commonEventManagerService_->SubscribeCommonEvent(
        subscribeInfo, commonEventListener);
        if (!result) {
            state.SkipWithError("SubscribeCommonEvent failed.");
        }
    }
}

/**
 * @tc.name: CommonEventSubscribeTestCase003
 * @tc.desc: SubscribeCommonEvent: event is empty
 * @tc.type: FUNC
 * @tc.require:
 */
BENCHMARK_F(BenchmarkCommonEventManagerService, CommonEventSubscribeTestCase003)(benchmark::State &state)
{
    std::string eventName = "";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventSubscriberBenchmark>(subscribeInfo);
    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);

    while (state.KeepRunning()) {
        bool result = commonEventManagerService_->SubscribeCommonEvent(
        subscribeInfo, commonEventListener);
        if (!result) {
            state.SkipWithError("SubscribeCommonEvent failed.");
        }
    }
}

/**
 * @tc.name: CommonEventUnsubscribeTestCase
 * @tc.desc: UnsubscribeCommonEvent
 * @tc.type: FUNC
 * @tc.require:
 */
BENCHMARK_F(BenchmarkCommonEventManagerService, CommonEventUnsubscribeTestCase)(benchmark::State &state)
{
    std::string eventName = "SUBSCRIBEEVENT_BENCHMARK";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventSubscriberBenchmark>(subscribeInfo);
    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);

    while (state.KeepRunning()) {
        bool result = commonEventManagerService_->UnsubscribeCommonEvent(
            commonEventListener);
        if (!result) {
            state.SkipWithError("UnsubscribeCommonEvent failed.");
        }
    }
}

/**
 * @tc.name: CommonEventPublishTestCase
 * @tc.desc: PublishCommonEvent
 * @tc.type: FUNC
 * @tc.require:
 */
BENCHMARK_F(BenchmarkCommonEventManagerService, CommonEventPublishTestCase)(benchmark::State &state)
{
    std::string eventName = "SUBSCRIBEEVENT_BENCHMARK";
    std::string eventAction = "SUBSCRIBEEVENT_BENCHMARK";
    bool stickty = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want testWant;
    testWant.SetAction(eventAction);
    CommonEventData commonEventData(testWant);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(stickty);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventSubscriberBenchmark>(subscribeInfo);
    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);

    while (state.KeepRunning()) {
        bool result = commonEventManagerService_->PublishCommonEvent(
            commonEventData, publishInfo, commonEventListener, UNDEFINED_USER);
        if (!result) {
            state.SkipWithError("PublishCommonEvent failed.");
        }
    }
}

/**
 * @tc.name: CommonEventDumpStateTestCase
 * @tc.desc: DumpState
 * @tc.type: FUNC
 * @tc.require:
 */
BENCHMARK_F(BenchmarkCommonEventManagerService, CommonEventDumpStateTestCase)(benchmark::State &state)
{
    std::string eventName = "SUBSCRIBEEVENT_BENCHMARK";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventSubscriberBenchmark>(subscribeInfo);
    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);

    bool result = commonEventManagerService_->SubscribeCommonEvent(
        subscribeInfo, commonEventListener);
    if (!result) {
        state.SkipWithError("DumpState subscribe common event failed.");
    }

    while (state.KeepRunning()) {
        std::vector<std::string> stateTest;
        if (commonEventManagerService_->DumpState(0, "", ALL_USER, stateTest)) {
            if (stateTest.size() < 1) {
                state.SkipWithError("DumpState failed.");
            }
        }
    }
}
}

// Run the benchmark
BENCHMARK_MAIN();