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

#include "common_event_manager_service.h"

#include "access_token_helper.h"
#include "accesstoken_kit.h"
#include "bundle_manager_helper.h"
#include "common_event_constant.h"
#include "datetime_ex.h"
#include "event_log_wrapper.h"
// #include "hitrace_meter.h"
#include "ipc_skeleton.h"
#include "publish_manager.h"
#include "system_ability_definition.h"
#include "xcollie/watchdog.h"
#include "ces_inner_error_code.h"

namespace OHOS {
namespace EventFwk {
using namespace OHOS::Notification;

CommonEventManagerService::CommonEventManagerService()
    : serviceRunningState_(ServiceRunningState::STATE_NOT_START),
      runner_(nullptr),
      handler_(nullptr)
{
    EVENT_LOGI("instance created");
}

CommonEventManagerService::~CommonEventManagerService()
{
    EVENT_LOGI("instance destroyed");
}

ErrCode CommonEventManagerService::Init()
{
    EVENT_LOGI("ready to init");
    innerCommonEventManager_ = std::make_shared<InnerCommonEventManager>();
    if (!innerCommonEventManager_) {
        EVENT_LOGE("Failed to init without inner service");
        return ERR_INVALID_OPERATION;
    }

    runner_ = EventRunner::Create("CesSrvMain");
    if (!runner_) {
        EVENT_LOGE("Failed to init due to create runner error");
        return ERR_INVALID_OPERATION;
    }
    handler_ = std::make_shared<EventHandler>(runner_);
    if (!handler_) {
        EVENT_LOGE("Failed to init due to create handler error");
        return ERR_INVALID_OPERATION;
    }
    if (handler_->GetEventRunner() != nullptr) {
        std::string threadName = handler_->GetEventRunner()->GetRunnerThreadName();
        if (HiviewDFX::Watchdog::GetInstance().AddThread(threadName, handler_) != 0) {
            EVENT_LOGE("Failed to Add main Thread");
        }
    }

    serviceRunningState_ = ServiceRunningState::STATE_RUNNING;

    return ERR_OK;
}

bool CommonEventManagerService::IsReady() const
{
    if (!innerCommonEventManager_) {
        EVENT_LOGE("innerCommonEventManager is null");
        return false;
    }

    if (!handler_) {
        EVENT_LOGE("handler is null");
        return false;
    }

    return true;
}

int32_t CommonEventManagerService::PublishCommonEvent(const CommonEventData &event,
    const CommonEventPublishInfo &publishinfo, const sptr<IRemoteObject> &commonEventListener,
    const int32_t &userId)
{
    EVENT_LOGI("enter");

    if (!IsReady()) {
        EVENT_LOGE("CommonEventManagerService not ready");
        return ERR_NOTIFICATION_CESM_ERROR;
    }

    return PublishCommonEventDetailed(event,
        publishinfo,
        commonEventListener,
        IPCSkeleton::GetCallingPid(),
        IPCSkeleton::GetCallingUid(),
        IPCSkeleton::GetCallingTokenID(),
        userId);
}

bool CommonEventManagerService::PublishCommonEvent(const CommonEventData &event,
    const CommonEventPublishInfo &publishinfo, const sptr<IRemoteObject> &commonEventListener, const uid_t &uid,
    const int32_t &callerToken, const int32_t &userId)
{
    EVENT_LOGI("enter");

    if (!IsReady()) {
        EVENT_LOGE("CommonEventManagerService not ready");
        return false;
    }

    if (!AccessTokenHelper::VerifyNativeToken(IPCSkeleton::GetCallingTokenID())) {
        EVENT_LOGE("Only sa can publish common event as proxy.");
        return false;
    }

    return PublishCommonEventDetailed(
        event, publishinfo, commonEventListener, UNDEFINED_PID, uid, callerToken, userId) == ERR_OK ? true : false;
}

int32_t CommonEventManagerService::PublishCommonEventDetailed(const CommonEventData &event,
    const CommonEventPublishInfo &publishinfo, const sptr<IRemoteObject> &commonEventListener, const pid_t &pid,
    const uid_t &uid, const int32_t &clientToken, const int32_t &userId)
{
    // HITRACE_METER_NAME(HITRACE_TAG_NOTIFICATION, __PRETTY_FUNCTION__);
    EVENT_LOGI("enter");

    EVENT_LOGI("clientToken = %{public}d", clientToken);
    if (AccessTokenHelper::IsDlpHap(clientToken)) {
        EVENT_LOGE("DLP hap not allowed to send common event");
        return ERR_NOTIFICATION_CES_NOT_SA_SYSTEM_APP;
    }
    struct tm recordTime = {0};
    if (!GetSystemCurrentTime(&recordTime)) {
        EVENT_LOGE("Failed to GetSystemCurrentTime");
        return ERR_NOTIFICATION_SYS_ERROR;
    }

    std::string bundleName = DelayedSingleton<BundleManagerHelper>::GetInstance()->GetBundleName(uid);

    if (DelayedSingleton<PublishManager>::GetInstance()->CheckIsFloodAttack(uid)) {
        EVENT_LOGE("Too many common events have been sent in a short period from %{public}s (pid = %{public}d, uid = "
                   "%{public}d, userId = %{public}d)",
            bundleName.c_str(),
            pid,
            uid,
            userId);
        return ERR_NOTIFICATION_CES_COMMON_PARAM_INVALID;
    }

    std::weak_ptr<InnerCommonEventManager> wp = innerCommonEventManager_;
    wptr<CommonEventManagerService> weakThis = this;
    std::function<void()> publishCommonEventFunc = [wp,
        event,
        publishinfo,
        commonEventListener,
        recordTime,
        pid,
        uid,
        clientToken,
        userId,
        bundleName,
        weakThis] () {
        std::shared_ptr<InnerCommonEventManager> innerCommonEventManager = wp.lock();
        if (innerCommonEventManager == nullptr) {
            EVENT_LOGE("innerCommonEventManager not exist");
            return;
        }
        sptr<CommonEventManagerService> commonEventManagerService = weakThis.promote();
        if (commonEventManagerService == nullptr) {
            EVENT_LOGE("CommonEventManager not exist");
            return;
        }
        bool ret = innerCommonEventManager->PublishCommonEvent(event,
            publishinfo,
            commonEventListener,
            recordTime,
            pid,
            uid,
            clientToken,
            userId,
            bundleName,
            commonEventManagerService);
        if (!ret) {
            EVENT_LOGE("failed to publish event %{public}s", event.GetWant().GetAction().c_str());
        }
    };
    return handler_->PostTask(publishCommonEventFunc) ? ERR_OK : ERR_NOTIFICATION_CESM_ERROR;
}

int32_t CommonEventManagerService::SubscribeCommonEvent(
    const CommonEventSubscribeInfo &subscribeInfo, const sptr<IRemoteObject> &commonEventListener)
{
    // HITRACE_METER_NAME(HITRACE_TAG_NOTIFICATION, __PRETTY_FUNCTION__);
    EVENT_LOGI("enter");

    if (!IsReady()) {
        EVENT_LOGE("CommonEventManagerService not ready");
        return ERR_NOTIFICATION_CES_COMMON_PARAM_INVALID;
    }

    struct tm recordTime = {0};
    if (!GetSystemCurrentTime(&recordTime)) {
        EVENT_LOGE("Failed to GetSystemCurrentTime");
        return ERR_NOTIFICATION_CES_COMMON_PARAM_INVALID;
    }

    auto callingUid = IPCSkeleton::GetCallingUid();
    auto callingPid = IPCSkeleton::GetCallingPid();
    std::string bundleName = DelayedSingleton<BundleManagerHelper>::GetInstance()->GetBundleName(callingUid);
    Security::AccessToken::AccessTokenID callerToken = IPCSkeleton::GetCallingTokenID();
    std::weak_ptr<InnerCommonEventManager> wp = innerCommonEventManager_;
    std::function<void()> subscribeCommonEventFunc = [wp,
        subscribeInfo,
        commonEventListener,
        recordTime,
        callingPid,
        callingUid,
        callerToken,
        bundleName] () {
        std::shared_ptr<InnerCommonEventManager> innerCommonEventManager = wp.lock();
        if (innerCommonEventManager == nullptr) {
            EVENT_LOGE("innerCommonEventManager not exist");
            return;
        }
        bool ret = innerCommonEventManager->SubscribeCommonEvent(subscribeInfo,
            commonEventListener,
            recordTime,
            callingPid,
            callingUid,
            callerToken,
            bundleName);
        if (!ret) {
            EVENT_LOGE("failed to subscribe event");
        }
    };
    return handler_->PostTask(subscribeCommonEventFunc) ? ERR_OK : ERR_NOTIFICATION_CESM_ERROR;
}

int32_t CommonEventManagerService::UnsubscribeCommonEvent(const sptr<IRemoteObject> &commonEventListener)
{
    // HITRACE_METER_NAME(HITRACE_TAG_NOTIFICATION, __PRETTY_FUNCTION__);
    EVENT_LOGI("enter");

    if (!IsReady()) {
        EVENT_LOGE("CommonEventManagerService not ready");
        return ERR_NOTIFICATION_CES_COMMON_PARAM_INVALID;
    }

    std::weak_ptr<InnerCommonEventManager> wp = innerCommonEventManager_;
    std::function<void()> unsubscribeCommonEventFunc = [wp, commonEventListener] () {
        std::shared_ptr<InnerCommonEventManager> innerCommonEventManager = wp.lock();
        if (innerCommonEventManager == nullptr) {
            EVENT_LOGE("innerCommonEventManager not exist");
            return;
        }
        bool ret = innerCommonEventManager->UnsubscribeCommonEvent(commonEventListener);
        if (!ret) {
            EVENT_LOGE("failed to unsubscribe event");
        }
    };
    
    return handler_->PostTask(unsubscribeCommonEventFunc) ? ERR_OK : ERR_NOTIFICATION_CESM_ERROR;
}

bool CommonEventManagerService::GetStickyCommonEvent(const std::string &event, CommonEventData &eventData)
{
    EVENT_LOGI("enter");

    if (!IsReady()) {
        EVENT_LOGE("CommonEventManagerService not ready");
        return false;
    }

    if (event.empty()) {
        EVENT_LOGE("event is empty");
        return false;
    }

    auto callingUid = IPCSkeleton::GetCallingUid();
    std::string bundleName = DelayedSingleton<BundleManagerHelper>::GetInstance()->GetBundleName(callingUid);
    const std::string permission = "ohos.permission.COMMONEVENT_STICKY";
    bool ret = AccessTokenHelper::VerifyAccessToken(IPCSkeleton::GetCallingTokenID(), permission);
    if (!ret) {
        EVENT_LOGE("No permission to get a sticky common event from %{public}s (uid = %{public}d)",
            bundleName.c_str(),
            callingUid);
        return false;
    }

    return innerCommonEventManager_->GetStickyCommonEvent(event, eventData);
}

bool CommonEventManagerService::DumpState(const uint8_t &dumpType, const std::string &event, const int32_t &userId,
    std::vector<std::string> &state)
{
    EVENT_LOGI("enter");

    if (!AccessTokenHelper::VerifyNativeToken(IPCSkeleton::GetCallingTokenID())) {
        EVENT_LOGE("Not subsystem or shell request");
        return false;
    }
    if (!IsReady()) {
        EVENT_LOGE("CommonEventManagerService not ready");
        return false;
    }

    innerCommonEventManager_->DumpState(dumpType, event, userId, state);

    return true;
}

bool CommonEventManagerService::FinishReceiver(
    const sptr<IRemoteObject> &proxy, const int32_t &code, const std::string &receiverData, const bool &abortEvent)
{
    EVENT_LOGI("enter");

    if (!IsReady()) {
        EVENT_LOGE("CommonEventManagerService not ready");
        return false;
    }
    std::weak_ptr<InnerCommonEventManager> wp = innerCommonEventManager_;
    std::function<void()> finishReceiverFunc = [wp, proxy, code, receiverData, abortEvent] () {
        std::shared_ptr<InnerCommonEventManager> innerCommonEventManager = wp.lock();
        if (innerCommonEventManager == nullptr) {
            EVENT_LOGE("innerCommonEventManager not exist");
            return;
        }
        innerCommonEventManager->FinishReceiver(proxy, code, receiverData, abortEvent);
    };
    
    return handler_->PostTask(finishReceiverFunc);
}

bool CommonEventManagerService::Freeze(const uid_t &uid)
{
    EVENT_LOGI("enter");

    if (!AccessTokenHelper::VerifyNativeToken(IPCSkeleton::GetCallingTokenID())) {
        EVENT_LOGE("Not subsystem request");
        return false;
    }
    if (!IsReady()) {
        EVENT_LOGE("CommonEventManagerService not ready");
        return false;
    }
    std::weak_ptr<InnerCommonEventManager> wp = innerCommonEventManager_;
    std::function<void()> freezeFunc = [wp, uid] () {
        std::shared_ptr<InnerCommonEventManager> innerCommonEventManager = wp.lock();
        if (innerCommonEventManager == nullptr) {
            EVENT_LOGE("innerCommonEventManager not exist");
            return;
        }
        innerCommonEventManager->Freeze(uid);
    };
    return handler_->PostImmediateTask(freezeFunc);
}

bool CommonEventManagerService::Unfreeze(const uid_t &uid)
{
    EVENT_LOGI("enter");

    if (!AccessTokenHelper::VerifyNativeToken(IPCSkeleton::GetCallingTokenID())) {
        EVENT_LOGE("Not subsystem request");
        return false;
    }
    if (!IsReady()) {
        EVENT_LOGE("CommonEventManagerService not ready");
        return false;
    }

    std::weak_ptr<InnerCommonEventManager> wp = innerCommonEventManager_;
    std::function<void()> unfreezeFunc = [wp, uid] () {
        std::shared_ptr<InnerCommonEventManager> innerCommonEventManager = wp.lock();
        if (innerCommonEventManager == nullptr) {
            EVENT_LOGE("innerCommonEventManager not exist");
            return;
        }
        innerCommonEventManager->Unfreeze(uid);
    };
    
    return handler_->PostImmediateTask(unfreezeFunc);
}

bool CommonEventManagerService::UnfreezeAll()
{
    EVENT_LOGI("enter");

    if (!AccessTokenHelper::VerifyNativeToken(IPCSkeleton::GetCallingTokenID())) {
        EVENT_LOGE("Not subsystem request");
        return false;
    }
    if (!IsReady()) {
        EVENT_LOGE("CommonEventManagerService not ready");
        return false;
    }

    std::weak_ptr<InnerCommonEventManager> wp = innerCommonEventManager_;
    std::function<void()> unfreezeAllFunc = [wp] () {
        std::shared_ptr<InnerCommonEventManager> innerCommonEventManager = wp.lock();
        if (innerCommonEventManager == nullptr) {
            EVENT_LOGE("innerCommonEventManager not exist");
            return;
        }
        innerCommonEventManager->UnfreezeAll();
    };

    return handler_->PostImmediateTask(unfreezeAllFunc);
}

int CommonEventManagerService::Dump(int fd, const std::vector<std::u16string> &args)
{
    EVENT_LOGI("enter");

    if (!AccessTokenHelper::VerifyNativeToken(IPCSkeleton::GetCallingTokenID())) {
        EVENT_LOGE("Not subsystem or shell request");
        return false;
    }
    if (!IsReady()) {
        EVENT_LOGE("CommonEventManagerService not ready");
        return ERR_INVALID_VALUE;
    }
    std::string result;
    innerCommonEventManager_->HiDump(args, result);
    int ret = dprintf(fd, "%s\n", result.c_str());
    if (ret < 0) {
        EVENT_LOGE("dprintf error");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}
}  // namespace EventFwk
}  // namespace OHOS
