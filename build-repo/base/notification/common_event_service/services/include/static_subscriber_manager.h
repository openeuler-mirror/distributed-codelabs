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

#ifndef FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_STATIC_SUBSCRIBER_MANAGER_H
#define FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_STATIC_SUBSCRIBER_MANAGER_H

#include <map>
#include <string>
#include <vector>

#include "nlohmann/json.hpp"

#include "accesstoken_kit.h"
#include "bundle_manager_helper.h"
#include "common_event_data.h"
#include "common_event_publish_info.h"
#include "singleton.h"

namespace OHOS {
namespace EventFwk {
class StaticSubscriberManager : public DelayedSingleton<StaticSubscriberManager> {
public:
    StaticSubscriberManager();

    virtual ~StaticSubscriberManager();

    /**
     * Publishes common event to the static subscriber.
     * @param data Indicates the common event data.
     * @param publishInfo Indicates the publish informattion.
     * @param callerToken Indicates the caller token.
     * @param userId Indicates the ID of user.
     * @param service Indicates the service.
     */
    void PublishCommonEvent(const CommonEventData &data, const CommonEventPublishInfo &publishInfo,
        const Security::AccessToken::AccessTokenID &callerToken, const int32_t &userId,
        const sptr<IRemoteObject> &service, const std::string &bundleName);

private:
    struct StaticSubscriberInfo {
        std::string name;
        std::string bundleName;
        int32_t userId = -1;
        std::string permission;

        bool operator==(const StaticSubscriberInfo &that) const
        {
            return (name == that.name) && (bundleName == that.bundleName) && (userId == that.userId) &&
                (permission == that.permission);
        }
    };

    bool InitAllowList();
    bool InitValidSubscribers();
    void UpdateSubscriber(const CommonEventData &data);
    void ParseEvents(const std::string &extensionName, const std::string &extensionBundleName,
        const int32_t &extensionUid, const std::string &profile);
    void AddSubscriber(const AppExecFwk::ExtensionAbilityInfo &extension);
    void AddToValidSubscribers(const std::string &eventName, const StaticSubscriberInfo &extension);
    void AddSubscriberWithBundleName(const std::string &bundleName, const int32_t &userId);
    void RemoveSubscriberWithBundleName(const std::string &bundleName, const int32_t &userId);
    bool VerifySubscriberPermission(const std::string &bundleName, const int32_t &userId,
        const std::vector<std::string> &permissions);
    bool VerifyPublisherPermission(const Security::AccessToken::AccessTokenID &callerToken,
        const std::string &permission);
    void SendStaticEventProcErrHiSysEvent(int32_t userId, const std::string &publisherName,
        const std::string &subscriberName, const std::string &eventName);

    std::vector<std::string> subscriberList_;
    std::map<std::string, std::vector<StaticSubscriberInfo>> validSubscribers_;
    bool hasInitAllowList_ = false;
    bool hasInitValidSubscribers_ = false;
    std::mutex subscriberMutex_;
};
}  // namespace EventFwk
}  // namespace OHOS

#endif  // FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_STATIC_SUBSCRIBER_MANAGER_H
