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

#ifndef FOUNDATION_EVENT_CESFWK_INNERKITS_INCLUDE_ICOMMON_EVENT_H
#define FOUNDATION_EVENT_CESFWK_INNERKITS_INCLUDE_ICOMMON_EVENT_H

#include "common_event_data.h"
#include "common_event_publish_info.h"
#include "common_event_subscribe_info.h"
#include "iremote_broker.h"
#include "iremote_object.h"

namespace OHOS {
namespace EventFwk {
class ICommonEvent : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.eventfwk.ICommonEvent");

    /**
     * Publishes a common event.
     *
     * @param event Indicates the common event data.
     * @param publishInfo Indicates the publish info.
     * @param commonEventListener Indicates the last subscriber to receive the event.
     * @param userId Indicates the user ID.
     * @return Returns true if successful; false otherwise.
     */
    virtual int32_t PublishCommonEvent(const CommonEventData &event, const CommonEventPublishInfo &publishinfo,
        const sptr<IRemoteObject> &commonEventListener, const int32_t &userId) = 0;

    /**
     * Publishes a common event.
     *
     * @param event Indicates the common event data.
     * @param publishInfo Indicates the publish info.
     * @param commonEventListener Indicates the last subscriber to receive the event.
     * @param uid Indicates the uid.
     * @param callerToken Indicates the caller token
     * @param userId Indicates the user ID.
     * @return Returns true if successful; false otherwise.
     */
    virtual bool PublishCommonEvent(const CommonEventData &event, const CommonEventPublishInfo &publishinfo,
        const sptr<IRemoteObject> &commonEventListener, const uid_t &uid, const int32_t &callerToken,
        const int32_t &userId) = 0;

    /**
     * Subscribes to common events.
     *
     * @param subscribeInfo Indicates the subscribe information.
     * @param commonEventListener Indicates the subscriber object.
     * @return Returns true if successful; false otherwise.
     */
    virtual int32_t SubscribeCommonEvent(
        const CommonEventSubscribeInfo &subscribeInfo, const sptr<IRemoteObject> &commonEventListener) = 0;

    /**
     * Unsubscribes from common events.
     *
     * @param commonEventListener Indicates the subscriber object.
     * @return Returns true if successful; false otherwise.
     */
    virtual int32_t UnsubscribeCommonEvent(const sptr<IRemoteObject> &commonEventListener) = 0;

    /**
     * Gets the current sticky common event
     *
     * @param event Indicates the common event.
     * @param eventData Indicates he common event data.
     * @return Returns true if successful; false otherwise.
     */
    virtual bool GetStickyCommonEvent(const std::string &event, CommonEventData &eventData) = 0;

    /**
     * Dumps the state for common event service.
     *
     * @param dumpType Indicates the dump type.
     * @param event Indicates the specified event.
     * @param userId Indicates the user id.
     * @param state Indicates the output result.
     * @return Returns true if successful; false otherwise.
     */
    virtual bool DumpState(const uint8_t &dumpType, const std::string &event, const int32_t &userId,
        std::vector<std::string> &state) = 0;

    /**
     * Finishes the receiver for the ordered common event.
     *
     * @param proxy Indicates the current subscriber object.
     * @param code Indicates the result code.
     * @param receiverData Indicates the result data.
     * @param abortEvent Indicates whether the current ordered common event should be aborted.
     * @return Returns true if successful; false otherwise.
     */
    virtual bool FinishReceiver(const sptr<IRemoteObject> &proxy, const int32_t &code,
        const std::string &receiverData, const bool &abortEvent) = 0;

    /**
     * Freezes the specified process.
     *
     * @param uid Indicates the uid of frozen process.
     * @return Returns true if successful; false otherwise.
     */
    virtual bool Freeze(const uid_t &uid) = 0;

    /**
     * Unfreezes the specified process.
     *
     * @param uid Indicates the uid of unfrozen process.
     * @return Returns true if successful; false otherwise.
     */
    virtual bool Unfreeze(const uid_t &uid) = 0;

    /**
     * Unfreezes all frozen applications.
     *
     * @return Returns true if successful; false otherwise.
     */
    virtual bool UnfreezeAll() = 0;

    enum class Message {
        CES_PUBLISH_COMMON_EVENT = 0,
        CES_PUBLISH_COMMON_EVENT2,
        CES_SUBSCRIBE_COMMON_EVENT,
        CES_UNSUBSCRIBE_COMMON_EVENT,
        CES_GET_STICKY_COMMON_EVENT,
        CES_FINISH_RECEIVER,
        CES_FREEZE,
        CES_UNFREEZE,
        CES_UNFREEZE_ALL,
        // ipc id 2001-3000 for tools
        // ipc id for dumping Subscribe State (2001)
        CES_DUMP_STATE = 2001,
    };
};
}  // namespace EventFwk
}  // namespace OHOS

#endif  // FOUNDATION_EVENT_CESFWK_INNERKITS_INCLUDE_ICOMMON_EVENT_H
