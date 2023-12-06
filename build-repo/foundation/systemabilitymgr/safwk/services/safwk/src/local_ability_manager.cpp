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

#include "local_ability_manager.h"

#include <chrono>
#include <cinttypes>
#include <iostream>
#include <sys/types.h>

#include "datetime_ex.h"
#include "errors.h"
// #include "hitrace_meter.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "safwk_log.h"
#include "string_ex.h"

namespace OHOS {
using std::u16string;
using std::string;
using std::vector;

namespace {
const string TAG = "LocalAbilityManager";

constexpr int32_t RETRY_TIMES_FOR_ONDEMAND = 10;
constexpr int32_t RETRY_TIMES_FOR_SAMGR = 50;
constexpr int32_t DEFAULT_SAID = -1;
constexpr std::chrono::milliseconds MILLISECONDS_WAITING_SAMGR_ONE_TIME(200);
constexpr std::chrono::milliseconds MILLISECONDS_WAITING_ONDEMAND_ONE_TIME(100);

const u16string BOOT_START_PHASE = u"BootStartPhase";
const u16string CORE_START_PHASE = u"CoreStartPhase";
constexpr int32_t MAX_SA_STARTUP_TIME = 100;
constexpr int32_t SUFFIX_LENGTH = 4; // .xml length

const string PROFILES_DIR = "/system/profile/";
const string DEFAULT_DIR = "/system/usr/";
const string PREFIX = PROFILES_DIR;
const string SUFFIX = "_trust.xml";

const string ONDEMAND_POOL = "SaOndemand";
const string INIT_POOL = "SaInit";

enum {
    BOOT_START = 1,
    CORE_START = 2,
    OTHER_START = 3,
};
}

IMPLEMENT_SINGLE_INSTANCE(LocalAbilityManager);

LocalAbilityManager::LocalAbilityManager()
{
    profileParser_ = std::make_shared<ParseUtil>();
    initPool_ = std::make_unique<ThreadPool>(INIT_POOL);
    ondemandPool_ = std::make_unique<ThreadPool>(ONDEMAND_POOL);
    ondemandPool_->Start(std::thread::hardware_concurrency());
    ondemandPool_->SetMaxTaskNum(MAX_TASK_NUMBER);
}

LocalAbilityManager::~LocalAbilityManager()
{
    ondemandPool_->Stop();
}

void LocalAbilityManager::DoStartSAProcess(const std::string& profilePath, int32_t saId)
{
    HILOGI(TAG, "DoStartSAProcess saId : %d", saId);
    string realProfilePath = "";
    if (!CheckAndGetProfilePath(profilePath, realProfilePath)) {
        HILOGE(TAG, "DoStartSAProcess invalid path");
        return;
    }
    {
        std::string traceTag = GetTraceTag(realProfilePath);
        // HITRACE_METER_NAME(HITRACE_TAG_SAMGR, traceTag);
        bool ret = InitSystemAbilityProfiles(realProfilePath, saId);
        if (!ret) {
            HILOGE(TAG, "InitSystemAbilityProfiles no right profile, will exit");
            return;
        }
        ret = CheckSystemAbilityManagerReady();
        if (!ret) {
            HILOGE(TAG, "CheckSystemAbilityManagerReady failed! will exit");
            return;
        }
        ret = InitializeSaProfiles(saId);
        if (!ret) {
            HILOGE(TAG, "InitializeSaProfiles failed! will exit");
            return;
        }
        ret = Run(saId);
        if (!ret) {
            HILOGE(TAG, "Run failed! will exit");
            return;
        }
    }

    IPCSkeleton::JoinWorkThread();
    ClearResource();
    HILOGE(TAG, "JoinWorkThread stop, will exit");
}

std::string LocalAbilityManager::GetTraceTag(const std::string& profilePath)
{
    std::vector<std::string> libPathVec;
    string traceTag = "default_proc";
    SplitStr(profilePath, "/", libPathVec);
    if ((libPathVec.size() > 0)) {
        traceTag = libPathVec[libPathVec.size() - 1];
        auto size = traceTag.length();
        if (size > SUFFIX_LENGTH) {
            return traceTag.substr(0, size - SUFFIX_LENGTH);
        }
    }
    return traceTag;
}

bool LocalAbilityManager::CheckAndGetProfilePath(const std::string& profilePath, std::string& realProfilePath)
{
    if (profilePath.length() > PATH_MAX) {
        HILOGE(TAG, "profilePath length too long!");
        return false;
    }
    char realPath[PATH_MAX] = {'\0'};
    if (realpath(profilePath.c_str(), realPath) == nullptr) {
        HILOGE(TAG, "xmlDocName path does not exist!");
        return false;
    }
    // realProfilePath must begin with "/system/profile/" or begin with "/system/usr/"
    realProfilePath = realPath;
    if (realProfilePath.find(PROFILES_DIR) != 0 && realProfilePath.find(DEFAULT_DIR) != 0) {
        HILOGE(TAG, "xmlDoc dir is not matched");
        return false;
    }
    return true;
}

bool LocalAbilityManager::CheckSystemAbilityManagerReady()
{
    int32_t timeout = RETRY_TIMES_FOR_SAMGR;
    constexpr int32_t duration = std::chrono::microseconds(MILLISECONDS_WAITING_SAMGR_ONE_TIME).count();
    sptr<ISystemAbilityManager> samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    while (samgrProxy == nullptr) {
        HILOGI(TAG, "waiting for samgr...");
        if (timeout > 0) {
            usleep(duration);
            samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        } else {
            HILOGE(TAG, "wait for samgr time out (10s)");
            return false;
        }
        timeout--;
    }
    return true;
}

bool LocalAbilityManager::InitSystemAbilityProfiles(const std::string& profilePath, int32_t saId)
{
    HILOGD(TAG, "[PerformanceTest] SAFWK parse system ability profiles!");
    int64_t begin = GetTickCount();
    bool ret = profileParser_->ParseSaProfiles(profilePath);
    if (!ret) {
        HILOGW(TAG, "ParseSaProfiles failed!");
        return false;
    }

    procName_ = profileParser_->GetProcessName();
    auto saInfos = profileParser_->GetAllSaProfiles();
    std::string process = Str16ToStr8(procName_);
    HILOGI(TAG, "[PerformanceTest] SAFWK parse process:%{public}s system ability profiles finished, spend:%{public}"
        PRId64 " ms", process.c_str(), (GetTickCount() - begin));
    std::string path = PREFIX + process + SUFFIX;
    bool isExist = profileParser_->CheckPathExist(path);
    if (isExist) {
        CheckTrustSa(path, process, saInfos);
    }
    begin = GetTickCount();
    if (saId != DEFAULT_SAID) {
        HILOGD(TAG, "[PerformanceTest] SAFWK LoadSaLib systemAbilityId:%{public}d", saId);
        bool result = profileParser_->LoadSaLib(saId);
        HILOGI(TAG, "[PerformanceTest] SAFWK LoadSaLib systemAbilityId:%{public}d finished, spend:%{public}"
            PRId64 " ms", saId, (GetTickCount() - begin));
        return result;
    } else {
        HILOGD(TAG, "[PerformanceTest] SAFWK load all libraries");
        profileParser_->OpenSo();
        HILOGI(TAG, "[PerformanceTest] SAFWK load all libraries finished, spend:%{public}" PRId64 " ms",
            (GetTickCount() - begin));
        return true;
    }
}

void LocalAbilityManager::CheckTrustSa(const std::string& path, const std::string& process,
    const std::list<SaProfile>& saInfos)
{
    HILOGD(TAG, "CheckTrustSa start");
    std::map<std::u16string, std::set<int32_t>> trustMaps;
    bool ret = profileParser_->ParseTrustConfig(path, trustMaps);
    if (ret && !trustMaps.empty()) {
        // 1.get allowed sa set in the process
        const auto& saSets = trustMaps[Str8ToStr16(process)];
        // 2.check to-load sa in the allowed sa set, and if to-load sa not in the allowed, will remove and not load it
        for (const auto& saInfo : saInfos) {
            if (saSets.find(saInfo.saId) == saSets.end()) {
                HILOGW(TAG, "sa : %{public}d not allow to load in %{public}s", saInfo.saId, process.c_str());
                profileParser_->RemoveSaProfile(saInfo.saId);
            }
        }
    }
}

void LocalAbilityManager::ClearResource()
{
    profileParser_->ClearResource();
}

bool LocalAbilityManager::AddAbility(SystemAbility* ability)
{
    if (ability == nullptr) {
        HILOGW(TAG, "try to add null ability!");
        return false;
    }

    int32_t saId = ability->GetSystemAbilitId();
    SaProfile saProfile;
    bool ret = profileParser_->GetProfile(saId, saProfile);
    if (!ret) {
        return false;
    }
    std::unique_lock<std::shared_mutex> writeLock(abilityMapLock_);
    auto iter = abilityMap_.find(saId);
    if (iter != abilityMap_.end()) {
        HILOGW(TAG, "try to add existed ability:%{public}d!", saId);
        return false;
    }
    HILOGI(TAG, "set profile attributes for SA:%{public}d", saId);
    ability->SetLibPath(saProfile.libPath);
    ability->SetRunOnCreate(saProfile.runOnCreate);
    ability->SetDependSa(saProfile.dependSa);
    ability->SetDependTimeout(saProfile.dependTimeout);
    ability->SetDistributed(saProfile.distributed);
    ability->SetDumpLevel(saProfile.dumpLevel);
    ability->SetCapability(saProfile.capability);
    ability->SetPermission(saProfile.permission);
    abilityMap_.emplace(saId, ability);
    return true;
}

bool LocalAbilityManager::RemoveAbility(int32_t systemAbilityId)
{
    if (systemAbilityId <= 0) {
        HILOGW(TAG, "invalid systemAbilityId");
        return false;
    }
    std::unique_lock<std::shared_mutex> writeLock(abilityMapLock_);
    (void)abilityMap_.erase(systemAbilityId);
    return true;
}

bool LocalAbilityManager::AddSystemAbilityListener(int32_t systemAbilityId, int32_t listenerSaId)
{
    if (!CheckInputSysAbilityId(systemAbilityId) || !CheckInputSysAbilityId(listenerSaId)) {
        HILOGW(TAG, "SA:%{public}d or listenerSA:%{public}d invalid!",
            systemAbilityId, listenerSaId);
        return false;
    }
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        HILOGE(TAG, "failed to get samgrProxy");
        return false;
    }

    {
        HILOGD(TAG, "SA:%{public}d, listenerSA:%{public}d", systemAbilityId, listenerSaId);
        std::lock_guard<std::mutex> autoLock(listenerLock_);
        auto& listenerSaIdList = listenerMap_[systemAbilityId];
        auto iter = std::find_if(listenerSaIdList.begin(), listenerSaIdList.end(), [listenerSaId](int32_t SaId) {
            return SaId == listenerSaId;
        });
        if (iter == listenerSaIdList.end()) {
            listenerSaIdList.emplace_back(listenerSaId);
        }
        HILOGI(TAG, "AddSystemAbilityListener SA:%{public}d, size:%{public}zu", systemAbilityId,
            listenerSaIdList.size());
        if (listenerSaIdList.size() > 1) {
            sptr<IRemoteObject> object = samgrProxy->CheckSystemAbility(systemAbilityId);
            if (object != nullptr) {
                NotifyAbilityListener(systemAbilityId, listenerSaId, "",
                    ISystemAbilityStatusChange::ON_ADD_SYSTEM_ABILITY);
            }
            return true;
        }
    }

    int32_t ret = samgrProxy->SubscribeSystemAbility(systemAbilityId, GetSystemAbilityStatusChange());
    if (ret) {
        HILOGE(TAG, "failed to subscribe sa:%{public}d, process name:%{public}s", systemAbilityId,
            Str16ToStr8(procName_).c_str());
        return false;
    }
    return true;
}

bool LocalAbilityManager::RemoveSystemAbilityListener(int32_t systemAbilityId, int32_t listenerSaId)
{
    if (!CheckInputSysAbilityId(systemAbilityId) || !CheckInputSysAbilityId(listenerSaId)) {
        HILOGW(TAG, "SA:%{public}d or listenerSA:%{public}d invalid!",
            systemAbilityId, listenerSaId);
        return false;
    }

    {
        HILOGD(TAG, "SA:%{public}d, listenerSA:%{public}d", systemAbilityId, listenerSaId);
        std::lock_guard<std::mutex> autoLock(listenerLock_);
        if (listenerMap_.count(systemAbilityId) == 0) {
            return true;
        }
        auto& listenerSaIdList = listenerMap_[systemAbilityId];
        auto iter = std::find_if(listenerSaIdList.begin(), listenerSaIdList.end(), [listenerSaId](int32_t SaId) {
            return SaId == listenerSaId;
        });
        if (iter != listenerSaIdList.end()) {
            listenerSaIdList.erase(iter);
        }
        HILOGI(TAG, "RemoveSystemAbilityListener SA:%{public}d, size:%{public}zu", systemAbilityId,
            listenerSaIdList.size());
        if (!listenerSaIdList.empty()) {
            return true;
        }
        listenerMap_.erase(systemAbilityId);
    }

    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        HILOGE(TAG, "failed to get samgrProxy");
        return false;
    }
    int32_t ret = samgrProxy->UnSubscribeSystemAbility(systemAbilityId, GetSystemAbilityStatusChange());
    if (ret) {
        HILOGE(TAG, "failed to unsubscribe SA:%{public}d, process name:%{public}s",
            systemAbilityId, Str16ToStr8(procName_).c_str());
        return false;
    }
    return true;
}

void LocalAbilityManager::NotifyAbilityListener(int32_t systemAbilityId, int32_t listenerSaId,
    const std::string& deviceId, int32_t code)
{
    HILOGI(TAG, "SA:%{public}d, listenerSA:%{public}d, code:%{public}d", systemAbilityId, listenerSaId, code);
    auto ability = GetAbility(listenerSaId);
    if (ability == nullptr) {
        HILOGE(TAG, "failed to get listener SA:%{public}d", listenerSaId);
        return;
    }

    switch (code) {
        case ISystemAbilityStatusChange::ON_ADD_SYSTEM_ABILITY: {
            HILOGD(TAG, "OnAddSystemAbility, SA:%{public}d", listenerSaId);
            ability->OnAddSystemAbility(systemAbilityId, deviceId);
            break;
        }
        case ISystemAbilityStatusChange::ON_REMOVE_SYSTEM_ABILITY: {
            HILOGD(TAG, "OnRemoveSystemAbility, SA:%{public}d", listenerSaId);
            ability->OnRemoveSystemAbility(systemAbilityId, deviceId);
            break;
        }
        default:
            break;
    }
}

void LocalAbilityManager::FindAndNotifyAbilityListeners(int32_t systemAbilityId,
    const std::string& deviceId, int32_t code)
{
    HILOGD(TAG, "SA:%{public}d, code:%{public}d", systemAbilityId, code);
    int64_t begin = GetTickCount();
    std::list<int32_t> listenerSaIdList;
    {
        std::lock_guard<std::mutex> autoLock(listenerLock_);
        auto iter = listenerMap_.find(systemAbilityId);
        if (iter != listenerMap_.end()) {
            listenerSaIdList = iter->second;
        } else {
            HILOGW(TAG, "SA:%{public}d not found", systemAbilityId);
        }
    }
    for (auto listenerSaId : listenerSaIdList) {
        NotifyAbilityListener(systemAbilityId, listenerSaId, deviceId, code);
    }
    HILOGI(TAG, "SA:%{public}d, code:%{public}d spend:%{public}" PRId64 " ms", systemAbilityId, code,
        GetTickCount() - begin);
}

bool LocalAbilityManager::OnStartAbility(int32_t systemAbilityId)
{
    HILOGD(TAG, "try to start SA:%{public}d", systemAbilityId);
    auto ability = GetAbility(systemAbilityId);
    if (ability == nullptr) {
        return false;
    }
    ability->Start();
    return true;
}

SystemAbility* LocalAbilityManager::GetAbility(int32_t systemAbilityId)
{
    std::shared_lock<std::shared_mutex> readLock(abilityMapLock_);
    auto it = abilityMap_.find(systemAbilityId);
    if (it == abilityMap_.end()) {
        HILOGW(TAG, "SA:%{public}d not register", systemAbilityId);
        return nullptr;
    }

    return it->second;
}

bool LocalAbilityManager::GetRunningStatus(int32_t systemAbilityId)
{
    auto ability = GetAbility(systemAbilityId);
    if (ability == nullptr) {
        return false;
    }

    return ability->GetRunningStatus();
}

void LocalAbilityManager::StartOndemandSystemAbility(int32_t systemAbilityId)
{
    HILOGD(TAG, "[PerformanceTest] SAFWK ondemand LoadSaLib systemAbilityId:%{public}d library", systemAbilityId);
    int64_t begin = GetTickCount();
    bool isExist = profileParser_->LoadSaLib(systemAbilityId);
    HILOGI(TAG, "[PerformanceTest] SAFWK ondemand LoadSaLib systemAbilityId:%{public}d, spend:%{public}" PRId64 " ms",
        systemAbilityId, (GetTickCount() - begin));
    if (isExist) {
        int32_t timeout = RETRY_TIMES_FOR_ONDEMAND;
        constexpr int32_t duration = std::chrono::microseconds(MILLISECONDS_WAITING_ONDEMAND_ONE_TIME).count();
        {
            std::shared_lock<std::shared_mutex> readLock(abilityMapLock_);
            auto it = abilityMap_.find(systemAbilityId);
            while (it == abilityMap_.end()) {
                HILOGI(TAG, "waiting for SA:%{public}d...", systemAbilityId);
                if (timeout > 0) {
                    usleep(duration);
                    it = abilityMap_.find(systemAbilityId);
                } else {
                    HILOGE(TAG, "waiting for SA:%{public}d time out (1s)", systemAbilityId);
                    return;
                }
                timeout--;
            }
        }

        if (!OnStartAbility(systemAbilityId)) {
            HILOGE(TAG, "failed to start ability:%{public}d", systemAbilityId);
        }
    } else {
        HILOGW(TAG, "SA:%{public}d not found", systemAbilityId);
    }
}

bool LocalAbilityManager::StartAbility(int32_t systemAbilityId)
{
    HILOGI(TAG, "[PerformanceTest] SAFWK received start systemAbilityId:%{public}d request", systemAbilityId);
    auto task = std::bind(&LocalAbilityManager::StartOndemandSystemAbility, this, systemAbilityId);
    ondemandPool_->AddTask(task);
    return true;
}

bool LocalAbilityManager::InitializeSaProfiles(int32_t saId)
{
    return (saId == DEFAULT_SAID) ? InitializeRunOnCreateSaProfiles() : InitializeOnDemandSaProfile(saId);
}

bool LocalAbilityManager::InitializeRunOnCreateSaProfiles()
{
    HILOGD(TAG, "initializing run-on-create sa profiles...");
    auto& saProfileList = profileParser_->GetAllSaProfiles();
    if (saProfileList.empty()) {
        HILOGW(TAG, "sa profile is empty");
        return false;
    }

    for (const auto& saProfile : saProfileList) {
        if (!InitializeSaProfilesInnerLocked(saProfile)) {
            HILOGW(TAG, "SA:%{public}d init fail", saProfile.saId);
            continue;
        }
    }
    return true;
}

bool LocalAbilityManager::InitializeOnDemandSaProfile(int32_t saId)
{
    HILOGD(TAG, "initializing ondemand sa profile...");
    SaProfile saProfile;
    bool ret = profileParser_->GetProfile(saId, saProfile);
    if (ret) {
        return InitializeSaProfilesInnerLocked(saProfile);
    }
    return false;
}

bool LocalAbilityManager::InitializeSaProfilesInnerLocked(const SaProfile& saProfile)
{
    std::unique_lock<std::shared_mutex> writeLock(abilityMapLock_);
    auto iterProfile = abilityMap_.find(saProfile.saId);
    if (iterProfile == abilityMap_.end()) {
        HILOGW(TAG, "SA:%{public}d not found", saProfile.saId);
        return false;
    }
    auto systemAbility = iterProfile->second;
    if (systemAbility == nullptr) {
        HILOGW(TAG, "SA:%{public}d is null", saProfile.saId);
        return false;
    }
    uint32_t phase = OTHER_START;
    if (saProfile.bootPhase == BOOT_START_PHASE) {
        phase = BOOT_START;
    } else if (saProfile.bootPhase == CORE_START_PHASE) {
        phase = CORE_START;
    }
    auto& saList = abilityPhaseMap_[phase];
    saList.emplace_back(systemAbility);
    return true;
}

vector<u16string> LocalAbilityManager::CheckDependencyStatus(const vector<u16string>& dependSa)
{
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        HILOGW(TAG, "failed to get samgrProxy");
        return dependSa;
    }
    vector<u16string> checkSaStatusResult;
    for (const auto& saName : dependSa) {
        int32_t systemAbilityId = 0;
        StrToInt(Str16ToStr8(saName), systemAbilityId);
        if (CheckInputSysAbilityId(systemAbilityId)) {
            sptr<IRemoteObject> saObject = samgrProxy->CheckSystemAbility(systemAbilityId);
            if (saObject == nullptr) {
                checkSaStatusResult.emplace_back(saName);
            }
        } else {
            HILOGW(TAG, "dependency's id:%{public}s is invalid", Str16ToStr8(saName).c_str());
        }
    }

    return checkSaStatusResult;
}

void LocalAbilityManager::StartSystemAbilityTask(SystemAbility* ability)
{
    if (ability != nullptr) {
        HILOGD(TAG, "StartSystemAbility is called for %{public}d", ability->GetSystemAbilitId());
        if (ability->GetDependSa().empty()) {
            ability->Start();
        } else {
            int64_t start = GetTickCount();
            int64_t dependTimeout = ability->GetDependTimeout();
            while (!CheckDependencyStatus(ability->GetDependSa()).empty()) {
                int64_t end = GetTickCount();
                int64_t duration = ((end >= start) ? (end - start) : (INT64_MAX - end + start));
                if (duration < dependTimeout) {
                    usleep(CHECK_DEPENDENT_SA_PERIOD);
                } else {
                    break;
                }
            }
            vector<u16string> unpreparedDeps = CheckDependencyStatus(ability->GetDependSa());
            if (unpreparedDeps.empty()) {
                ability->Start();
            } else {
                for (const auto& unpreparedDep : unpreparedDeps) {
                    HILOGI(TAG, "%{public}d's dependency:%{public}s not started in %{public}d ms",
                        ability->GetSystemAbilitId(), Str16ToStr8(unpreparedDep).c_str(), ability->GetDependTimeout());
                }
            }
        }
    }

    std::lock_guard<std::mutex> lock(startPhaseLock_);
    if (startTaskNum_ > 0) {
        --startTaskNum_;
    }
    startPhaseCV_.notify_one();
}

void LocalAbilityManager::RegisterOnDemandSystemAbility(int32_t saId)
{
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        HILOGI(TAG, "failed to get samgrProxy");
        return;
    }

    auto& saProfileList = profileParser_->GetAllSaProfiles();
    for (const auto& saProfile : saProfileList) {
        if (NeedRegisterOnDemand(saProfile, saId)) {
            HILOGD(TAG, "register ondemand ability:%{public}d to samgr", saProfile.saId);
            int32_t ret = samgrProxy->AddOnDemandSystemAbilityInfo(saProfile.saId, procName_);
            if (ret != ERR_OK) {
                HILOGI(TAG, "failed to add ability info for on-demand SA:%{public}d", saProfile.saId);
            }
        }
    }
}

// on default load, not-run-on-create SA will register to OnDemandSystemAbility
// on demand load, other SA will register to OnDemandSystemAbility, although some are runOnCreate
bool LocalAbilityManager::NeedRegisterOnDemand(const SaProfile& saProfile, int32_t saId)
{
    return (saId == DEFAULT_SAID && !saProfile.runOnCreate) ||
        (saId != DEFAULT_SAID && saProfile.saId != saId);
}

void LocalAbilityManager::StartPhaseTasks(const std::list<SystemAbility*>& systemAbilityList)
{
    if (systemAbilityList.empty()) {
        return;
    }

    for (auto systemAbility : systemAbilityList) {
        if (systemAbility != nullptr) {
            HILOGD(TAG, "add phase task for SA:%{public}d", systemAbility->GetSystemAbilitId());
            std::lock_guard<std::mutex> autoLock(startPhaseLock_);
            ++startTaskNum_;
            auto task = std::bind(&LocalAbilityManager::StartSystemAbilityTask, this, systemAbility);
            initPool_->AddTask(task);
        }
    }

    int64_t begin = GetTickCount();
    HILOGD(TAG, "start waiting for all tasks!");
    std::unique_lock<std::mutex> lck(startPhaseLock_);
    if (!startPhaseCV_.wait_for(lck, std::chrono::seconds(MAX_SA_STARTUP_TIME),
        [this] () { return startTaskNum_ == 0; })) {
        HILOGW(TAG, "start timeout!");
    }
    startTaskNum_ = 0;
    int64_t end = GetTickCount();
    HILOGI(TAG, "start tasks finished and spend %{public}" PRId64 " ms", (end - begin));
}

void LocalAbilityManager::FindAndStartPhaseTasks()
{
    std::shared_lock<std::shared_mutex> readLock(abilityMapLock_);
    for (uint32_t startType = BOOT_START; startType <= OTHER_START; ++startType) {
        auto iter = abilityPhaseMap_.find(startType);
        if (iter != abilityPhaseMap_.end()) {
            StartPhaseTasks(iter->second);
        }
    }
}

bool LocalAbilityManager::Run(int32_t saId)
{
    HILOGD(TAG, "local ability manager is running...");
    bool addResult = AddLocalAbilityManager();
    if (!addResult) {
        HILOGE(TAG, "failed to add local abilitymanager");
        return false;
    }
    HILOGD(TAG, "success to add process name:%{public}s", Str16ToStr8(procName_).c_str());
    uint32_t concurrentThreads = std::thread::hardware_concurrency();
    HILOGI(TAG, "concurrentThreads is %{public}d", concurrentThreads);
    initPool_->Start(concurrentThreads);
    initPool_->SetMaxTaskNum(MAX_TASK_NUMBER);

    FindAndStartPhaseTasks();
    RegisterOnDemandSystemAbility(saId);
    initPool_->Stop();
    return true;
}

bool LocalAbilityManager::AddLocalAbilityManager()
{
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        HILOGE(TAG, "failed to get samgrProxy");
        return false;
    }

    if (localAbilityManager_ == nullptr) {
        localAbilityManager_ = this;
    }
    int32_t ret = samgrProxy->AddSystemProcess(procName_, localAbilityManager_);
    return ret == ERR_OK;
}

sptr<ISystemAbilityStatusChange> LocalAbilityManager::GetSystemAbilityStatusChange()
{
    std::lock_guard<std::mutex> autoLock(listenerLock_);
    if (statusChangeListener_ == nullptr) {
        statusChangeListener_ = new SystemAbilityListener();
    }
    return statusChangeListener_;
}

void LocalAbilityManager::SystemAbilityListener::OnAddSystemAbility(int32_t systemAbilityId,
    const std::string& deviceId)
{
    HILOGD(TAG, "SA:%{public}d added", systemAbilityId);
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW(TAG, "SA:%{public}d is invalid!", systemAbilityId);
        return;
    }

    GetInstance().FindAndNotifyAbilityListeners(systemAbilityId, deviceId,
        ISystemAbilityStatusChange::ON_ADD_SYSTEM_ABILITY);
}

void LocalAbilityManager::SystemAbilityListener::OnRemoveSystemAbility(int32_t systemAbilityId,
    const std::string& deviceId)
{
    HILOGD(TAG, "SA:%{public}d removed", systemAbilityId);
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW(TAG, "SA:%{public}d is invalid!", systemAbilityId);
        return;
    }

    GetInstance().FindAndNotifyAbilityListeners(systemAbilityId, deviceId,
        ISystemAbilityStatusChange::ON_REMOVE_SYSTEM_ABILITY);
}
}
