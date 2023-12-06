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

#ifndef FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_EVENT_REPORT_H
#define FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_EVENT_REPORT_H

#include <unordered_map>

#ifdef HAS_HISYSEVENT_PART
#include "hisysevent.h"
#endif

namespace OHOS {
namespace EventFwk {
namespace {
// event name
constexpr char ORDERED_EVENT_PROC_TIMEOUT[] = "ORDERED_EVENT_PROC_TIMEOUT";
constexpr char STATIC_EVENT_PROC_ERROR[] = "STATIC_EVENT_PROC_ERROR";
constexpr char SUBSCRIBER_EXCEED_MAXIMUM[] = "SUBSCRIBER_EXCEED_MAXIMUM";
constexpr char PUBLISH_ERROR[] = "PUBLISH_ERROR";
constexpr char SUBSCRIBE[] = "SUBSCRIBE";
constexpr char UNSUBSCRIBE[] = "UNSUBSCRIBE";
constexpr char PUBLISH[] = "PUBLISH";
} // namespace

struct EventInfo {
    int32_t userId;
    int32_t pid;
    int32_t uid;
    uint32_t subscriberNum;
    std::string publisherName;
    std::string subscriberName;
    std::string eventName;

    EventInfo() : userId(-1), pid(0), uid(0), subscriberNum(0) {}
};

class EventReport {
public:
    /**
     * @brief send hisysevent
     *
     * @param eventName event name, corresponding to the document 'hisysevent.yaml'
     * @param eventInfo event info
     */
    static void SendHiSysEvent(const std::string &eventName, const EventInfo &eventInfo);

private:
#ifdef HAS_HISYSEVENT_PART
    // fault event
    static void InnerSendOrderedEventProcTimeoutEvent(const EventInfo &eventInfo);
    static void InnerSendStaticEventProcErrorEvent(const EventInfo &eventInfo);
    static void InnerSendSubscriberExceedMaximumEvent(const EventInfo &eventInfo);
    static void InnerSendPublishErrorEvent(const EventInfo &eventInfo);

    // statistic event
    static void InnerSendSubscribeEvent(const EventInfo &eventInfo);
    static void InnerSendUnSubscribeEvent(const EventInfo &eventInfo);
    static void InnerSendPublishEvent(const EventInfo &eventInfo);

    template<typename... Types>
    static void InnerEventWrite(const std::string &eventName,
        HiviewDFX::HiSysEvent::EventType type, Types... keyValues);

    static std::unordered_map<std::string, void (*)(const EventInfo &eventInfo)> cesSysEventFuncMap_;
#endif
};
} // namespace EventFwk
} // namespace OHOS
#endif  // FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_EVENT_REPORT_H