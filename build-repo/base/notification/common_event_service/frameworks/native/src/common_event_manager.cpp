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

#include "common_event_manager.h"

#include "common_event.h"
#include "common_event_constant.h"
#include "event_log_wrapper.h"
#include "singleton.h"

namespace OHOS {
namespace EventFwk {
bool CommonEventManager::PublishCommonEvent(const CommonEventData &data)
{
    CommonEventPublishInfo publishInfo;
    return PublishCommonEventAsUser(data, publishInfo, nullptr, UNDEFINED_USER);
}

bool CommonEventManager::PublishCommonEventAsUser(const CommonEventData &data, const int32_t &userId)
{
    CommonEventPublishInfo publishInfo;
    return PublishCommonEventAsUser(data, publishInfo, nullptr, userId);
}

bool CommonEventManager::PublishCommonEvent(const CommonEventData &data, const CommonEventPublishInfo &publishInfo)
{
    return NewPublishCommonEvent(data, publishInfo) == ERR_OK ? true : false;
}

int32_t CommonEventManager::NewPublishCommonEvent(const CommonEventData &data,
    const CommonEventPublishInfo &publishInfo)
{
    return NewPublishCommonEventAsUser(data, publishInfo, nullptr, UNDEFINED_USER);
}

bool CommonEventManager::PublishCommonEventAsUser(const CommonEventData &data,
    const CommonEventPublishInfo &publishInfo, const int32_t &userId)
{
    return NewPublishCommonEventAsUser(data, publishInfo, userId) == ERR_OK ? true : false;
}

int32_t CommonEventManager::NewPublishCommonEventAsUser(const CommonEventData &data,
    const CommonEventPublishInfo &publishInfo, const int32_t &userId)
{
    return NewPublishCommonEventAsUser(data, publishInfo, nullptr, userId);
}

bool CommonEventManager::PublishCommonEvent(const CommonEventData &data, const CommonEventPublishInfo &publishInfo,
    const std::shared_ptr<CommonEventSubscriber> &subscriber)
{
    return PublishCommonEventAsUser(data, publishInfo, subscriber, UNDEFINED_USER);
}

bool CommonEventManager::PublishCommonEventAsUser(const CommonEventData &data,
    const CommonEventPublishInfo &publishInfo, const std::shared_ptr<CommonEventSubscriber> &subscriber,
    const int32_t &userId)
{
    EVENT_LOGI("enter");
    return NewPublishCommonEventAsUser(data, publishInfo, subscriber, userId) == ERR_OK ? true : false;
}

int32_t CommonEventManager::NewPublishCommonEventAsUser(const CommonEventData &data,
    const CommonEventPublishInfo &publishInfo, const std::shared_ptr<CommonEventSubscriber> &subscriber,
    const int32_t &userId)
{
    EVENT_LOGI("enter");
    return DelayedSingleton<CommonEvent>::GetInstance()->PublishCommonEventAsUser(data, publishInfo, subscriber,
        userId);
}

bool CommonEventManager::PublishCommonEvent(const CommonEventData &data, const CommonEventPublishInfo &publishInfo,
    const std::shared_ptr<CommonEventSubscriber> &subscriber, const uid_t &uid, const int32_t &callerToken)
{
    return PublishCommonEventAsUser(data, publishInfo, subscriber, uid, callerToken, UNDEFINED_USER);
}

bool CommonEventManager::PublishCommonEventAsUser(const CommonEventData &data,
    const CommonEventPublishInfo &publishInfo, const std::shared_ptr<CommonEventSubscriber> &subscriber,
    const uid_t &uid, const int32_t &callerToken, const int32_t &userId)
{
    EVENT_LOGI("enter");
    return DelayedSingleton<CommonEvent>::GetInstance()->PublishCommonEventAsUser(data, publishInfo, subscriber, uid,
        callerToken, userId);
}

bool CommonEventManager::SubscribeCommonEvent(const std::shared_ptr<CommonEventSubscriber> &subscriber)
{
    return NewSubscribeCommonEvent(subscriber) == ERR_OK ? true : false;
}

int32_t CommonEventManager::NewSubscribeCommonEvent(const std::shared_ptr<CommonEventSubscriber> &subscriber)
{
    return DelayedSingleton<CommonEvent>::GetInstance()->SubscribeCommonEvent(subscriber);
}

bool CommonEventManager::UnSubscribeCommonEvent(const std::shared_ptr<CommonEventSubscriber> &subscriber)
{
    return NewUnSubscribeCommonEvent(subscriber) == ERR_OK ? true : false;
}

int32_t CommonEventManager::NewUnSubscribeCommonEvent(const std::shared_ptr<CommonEventSubscriber> &subscriber)
{
    return DelayedSingleton<CommonEvent>::GetInstance()->UnSubscribeCommonEvent(subscriber);
}

bool CommonEventManager::GetStickyCommonEvent(const std::string &event, CommonEventData &commonEventData)
{
    return DelayedSingleton<CommonEvent>::GetInstance()->GetStickyCommonEvent(event, commonEventData);
}

bool CommonEventManager::Freeze(const uid_t &uid)
{
    return DelayedSingleton<CommonEvent>::GetInstance()->Freeze(uid);
}

bool CommonEventManager::Unfreeze(const uid_t &uid)
{
    return DelayedSingleton<CommonEvent>::GetInstance()->Unfreeze(uid);
}

bool CommonEventManager::UnfreezeAll()
{
    return DelayedSingleton<CommonEvent>::GetInstance()->UnfreezeAll();
}
}  // namespace EventFwk
}  // namespace OHOS