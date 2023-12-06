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

#include "static_subscriber_manager.h"

#include <fstream>

#include "ability_manager_helper.h"
#include "access_token_helper.h"
#include "bundle_manager_helper.h"
#include "common_event_constant.h"
#include "common_event_support.h"
#include "event_log_wrapper.h"
#include "event_report.h"
// #include "hitrace_meter.h"
#include "os_account_manager_helper.h"

namespace OHOS {
namespace EventFwk {
namespace {
const std::string STATIC_SUBSCRIBER_CONFIG_FILE = "/system/etc/static_subscriber_config.json";
const std::string CONFIG_APPS = "apps";
constexpr static char JSON_KEY_COMMON_EVENTS[] = "commonEvents";
constexpr static char JSON_KEY_NAME[] = "name";
constexpr static char JSON_KEY_PERMISSION[] = "permission";
constexpr static char JSON_KEY_EVENTS[] = "events";
}

StaticSubscriberManager::StaticSubscriberManager() {}

StaticSubscriberManager::~StaticSubscriberManager() {}

bool StaticSubscriberManager::InitAllowList()
{
    EVENT_LOGI("enter");

    nlohmann::json jsonObj;
    std::ifstream jfile(STATIC_SUBSCRIBER_CONFIG_FILE);
    if (!jfile.is_open()) {
        EVENT_LOGI("json file can not open");
        hasInitAllowList_ = false;
        return false;
    }
    jfile >> jsonObj;
    jfile.close();

    for (auto j : jsonObj[CONFIG_APPS]) {
        subscriberList_.emplace_back(j.get<std::string>());
    }
    hasInitAllowList_ = true;
    return true;
}

bool StaticSubscriberManager::InitValidSubscribers()
{
    EVENT_LOGI("enter");

    if (!validSubscribers_.empty()) {
        validSubscribers_.clear();
    }
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensions;
    // get all static subscriber type extensions
    if (!DelayedSingleton<BundleManagerHelper>::GetInstance()->QueryExtensionInfos(extensions)) {
        EVENT_LOGE("QueryExtensionInfos failed");
        return false;
    }
    // filter legal extensions and add them to valid map
    for (auto extension : extensions) {
        if (find(subscriberList_.begin(), subscriberList_.end(), extension.bundleName) == subscriberList_.end()) {
            continue;
        }
        EVENT_LOGI("find legal extension, bundlename = %{public}s", extension.bundleName.c_str());
        AddSubscriber(extension);
    }
    hasInitValidSubscribers_ = true;
    return true;
}

void StaticSubscriberManager::PublishCommonEvent(const CommonEventData &data,
    const CommonEventPublishInfo &publishInfo, const Security::AccessToken::AccessTokenID &callerToken,
    const int32_t &userId, const sptr<IRemoteObject> &service, const std::string &bundleName)
{
    // HITRACE_METER_NAME(HITRACE_TAG_NOTIFICATION, __PRETTY_FUNCTION__);
    EVENT_LOGI("enter, event = %{public}s, userId = %{public}d", data.GetWant().GetAction().c_str(), userId);

    std::lock_guard<std::mutex> lock(subscriberMutex_);
    if (!hasInitAllowList_ && !InitAllowList()) {
        EVENT_LOGE("failed to init subscriber list");
        return;
    }

    if ((!hasInitValidSubscribers_ ||
        data.GetWant().GetAction() == CommonEventSupport::COMMON_EVENT_BOOT_COMPLETED ||
        data.GetWant().GetAction() == CommonEventSupport::COMMON_EVENT_LOCKED_BOOT_COMPLETED ||
        data.GetWant().GetAction() == CommonEventSupport::COMMON_EVENT_USER_SWITCHED ||
        data.GetWant().GetAction() == CommonEventSupport::COMMON_EVENT_UID_REMOVED ||
        data.GetWant().GetAction() == CommonEventSupport::COMMON_EVENT_USER_STARTED) &&
        !InitValidSubscribers()) {
        EVENT_LOGE("failed to init Init valid subscribers map!");
        return;
    }

    UpdateSubscriber(data);

    auto targetSubscribers = validSubscribers_.find(data.GetWant().GetAction());
    if (targetSubscribers != validSubscribers_.end()) {
        for (auto subscriber : targetSubscribers->second) {
            EVENT_LOGW("subscriber.userId = %{public}d, userId = %{public}d", subscriber.userId, userId);
            if (subscriber.userId < SUBSCRIBE_USER_SYSTEM_BEGIN) {
                EVENT_LOGW("subscriber userId is invalid, subscriber.userId = %{public}d", subscriber.userId);
                SendStaticEventProcErrHiSysEvent(userId, bundleName, subscriber.bundleName, data.GetWant().GetAction());
                continue;
            }
            if ((subscriber.userId > SUBSCRIBE_USER_SYSTEM_END) && (userId != ALL_USER)
                && (subscriber.userId != userId)) {
                EVENT_LOGW("subscriber userId is not match, subscriber.userId = %{public}d, userId = %{public}d",
                    subscriber.userId, userId);
                SendStaticEventProcErrHiSysEvent(userId, bundleName, subscriber.bundleName, data.GetWant().GetAction());
                continue;
            }
            // judge if app is system app.
            if (!DelayedSingleton<BundleManagerHelper>::GetInstance()->
                CheckIsSystemAppByBundleName(subscriber.bundleName, subscriber.userId)) {
                EVENT_LOGW("subscriber is not system app, not allow.");
                continue;
            }
            if (!VerifyPublisherPermission(callerToken, subscriber.permission)) {
                EVENT_LOGW("publisher does not have required permission %{public}s", subscriber.permission.c_str());
                SendStaticEventProcErrHiSysEvent(userId, bundleName, subscriber.bundleName, data.GetWant().GetAction());
                continue;
            }
            if (!VerifySubscriberPermission(subscriber.bundleName, subscriber.userId,
                publishInfo.GetSubscriberPermissions())) {
                EVENT_LOGW("subscriber does not have required permissions");
                SendStaticEventProcErrHiSysEvent(userId, bundleName, subscriber.bundleName, data.GetWant().GetAction());
                continue;
            }
            if (!publishInfo.GetBundleName().empty() && subscriber.bundleName != publishInfo.GetBundleName()) {
                EVENT_LOGW("subscriber bundleName is not match, subscriber.bundleName = %{public}s, "
                    "bundleName = %{public}s", subscriber.bundleName.c_str(), publishInfo.GetBundleName().c_str());
                continue;
            }
            AAFwk::Want want;
            want.SetElementName(subscriber.bundleName, subscriber.name);
            EVENT_LOGI("Ready to connect to subscriber %{public}s in bundle %{public}s",
                subscriber.name.c_str(), subscriber.bundleName.c_str());
            DelayedSingleton<AbilityManagerHelper>::GetInstance()->
                ConnectAbility(want, data, service, subscriber.userId);
        }
    }
}

bool StaticSubscriberManager::VerifyPublisherPermission(const Security::AccessToken::AccessTokenID &callerToken,
    const std::string &permission)
{
    EVENT_LOGI("enter");
    if (permission.empty()) {
        EVENT_LOGI("no need permission");
        return true;
    }
    return AccessTokenHelper::VerifyAccessToken(callerToken, permission);
}

bool StaticSubscriberManager::VerifySubscriberPermission(const std::string &bundleName, const int32_t &userId,
    const std::vector<std::string> &permissions)
{
    // get hap tokenid with default instindex(0), this should be modified later.
    Security::AccessToken::AccessTokenID tokenId = AccessTokenHelper::GetHapTokenID(userId, bundleName, 0);
    for (auto permission : permissions) {
        if (permission.empty()) {
            continue;
        }
        if (!AccessTokenHelper::VerifyAccessToken(tokenId, permission)) {
            EVENT_LOGW("subscriber does not have required permission : %{public}s", permission.c_str());
            return false;
        }
    }
    return true;
}

void StaticSubscriberManager::ParseEvents(const std::string &extensionName, const std::string &extensionBundleName,
    const int32_t &extensionUserId, const std::string &profile)
{
    EVENT_LOGI("enter, subscriber name = %{public}s, bundle name = %{public}s, userId = %{public}d",
        extensionName.c_str(), extensionBundleName.c_str(), extensionUserId);
    
    if (profile.empty()) {
        EVENT_LOGE("invalid profile");
        return;
    }
    if (!nlohmann::json::accept(profile.c_str())) {
        EVENT_LOGE("invalid format profile");
        return;
    }

    nlohmann::json jsonObj = nlohmann::json::parse(profile, nullptr, false);
    if (jsonObj.is_null() || jsonObj.empty()) {
        EVENT_LOGE("invalid jsonObj");
        return;
    }
    nlohmann::json commonEventsObj = jsonObj[JSON_KEY_COMMON_EVENTS];
    if (commonEventsObj.is_null() || !commonEventsObj.is_array() || commonEventsObj.empty()) {
        EVENT_LOGE("invalid common event obj size");
        return;
    }
    for (auto commonEventObj : commonEventsObj) {
        if (commonEventObj.is_null() || !commonEventObj.is_object()) {
            EVENT_LOGW("invalid common event obj");
            continue;
        }
        if (commonEventObj[JSON_KEY_NAME].is_null() || !commonEventObj[JSON_KEY_NAME].is_string()) {
            EVENT_LOGW("invalid common event ability name obj");
            continue;
        }
        if (commonEventObj[JSON_KEY_NAME].get<std::string>() != extensionName) {
            EVENT_LOGW("extensionName is not match");
            continue;
        }
        if (commonEventObj[JSON_KEY_PERMISSION].is_null() || !commonEventObj[JSON_KEY_PERMISSION].is_string()) {
            EVENT_LOGW("invalid permission obj");
            continue;
        }
        if (commonEventObj[JSON_KEY_EVENTS].is_null() || !commonEventObj[JSON_KEY_EVENTS].is_array() ||
            commonEventObj[JSON_KEY_EVENTS].empty()) {
            EVENT_LOGW("invalid events obj");
            continue;
        }

        for (auto e : commonEventObj[JSON_KEY_EVENTS]) {
            if (e.is_null() || !e.is_string()) {
                EVENT_LOGW("invalid event obj");
                continue;
            }
            StaticSubscriberInfo subscriber = { .name = extensionName, .bundleName = extensionBundleName,
                .userId = extensionUserId, .permission = commonEventObj[JSON_KEY_PERMISSION].get<std::string>() };
            AddToValidSubscribers(e.get<std::string>(), subscriber);
        }
    }
}

void StaticSubscriberManager::AddSubscriber(const AppExecFwk::ExtensionAbilityInfo &extension)
{
    EVENT_LOGI("enter, subscriber bundlename = %{public}s", extension.bundleName.c_str());

    std::vector<std::string> profileInfos;
    if (!DelayedSingleton<BundleManagerHelper>::GetInstance()->GetResConfigFile(extension, profileInfos)) {
        EVENT_LOGE("GetProfile failed");
        return;
    }
    for (auto profile : profileInfos) {
        int32_t userId = -1;
        if (DelayedSingleton<OsAccountManagerHelper>::GetInstance()->GetOsAccountLocalIdFromUid(
            extension.applicationInfo.uid, userId) != ERR_OK) {
            EVENT_LOGE("GetOsAccountLocalIdFromUid failed, uid = %{public}d", extension.applicationInfo.uid);
            return;
        }
        ParseEvents(extension.name, extension.bundleName, userId, profile);
    }
}

void StaticSubscriberManager::AddToValidSubscribers(const std::string &eventName,
    const StaticSubscriberInfo &subscriber)
{
    if (validSubscribers_.find(eventName) != validSubscribers_.end()) {
        for (auto sub : validSubscribers_[eventName]) {
            if ((sub.name == subscriber.name) &&
                (sub.bundleName == subscriber.bundleName) &&
                (sub.userId == subscriber.userId)) {
                EVENT_LOGI("subscriber already exist, event = %{public}s, bundlename = %{public}s, name = %{public}s,"
                    "userId = %{public}d", eventName.c_str(), subscriber.bundleName.c_str(), subscriber.name.c_str(),
                    subscriber.userId);
                return;
            }
        }
    }
    validSubscribers_[eventName].emplace_back(subscriber);
    EVENT_LOGI("subscriber added, event = %{public}s, bundlename = %{public}s, name = %{public}s, userId = %{public}d",
        eventName.c_str(), subscriber.bundleName.c_str(), subscriber.name.c_str(), subscriber.userId);
}

void StaticSubscriberManager::AddSubscriberWithBundleName(const std::string &bundleName, const int32_t &userId)
{
    EVENT_LOGI("enter, bundleName = %{public}s, userId = %{public}d", bundleName.c_str(), userId);

    std::vector<AppExecFwk::ExtensionAbilityInfo> extensions;
    if (!DelayedSingleton<BundleManagerHelper>::GetInstance()->QueryExtensionInfos(extensions, userId)) {
        EVENT_LOGE("QueryExtensionInfos failed");
        return;
    }

    for (auto extension : extensions) {
        if ((extension.bundleName == bundleName) &&
            find(subscriberList_.begin(), subscriberList_.end(), extension.bundleName) != subscriberList_.end()) {
            AddSubscriber(extension);
        }
    }
}

void StaticSubscriberManager::RemoveSubscriberWithBundleName(const std::string &bundleName, const int32_t &userId)
{
    EVENT_LOGI("enter, bundleName = %{public}s, userId = %{public}d", bundleName.c_str(), userId);

    for (auto it = validSubscribers_.begin(); it != validSubscribers_.end();) {
        auto subIt = it->second.begin();
        while (subIt != it->second.end()) {
            if ((subIt->bundleName == bundleName) && (subIt->userId == userId)) {
                EVENT_LOGI("remove subscriber, event = %{public}s, bundlename = %{public}s, userId = %{public}d",
                    it->first.c_str(), bundleName.c_str(), userId);
                subIt = it->second.erase(subIt);
            } else {
                subIt++;
            }
        }
        if (it->second.empty()) {
            validSubscribers_.erase(it++);
        } else {
            it++;
        }
    }
}

void StaticSubscriberManager::UpdateSubscriber(const CommonEventData &data)
{
    // HITRACE_METER_NAME(HITRACE_TAG_NOTIFICATION, __PRETTY_FUNCTION__);
    EVENT_LOGI("enter");

    if ((data.GetWant().GetAction() != CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED) &&
        (data.GetWant().GetAction() != CommonEventSupport::COMMON_EVENT_PACKAGE_CHANGED) &&
        (data.GetWant().GetAction() != CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED)) {
        EVENT_LOGI("no need to update map");
        return;
    }

    std::string bundleName = data.GetWant().GetElement().GetBundleName();
    int32_t uid = data.GetWant().GetIntParam(AppExecFwk::Constants::UID, -1);
    int32_t userId = -1;
    if (DelayedSingleton<OsAccountManagerHelper>::GetInstance()->GetOsAccountLocalIdFromUid(uid, userId) != ERR_OK) {
        EVENT_LOGW("GetOsAccountLocalIdFromUid failed, uid = %{public}d", uid);
        return;
    }
    std::vector<int> osAccountIds;
    if (DelayedSingleton<OsAccountManagerHelper>::GetInstance()->QueryActiveOsAccountIds(osAccountIds) != ERR_OK) {
        EVENT_LOGW("failed to QueryActiveOsAccountIds!");
        return;
    }
    if (find(osAccountIds.begin(), osAccountIds.end(), userId) == osAccountIds.end()) {
        EVENT_LOGW("userId is not active, no need to update.");
        return;
    }
    EVENT_LOGI("active uid = %{public}d, userId = %{public}d", uid, userId);
    if (data.GetWant().GetAction() == CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED) {
        EVENT_LOGI("UpdateSubscribersMap bundle %{public}s ready to add", bundleName.c_str());
        AddSubscriberWithBundleName(bundleName, userId);
    } else if (data.GetWant().GetAction() == CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED) {
        EVENT_LOGI("UpdateSubscribersMap bundle %{public}s ready to remove", bundleName.c_str());
        RemoveSubscriberWithBundleName(bundleName, userId);
    } else {
        EVENT_LOGI("UpdateSubscribersMap bundle %{public}s ready to update", bundleName.c_str());
        RemoveSubscriberWithBundleName(bundleName, userId);
        AddSubscriberWithBundleName(bundleName, userId);
    }
}

void StaticSubscriberManager::SendStaticEventProcErrHiSysEvent(int32_t userId, const std::string &publisherName,
    const std::string &subscriberName, const std::string &eventName)
{
    EventInfo eventInfo;
    eventInfo.userId = userId;
    eventInfo.publisherName = publisherName;
    eventInfo.subscriberName = subscriberName;
    eventInfo.eventName = eventName;
    EventReport::SendHiSysEvent(STATIC_EVENT_PROC_ERROR, eventInfo);
}
}  // namespace EventFwk
}  // namespace OHOS
