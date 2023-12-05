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

#ifndef FOUNDATION_EVENT_CESFWK_INNERKITS_INCLUDE_COMMON_EVENT_PROXY_H
#define FOUNDATION_EVENT_CESFWK_INNERKITS_INCLUDE_COMMON_EVENT_PROXY_H

#include "icommon_event.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace EventFwk {
class CommonEventProxy : public IRemoteProxy<ICommonEvent> {
public:
    /**
     * Constructor.
     *
     * @param object Indicates the remote object
     */
    explicit CommonEventProxy(const sptr<IRemoteObject> &object);

    virtual ~CommonEventProxy();

    /**
     * Publishes a common event.
     *
     * @param event Indicates the common event data.
     * @param publishInfo Indicates the publish info.
     * @param commonEventListener Indicates the last subscriber to receive the event.
     * @param userId Indicates the user ID.
     * @return Returns true if success; false otherwise.
     */
    int32_t PublishCommonEvent(const CommonEventData &event, const CommonEventPublishInfo &publishInfo,
        const sptr<IRemoteObject> &commonEventListener, const int32_t &userId) override;

    /**
     * Publishes a common event.
     *
     * @param event Indicates the common event data
     * @param publishInfo Indicates the publish info
     * @param commonEventListener Indicates the last subscriber to receive the event
     * @param uid Indicates the uid
     * @param callerToken Indicates the caller token
     * @param userId Indicates the user ID
     * @return Returns true if success; false otherwise.
     */
    bool PublishCommonEvent(const CommonEventData &event, const CommonEventPublishInfo &publishinfo,
        const sptr<IRemoteObject> &commonEventListener, const uid_t &uid, const int32_t &callerToken,
        const int32_t &userId) override;

    /**
     * Subscribes to common events.
     *
     * @param subscribeInfo Indicates the subscribe information
     * @param commonEventListener Indicates the subscriber object
     * @return Returns true if successful; false otherwise.
     */
    int32_t SubscribeCommonEvent(
        const CommonEventSubscribeInfo &subscribeInfo, const sptr<IRemoteObject> &commonEventListener) override;

    /**
     * Unsubscribes from common events.
     *
     * @param commonEventListener Indicates the subscriber object
     * @return Returns true if successful; false otherwise.
     */
    int32_t UnsubscribeCommonEvent(const sptr<IRemoteObject> &commonEventListener) override;

    /**
     * Gets the current sticky common event
     *
     * @param event Indicates the common event.
     * @param eventData Indicates the common event data.
     * @return Returns true if successful; false otherwise.
     */
    bool GetStickyCommonEvent(const std::string &event, CommonEventData &eventData) override;

    /**
     * Dumps the state for common event service.
     *
     * @param dumpType Indicates the dump type.
     * @param event Indicates the specified event.
     * @param userId Indicates the user id.
     * @param state Indicates the output result.
     * @return Returns true if successful; false otherwise.
     */
    bool DumpState(const uint8_t &dumpType, const std::string &event, const int32_t &userId,
        std::vector<std::string> &state) override;

    /**
     * Finishes the receiver for the ordered common event.
     *
     * @param proxy Indicates the current subscriber object.
     * @param code Indicates the result code.
     * @param receiverData Indicates the result data.
     * @param abortEvent Indicates whether the current ordered common event should be aborted.
     * @return Returns true if successful; false otherwise.
     */
    bool FinishReceiver(const sptr<IRemoteObject> &proxy, const int32_t &code, const std::string &receiverData,
        const bool &abortEvent) override;

    /**
     * Freezes the specified process.
     *
     * @param uid Indicates the uid of frozen process.
     * @return Returns true if successful; false otherwise.
     */
    bool Freeze(const uid_t &uid) override;

    /**
     * Unfreezes the specified process.
     *
     * @param uid Indicates the uid of unfrozen process.
     * @return Returns true if successful; false otherwise.
     */
    bool Unfreeze(const uid_t &uid) override;

    /**
     * Unfreezes all frozen applications.
     *
     * @return Returns true if successful; false otherwise.
     */
    bool UnfreezeAll() override;

private:
    bool SendRequest(ICommonEvent::Message code, MessageParcel &data, MessageParcel &reply);

private:
    static inline BrokerDelegator<CommonEventProxy> delegator_;
};
}  // namespace EventFwk
}  // namespace OHOS

#endif  // FOUNDATION_EVENT_CESFWK_INNERKITS_INCLUDE_COMMON_EVENT_PROXY_H
