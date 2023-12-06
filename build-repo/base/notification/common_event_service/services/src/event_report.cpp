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

#include "event_log_wrapper.h"
#include "event_report.h"

namespace OHOS {
namespace EventFwk {
namespace {
// event params
const std::string EVENT_PARAM_USER_ID = "USER_ID";
const std::string EVENT_PARAM_PID = "PID";
const std::string EVENT_PARAM_UID = "UID";
const std::string EVENT_PARAM_SUBSCRIBER_NUM = "SUBSCRIBER_NUM";
const std::string EVENT_PARAM_PUBLISHER_BUNDLE_NAME = "PUBLISHER_BUNDLE_NAME";
const std::string EVENT_PARAM_SUBSCRIBER_BUNDLE_NAME = "SUBSCRIBER_BUNDLE_NAME";
const std::string EVENT_PARAM_EVENT_NAME = "EVENT_NAME";
} // namespace

void EventReport::SendHiSysEvent(const std::string &eventName, const EventInfo &eventInfo)
{
#ifndef HAS_HISYSEVENT_PART
    EVENT_LOGD("Hisysevent is disabled");
#else
    auto iter = cesSysEventFuncMap_.find(eventName);
    if (iter == cesSysEventFuncMap_.end()) {
        return;
    }

    iter->second(eventInfo);
#endif
}

#ifdef HAS_HISYSEVENT_PART
std::unordered_map<std::string, void (*)(const EventInfo& eventInfo)> EventReport::cesSysEventFuncMap_ = {
    {ORDERED_EVENT_PROC_TIMEOUT, [](const EventInfo& eventInfo) {
        InnerSendOrderedEventProcTimeoutEvent(eventInfo);
    }},
    {STATIC_EVENT_PROC_ERROR, [](const EventInfo& eventInfo) {
        InnerSendStaticEventProcErrorEvent(eventInfo);
    }},
    {SUBSCRIBER_EXCEED_MAXIMUM, [](const EventInfo& eventInfo) {
        InnerSendSubscriberExceedMaximumEvent(eventInfo);
    }},
    {PUBLISH_ERROR, [](const EventInfo& eventInfo) {
        InnerSendPublishErrorEvent(eventInfo);
    }},
    {SUBSCRIBE, [](const EventInfo& eventInfo) {
        InnerSendSubscribeEvent(eventInfo);
    }},
    {UNSUBSCRIBE, [](const EventInfo& eventInfo) {
        InnerSendUnSubscribeEvent(eventInfo);
    }},
    {PUBLISH, [](const EventInfo& eventInfo) {
        InnerSendPublishEvent(eventInfo);
    }},
};

void EventReport::InnerSendOrderedEventProcTimeoutEvent(const EventInfo &eventInfo)
{
    InnerEventWrite(
        ORDERED_EVENT_PROC_TIMEOUT,
        HiviewDFX::HiSysEvent::EventType::FAULT,
        EVENT_PARAM_USER_ID, eventInfo.userId,
        EVENT_PARAM_SUBSCRIBER_BUNDLE_NAME, eventInfo.subscriberName,
        EVENT_PARAM_PID, eventInfo.pid,
        EVENT_PARAM_UID, eventInfo.uid,
        EVENT_PARAM_EVENT_NAME, eventInfo.eventName);
}

void EventReport::InnerSendStaticEventProcErrorEvent(const EventInfo &eventInfo)
{
    InnerEventWrite(
        STATIC_EVENT_PROC_ERROR,
        HiviewDFX::HiSysEvent::EventType::FAULT,
        EVENT_PARAM_USER_ID, eventInfo.userId,
        EVENT_PARAM_PUBLISHER_BUNDLE_NAME, eventInfo.publisherName,
        EVENT_PARAM_SUBSCRIBER_BUNDLE_NAME, eventInfo.subscriberName,
        EVENT_PARAM_EVENT_NAME, eventInfo.eventName);
}

void EventReport::InnerSendSubscriberExceedMaximumEvent(const EventInfo &eventInfo)
{
    InnerEventWrite(
        SUBSCRIBER_EXCEED_MAXIMUM,
        HiviewDFX::HiSysEvent::EventType::FAULT,
        EVENT_PARAM_USER_ID, eventInfo.userId,
        EVENT_PARAM_EVENT_NAME, eventInfo.eventName,
        EVENT_PARAM_SUBSCRIBER_NUM, eventInfo.subscriberNum);
}

void EventReport::InnerSendPublishErrorEvent(const EventInfo &eventInfo)
{
    InnerEventWrite(
        PUBLISH_ERROR,
        HiviewDFX::HiSysEvent::EventType::FAULT,
        EVENT_PARAM_USER_ID, eventInfo.userId,
        EVENT_PARAM_PUBLISHER_BUNDLE_NAME, eventInfo.publisherName,
        EVENT_PARAM_PID, eventInfo.pid,
        EVENT_PARAM_UID, eventInfo.uid,
        EVENT_PARAM_EVENT_NAME, eventInfo.eventName);
}

void EventReport::InnerSendSubscribeEvent(const EventInfo &eventInfo)
{
    InnerEventWrite(
        SUBSCRIBE,
        HiviewDFX::HiSysEvent::EventType::STATISTIC,
        EVENT_PARAM_USER_ID, eventInfo.userId,
        EVENT_PARAM_SUBSCRIBER_BUNDLE_NAME, eventInfo.subscriberName,
        EVENT_PARAM_PID, eventInfo.pid,
        EVENT_PARAM_UID, eventInfo.uid,
        EVENT_PARAM_EVENT_NAME, eventInfo.eventName);
}

void EventReport::InnerSendUnSubscribeEvent(const EventInfo &eventInfo)
{
    InnerEventWrite(
        UNSUBSCRIBE,
        HiviewDFX::HiSysEvent::EventType::STATISTIC,
        EVENT_PARAM_USER_ID, eventInfo.userId,
        EVENT_PARAM_SUBSCRIBER_BUNDLE_NAME, eventInfo.subscriberName,
        EVENT_PARAM_PID, eventInfo.pid,
        EVENT_PARAM_UID, eventInfo.uid,
        EVENT_PARAM_EVENT_NAME, eventInfo.eventName);
}

void EventReport::InnerSendPublishEvent(const EventInfo &eventInfo)
{
    InnerEventWrite(
        PUBLISH,
        HiviewDFX::HiSysEvent::EventType::STATISTIC,
        EVENT_PARAM_USER_ID, eventInfo.userId,
        EVENT_PARAM_PUBLISHER_BUNDLE_NAME, eventInfo.publisherName,
        EVENT_PARAM_PID, eventInfo.pid,
        EVENT_PARAM_UID, eventInfo.uid,
        EVENT_PARAM_EVENT_NAME, eventInfo.eventName);
}

template<typename... Types>
void EventReport::InnerEventWrite(const std::string &eventName,
    HiviewDFX::HiSysEvent::EventType type, Types... keyValues)
{
    HiviewDFX::HiSysEvent::Write(
        HiviewDFX::HiSysEvent::Domain::COMMONEVENT,
        eventName,
        static_cast<HiviewDFX::HiSysEvent::EventType>(type),
        keyValues...);
}
#endif
} // namespace EventFwk
} // namespace OHOS