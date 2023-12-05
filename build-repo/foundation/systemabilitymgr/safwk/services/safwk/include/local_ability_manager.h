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

#ifndef LOCAL_ABILITY_MANAGER_H
#define LOCAL_ABILITY_MANAGER_H

#include <string>
#include <map>
#include <list>
#include <unistd.h>
#include <condition_variable>
#include <shared_mutex>
#include "local_ability_manager_stub.h"
#include "system_ability.h"
#include "thread_pool.h"
#include "parse_util.h"
#include "single_instance.h"
#include "system_ability_status_change_stub.h"

namespace OHOS {
// Check all dependencies's availability before the timeout period ended, [200, 60000].
const int32_t MIN_DEPENDENCY_TIMEOUT = 200;
const int32_t MAX_DEPENDENCY_TIMEOUT = 60000;
const int32_t DEFAULT_DEPENDENCY_TIMEOUT = 6000;

class LocalAbilityManager : public LocalAbilityManagerStub {
    DECLARE_SINGLE_INSTANCE_BASE(LocalAbilityManager);

public:
    bool AddAbility(SystemAbility* ability);
    bool RemoveAbility(int32_t systemAbilityId);
    SystemAbility* GetAbility(int32_t systemAbilityId);
    bool GetRunningStatus(int32_t systemAbilityId);
    bool AddSystemAbilityListener(int32_t systemAbilityId, int32_t listenerSaId);
    bool RemoveSystemAbilityListener(int32_t systemAbilityId, int32_t listenerSaId);
    std::vector<std::u16string> CheckDependencyStatus(const std::vector<std::u16string>& dependSas);
    void StartSystemAbilityTask(SystemAbility* sa);
    bool CheckSystemAbilityManagerReady();
    bool InitSystemAbilityProfiles(const std::string& profilePath, int32_t saId);
    void ClearResource();
    void StartOndemandSystemAbility(int32_t systemAbilityId);

    bool StartAbility(int32_t systemAbilityId) override;
    void DoStartSAProcess(const std::string& profilePath, int32_t saId);
private:
    LocalAbilityManager();
    ~LocalAbilityManager();

    bool AddLocalAbilityManager();
    void RegisterOnDemandSystemAbility(int32_t saId);
    void FindAndStartPhaseTasks();
    void StartPhaseTasks(const std::list<SystemAbility*>& startTasks);
    void CheckTrustSa(const std::string& path, const std::string& process, const std::list<SaProfile>& saInfos);
    sptr<ISystemAbilityStatusChange> GetSystemAbilityStatusChange();
    void FindAndNotifyAbilityListeners(int32_t systemAbilityId, const std::string& deviceId, int32_t code);
    void NotifyAbilityListener(int32_t systemAbilityId, int32_t listenerSaId,
        const std::string& deviceId, int32_t code);

    class SystemAbilityListener : public SystemAbilityStatusChangeStub {
    public:
        void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
        void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    };

    bool CheckAndGetProfilePath(const std::string& profilePath, std::string& realProfilePath);
    bool InitializeSaProfiles(int32_t saId);
    bool InitializeRunOnCreateSaProfiles();
    bool InitializeOnDemandSaProfile(int32_t saId);
    bool InitializeSaProfilesInnerLocked(const SaProfile& saProfile);
    bool Run(int32_t saId);
    bool NeedRegisterOnDemand(const SaProfile& saProfile, int32_t saId);
    bool OnStartAbility(int32_t systemAbilityId);
    std::string GetTraceTag(const std::string& profilePath);

    std::map<int32_t, SystemAbility*> abilityMap_;
    std::map<uint32_t, std::list<SystemAbility*>> abilityPhaseMap_;
    std::shared_mutex abilityMapLock_;
    sptr<LocalAbilityManager> localAbilityManager_;

    // Max task number in pool is 20.
    const int32_t MAX_TASK_NUMBER = 20;
    // Check dependent sa status every 50 ms, it equals 50000us.
    const int32_t CHECK_DEPENDENT_SA_PERIOD = 50000;

    sptr<ISystemAbilityStatusChange> statusChangeListener_;
    std::map<int32_t, std::list<int32_t>> listenerMap_;
    std::mutex listenerLock_;

    std::shared_ptr<ParseUtil> profileParser_;

    std::condition_variable startPhaseCV_;
    std::mutex startPhaseLock_;
    int32_t startTaskNum_ = 0;
    std::u16string procName_;

    // Thread pool used to start system abilities in parallel.
    std::unique_ptr<ThreadPool> initPool_;
    // Thread pool used to start ondemand system abilities in parallel.
    std::unique_ptr<ThreadPool> ondemandPool_;
};
}
#endif
