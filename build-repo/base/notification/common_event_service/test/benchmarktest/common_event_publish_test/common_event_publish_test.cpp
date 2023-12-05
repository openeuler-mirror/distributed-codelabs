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

#include "common_event_constant.h"
#include "common_event_listener.h"
#include "common_event_manager.h"
#include "singleton.h"

using namespace OHOS::EventFwk;

namespace {
class BenchmarkCommonEventPublish : public benchmark::Fixture {
public:
    BenchmarkCommonEventPublish()
    {
        Iterations(iterations);
        Repetitions(repetitions);
        ReportAggregatesOnly();
    }

    ~BenchmarkCommonEventPublish() override = default;

    void SetUp(const ::benchmark::State &state) override
    {}

    void TearDown(const ::benchmark::State &state) override
    {}

protected:
    const int32_t repetitions = 1;
    const int32_t iterations = 100;
};

class CommonEventSubscriberBenchmark : public CommonEventSubscriber {
public:
    explicit CommonEventSubscriberBenchmark(const CommonEventSubscribeInfo &subscribeInfo) : CommonEventSubscriber(
        subscribeInfo) {};
    virtual ~CommonEventSubscriberBenchmark() {};
    virtual void OnReceiveEvent(const CommonEventData &data) {};
};

/**
* @tc.name: SubscribeCommonEventTestCase
* @tc.desc: SubscribeCommonEvent
* @tc.type: FUNC
* @tc.require:
*/
BENCHMARK_F(BenchmarkCommonEventPublish, SubscribeCommonEventTestCase)(benchmark::State &state)
{
    std::string eventName = "SUBSCRIBER_EVENT_BENCHMARK";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventSubscriberBenchmark>(subscribeInfo);
    while (state.KeepRunning()) {
        CommonEventManager::SubscribeCommonEvent(subscriberPtr);
    }
}

/**
* @tc.name: UnsubscribeCommonEventTestCase
* @tc.desc: UnsubscribeCommonEvent
* @tc.type: FUNC
* @tc.require:
*/
BENCHMARK_F(BenchmarkCommonEventPublish, UnsubscribeCommonEventTestCase)(benchmark::State &state)
{
    std::string eventName = "UNSUBSCRIBER_EVENT_BENCHMARK";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(1000);
    auto subscriberPtr = std::make_shared<CommonEventSubscriberBenchmark>(subscribeInfo);

    while (state.KeepRunning()) {
        if (OHOS::EventFwk::CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
            OHOS::EventFwk::CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
        }
    }
}

/**
* @tc.name: PublishCommonEventTestCase
* @tc.desc: PublishCommonEvent
* @tc.type: FUNC
* @tc.require:
*/
BENCHMARK_F(BenchmarkCommonEventPublish, PublishCommonEventTestCase)(benchmark::State &state)
{
    std::string eventAction = "PUBLISH_COMMON_EVENT_BENCHMARK";
    std::string entity = "ADDENTITY";
    bool stickty = false;
    Want testWant;
    testWant.SetAction(eventAction);
    testWant.AddEntity(entity);
    CommonEventData commonEventData(testWant);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(stickty);

    while (state.KeepRunning()) {
        OHOS::EventFwk::CommonEventManager::PublishCommonEvent(commonEventData, publishInfo);
    }
}
}

// Run the benchmark
BENCHMARK_MAIN();