/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_COMMON_EVENT_STICKY_MANAGER_H
#define FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_COMMON_EVENT_STICKY_MANAGER_H

#include "common_event_record.h"
#include "common_event_subscribe_info.h"
#include "singleton.h"

namespace OHOS {
namespace EventFwk {
class CommonEventStickyManager : public DelayedSingleton<CommonEventStickyManager> {
public:
    using CommonEventRecordPtr = std::shared_ptr<CommonEventRecord>;
    using CommonEventDataPtr = std::shared_ptr<CommonEventData>;
    using CommonEventPublishInfoPtr = std::shared_ptr<CommonEventPublishInfo>;
    using SubscribeInfoPtr = std::shared_ptr<CommonEventSubscribeInfo>;

    /**
     * Finds the sticky events.
     *
     * @param subscribeInfo Indicates the subscribe information.
     * @param commonEventRecords Indicates the records of sticky common event.
     * @return Returns result code.
     */
    int FindStickyEvents(const SubscribeInfoPtr &subscribeInfo, std::vector<CommonEventRecordPtr> &commonEventRecords);

    /**
     * Gets the sticky event.
     *
     * @param event Indicates the event name.
     * @param eventData Indicates the common event data.
     * @return Returns true if successful; false otherwise.
     */
    bool GetStickyCommonEvent(const std::string &event, CommonEventData &eventData);

    /**
     * Updates the sticky events.
     *
     * @param eventRecord Indicates the record of sticky common event.
     * @return Returns result code.
     */
    int UpdateStickyEvent(const CommonEventRecord &eventRecord);

    /**
     * Dumps the state of the sticky events.
     *
     * @param event Specifies the information for the common event. Set null string ("") if you want to dump all.
     * @param userId Indicates the user ID.
     * @param state Indicates the state of common event service.
     */
    void DumpState(const std::string &event, const int32_t &userId, std::vector<std::string> &state);

private:
    void FindStickyEventsLocked(
        const std::vector<std::string> &events, std::vector<CommonEventRecordPtr> &commonEventRecords);

    bool GetStickyCommonEventLocked(const std::string &event, CommonEventData &eventData);

    int UpdateStickyEventLocked(const std::string &event, const CommonEventRecordPtr &record);

    void GetStickyCommonEventRecords(
        const std::string &event, const int32_t &userId, std::vector<CommonEventRecordPtr> &records);

private:
    std::mutex mutex_;
    std::map<std::string, CommonEventRecordPtr> commonEventRecords_;
};
}  // namespace EventFwk
}  // namespace OHOS

#endif  // FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_COMMON_EVENT_STICKY_MANAGER_H