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

#include "common_event_control_manager.h"

#include <cinttypes>

#include "access_token_helper.h"
#include "bundle_manager_helper.h"
#include "common_event_constant.h"
#include "event_log_wrapper.h"
#include "event_report.h"
// #include "hitrace_meter.h"
#include "ievent_receive.h"
#include "system_time.h"
#include "xcollie/watchdog.h"

namespace OHOS {
namespace EventFwk {
constexpr int32_t LENGTH = 80;
constexpr int32_t DOUBLE = 2;
const std::string CONNECTOR = " or ";

CommonEventControlManager::CommonEventControlManager()
    : handler_(nullptr), handlerOrdered_(nullptr), pendingTimeoutMessage_(false), scheduled_(false)
{
    EVENT_LOGD("enter");
}

CommonEventControlManager::~CommonEventControlManager()
{
    EVENT_LOGD("enter");
}

bool CommonEventControlManager::PublishCommonEvent(
    const CommonEventRecord &eventRecord, const sptr<IRemoteObject> &commonEventListener)
{
    // HITRACE_METER_NAME(HITRACE_TAG_NOTIFICATION, __PRETTY_FUNCTION__);
    EVENT_LOGI("enter");

    bool ret = false;

    if (!eventRecord.publishInfo->IsOrdered()) {
        ret = ProcessUnorderedEvent(eventRecord);
    } else {
        ret = ProcessOrderedEvent(eventRecord, commonEventListener);
    }

    return ret;
}

bool CommonEventControlManager::PublishStickyCommonEvent(
    const CommonEventRecord &eventRecord, const std::shared_ptr<EventSubscriberRecord> &subscriberRecord)
{
    // HITRACE_METER_NAME(HITRACE_TAG_NOTIFICATION, __PRETTY_FUNCTION__);
    EVENT_LOGI("enter");

    if (!subscriberRecord) {
        EVENT_LOGE("subscriberRecord is null");
        return false;
    }
    return ProcessUnorderedEvent(eventRecord, subscriberRecord);
}

bool CommonEventControlManager::PublishFreezeCommonEvent(const uid_t &uid)
{
    // HITRACE_METER_NAME(HITRACE_TAG_NOTIFICATION, __PRETTY_FUNCTION__);
    EVENT_LOGI("enter");

    if (!GetUnorderedEventHandler()) {
        EVENT_LOGE("failed to get eventhandler");
        return false;
    }
    PublishFrozenEventsInner(DelayedSingleton<CommonEventSubscriberManager>::GetInstance()->GetFrozenEvents(uid));
    return true;
}

bool CommonEventControlManager::PublishAllFreezeCommonEvents()
{
    // HITRACE_METER_NAME(HITRACE_TAG_NOTIFICATION, __PRETTY_FUNCTION__);
    EVENT_LOGI("enter");

    if (!GetUnorderedEventHandler()) {
        EVENT_LOGE("failed to get eventhandler");
        return false;
    }

    std::map<uid_t, FrozenRecords> frozenEventRecords =
        DelayedSingleton<CommonEventSubscriberManager>::GetInstance()->GetAllFrozenEvents();
    for (auto record : frozenEventRecords) {
        PublishFrozenEventsInner(record.second);
    }
    return true;
}

void CommonEventControlManager::PublishFrozenEventsInner(const FrozenRecords &frozenRecords)
{
    for (auto record : frozenRecords) {
        for (auto vec : record.second) {
            if (!record.first || !vec) {
                EVENT_LOGW("failed to find record");
                continue;
            }

            EventSubscriberRecord subscriberRecord = *(record.first);
            CommonEventRecord eventRecord = *vec;
            std::weak_ptr<CommonEventControlManager> weak = shared_from_this();
            auto innerCallback = [weak, subscriberRecord, eventRecord]() {
                auto control = weak.lock();
                if (control == nullptr) {
                    EVENT_LOGE("CommonEventControlManager is null");
                    return;
                }
                control->NotifyFreezeEvents(subscriberRecord, eventRecord);
            };

            handler_->PostImmediateTask(innerCallback);
        }
    }
}

bool CommonEventControlManager::NotifyFreezeEvents(
    const EventSubscriberRecord &subscriberRecord, const CommonEventRecord &eventRecord)
{
    EVENT_LOGI("enter");

    sptr<IEventReceive> commonEventListenerProxy = iface_cast<IEventReceive>(subscriberRecord.commonEventListener);
    if (!commonEventListenerProxy) {
        EVENT_LOGE("Fail to get IEventReceive proxy");
        return false;
    }

    int8_t ret = CheckPermission(subscriberRecord, eventRecord);
    if (ret != OrderedEventRecord::DELIVERED) {
        EVENT_LOGE("check permission is failed");
        return false;
    }
    if (eventRecord.commonEventData == nullptr) {
        EVENT_LOGE("commonEventData == nullptr");
        return false;
    }
    EVENT_LOGI("Send common event %{public}s to subscriber %{public}s (pid = %{public}d, uid = %{public}d) "
                "when unfreezed",
        eventRecord.commonEventData->GetWant().GetAction().c_str(),
        subscriberRecord.eventRecordInfo.bundleName.c_str(),
        subscriberRecord.eventRecordInfo.pid,
        subscriberRecord.eventRecordInfo.uid);
    commonEventListenerProxy->NotifyEvent(*(eventRecord.commonEventData),
        false, eventRecord.publishInfo->IsSticky());
    AccessTokenHelper::RecordSensitivePermissionUsage(subscriberRecord.eventRecordInfo.callerToken,
        eventRecord.commonEventData->GetWant().GetAction());
    return true;
}

bool CommonEventControlManager::GetUnorderedEventHandler()
{
    if (!handler_) {
        handler_ = std::make_shared<EventHandler>(EventRunner::Create("CesSrvUnorderEventHandler"));
        if (!handler_) {
            EVENT_LOGE("Failed to create UnorderedEventHandler");
            return false;
        }
    }
    if (handler_->GetEventRunner() != nullptr) {
        std::string threadName = handler_->GetEventRunner()->GetRunnerThreadName();
        if (HiviewDFX::Watchdog::GetInstance().AddThread(threadName, handler_) != 0) {
            EVENT_LOGE("Failed to Add handler Thread");
        }
    }
    return true;
}

bool CommonEventControlManager::NotifyUnorderedEvent(std::shared_ptr<OrderedEventRecord> &eventRecord)
{
    // HITRACE_METER_NAME(HITRACE_TAG_NOTIFICATION, __PRETTY_FUNCTION__);
    EVENT_LOGI("enter");
    if (!eventRecord) {
        EVENT_LOGI("Invalid event record.");
        return false;
    }
    std::lock_guard<std::mutex> lock(unorderedMutex_);
    EVENT_LOGI("event = %{public}s, receivers size = %{public}zu",
        eventRecord->commonEventData->GetWant().GetAction().c_str(), eventRecord->receivers.size());
    for (auto vec : eventRecord->receivers) {
        if (vec == nullptr) {
            EVENT_LOGE("invalid vec");
            continue;
        }
        size_t index = eventRecord->nextReceiver++;
        eventRecord->curReceiver = vec->commonEventListener;
        if (vec->isFreeze) {
            eventRecord->deliveryState[index] = OrderedEventRecord::SKIPPED;
            DelayedSingleton<CommonEventSubscriberManager>::GetInstance()->InsertFrozenEvents(vec, *eventRecord);
        } else {
            sptr<IEventReceive> commonEventListenerProxy = iface_cast<IEventReceive>(vec->commonEventListener);
            if (!commonEventListenerProxy) {
                eventRecord->deliveryState[index] = OrderedEventRecord::SKIPPED;
                EVENT_LOGE("Failed to get IEventReceive proxy");
                continue;
            }
            int8_t ret = CheckPermission(*vec, *eventRecord);
            eventRecord->deliveryState[index] = ret;
            if (ret == OrderedEventRecord::DELIVERED) {
                eventRecord->state = OrderedEventRecord::RECEIVING;
                commonEventListenerProxy->NotifyEvent(
                    *(eventRecord->commonEventData), false, eventRecord->publishInfo->IsSticky());
                eventRecord->state = OrderedEventRecord::RECEIVED;
                AccessTokenHelper::RecordSensitivePermissionUsage(vec->eventRecordInfo.callerToken,
                    eventRecord->commonEventData->GetWant().GetAction());
            }
        }
    }

    EnqueueHistoryEventRecord(eventRecord, false);

    auto it = std::find(unorderedEventQueue_.begin(), unorderedEventQueue_.end(), eventRecord);
    if (it != unorderedEventQueue_.end()) {
        unorderedEventQueue_.erase(it);
    }
    return true;
}

bool CommonEventControlManager::ProcessUnorderedEvent(
    const CommonEventRecord &eventRecord, const std::shared_ptr<EventSubscriberRecord> &subscriberRecord)
{
    // HITRACE_METER_NAME(HITRACE_TAG_NOTIFICATION, __PRETTY_FUNCTION__);
    EVENT_LOGI("enter");

    bool ret = false;

    if (!GetUnorderedEventHandler()) {
        EVENT_LOGE("failed to get eventhandler");
        return ret;
    }

    std::shared_ptr<OrderedEventRecord> eventRecordPtr = std::make_shared<OrderedEventRecord>();
    if (eventRecordPtr == nullptr) {
        EVENT_LOGE("eventRecordPtr is null");
        return ret;
    }

    std::shared_ptr<CommonEventSubscriberManager> spinstance =
        DelayedSingleton<CommonEventSubscriberManager>::GetInstance();

    eventRecordPtr->FillCommonEventRecord(eventRecord);
    if (subscriberRecord) {
        eventRecordPtr->receivers.emplace_back(subscriberRecord);
    } else {
        eventRecordPtr->receivers = spinstance->GetSubscriberRecords(eventRecord);
    }

    for (auto vec : eventRecordPtr->receivers) {
        eventRecordPtr->deliveryState.emplace_back(OrderedEventRecord::PENDING);
    }

    EnqueueUnorderedRecord(eventRecordPtr);

    std::weak_ptr<CommonEventControlManager> weak = shared_from_this();
    auto innerCallback = [weak, eventRecordPtr]() {
        auto manager = weak.lock();
        if (manager == nullptr) {
            EVENT_LOGE("CommonEventControlManager is null");
            return;
        }
        std::shared_ptr<OrderedEventRecord> ordered = eventRecordPtr;
        manager->NotifyUnorderedEvent(ordered);
    };

    if (eventRecord.isSystemEvent) {
        ret = handler_->PostImmediateTask(innerCallback);
    } else {
        ret = handler_->PostTask(innerCallback);
    }

    return ret;
}

std::shared_ptr<OrderedEventRecord> CommonEventControlManager::GetMatchingOrderedReceiver(
    const sptr<IRemoteObject> &proxy)
{
    EVENT_LOGI("enter");

    std::lock_guard<std::mutex> lock(orderedMutex_);

    if (!orderedEventQueue_.empty()) {
        std::shared_ptr<OrderedEventRecord> firstRecord = orderedEventQueue_.front();
        if ((firstRecord != nullptr) && (firstRecord->curReceiver == proxy)) {
            return firstRecord;
        }
    }

    return nullptr;
}

bool CommonEventControlManager::GetOrderedEventHandler()
{
    if (!handlerOrdered_) {
        handlerOrdered_ = std::make_shared<OrderedEventHandler>(
            EventRunner::Create("CesSrvOrderEventHandler"), shared_from_this());
        if (!handlerOrdered_) {
            EVENT_LOGE("Failed to create OrderedEventHandler");
            return false;
        }
    }
    if (handlerOrdered_->GetEventRunner() != nullptr) {
        std::string threadName = handlerOrdered_->GetEventRunner()->GetRunnerThreadName();
        if (HiviewDFX::Watchdog::GetInstance().AddThread(threadName, handlerOrdered_) != 0) {
            EVENT_LOGE("Failed to Add Ordered Thread");
        }
    }
    return true;
}

bool CommonEventControlManager::ProcessOrderedEvent(
    const CommonEventRecord &eventRecord, const sptr<IRemoteObject> &commonEventListener)
{
    // HITRACE_METER_NAME(HITRACE_TAG_NOTIFICATION, __PRETTY_FUNCTION__);
    EVENT_LOGI("enter");

    bool ret = false;

    if (!GetOrderedEventHandler()) {
        EVENT_LOGE("failed to get eventhandler");
        return ret;
    }

    std::shared_ptr<OrderedEventRecord> eventRecordPtr = std::make_shared<OrderedEventRecord>();
    if (eventRecordPtr == nullptr) {
        EVENT_LOGE("eventRecordPtr is null");
        return ret;
    }

    std::shared_ptr<CommonEventSubscriberManager> spinstance =
        DelayedSingleton<CommonEventSubscriberManager>::GetInstance();

    eventRecordPtr->FillCommonEventRecord(eventRecord);
    eventRecordPtr->resultTo = commonEventListener;
    eventRecordPtr->state = OrderedEventRecord::IDLE;
    eventRecordPtr->nextReceiver = 0;
    eventRecordPtr->receivers = spinstance->GetSubscriberRecords(eventRecord);
    for (auto vec : eventRecordPtr->receivers) {
        eventRecordPtr->deliveryState.emplace_back(OrderedEventRecord::PENDING);
    }

    EnqueueOrderedRecord(eventRecordPtr);

    ret = ScheduleOrderedCommonEvent();

    return ret;
}

bool CommonEventControlManager::EnqueueUnorderedRecord(const std::shared_ptr<OrderedEventRecord> &eventRecordPtr)
{
    if (eventRecordPtr == nullptr) {
        EVENT_LOGE("eventRecordPtr is null");
        return false;
    }

    std::lock_guard<std::mutex> lock(unorderedMutex_);

    unorderedEventQueue_.emplace_back(eventRecordPtr);

    return true;
}

bool CommonEventControlManager::EnqueueOrderedRecord(const std::shared_ptr<OrderedEventRecord> &eventRecordPtr)
{
    if (eventRecordPtr == nullptr) {
        EVENT_LOGE("eventRecordPtr is null");
        return false;
    }

    std::lock_guard<std::mutex> lock(orderedMutex_);

    orderedEventQueue_.emplace_back(eventRecordPtr);

    return true;
}

void CommonEventControlManager::EnqueueHistoryEventRecord(
    const std::shared_ptr<OrderedEventRecord> &eventRecordPtr, bool hasLastSubscribe)
{
    EVENT_LOGI("enter");

    if (eventRecordPtr == nullptr) {
        EVENT_LOGE("eventRecordPtr is nullptr");
        return;
    }

    HistoryEventRecord record;
    record.want = eventRecordPtr->commonEventData->GetWant();
    record.code = eventRecordPtr->commonEventData->GetCode();
    record.data = eventRecordPtr->commonEventData->GetData();

    record.sticky = eventRecordPtr->publishInfo->IsSticky();
    record.ordered = eventRecordPtr->publishInfo->IsOrdered();
    record.subscriberPermissions = eventRecordPtr->publishInfo->GetSubscriberPermissions();

    record.recordTime = eventRecordPtr->recordTime;
    record.pid = eventRecordPtr->eventRecordInfo.pid;
    record.uid = eventRecordPtr->eventRecordInfo.uid;
    record.userId = eventRecordPtr->userId;
    record.bundleName = eventRecordPtr->eventRecordInfo.bundleName;
    record.isSystemApp = eventRecordPtr->eventRecordInfo.isSystemApp;
    record.isSystemEvent = eventRecordPtr->isSystemEvent;

    for (auto vec : eventRecordPtr->receivers) {
        if (vec == nullptr) {
            continue;
        }
        HistorySubscriberRecord receiver;
        receiver.recordTime = vec->recordTime;
        receiver.bundleName = vec->eventRecordInfo.bundleName;
        receiver.priority = vec->eventSubscribeInfo->GetPriority();
        receiver.userId = vec->eventSubscribeInfo->GetUserId();
        receiver.permission = vec->eventSubscribeInfo->GetPermission();
        receiver.deviceId = vec->eventSubscribeInfo->GetDeviceId();
        receiver.isFreeze = vec->isFreeze;
        receiver.freezeTime = vec->freezeTime;
        record.receivers.emplace_back(receiver);
    }

    record.hasLastSubscribe = hasLastSubscribe;
    record.deliveryState = eventRecordPtr->deliveryState;
    record.dispatchTime = eventRecordPtr->dispatchTime;
    record.receiverTime = eventRecordPtr->receiverTime;
    record.state = eventRecordPtr->state;
    record.resultAbort = eventRecordPtr->resultAbort;

    std::lock_guard<std::mutex> lock(historyMutex_);
    if (historyEventRecords_.size() == HISTORY_MAX_SIZE) {
        historyEventRecords_.erase(historyEventRecords_.begin());
    }
    historyEventRecords_.emplace_back(record);
}

bool CommonEventControlManager::ScheduleOrderedCommonEvent()
{
    EVENT_LOGI("enter");

    if (scheduled_) {
        return true;
    }

    scheduled_ = true;

    return handlerOrdered_->SendEvent(InnerEvent::Get(OrderedEventHandler::ORDERED_EVENT_START));
}

bool CommonEventControlManager::NotifyOrderedEvent(std::shared_ptr<OrderedEventRecord> &eventRecordPtr, size_t index)
{
    // HITRACE_METER_NAME(HITRACE_TAG_NOTIFICATION, __PRETTY_FUNCTION__);
    EVENT_LOGI("enter with index %{public}zu", index);
    if (eventRecordPtr == nullptr) {
        EVENT_LOGE("eventRecordPtr = nullptr");
        return false;
    }
    size_t receiverNum = eventRecordPtr->receivers.size();
    if ((index < 0) || (index >= receiverNum)) {
        EVENT_LOGE("Invalid index (= %{public}zu)", index);
        return false;
    }
    int8_t ret = CheckPermission(*(eventRecordPtr->receivers[index]), *eventRecordPtr);
    if (ret == OrderedEventRecord::SKIPPED) {
        eventRecordPtr->deliveryState[index] = ret;
        return true;
    }
    if (ret == OrderedEventRecord::DELIVERED) {
        if (eventRecordPtr->receivers[index]->isFreeze) {
            EVENT_LOGI("vec isFreeze: %{public}d", eventRecordPtr->receivers[index]->isFreeze);
            DelayedSingleton<CommonEventSubscriberManager>::GetInstance()->InsertFrozenEvents(
                eventRecordPtr->receivers[index], *eventRecordPtr);
            eventRecordPtr->deliveryState[index] = OrderedEventRecord::SKIPPED;
            eventRecordPtr->curReceiver = nullptr;
            return true;
        }
        eventRecordPtr->deliveryState[index] = ret;
        eventRecordPtr->curReceiver = eventRecordPtr->receivers[index]->commonEventListener;
        eventRecordPtr->state = OrderedEventRecord::RECEIVING;
        sptr<IEventReceive> receiver = iface_cast<IEventReceive>(eventRecordPtr->curReceiver);
        if (!receiver) {
            EVENT_LOGE("Failed to get IEventReceive proxy");
            eventRecordPtr->curReceiver = nullptr;
            return false;
        }
        eventRecordPtr->state = OrderedEventRecord::RECEIVED;
        EVENT_LOGI("NotifyOrderedEvent event = %{public}s",
            eventRecordPtr->commonEventData->GetWant().GetAction().c_str());
        receiver->NotifyEvent(*(eventRecordPtr->commonEventData), true, eventRecordPtr->publishInfo->IsSticky());
        AccessTokenHelper::RecordSensitivePermissionUsage(
            eventRecordPtr->receivers[index]->eventRecordInfo.callerToken,
            eventRecordPtr->commonEventData->GetWant().GetAction());
    }
    return true;
}

void CommonEventControlManager::ProcessNextOrderedEvent(bool isSendMsg)
{
    // HITRACE_METER_NAME(HITRACE_TAG_NOTIFICATION, __PRETTY_FUNCTION__);
    EVENT_LOGI("enter");

    if (isSendMsg) {
        scheduled_ = false;
    }

    std::shared_ptr<OrderedEventRecord> sp = nullptr;

    std::lock_guard<std::mutex> lock(orderedMutex_);

    do {
        if (orderedEventQueue_.empty()) {
            EVENT_LOGE("orderedEventQueue_ is empty");
            return;
        }

        sp = orderedEventQueue_.front();
        bool forceReceive = false;
        size_t numReceivers = sp->receivers.size();
        uint64_t nowSysTime = static_cast<uint64_t>(SystemTime::GetNowSysTime());

        if (sp->dispatchTime > 0) {
            if ((numReceivers > 0) && (nowSysTime > static_cast<uint64_t>(sp->dispatchTime) +
                (DOUBLE * TIMEOUT * numReceivers))) {
                CurrentOrderedEventTimeout(false);
                forceReceive = true;
                sp->state = OrderedEventRecord::IDLE;
            }
        }

        if (sp->state != OrderedEventRecord::IDLE) {
            return;
        }

        if ((sp->receivers.empty()) || (sp->nextReceiver >= numReceivers) || sp->resultAbort || forceReceive) {
            // No more receivers for this ordered common event, then process the final result receiver
            bool hasLastSubscribe = (sp->resultTo != nullptr) ? true : false;
            if (sp->resultTo != nullptr) {
                EVENT_LOGI("Process the final subscriber");
                sptr<IEventReceive> receiver = iface_cast<IEventReceive>(sp->resultTo);
                if (!receiver) {
                    EVENT_LOGE("Failed to get IEventReceive proxy");
                    return;
                }
                receiver->NotifyEvent(*(sp->commonEventData), true, sp->publishInfo->IsSticky());
                sp->resultTo = nullptr;
            }

            CancelTimeout();

            EnqueueHistoryEventRecord(sp, hasLastSubscribe);

            orderedEventQueue_.erase(orderedEventQueue_.begin());

            sp = nullptr;
        }
    } while (sp == nullptr);

    size_t recIdx = sp->nextReceiver++;
    SetTime(recIdx, sp, pendingTimeoutMessage_);

    NotifyOrderedEvent(sp, recIdx);
    if (sp->curReceiver == nullptr) {
        sp->state = OrderedEventRecord::IDLE;
        ScheduleOrderedCommonEvent();
    }
}

void CommonEventControlManager::SetTime(size_t recIdx, std::shared_ptr<OrderedEventRecord> &sp, bool timeoutMessage)
{
    EVENT_LOGI("enter");

    sp->receiverTime = SystemTime::GetNowSysTime();

    if (recIdx == 0) {
        sp->dispatchTime = sp->receiverTime;
    }

    if (!timeoutMessage) {
        int64_t timeoutTime = sp->receiverTime + TIMEOUT;
        SetTimeout(timeoutTime);
    }
}

bool CommonEventControlManager::SetTimeout(int64_t timeoutTime)
{
    EVENT_LOGI("enter");

    bool ret = true;

    if (!pendingTimeoutMessage_) {
        pendingTimeoutMessage_ = true;
        ret = handlerOrdered_->SendTimingEvent(OrderedEventHandler::ORDERED_EVENT_TIMEOUT, timeoutTime);
    }

    return ret;
}

bool CommonEventControlManager::CancelTimeout()
{
    EVENT_LOGI("enter");

    if (pendingTimeoutMessage_) {
        pendingTimeoutMessage_ = false;
        handlerOrdered_->RemoveEvent(OrderedEventHandler::ORDERED_EVENT_TIMEOUT);
    }

    return true;
}

void CommonEventControlManager::CurrentOrderedEventTimeout(bool isFromMsg)
{
    EVENT_LOGI("enter");

    if (isFromMsg) {
        pendingTimeoutMessage_ = false;
    }

    if (orderedEventQueue_.empty()) {
        EVENT_LOGE("empty orderedEventQueue_");
        return;
    }

    int64_t nowSysTime = SystemTime::GetNowSysTime();
    std::shared_ptr<OrderedEventRecord> sp = orderedEventQueue_.front();

    if (isFromMsg) {
        int64_t timeoutTime = sp->receiverTime + TIMEOUT;
        if (timeoutTime > nowSysTime) {
            SetTimeout(timeoutTime);
            return;
        }
    }

    // The processing of current receiver has timeout
    sp->receiverTime = nowSysTime;

    if (sp->nextReceiver > 0) {
        std::shared_ptr<EventSubscriberRecord> subscriberRecord = sp->receivers[sp->nextReceiver - 1];
        EVENT_LOGW("Timeout: When %{public}s (pid = %{public}d, uid = %{public}d) process common event %{public}s",
            subscriberRecord->eventRecordInfo.bundleName.c_str(),
            subscriberRecord->eventRecordInfo.pid,
            subscriberRecord->eventRecordInfo.uid,
            sp->commonEventData->GetWant().GetAction().c_str());
        SendOrderedEventProcTimeoutHiSysEvent(subscriberRecord, sp->commonEventData->GetWant().GetAction());

        sp->deliveryState[sp->nextReceiver - 1] = OrderedEventRecord::TIMEOUT;
    }

    // Forced to finish the current receiver to process the next receiver
    int32_t code = sp->commonEventData->GetCode();
    const std::string &strRef = sp->commonEventData->GetData();
    bool abort = sp->resultAbort;
    FinishReceiver(sp, code, strRef, abort);

    ScheduleOrderedCommonEvent();

    return;
}

bool CommonEventControlManager::FinishReceiver(std::shared_ptr<OrderedEventRecord> recordPtr, const int32_t &code,
    const std::string &receiverData, const bool &abortEvent)
{
    EVENT_LOGI("enter");

    if (recordPtr == nullptr) {
        EVENT_LOGE("recordPtr is null");
        return false;
    }

    EVENT_LOGI("enter recordPtr->state=%{public}d", recordPtr->state);

    int8_t state = recordPtr->state;
    recordPtr->state = OrderedEventRecord::IDLE;
    recordPtr->curReceiver = nullptr;
    recordPtr->commonEventData->SetCode(code);
    recordPtr->commonEventData->SetData(receiverData);
    recordPtr->resultAbort = abortEvent;

    return state == OrderedEventRecord::RECEIVED;
}

bool CommonEventControlManager::FinishReceiverAction(std::shared_ptr<OrderedEventRecord> recordPtr, const int32_t &code,
    const std::string &receiverData, const bool &abortEvent)
{
    EVENT_LOGI("enter");

    if (recordPtr == nullptr) {
        EVENT_LOGE("recordPtr is nullptr");
        return false;
    }

    bool doNext = false;
    doNext = FinishReceiver(recordPtr, code, receiverData, abortEvent);
    if (doNext) {
        ProcessNextOrderedEvent(false);
    }

    return true;
}

int8_t CommonEventControlManager::CheckPermission(
    const EventSubscriberRecord &subscriberRecord, const CommonEventRecord &eventRecord)
{
    EVENT_LOGI("enter");

    bool ret = false;
    ret = CheckSubscriberPermission(subscriberRecord, eventRecord);
    if (!ret) {
        return OrderedEventRecord::SKIPPED;
    }

    std::string subscriberRequiredPermission = subscriberRecord.eventSubscribeInfo->GetPermission();
    ret = CheckSubscriberRequiredPermission(subscriberRequiredPermission, eventRecord, subscriberRecord);
    if (!ret) {
        return OrderedEventRecord::SKIPPED;
    }

    std::vector<std::string> publisherRequiredPermissions = eventRecord.publishInfo->GetSubscriberPermissions();
    ret = CheckPublisherRequiredPermissions(publisherRequiredPermissions, subscriberRecord, eventRecord);
    if (!ret) {
        return OrderedEventRecord::SKIPPED;
    }

    return OrderedEventRecord::DELIVERED;
}

bool CommonEventControlManager::CheckSubscriberPermission(
    const EventSubscriberRecord &subscriberRecord, const CommonEventRecord &eventRecord)
{
    EVENT_LOGI("enter");
    bool ret = false;
    std::string lackPermission {};

    Permission permission = DelayedSingleton<CommonEventPermissionManager>::GetInstance()->GetEventPermission(
        eventRecord.commonEventData->GetWant().GetAction());
    if (permission.names.empty()) {
        return true;
    }

    if (!subscriberRecord.eventRecordInfo.isProxy && (subscriberRecord.eventRecordInfo.isSubsystem
        || subscriberRecord.eventRecordInfo.isSystemApp)) {
        return true;
    }

    if (permission.names.size() == 1) {
        ret = AccessTokenHelper::VerifyAccessToken(subscriberRecord.eventRecordInfo.callerToken, permission.names[0]);
        lackPermission = permission.names[0];
    } else {
        if (permission.state == PermissionState::AND) {
            for (auto vec : permission.names) {
                ret = AccessTokenHelper::VerifyAccessToken(subscriberRecord.eventRecordInfo.callerToken, vec);
                if (!ret) {
                    lackPermission = vec;
                    break;
                }
            }
        } else if (permission.state == PermissionState::OR) {
            for (auto vec : permission.names) {
                ret = AccessTokenHelper::VerifyAccessToken(subscriberRecord.eventRecordInfo.callerToken, vec);
                lackPermission += vec + CONNECTOR;
                if (ret) {
                    break;
                }
            }
            lackPermission = lackPermission.substr(0, lackPermission.length() - CONNECTOR.length());
        } else {
            EVENT_LOGW("Invalid Permission.");
            return false;
        }
    }
    if (!ret) {
        EVENT_LOGW("No permission to receive common event %{public}s, "
                    "due to subscriber %{public}s (pid = %{public}d, uid = %{public}d) lacks "
                    "the %{public}s permission.",
            eventRecord.commonEventData->GetWant().GetAction().c_str(),
            subscriberRecord.eventRecordInfo.bundleName.c_str(),
            subscriberRecord.eventRecordInfo.pid,
            subscriberRecord.eventRecordInfo.uid,
            lackPermission.c_str());
    }

    return ret;
}

bool CommonEventControlManager::CheckSubscriberRequiredPermission(const std::string &subscriberRequiredPermission,
    const CommonEventRecord &eventRecord, const EventSubscriberRecord &subscriberRecord)
{
    bool ret = false;

    if (subscriberRequiredPermission.empty()) {
        return true;
    }

    ret = AccessTokenHelper::VerifyAccessToken(eventRecord.eventRecordInfo.callerToken, subscriberRequiredPermission);
    if (!ret) {
        EVENT_LOGW("No permission to send common event %{public}s "
                    "from %{public}s (pid = %{public}d, uid = %{public}d), userId = %{public}d "
                    "to %{public}s (pid = %{public}d, uid = %{public}d), userId = %{public}d "
                    "due to registered subscriber requires the %{public}s permission.",
            eventRecord.commonEventData->GetWant().GetAction().c_str(),
            eventRecord.eventRecordInfo.bundleName.c_str(),
            eventRecord.eventRecordInfo.pid,
            eventRecord.eventRecordInfo.uid,
            eventRecord.userId,
            subscriberRecord.eventRecordInfo.bundleName.c_str(),
            subscriberRecord.eventRecordInfo.pid,
            subscriberRecord.eventRecordInfo.uid,
            subscriberRecord.eventSubscribeInfo->GetUserId(),
            subscriberRequiredPermission.c_str());
    }

    return ret;
}

bool CommonEventControlManager::CheckPublisherRequiredPermissions(
    const std::vector<std::string> &publisherRequiredPermissions, const EventSubscriberRecord &subscriberRecord,
    const CommonEventRecord &eventRecord)
{
    bool ret = false;

    if (publisherRequiredPermissions.empty()) {
        return true;
    }

    for (auto publisherRequiredPermission : publisherRequiredPermissions) {
        ret = AccessTokenHelper::VerifyAccessToken(
            subscriberRecord.eventRecordInfo.callerToken, publisherRequiredPermission);
        if (!ret) {
            EVENT_LOGW("No permission to receive common event %{public}s "
                        "to %{public}s (pid = %{public}d, uid = %{public}d), userId = %{public}d "
                        "due to publisher %{public}s (pid = %{public}d, uid = %{public}d),"
                        " userId = %{public}d requires the %{public}s permission.",
                eventRecord.commonEventData->GetWant().GetAction().c_str(),
                subscriberRecord.eventRecordInfo.bundleName.c_str(),
                subscriberRecord.eventRecordInfo.pid,
                subscriberRecord.eventRecordInfo.uid,
                subscriberRecord.eventSubscribeInfo->GetUserId(),
                eventRecord.eventRecordInfo.bundleName.c_str(),
                eventRecord.eventRecordInfo.pid,
                eventRecord.eventRecordInfo.uid,
                eventRecord.userId,
                publisherRequiredPermission.c_str());
            break;
        }
    }

    return ret;
}

void CommonEventControlManager::GetUnorderedEventRecords(
    const std::string &event, const int32_t &userId, std::vector<std::shared_ptr<OrderedEventRecord>> &records)
{
    EVENT_LOGI("enter");
    if (event.empty() && userId == ALL_USER) {
        records = unorderedEventQueue_;
    } else if (event.empty()) {
        for (auto vec : unorderedEventQueue_) {
            if (vec->userId == userId) {
                records.emplace_back(vec);
            }
        }
    } else if (userId == ALL_USER) {
        for (auto vec : unorderedEventQueue_) {
            if (vec->commonEventData->GetWant().GetAction() == event) {
                records.emplace_back(vec);
            }
        }
    } else {
        for (auto vec : unorderedEventQueue_) {
            if (vec->commonEventData->GetWant().GetAction() == event && vec->userId == userId) {
                records.emplace_back(vec);
            }
        }
    }
}

void CommonEventControlManager::GetOrderedEventRecords(
    const std::string &event, const int32_t &userId, std::vector<std::shared_ptr<OrderedEventRecord>> &records)
{
    EVENT_LOGI("enter");
    if (event.empty() && userId == ALL_USER) {
        records = orderedEventQueue_;
    } else if (event.empty()) {
        for (auto vec : orderedEventQueue_) {
            if (vec->userId == userId) {
                records.emplace_back(vec);
            }
        }
    } else if (userId == ALL_USER) {
        for (auto vec : orderedEventQueue_) {
            if (vec->commonEventData->GetWant().GetAction() == event) {
                records.emplace_back(vec);
            }
        }
    } else {
        for (auto vec : orderedEventQueue_) {
            if (vec->commonEventData->GetWant().GetAction() == event && vec->userId == userId) {
                records.emplace_back(vec);
            }
        }
    }
}

void CommonEventControlManager::GetHistoryEventRecords(
    const std::string &event, const int32_t &userId, std::vector<HistoryEventRecord> &records)
{
    EVENT_LOGI("enter");
    if (event.empty() && userId == ALL_USER) {
        records = historyEventRecords_;
    } else if (event.empty()) {
        for (auto vec : historyEventRecords_) {
            if (vec.userId == userId) {
                records.emplace_back(vec);
            }
        }
    } else if (userId == ALL_USER) {
        for (auto vec : historyEventRecords_) {
            if (vec.want.GetAction() == event) {
                records.emplace_back(vec);
            }
        }
    } else {
        for (auto vec : historyEventRecords_) {
            if (vec.want.GetAction() == event && vec.userId == userId) {
                records.emplace_back(vec);
            }
        }
    }
}

void CommonEventControlManager::DumpStateByCommonEventRecord(
    const std::shared_ptr<OrderedEventRecord> &record, std::string &dumpInfo)
{
    EVENT_LOGI("enter");

    char systime[LENGTH];
    strftime(systime, sizeof(char) * LENGTH, "%Y%m%d %I:%M %p", &record->recordTime);

    std::string recordTime = "\tTime: " + std::string(systime) + "\n";
    std::string pid = "\tPID: " + std::to_string(record->eventRecordInfo.pid) + "\n";
    std::string uid = "\tUID: " + std::to_string(record->eventRecordInfo.uid) + "\n";
    std::string userId;
    switch (record->userId) {
        case UNDEFINED_USER:
            userId = "UNDEFINED_USER";
            break;
        case ALL_USER:
            userId = "ALL_USER";
            break;
        default:
            userId = std::to_string(record->userId);
            break;
    }
    userId = "\tUSERID: " + userId + "\n";
    std::string bundleName = "\tBundleName: " + record->eventRecordInfo.bundleName + "\n";

    std::string permission = "\tRequiredPermission: ";
    std::string separator;
    size_t permissionNum = 0;
    for (auto permissionVec : record->publishInfo->GetSubscriberPermissions()) {
        if (permissionNum == 0) {
            separator = "";
        } else {
            separator = ", ";
        }
        permission = permission + separator + permissionVec;
        permissionNum++;
    }
    permission = permission + "\n";

    std::string isSticky;
    if (record->publishInfo->IsSticky()) {
        isSticky = "\tIsSticky: true\n";
    } else {
        isSticky = "\tIsSticky: false\n";
    }

    std::string isOrdered;
    if (record->publishInfo->IsOrdered()) {
        isOrdered = "\tIsOrdered: true\n";
    } else {
        isOrdered = "\tIsOrdered: false\n";
    }
    std::string isSystemApp = record->eventRecordInfo.isSystemApp ? "true" : "false";
    isSystemApp = "\tIsSystemApp: " + isSystemApp + "\n";
    std::string isSystemEvent = record->isSystemEvent ? "true" : "false";
    isSystemEvent = "\tIsSystemEvent: " + isSystemEvent + "\n";

    std::string action = "\t\tAction: " + record->commonEventData->GetWant().GetAction() + "\n";

    std::string entities = "\t\tEntity: ";
    size_t entityNum = 0;
    for (auto entitiesVec : record->commonEventData->GetWant().GetEntities()) {
        if (entityNum == 0) {
            separator = "";
        } else {
            separator = ", ";
        }
        entities = entities + separator + entitiesVec;
        entityNum++;
    }
    entities = entities + "\n";

    std::string scheme = "\t\tScheme: " + record->commonEventData->GetWant().GetScheme() + "\n";
    std::string uri = "\t\tUri: " + record->commonEventData->GetWant().GetUriString() + "\n";
    std::string flags = "\t\tFlags: " + std::to_string(record->commonEventData->GetWant().GetFlags()) + "\n";
    std::string type = "\t\tType: " + record->commonEventData->GetWant().GetType() + "\n";
    std::string bundle = "\t\tBundleName: " + record->commonEventData->GetWant().GetBundle() + "\n";
    std::string ability = "\t\tAbilityName: " + record->commonEventData->GetWant().GetElement().GetAbilityName() + "\n";
    std::string deviced = "\t\tDevicedID: " + record->commonEventData->GetWant().GetElement().GetDeviceID() + "\n";

    std::string want = "\tWant:\n" + action + entities + scheme + uri + flags + type + bundle + ability + deviced;
    std::string code = "\tCode: " + std::to_string(record->commonEventData->GetCode()) + "\n";
    std::string data = "\tData: " + record->commonEventData->GetData() + "\n";

    std::string lastSubscriber;
    if (record->resultTo) {
        lastSubscriber = "\tHasLastSubscriber: true\n";
    } else {
        lastSubscriber = "\tHasLastSubscriber: false\n";
    }

    std::string state;
    switch (record->state) {
        case OrderedEventRecord::IDLE:
            state = "\tEventState: IDLE\n";
            break;
        case OrderedEventRecord::RECEIVING:
            state = "\tEventState: RECEIVING\n";
            break;
        case OrderedEventRecord::RECEIVED:
            state = "\tEventState: RECEIVED\n";
            break;
    }

    std::string dispatchTime = "\tDispatchTime: " + std::to_string(record->dispatchTime) + "\n";
    std::string receiverTime = "\tReceiverTime: " + std::to_string(record->receiverTime) + "\n";
    std::string resultAbort = record->resultAbort ? "true" : "false";
    resultAbort = "\tResultAbort: " + resultAbort + "\n";

    dumpInfo = recordTime + pid + uid + userId + bundleName + permission + isSticky + isOrdered + isSystemApp +
               isSystemEvent + want + code + data + lastSubscriber + state + receiverTime + dispatchTime + resultAbort;
}

void CommonEventControlManager::DumpHistoryStateByCommonEventRecord(
    const HistoryEventRecord &record, std::string &dumpInfo)
{
    EVENT_LOGI("enter");

    char systime[LENGTH];
    strftime(systime, sizeof(char) * LENGTH, "%Y%m%d %I:%M %p", &record.recordTime);

    std::string recordTime = "\tTime: " + std::string(systime) + "\n";
    std::string pid = "\tPID: " + std::to_string(record.pid) + "\n";
    std::string uid = "\tUID: " + std::to_string(record.uid) + "\n";
    std::string userId;
    switch (record.userId) {
        case UNDEFINED_USER:
            userId = "UNDEFINED_USER";
            break;
        case ALL_USER:
            userId = "ALL_USER";
            break;
        default:
            userId = std::to_string(record.userId);
            break;
    }
    userId = "\tUSERID: " + userId + "\n";
    std::string bundleName = "\tBundleName: " + record.bundleName + "\n";

    std::string permission = "\tRequiredPermission: ";
    std::string separator;
    size_t permissionNum = 0;
    for (auto permissionVec : record.subscriberPermissions) {
        if (permissionNum == 0) {
            separator = "";
        } else {
            separator = ", ";
        }
        permission = permission + separator + permissionVec;
        permissionNum++;
    }
    permission = permission + "\n";

    std::string isSticky;
    if (record.sticky) {
        isSticky = "\tIsSticky: true\n";
    } else {
        isSticky = "\tIsSticky: false\n";
    }

    std::string isOrdered;
    if (record.ordered) {
        isOrdered = "\tIsOrdered: true\n";
    } else {
        isOrdered = "\tIsOrdered: false\n";
    }
    std::string isSystemApp = record.isSystemApp ? "true" : "false";
    isSystemApp = "\tIsSystemApp: " + isSystemApp + "\n";
    std::string isSystemEvent = record.isSystemEvent ? "true" : "false";
    isSystemEvent = "\tIsSystemEvent: " + isSystemEvent + "\n";

    std::string action = "\t\tAction: " + record.want.GetAction() + "\n";

    std::string entities = "\t\tEntity: ";
    size_t entityNum = 0;
    for (auto entitiesVec : record.want.GetEntities()) {
        if (entityNum == 0) {
            separator = "";
        } else {
            separator = ", ";
        }
        entities = entities + separator + entitiesVec;
        entityNum++;
    }
    entities = entities + "\n";

    std::string scheme = "\t\tScheme: " + record.want.GetScheme() + "\n";
    std::string uri = "\t\tUri: " + record.want.GetUriString() + "\n";
    std::string flags = "\t\tFlags: " + std::to_string(record.want.GetFlags()) + "\n";
    std::string type = "\t\tType: " + record.want.GetType() + "\n";
    std::string bundle = "\t\tBundleName: " + record.want.GetBundle() + "\n";
    std::string ability = "\t\tAbilityName: " + record.want.GetElement().GetAbilityName() + "\n";
    std::string deviced = "\t\tDevicedID: " + record.want.GetElement().GetDeviceID() + "\n";

    std::string want = "\tWant:\n" + action + entities + scheme + uri + flags + type + bundle + ability + deviced;
    std::string code = "\tCode: " + std::to_string(record.code) + "\n";
    std::string data = "\tData: " + record.data + "\n";

    std::string lastSubscriber;
    if (record.hasLastSubscribe) {
        lastSubscriber = "\tHasLastSubscriber: true\n";
    } else {
        lastSubscriber = "\tHasLastSubscriber: false\n";
    }

    std::string state;
    switch (record.state) {
        case OrderedEventRecord::IDLE:
            state = "\tEventState: IDLE\n";
            break;
        case OrderedEventRecord::RECEIVING:
            state = "\tEventState: RECEIVING\n";
            break;
        case OrderedEventRecord::RECEIVED:
            state = "\tEventState: RECEIVED\n";
            break;
    }

    std::string dispatchTime = "\tDispatchTime: " + std::to_string(record.dispatchTime) + "\n";
    std::string receiverTime = "\tReceiverTime: " + std::to_string(record.receiverTime) + "\n";
    std::string resultAbort = record.resultAbort ? "true" : "false";
    resultAbort = "\tResultAbort: " + resultAbort + "\n";

    dumpInfo = recordTime + pid + uid + userId + bundleName + permission + isSticky + isOrdered + isSystemApp +
               isSystemEvent + want + code + data + lastSubscriber + state + receiverTime + dispatchTime + resultAbort;
}

void CommonEventControlManager::DumpStateBySubscriberRecord(
    const std::shared_ptr<OrderedEventRecord> &record, std::string &dumpInfo)
{
    EVENT_LOGI("enter");

    if (record->receivers.empty()) {
        dumpInfo = "\tSubscribers:\tNo information";
        return;
    }

    size_t num = 0;
    for (auto receiver : record->receivers) {
        num++;

        std::string title = std::to_string(num);
        if (num == 1) {
            title = "\tSubscribers:\tTotal " + std::to_string(record->receivers.size()) + " subscribers\n\tNO " +
                    title + "\n";
        } else {
            title = "\tNO " + title + "\n";
        }

        std::string dumpInfoBySubscriber;
        DelayedSingleton<CommonEventSubscriberManager>::GetInstance()->DumpDetailed(
            title, receiver, "\t\t", dumpInfoBySubscriber);

        std::string deliveryState;
        switch (record->deliveryState[num - 1]) {
            case OrderedEventRecord::PENDING:
                deliveryState = "\t\tEventState: PENDING\n";
                break;
            case OrderedEventRecord::DELIVERED:
                deliveryState = "\t\tEventState: DELIVERED\n";
                break;
            case OrderedEventRecord::SKIPPED:
                deliveryState = "\t\tEventState: SKIPPED\n";
                break;
            case OrderedEventRecord::TIMEOUT:
                deliveryState = "\t\tEventState: TIMEOUT\n";
                break;
        }
        dumpInfo = dumpInfo + dumpInfoBySubscriber + deliveryState;
    }
}

void CommonEventControlManager::DumpHistoryStateBySubscriberRecord(
    const HistoryEventRecord &record, std::string &dumpInfo)
{
    EVENT_LOGI("enter");

    if (record.receivers.empty()) {
        dumpInfo = "\tSubscribers:\tNo information";
        return;
    }

    size_t num = 0;
    for (auto receiver : record.receivers) {
        num++;

        std::string title = std::to_string(num);
        if (num == 1) {
            title = "\tSubscribers:\tTotal " + std::to_string(record.receivers.size()) +
                " subscribers\n\tNO " + title + "\n";
        } else {
            title = "\tNO " + title + "\n";
        }

        char systime[LENGTH];
        strftime(systime, sizeof(char) * LENGTH, "%Y%m%d %I:%M %p", &receiver.recordTime);
        std::string format = "\t\t";
        std::string recordTime = format + "Time: " + std::string(systime) + "\n";

        std::string bundleName = format + "BundleName: " + receiver.bundleName + "\n";
        std::string priority = format + "Priority: " + std::to_string(receiver.priority) + "\n";
        std::string userId;
        switch (receiver.userId) {
            case UNDEFINED_USER:
                userId = "UNDEFINED_USER";
                break;
            case ALL_USER:
                userId = "ALL_USER";
                break;
            default:
                userId = std::to_string(receiver.userId);
                break;
        }
        userId = format + "USERID: " + userId + "\n";
        std::string permission = format + "Permission: " + receiver.permission + "\n";
        std::string deviceId = format + "DevicedID: " + receiver.deviceId + "\n";

        std::string isFreeze = receiver.isFreeze ? "true" : "false";
        isFreeze = format + "IsFreeze: " + isFreeze + "\n";

        std::string freezeTime;
        if (receiver.freezeTime == 0) {
            freezeTime = format + "FreezeTime:  -\n";
        } else {
            freezeTime = format + "FreezeTime: " + std::to_string(receiver.freezeTime) + "\n";
        }

        std::string deliveryState;
        switch (record.deliveryState[num - 1]) {
            case OrderedEventRecord::PENDING:
                deliveryState = "\t\tEventState: PENDING\n";
                break;
            case OrderedEventRecord::DELIVERED:
                deliveryState = "\t\tEventState: DELIVERED\n";
                break;
            case OrderedEventRecord::SKIPPED:
                deliveryState = "\t\tEventState: SKIPPED\n";
                break;
            case OrderedEventRecord::TIMEOUT:
                deliveryState = "\t\tEventState: TIMEOUT\n";
                break;
        }
        dumpInfo = dumpInfo + title + recordTime + bundleName + priority + userId + permission + deviceId + isFreeze +
                   freezeTime + deliveryState;
    }
}

void CommonEventControlManager::DumpState(
    const std::string &event, const int32_t &userId, std::vector<std::string> &state)
{
    EVENT_LOGI("enter");

    std::vector<std::shared_ptr<OrderedEventRecord>> records;
    std::vector<std::shared_ptr<OrderedEventRecord>> unorderedRecords;
    std::vector<std::shared_ptr<OrderedEventRecord>> orderedRecords;
    std::lock_guard<std::mutex> orderedLock(orderedMutex_);
    std::lock_guard<std::mutex> unorderedLock(unorderedMutex_);
    GetUnorderedEventRecords(event, userId, unorderedRecords);
    GetOrderedEventRecords(event, userId, orderedRecords);
    records.insert(records.end(), unorderedRecords.begin(), unorderedRecords.end());
    records.insert(records.end(), orderedRecords.begin(), orderedRecords.end());

    if (records.empty()) {
        state.emplace_back("Pending Events:\tNo information");
        return;
    }

    size_t num = 0;
    for (auto record : records) {
        num++;

        std::string no = std::to_string(num);
        if (num == 1) {
            no = "Pending Events:\tTotal " + std::to_string(records.size()) + " information\nNO " + no + "\n";
        } else {
            no = "NO " + no + "\n";
        }
        std::string commonEventRecord;
        DumpStateByCommonEventRecord(record, commonEventRecord);
        std::string subscriberRecord;
        DumpStateBySubscriberRecord(record, subscriberRecord);
        std::string stateInfo = no + commonEventRecord + subscriberRecord;
        state.emplace_back(stateInfo);
    }
}

void CommonEventControlManager::DumpHistoryState(
    const std::string &event, const int32_t &userId, std::vector<std::string> &state)
{
    EVENT_LOGI("enter");

    std::vector<HistoryEventRecord> records;
    std::lock_guard<std::mutex> lock(historyMutex_);
    GetHistoryEventRecords(event, userId, records);

    if (records.empty()) {
        state.emplace_back("History Events:\tNo information");
        return;
    }

    size_t num = 0;
    for (auto record : records) {
        num++;

        std::string no = std::to_string(num);
        if (num == 1) {
            no = "History Events:\tTotal " + std::to_string(records.size()) + " information\nNO " + no + "\n";
        } else {
            no = "NO " + no + "\n";
        }
        std::string commonEventRecord;
        DumpHistoryStateByCommonEventRecord(record, commonEventRecord);
        std::string subscriberRecord;
        DumpHistoryStateBySubscriberRecord(record, subscriberRecord);
        std::string stateInfo = no + commonEventRecord + subscriberRecord;
        state.emplace_back(stateInfo);
    }
}

void CommonEventControlManager::SendOrderedEventProcTimeoutHiSysEvent(
    const std::shared_ptr<EventSubscriberRecord> &subscriberRecord, const std::string &eventName)
{
    if (subscriberRecord == nullptr) {
        return;
    }

    EventInfo eventInfo;
    if (subscriberRecord->eventSubscribeInfo != nullptr) {
        eventInfo.userId = subscriberRecord->eventSubscribeInfo->GetUserId();
    }
    eventInfo.subscriberName = subscriberRecord->eventRecordInfo.bundleName;
    eventInfo.pid = subscriberRecord->eventRecordInfo.pid;
    eventInfo.uid = static_cast<int32_t>(subscriberRecord->eventRecordInfo.uid);
    eventInfo.eventName = eventName;
    EventReport::SendHiSysEvent(ORDERED_EVENT_PROC_TIMEOUT, eventInfo);
}
}  // namespace EventFwk
}  // namespace OHOS