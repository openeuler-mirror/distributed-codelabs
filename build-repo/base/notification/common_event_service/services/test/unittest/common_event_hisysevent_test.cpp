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

#include <gtest/gtest.h>
#include <numeric>
#include "event_report.h"

namespace OHOS {
namespace EventFwk {
using namespace testing::ext;

namespace {
const std::string TEST_EVENT_NAME = "usual.event.TEST_HISYSEVENT";
const std::string TEST_SUBSCRIBER_BUNDLE_NAME = "subscriber";
const std::string TEST_PUBLISHER_BUNDLE_NAME = "publisher";
constexpr int32_t TEST_USER_ID = 1000;
constexpr int32_t TEST_SUBSCRIBER_NUM = 256;
} // namespace

class CommonEventHisyseventTest : public testing::Test {
public:
    CommonEventHisyseventTest()
    {}
    ~CommonEventHisyseventTest()
    {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CommonEventHisyseventTest::SetUpTestCase(void)
{}

void CommonEventHisyseventTest::TearDownTestCase(void)
{}

void CommonEventHisyseventTest::SetUp(void)
{}

void CommonEventHisyseventTest::TearDown(void)
{}

/**
 * @tc.name: SendOrderedEventProcTimeoutSysEvent_0100
 * @tc.desc: Send "ORDERED_EVENT_PROC_TIMEOUT" hisysevent.
 * @tc.type: FUNC
 * @tc.require: I582Y4
 */
HWTEST_F(CommonEventHisyseventTest, SendOrderedEventProcTimeoutSysEvent_0100, Level1)
{
    GTEST_LOG_(INFO) << "SendOrderedEventProcTimeoutSysEvent_0100 start";

    EventInfo eventInfo;
    eventInfo.userId = TEST_USER_ID;
    eventInfo.subscriberName = TEST_SUBSCRIBER_BUNDLE_NAME;
    eventInfo.pid = getpid();
    eventInfo.uid = getuid();
    eventInfo.eventName = TEST_EVENT_NAME;
    EventReport::SendHiSysEvent(ORDERED_EVENT_PROC_TIMEOUT, eventInfo);

    GTEST_LOG_(INFO) << "SendOrderedEventProcTimeoutSysEvent_0100 end";
}

/**
 * @tc.name: SendStaticEventProcErrorSysEvent_0100
 * @tc.desc: Send "STATIC_EVENT_PROC_ERROR" hisysevent.
 * @tc.type: FUNC
 * @tc.require: I582Y4
 */
HWTEST_F(CommonEventHisyseventTest, SendStaticEventProcErrorSysEvent_0100, Level1)
{
    GTEST_LOG_(INFO) << "SendStaticEventProcErrorSysEvent_0100 start";

    EventInfo eventInfo;
    eventInfo.userId = TEST_USER_ID;
    eventInfo.publisherName = TEST_PUBLISHER_BUNDLE_NAME;
    eventInfo.subscriberName = TEST_SUBSCRIBER_BUNDLE_NAME;
    eventInfo.eventName = TEST_EVENT_NAME;
    EventReport::SendHiSysEvent(STATIC_EVENT_PROC_ERROR, eventInfo);

    GTEST_LOG_(INFO) << "SendStaticEventProcErrorSysEvent_0100 end";
}

/**
 * @tc.name: SendSubscriberExceedMaximumSysEvent_0100
 * @tc.desc: Send "SUBSCRIBER_EXCEED_MAXIMUM" hisysevent.
 * @tc.type: FUNC
 * @tc.require: I582Y4
 */
HWTEST_F(CommonEventHisyseventTest, SendSubscriberExceedMaximumSysEvent_0100, Level1)
{
    GTEST_LOG_(INFO) << "SendSubscriberExceedMaximumSysEvent_0100 start";

    EventInfo eventInfo;
    eventInfo.userId = TEST_USER_ID;
    eventInfo.eventName = TEST_EVENT_NAME;
    eventInfo.subscriberNum = TEST_SUBSCRIBER_NUM;
    EventReport::SendHiSysEvent(SUBSCRIBER_EXCEED_MAXIMUM, eventInfo);

    GTEST_LOG_(INFO) << "SendSubscriberExceedMaximumSysEvent_0100 end";
}

/**
 * @tc.name: SendPublishEventErrorSysEvent_0100
 * @tc.desc: Send "PUBLISH_EVENT_ERROR" hisysevent.
 * @tc.type: FUNC
 * @tc.require: I582Y4
 */
HWTEST_F(CommonEventHisyseventTest, SendPublishEventErrorSysEvent_0100, Level1)
{
    GTEST_LOG_(INFO) << "SendSubscriberExceedMaximumSysEvent_0100 start";

    EventInfo eventInfo;
    eventInfo.userId = TEST_USER_ID;
    eventInfo.publisherName = TEST_SUBSCRIBER_BUNDLE_NAME;
    eventInfo.pid = getpid();
    eventInfo.uid = getuid();
    eventInfo.eventName = TEST_EVENT_NAME;
    EventReport::SendHiSysEvent(PUBLISH_ERROR, eventInfo);

    GTEST_LOG_(INFO) << "SendPublishEventErrorSysEvent_0100 end";
}

/**
 * @tc.name: SendSubscribeSysEvent_0100
 * @tc.desc: Send "SUBSCRIBE_EVENT" hisysevent.
 * @tc.type: FUNC
 * @tc.require: I582Y4
 */
HWTEST_F(CommonEventHisyseventTest, SendSubscribeSysEvent_0100, Level1)
{
    GTEST_LOG_(INFO) << "SendSubscribeSysEvent_0100 start";

    EventInfo eventInfo;
    eventInfo.userId = TEST_USER_ID;
    eventInfo.subscriberName = TEST_SUBSCRIBER_BUNDLE_NAME;
    eventInfo.pid = getpid();
    eventInfo.uid = getuid();
    std::vector<std::string> events = {"event1", "event2", "event3"};
    eventInfo.eventName = std::accumulate(events.begin(), events.end(), eventInfo.eventName,
        [events](std::string eventName, const std::string &str) {
            return (str == events.front()) ? (eventName + str) : (eventName + "," + str);
        });
    EventReport::SendHiSysEvent(SUBSCRIBE, eventInfo);

    std::vector<std::string> anotherEvent = {"event"};
    eventInfo.eventName = std::accumulate(anotherEvent.begin(), anotherEvent.end(), std::string(""),
        [anotherEvent](std::string eventName, const std::string &str) {
            return (str == anotherEvent.front()) ? (eventName + str) : (eventName + "," + str);
        });
    EventReport::SendHiSysEvent(SUBSCRIBE, eventInfo);

    GTEST_LOG_(INFO) << "SendSubscribeSysEvent_0100 end";
}

/**
 * @tc.name: SendUnSubscribeSysEvent_0100
 * @tc.desc: Send "UNSUBSCRIBE_EVENT" hisysevent.
 * @tc.type: FUNC
 * @tc.require: I582Y4
 */
HWTEST_F(CommonEventHisyseventTest, SendUnSubscribeSysEvent_0100, Level1)
{
    GTEST_LOG_(INFO) << "SendUnSubscribeSysEvent_0100 start";

    EventInfo eventInfo;
    eventInfo.userId = TEST_USER_ID;
    eventInfo.subscriberName = TEST_SUBSCRIBER_BUNDLE_NAME;
    eventInfo.pid = getpid();
    eventInfo.uid = getuid();
    std::vector<std::string> events = {"event1_1", "event1_2", "event1_3"};
    eventInfo.eventName = std::accumulate(events.begin(), events.end(), eventInfo.eventName,
        [events](std::string eventName, const std::string &str) {
            return (str == events.front()) ? (eventName + str) : (eventName + "," + str);
        });
    EventReport::SendHiSysEvent(UNSUBSCRIBE, eventInfo);

    eventInfo.eventName = "";
    std::vector<std::string> anotherEvent = {"event"};
    eventInfo.eventName = std::accumulate(anotherEvent.begin(), anotherEvent.end(), eventInfo.eventName,
        [anotherEvent](std::string eventName, const std::string &str) {
            return (str == anotherEvent.front()) ? (eventName + str) : (eventName + "," + str);
        });
    EventReport::SendHiSysEvent(UNSUBSCRIBE, eventInfo);

    GTEST_LOG_(INFO) << "SendUnSubscribeSysEvent_0100 end";
}

/**
 * @tc.name: SendPublishSysEvent_0100
 * @tc.desc: Send "PUBLISH_EVENT" hisysevent.
 * @tc.type: FUNC
 * @tc.require: I582Y4
 */
HWTEST_F(CommonEventHisyseventTest, SendPublishSysEvent_0100, Level1)
{
    GTEST_LOG_(INFO) << "SendPublishSysEvent_0100 start";

    EventInfo eventInfo;
    eventInfo.userId = TEST_USER_ID;
    eventInfo.publisherName = TEST_PUBLISHER_BUNDLE_NAME;
    eventInfo.pid = getpid();
    eventInfo.uid = getuid();
    eventInfo.eventName = TEST_EVENT_NAME;
    EventReport::SendHiSysEvent(PUBLISH, eventInfo);

    GTEST_LOG_(INFO) << "SendPublishSysEvent_0100 end";
}
}  // namespace EventFwk
}  // namespace OHOS