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

#include "common_event.h"
#include "common_event_constant.h"
#include "common_event_death_recipient.h"
#include "event_log_wrapper.h"
// #include "hitrace_meter.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "ces_inner_error_code.h"

namespace OHOS {
namespace EventFwk {
using namespace OHOS::Notification;

bool CommonEvent::PublishCommonEvent(const CommonEventData &data, const CommonEventPublishInfo &publishInfo,
    const std::shared_ptr<CommonEventSubscriber> &subscriber)
{
    // HITRACE_METER_NAME(HITRACE_TAG_NOTIFICATION, __PRETTY_FUNCTION__);
    EVENT_LOGI("enter");
    sptr<IRemoteObject> commonEventListener = nullptr;
    if (!PublishParameterCheck(data, publishInfo, subscriber, commonEventListener)) {
        EVENT_LOGE("failed to check publish parameter");
        return false;
    }
    EVENT_LOGD("before PublishCommonEvent proxy valid state is %{public}d", isProxyValid_);
    return commonEventProxy_->PublishCommonEvent(
        data, publishInfo, commonEventListener, UNDEFINED_USER) == ERR_OK ? true : false;
}

int32_t CommonEvent::PublishCommonEventAsUser(const CommonEventData &data, const CommonEventPublishInfo &publishInfo,
    const std::shared_ptr<CommonEventSubscriber> &subscriber, const int32_t &userId)
{
    // HITRACE_METER_NAME(HITRACE_TAG_NOTIFICATION, __PRETTY_FUNCTION__);
    EVENT_LOGI("enter");
    sptr<IRemoteObject> commonEventListener = nullptr;
    if (!PublishParameterCheck(data, publishInfo, subscriber, commonEventListener)) {
        EVENT_LOGE("failed to check publish parameter");
        return ERR_NOTIFICATION_CES_COMMON_PARAM_INVALID;
    }
    EVENT_LOGD("before PublishCommonEvent proxy valid state is %{public}d", isProxyValid_);
    return commonEventProxy_->PublishCommonEvent(data, publishInfo, commonEventListener, userId);
}

bool CommonEvent::PublishCommonEvent(const CommonEventData &data, const CommonEventPublishInfo &publishInfo,
    const std::shared_ptr<CommonEventSubscriber> &subscriber, const uid_t &uid, const int32_t &callerToken)
{
    // HITRACE_METER_NAME(HITRACE_TAG_NOTIFICATION, __PRETTY_FUNCTION__);
    EVENT_LOGI("enter");
    sptr<IRemoteObject> commonEventListener = nullptr;
    if (!PublishParameterCheck(data, publishInfo, subscriber, commonEventListener)) {
        EVENT_LOGE("failed to check publish parameter");
        return false;
    }
    EVENT_LOGD("before PublishCommonEvent proxy valid state is %{public}d", isProxyValid_);
    return commonEventProxy_->PublishCommonEvent(data, publishInfo, commonEventListener, uid, callerToken,
        UNDEFINED_USER);
}

bool CommonEvent::PublishCommonEventAsUser(const CommonEventData &data, const CommonEventPublishInfo &publishInfo,
    const std::shared_ptr<CommonEventSubscriber> &subscriber, const uid_t &uid,
    const int32_t &callerToken, const int32_t &userId)
{
    // HITRACE_METER_NAME(HITRACE_TAG_NOTIFICATION, __PRETTY_FUNCTION__);
    EVENT_LOGI("enter");
    sptr<IRemoteObject> commonEventListener = nullptr;
    if (!PublishParameterCheck(data, publishInfo, subscriber, commonEventListener)) {
        EVENT_LOGE("failed to check publish parameter");
        return false;
    }
    EVENT_LOGD("before PublishCommonEvent proxy valid state is %{public}d", isProxyValid_);
    return commonEventProxy_->PublishCommonEvent(data, publishInfo, commonEventListener, uid, callerToken, userId);
}

bool CommonEvent::PublishParameterCheck(const CommonEventData &data, const CommonEventPublishInfo &publishInfo,
    const std::shared_ptr<CommonEventSubscriber> &subscriber, sptr<IRemoteObject> &commonEventListener)
{
    EVENT_LOGI("enter");
    if (data.GetWant().GetAction() == std::string()) {
        EVENT_LOGE("the commonEventdata action is null");
        return false;
    }

    if (!publishInfo.IsOrdered() && (subscriber != nullptr)) {
        EVENT_LOGE("When publishing unordered events, the subscriber object is not required.");
        return false;
    }

    if (!GetCommonEventProxy()) {
        EVENT_LOGE("the commonEventProxy_ is null");
        return false;
    }

    if (subscriber) {
        if (CreateCommonEventListener(subscriber, commonEventListener) == SUBSCRIBE_FAILED) {
            EVENT_LOGE("failed to CreateCommonEventListener");
            return false;
        }
    }

    return true;
}

int32_t CommonEvent::SubscribeCommonEvent(const std::shared_ptr<CommonEventSubscriber> &subscriber)
{
    // HITRACE_METER_NAME(HITRACE_TAG_NOTIFICATION, __PRETTY_FUNCTION__);
    EVENT_LOGI("enter");

    if (subscriber == nullptr) {
        EVENT_LOGE("the subscriber is null");
        return ERR_NOTIFICATION_CES_COMMON_PARAM_INVALID;
    }

    if (subscriber->GetSubscribeInfo().GetMatchingSkills().CountEvent() == 0) {
        EVENT_LOGE("the subscriber has no event");
        return ERR_NOTIFICATION_CES_COMMON_PARAM_INVALID;
    }

    if (!GetCommonEventProxy()) {
        EVENT_LOGE("the commonEventProxy_ is null");
        return ERR_NOTIFICATION_CES_COMMON_PARAM_INVALID;
    }

    sptr<IRemoteObject> commonEventListener = nullptr;
    uint8_t subscribeState = CreateCommonEventListener(subscriber, commonEventListener);
    if (subscribeState == INITIAL_SUBSCRIPTION) {
        EVENT_LOGD("before SubscribeCommonEvent proxy valid state is %{public}d", isProxyValid_);
        return commonEventProxy_->SubscribeCommonEvent(subscriber->GetSubscribeInfo(), commonEventListener);
    } else if (subscribeState == ALREADY_SUBSCRIBED) {
        return ERR_OK;
    } else {
        return ERR_NOTIFICATION_CES_COMMON_SYSTEMCAP_NOT_SUPPORT;
    }
}

int32_t CommonEvent::UnSubscribeCommonEvent(const std::shared_ptr<CommonEventSubscriber> &subscriber)
{
    // HITRACE_METER_NAME(HITRACE_TAG_NOTIFICATION, __PRETTY_FUNCTION__);
    EVENT_LOGI("enter");

    if (subscriber == nullptr) {
        EVENT_LOGE("the subscriber is null");
        return ERR_NOTIFICATION_CES_COMMON_PARAM_INVALID;
    }

    if (!GetCommonEventProxy()) {
        EVENT_LOGE("the commonEventProxy is null");
        return ERR_NOTIFICATION_CES_COMMON_PARAM_INVALID;
    }

    std::lock_guard<std::mutex> lock(eventListenersMutex_);
    auto eventListener = eventListeners_.find(subscriber);
    if (eventListener != eventListeners_.end()) {
        EVENT_LOGD("before UnsubscribeCommonEvent proxy valid state is %{public}d", isProxyValid_);
        if (commonEventProxy_->UnsubscribeCommonEvent(eventListener->second->AsObject()) == ERR_OK) {
            eventListener->second->Stop();
            eventListeners_.erase(eventListener);
            return ERR_OK;
        }
        return ERR_NOTIFICATION_CES_COMMON_SYSTEMCAP_NOT_SUPPORT;
    } else {
        EVENT_LOGW("No specified subscriber has been registered");
    }

    return ERR_OK;
}

bool CommonEvent::GetStickyCommonEvent(const std::string &event, CommonEventData &eventData)
{
    EVENT_LOGI("enter");

    if (!GetCommonEventProxy()) {
        EVENT_LOGE("the commonEventProxy is null");
        return false;
    }

    if (event.empty()) {
        EVENT_LOGE("event is empty");
        return false;
    }
    EVENT_LOGD("before GetStickyCommonEvent proxy valid state is %{public}d", isProxyValid_);
    return commonEventProxy_->GetStickyCommonEvent(event, eventData);
}

bool CommonEvent::FinishReceiver(
    const sptr<IRemoteObject> &proxy, const int32_t &code, const std::string &data, const bool &abortEvent)
{
    EVENT_LOGI("enter");

    if (proxy == nullptr) {
        EVENT_LOGE("the proxy is null");
        return false;
    }

    if (!GetCommonEventProxy()) {
        EVENT_LOGE("the commonEventProxy is null");
        return false;
    }
    EVENT_LOGD("before FinishReceiver proxy valid state is %{public}d", isProxyValid_);
    return commonEventProxy_->FinishReceiver(proxy, code, data, abortEvent);
}

bool CommonEvent::DumpState(const uint8_t &dumpType, const std::string &event, const int32_t &userId,
    std::vector<std::string> &state)
{
    EVENT_LOGI("enter");

    if (!GetCommonEventProxy()) {
        EVENT_LOGE("the commonEventProxy is null");
        return false;
    }
    EVENT_LOGD("before DumpState proxy valid state is %{public}d", isProxyValid_);
    return commonEventProxy_->DumpState(dumpType, event, userId, state);
}

void CommonEvent::ResetCommonEventProxy()
{
    EVENT_LOGI("enter");
    std::lock_guard<std::mutex> lock(mutex_);
    isProxyValid_ = false;
    if ((commonEventProxy_ == nullptr) || (commonEventProxy_->AsObject() == nullptr)) {
        EVENT_LOGE("invalid proxy state");
        return;
    }
    commonEventProxy_->AsObject()->RemoveDeathRecipient(recipient_);
}

bool CommonEvent::Freeze(const uid_t &uid)
{
    EVENT_LOGI("enter");

    if (!GetCommonEventProxy()) {
        EVENT_LOGE("the commonEventProxy is null");
        return false;
    }
    EVENT_LOGD("before Freeze proxy valid state is %{public}d", isProxyValid_);
    return commonEventProxy_->Freeze(uid);
}

bool CommonEvent::Unfreeze(const uid_t &uid)
{
    EVENT_LOGI("enter");

    if (!GetCommonEventProxy()) {
        EVENT_LOGE("the commonEventProxy is null");
        return false;
    }
    EVENT_LOGD("before Unfreeze proxy valid state is %{public}d", isProxyValid_);
    return commonEventProxy_->Unfreeze(uid);
}

bool CommonEvent::UnfreezeAll()
{
    EVENT_LOGI("enter");

    if (!GetCommonEventProxy()) {
        EVENT_LOGE("the commonEventProxy is null");
        return false;
    }
    EVENT_LOGD("before UnfreezeAll proxy valid state is %{public}d", isProxyValid_);
    return commonEventProxy_->UnfreezeAll();
}

bool CommonEvent::GetCommonEventProxy()
{
    EVENT_LOGI("enter");

    if (!commonEventProxy_ || !isProxyValid_) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!commonEventProxy_ || !isProxyValid_) {
            sptr<ISystemAbilityManager> systemAbilityManager =
                SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
            if (!systemAbilityManager) {
                EVENT_LOGE("Failed to get system ability mgr.");
                return false;
            }

            sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(COMMON_EVENT_SERVICE_ID);
            if (!remoteObject) {
                EVENT_LOGE("Failed to get COMMON Event Manager.");
                return false;
            }

            commonEventProxy_ = iface_cast<ICommonEvent>(remoteObject);
            if ((!commonEventProxy_) || (!commonEventProxy_->AsObject())) {
                EVENT_LOGE("Failed to get COMMON Event Manager's proxy");
                return false;
            }

            recipient_ = new (std::nothrow) CommonEventDeathRecipient();
            if (!recipient_) {
                EVENT_LOGE("Failed to create death Recipient ptr CommonEventDeathRecipient!");
                return false;
            }
            commonEventProxy_->AsObject()->AddDeathRecipient(recipient_);
            isProxyValid_ = true;
        }
    }
    return true;
}

uint8_t CommonEvent::CreateCommonEventListener(
    const std::shared_ptr<CommonEventSubscriber> &subscriber, sptr<IRemoteObject> &commonEventListener)
{
    if (subscriber == nullptr) {
        EVENT_LOGE("subscriber is null");
        return SUBSCRIBE_FAILED;
    }

    std::lock_guard<std::mutex> lock(eventListenersMutex_);

    auto eventListener = eventListeners_.find(subscriber);
    if (eventListener != eventListeners_.end()) {
        commonEventListener = eventListener->second->AsObject();
        EVENT_LOGW("subscriber has common event listener");
        return ALREADY_SUBSCRIBED;
    } else {
        if (eventListeners_.size() == SUBSCRIBER_MAX_SIZE) {
            EVENT_LOGE("the maximum number of subscriptions has been reached");
            return SUBSCRIBE_FAILED;
        }

        sptr<CommonEventListener> listener = new CommonEventListener(subscriber);
        if (!listener) {
            EVENT_LOGE("the common event listener is null");
            return SUBSCRIBE_FAILED;
        }
        commonEventListener = listener->AsObject();
        eventListeners_[subscriber] = listener;
    }

    return INITIAL_SUBSCRIPTION;
}
}  // namespace EventFwk
}  // namespace OHOS