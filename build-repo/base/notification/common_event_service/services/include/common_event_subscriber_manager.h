/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_COMMON_EVENT_SUBSCRIBER_MANAGER_H
#define FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_COMMON_EVENT_SUBSCRIBER_MANAGER_H

#include "common_event_constant.h"
#include "common_event_record.h"
#include "common_event_subscribe_info.h"
#include "iremote_object.h"
#include "singleton.h"

namespace OHOS {
namespace EventFwk {
struct EventSubscriberRecord {
    std::shared_ptr<CommonEventSubscribeInfo> eventSubscribeInfo;
    sptr<IRemoteObject> commonEventListener;
    EventRecordInfo eventRecordInfo;
    struct tm recordTime {0};
    bool isFreeze;
    int64_t freezeTime;

    EventSubscriberRecord()
        : eventSubscribeInfo(nullptr),
          commonEventListener(nullptr),
          isFreeze(false),
          freezeTime(0)
    {}
};

struct FrozenEventRecord {
    std::shared_ptr<EventSubscriberRecord> subscriberRecordPtr;
    std::vector<std::shared_ptr<CommonEventRecord>> eventRecordPtrs;

    FrozenEventRecord() : subscriberRecordPtr(nullptr)
    {}
};

inline bool operator<(const std::shared_ptr<EventSubscriberRecord> &a, const std::shared_ptr<EventSubscriberRecord> &b)
{
    return a->eventSubscribeInfo->GetPriority() > b->eventSubscribeInfo->GetPriority();
}

using SubscriberRecordPtr = std::shared_ptr<EventSubscriberRecord>;
using SubscribeInfoPtr = std::shared_ptr<CommonEventSubscribeInfo>;
using EventRecordPtr = std::shared_ptr<CommonEventRecord>;
using FrozenRecords = std::map<SubscriberRecordPtr, std::vector<EventRecordPtr>>;

class CommonEventSubscriberManager : public DelayedSingleton<CommonEventSubscriberManager> {
public:
    CommonEventSubscriberManager();

    virtual ~CommonEventSubscriberManager() override;

    /**
     * Inserts a specific subscriber.
     *
     * @param eventSubscribeInfo Indicates the subscribe information.
     * @param commonEventListener Indicates the subscriber object.
     * @param recordTime Indicates the time of record.
     * @param eventRecordInfo Indicates the information of event record.
     * @return Returns the subscribe record.
     */
    std::shared_ptr<EventSubscriberRecord> InsertSubscriber(const SubscribeInfoPtr &eventSubscribeInfo,
        const sptr<IRemoteObject> &commonEventListener, const struct tm &recordTime,
        const EventRecordInfo &eventRecordInfo);

    /**
     * Removes subscriber.
     *
     * @param commonEventListener Indicates the subscriber object.
     * @return Returns the result code.
     */
    int RemoveSubscriber(const sptr<IRemoteObject> &commonEventListener);

    /**
     * Gets subscriber records.
     *
     * @param eventRecord Indicates the event record.
     * @return Returns the subscriber records.
     */
    std::vector<SubscriberRecordPtr> GetSubscriberRecords(const CommonEventRecord &eventRecord);

    /**
     * @brief Get the subscribe record by subscriber object.
     *
     * @param commonEventListener Indicates the subscriber object.
     * @return std::shared_ptr<EventSubscriberRecord>
     */
    std::shared_ptr<EventSubscriberRecord> GetSubscriberRecord(const sptr<IRemoteObject> &commonEventListener);

    /**
     * Updates freeze information.
     *
     * @param uid Indicates the uid of the application.
     * @param freezeState Indicates the freeze state.
     * @param freezeTime Indicates the freeze time.
     */
    void UpdateFreezeInfo(const uid_t &uid, const bool &freezeState, const int64_t &freezeTime = 0);

    /**
     * Updates freeze information of all applications.
     *
     * @param freezeState Indicates the freeze state.
     * @param freezeTime Indicates the freeze time.
     */
    void UpdateAllFreezeInfos(const bool &freezeState, const int64_t &freezeTime = 0);

    /**
     * Inserts freeze events.
     *
     * @param eventListener Indicates the subscriber object.
     * @param eventRecord Indicates the event record.
     */
    void InsertFrozenEvents(const SubscriberRecordPtr &eventListener, const CommonEventRecord &eventRecord);

    /**
     * Gets the frozen events.
     *
     * @param uid Indicates the uid of the application.
     * @return Returns the frozen events.
     */
    FrozenRecords GetFrozenEvents(const uid_t &uid);

    /**
     * Gets all frozen events.
     *
     * @return Returns all frozen events.
     */
    std::map<uid_t, FrozenRecords> GetAllFrozenEvents();

    /**
     * Dumps detailed information for specific subscriber record info.
     *
     * @param title Indicates the log tag.
     * @param record Indicates the subscriber record.
     * @param format Indicates the log format.
     * @param dumpInfo Indicates the output information.
     */
    void DumpDetailed(
        const std::string &title, const SubscriberRecordPtr &record, const std::string format, std::string &dumpInfo);

    /**
     * Dumps state information.
     *
     * @param event Specifies the information for the common event. Set null string ("") if you want to dump all.
     * @param userId Indicates the user ID.
     * @param state Indicates the output information.
     */
    void DumpState(const std::string &event, const int32_t &userId, std::vector<std::string> &state);

private:
    bool InsertSubscriberRecordLocked(const std::vector<std::string> &events, const SubscriberRecordPtr &record);

    int RemoveSubscriberRecordLocked(const sptr<IRemoteObject> &commonEventListener);

    bool CheckSubscriberByUserId(const int32_t &subscriberUserId, const bool &isSystemApp, const int32_t &userId);

    void GetSubscriberRecordsByWantLocked(const CommonEventRecord &eventRecord,
        std::vector<SubscriberRecordPtr> &records);

    void GetSubscriberRecordsByEvent(
        const std::string &event, const int32_t &userId, std::vector<SubscriberRecordPtr> &records);

    void RemoveFrozenEventsBySubscriber(const SubscriberRecordPtr &subscriberRecord);

    void RemoveFrozenEvents(const uid_t &uid);

    void SendSubscriberExceedMaximumHiSysEvent(int32_t userId, const std::string &eventName, uint32_t subscriberNum);

private:
    std::mutex mutex_;
    sptr<IRemoteObject::DeathRecipient> death_;
    std::map<std::string, std::multiset<SubscriberRecordPtr>> eventSubscribers_;
    std::vector<SubscriberRecordPtr> subscribers_;
    std::map<uid_t, FrozenRecords> frozenEvents_;
    const time_t FREEZE_EVENT_TIMEOUT = 30; // How long we keep records. Unit: second
};
}  // namespace EventFwk
}  // namespace OHOS
#endif  // FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_COMMON_EVENT_SUBSCRIBER_MANAGER_H