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

#include "multiuser/os_account_observer.h"

// #include "common_event_manager.h"
// #include "common_event_support.h"
#include "device/device_manager_agent.h"
#include "dfsu_mount_argument_descriptors.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace std;
namespace {
static const std::string SAME_ACCOUNT = "account";
static const std::string ACCOUNT_LESS = "non_account";
static constexpr int DEFAULT_ACCOUNT = 100;
} // namespace

OsAccountObserver::OsAccountObserver()
{
    LOGI("init first to create network of default user");
    lock_guard<mutex> lock(serializer_);
    curUsrId = DEFAULT_ACCOUNT;
    AddMPInfo(curUsrId, SAME_ACCOUNT);
    AddMPInfo(curUsrId, ACCOUNT_LESS);
    LOGI("init first to create network of user %{public}d, done", DEFAULT_ACCOUNT);
}

OsAccountObserver::~OsAccountObserver()
{
}

void OsAccountObserver::AddMPInfo(const int id, const std::string &relativePath)
{
    auto smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(id, relativePath));
    auto dm = DeviceManagerAgent::GetInstance();
    dm->Recv(make_unique<DfsuCmd<DeviceManagerAgent, weak_ptr<MountPoint>>>(&DeviceManagerAgent::JoinGroup, smp));
    mountPoints_[id].emplace_back(smp);
}

// void OsAccountObserver::OnReceiveEvent(const EventFwk::CommonEventData &eventData)
// {
//     const AAFwk::Want& want = eventData.GetWant();
//     std::string action = want.GetAction();
//     LOGI("AccountSubscriber: OnReceiveEvent action:%{public}s.", action.c_str());
//     if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED) {
//         int32_t id = eventData.GetCode();
//         LOGI("user id changed to %{public}d", id);
//         lock_guard<mutex> lock(serializer_);
//         if (curUsrId != -1 && curUsrId != id) {
//             // first stop curUsrId network
//             RemoveMPInfo(curUsrId);
//         }

//         // then start new network
//         curUsrId = id;
//         AddMPInfo(id, SAME_ACCOUNT);
//         AddMPInfo(id, ACCOUNT_LESS);
//         LOGI("user id %{public}d, add network done", curUsrId);
//     }
// }

void OsAccountObserver::RemoveMPInfo(const int id)
{
    auto iter = mountPoints_.find(id);
    if (iter == mountPoints_.end()) {
        LOGE("user id %{public}d not find in map", curUsrId);
        return;
    }

    auto dm = DeviceManagerAgent::GetInstance();
    for (auto smp : iter->second) {
        dm->Recv(make_unique<DfsuCmd<DeviceManagerAgent, weak_ptr<MountPoint>>>(&DeviceManagerAgent::QuitGroup, smp));
    }
    mountPoints_.erase(iter);

    LOGE("remove mount info of user id %{public}d", id);
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
