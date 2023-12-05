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

#include "ipc/daemon.h"

#include <exception>
#include <stdexcept>

// #include "common_event_manager.h"
// #include "common_event_support.h"
#include "iremote_object.h"
#include "mountpoint/mount_manager.h"
#include "system_ability_definition.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace std;

REGISTER_SYSTEM_ABILITY_BY_ID(Daemon, FILEMANAGEMENT_DISTRIBUTED_FILE_DAEMON_SA_ID, true);

void Daemon::PublishSA()
{
    LOGI("Begin to init");
    if (!registerToService_) {
        bool ret = SystemAbility::Publish(this);
        if (!ret) {
            throw runtime_error("Failed to publish the daemon");
        }
        registerToService_ = true;
    }
    LOGI("Init finished successfully");
}

void Daemon::RegisterOsAccount()
{
    // EventFwk::MatchingSkills matchingSkills;
    // matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
    // EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subScriber_ = std::make_shared<OsAccountObserver>();
    // bool subRet = EventFwk::CommonEventManager::SubscribeCommonEvent(subScriber_);
    // if (!subRet) {
    //     LOGE("Subscribe common event failed");
    // }
}

void Daemon::OnStart()
{
    LOGI("Begin to start service");
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        LOGD("Daemon has already started");
        return;
    }

    try {
        PublishSA();
        // AddSystemAbilityListener(COMMON_EVENT_SERVICE_ID);
        AddSystemAbilityListener(FILEMANAGEMENT_DISTRIBUTED_FILE_DAEMON_SA_ID);
    } catch (const exception &e) {
        LOGE("%{public}s", e.what());
    }

    state_ = ServiceRunningState::STATE_RUNNING;
    LOGI("Start service successfully");
}

void Daemon::OnStop()
{
    LOGI("Begin to stop");
    state_ = ServiceRunningState::STATE_NOT_START;
    registerToService_ = false;
    // bool subRet = EventFwk::CommonEventManager::UnSubscribeCommonEvent(subScriber_);
    // if (!subRet) {
    //     LOGE("UnSubscribe common event failed");
    // }
    subScriber_ = nullptr;
    LOGI("Stop finished successfully");
}

void Daemon::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    (void)systemAbilityId;
    (void)deviceId;
    RegisterOsAccount();
}

void Daemon::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    (void)deviceId;
    if (systemAbilityId != COMMON_EVENT_SERVICE_ID) {
        LOGE("systemAbilityId is not COMMON_EVENT_SERVICE_ID");
        return;
    }

    if (subScriber_ == nullptr) {
        LOGE("Daemon::OnRemoveSystemAbility subscriberPtr is nullptr");
        return;
    }

    // bool subscribeResult = EventFwk::CommonEventManager::UnSubscribeCommonEvent(subScriber_);
    LOGI("Daemon::OnRemoveSystemAbility");
    subScriber_ = nullptr;
}

int32_t Daemon::EchoServerDemo(const string &echoStr)
{
    (void)echoStr;
    return 0;
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
