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

#ifndef SERVICES_SAMGR_NATIVE_INCLUDE_SYSTEM_ABILITY_MANAGER_H_
#define SERVICES_SAMGR_NATIVE_INCLUDE_SYSTEM_ABILITY_MANAGER_H_

#include <map>
#include <mutex>
#include <set>
#include <shared_mutex>
#include <string>
#include <utility>

#include "event_handler.h"
#include "dbinder_service.h"
#include "dbinder_service_stub.h"
#include "rpc_callback_imp.h"
#include "thread_pool.h"
#include "timer.h"
#include "sa_profiles.h"
#include "system_ability_definition.h"
#include "system_ability_manager_stub.h"

namespace OHOS {
struct SAInfo {
    sptr<IRemoteObject> remoteObj;
    bool isDistributed = false;
    std::u16string capability;
    std::string permission;
};

enum {
    UUID = 0,
    NODE_ID,
    UNKNOWN,
};

class SystemAbilityManager : public SystemAbilityManagerStub {
public:
    virtual ~SystemAbilityManager();
    static sptr<SystemAbilityManager> GetInstance();

    int32_t RemoveSystemAbility(const sptr<IRemoteObject>& ability);
    std::vector<std::u16string> ListSystemAbilities(uint32_t dumpFlags) override;

    void SetDeviceName(const std::u16string &name);

    const std::u16string& GetDeviceName() const;

    const sptr<DBinderService> GetDBinder() const;

    sptr<IRemoteObject> GetSystemAbility(int32_t systemAbilityId) override;

    sptr<IRemoteObject> CheckSystemAbility(int32_t systemAbilityId) override;

    int32_t RemoveSystemAbility(int32_t systemAbilityId) override;

    int32_t SubscribeSystemAbility(int32_t systemAbilityId, const sptr<ISystemAbilityStatusChange>& listener) override;
    int32_t UnSubscribeSystemAbility(int32_t systemAbilityId,
        const sptr<ISystemAbilityStatusChange>& listener) override;
    void UnSubscribeSystemAbility(const sptr<IRemoteObject>& remoteObject);

    sptr<IRemoteObject> GetSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;

    sptr<IRemoteObject> CheckSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;

    int32_t AddOnDemandSystemAbilityInfo(int32_t systemAbilityId, const std::u16string& procName) override;

    sptr<IRemoteObject> CheckSystemAbility(int32_t systemAbilityId, bool& isExist) override;

    void NotifyRemoteSaDied(const std::u16string& name);
    void NotifyRemoteDeviceOffline(const std::string& deviceId);
    int32_t AddSystemAbility(int32_t systemAbilityId, const sptr<IRemoteObject>& ability,
        const SAExtraProp& extraProp) override;
    std::string TransformDeviceId(const std::string& deviceId, int32_t type, bool isPrivate);
    std::string GetLocalNodeId();
    void Init();

    int32_t AddSystemProcess(const std::u16string& procName, const sptr<IRemoteObject>& procObject) override;
    int32_t RemoveSystemProcess(const sptr<IRemoteObject>& procObject);

    int32_t LoadSystemAbility(int32_t systemAbilityId, const sptr<ISystemAbilityLoadCallback>& callback) override;
    int32_t LoadSystemAbility(int32_t systemAbilityId, const std::string& deviceId,
        const sptr<ISystemAbilityLoadCallback>& callback) override;
    void OnAbilityCallbackDied(const sptr<IRemoteObject>& remoteObject);
    void OnRemoteCallbackDied(const sptr<IRemoteObject>& remoteObject);
    sptr<IRemoteObject> GetSystemAbilityFromRemote(int32_t systemAbilityId);
    bool LoadSystemAbilityFromRpc(const std::string& srcDeviceId, int32_t systemAbilityId,
        const sptr<ISystemAbilityLoadCallback>& callback);
    void NotifyRpcLoadCompleted(const std::string& srcDeviceId, int32_t systemAbilityId,
        const sptr<IRemoteObject>& remoteObject);
    void StartDfxTimer();
    void DoLoadForPerf();
private:
    enum class AbilityState {
        INIT,
        STARTING,
        STARTED,
    };

    using CallbackList = std::list<std::pair<sptr<ISystemAbilityLoadCallback>, int32_t>>;

    struct AbilityItem {
        AbilityState state = AbilityState::INIT;
        std::map<std::string, CallbackList> callbackMap; // key : networkid
    };

    SystemAbilityManager();
    void DoInsertSaData(const std::u16string& name, const sptr<IRemoteObject>& ability, const SAExtraProp& extraProp);
    bool IsNameInValid(const std::u16string& name);
    int32_t StartOnDemandAbility(int32_t systemAbilityId);
    void ParseRemoteSaName(const std::u16string& name, std::string& deviceId, std::u16string& saName);
    bool IsLocalDeviceId(const std::string& deviceId);
    bool CheckDistributedPermission();
    int32_t AddSystemAbility(const std::u16string& name, const sptr<IRemoteObject>& ability,
        const SAExtraProp& extraProp);
    int32_t FindSystemAbilityNotify(int32_t systemAbilityId, int32_t code);
    int32_t FindSystemAbilityNotify(int32_t systemAbilityId, const std::string& deviceId, int32_t code);

    sptr<IRemoteObject> GetSystemProcess(const std::u16string& procName);

    void InitSaProfile();
    bool GetSaProfile(int32_t saId, SaProfile& saProfile);
    void NotifySystemAbilityChanged(int32_t systemAbilityId, const std::string& deviceId, int32_t code,
        const sptr<ISystemAbilityStatusChange>& listener);
    void UnSubscribeSystemAbilityLocked(std::list<std::pair<sptr<ISystemAbilityStatusChange>, int32_t>>& listenerList,
        const sptr<IRemoteObject>& listener);

    void SendSystemAbilityAddedMsg(int32_t systemAbilityId, const sptr<IRemoteObject>& remoteObject);
    void SendSystemAbilityRemovedMsg(int32_t systemAbilityId);

    void NotifySystemAbilityLoaded(int32_t systemAbilityId, const sptr<IRemoteObject>& remoteObject);
    void NotifySystemAbilityLoaded(int32_t systemAbilityId, const sptr<IRemoteObject>& remoteObject,
        const sptr<ISystemAbilityLoadCallback>& callback);
    void NotifySystemAbilityLoadFail(int32_t systemAbilityId, const sptr<ISystemAbilityLoadCallback>& callback);
    int32_t StartingSystemProcess(const std::u16string& name, int32_t systemAbilityId);
    void StartOnDemandAbility(const std::u16string& name, int32_t systemAbilityId);
    void StartOnDemandAbilityInner(const std::u16string& name, int32_t systemAbilityId, AbilityItem& abilityItem);
    int32_t StartDynamicSystemProcess(const std::u16string& name, int32_t systemAbilityId);
    void RemoveStartingAbilityCallback(CallbackList& callbackList, const sptr<IRemoteObject>& remoteObject);
    void RemoveStartingAbilityCallbackForDevice(AbilityItem& abilityItem, const sptr<IRemoteObject>& remoteObject);
    void RemoveStartingAbilityCallbackLocked(std::pair<sptr<ISystemAbilityLoadCallback>, int32_t>& itemPair);
    void SendCheckLoadedMsg(int32_t systemAbilityId, const std::u16string& name, const std::string& srcDeviceId,
        const sptr<ISystemAbilityLoadCallback>& callback);
    void RemoveCheckLoadedMsg(int32_t systemAbilityId);
    void SendLoadedSystemAblityMsg(int32_t systemAbilityId, const sptr<IRemoteObject>& remoteObject,
        const sptr<ISystemAbilityLoadCallback>& callback);
    void DoLoadRemoteSystemAbility(int32_t systemAbilityId, int32_t callingPid,
        int32_t callingUid, const std::string& deviceId, const sptr<ISystemAbilityLoadCallback>& callback);
    sptr<DBinderServiceStub> DoMakeRemoteBinder(int32_t systemAbilityId, int32_t callingPid, int32_t callingUid,
        const std::string& deviceId);
    void RemoveRemoteCallbackLocked(std::list<sptr<ISystemAbilityLoadCallback>>& callbacks,
        const sptr<IRemoteObject>& remoteObject);
    void CleanCallbackForLoadFailed(int32_t systemAbilityId, const std::u16string& name,
        const std::string& srcDeviceId, const sptr<ISystemAbilityLoadCallback>& callback);

    void UpdateSaFreMap(int32_t pid, int32_t saId);
    uint64_t GenerateFreKey(int32_t pid, int32_t saId) const;
    void ReportGetSAPeriodically();
    void OndemandLoad();
    void OndemandLoadForPerf();
    std::list<int32_t> GetAllOndemandSa();

    std::u16string deviceName_;
    static sptr<SystemAbilityManager> instance;
    static std::mutex instanceLock;
    sptr<IRemoteObject::DeathRecipient> abilityDeath_;
    sptr<IRemoteObject::DeathRecipient> systemProcessDeath_;
    sptr<IRemoteObject::DeathRecipient> abilityStatusDeath_;
    sptr<IRemoteObject::DeathRecipient> abilityCallbackDeath_;
    sptr<IRemoteObject::DeathRecipient> remoteCallbackDeath_;
    sptr<DBinderService> dBinderService_;
    std::shared_ptr<RpcSystemAbilityCallback> rpcCallbackImp_;

    // must hold abilityMapLock_ never access other locks
    std::shared_mutex abilityMapLock_;
    std::map<int32_t, SAInfo> abilityMap_;

    // maybe hold listenerMapLock_ and then access onDemandLock_
    std::recursive_mutex listenerMapLock_;
    std::map<int32_t, std::list<std::pair<sptr<ISystemAbilityStatusChange>, int32_t>>> listenerMap_;
    std::map<int32_t, int32_t> subscribeCountMap_;

    std::recursive_mutex onDemandLock_;
    std::map<int32_t, std::u16string> onDemandAbilityMap_;
    std::map<int32_t, AbilityItem> startingAbilityMap_;
    std::map<std::u16string, sptr<IRemoteObject>> systemProcessMap_;
    std::map<std::u16string, int64_t> startingProcessMap_;
    std::map<int32_t, int32_t> callbackCountMap_;

    std::shared_ptr<AppExecFwk::EventHandler> workHandler_;

    std::map<int32_t, SaProfile> saProfileMap_;
    std::mutex saProfileMapLock_;

    std::mutex loadRemoteLock_;
    std::map<std::string, std::list<sptr<ISystemAbilityLoadCallback>>> remoteCallbacks_; // key : said_deviceId

    ThreadPool loadPool_;

    std::mutex saFrequencyLock_;
    std::map<uint64_t, int32_t> saFrequencyMap_; // {pid_said, count}

    std::unique_ptr<Utils::Timer> reportEventTimer_;
};
} // namespace OHOS

#endif // !defined(SERVICES_SAMGR_NATIVE_INCLUDE_SYSTEM_ABILITY_MANAGER_H_)
