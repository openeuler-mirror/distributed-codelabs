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

#ifndef FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_INNER_COMMON_EVENT_MANAGER_H
#define FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_INNER_COMMON_EVENT_MANAGER_H

#include "access_token_helper.h"
#include "common_event_control_manager.h"
#include "icommon_event.h"
#include "static_subscriber_manager.h"

namespace OHOS {
namespace EventFwk {
class InnerCommonEventManager {
public:
    InnerCommonEventManager();

    virtual ~InnerCommonEventManager() {};

    /**
     * Publishes a common event.
     *
     * @param data Indicates the common event data.
     * @param publishInfo Indicates the publish info.
     * @param commonEventListener Indicates the common event subscriber.
     * @param recordTime Indicates the time of record.
     * @param pid Indicates the pid of application.
     * @param uid Indicates the uid of application.
     * @param callerToken Indicates the token of caller.
     * @param userId Indicates the user ID.
     * @param bundleName Indicates the name of bundle.
     * @param service Indicates the common event service.
     * @return Returns true if successful; false otherwise.
     */
    bool PublishCommonEvent(const CommonEventData &data, const CommonEventPublishInfo &publishinfo,
        const sptr<IRemoteObject> &commonEventListener, const struct tm &recordTime, const pid_t &pid, const uid_t &uid,
        const Security::AccessToken::AccessTokenID &callerToken, const int32_t &userId, const std::string &bundleName,
        const sptr<IRemoteObject> &service = nullptr);

    /**
     * Subscribes to common events.
     *
     * @param subscribeInfo Indicates the subscribe info.
     * @param commonEventListener Indicates the common event subscriber.
     * @param recordTime Indicates the time of record.
     * @param pid Indicates the pid of application.
     * @param uid Indicates the uid of application.
     * @param callerToken Indicates the token of caller.
     * @param bundleName Indicates the name of bundle.
     * @return Returns true if successful; false otherwise.
     */
    bool SubscribeCommonEvent(const CommonEventSubscribeInfo &subscribeInfo,
        const sptr<IRemoteObject> &commonEventListener, const struct tm &recordTime, const pid_t &pid, const uid_t &uid,
        const Security::AccessToken::AccessTokenID &callerToken, const std::string &bundleName);

    /**
     * Unsubscribes from common events.
     *
     * @param commonEventListener Indicates the common event subscriber.
     * @return Returns true if successful; false otherwise.
     */
    bool UnsubscribeCommonEvent(const sptr<IRemoteObject> &commonEventListener);

    /**
     * Gets the current sticky common event
     *
     * @param event Indicates the common event.
     * @param eventData Indicates the common event data.
     * @return Returns true if successful; false otherwise.
     */
    bool GetStickyCommonEvent(const std::string &event, CommonEventData &eventData);

    /**
     * Dumps state of common event service.
     *
     * @param dumpType Indicates the dump type.
     * @param event Specifies the information for the common event. Set null string ("") if you want to dump all.
     * @param userId Indicates the user ID.
     * @param state Indicates the state of common event service.
     */
    void DumpState(const uint8_t &dumpType, const std::string &event, const int32_t &userId,
        std::vector<std::string> &state);

    /**
     * Finishes Receiver.
     *
     * @param proxy Indicates the receiver proxy.
     * @param code Indicates the code of a common event.
     * @param data Indicates the data of a common event.
     * @param abortEvent Indicates Whether to cancel the current common event.
     */
    void FinishReceiver(
        const sptr<IRemoteObject> &proxy, const int32_t &code, const std::string &receiverData, const bool &abortEvent);

    /**
     * Freezes application.
     *
     * @param uid Indicates the uid of application.
     */
    void Freeze(const uid_t &uid);

    /**
     * Unfreezes application.
     *
     * @param uid Indicates the uid of application.
     */
    void Unfreeze(const uid_t &uid);

    /**
     * Unfreezes all frozen applications.
     */
    void UnfreezeAll();

    /**
     * dump event for hidumper.
     *
     * @param args Indicates the dump options.
     * @param result the result of dump
     */
    void HiDump(const std::vector<std::u16string> &args, std::string &result);
private:
    bool ProcessStickyEvent(const CommonEventRecord &record);
    bool PublishStickyEvent(const std::shared_ptr<CommonEventSubscribeInfo> &sp,
        const std::shared_ptr<EventSubscriberRecord> &subscriberRecord);
    bool CheckUserId(const pid_t &pid, const uid_t &uid, const Security::AccessToken::AccessTokenID &callerToken,
        bool &isSubsystem, bool &isSystemApp, bool &isProxy, int32_t &userId);
    void SendSubscribeHiSysEvent(int32_t userId, const std::string &subscriberName, int32_t pid, int32_t uid,
        const std::vector<std::string> &events);
    void SendUnSubscribeHiSysEvent(const sptr<IRemoteObject> &commonEventListener);
    void SendPublishHiSysEvent(int32_t userId, const std::string &publisherName, int32_t pid, int32_t uid,
        const std::string &events, bool succeed);

private:
    std::shared_ptr<CommonEventControlManager> controlPtr_;
    std::shared_ptr<StaticSubscriberManager> staticSubscriberManager_;
    DISALLOW_COPY_AND_MOVE(InnerCommonEventManager);
    time_t sysEventTime = 0;
};
}  // namespace EventFwk
}  // namespace OHOS
#endif  // FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_INNER_COMMON_EVENT_MANAGER_H
